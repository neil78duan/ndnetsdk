/* file : udt_socket.c 
 * implete udp data transfer
 * neil 
 * 2005-11-17
 */

#include "nd_net/nd_netlib.h"

int post_udt_datagram(nd_udt_node* socket_node, void *data, size_t len) ;
int post_udt_datagramex(nd_udt_node* socket_node, struct ndudt_pocket *packet, size_t len) ;
int flush_send_window(nd_udt_node* socket_node);			//发送 窗口中的数据

int udt_send_fin(nd_udt_node *socket_node) ;


//extern nd_udt_node * udt_icmp_connect(nd_udt_node *socket_node, const char *host, short port, struct nd_proxy_info *proxy);
/* connect to remote host 
 * realize 3-times handshake
 */
nd_udt_node* udt_connect(nd_udt_node *socket_node,const char *host, short port, struct nd_proxy_info *proxy)
{
	ENTER_FUNC()
	
	if (-1 == nd_udp_connect((nd_handle)socket_node, host, port, proxy)) {
		nd_showerror();
		LEAVE_FUNC();
		return 0;
	}

	if(0== _udt_syn(socket_node) ){
		socket_node->start_time = nd_time() ;
		LEAVE_FUNC();
		return socket_node ; //syn success connect OK!
	}
	else {
		LEAVE_FUNC();
		return NULL;
	}
}


static int _udt_fin_send_handle(nd_udt_node* socket_node)
{

	int i ,ret;
	//udt_pocketbuf pocket ;
	if (!(socket_node->status & NETSTAT_SENDCLOSE)) {
		int  fin_ok = 0;
		//not send close 
		for(i=0; i<MAX_ATTEMPT_SEND; i++) {
			if(-1== udt_send_fin(socket_node) ) {
				udt_reset(socket_node, 0);
				return -1 ;
			}
			ret = socket_node->recv_data((nd_handle)socket_node, socket_node->retrans_timeout << 2);
			if (NETSTAT_SENDCLOSE & socket_node->status) {
				fin_ok = 1;
				break;
			}
			else if (ret ==0 ) {
				continue ;
			}
			else if (-1 == ret) {
				udt_reset(socket_node, 0);
				return -1;
			}		
		}

		if (!fin_ok) {
			udt_reset(socket_node, 0);
			nd_object_seterror((nd_handle)socket_node, NDERR_TIMEOUT);
			return -1;
		}
	}
	
	if(NETSTAT_RECVCLOSE & socket_node->status) {
		return 0;
	}

	for (i = 0; i < MAX_ATTEMPT_SEND; i++) {
		ret = socket_node->recv_data((nd_handle)socket_node, socket_node->retrans_timeout << 2);

		if (NETSTAT_RECVCLOSE & socket_node->status) {
			break;
		}

		else if (ret == 0) {
			continue;
		}
		else if (-1 == ret) {
			udt_reset(socket_node, 0);
			return -1;
		}
	}
	return 0;
}

// 
// int _connector_close(nd_udt_node* socket_node, int force)
// {
// 	ENTER_FUNC();
// 	
// 	LEAVE_FUNC();
// 	return 0;
// }


//发送fin包,并且等待确认
//实现改进3次握手
//发送fin并且等待确认
int udt_close(nd_udt_node* socket_node,int force)
{
	ENTER_FUNC();
	
	if (force)
		udt_reset(socket_node, 1);
	else {
		if (NETSTAT_ESTABLISHED == socket_node->status) {
			while (flush_send_window(socket_node) > 0) {}
		}
		_udt_fin_send_handle(socket_node);
	}
	nd_udp_close((nd_handle)socket_node, force);

	LEAVE_FUNC();
	return 0;
		
}


void udt_reset(nd_udt_node* socket_node, int issend_reset) 
{
	if (issend_reset) {
		struct ndudt_pocket pocket;
		init_udt_pocket(&pocket);
		SET_RESET(&pocket);
		write_pocket_to_socket(socket_node, &pocket, ndt_header_size(&pocket));
	}
	socket_node->status |= NETSTAT_FINSEND + NETSTAT_RESET;
	if (!socket_node->is_accept && socket_node->fd) {
		nd_udp_close((nd_handle)socket_node, 1);
	}

}

int read_packet_from_socket(nd_udt_node *socket_node, char *buf, size_t size, ndtime_t tmout)
{
	int len = nd_udp_read((struct nd_udp_node*)socket_node, buf, size, tmout);
	if (len <= 0) {
		return len;
	}

	if (!_test_checksum((struct ndudt_header*)buf, len)) {
		socket_node->myerrno = NDERR_BADPACKET;
		return 0;
	}
	udt_net2host((struct ndudt_pocket*)buf);
	return len;

}


//把封包通过socket发送出去, len 是包的总长度
int write_pocket_to_socket(nd_udt_node *socket_node,struct ndudt_pocket *pocket, size_t len)
{
	int ret ;
	
	pocket->local_port = socket_node->local_port;

	if(NDUDT_DGRAM!=pocket->header.udt_type){
		pocket->window_len = (u_16) send_window(socket_node) ;
	}
	
	udt_host2net(pocket) ;
	
	_set_checksum(&pocket->header, (int)len);

	nd_assert(socket_node->sys_sock_write);
	ret = nd_udp_send((nd_handle)socket_node, (void*)pocket, len) ;
	
	udt_net2host(pocket) ;
	if(ret==len) {
		socket_node->last_active = nd_time() ;
	}
	return ret ;
}


int udt_send(nd_udt_node* socket_node,void *data, int len )
{
	ENTER_FUNC();
	int ret ;
	
	if(socket_node->status < NETSTAT_ESTABLISHED || (socket_node->status &NETSTAT_FINSEND)) {
		socket_node->myerrno = NDERR_CLOSED;
		LEAVE_FUNC();
		return -1 ;
	}

	if(len < 1 || !data || len >= ndlbuf_capacity(&socket_node->send_buffer)) {
		socket_node->myerrno = NDERR_INVALID_INPUT;
		LEAVE_FUNC();
		return -1 ;
	}
	socket_node->myerrno = NDERR_SUCCESS;
	
	nd_send_lock((nd_netui_handle)socket_node) ;
	ret = ndlbuf_write(&socket_node->send_buffer, data, len, EBUF_SPECIFIED) ;
	nd_send_unlock((nd_netui_handle)socket_node) ;
	LEAVE_FUNC();
	return ret ;
	
}


//发送udt 窗口中的数据
int flush_send_window(nd_udt_node* socket_node)
{
	ENTER_FUNC();
	int ret =0,isack=0;
	size_t header_len=0, len=0 ;
	char *send_addr;	
	struct ndudt_pocket *packet_hdr, packet_backup;
	
	if( (socket_node->status & NETSTAT_SENDCLOSE) || socket_node->is_retranslate) {
		LEAVE_FUNC();
		return 0;
	}

	send_addr = send_window_start(socket_node, &len) ;
	if(!send_addr || len ==0 ){	
		LEAVE_FUNC();
		return 0;
	}

	//把报头放到数据前面,避免copy数据
	if(get_socket_ack(socket_node)) {
		isack =1 ;
		packet_hdr =(struct ndudt_pocket *) (((struct ndudt_pocket*)send_addr )-1);
		header_len = sizeof(struct ndudt_pocket) ;
		memcpy(&packet_backup,packet_hdr, header_len) ;
		init_udt_pocket(packet_hdr) ;

		set_pocket_ack(packet_hdr, socket_node->received_sequence) ;
		set_socket_ack(socket_node, 0) ;
	}
	else{
		isack =1 ;
		packet_hdr =(struct ndudt_pocket *) (((struct _ndudt_unack_packet*)send_addr )-1);
		header_len = sizeof(struct _ndudt_unack_packet) ;
		memcpy(&packet_backup,packet_hdr, sizeof(struct ndudt_pocket)) ;
		init_udt_pocket(packet_hdr) ;
	}

	/*发送序列号,当前发送到滑动窗口的位置*/
	packet_hdr->sequence = socket_node->send_sequence ;

	ret = write_pocket_to_socket(socket_node, packet_hdr, len+header_len) ;
	if(ret==(len+header_len) ){
		socket_node->last_resend_tm = socket_node->last_active  ;
		socket_node->send_sequence += (u_32) len ;
		socket_node->window_len -= len ;
		if((int)(socket_node->window_len) <0)
			socket_node->window_len = 0 ;
	}

	memcpy(packet_hdr, &packet_backup, sizeof(struct ndudt_pocket)) ;
	LEAVE_FUNC();
	return ret;


}

int retranslate_data(nd_udt_node* socket_node)
{
	ENTER_FUNC();
	int ret =0,sendlen;
	int len ;
	size_t header_len ;
	u_32 sendseq = socket_node->acknowledged_seq ;
	char *send_addr;	
	struct ndudt_pocket *packet_hdr, packet_backup;

	send_addr = ndlbuf_data(&socket_node->send_buffer);

	if (0==socket_node->is_retranslate) {
		len = (int)(socket_node->send_sequence - socket_node->acknowledged_seq) ;
		socket_node->retrans_seq = socket_node->acknowledged_seq ;
	}
	else {
		len = (int)(socket_node->retrans_seq - socket_node->acknowledged_seq) ;
	}

	if (len <=0)	{
		LEAVE_FUNC();
		return ret ;
	}

	len = min(len, NDUDT_FRAGMENT_LEN) ;

	//把报头放到数据前面,避免copy数据
	if(get_socket_ack(socket_node)) {
		packet_hdr =(struct ndudt_pocket *) (((struct ndudt_pocket*)send_addr )-1);
		header_len = sizeof(struct ndudt_pocket) ;
		memcpy(&packet_backup,packet_hdr, header_len) ;
		set_pocket_ack(packet_hdr, socket_node->received_sequence) ;
		set_socket_ack(socket_node, 0) ;
	}
	else{
		packet_hdr =(struct ndudt_pocket *) (((struct _ndudt_unack_packet*)send_addr )-1);
		header_len = sizeof(struct _ndudt_unack_packet) ;
		memcpy(&packet_backup,packet_hdr, header_len) ;
	}
	init_udt_pocket(packet_hdr) ;

	/*发送序列号,当前发送到滑动窗口的位置*/
	packet_hdr->sequence = sendseq ;

	sendlen = write_pocket_to_socket(socket_node, packet_hdr, len+header_len) ;
	memcpy(packet_hdr, &packet_backup,header_len) ;
	if(sendlen==(len+header_len) ){
		socket_node->last_resend_tm = socket_node->last_active  ;
		if((int)(socket_node->window_len) <0)
			socket_node->window_len = 0 ;

		if (0==socket_node->is_retranslate)
			socket_node->retrans_seq += len  ;
		ret = len ;
		
	}
	LEAVE_FUNC();
	return ret;
}

/* 检测是否超时,如果超时重传数据包
 * on error return -1 the connect need to be close
 */
int udt_retranslate(nd_udt_node* socket_node)
{
	int ret ;
	ndtime_t interv ;
	int unnotified = (int)(socket_node->send_sequence - socket_node->acknowledged_seq) ;
	if(unnotified<= 0)
		return 0 ;

	interv = socket_node->update_tm - socket_node->last_resend_tm ;
	if (interv < socket_node->retrans_timeout ) {
		return 0;
	}

	if (socket_node->is_retranslate) {
		if (socket_node->resend_times >= MAX_ATTEMPT_SEND)	{
			nd_object_seterror((nd_handle)socket_node, NDERR_WOULD_BLOCK);
			return -1;
		}
	}

	ret = retranslate_data(socket_node) ;
	if(ret > 0) {
		if (0==socket_node->is_retranslate) {
			socket_node->is_retranslate =1 ;
			socket_node->resend_times =1 ;
		}
		else {
			socket_node->resend_times++ ;
			socket_node->retrans_timeout = socket_node->retrans_timeout << 1;
		}
	}
	return ret ;
}

/*
 * update_socket() 
 * send data in background
 * retranslate, active ,ack etc..
 * return -1 error check error code
 * else return 1 
 */
int update_socket(nd_udt_node* socket_node)
{
	ENTER_FUNC();
	ndtime_t now = nd_time() ;
	socket_node->update_tm = now ;	
	nd_object_seterror((nd_handle)socket_node,NDERR_SUCCESS) ;	

	if (socket_node->status & NETSTAT_ESTABLISHED && !list_empty(&socket_node->pre_list)) {
		_update_prelist(socket_node);
	}
	//check data need to be send 
	if (flush_send_window(socket_node) > 0) {
		while (flush_send_window(socket_node) > 0){} 
	}
	else {
		if (-1 == udt_retranslate(socket_node)) {
			LEAVE_FUNC();
			return -1 ;
		}
	}	

	if (get_socket_ack(socket_node)){
		//send ack packet
		udt_send_ack(socket_node) ;
		set_socket_ack(socket_node,0) ;
	}
	LEAVE_FUNC();
	return 1 ;
}

//得到发送窗口起始地址
char *send_window_start(nd_udt_node* socket_node, size_t *sendlen)
{
	char *addr ;
	size_t data_len = ndlbuf_datalen(&socket_node->send_buffer) ;
	size_t un_ack = socket_node->send_sequence - socket_node->acknowledged_seq ;
	if(data_len <= un_ack){
		return NULL ;
	}
	addr = (char *) ndlbuf_data( & socket_node->send_buffer) ;
	*sendlen = data_len - un_ack ;
	*sendlen = min(*sendlen, NDUDT_FRAGMENT_LEN) ;
	*sendlen = min(socket_node->window_len,*sendlen) ;
	return (addr + un_ack ) ;
}


int udt_sendto(nd_udt_node* socket_node,void *data, int len ) 
{
	ENTER_FUNC();
	int ret ;
	struct ndudp_packet tmp,*p_send ;

	if(len < 1 ||  len >(ND_UDP_PACKET_SIZE-UDP_PACKET_HEAD_SIZE)){
		nd_object_seterror((nd_handle)socket_node, NDERR_INVALID_INPUT);
		LEAVE_FUNC();
		return -1 ;
	}

	p_send = (struct ndudp_packet*) data;
	--p_send ;
	tmp = *p_send ;

	init_udt_header(&p_send->header) ;
	p_send->header.udt_type=NDUDT_DGRAM ;

	len += UDP_PACKET_HEAD_SIZE ;
	ret = write_pocket_to_socket(socket_node,(struct ndudt_pocket*)p_send, len) ;
	
	*p_send = tmp;
	LEAVE_FUNC();
	return ret ;
}


int udt_connector_send(nd_udt_node* socket_addr, nd_packhdr_t *msg_buf, int flag)
{
	ENTER_FUNC();
	int ret ;
	struct packet_hdr hdr = *msg_buf;
	socket_addr->myerrno = NDERR_SUCCESS ;
	nd_assert(msg_buf) ;

	packet_ntoh(&hdr);

	if(ESF_POST & flag) {		
		ret = udt_sendto(socket_addr, msg_buf, nd_pack_len(&hdr)) ;			
	}
	else {
		ret = udt_send(socket_addr, msg_buf, nd_pack_len(&hdr)) ;
		if(ret >= 0) {
			flush_send_window(socket_addr);
		}
	}
	LEAVE_FUNC();
	return ret ;
}

//Wait and read data from nd_udt_node
//return 0 no data in coming, -1 error check error code ,else return lenght of in come data
int _wait_data(nd_udt_node *socket_node,udt_pocketbuf* buf,ndtime_t tmout)
{
	ENTER_FUNC();
	int ret = 0;
	NDUINT32 waittime = 0;
	if (tmout > 50) {
		waittime = 50;
	}
	else {
		waittime = tmout;
	}
	update_socket( socket_node);
RE_WAIT:

	ret = read_packet_from_socket(socket_node, (char*)buf, sizeof(udt_pocketbuf), waittime);
	if (ret <= 0) {
		if (ret == 0 || socket_node->myerrno == NDERR_WOULD_BLOCK) {
			NDUINT16 err = socket_node->myerrno;
			if (-1 == update_socket(socket_node)) {
				LEAVE_FUNC();
				return -1;
			}
			tmout -= waittime;
			socket_node->myerrno = err;
			if ((int)tmout > 0) {
				goto RE_WAIT;
			}
		}
		LEAVE_FUNC();
		return ret;
	}
	else {
		if (!socket_node->check_entry(socket_node, &buf->pocket, ret, &socket_node->last_read)) {
			socket_node->myerrno = NDERR_BADPACKET;
			LEAVE_FUNC();
			return -1;
		}

		nd_assert(PROTOCOL_UDT == POCKET_PROTOCOL(&buf->pocket));
		if (socket_node->local_port) {
			if (socket_node->local_port != buf->pocket.local_port) {
				socket_node->myerrno = NDERR_BADPACKET;
				LEAVE_FUNC();
				return -1;
			}
		}
		else {
			if (NDUDT_SYN != POCKET_TYPE(&buf->pocket)) {
				socket_node->myerrno = NDERR_BADPACKET;
				LEAVE_FUNC();
				return -1;
			}
		}
	}

	socket_node->update_tm = socket_node->last_recv ;
	
	LEAVE_FUNC();
	return ret ;
}

int check_income_udp_packet(nd_udt_node *node, struct ndudt_pocket *pocket, int len,SOCKADDR_IN *addr)
{
	return !nd_sockadd_in_cmp(&node->remote_addr, addr);
}

int udt_recv(nd_udt_node *socket_node, void *buffer, size_t size)
{
	ENTER_FUNC();
	int ret = 0;
	ndtime_t beginTm;

	UDT_RECV_USER_DATA(socket_node) = 0;

	if (!socket_node->is_accept) {
		udt_pocketbuf databuf;
		ret = _wait_data(socket_node, &databuf, 0);
		if (ret > 0) {
			if (-1== _udt_packet_handler(socket_node, &databuf.pocket, (size_t)ret)) {
				LEAVE_FUNC();
				return -1;
			}
		}
	}
	ret = 0;
	if (ndlbuf_datalen(&(socket_node->recv_buffer)) > 0) {
		ret = ndlbuf_read(&(socket_node->recv_buffer), buffer, size, 0);
	}
	LEAVE_FUNC();
	return ret;
}

static int udt_wait_packet(nd_udt_node *socket_node, ndtime_t tmout)
{
	ENTER_FUNC();
	int ret = 0;
	udt_pocketbuf databuf;
	ndtime_t beginTm;

	UDT_RECV_USER_DATA(socket_node) = 0;
RE_WAIT:

	beginTm = nd_time();
	ret = _wait_data(socket_node, &databuf, tmout);
	if (ret <= 0) {
		LEAVE_FUNC();
		return ret;
	}
	else {
		int leftTime;
		ret = _udt_packet_handler(socket_node, &databuf.pocket, (size_t)ret);
		if (ret > 0) {
			size_t data_len = ndlbuf_datalen(&(socket_node->recv_buffer));
			nd_packhdr_t *msg_addr = (nd_packhdr_t *)ndlbuf_data(&(socket_node->recv_buffer));

			if (data_len >= socket_node->get_pack_size((nd_handle)socket_node, msg_addr)) {
				LEAVE_FUNC();
				return (int)data_len;
			}
		}
		else if (-1 == ret) {
			LEAVE_FUNC();
			return -1;
		}

		leftTime =(int)( tmout - (nd_time() - beginTm));
		if (leftTime > 0) {
			tmout = leftTime;
			goto RE_WAIT;
		}	

		LEAVE_FUNC();
		return 0;
	}
}

// pre list
static void _release_pre_list(nd_udt_node *socket_node)
{
	struct list_head *pos, *next;

	list_for_each_safe(pos, next, &socket_node->pre_list) {
		struct _udt_packet_node *pack_node = list_entry(pos, struct _udt_packet_node, list);
		list_del_init(&pack_node->list);
		free(pack_node);
	}
	INIT_LIST_HEAD(&socket_node->pre_list);
}

int _addto_pre_list(nd_udt_node *socket_node, struct ndudt_pocket *packet, int len)
{
	struct list_head *pos, *next;
	size_t size = sizeof(struct _udt_packet_node) + len;
	struct _udt_packet_node *pnode = (struct _udt_packet_node *) malloc(size);
	if (!pnode) {
		return -1;
	}
	
	INIT_LIST_HEAD(&pnode->list);
	pnode->recvTm = nd_time();
	pnode->size = len;
	memcpy(&pnode->pack, packet, len);

	list_for_each_safe(pos, next, &socket_node->pre_list) {
		struct _udt_packet_node *plist = list_entry(pos, struct _udt_packet_node, list);
		int tmval = (int)(pnode->recvTm - plist->recvTm );
		if (tmval < 0) {
			list_add(&pnode->list, pos);
			return 0;
		}
		else if (0 == tmval) {
			int seqIndex = pnode->pack.sequence - plist->pack.sequence;
			if (seqIndex < 0) {

				list_add(&pnode->list, pos);
			}
			else {
				list_add_tail(&pnode->list, pos);
			}
			return 0;
		}
	}

	list_add_tail(&pnode->list, &socket_node->pre_list);
	return 0;
}


static int _handle_pre_packet(nd_udt_node* socket_node, struct ndudt_pocket *pocket, size_t len)
{
	ENTER_FUNC();
	int data_len;
	char *data;
	nd_netbuf_t *recvbuf = &socket_node->recv_buffer;

	nd_assert(socket_node && pocket);

	data_len = (int)len - ndt_header_size(pocket);
	
	if (pocket->sequence != socket_node->received_sequence) {
		LEAVE_FUNC();
		return 1;
	}

	if (data_len > ndlbuf_free_capacity(&socket_node->recv_buffer)) {
		LEAVE_FUNC();
		return -1;
	}

	data = pocket_data(pocket);
	set_socket_ack(socket_node, 1);
	socket_node->received_sequence += data_len;

	data_len = ndlbuf_write(recvbuf, data, data_len, EBUF_ALL);
	UDT_RECV_USER_DATA(socket_node) = 1;

	LEAVE_FUNC();
	return 0;
}

int _tryto_fetch_prelist(nd_udt_node *socket_node)
{
	ENTER_FUNC();
	int ret = 0;
	struct list_head *pos, *next;
	list_for_each_safe(pos, next, &socket_node->pre_list) {
		struct _udt_packet_node *plist = list_entry(pos, struct _udt_packet_node, list);
		int ret = _handle_pre_packet(socket_node, &plist->pack, plist->size);
		if (-1 == ret) {
			break;
		}
		else if (0 == ret) {
			list_del_init(&plist->list);
			free(plist);
			++ret;
		}
		else {
			break;
		}
	}
	LEAVE_FUNC();
	return ret;
}

int _update_prelist(nd_udt_node *socket_node)
{
	ENTER_FUNC();
	int ret = 0;
	struct list_head *pos, *next;
	list_for_each_safe(pos, next, &socket_node->pre_list) {
		struct _udt_packet_node *plist = list_entry(pos, struct _udt_packet_node, list);
		int seq_val = (int)(plist->pack.sequence - socket_node->received_sequence);
		int tm_val = (int)(nd_time() - plist->recvTm);
		if (seq_val < 0 || tm_val > 60000) {
			list_del_init(&plist->list);
			free(plist);
			++ret;
		}
	}
	LEAVE_FUNC();
	return ret;
}

void _udt_connector_init(nd_udt_node *socket_node)
{
	socket_node->type = NDHANDLE_UDPNODE ;
	socket_node->size = sizeof(nd_udt_node) ;
	socket_node->local_port = 0;

	socket_node->sys_sock_read = (socket_sys_entry)udt_recv;
	socket_node->sys_sock_write = (socket_sys_entry)udt_connector_send;
	socket_node->wait_readable = (socket_wait_entry)_wait_read_msg;
	socket_node->wait_writable = (socket_wait_entry)_wait_write_msg;

	socket_node->sys_sock_close = (socket_close_entry)udt_close;
	socket_node->recv_data = (socket_wait_entry)udt_wait_packet;

	socket_node->packet_write = (packet_write_entry )udt_connector_send;
	//socket_node->sock_write = (socket_write_entry) nd_udp_send;
	//socket_node->sock_read = (socket_read_entry)nd_udp_read;

	socket_node->data_entry = (data_in_entry) nd_dft_packet_handler ;
	socket_node->msg_entry = (net_msg_entry) nd_translate_message ;

	socket_node->protocol = PROTOCOL_UDT ;
	socket_node->sock_type = SOCK_DGRAM ;
	//socket_node->udt_close_entry = udt_close;
	socket_node->update_entry = (net_update_entry) update_socket ;
	socket_node->get_pack_size = nd_net_getpack_size ;
	//socket_node->wait_entry = (wait_message_entry)udt_wait_packet;
	socket_node->status = NETSTAT_CLOSED;

	socket_node->msg_caller = socket_node;
	
	socket_node->check_entry = check_income_udp_packet ;
	socket_node->_rtt.average = RETRANSLATE_TIME ;	
	socket_node->_rtt.deviation = 0;
	socket_node->retrans_timeout = RETRANSLATE_TIME*TIME_OUT_BETA ;
	init_crypt_key(&socket_node->crypt_key) ;

	INIT_LIST_HEAD(&socket_node->__release_cb_hdr);
	INIT_LIST_HEAD(&socket_node->pre_list);
}

void nd_udtnode_init(nd_udt_node *socket_node)
{	
	memset(socket_node,0 , sizeof(*socket_node) );
	
	_udt_connector_init(socket_node) ;
	nd_net_connbuf_init((nd_netui_handle)socket_node) ;
}

void nd_udtnode_reset(nd_udt_node *socket_node)
{
	//nd_udt_node tmp_node = *socket_node;
	nd_assert(socket_node) ;

	_release_pre_list(socket_node);
	_nd_object_on_destroy((nd_handle)socket_node,0)  ;
	

	socket_node->sys_error = 0;
	socket_node->myerrno = 0;
	socket_node->last_recv = socket_node->last_push = nd_time();
	socket_node->local_port = 0;
	socket_node->level = 0;

	socket_node->status = NETSTAT_CLOSED;

	socket_node->need_ack = 0;
	socket_node->is_retranslate = 0;
	socket_node->user_data_in = 0;
	socket_node->resend_times = 0;
	socket_node->last_resend_tm = 0;
	socket_node->last_active = 0;
	socket_node->update_tm = 0;
	socket_node->send_sequence = 0;
	socket_node->acknowledged_seq = 0;
	socket_node->received_sequence = 0;
	socket_node->retrans_seq = 0;
	socket_node->window_len = 0;


	socket_node->_rtt.average = RETRANSLATE_TIME;
	socket_node->_rtt.deviation = 0;
	socket_node->retrans_timeout = RETRANSLATE_TIME * TIME_OUT_BETA;

	///////////////////////////
	ndlbuf_reset(&(socket_node->recv_buffer));	
	ndlbuf_reset(&(socket_node->send_buffer));	

}

void _deinit_udt_socket(nd_udt_node *socket_node) 
{
	net_release_sendlock((nd_netui_handle)socket_node) ;

	_release_pre_list(socket_node);
	socket_node->status = NETSTAT_CLOSED;

	ndlbuf_destroy(&(socket_node->recv_buffer)) ;
	ndlbuf_destroy(&(socket_node->send_buffer)) ;
	_nd_object_on_destroy((nd_handle)socket_node,0)  ;
	
}

