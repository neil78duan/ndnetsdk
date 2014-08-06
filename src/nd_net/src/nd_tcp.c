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
	ret = send(node->fd, data, (int)len, 0) ;
	if(ret > 0) {
		node->send_len += ret ; 
		node->last_push = nd_time() ;
	}
	else if(-1==ret)  {
		node->sys_error = nd_socket_last_error() ;
		if (node->sys_error == ESOCKETTIMEOUT) {
			node->myerrno = NDERR_WUOLD_BLOCK ;
		}
		else {
			node->myerrno = NDERR_IO ;
		}
	}
	LEAVE_FUNC();
	return ret ;
};

//发送一个完整的消息
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
				node->myerrno = NDERR_WUOLD_BLOCK ;
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
int nd_tcpnode_connect(char *host, int port, struct nd_tcp_node *node,struct nd_proxy_info *proxy)
{
	ENTER_FUNC()
	nd_assert(node ) ;
	nd_assert(host) ;
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
	LEAVE_FUNC();
	return 0 ;
}

int nd_tcpnode_close(struct nd_tcp_node *node,int force)
{
	ENTER_FUNC()
	//nd_assert(0);
	nd_assert(node) ;
	if(node->fd==0) {
		LEAVE_FUNC();
		return 0 ;
	}
	nd_socket_close(node->fd) ;
	node->fd = 0 ;
	node->status = ETS_DEAD ;

	LEAVE_FUNC();
	return 0 ;
}

#if 1 
static int __tcpnode_send(struct nd_tcp_node *node, nd_packhdr_t *msg_buf,int flag)
{
	ENTER_FUNC()
	signed int ret =0;
	size_t datalen = node->get_pack_size(( nd_handle )node, msg_buf) ; //(size_t)nd_pack_len( msg_buf) ;
	nd_assert(node) ;
	nd_assert(msg_buf) ;
	nd_assert(datalen<ndlbuf_capacity(&node->send_buffer)) ;
	//packet_hton(msg_buf) ;//把网络消息的主机格式变成网络格式
	
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
			if(-1 == _tcpnode_push_sendbuf(node,1) ){ //清空缓冲
				LEAVE_FUNC();
				if (NDERR_WUOLD_BLOCK==node->myerrno ){
					return 0 ;
				}
				return -1 ;
			}
			ret = ndlbuf_write(&(node->send_buffer),(void*)msg_buf,datalen,EBUF_SPECIFIED) ;
		}
	}
	else if(datalen>ALONE_SEND_SIZE) {
		//数据需要单独发送,不适用缓冲too long
		ret = node->sock_write((nd_handle)node,msg_buf,datalen) ;
		if(-1==ret ) {
			if(node->sys_error!=ESOCKETTIMEOUT){
				LEAVE_FUNC();
				return -1 ;
			}
			else {
				//数据不能发送,尝试写入缓冲
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
			ret =(int) datalen ;
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
				push_len = _tcpnode_push_sendbuf(node,0) ;
			}
			if (push_len==-1 && NDERR_WUOLD_BLOCK!=node->myerrno )	{
				LEAVE_FUNC();
				return -1;
			}
		}
	}
	else if (ret==-1) 
		ret = 0;

	LEAVE_FUNC();
	return ret ;
}
//修改了发送方式,如果是session 发送失败将关闭连接
int nd_tcpnode_send(struct nd_tcp_node *node, nd_packhdr_t *msg_buf,int flag)
{
	if (node->is_session){
		int ret ;
		flag  &= ~ESF_URGENCY ; //can not use
		ret = __tcpnode_send(node, msg_buf, flag) ;
		if (-1==ret){
			node->myerrno = NDERR_WRITE ;
			TCPNODE_SET_RESET(node);
			//nd_logdebug("send to %s error would be close \n" AND nd_inet_ntoa(node->remote_addr.sin_addr,NULL)) ;
			//nd_assert("send error "==0) ;
		}
		return ret ;
	}
	else {
		return __tcpnode_send(node, msg_buf, flag) ; 
	}

}
#else 
/*
 *	send data throught nd_tcp_node
 */
int nd_tcpnode_send(struct nd_tcp_node *node, nd_packhdr_t *msg_buf,int flag)
{
	ENTER_FUNC() ;
	signed int ret ;
	//size_t datalen =(size_t)(NDNET_DATALEN(msg_buf) + ND_PACKET_HDR_SIZE) ;
	//nd_msgbuf_t
	size_t datalen =  node->get_pack_size(node, msg_buf) ;//(size_t)nd_pack_len( msg_buf) ;
	
	nd_assert(node) ;
	nd_assert(msg_buf) ;
	nd_assert(datalen<ndlbuf_capacity(&node->send_buffer)) ;

	//packet_hton(msg_buf) ;//把网络消息的主机格式变成网络格式
	
	if((ESF_WRITEBUF+ESF_POST)&flag ) {	//写入发送缓冲
		size_t space_len = ndlbuf_free_capacity(&(node->send_buffer)) ;
		if(space_len<datalen) {
			if(ESF_POST&flag) {
				node->myerrno = NDERR_LIMITED ;
				LEAVE_FUNC();
				return -1 ;
			}
			if(-1==nd_tcpnode_flush_sendbuf_force(node)) {
				nd_assert(0);
				LEAVE_FUNC();
				return -1 ;
			}
		}
		ret = ndlbuf_write(&(node->send_buffer), (void*)msg_buf,datalen,EBUF_SPECIFIED) ;
		LEAVE_FUNC();
		return ret ;
		
	}
	else if(ESF_URGENCY&flag) { //紧急发送
		if(-1 == nd_tcpnode_flush_sendbuf_force(node) ){
			LEAVE_FUNC();
			return -1 ;
		}
		ret = socket_send_one_msg(node,msg_buf,datalen) ;
		LEAVE_FUNC();
		return ret ;
	}
	else {			//正常发送
		/*
		 * 这里会对发送进行优化,如果数据少则放到缓冲中,如果缓冲中数据多,则一起发送出起
		 * 需要对缓冲上限进行限制,达到一定程度强制发送出去!
		 * 发送过程:
			1. 如果有数据在缓冲,并且数据能够放入缓则只是写入缓冲,
				写入缓冲以后尝试发送(主要是为了减少WRITE次数,并且保证缓冲区数据在前
			2. 缓冲没有数据,若达到发送下限则直接发送,否则写入缓冲
			3. 缓冲有数据,数据必须按先后顺序发送
		 */
		
		if(ndlbuf_datalen(&(node->send_buffer))>0) {
			//处理缓冲数据
			size_t space_len = ndlbuf_free_capacity(&(node->send_buffer)) ;
			if(space_len >= datalen) {
				ret = ndlbuf_write(&(node->send_buffer),(void*)msg_buf,datalen,EBUF_SPECIFIED) ;
				nd_assert(ret == datalen);
				nd_tcpnode_tryto_flush_sendbuf(node);
				LEAVE_FUNC();
				return datalen ;
			}
			else {
				if(-1 == nd_tcpnode_flush_sendbuf_force(node) ){ //清空缓冲
					LEAVE_FUNC();
					return -1 ;
				}
			}
		}
		//now send buffer is empty
		if(datalen>ALONE_SEND_SIZE) {
			//数据需要单独发送,不适用缓冲too long
			nd_assert(ndlbuf_datalen(&(node->send_buffer))==0) ;
			ret = node->sock_write((nd_handle)node,msg_buf,datalen) ;
			if(-1==ret ) {
				if(node->sys_error!=ESOCKETTIMEOUT){
					LEAVE_FUNC();
					return -1 ;
				}
				else {
					//数据不能发送,尝试写入缓冲
					ret = ndlbuf_write(&(node->send_buffer),(void*)msg_buf,datalen,EBUF_SPECIFIED) ;

					LEAVE_FUNC();
					return ret ;
				}
			}
			else if(ret==datalen) {
				LEAVE_FUNC();
				return datalen ;
			}
			else {
				int wlen ;
				char *padd = (char*) msg_buf ;
				padd += ret ;
				wlen = ndlbuf_write(&(node->send_buffer),padd,datalen-ret,EBUF_SPECIFIED) ;
				LEAVE_FUNC();
				return datalen ;
			}
		}
		else {
			//数据太少写入缓冲
			ret = ndlbuf_write(&(node->send_buffer),(void*)msg_buf,datalen,EBUF_SPECIFIED) ;
			LEAVE_FUNC();
			return ret ;
		}
	}	
}
#endif

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
			node->myerrno = NDERR_WUOLD_BLOCK ;
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

/*等待一个网络消息消息
*如果有网络消息到了则返回消息的长度
*超时,出错返回-1,need to be close,check error code
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
				node->myerrno = NDERR_WUOLD_BLOCK ;
			}
			else {
				node->myerrno = NDERR_IO ;
			}
			LEAVE_FUNC();
			return -1 ;
		}
		else if(ret==0){
			node->myerrno = NDERR_WUOLD_BLOCK ;
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
		ret = _tcpnode_push_sendbuf(conn_node,0) ;
		nd_send_unlock((nd_netui_handle)conn_node) ;
		LEAVE_FUNC();
		return ret ;
	}
	LEAVE_FUNC();
	return 0;
}

int _tcpnode_push_sendbuf(struct nd_tcp_node *conn_node,int force) 
{
	ENTER_FUNC()
	signed int ret = 0;
	
	nd_netbuf_t *pbuf = &(conn_node->send_buffer) ;
	size_t data_len = ndlbuf_datalen(pbuf) ;
	if(data_len==0 || !check_connect_valid(conn_node)){
		LEAVE_FUNC();
		return 0;
	}
	/*
	if(force && !conn_node->is_session)
		ret = (signed int)socket_send_one_msg(conn_node,ndlbuf_data(pbuf),data_len) ;
	else */	
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
	if(0==nd_send_trytolock((nd_netui_handle)node)) {
		nd_netbuf_t *pbuf = &(node->send_buffer) ;
		size_t data_len = ndlbuf_datalen(pbuf) ;
		
		int alive_timeout = node->disconn_timeout >> 2 ;
		alive_timeout = alive_timeout? alive_timeout:ND_ALIVE_TIMEOUT;

		if(data_len > 0) {
			ret = (signed int)_socket_send(node, ndlbuf_data(pbuf),data_len) ;
			if(ret>0) {
				nd_assert(ret<= data_len) ;
				ndlbuf_sub_data(pbuf,(size_t)ret) ;
			}
			else if(ret < 0) {
				if (node->myerrno == NDERR_WUOLD_BLOCK ){
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
		nd_send_unlock((nd_netui_handle)node) ;
	}
	
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

}

#define ND_DEFAULT_TCP_WINDOWS_BUF (1024 * 128)
/* set ndnet connector default options */
void _set_ndtcp_conn_dft_option(ndsocket_t sock_fd)
{
	int sock_bufsize = ND_DEFAULT_TCP_WINDOWS_BUF *2 , new_bufsize ;
	int output_len ;
	
	struct linger lin ;
	lin.l_onoff = 1 ;		//delay close 2 seconds 
	lin.l_linger = 2 ;

	//set receive buffer
	setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (sock_opval_t)&sock_bufsize, sizeof(sock_bufsize)) ;
	new_bufsize = 0 ;
	output_len = sizeof(new_bufsize) ;
	getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (sock_opval_t)&new_bufsize, (socklen_t*)&output_len) ;
	if (new_bufsize != sock_bufsize){
		nd_logerror("set socket receive buffer error setval =%d new val=%d\n" AND sock_bufsize AND  new_bufsize ) ;
	}

	//set send buffer
	sock_bufsize = ND_DEFAULT_TCP_WINDOWS_BUF * 2;
	setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (sock_opval_t)&sock_bufsize, sizeof(sock_bufsize)) ;
	new_bufsize = 0 ;
	output_len = sizeof(new_bufsize) ;
	getsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (sock_opval_t)&new_bufsize, (socklen_t*)&output_len) ;
	if (new_bufsize != sock_bufsize){
		nd_logerror("set socket send buffer error setval =%d new val=%d\n" AND sock_bufsize AND  new_bufsize ) ;
	}
	//set delay close
	setsockopt(sock_fd, SOL_SOCKET, SO_LINGER, (sock_opval_t)&lin, sizeof(lin)) ;		
}


/* set ndnet connector default options */
void _set_ndtcp_session_dft_option(ndsocket_t sock_fd)
{
	int sock_bufsize = ND_DEFAULT_TCP_WINDOWS_BUF , new_bufsize ;
	int output_len ;
	
	//set receive buffer
	setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (sock_opval_t)&sock_bufsize, sizeof(sock_bufsize)) ;
	new_bufsize = 0 ;
	output_len = sizeof(new_bufsize) ;
	getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (sock_opval_t)&new_bufsize, (socklen_t*)&output_len) ;
	if (new_bufsize != sock_bufsize){
		nd_logerror("set socket receive buffer error setval =%d new val=%d\n" AND sock_bufsize AND  new_bufsize ) ;
	}

	//set send buffer
	sock_bufsize = ND_DEFAULT_TCP_WINDOWS_BUF ;
	setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (sock_opval_t)&sock_bufsize, sizeof(sock_bufsize)) ;
	new_bufsize = 0 ;
	output_len = sizeof(new_bufsize) ;
	getsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (sock_opval_t)&new_bufsize, (socklen_t*)&output_len) ;
	if (new_bufsize != sock_bufsize){
		nd_logerror("set socket send buffer error setval =%d new val=%d\n" AND sock_bufsize AND  new_bufsize ) ;
	}
}

/* set socket attribute */
int _set_socket_addribute(ndsocket_t sock_fd)
{
#if 0
	int sock_bufsize = 0 , new_bufsize = ND_PACKET_SIZE *2 ;
	int output_len ;
	struct timeval timeout ;
	int ret ;
	int keeplive = 1 ;
	int value ;
	
	/* 设置接收和发送的缓冲BUF*/
	output_len= sizeof(sock_bufsize) ;
	ret = getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF,(void*)&sock_bufsize,(socklen_t*)&output_len) ;
	
	if(sock_bufsize < new_bufsize)
		setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (void*)&new_bufsize, sizeof(new_bufsize)) ;
	
	ret = getsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF,(void*)&sock_bufsize,(socklen_t*)&output_len) ;
	if(sock_bufsize < new_bufsize)
		setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (void*)&new_bufsize, sizeof(new_bufsize)) ;
	
//#ifdef __LINUX__	
	/*设置接收和发送的超时*/
	timeout.tv_sec = 1 ;			// one second 
	timeout.tv_usec = 0 ;		
	
	setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (void*)&timeout,sizeof(timeout)) ;
	setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, (void*)&timeout,sizeof(timeout)) ;

	
	/*设置保持活动选项*/
	ret = setsockopt(sock_fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keeplive,sizeof(keeplive)) ;
	//if(ret )
	//	PERROR("setsockopt") ;		//only for test
	
//#endif 
	/* 设置接收下限*/
	value = ND_PACKET_HDR_SIZE ;
	setsockopt(sock_fd, SOL_SOCKET, SO_RCVLOWAT, (void*)&value,sizeof(value)) ;
	
#endif
	return 0 ;
}
