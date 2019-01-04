/* file connect.c
 * net connect by tcp or udp
 * this module realized connect by net message on tcp/udp
 * init connect socket, recv data ,send data,crypt data,parse data to message,close connect 
 *
 * version 1.0
 * neil 
 * all right reserved 2007
 */

//#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
//#include "nd_common/nd_alloc.h"

static int __wait_writablity = 30 ;

int nd_set_wait_writablity_time(int newtimeval)
{
	int ret = __wait_writablity;
	__wait_writablity = newtimeval ;
	return ret ;
}

int nd_get_wait_writablity_time()
{
	return __wait_writablity ;
}
int _socket_send(struct nd_tcp_node *node,void *data , size_t len)
{
	ENTER_FUNC()
	int ret ;
	ret = (int) send(node->fd, data, len, 0) ;
	if(ret > 0) {
		node->send_len += ret ; 
		node->last_push = nd_time() ;
	}
	else if(-1==ret)  {
		node->sys_error = nd_socket_last_error() ;
		if (node->sys_error == ESOCKETTIMEOUT) {
			node->myerrno = NDERR_WOULD_BLOCK;
		}
		else {
			node->myerrno = NDERR_IO ;
		}
	}
	LEAVE_FUNC();
	return ret ;
};

//send a completed package (using in ND protocol)
int socket_send_one_msg(struct nd_tcp_node *node,void *data , size_t len)
{
	ENTER_FUNC()
	int times = 0 ;
	int ret ,send_ok = 0;
	
RE_SEND:
	nd_assert(node);
	nd_assert(node->sock_write) ;
	if (times>3){
		LEAVE_FUNC();
		return 0 ;
	}
	times++ ;
	ret = node->sock_write((nd_handle)node, data,len) ;
	if(-1==ret ) {
		if(node->sys_error==ESOCKETTIMEOUT && node->is_session){
			int wait_ret = nd_socket_wait_writablity(node->fd,__wait_writablity);
			if( wait_ret > 0)
				goto RE_SEND ;
			else if(wait_ret==0) {
				LEAVE_FUNC();
				return 0 ;
			}
			else { 
				node->myerrno = NDERR_WRITE ;
				TCPNODE_SET_RESET(node);
				LEAVE_FUNC();
				return -1;
			}
		}
		else {
			node->myerrno = NDERR_IO ;
			TCPNODE_SET_RESET(node) ;
			LEAVE_FUNC();
			return -1 ;
		}
	}
	else if(ret>0 && ret < len) {
		int wait_ret = nd_socket_wait_writablity(node->fd,__wait_writablity);
		if(wait_ret < 0 ){
			node->sys_error = nd_socket_last_error() ;
			if (node->sys_error == ESOCKETTIMEOUT) {
				node->myerrno = NDERR_WOULD_BLOCK;
			}
			else {
				TCPNODE_SET_RESET(node) ;
				node->myerrno = NDERR_IO ;
			}
			LEAVE_FUNC();
			return -1;
		}
		else if(0==wait_ret) {
			LEAVE_FUNC();
			return 0 ;
		}

		data = (char*)data + ret ;
		len -= ret ;
		send_ok += ret ;

		node->last_push = nd_time() ;
		goto RE_SEND ;
	}
	else {
		send_ok += ret ;
		LEAVE_FUNC();
		return send_ok ;
	}
}
//connect remote host
int nd_tcpnode_connect(const char *host, int port, struct nd_tcp_node *node,struct nd_proxy_info *proxy)
{
	ENTER_FUNC()
	nd_assert(node ) ;
	nd_assert(host) ;

	node->sys_error = 0;
	node->last_push = nd_time();
	ndlbuf_reset(&(node->recv_buffer));		/* buffer store data recv from net */
	ndlbuf_reset(&(node->send_buffer));		/* buffer store data send from net */

	if(proxy && proxy->proxy_type != ND_PROXY_NOPROXY ) {
		node->fd = nd_proxy_connect(host, port, &(node->remote_addr), proxy,0)  ;
	}
	else {
		node->fd = nd_socket_tcp_connect(host, (short)port,&(node->remote_addr)) ;
	}

	if(node->fd<=0){
		node->myerrno = NDERR_OPENFILE;
		LEAVE_FUNC();
		return -1 ;
	}
	TCPNODE_SET_OK(node) ;
	nd_socket_nonblock(node->fd,1);
	_set_ndtcp_conn_dft_option(node->fd);	
	node->start_time = nd_time() ;
	if (node->remote_addr.sin_family == AF_INET6) {
		node->is_ipv6 = 1;
	}
	LEAVE_FUNC();
	return 0 ;
}

int nd_tcpnode_close(struct nd_tcp_node *node,int force)
{
	ENTER_FUNC()
	//nd_assert(0);
	nd_assert(node);
	node->status = ETS_DEAD;
	if(node->fd==0) {
		LEAVE_FUNC();
		return 0 ;
	}
	nd_socket_close(node->fd) ;
	node->fd = 0 ;

	LEAVE_FUNC();
	return 0 ;
}

//static int __tcpnode_send(struct nd_tcp_node *node, nd_packhdr_t *msg_buf,int flag)
static int __tcpnode_send(struct nd_tcp_node *node, void *msg_buf, size_t datalen, int flag)
{
	ENTER_FUNC()
	signed int ret =0;
	//size_t datalen = node->get_pack_size(( nd_handle )node, msg_buf) ; //(size_t)nd_pack_len( msg_buf) ;
	nd_assert(node) ;
	nd_assert(msg_buf) ;
	
	if(ndlbuf_datalen(&(node->send_buffer))>0) {
		size_t space_len = ndlbuf_free_capacity(&(node->send_buffer)) ;
		if(space_len > datalen) {
			ret = ndlbuf_write(&(node->send_buffer),(void*)msg_buf,datalen,EBUF_SPECIFIED) ;
		}
		else {
			if (flag & ESF_POST){
				LEAVE_FUNC();
				return 0 ;
			}
			if( _tcpnode_push_force(node) <=0 ){ //clear send buffer
				LEAVE_FUNC();
				//if (NDERR_WOULD_BLOCK == node->myerrno){
				//	return 0 ;
				//}
				return -1 ;
			}

			space_len = ndlbuf_free_capacity(&(node->send_buffer));
			if (space_len >= datalen) {
				ret = ndlbuf_write(&(node->send_buffer), (void*)msg_buf, datalen, EBUF_SPECIFIED);
				if (-1 == ret) {
					node->myerrno = NDERR_WOULD_BLOCK;
				}
			}
			else {
				if (ndlbuf_datalen(&(node->send_buffer)) == 0) {
					ret = node->sock_write((nd_handle)node, msg_buf, datalen);
				}
				else {
					LEAVE_FUNC();
					node->myerrno = NDERR_WOULD_BLOCK;
					return -1;
				}
			}
			
		}
	}
	else if(datalen>ALONE_SEND_SIZE) {
		//data is too big , need to send right now ,not buffer
		ret = node->sock_write((nd_handle)node,msg_buf,datalen) ;
		if(-1==ret ) {
			if(node->sys_error!=ESOCKETTIMEOUT){
				LEAVE_FUNC();
				return -1 ;
			}
			else {
				//send error , wirte buffer 
				if (flag & ESF_POST){
					LEAVE_FUNC();
					return 0 ;
				}
				ret = ndlbuf_write(&(node->send_buffer),(void*)msg_buf,datalen,EBUF_SPECIFIED) ;
			}
		}
		else if(ret==datalen) {
			LEAVE_FUNC();
			return (int)datalen ;
		}
		else {
			int wlen ;
			char *padd = (char*) msg_buf ;
			padd += ret ;
			wlen = ndlbuf_write(&(node->send_buffer),padd,datalen-ret,EBUF_SPECIFIED) ;
			if (wlen > 0) {
				ret = wlen + ret;
			}
		}
	}
	else {
		ret = ndlbuf_write(&(node->send_buffer),(void*)msg_buf,datalen,EBUF_SPECIFIED) ;
	}

	if (ret> 0 ) {
		size_t space_len = ndlbuf_free_capacity(&(node->send_buffer)) ;
		if ( flag & ESF_URGENCY || space_len > ALONE_SEND_SIZE){
			int push_len ;
			if (node->is_session) {
				push_len =nd_tcpnode_tryto_flush_sendbuf(node) ;
			}
			else {
				push_len = _tcpnode_push_sendbuf(node) ;
			}
			if (push_len == -1 && NDERR_WOULD_BLOCK != node->myerrno)	{
				LEAVE_FUNC();
				return -1;
			}
		}
	}
	
	LEAVE_FUNC();
	return ret ;
}
//send api of tcp-node (fot nd protocol)
int nd_tcpnode_send(struct nd_tcp_node *node, nd_packhdr_t *msg_buf,int flag)
{
	size_t datalen = node->get_pack_size((nd_handle)node, msg_buf); 

	if (node->is_session){
		int ret ;
		flag  &= ~ESF_URGENCY ; //can not use
		ret = __tcpnode_send(node, (void*)msg_buf,datalen, flag) ;
		if (-1==ret){
			node->myerrno = NDERR_WRITE ;
			TCPNODE_SET_RESET(node);
			//nd_logdebug("send to %s error would be close \n" AND nd_inet_ntoa(node->remote_addr.sin_addr,NULL)) ;
			//nd_assert("send error "==0) ;
		}
		return ret ;
	}
	else {
		return __tcpnode_send(node, (void*)msg_buf, datalen, flag) ;
	}
}

//send stream data (tcp data) 
int nd_tcpnode_stream_send(struct nd_tcp_node *node, void*data, size_t len, int flag)
{
	ENTER_FUNC()
	int sendlen = 0;
	int timeout_count = 0;
	int ret ;
	do {
		node->myerrno = NDERR_SUCCESS;
		ret = __tcpnode_send(node, (void*)data, len, flag);
		if (ret <= 0) {
			if (node->myerrno == NDERR_WOULD_BLOCK) {
				++timeout_count;
				if (timeout_count > 100) {
					nd_logerror("send error : %s\n", nd_error_desc(node->myerrno)) ;
					node->myerrno = NDERR_TIMEOUT;
					LEAVE_FUNC();
					return -1;
				}
				
				if (-1 == nd_socket_wait_writablity(node->fd, 100)) {
					node->myerrno = NDERR_WRITE;
					nd_logerror("send error : %s\n", nd_error_desc(node->myerrno)) ;
					LEAVE_FUNC();
					return -1;
				}
			}
			
		}
		else if (ret == (int)len) {
			LEAVE_FUNC();
			return (int)(sendlen + len);
		}
		else if (ret < (int)len) {
			timeout_count = 0;
			sendlen += ret;
			len -= ret;
			data = (void*)((char*)data + ret);
			if (node->myerrno == NDERR_WOULD_BLOCK) {
				nd_socket_wait_writablity(node->fd, 100) ;
			}
		}
	} while (len > 0);
	
	LEAVE_FUNC();
	return sendlen;
}

int nd_tcpnode_read(struct nd_tcp_node *node)
{
	ENTER_FUNC()
	int read_len ;
	char *addr = ndlbuf_tail(&(node->recv_buffer));
	size_t space_len = ndlbuf_freespace(&(node->recv_buffer)) ;
	TCPNODE_READ_AGAIN(node) = 0;
	if(space_len<=0){
		int datalen =(int) ndlbuf_datalen(&(node->recv_buffer)) ;
		if (datalen> 0)	{
			LEAVE_FUNC();
			return datalen ;
		}
		ndlbuf_move_ahead(&(node->recv_buffer)) ;
		space_len = ndlbuf_freespace(&(node->recv_buffer)) ;
		addr = ndlbuf_tail(&(node->recv_buffer));
	}

	read_len = nd_socket_tcp_read(node->fd, addr,space_len);
	if(read_len > 0) {
		node->last_recv = nd_time();
		node->recv_len += read_len ;
		ndlbuf_add_data(&(node->recv_buffer),(size_t)read_len) ;
		if(read_len>=space_len)
			TCPNODE_READ_AGAIN(node) = 1;
	}
	else if(read_len==0) {
		node->myerrno = NDERR_CLOSED ;
		read_len = -1;
	}
	else {
		node->sys_error = nd_socket_last_error() ;
		if(node->sys_error==ESOCKETTIMEOUT) {
			node->myerrno = NDERR_WOULD_BLOCK;
			read_len = 0;
		}
		else
			node->myerrno = NDERR_IO ;
	}
	if(read_len==-1) {
		LEAVE_FUNC();
		return -1 ;
	}
	else{ 
		 read_len = (int)ndlbuf_datalen(&(node->recv_buffer)) ;
		 LEAVE_FUNC();
		 return read_len;
	}
}

/* wait a tcp-node income data 
* return the data length received from the @node 
* return -1 on error ,need to be close,check error code
* RETURN 0 time out, NOT read data 
*/
int tcpnode_wait_msg(struct nd_tcp_node *node, ndtime_t tmout)
{
	ENTER_FUNC()
	int ret,read_len;
	if(tmout) {
		fd_set rfds;
		struct timeval tmvel ;

		FD_ZERO(&rfds) ;
		FD_SET(node->fd,&rfds) ;

		if(-1==(int)tmout){
			ret = select (node->fd + 1, &rfds, NULL,  NULL, NULL) ;
		}
		else {
			tmvel.tv_sec = tmout/1000 ; 
			tmvel.tv_usec = (tmout%1000) * 1000;
			ret = select (node->fd + 1,  &rfds, NULL, NULL, &tmvel) ;
		}
		if(ret==-1) {
			node->sys_error = nd_socket_last_error() ;
			if (node->sys_error == ESOCKETTIMEOUT) {
				node->myerrno = NDERR_WOULD_BLOCK;
			}
			else {
				node->myerrno = NDERR_IO ;
			}
			LEAVE_FUNC();
			return -1 ;
		}
		else if(ret==0){
			node->myerrno = NDERR_WOULD_BLOCK;
			LEAVE_FUNC();
			return 0;
		}
		if(!FD_ISSET(node->fd, &rfds)){
			node->myerrno = NDERR_IO ;
			LEAVE_FUNC();
			return -1;
		}
	}

	read_len = nd_tcpnode_read(node) ;

	if(read_len<=0) {
		LEAVE_FUNC();
		return read_len ;
	}
	else {		
		int used_len = 0 ;
		size_t data_len;
		//struct ndnet_msg *msg_addr ;
		nd_packhdr_t *msg_addr ;

		data_len = ndlbuf_datalen(&(node->recv_buffer) );
		if(data_len < ND_PACKET_HDR_SIZE){
			ndlbuf_move_ahead(&(node->recv_buffer));
			LEAVE_FUNC();
			return 0 ;
		}

		msg_addr = (nd_packhdr_t *)ndlbuf_data(&(node->recv_buffer)) ;
		//used_len = nd_pack_len(msg_addr) ;
		used_len = (int) node->get_pack_size((nd_handle)node, msg_addr) ;

		if(used_len >= ND_PACKET_SIZE) {
			node->myerrno = NDERR_BADPACKET ;
			LEAVE_FUNC();
			return -1 ;
		}
		if(used_len<=data_len){
			LEAVE_FUNC();
			return used_len ;
		}
	}
	LEAVE_FUNC();
	return 0;
}

int nd_tcpnode_tryto_flush_sendbuf(struct nd_tcp_node *conn_node) 
{
	ENTER_FUNC()
	if(ndlbuf_datalen(&(conn_node->send_buffer)) >=SENDBUF_PUSH ||
		(nd_time() - conn_node->last_push) >= SENDBUF_TMVAL) {
		int ret = 0 ;
		nd_send_lock((nd_netui_handle)conn_node) ;
		ret = _tcpnode_push_sendbuf(conn_node) ;
		nd_send_unlock((nd_netui_handle)conn_node) ;
		LEAVE_FUNC();
		return ret ;
	}
	LEAVE_FUNC();
	return 0;
}

int _tcpnode_push_force(struct nd_tcp_node *conn_node)
{
	ENTER_FUNC()
	signed int ret = 0;

	nd_netbuf_t *pbuf = &(conn_node->send_buffer);
	size_t data_len = ndlbuf_datalen(pbuf);

	if (data_len == 0 || !check_connect_valid(conn_node)) {
		LEAVE_FUNC();
		return 0;
	}
RESEND:
	conn_node->myerrno = NDERR_SUCCESS;
	ret = (signed int)_socket_send(conn_node, ndlbuf_data(pbuf), data_len);
	if (ret <= 0) {
		if (conn_node->myerrno && conn_node->myerrno != NDERR_WOULD_BLOCK) {
			LEAVE_FUNC();
			return -1;
		}

	}
	else {
		nd_assert(ret <= data_len);
		ndlbuf_sub_data(pbuf, (size_t)ret);
	}
	data_len = ndlbuf_datalen(pbuf); 
	if (data_len > 0) {
		if (1 == nd_socket_wait_writablity(conn_node->fd, 1000)) {
			goto RESEND;
		}
	}
	LEAVE_FUNC();
	return ret;
}

int _tcpnode_push_sendbuf(struct nd_tcp_node *conn_node) 
{
	ENTER_FUNC()
	signed int ret = 0;
	
	nd_netbuf_t *pbuf = &(conn_node->send_buffer) ;
	size_t data_len = ndlbuf_datalen(pbuf) ;
	
	if(data_len==0 || !check_connect_valid(conn_node)){
		LEAVE_FUNC();
		return 0;
	}
	ret = (signed int)_socket_send(conn_node,ndlbuf_data(pbuf),data_len) ;
	if(ret>0) {
		nd_assert(ret<= data_len) ;
		ndlbuf_sub_data(pbuf,(size_t)ret) ;
	}
	LEAVE_FUNC();
	return ret ;
}

int _tcp_node_update(struct nd_tcp_node *node)
{
	ENTER_FUNC()
	ndtime_t now = nd_time() ;
	int ret = 0;

#if 1
	int alive_timeout = node->disconn_timeout >> 1;
	if (!node->fd || nd_object_check_error((nd_handle)node) || !TCPNODE_CHECK_OK(node)) {
		LEAVE_FUNC();
		return -1;
	}

	alive_timeout = alive_timeout? alive_timeout:ND_ALIVE_TIMEOUT;

	if (nd_tcpnode_flush_sendbuf((nd_netui_handle)node) == 0) {		
		if ((now - node->last_push) > alive_timeout){
			nd_sysresv_pack_t alive;
			nd_make_alive_pack(&alive);
			packet_hton(&alive);
			ret = nd_tcpnode_send(node, &alive.hdr, ESF_URGENCY);
		}
	}
	TCPNODE_TRY_CALLBACK_WRITE(node);
	
#else 
	if(0==nd_send_trytolock((nd_netui_handle)node)) {
		nd_netbuf_t *pbuf = &(node->send_buffer) ;
		size_t data_len = ndlbuf_datalen(pbuf) ;
		
		int alive_timeout = node->disconn_timeout >> 1 ;
		alive_timeout = alive_timeout? alive_timeout:ND_ALIVE_TIMEOUT;

		if(data_len > 0) {
			ret = (signed int)_socket_send(node, ndlbuf_data(pbuf),data_len) ;
			if(ret>0) {
				nd_assert(ret<= data_len) ;
				ndlbuf_sub_data(pbuf,(size_t)ret) ;
			}
			else if(ret < 0) {
				if (node->myerrno == NDERR_WOULD_BLOCK){
					ret = 0;
				}
			}
		}
		
		if ((now - node->last_push) > alive_timeout ){
			nd_sysresv_pack_t alive ;
			nd_make_alive_pack(&alive) ;
            packet_hton(&alive);
			//nd_packhdr_t pack_hdr = {ND_USERMSG_HDRLEN,NDNETMSG_VERSION,0,0,0} ;
			//pack_hdr.ndsys_msg = 1 ;
			ret =nd_tcpnode_send(node, &alive.hdr,ESF_URGENCY) ;
		}
		
		TCPNODE_TRY_CALLBACK_WRITE(node) ;
		nd_send_unlock((nd_netui_handle)node) ;
	}
	
#endif
	if (now - node->last_recv > node->disconn_timeout) {
		node->myerrno = NDERR_TIMEOUT ;
		LEAVE_FUNC();
		return -1 ;
	}
	LEAVE_FUNC();
	return ret ;
}

void nd_tcpnode_reset(struct nd_tcp_node *conn_node) 
{
	ENTER_FUNC()
	if(conn_node->fd){
		nd_socket_close(conn_node->fd) ;
		conn_node->fd = 0 ;
	}
	
	_nd_object_on_destroy((nd_handle)conn_node,0)  ;
	
	conn_node->sys_error = 0 ;
	conn_node->last_push = nd_time();
	ndlbuf_reset(&(conn_node->recv_buffer)) ;		/* buffer store data recv from net */
	ndlbuf_reset(&(conn_node->send_buffer)) ;		/* buffer store data send from net */
	LEAVE_FUNC();
}

void _tcp_connector_init(struct nd_tcp_node *conn_node)
{
	ENTER_FUNC()
	nd_assert(conn_node) ;
	conn_node->size = sizeof(struct nd_tcp_node) ;
	conn_node->write_entry =(packet_write_entry ) nd_tcpnode_send ;
	conn_node->sock_write = (socket_write_entry) _socket_send ;
	conn_node->sock_read = NULL;
	conn_node->update_entry = (net_update_entry)_tcp_node_update ;
	conn_node->data_entry = (data_in_entry) nd_dft_packet_handler ;
	conn_node->msg_entry = (net_msg_entry) nd_translate_message ;
	conn_node->get_pack_size = nd_net_getpack_size ;
	conn_node->sock_type = SOCK_STREAM ;
	conn_node->status = ETS_DEAD;				/*socket state in game 0 not read 1 ready*/
	conn_node->start_time =nd_time() ;		
	conn_node->last_push = nd_time() ;
	conn_node->disconn_timeout = ND_DFT_DISSCONN_TIMEOUT ;	
	init_crypt_key(&conn_node->crypt_key);
	conn_node->type = NDHANDLE_TCPNODE ;
	
	INIT_LIST_HEAD(&conn_node->__release_cb_hdr) ;
	
	LEAVE_FUNC();
}
void nd_tcpnode_init(struct nd_tcp_node *conn_node) 
{
	ENTER_FUNC()
	memset(conn_node, 0, sizeof(*conn_node)) ;

	_tcp_connector_init(conn_node);
	conn_node->type = NDHANDLE_TCPNODE ;
	ndlbuf_init(&(conn_node->recv_buffer), ND_NETBUF_SIZE) ;
	ndlbuf_init(&(conn_node->send_buffer), ND_NETBUF_SIZE) ;

	LEAVE_FUNC();
}

void nd_tcpnode_deinit(struct nd_tcp_node *conn_node) 
{
	net_release_sendlock((nd_netui_handle)conn_node)  ;
	conn_node->status = ETS_DEAD ;
	ndlbuf_destroy(&(conn_node->recv_buffer)) ;
	ndlbuf_destroy(&(conn_node->send_buffer)) ;
	
	_nd_object_on_destroy((nd_handle)conn_node,0)  ;
	
}

#define ND_DEFAULT_TCP_WINDOWS_BUF (1024 * 128)
/* set ndnet connector default options */
void _set_ndtcp_conn_dft_option(ndsocket_t sock_fd)
{
	size_t sock_bufsize = ND_DEFAULT_TCP_WINDOWS_BUF  ;
	socklen_t output_len ;
	
	struct linger lin ;
	lin.l_onoff = 1 ;		//delay close 2 seconds 
	lin.l_linger = 2 ;

	//set receive buffer
//	output_len = sizeof(sock_bufsize) ;
//	if(-1==setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (sock_opval_t)&sock_bufsize,output_len ) ) {
//		//nd_logerror("set socket receive buffer error %s\n" AND nd_last_error() ) ;
//	}
	
	/*
	new_bufsize = 0 ;
	output_len = sizeof(new_bufsize) ;
	getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (sock_opval_t)&new_bufsize, &output_len) ;
	if (new_bufsize != sock_bufsize){
		nd_logerror("set socket receive buffer error setval =%d new val=%d\n" AND sock_bufsize AND  new_bufsize ) ;
	}*/

	//set send buffer
	sock_bufsize = ND_DEFAULT_TCP_WINDOWS_BUF ;	
	output_len = sizeof(sock_bufsize) ;
	
//	if(-1==setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (sock_opval_t)&sock_bufsize, sizeof(sock_bufsize)) ){
//		//nd_logerror("set socket send buffer error %s\n" AND nd_last_error() ) ;
//	}
	
	/*
	new_bufsize = 0 ;
	output_len = sizeof(new_bufsize) ;
	getsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (sock_opval_t)&new_bufsize, &output_len) ;
	if (new_bufsize != sock_bufsize){
		nd_logerror("set socket send buffer error setval =%d new val=%d\n" AND sock_bufsize AND  new_bufsize ) ;
	}*/
	
	//set delay close
	setsockopt(sock_fd, SOL_SOCKET, SO_LINGER, (sock_opval_t)&lin, sizeof(lin)) ;		
}


/* set ndnet connector default options */
void _set_ndtcp_session_dft_option(ndsocket_t sock_fd)
{
	int sock_bufsize = ND_DEFAULT_TCP_WINDOWS_BUF  ;
	int output_len ;
	
	//set receive buffer
	output_len = sizeof(sock_bufsize) ;
	if(-1==setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (sock_opval_t)&sock_bufsize,output_len ) ) {
		nd_logerror("set socket receive buffer error %s\n" AND nd_last_error() ) ;
	}
	
	//set send buffer
	sock_bufsize = ND_DEFAULT_TCP_WINDOWS_BUF ;	
	output_len = sizeof(sock_bufsize) ;	
	if(-1==setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (sock_opval_t)&sock_bufsize, sizeof(sock_bufsize)) ){
		nd_logerror("set socket send buffer error %s\n" AND nd_last_error() ) ;
	}
	
}
