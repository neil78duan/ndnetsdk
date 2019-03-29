/* file : udt_net.c 
 * implete udp data transfer protocol
 * neil 
 * 2007-11-27
 */

//#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
//#include "nd_common/nd_alloc.h"


int _handle_income_data(nd_udt_node* socket_node, struct ndudt_pocket *pocket, size_t len);
extern int retranslate_data(nd_udt_node* socket_node);
extern int flush_send_window(nd_udt_node* socket_node);			//���� �����е�����
//�ظ��Է�����������
// ����-1 ����,0 succes
int _handle_syn(nd_udt_node *socket_node,struct ndudt_pocket *pocket)
{
	if(socket_node->status==NETSTAT_LISTEN) {
		struct ndudt_pocket syn_ack ;		
		init_udt_pocket(&syn_ack) ;
		SET_SYN(&syn_ack) ;

		socket_node->acknowledged_seq = socket_node->send_sequence ;
		socket_node->received_sequence = pocket->sequence + 1;		//���ܵ��Է���ϵ�к�+1�Ա�ظ��Է�
		
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
		socket_node->session_id = pocket->session_id ;
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
 * ����udt���ݰ�
 * ������-1 ,��Ҫ����socket::myerro�����Ӧ�Ĵ����,��������ش���
 * ���򷵻ش������ݵĳ���(��Ч����)
 * ����0 û�ж�ȡ����
 */
int _udt_packet_handler(nd_udt_node *socket_node,struct ndudt_pocket *pocket,size_t len)
{
	ENTER_FUNC()
	int ret = 0;	
	nd_object_seterror((nd_handle)socket_node, NDERR_SUCCESS);
	socket_node->last_recv = nd_time();		//record received data time
	socket_node->update_tm = socket_node->last_recv ;


	if(pocket->window_len > 0)
		socket_node->window_len = pocket->window_len ;
	switch(POCKET_TYPE(pocket)) {
	case NDUDT_DATA:
		if(POCKET_ACK(pocket)) {
			_handle_ack(socket_node,pocket->ack_seq);
		}
		ret = _handle_income_data(socket_node, pocket, len) ;
		if(ret > 0) {
			//�����ݵ���,�Ѳ��������ݷ��ͳ�ȥ
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
			socket_node->send_sequence = pocket->sequence ;		//�Է���ʧ��Ϊֹ,�����￪ʼ�ط�
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
				ret = -1 ;
			}
			socket_node->myerrno = NDERR_CLOSED;
		}
		udt_send_ack(socket_node) ;
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


//������ܵ�������,�����뻺��
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
	seq_offset = pocket->sequence -socket_node->received_sequence ;
	if(seq_offset > 0){
		//���� ������Ҫ����һ�µȴ��´ΰ��ĵ���
		struct ndudt_pocket lost_ntf;
		init_udt_pocket(&lost_ntf);
		set_pocket_ack(&lost_ntf, socket_node->received_sequence) ;
		SET_LOST(&lost_ntf);
		lost_ntf.sequence = pocket->sequence ;
		write_pocket_to_socket(socket_node, &lost_ntf, ndt_header_size(&lost_ntf));
		//nd_log_screen("lost data \n") ;
		LEAVE_FUNC();
		return 0;
	}
	else if(seq_offset < 0) {
		//������ش���,ack��ʧ
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
	if (data_len > 0 && socket_node->is_session) {
		if (-1 == handle_recv_data((nd_netui_handle)socket_node,(nd_handle)socket_node->srv_root)) {
			LEAVE_FUNC();
			return -1;
		}
	}

// 	if (!socket_node->data_entry){
// 		data_len = ndlbuf_write(recvbuf,data, data_len,EBUF_ALL) ;
// 	}
// 	else {
// 		//notify client program
// 		if (ndlbuf_datalen(recvbuf) > 0){
// 			int ret = 0 ;
// 			//size_t w_len = ndlbuf_write(recvbuf,data, data_len,EBUF_ALL) ;
// 			//nd_assert(w_len == data_len) ;
// 			ret = socket_node->data_entry((nd_handle)socket_node, ndlbuf_data(recvbuf), ndlbuf_datalen(recvbuf),(nd_handle) socket_node->srv_root) ;
// 			if(ret > 0) {
// 				ndlbuf_sub_data(recvbuf,ret) ;
// 			}
// 			else if(-1==ret) {
// 				socket_node->myerrno = NDERR_USER ;
// 				data_len = -1;
// 			}
// 		}
// 		else {
// 			int ret = 0 ;
// 			ret = socket_node->data_entry((nd_handle)socket_node, data,data_len, (nd_handle)socket_node->srv_root) ;						
// 			if(data_len != ret) {
// 				ndlbuf_write(recvbuf,data+ret, data_len - ret,EBUF_ALL) ;
// 			}
// 			else if(-1==ret) {
// 				socket_node->myerrno = NDERR_USER ;
// 				data_len = -1 ;
// 			}
// 
// 		}
// 	}

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
		//ret = _wait_data(socket_node, &pocket,WAIT_CONNECT_TIME) ;
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
		//�յ�client��syn-ack
		nd_assert(socket_node->send_sequence+1== ack_seq) ;
		if(socket_node->send_sequence+1== ack_seq) {
			nd_udtsrv *root ;
			socket_node->status = NETSTAT_ACCEPT ;
			socket_node->acknowledged_seq = ack_seq ;
			++ (socket_node->send_sequence) ;		//׼����ʼ��һ����Ϣ

			root =(nd_udtsrv *) socket_node->srv_root ;
			nd_assert(root) ;

			if(root && root->base.connect_in_callback){				
				//socket_node->start_time = nd_time() ;
				socket_node->start_time = socket_node->update_tm ;
				if(-1==root->base.connect_in_callback(socket_node,&socket_node->remote_addr,(nd_handle)root) ) {
					socket_node->myerrno = NDERR_USER;
					return -1;
				}
				socket_node->status = NETSTAT_ESTABLISHED ;
			}			
		}

		return 0;
	}

	if((socket_node->status & NETSTAT_FINSEND) ){
		//���ͱ��ر�
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
		socket_node->status |= NETSTAT_FINSEND ;
		socket_node->is_retranslate =1;
		socket_node->resend_times = 0;
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

//���ݷ���������㳬ʱֵ
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

