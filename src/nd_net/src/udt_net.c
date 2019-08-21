/* file : udt_net.c 
 * implete udp data transfer protocol
 * neil 
 * 2007-11-27
 */

#include "nd_net/nd_netlib.h"

int _handle_income_data(nd_udt_node* socket_node, struct ndudt_pocket *pocket, size_t len);
extern int retranslate_data(nd_udt_node* socket_node);
extern int flush_send_window(nd_udt_node* socket_node);			//发送 窗口中的数据
//回复对方的连接请求
// 返回-1 出错,0 succes
int _handle_syn(nd_udt_node *socket_node,struct ndudt_pocket *pocket)
{
	if(socket_node->status==NETSTAT_LISTEN) {
		struct ndudt_pocket syn_ack ;		
		init_udt_pocket(&syn_ack) ;
		SET_SYN(&syn_ack) ;

		socket_node->acknowledged_seq = socket_node->send_sequence ;
		socket_node->received_sequence = pocket->sequence + 1;		//接受到对方的系列号+1以便回复对方
		
		set_pocket_ack(&syn_ack,socket_node->received_sequence) ;
		syn_ack.sequence = socket_node->send_sequence ;
		write_pocket_to_socket(socket_node, &syn_ack, ndt_header_size(&syn_ack));
		socket_node->status = NETSTAT_SYNRECV ;
		return 0 ;
	}

	else if(NETSTAT_SYNSEND==socket_node->status){
		//connect (client side received syn ack
		if(!pocket->header.ack || socket_node->send_sequence+1!=pocket->ack_seq) {
			nd_object_seterror((nd_handle)socket_node,NDERR_BADPACKET);
			nd_assert(0) ;
			return -1;
		}
		socket_node->local_port = pocket->local_port;
		socket_node->received_sequence = pocket->sequence +1 ;
		socket_node->acknowledged_seq = pocket->ack_seq ;		
		
		socket_node->status = NETSTAT_ESTABLISHED;
		++ (socket_node->send_sequence) ;
		udt_send_ack(socket_node) ;
		return 0 ;
	}
	
	else if(socket_node->status < NETSTAT_ACCEPT){
		
		nd_assert(socket_node->received_sequence == pocket->sequence + 1) ;
		if(socket_node->received_sequence == pocket->sequence + 1) {
			udt_send_ack(socket_node) ;
			return 0 ;
		}
		return -1 ;
	}
	else {
		nd_object_seterror((nd_handle)socket_node,NDERR_BADPACKET);
		return -1;
	}
}

/*
 * 处理udt数据包
 * 出错返回-1 ,需要根据socket::myerro检测相应的错误号,并做出相关处理
 * 否则返回处理数据的长度(有效数据)
 * 返回0 没有读取数据
 */
int _udt_packet_handler(nd_udt_node *socket_node,struct ndudt_pocket *pocket,size_t len)
{
	ENTER_FUNC()
	int ret = 0;

	if (socket_node->status & NETSTAT_RESET) {
		//ack reset
		struct ndudt_pocket pocket;
		init_udt_pocket(&pocket);
		SET_RESET(&pocket);
		write_pocket_to_socket(socket_node, &pocket, ndt_header_size(&pocket));
		LEAVE_FUNC();
		return 0;
	}

	socket_node->last_recv = nd_time();		//record received data time
	socket_node->update_tm = socket_node->last_recv ;

	nd_object_seterror((nd_handle)socket_node, NDERR_SUCCESS);

	if(pocket->window_len > 0)
		socket_node->window_len = pocket->window_len ;
	switch(POCKET_TYPE(pocket)) {
	case NDUDT_DATA:
		if(POCKET_ACK(pocket)) {
			_handle_ack(socket_node,pocket->ack_seq);
		}
		ret = _handle_income_data(socket_node, pocket, len) ;
		if(ret > 0) {
			//有数据到来,把产生的数据发送出去
			while (flush_send_window(socket_node) > 0){} 
		}
		break ;
	case NDUDT_DGRAM: 
		ret = (int)len;
		if (socket_node->data_entry) {		
			struct ndudp_packet *udp_pack = (struct ndudp_packet *) pocket;
			ret = socket_node->data_entry((nd_handle)socket_node, udp_pack->data, len - sizeof(struct ndudp_packet), socket_node->srv_root ) ;
		}
		break ;
	case NDUDT_SYN:
		ret = _handle_syn(socket_node, pocket) ;	//return -1 on error 0 success
		break ;
	case NDUDT_ALIVE:
		break ;
	case NDUDT_ACK:
		ret = _handle_ack(socket_node,pocket->ack_seq);
		break ;
	case NDUDT_LOST:
		_handle_ack(socket_node,pocket->ack_seq);
		{
			int offset_lost = pocket->sequence - socket_node->acknowledged_seq ;
			int offset_unget = socket_node->send_sequence - pocket->sequence ;
			if(offset_lost <= 0 || offset_unget < 0)
				return 0 ;
			socket_node->send_sequence = pocket->sequence ;		//对方丢失的为止,从这里开始重发
			retranslate_data(socket_node) ;
			//++socket_node->resend_times  ;
		}
		break ;
	case NDUDT_FIN:
		if(POCKET_ACK(pocket)) {
			_handle_ack(socket_node,pocket->ack_seq);
		}
		if (!(NETSTAT_RECVCLOSE&socket_node->status)) {
			socket_node->status |= NETSTAT_RECVCLOSE ;
			socket_node->received_sequence = pocket->sequence ; 
			if((NETSTAT_SENDCLOSE+NETSTAT_FINSEND) & socket_node->status) {
				socket_node->status |= NETSTAT_RESET ;
			}
		}
		udt_send_ack(socket_node);
		socket_node->myerrno = NDERR_CLOSED;
		ret = -1;
		break ;
	case NDUDT_RESET:
		socket_node->status |= NETSTAT_RESET ;
		socket_node->myerrno = NDERR_RESET ;
		ret = -1 ;
		break ;
	default :
		break ;
	}
	if (-1==ret && socket_node->myerrno == NDERR_BADPACKET){
		ret = 0 ;
	}
	LEAVE_FUNC();
	return ret;
}


//处理接受到的数据,并放入缓冲
//return value : 0  there is no valid data incoming,
//  on error return -1 check error code
// else return length of valid data 
int _handle_income_data(nd_udt_node* socket_node, struct ndudt_pocket *pocket, size_t len)
{
	ENTER_FUNC()
	int data_len, seq_offset ;
	char *data;
	nd_netbuf_t *recvbuf = &socket_node->recv_buffer;
	
	nd_assert(socket_node && pocket) ;

	data_len = (int)len - ndt_header_size(pocket);
	if(data_len <= 0) {
		LEAVE_FUNC();
		return 0 ;
	}

	//check sequence
	seq_offset = (int)(pocket->sequence -socket_node->received_sequence );
	if(seq_offset > 0){
		//丢包 这里需要保存一下等待下次包的到来
// 		struct ndudt_pocket lost_ntf;
// 		init_udt_pocket(&lost_ntf);
// 		set_pocket_ack(&lost_ntf, socket_node->received_sequence) ;
// 		SET_LOST(&lost_ntf);
// 		lost_ntf.sequence = pocket->sequence ;
// 		write_pocket_to_socket(socket_node, &lost_ntf, ndt_header_size(&lost_ntf));
		//nd_log_screen("lost data \n") ;

		_addto_pre_list(socket_node, pocket, (int)len); 

		LEAVE_FUNC();
		return 0;
	}
	else if(seq_offset < 0) {
		//这个是重传报,ack丢失
		udt_send_ack(socket_node);
		LEAVE_FUNC();
		return 0;
	}

	if(data_len > ndlbuf_free_capacity(& socket_node->recv_buffer) ){
		LEAVE_FUNC();
		return 0 ;
	}

	data = pocket_data(pocket) ;
	set_socket_ack( socket_node, 1) ;
	socket_node->received_sequence += data_len ;

	data_len = ndlbuf_write(recvbuf, data, data_len, EBUF_ALL);
	UDT_RECV_USER_DATA(socket_node) = 1;

	if (!list_empty(&socket_node->pre_list)) {
		_tryto_fetch_prelist(socket_node);
	}
	LEAVE_FUNC();
		
	return data_len ;
}


/*request connect with server*/
int _udt_syn(nd_udt_node *socket_node)
{
	int i,ret;
	size_t len ;
	struct ndudt_pocket syn_pocket ;
	udt_pocketbuf pocket ;
	init_udt_pocket(&syn_pocket) ;
	SET_SYN(&syn_pocket) ;
	//srand(nd_time());
	syn_pocket.sequence = socket_node->send_sequence ;//= rand();
	
	len = ndt_header_size(&syn_pocket);
	for (i=0; i<3; i++){
		if(-1==write_pocket_to_socket(socket_node,&syn_pocket, len) ) {
			return -1 ;
		}
		socket_node->status = NETSTAT_SYNSEND ;
		ret = read_packet_from_socket(socket_node, (char*)&pocket, sizeof(udt_pocketbuf), WAIT_CONNECT_TIME);
		if(ret>0) {
			if(_udt_packet_handler(socket_node,(struct ndudt_pocket *) &pocket, ret) >= 0) {
				if(NETSTAT_ESTABLISHED==socket_node->status)
					return 0 ;
			}
		}
	}
	return -1;
}


int udt_send_ack(nd_udt_node *socket_node)
{
	struct ndudt_pocket ack_pocket;
	init_udt_pocket(&ack_pocket);
	set_pocket_ack(&ack_pocket, socket_node->received_sequence) ;
	SET_ACK(&ack_pocket);

	return write_pocket_to_socket(socket_node, &ack_pocket, ndt_header_size(&ack_pocket));
}


int _handle_ack(nd_udt_node *socket_node, u_32 ack_seq)
{
	int notified =(int)( ack_seq - socket_node->acknowledged_seq) ;
	int unnotified = (int)(socket_node->send_sequence - ack_seq ) ;

	if (ack_seq <= socket_node->acknowledged_seq) {
		//already handled this ack 
		return 0;
	}

	if (socket_node->status==NETSTAT_SYNRECV){
		if(notified < 0 ){
			nd_object_seterror((nd_handle)socket_node,NDERR_BADPACKET);
			return 0;
		}
		//收到client的syn-ack
		nd_assert(socket_node->send_sequence+1== ack_seq) ;
		if(socket_node->send_sequence+1== ack_seq) {
			nd_udtsrv *root ;
			socket_node->status = NETSTAT_ACCEPT ;
			socket_node->acknowledged_seq = ack_seq ;
			++ (socket_node->send_sequence) ;		//准备开始下一个信息

			root =(nd_udtsrv *) socket_node->srv_root ;
			nd_assert(root) ;
			if (root && root->accept_proc) {
				root->accept_proc((nd_handle)root, socket_node, (SOCKADDR_IN*)&socket_node->remote_addr);
				socket_node->start_time = socket_node->update_tm;
				socket_node->status = NETSTAT_ESTABLISHED;
			}

		}

		return 0;
	}

	if((socket_node->status & NETSTAT_FINSEND) ){
		//发送被关闭
		if (socket_node->send_sequence==ack_seq){
			socket_node->send_sequence = ack_seq ;
			ndlbuf_reset(&socket_node->send_buffer);
			socket_node->status |= NETSTAT_SENDCLOSE ;
			if(NETSTAT_RECVCLOSE &socket_node->status) {
				socket_node->status |= NETSTAT_RESET ;
			}
		}
		return 0;
	}

	if (notified > 0){
		if( ndlbuf_datalen(&socket_node->send_buffer) > 0 && unnotified >= 0 ) {
			nd_assert(ndlbuf_datalen(&socket_node->send_buffer) >=notified );
			ndlbuf_sub_data(&socket_node->send_buffer,notified) ;
			socket_node->acknowledged_seq = ack_seq ;
			if(0==socket_node->is_retranslate) {
				ndtime_t interv = socket_node->update_tm - socket_node->last_resend_tm ;
				socket_node->retrans_timeout = calc_timeouval(&socket_node->_rtt, interv) ;
			}
		}
	}

	if (socket_node->is_retranslate) {
		if ((int)(ack_seq - socket_node->retrans_seq) >= 0)	{
			socket_node->is_retranslate = 0;
			socket_node->retrans_seq = ack_seq ;
			socket_node->resend_times = 0 ;
		}
	}
	return 0;
	
}


int udt_send_fin(nd_udt_node *socket_node)
{

	int ret;
	size_t len ;
	struct ndudt_pocket fin_pocket ;
	init_udt_pocket(&fin_pocket) ;
	SET_FIN(&fin_pocket) ;

	len = ndt_header_size(&fin_pocket);

	if (!(socket_node->status & NETSTAT_FINSEND )) {
		socket_node->status |= NETSTAT_FINSEND;
		socket_node->is_retranslate =0;
		socket_node->resend_times = 0;
	}
	else {
		socket_node->is_retranslate = 1;
		++socket_node->resend_times ;
	}

	fin_pocket.sequence = socket_node->send_sequence;// + 1 ;
	++(socket_node->resend_times) ;
	ret = write_pocket_to_socket(socket_node,&fin_pocket, len) ;
	if(ret > 0) {
		socket_node->last_resend_tm = socket_node->last_active ;
		return 0;
	}
	return -1;

}


int _test_checksum(struct ndudt_header *data, int size)
{
	NDUINT16 checksum, calc_cs;

	checksum = ntohs(data->checksum);
	data->checksum = 0;

	calc_cs = nd_checksum((NDUINT16*)data, size);
	
	if (checksum != calc_cs) {
		return 0;
	}
	return 1;
}

void _set_checksum(struct ndudt_header *data,int size)
{
	data->checksum = 0; 
	data->checksum = nd_checksum((NDUINT16*)data, size);

	data->checksum = htons(data->checksum);

}

//根据封包往返计算超时值
ndtime_t calc_timeouval(struct nd_rtt *rtt, int measuerment) 
{
	int ret ;
	int err = measuerment - (rtt->average>>3) ;
	rtt->average += err ;
	if(err<0) {
		err = -err ;
	}
	err = err - (rtt->deviation >> 2) ;
	rtt->deviation  += err ;

	ret = ((rtt->average>>2) + rtt->deviation ) >> 1 ;
	if(ret <500)
		ret = 500 ;
	return (ndtime_t)ret ;
}

