/* file : udt_socket.c 
 * implete udp data transfer
 * neil 
 * 2005-11-17
 */

//#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
//#include "nd_common/nd_alloc.h"

int post_udt_datagram(nd_udt_node* socket_node, void *data, size_t len) ;
int post_udt_datagramex(nd_udt_node* socket_node, struct ndudt_pocket *packet, size_t len) ;
int flush_send_window(nd_udt_node* socket_node);			//发送 窗口中的数据

int udt_send_fin(nd_udt_node *socket_node) ;


extern nd_udt_node * udt_icmp_connect(nd_udt_node *socket_node, const char *host, short port, struct nd_proxy_info *proxy);
/* connect to remote host 
 * realize 3-times handshake
 */
nd_udt_node* udt_connect(nd_udt_node *socket_node,const char *host, short port, struct nd_proxy_info *proxy)
{
	ENTER_FUNC()
	if(socket_node->sock_type == SOCK_RAW) {
		if (!udt_icmp_connect(socket_node,host, port, proxy)) {
			nd_showerror() ;
			LEAVE_FUNC();
			return 0;
		}
	}
	else {
		if (-1==nd_udp_connect((nd_handle)socket_node,host, port, proxy)) {
			nd_showerror() ;
			LEAVE_FUNC();
			return 0;
		}
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


int udt_connector_close(nd_udt_node* socket_node)
{

	int i ,ret;
	udt_pocketbuf pocket ;
	if (!(socket_node->status & NETSTAT_SENDCLOSE)) {
		int  fin_ok = 0;
		//not send close 
		for(i=0; i<MAX_ATTEMPT_SEND; i++) {
			if(-1== udt_send_fin(socket_node) ) {
				return -1 ;
			}
REWAITDATA:
			ret = _wait_data(socket_node, &pocket,socket_node->retrans_timeout<<1) ;
			if (ret ==0 ) {
				socket_node->myerrno = NDERR_TIMEOUT ;
				return -1 ;
				//continue ;
			}
			else if(-1==ret)
				return -1 ;

			if(_udt_packet_handler(socket_node,(struct ndudt_pocket *) &pocket, ret) >= 0) {
				if(NETSTAT_SENDCLOSE & socket_node->status){
					fin_ok =1 ;
					break ;
				}
				else {
					goto REWAITDATA ;
				}
				
			}			
		}
		if(!fin_ok)
			return -1;
	}
	
	if(NETSTAT_RECVCLOSE & socket_node->status) {
		return 0;
	}
	//waiting for close by peer
	for(i=0; i<MAX_ATTEMPT_SEND; i++) {
		if((ret=_wait_data(socket_node, &pocket,RETRANSLATE_TIME)) >0) {
			_udt_packet_handler(socket_node,(struct ndudt_pocket *) &pocket, ret) ;
			if(NETSTAT_RECVCLOSE & socket_node->status||NETSTAT_RESET & socket_node->status)
				return 0 ;
		}
	}
	return -1;
}

//关闭服务器端的socket
void _close_listend_socket(nd_udt_node* socket_node)
{
	nd_assert(socket_node->is_accept) ;
	if((NETSTAT_ESTABLISHED & socket_node->status) && 
		!(socket_node->status & NETSTAT_SENDCLOSE) ){
		udt_send_fin(socket_node);		
	}
}


//发送fin包,并且等待确认
//实现改进3次握手
//发送fin并且等待确认
int udt_close(nd_udt_node* socket_node,int force)
{
	ENTER_FUNC()
	if (NETSTAT_ESTABLISHED == socket_node->status) {
		while (flush_send_window(socket_node) > 0){} 
	}
	if(socket_node->is_accept){
		_close_listend_socket(socket_node) ;
		LEAVE_FUNC();
		return 0;
	}
	else {
		if (force)
			udt_reset(socket_node,1) ;
		else 
			udt_connector_close(socket_node) ;
		if (socket_node->fd) {
			nd_socket_close(socket_node->fd);
			socket_node->fd = 0;
		}
	}
	LEAVE_FUNC();
	return 0;
		
}


void send_reset_packet(nd_udt_node* socket_node) 
{
	struct ndudt_pocket pocket;
	init_udt_pocket(&pocket);
	SET_RESET(&pocket);
	write_pocket_to_socket(socket_node, &pocket, ndt_header_size(&pocket));	
}

void udt_reset(nd_udt_node* socket_node, int issend_reset) 
{
	if (!(socket_node->status & NETSTAT_RESET) ){
		socket_node->status |= NETSTAT_RESET ;
		send_reset_packet(socket_node) ;
	}
}


//把封包通过socket发送出去, len 是包的总长度
int write_pocket_to_socket(nd_udt_node *socket_node,struct ndudt_pocket *pocket, size_t len)
{
	int ret ;
	
	POCKET_SESSIONID(pocket) = socket_node->session_id ;

	if(NDUDT_DGRAM!=pocket->header.udt_type){
		pocket->window_len = (u_16) send_window(socket_node) ;
	}
	
	udt_host2net(pocket) ;
	
	nd_assert(socket_node->sock_write);
	ret = socket_node->sock_write((nd_handle)socket_node, (void*)pocket, len) ;
	
	udt_net2host(pocket) ;
	if(ret==len) {
		socket_node->last_active = nd_time() ;
	}
	return ret ;
}


int udt_send(nd_udt_node* socket_node,void *data, int len )
{
	int ret ;
	
	if(socket_node->status != NETSTAT_ESTABLISHED) {
		socket_node->myerrno = NDERR_INVALID_HANDLE ;
		return -1 ;
	}

	if(len < 1 || !data || len >= ndlbuf_capacity(&socket_node->send_buffer)) {
		socket_node->myerrno = NDERR_INVALID_INPUT;
		return -1 ;
	}
	socket_node->myerrno = NDERR_SUCCESS;
	
	nd_send_lock((nd_netui_handle)socket_node) ;
	ret = ndlbuf_write(&socket_node->send_buffer, data, len, EBUF_SPECIFIED) ;
	nd_send_unlock((nd_netui_handle)socket_node) ;
	
	return ret ;
	
}


//发送udt 窗口中的数据
int flush_send_window(nd_udt_node* socket_node)
{
	int ret =0,isack=0;
	size_t header_len=0, len=0 ;
	char *send_addr;	
	struct ndudt_pocket *packet_hdr, packet_backup;
	
	if( (socket_node->status & NETSTAT_FINSEND+NETSTAT_RECVCLOSE) || socket_node->is_retranslate) {
		return 0;
	}

	send_addr = send_window_start(socket_node, &len) ;
	if(!send_addr || len ==0 ){	
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
		memcpy(&packet_backup,packet_hdr, header_len) ;
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

	memcpy(packet_hdr, &packet_backup,header_len) ;
	return ret;


}

int retranslate_data(nd_udt_node* socket_node)
{

	int ret =0,sendlen;
	int len ;// = (int)(socket_node->send_sequence - socket_node->acknowledged_seq) ;
	size_t header_len ;
	u_32 sendseq = socket_node->acknowledged_seq ;
	char *send_addr;	
	struct ndudt_pocket *packet_hdr, packet_backup;

	//nd_assert(len <=ndlbuf_datalen(&socket_node->send_buffer));
	send_addr = ndlbuf_data(&socket_node->send_buffer);

	if (0==socket_node->is_retranslate) {
		len = (int)(socket_node->send_sequence - socket_node->acknowledged_seq) ;
		socket_node->retrans_seq = socket_node->acknowledged_seq ;
	}
	else {
		len = (int)(socket_node->retrans_seq - socket_node->acknowledged_seq) ;
	}

//RE_SEND:
	if (len <=0)	{
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
		/*
		ret += len ;
		sendseq += len ;
		send_addr += len ;
		len = (int)(socket_node->send_sequence - sendseq)  ;
		if(len > 0)
			goto RE_SEND ;
		*/
	}
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

	//interv = nd_time() - socket_node->last_resend_tm ;
	interv = socket_node->update_tm - socket_node->last_resend_tm ;
	if (interv < socket_node->retrans_timeout ) {
		return 0;
	}

	if (socket_node->is_retranslate) {
		if (socket_node->resend_times >= MAX_ATTEMPT_SEND)	{
			nd_object_seterror((nd_handle)socket_node,NDERR_WUOLD_BLOCK) ;
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
	size_t len=0;
//	char *send_addr ;
	ndtime_t now = nd_time() ;

	socket_node->update_tm = now ;
	
	nd_object_seterror((nd_handle)socket_node,NDERR_SUCCESS) ;	

	//check data need to be send 
	if (flush_send_window(socket_node) > 0) {
		while (flush_send_window(socket_node) > 0){} 
	}
	else {
		if(-1== udt_retranslate(socket_node) ){
			return -1 ;
		}
	}	

	if (get_socket_ack(socket_node)){
		//send ack packet
		udt_send_ack(socket_node) ;
		set_socket_ack(socket_node,0) ;
	}
	/*	
	if((now-socket_node->last_active) > ACTIVE_TIME){
		//time out send active pocket 
		struct ndudt_pocket alive_pocket;
		init_udt_pocket(&alive_pocket);
		set_pocket_ack(&alive_pocket, socket_node->received_sequence) ;
		SET_ALIVE(&alive_pocket);

		if(-1==write_pocket_to_socket(socket_node, &alive_pocket, ndt_header_size(&alive_pocket)) ) {
			nd_assert(0) ;
			return -1 ;
		}
	}
	*/
	/*
	//检测超时
	if((now - socket_node->last_recv) > (ACTIVE_TIME *10)) {
	//connect time out, need to be close 
	nd_object_seterror((nd_handle)socket_node,NDERR_TIMEOUT) ;
	nd_assert(0) ;
	return -1 ;
	}
	*/

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
	int ret ;
	struct ndudp_packet tmp,*p_send ;

	if(len < 1 ||  len >(ND_UDP_PACKET_SIZE-UDP_PACKET_HEAD_SIZE)){
		nd_object_seterror((nd_handle)socket_node, NDERR_INVALID_INPUT) ;
		return -1 ;
	}

	p_send = (struct ndudp_packet*) data;
	--p_send ;
	tmp = *p_send ;

	init_udt_header(&p_send->header) ;
	p_send->header.udt_type=NDUDT_DGRAM ;

	len += UDP_PACKET_HEAD_SIZE ;
	ret = write_pocket_to_socket(socket_node,(struct ndudt_pocket*)p_send, len) ;
	
	*p_send = tmp ;
	return ret ;
}


int udt_connector_send(nd_udt_node* socket_addr, nd_packhdr_t *msg_buf, int flag)
{
	int ret ;
	socket_addr->myerrno = NDERR_SUCCESS ;
	nd_assert(msg_buf) ;
	if(ESF_POST & flag) {
		
		return udt_sendto(socket_addr, msg_buf, nd_pack_len(msg_buf)) ;			
	}
	else {		//用udt发送可靠消息
		ret = udt_send(socket_addr, msg_buf, nd_pack_len(msg_buf)) ;
		if(ret <= 0) {
			return ret ;
		}
	}
	return ret ;
}

//Wait and read data from nd_udt_node
//return 0 no data in coming, -1 error check error code ,else return lenght of in come data
int _wait_data(nd_udt_node *socket_node,udt_pocketbuf* buf,ndtime_t outval) 
{
	ENTER_FUNC()
	int ret = socket_node->sock_read((nd_handle)socket_node,(char*)buf, sizeof(udt_pocketbuf),outval) ;
	if(ret <= 0) {
		if (socket_node->myerrno ==NDERR_BADPACKET) {
			socket_node->myerrno = NDERR_WUOLD_BLOCK ;
			ret = 0 ;
		}
		LEAVE_FUNC();
		return ret ;
	}

	socket_node->update_tm = socket_node->last_recv ;
	
	//数据检测
	if(!socket_node->check_entry(socket_node,&buf->pocket,ret, &socket_node->last_read) ) {
		socket_node->myerrno = NDERR_WUOLD_BLOCK ;
		LEAVE_FUNC();
		return -1;
	} 

	nd_assert(PROTOCOL_UDT==POCKET_PROTOCOL(&buf->pocket));
	if(socket_node->session_id ) {
		if(socket_node->session_id!=POCKET_SESSIONID(&buf->pocket)){
			socket_node->myerrno = NDERR_BADPACKET ;
			LEAVE_FUNC();
			return -1 ;
		}
	}
	else {
		if(NDUDT_SYN!=POCKET_TYPE(&buf->pocket)) {
			socket_node->myerrno = NDERR_BADPACKET ;
			LEAVE_FUNC();
			return -1 ;
		}
	}
	LEAVE_FUNC();
	return ret ;
}

int check_income_udp_packet(nd_udt_node *node, struct ndudt_pocket *pocket, int len,SOCKADDR_IN *addr)
{
	if(node->remote_addr.sin_addr.s_addr==addr->sin_addr.s_addr &&
		node->remote_addr.sin_port == addr->sin_port) {
			return 1 ;
	}
	return 0;
}

void _udt_connector_init(nd_udt_node *socket_node)
{
	socket_node->type = NDHANDLE_UDPNODE ;
	socket_node->size = sizeof(nd_udt_node) ;

	socket_node->write_entry = (packet_write_entry )udt_connector_send;
	socket_node->sock_write = (socket_write_entry) nd_udp_send;
	socket_node->sock_read = (socket_read_entry)nd_udp_read;

	socket_node->data_entry = (data_in_entry) nd_dft_packet_handler ;
	socket_node->msg_entry = (net_msg_entry) nd_translate_message ;

	socket_node->protocol = PROTOCOL_UDT ;
	socket_node->sock_type = SOCK_DGRAM ;
	socket_node->protocol_entry = (udp_protocol_entry) _udt_packet_handler ;
	socket_node->update_entry = (net_update_entry) update_socket ;
	socket_node->get_pack_size = nd_net_getpack_size ;
	socket_node->status = NETSTAT_CLOSED;

	socket_node->check_entry = check_income_udp_packet ;
	socket_node->_rtt.average = RETRANSLATE_TIME ;				//记录样本往返时间
	socket_node->_rtt.deviation = 0;
	socket_node->retrans_timeout = RETRANSLATE_TIME*TIME_OUT_BETA ;			//超时重传时间和等待关闭时间(记录时间间隔不是绝对时间)
	init_crypt_key(&socket_node->crypt_key) ;		//加密解密密钥(对称密钥)

	INIT_LIST_HEAD(&socket_node->__release_cb_hdr) ;
}

void nd_udtnode_init(nd_udt_node *socket_node)
{	
	memset(socket_node,0 , sizeof(socket_node) );
	
	_udt_connector_init(socket_node) ;
	nd_net_connbuf_init((nd_netui_handle)socket_node) ;
}

void nd_udtnode_reset(nd_udt_node *socket_node)
{
	nd_udt_node tmp_node = *socket_node;
	nd_assert(socket_node) ;

	_nd_object_on_destroy((nd_handle)socket_node,0)  ;
	
	udt_reset(socket_node,0);	

	_udt_connector_init(socket_node) ;

#define SET_VAL(valname) socket_node->valname = tmp_node.valname 
	SET_VAL(close_entry) ;
	SET_VAL(msg_handle) ;
	SET_VAL(srv_root) ;
	SET_VAL(user_data) ;
	SET_VAL(write_entry) ;
	SET_VAL(sock_write) ;
	SET_VAL(data_entry) ;
	SET_VAL(msg_entry) ;
	SET_VAL(update_entry) ;
	SET_VAL(get_pack_size) ;
	SET_VAL(start_time) ;
	SET_VAL(disconn_timeout) ;
	SET_VAL(sock_read) ;
	SET_VAL(crypt_key) ;
#undef SET_VAL

	ndlbuf_reset(&(socket_node->recv_buffer)) ;
	ndlbuf_reset(&(socket_node->send_buffer)) ;

}

void _deinit_udt_socket(nd_udt_node *socket_node) 
{
	net_release_sendlock((nd_netui_handle)socket_node) ;

	socket_node->status = NETSTAT_CLOSED;

	ndlbuf_destroy(&(socket_node->recv_buffer)) ;
	ndlbuf_destroy(&(socket_node->send_buffer)) ;
	_nd_object_on_destroy((nd_handle)socket_node,0)  ;
	
}

