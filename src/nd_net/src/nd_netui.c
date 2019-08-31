/* file nd_netui.c
 * implement net user interface
 * 
 *
 * neil duan 
 * all right reserved 2008
 */

/*
 * net session/connector api
 */
#define ND_IMPLEMENT_HANDLE
typedef struct netui_info *nd_handle;

//#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
//#include "nd_common/nd_alloc.h"

static int _crypt_unit_len ;		//crypt unit length
static nd_netcrypt __net_encrypt, __net_decrypt ;
//static int nd_net_message_version_error(nd_netui_handle node) ;

/* connector tick  */
int nd_connector_update(nd_netui_handle net_handle,ndtime_t timeout) 
{
	ENTER_FUNC()
	int ret =0;
	int read_len ;
	nd_assert(net_handle ) ;

	if(!nd_connector_valid(net_handle) || nd_tryto_clear_err(net_handle)) {
		LEAVE_FUNC();
		return -1;
	}

RE_READ:

	//read_len = socket_node->recv_data((nd_handle)socket_node, timeout);
	read_len = nd_netobj_packet_recv(net_handle, timeout);
	if (-1 == read_len) {
		LEAVE_FUNC();
		return -1;
	}
	else if (0 == read_len) {
		if (nd_netobj_update(net_handle) == -1) {
			ret = -1;
		}
		LEAVE_FUNC();
		return ret;
	}
	else {
		ret += read_len;
		if (-1 == handle_recv_data(net_handle, NULL)) {
			LEAVE_FUNC();
			return -1;
		}
		if (TCPNODE_READ_AGAIN(net_handle)) {
			goto RE_READ;
		}
		nd_netobj_update(net_handle);
	}
		
	LEAVE_FUNC();
	return ret;	
}

//data handler
int handle_recv_data(nd_netui_handle node, nd_handle h_listen)
{
	ENTER_FUNC()
	int ret =0;

	nd_assert(node) ;
	nd_assert(node->data_entry) ;

	if (nd_tryto_clear_err(node))	{
		LEAVE_FUNC();
		return -1;
	}
	
	if(ndlbuf_datalen(&(node->recv_buffer) ) == 0){
		ndlbuf_reset(&(node->recv_buffer));
		LEAVE_FUNC();
		return 0 ;
	}
	//////////////////////////////////////////////////////////////////////////	
	if (node->user_def_data_hook){	
		int data_len = (int)ndlbuf_datalen(&(node->recv_buffer));
		ret = node->data_entry(node, ndlbuf_data(&(node->recv_buffer)), data_len, node->srv_root) ;
		if(ret > 0 ){
			ndlbuf_sub_data(&(node->recv_buffer),ret) ;
		}		
	}
	else {
		int read_len = 0;
		
#if defined(__ND_IOS__) || defined(__ND_ANDROID__)
		nd_packetbuf_t pack_buf ;
#else 
		static __ndthread  nd_packetbuf_t pack_buf ;
#endif
		
		nd_hdr_init(&pack_buf.hdr) ;

		//get received data
		while((read_len = nd_net_fetch_msg(node, &pack_buf.hdr) ) > 0){
			if (read_len != pack_buf.hdr.length){
				nd_assert(0);
				return -1;
			}
			//read_len = _packet_handler(node, &pack_buf.hdr, h_listen) ;
			read_len = node->data_entry(node, (void*)&pack_buf, read_len, h_listen);
			if(-1==read_len) {
				ret = -1 ;
				break ;
			}
			ret += read_len;
			if (NDERR_USER_BREAK == nd_object_lasterror(node)){
				nd_object_seterror(node,NDERR_SUCCESS) ;
				break ;
			}
			nd_hdr_init(&pack_buf.hdr);
		}
		if (-1==read_len) {
			ret = -1;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	
	LEAVE_FUNC();
	return ret ;

}

//packet handler function entry 
// translate message to message handler 
int _packet_handler(nd_netui_handle node,nd_packhdr_t *msg, nd_handle h_listen)
{
	ENTER_FUNC()
	int ret = nd_pack_len(msg) ;
	nd_assert(node) ;
	nd_assert(node->msg_entry);

	if (msg->ndsys_msg){
		if(-1==nd_net_sysmsg_hander(node,(nd_sysresv_pack_t *)msg))
			ret = -1;
	}
	else {
		ret = node->msg_entry(node, msg,h_listen) ; 
	}
	
	LEAVE_FUNC();
	return ret;
}


//default net message handle , parse data to ND_PROTOCOL
int nd_dft_packet_handler(nd_netui_handle node,void *data , size_t data_len , nd_handle h_listen)
{
	if (!data || data_len==0){
		return 0;
	}
	return _packet_handler(node, (nd_packhdr_t*)data, h_listen);

}

// send packet by tcp
static int _tcp_connector_send(struct nd_tcp_node* socket_addr, nd_packhdr_t *msg_buf, int flag) 
{
	ENTER_FUNC()
	int ret ;
	if(!TCPNODE_CHECK_OK(socket_addr)) {
		socket_addr->myerrno = NDERR_BADSOCKET ;
		LEAVE_FUNC();
		return -1 ;
	}
	socket_addr->myerrno = NDERR_SUCCESS ;
	nd_send_lock((nd_netui_handle)socket_addr) ;	
	ret = nd_tcpnode_send( socket_addr,	msg_buf, flag) ;
	nd_send_unlock((nd_netui_handle)socket_addr) ;

	LEAVE_FUNC();
	return ret ;
}

int nd_connector_send(nd_netui_handle net_handle, nd_packhdr_t *msg_buf, int flag) 
{
	ENTER_FUNC()
	int ret ;
	nd_assert(net_handle) ;
	nd_assert(msg_buf) ;
	nd_assert(net_handle->packet_write) ;

	if(!nd_connector_valid(net_handle)|| nd_tryto_clear_err(net_handle)) {
		LEAVE_FUNC();
		return -1;
	}

	if (msg_buf->ndsys_msg == 0 && net_handle->save_send_stream) {
		if ((flag & ESF_ENCRYPT)) {
			msg_buf->encrypt = 1 ;
			nd_netobj_send_stream_save(net_handle, msg_buf, (int)nd_pack_len(msg_buf));
			msg_buf->encrypt = 0 ;
		}
		else {
			nd_netobj_send_stream_save(net_handle, msg_buf, (int)nd_pack_len(msg_buf));
		}
	}

    msg_buf->version = NDNETMSG_VERSION ;
	if((flag & ESF_ENCRYPT) && !msg_buf->encrypt) {
		nd_packetbuf_t crypt_buf ;
		memcpy(&crypt_buf, msg_buf, nd_pack_len(msg_buf)) ;
		nd_packet_encrypt(net_handle, &crypt_buf) ;
		nd_atomic_inc(&net_handle->send_pack_times) ;
        
        packet_hton(&crypt_buf) ;
		ret =net_handle->packet_write(net_handle,&crypt_buf.hdr, flag) ;
	}
	else {
		nd_atomic_inc(&net_handle->send_pack_times) ;
        
        packet_hton(msg_buf) ;
		ret =net_handle->packet_write(net_handle,msg_buf, flag) ;
        packet_ntoh(msg_buf) ;
	}
    
#ifdef ND_TRACE_MESSAGE
	if (ret > 0 && msg_buf->ndsys_msg==0) {
		int maxId =(int) ND_USERMSG_MAXID(msg_buf), minId = (int)ND_USERMSG_MINID(msg_buf), msgLen =(int) ND_USERMSG_LEN(msg_buf);
		
		if (net_handle->is_log_send || nd_message_is_log(net_handle, maxId, minId)) {
			nd_logmsg("TRACE Message SEND (%d,%d) data-lenght=%d SUCCESS!!\n", maxId,minId, msgLen);
		}

	}
#endif
	
	LEAVE_FUNC();
	return ret ;
}


int nd_connector_send_stream(nd_handle net_handle, void* data, size_t len, int flag)
{
	ENTER_FUNC()
	int ret;
	nd_assert(net_handle);
	nd_assert(data);
	nd_assert(net_handle->packet_write);

	if (!nd_connector_valid(net_handle) || nd_tryto_clear_err(net_handle)) {
		LEAVE_FUNC();
		return -1;
	}

	ret = nd_tcpnode_stream_send((struct nd_tcp_node*)net_handle, data, len, flag);

	LEAVE_FUNC();
	return ret;
}


int nd_connector_raw_write(nd_handle net_handle , void *data, size_t size) 
{
	int ret ;
	ENTER_FUNC() 
	nd_assert(net_handle->sys_sock_write) ;
		
	ret = net_handle->sys_sock_write(net_handle , data,  size);
	LEAVE_FUNC() ;
	return ret ;
}

/*
 * reconnect to server , not change connector status 
 */
int nd_reconnect(nd_netui_handle net_handle, ndip_t ip, int port, struct nd_proxy_info *proxy)
{
	ENTER_FUNC() 
	int ret = 0 ;
	char ip_text[64] ;
	ndtime_t starttime = net_handle->start_time ;
	nd_connector_close(net_handle, 0) ;


	ret = nd_connector_open( net_handle,ND_INET_NTOA(ip,ip_text), port, proxy) ;
	if (ret == 0){
		net_handle->start_time = starttime;
		nd_object_seterror(net_handle,NDERR_SUCCESS);
	}
	LEAVE_FUNC();
	return ret ;
}

/*
 * 
 */
int nd_reconnectex(nd_netui_handle net_handle, const char *host, int port, struct nd_proxy_info *proxy)
{
	ENTER_FUNC() 

	int ret = 0 ;
	ndtime_t starttime = net_handle->start_time ;
	nd_connector_close(net_handle, 0) ;


	ret = nd_connector_open( net_handle,host, port, proxy) ;	
	if (ret == 0){
		net_handle->start_time = starttime;
		nd_object_seterror(net_handle,NDERR_SUCCESS);
	}
	LEAVE_FUNC();
	return ret ;

// 	nd_assert(net_handle ) ;
// 	nd_connector_reset(net_handle) ;
// 	return nd_connector_open( net_handle,host, port, proxy) ;	
}


int nd_connector_open(nd_netui_handle net_handle,const char *host, int port,struct nd_proxy_info *proxy)
{	
	ENTER_FUNC()
	int ret =0;

	nd_connector_reset(net_handle);

	if (net_handle->fd){
	}
// 	ndlbuf_reset(&(net_handle->recv_buffer));		/* buffer store data recv from net */
// 	ndlbuf_reset(&(net_handle->send_buffer));		/* buffer store data send from net */
// 
// 	net_handle->last_push = net_handle->last_recv = nd_time();
// 	net_handle->sys_error = 0;
// 	net_handle->myerrno = NDERR_SUCCESS;
// 	nd_connect_level_set(net_handle, 0);

	if(net_handle->type==NDHANDLE_TCPNODE){
		struct nd_tcp_node* socket_addr=(struct nd_tcp_node*)net_handle ;
		ret = nd_tcpnode_connect(host, port,socket_addr, proxy ) ;
		if(-1==ret  ) {
			socket_addr->myerrno = NDERR_OPENFILE ;
		}else {
			nd_connect_level_set(net_handle, EPL_CONNECT);
			net_init_sendlock(net_handle) ;
			socket_addr->packet_write = (packet_write_entry)_tcp_connector_send ;
		}
	}
	else if(net_handle->protocol==PROTOCOL_UDT) {
		nd_udt_node* socket_addr =(nd_udt_node*)net_handle ;
		nd_netui_handle n_handle =(nd_netui_handle) udt_connect(socket_addr,host, (short)port,proxy) ;

		if(n_handle){				
			net_init_sendlock(net_handle) ;		//client socket need send lock
		}
		else {
			ret = socket_addr->status == NETSTAT_ESTABLISHED;
			nd_connect_level_set(net_handle, EPL_CONNECT);
		}
	}
	else {
		ret = -1 ;
	}


	LEAVE_FUNC();
	return ret ;
}

int nd_connector_valid(nd_netui_handle net_handle)
{
	ENTER_FUNC()
	nd_assert(net_handle) ;
	if (!net_handle->fd || nd_object_check_error(net_handle)) {
		LEAVE_FUNC();
		return 0 ;
	}
	//nd_msgtable_destroy(net_handle) ;
	if(net_handle->type==NDHANDLE_TCPNODE){
		if(TCPNODE_CHECK_OK(net_handle)) {
			LEAVE_FUNC();
			return 1 ;
		}
	}
	else if(net_handle->type==NDHANDLE_UDPNODE) {
		if(net_handle->protocol==PROTOCOL_UDT) {
			LEAVE_FUNC();
			return (net_handle->status & NETSTAT_ESTABLISHED );
		}
		LEAVE_FUNC();
		return 1;
	}
	LEAVE_FUNC();
	return 0;
}

int nd_connector_close(nd_netui_handle net_handle, int flag)
{	
	ENTER_FUNC()
	//struct net_handle_header *h_header =(struct net_handle_header *)net_handle ;
	nd_assert(net_handle) ;
	//nd_msgtable_destroy(net_handle) ;
	nd_connect_level_set(net_handle, EPL_NONE);
	if(net_handle->type==NDHANDLE_TCPNODE){

		if (((struct nd_tcp_node*)net_handle)->status != ETS_DEAD )	{
			if (net_handle->data_entry) {
				net_handle->data_entry(net_handle,NULL,0,0) ;
			}
			nd_tcpnode_close((struct nd_tcp_node*)net_handle,flag);
			net_release_sendlock(net_handle) ;
			_nd_object_on_destroy(net_handle, 1) ;
		}
		LEAVE_FUNC();
		return 0 ;
	}
	else if(net_handle->type==NDHANDLE_UDPNODE) {
		if(net_handle->protocol==PROTOCOL_UDT) {
			nd_udt_node *socket_addr = (nd_udt_node*)net_handle ;

			if(socket_addr->fd>0) {
				udt_close(socket_addr,1);
				net_release_sendlock(net_handle) ;		//client socket need send lock
			}
			
			_nd_object_on_destroy(net_handle, 1) ;
			LEAVE_FUNC();
			return 0;
		}
		else {
			if (net_handle->fd) {
				nd_socket_close(net_handle->fd);
				net_handle->fd = 0;
			}
		}

	}
	LEAVE_FUNC();
	return -1;
}

/* reset connector
 * reset connector ,but do not change message-table, crypt-info
 */
int nd_connector_reset(nd_handle net_handle) 
{
	ENTER_FUNC()
	nd_assert(net_handle) ;
	//nd_connector_close( net_handle, 0 ) ;

	if(net_handle->type==NDHANDLE_TCPNODE){
		nd_tcpnode_reset((struct nd_tcp_node *)net_handle) ;
	}
	else if(net_handle->type==NDHANDLE_UDPNODE){
		nd_udtnode_reset((nd_udt_node*)net_handle) ;
	}

	LEAVE_FUNC();
	return 0 ;

}

ndsocket_t nd_connector_fd(nd_handle net_handle)
{
	nd_assert(net_handle) ;
	return  ((struct nd_tcp_node*)net_handle)->fd ;
	
}

/* 
 */
int _connector_destroy(nd_handle net_handle, int force) 
{
	int ret = -1;
	ENTER_FUNC()
	nd_assert(net_handle) ;
	//nd_msgtable_destroy(net_handle) ;
	if(net_handle->type==NDHANDLE_TCPNODE){
		struct nd_tcp_node *socket_addr = (struct nd_tcp_node*)net_handle ;
		
		nd_tcpnode_close(socket_addr,force);
		nd_msgtable_destroy(net_handle,0) ;
		nd_tcpnode_deinit((struct nd_tcp_node*)net_handle) ;
		ret = 0 ;
	}
	else if(net_handle->type==NDHANDLE_UDPNODE) {
		if(net_handle->protocol==PROTOCOL_UDT) {
			nd_udt_node *socket_addr = (nd_udt_node*)net_handle ;

			if(socket_addr->fd>0) {
				udt_close(socket_addr,1);
				net_release_sendlock(net_handle) ;		//client socket need send lock
				_deinit_udt_socket(socket_addr) ;		//client socket need send lock
			}
		}
		else {
			if (net_handle->fd) {
				nd_socket_close(net_handle->fd);
				net_handle->fd = 0;
			}
		}

		nd_msgtable_destroy(net_handle,0) ;
		ret = 0;
		
	}
	LEAVE_FUNC();
	return ret;
}



/* wait message , if the message income, read to message buf , untill timeout
 * return 0 timeout
 * on error return -1 
 * else return data length
 */
int nd_connector_waitmsg(nd_netui_handle net_handle, nd_packetbuf_t *msgbuf, ndtime_t tmout)
{
	ENTER_FUNC();
	int ret = 0;
	ndtime_t waittime = 0;

	if(!nd_connector_valid(net_handle)|| nd_tryto_clear_err(net_handle)) {
		LEAVE_FUNC();
		return -1;
	}

	ret =nd_net_fetch_msg(net_handle, (nd_packhdr_t *)msgbuf) ;
	if(ret>0){
		LEAVE_FUNC();
		return ret;
	}

TCP_REWAIT:
	if (tmout > 1000) {
		waittime = 1000;
	}
	else {
		waittime = tmout;
	}

	ret = nd_netobj_packet_recv(net_handle, waittime);
	if (ret <= 0) {
		if (net_handle->myerrno == NDERR_WOULD_BLOCK) {
			if (nd_netobj_update((nd_handle)net_handle) == -1) {
				LEAVE_FUNC();
				return -1;
			}
			tmout -= waittime;
			if ((int)tmout > 0)
				goto TCP_REWAIT;
		}

		LEAVE_FUNC();
		return ret;
	}
	else {
		ret = nd_net_fetch_msg(net_handle, (nd_packhdr_t *)msgbuf);
	}
	
	LEAVE_FUNC();
	return ret ;

}

int nd_connector_handled_data(nd_netui_handle net_handle, size_t size) 
{
	
	struct nd_tcp_node *socket_addr = (struct nd_tcp_node*)net_handle ;

	nd_assert(net_handle) ;
	ndlbuf_sub_data(&(socket_addr->recv_buffer),size) ;
	return 0 ;
	
}

//
int nd_connector_raw_waitdata(nd_netui_handle net_handle, void *buf, size_t size, ndtime_t timeout) 
{
	ENTER_FUNC()
	int ret = 0;
	if(!nd_connector_valid(net_handle)|| nd_tryto_clear_err(net_handle)) {
		LEAVE_FUNC();
		return -1;
	}

	struct nd_tcp_node *socket_node = (struct nd_tcp_node*)net_handle;
	if (timeout) {
		//if (nd_socket_wait_read(socket_node->fd, timeout) <= 0) {
		//	socket_node->myerrno = NDERR_WOULD_BLOCK;
		if(nd_netobj_wait_readable(net_handle,timeout)<=0){
			LEAVE_FUNC();
			return -1;
		}

	}

	ret = socket_node->sys_sock_read(net_handle, buf, size);
	if (ret == 0) {
		LEAVE_FUNC();
		return 0;
	}
	if (ret == -1) {
		if (nd_last_errno() == ESOCKETTIMEOUT) {
			socket_node->myerrno = NDERR_WOULD_BLOCK;
		}
		else {
			socket_node->myerrno = NDERR_BADSOCKET;
		}
	}
	
	LEAVE_FUNC();
	return ret ;
}

void nd_net_set_crypt(nd_netcrypt encrypt_func, nd_netcrypt decrypt_func,int crypt_unit) 
{
	ENTER_FUNC() 
	__net_encrypt = encrypt_func ;
	__net_decrypt = decrypt_func ;
	_crypt_unit_len = crypt_unit ;
	LEAVE_FUNC();
}
//crypt packet , the data length would be modified 
int nd_packet_encrypt(nd_netui_handle net_handle, nd_packetbuf_t *msgbuf)
{
	nd_assert(net_handle) ;
	nd_assert(msgbuf);

	return nd_packet_encrypt_key(&(net_handle->crypt_key ), msgbuf) ;	
}

int nd_packet_encrypt_key(nd_cryptkey *pcrypt_key, nd_packetbuf_t *msgbuf)
{
	ENTER_FUNC() 
	int datalen ;
	
	nd_assert(msgbuf);
	
	datalen = (int) nd_pack_len(&msgbuf->hdr) - ND_PACKET_HDR_SIZE;	
	if(datalen<=0 || datalen> (ND_PACKET_DATA_SIZE - _crypt_unit_len)) {
		LEAVE_FUNC();
		return 0;
	}
	
	//crypt 
	if(__net_encrypt && is_valid_crypt(pcrypt_key)) {
		
		int new_len  ;
		
		new_len = __net_encrypt((unsigned char*)msgbuf->data, datalen, pcrypt_key->key) ;
		if(0==new_len) {
			LEAVE_FUNC();
			return 0 ;
		}
		
		msgbuf->hdr.encrypt = 1 ;
		if(new_len> datalen) {
			msgbuf->hdr.stuff =1 ;
			msgbuf->hdr.stuff_len = (new_len -datalen) ; 
			msgbuf->hdr.length += msgbuf->hdr.stuff_len ;
		}
		LEAVE_FUNC();
		return new_len ;
	}
	LEAVE_FUNC();
	return 0 ;
}

int nd_packet_decrypt(nd_netui_handle net_handle, nd_packetbuf_t *msgbuf)
{
	return nd_packet_decrypt_key(&(net_handle->crypt_key ), msgbuf) ; 
}

int nd_packet_decrypt_key(nd_cryptkey *pcrypt_key,nd_packetbuf_t *msgbuf)
{
	ENTER_FUNC()
	int datalen ;
	
	nd_assert(msgbuf);
	nd_assert(msgbuf->hdr.encrypt);
	
	datalen = (int) nd_pack_len(&msgbuf->hdr) - ND_PACKET_HDR_SIZE;	
	if(datalen<=0 || datalen> (ND_PACKET_DATA_SIZE - _crypt_unit_len)){
		LEAVE_FUNC();
		return 0;
	}
	//decrypt 
	if(__net_decrypt && is_valid_crypt(pcrypt_key)) {
		
		int new_len = __net_decrypt((unsigned char *)msgbuf->data, datalen, pcrypt_key->key) ;
		if(new_len <= 0 || new_len!=datalen) {
			//char buf[20] ;
			//SOCKADDR_IN *addr =& (net_handle->remote_addr );
			//nd_logdebug("[%s] send data error :unknow crypt data\n" AND nd_inet_ntoa( addr->sin_addr.s_addr, buf )) ;
			LEAVE_FUNC();
			return -1 ;
		}
		msgbuf->hdr.encrypt = 0 ;
		
		if(msgbuf->hdr.stuff) {
			LEAVE_FUNC();
			
			if (msgbuf->hdr.stuff_len!=msgbuf->data[datalen-1]) {
				return 0;
			}
			else if(nd_pack_len(&msgbuf->hdr) > msgbuf->data[datalen-1]) 
				return (nd_pack_len(&msgbuf->hdr) - msgbuf->data[datalen-1]) ;
			else 
				return 0;
		}
	}
	LEAVE_FUNC();
	return nd_pack_len(&msgbuf->hdr) ;
}

void nd_teaKeyToHostorder(tea_k *outkey, tea_k *key)
{
	
	outkey->k[0] = ntohl(key->k[0]) ;
	outkey->k[1] = ntohl(key->k[1]) ;
	outkey->k[2] = ntohl(key->k[2]) ;
	outkey->k[3] = ntohl(key->k[3]) ;
}

void nd_teaKeyToNetorder(tea_k *outkey, tea_k *key)
{
	outkey->k[0] = htonl(key->k[0]) ;
	outkey->k[1] = htonl(key->k[1]) ;
	outkey->k[2] = htonl(key->k[2]) ;
	outkey->k[3] = htonl(key->k[3]) ;
	
}

size_t nd_net_getpack_size(nd_handle  handle, void *data) 
{
    NDUINT16 len = ntohs (nd_pack_len((nd_packhdr_t*)data) ) ;
	return (size_t) len;
}



int nd_net_sysmsg_hander(nd_netui_handle node, nd_sysresv_pack_t *pack)
{
	NDUINT16 cksum = pack->checksum ;
	
	nd_packet_ntoh(&pack->hdr) ;
	
	pack->checksum = 0 ;
	pack->checksum = nd_checksum((NDUINT16 *)pack, pack->hdr.length ) ;
	if (cksum != pack->checksum){
		node->myerrno= NDERR_BADPACKET ; 
		nd_logerror("nd-system-msg error checkSum error send=%x, calc=%x\n", cksum,pack->checksum ) ;
		return -1 ;
	}
	//msg handle
	if (pack->msgid == ERSV_ALIVE_ID) {
		if (pack->hdr.length != sizeof(nd_sysresv_pack_t) || pack->hdr.stuff_len != 5 ){
			node->myerrno= NDERR_BADPACKET ; 
			nd_logerror("nd-system-msg input data error \n" ) ;
			return -1 ;
		}
		else {			
			nd_sysresv_pack_t alive ;
			nd_make_alive_pack(&alive) ;
			nd_connector_send(node, &alive.hdr, ESF_URGENCY) ;	
		}
		
	}
	else if(ERSV_VERSION_ERROR==pack->msgid) {
		nd_logerror("nd-system-msg message id error \n" ) ;
		node->myerrno = NDERR_VERSION ;
		return -1;
	}
	return 0;	
}


#undef  ND_IMPLEMENT_HANDLE
