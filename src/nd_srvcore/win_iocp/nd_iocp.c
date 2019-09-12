/* file : nd_iocp.c
 * implement windows iocp model using  BindIoCompletionCallback API
 *
 * version 
 * all right reserved by neil duan 
 * 2007-11
 */

#include "nd_srvcore/nd_srvlib.h"
#include "nd_iocp.h"
//#include "nd_common/nd_alloc.h"

#include <mswsock.h>
#pragma comment(lib, "Mswsock.lib")

static int iocp_session_handle_close(nd_handle h, int force) ;
BindIoCPCallback g_BindIOCPEntry = NULL;

#if 0

extern HANDLE g_hIOCompletionPort ;

int update_iocp_cm(struct listen_contex *ls_info);
//使用自定义线程池
int thread_iocp_entry(struct listen_contex *listen_info) ;
int nd_start_iocp_listen(struct listen_contex *listen_info)
{
	nd_log_screen(_NDT("START iocp listen\n"));
	return thread_iocp_entry(listen_info);
}
#else 
int nd_start_iocp_listen(struct listen_contex *listen_info)
{
	ENTER_FUNC()
	ndsocket_t listen_fd = get_listen_fd(listen_info) ;
	int num ;
	nd_handle context = nd_thsrv_gethandle(listen_info->listen_id) ;

	
	nd_log_screen(("START iocp listen\n"));
	if(!g_BindIOCPEntry){
		g_BindIOCPEntry = (BindIoCPCallback)
			GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "BindIoCompletionCallback");
	}
	
	if (!g_BindIOCPEntry((HANDLE)listen_fd, iocp_callback, 0))	{
		nd_showerror() ;
		LEAVE_FUNC();
		return -1 ;
	}
	
	num = pre_accept(&listen_info->tcp) ;
	if(0==num){
		LEAVE_FUNC();
		return -1 ;
	}
	while (!nd_thsrv_isexit(context)){
		int sleep = LISTEN_INTERVAL;
		int msgret = nd_thsrv_msghandler(context) ;
		if(-1== msgret)		//处理线程消息
			break ;
		else if(msgret)
			sleep = 0 ;	

		if(update_connector_hub((nd_listen_handle)listen_info) > 0)
			sleep = 0 ;
		
		update_iocp_cm(listen_info) ;

		if(sleep)
			nd_sleep(LISTEN_INTERVAL) ;

	}
	LEAVE_FUNC();
	return 0;
}

#endif
//把节点acceptEx好之后放到完成端口句柄中
int pre_accept(struct nd_srv_node *srv_node)
{
	int i ,ret = 0 ;
	int listen_nums = srv_node->conn_manager.free_num(&srv_node->conn_manager) ;
	

	for (i=0; i<listen_nums; i++){
		struct nd_session_iocp *iocp_map;
		struct nd_session_tcp *client =
			(struct nd_session_tcp *)srv_node->conn_manager.alloc (nd_srv_get_allocator(srv_node)) ;
		if(!client)
			break ;
		iocp_map = list_entry(client, struct nd_session_iocp,__client_map) ;
		if(srv_node->conn_manager.init ) {
			srv_node->conn_manager.init (client, (nd_handle)srv_node) ;
		}
		else {
			nd_iocp_node_init(iocp_map,(nd_handle)srv_node) ;
		}
		if(-1==nd_init_iocp_client_map(iocp_map, srv_node->fd) )
			break ;
		client->connect_node.srv_root = (nd_handle )srv_node ;
		++ret ;
	}
	return ret ;
}

int check_repre_accept(struct listen_contex *listen_info)
{
	return listen_info->tcp.conn_manager.free_num(&listen_info->tcp.conn_manager) ;
}

//初始化IOCP的节点函数,外部函数使用的
int nd_iocp_node_init(struct nd_session_iocp *iocp_map,nd_handle h_listen)
{
	struct listen_contex *lc = (struct listen_contex*) h_listen ;
	nd_session_tcp_init(&iocp_map->__client_map,h_listen) ;
	iocp_map->__client_map.connect_node.size = sizeof(struct nd_session_iocp) ;
	iocp_map->__client_map.connect_node.close_entry = (nd_close_callback ) iocp_session_handle_close ;

	iocp_map->__client_map.connect_node.msg_entry =lc->tcp.msg_entry ;
	
	iocp_map->__client_map.connect_node.packet_write =(packet_write_entry)nd_iocp_sendmsg;
	iocp_map->__client_map.connect_node.sys_sock_write = (socket_sys_entry)iocp_socket_write ;
	iocp_map->in_sending = 0 ;
	return 0 ;
}
int nd_init_iocp_client_map(struct nd_session_iocp *iocp_map,int listen_fd)
{
	ndsocket_t fd = INVALID_SOCKET;
	DWORD dwBytesRecvd;
	nd_netbuf_t  *buf_addr;
	
	nd_atomic_set(&TCPNODE_STATUS(iocp_map),ETS_DEAD) ;

	iocp_map->__client_map.connect_node.size = sizeof(struct nd_session_iocp) ;
	
	iocp_map->total_send = 0;		//write buf total
	iocp_map->send_len = 0;				// had been send length
	iocp_map->in_sending = 0 ;

	buf_addr = iocp_recv_buf(iocp_map) ;
	iocp_map->wsa_readbuf.buf =ndlbuf_addr(buf_addr) ;
	iocp_map->wsa_readbuf.len = 0;

	buf_addr = iocp_send_buf (iocp_map) ;
	iocp_map->wsa_writebuf.buf =ndlbuf_addr(buf_addr) ; 
	iocp_map->wsa_writebuf.len = 0 ;
	
	memset(&iocp_map->ov_read, 0, sizeof(iocp_map->ov_read));
	memset(&iocp_map->ov_write, 0, sizeof(iocp_map->ov_write));

	iocp_map->ov_read.client_addr = iocp_map ;
	iocp_map->ov_write.client_addr = iocp_map ;
	

	iocp_map->__wait_buffers =0 ;
	iocp_map->schedule_close_time = 0 ;

	fd =(ndsocket_t) WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(INVALID_SOCKET==fd){
		nd_showerror() ;
		return -1;
	}
	TCPNODE_FD(iocp_map) = fd ;
	
	dwBytesRecvd =(DWORD) ndlbuf_capacity(&(iocp_map->__client_map.connect_node.recv_buffer)) - (sizeof(struct sockaddr_in)+16)*2;

	if (!AcceptEx(listen_fd,
		fd,
		iocp_map->wsa_readbuf.buf,
		dwBytesRecvd ,
		sizeof(struct sockaddr_in) + 16,
		sizeof(struct sockaddr_in) + 16,
		&dwBytesRecvd,
		&iocp_map->ov_read.overlapped))
	{
		DWORD dwLastErr = WSAGetLastError() ;
		if(ERROR_IO_PENDING!=dwLastErr )  {
			SetLastError(dwLastErr) ;
			nd_showerror() ;
			nd_socket_close(fd) ;
			return -1 ;
		}
	}

	if(INVALID_SOCKET!=fd) {
		int val = 0 ;
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF,(char*)&val, sizeof(val)) ;
		val = 0 ;
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF,(char*)&val, sizeof(val)) ;
	}
	nd_atomic_set(&TCPNODE_STATUS(iocp_map),ETS_ACCEPT) ;	
	return 0 ;
}


void CALLBACK iocp_callback(DWORD dwErrorCode, DWORD dwByteCount,LPOVERLAPPED lpOverlapped)
{
	ENTER_FUNC()
	struct ND_OVERLAPPED_PLUS *ov = (struct ND_OVERLAPPED_PLUS *)lpOverlapped ;
	struct nd_session_iocp *pclient ;

	if(ov==NULL || nd_host_check_exit())  {
		LEAVE_FUNC();
		return ;
	}
	pclient  = ov->client_addr ;
	if (!pclient){
		return ;
	}

	if(dwErrorCode  ) {
		if ( nd_atomic_read(&TCPNODE_STATUS(pclient))==ETS_DEAD || 
			nd_atomic_read(&TCPNODE_STATUS(pclient)) == ETS_ACCEPT)	{
				LEAVE_FUNC();
				return ;
		}
	}

	if(0==dwByteCount ) {
		nd_object_seterror((nd_handle)pclient,NDERR_CLOSED) ;
		if(((nd_netui_handle)pclient)->session_id==0 || nd_atomic_read(&TCPNODE_STATUS(pclient)) == ETS_ACCEPT) {
			iocp_close_client(pclient,0) ;
			LEAVE_FUNC();
			return ;
		}
		else {
			struct iocp_lock_info lockinfo ;
			if(0==iocp_cm_lock(pclient, &lockinfo) ) {
				iocp_close_client(pclient,0) ;
				iocp_cm_unlock(&lockinfo) ;
			}	
		}

		LEAVE_FUNC();
		return ;
	}	
		
	if(ov== &pclient->ov_write ) {
		//write data success 
		struct iocp_lock_info lockinfo ;		
		if(0==iocp_cm_lock(pclient, &lockinfo) ) {
			if (nd_atomic_read(&TCPNODE_STATUS(pclient)) == ETS_CONNECTED) {
				nd_netbuf_t  *send_buf = iocp_send_buf(pclient) ;
				if (nd_atomic_read(&(pclient->in_sending)))	 {
					ndlbuf_sub_data(send_buf,(size_t)dwByteCount) ;
					pclient->send_len += dwByteCount ;
					pclient->__client_map.connect_node.send_len += dwByteCount ;
					nd_atomic_set(&pclient->in_sending,0) ;
				}
				if(0==iocp_unnotified_length(pclient) && ndlbuf_datalen(send_buf)>0)
					iocp_write(pclient) ;
			}			
			iocp_cm_unlock(&lockinfo) ;
		}

	}
	else if(ov==&pclient->ov_read) {		
		data_income(pclient, dwByteCount) ;
	}
	else {
		nd_assert(0) ;
	}

	LEAVE_FUNC();
}
int data_income(struct nd_session_iocp *pclient, DWORD dwRecvBytes )
{
	int read_len ;
	struct iocp_lock_info lockinfo ;

	ENTER_FUNC()
	if(	nd_atomic_read(&TCPNODE_STATUS(pclient))==ETS_ACCEPT) {
		if(-1==iocp_accept(pclient)){
			LEAVE_FUNC();
			return 0;
		}
	}

	nd_assert(	nd_atomic_read(&TCPNODE_STATUS(pclient))!=ETS_ACCEPT) ;

	if(0!=iocp_cm_lock(pclient,&lockinfo)) {
		LEAVE_FUNC();
		return -1 ;
	}

	if((int)dwRecvBytes>0){
		nd_netbuf_t  *read_buf = iocp_recv_buf(pclient);
		nd_assert(dwRecvBytes<=ndlbuf_freespace(read_buf));
		ndlbuf_add_data(read_buf,dwRecvBytes) ;
		pclient->__client_map.connect_node.recv_len += dwRecvBytes ;
		pclient->__client_map.connect_node.last_recv = nd_time() ;
	}


	if (-1 == handle_recv_data((nd_netui_handle)pclient, pclient->__client_map.connect_node.srv_root)) {
		iocp_close_client(pclient,0);
		iocp_cm_unlock(&lockinfo) ;
		LEAVE_FUNC();
		return 0;
	}
	
	//再次投递一个请求
	read_len = iocp_read(pclient) ;
	if (read_len==0){
		iocp_close_client(pclient,0);
		//return read_len;
	}
	iocp_cm_unlock(&lockinfo) ;
	LEAVE_FUNC();
	return read_len ;

}

//send data in iocp internal 
//return bytes of send
// return zero no data send or overlapped
// -1 error
int iocp_write(struct nd_session_iocp *iocp_map)
{
	ENTER_FUNC()
	int ret ;
	size_t data_len ;
	nd_netbuf_t  *send_buf;
	DWORD  dwBytesWritten = 0, dwFlags = 0;

	if(nd_atomic_read(&TCPNODE_STATUS(iocp_map))==ETS_DEAD || nd_atomic_read(&TCPNODE_STATUS(iocp_map))==ETS_TRYTO_CLOSE) {
		nd_object_seterror((nd_handle)iocp_map, NDERR_INVALID_HANDLE) ;
		LEAVE_FUNC();
		return -1 ;
	}
	if(0!=nd_testandset(&(iocp_map->in_sending) )) {
		return 0;
	}
	nd_object_seterror((nd_handle)iocp_map, NDERR_SUCCESS) ;
	
	send_buf = iocp_send_buf(iocp_map) ;
	data_len = ndlbuf_datalen(send_buf) ;
	
	nd_assert(data_len>0);

	iocp_map->wsa_writebuf.len =(ULONG) data_len ;
	iocp_map->wsa_writebuf.buf = ndlbuf_data(send_buf) ;
	bzero(&iocp_map->ov_write.overlapped,sizeof(iocp_map->ov_write.overlapped));
	iocp_map->total_send += data_len ;
	
	ret = WSASend(IOCP_MAP_FD(iocp_map),
		&iocp_map->wsa_writebuf,1,&dwBytesWritten,dwFlags,(WSAOVERLAPPED*)&iocp_map->ov_write,NULL);
	
	if(0==ret) {
		//iocp_map->total_send += dwBytesWritten;
		//nd_assert(dwBytesWritten==data_len) ;
		iocp_map->__client_map.connect_node.last_push = nd_time() ;
		LEAVE_FUNC();
		return (int)data_len ;
	}
	else {
		nd_atomic_set(&(iocp_map->in_sending),0) ;
		nd_assert(dwBytesWritten==0) ;
		iocp_map->__client_map.connect_node.sys_error = GetLastError() ;
		if(WSA_IO_PENDING==iocp_map->__client_map.connect_node.sys_error||
			EWOULDBLOCK==iocp_map->__client_map.connect_node.sys_error||
			WSAEWOULDBLOCK==iocp_map->__client_map.connect_node.sys_error) {

			LEAVE_FUNC();
			return 0 ;
		}
		nd_object_seterror((nd_handle)iocp_map, NDERR_WRITE) ; 
		//TCPNODE_SET_RESET(iocp_map);
		LEAVE_FUNC();
		return -1 ;
	}

}

/*iocp_read
 * 这里使用的异步重叠接受数据,实际上这里只是进行一次异步接受的投递
 * 即便是返回了接受到的数据,依然会在callback函数中被通知 
 */
int iocp_read(struct nd_session_iocp *iocp_map)
{
	int ret ;
	size_t space_len ;
	nd_netbuf_t  *read_buf;
	DWORD dwLastError = 0, dwBytesRecvd = 0, dwFlags = 0;

	if(nd_atomic_read(&TCPNODE_STATUS(iocp_map))==ETS_DEAD ){
		return -1;
	}
	
	nd_object_seterror((nd_handle)iocp_map, NDERR_SUCCESS) ;
	read_buf = iocp_recv_buf(iocp_map) ;

	if(ndlbuf_datalen(read_buf)==0) {
		ndlbuf_reset(read_buf);
	}
	else {
		ndlbuf_move_ahead(read_buf);
	}
	
	space_len = ndlbuf_freespace(read_buf) ;
	nd_assert(space_len>0);
	nd_assert(space_len<=ndlbuf_capacity(read_buf));
	if( space_len == 0){ 
		nd_assert(0);
		return 0 ;		//need to be close
	}
	
	iocp_map->wsa_readbuf.len = (ULONG) space_len ;
	iocp_map->wsa_readbuf.buf = ndlbuf_addr(read_buf) ;
	bzero(&iocp_map->ov_read.overlapped,sizeof(iocp_map->ov_read.overlapped)) ;

	ret = WSARecv(IOCP_MAP_FD(iocp_map),&(iocp_map->wsa_readbuf),1,
		&dwBytesRecvd,&dwFlags,(WSAOVERLAPPED*)&iocp_map->ov_read,NULL);

	if(0==ret) {
		if(0==dwBytesRecvd) {
			nd_object_seterror((nd_handle)iocp_map, NDERR_WOULD_BLOCK) ;
			return -1 ;
		}
		return (int)dwBytesRecvd ;
	}
	else {
		int serr = WSAGetLastError();
		iocp_map->__client_map.connect_node.sys_error = serr;
		if(WSA_IO_PENDING==serr || WSAEINTR==serr || WSAEWOULDBLOCK==serr) {
			nd_object_seterror((nd_handle)iocp_map, NDERR_WOULD_BLOCK) ;
			return -1 ;
		}
		nd_object_seterror((nd_handle)iocp_map, NDERR_READ) ;
		return 0 ;
	}

}

//利用close handle是调用的函数
int iocp_session_handle_close(nd_handle h, int force)
{
	if(force) {
		return iocp_close_client((struct nd_session_iocp *)h , force) ;
	}
	else {
		struct nd_session_iocp *client = (struct nd_session_iocp *)h ;
		nd_netbuf_t * send_buf = iocp_send_buf(client) ;
		size_t data_len = ndlbuf_datalen(send_buf) ;

		if(0==iocp_unnotified_length(client) &&0==data_len) {
			return iocp_close_client((struct nd_session_iocp *)h , 0) ;
		}
		else {
			if(data_len && TCPNODE_CHECK_OK(client)) {
				iocp_write(client) ;
			}
			//avoid pending data lost
			TCPNODE_SET_CLOSED(client) ;
			client->schedule_close_time = nd_time() ;
			return 0;
		}
	}
}

int iocp_close_client(struct nd_session_iocp *iocp_map, int force)
{
	struct nd_srv_node *srv_node =(struct nd_srv_node *) iocp_map->__client_map.connect_node.srv_root ;
	nd_assert(srv_node) ;
	if(nd_atomic_read(&TCPNODE_STATUS(iocp_map))==ETS_DEAD){
		return -1;
	}
	
	CancelIo((HANDLE)IOCP_MAP_FD(iocp_map));
	if(ETS_ACCEPT==nd_atomic_read(&TCPNODE_STATUS(iocp_map))) {
		nd_tcpnode_close(&(iocp_map->__client_map.connect_node) , force) ;
		if(nd_host_check_exit()) {
			return 0 ;
		}
		if(-1==nd_init_iocp_client_map(iocp_map, srv_node->fd) ) {
			srv_node->conn_manager.dealloc (iocp_map,nd_srv_get_allocator(srv_node));
		} 
		return 0;
	}
	else {
		nd_session_tcp_close(&iocp_map->__client_map, force);
		nd_atomic_set(&TCPNODE_STATUS(iocp_map), ETS_DEAD) ;
		if(-1==force)
			return 0;
		if(check_repre_accept((struct listen_contex *)srv_node)>0) {		
			pre_accept(srv_node) ;
		}
	}

	return 0 ;
}

int iocp_accept(struct nd_session_iocp *node)
{
	NDUINT16 session_id;
	int local_len, remote_len ;
	SOCKADDR_IN *local_addr, *remote_addr ;
	struct nd_srv_node *srv_root ;
	nd_netbuf_t  *recv_buf;
	nd_assert(nd_atomic_read(&TCPNODE_STATUS(node))==ETS_ACCEPT) ;
	if(nd_atomic_read(&TCPNODE_STATUS(node))!=ETS_ACCEPT)
		return -1 ;
	recv_buf = iocp_recv_buf(node) ;
	GetAcceptExSockaddrs(node->wsa_readbuf.buf,
		(DWORD)(ndlbuf_capacity(recv_buf) - (sizeof(struct sockaddr_in)+16)*2),
		sizeof(struct sockaddr_in)+16,
		sizeof(struct sockaddr_in)+16,
		(struct sockaddr **)&local_addr, &local_len,
		(struct sockaddr **)&remote_addr, &remote_len) ;

	memcpy(&node->__client_map.connect_node.remote_addr,remote_addr,sizeof(SOCKADDR_IN)) ;
#ifdef USER_THREAD_POLL
	if(NULL==CreateIoCompletionPort((HANDLE)IOCP_MAP_FD(node),g_hIOCompletionPort,(DWORD) node, 0)){
		nd_showerror() ;
		iocp_close_client(node, 0);
		return -1 ;

	}
#else 
	if(!g_BindIOCPEntry((HANDLE)IOCP_MAP_FD(node),iocp_callback,0)) {
		nd_showerror() ;
		iocp_close_client(node, 0);
		return -1 ;
	}
#endif
#if 0//defined(ND_DEBUG)
	else {
		char buf[32] ;
		char  *pszTemp = nd_inet_ntoa( remote_addr->sin_addr.s_addr ,buf);
		ndprintf(_NDT("Connect from %s:%d"), pszTemp, htons(remote_addr->sin_port));
		
	}
#endif

	srv_root =(struct nd_srv_node *) (node->__client_map.connect_node.srv_root) ;
	nd_assert(srv_root) ;

	if (((struct listen_contex*)srv_root)->close_accept ){
		iocp_close_client(node, 0);
		return -1 ;
	}

	//accept this client
	node->__client_map.connect_node.session_id = session_id =
				srv_root->conn_manager.accept (&srv_root->conn_manager,node);
	nd_assert(node->__client_map.connect_node.session_id) ;
	if(node->__client_map.connect_node.session_id==0) {
		iocp_close_client(node, 0);
		return -1 ;
	}

	memcpy(&(node->__client_map.connect_node.remote_addr),remote_addr, sizeof(*remote_addr)) ;
	if(srv_root->connect_in_callback){
		if(-1==srv_root->connect_in_callback(&node->__client_map,remote_addr,(nd_handle)srv_root) ){
			iocp_close_client(node, 0);
			srv_root->conn_manager.unlock (&srv_root->conn_manager,session_id);
			return -1 ;
		}
	}
	
	nd_atomic_set(&TCPNODE_STATUS(node),ETS_CONNECTED) ;

	node->__client_map.connect_node.start_time = nd_time();
	node->__client_map.connect_node.last_recv = nd_time(); 
	srv_root->conn_manager.unlock (&srv_root->conn_manager,session_id);
	return 0 ;
}

//发送格式化数据
int nd_iocp_sendmsg(struct nd_session_iocp *iocp_map,nd_packhdr_t *msg_buf, int flag) 
{
	struct netui_info *socket_addr = (struct netui_info* )& iocp_map->__client_map.connect_node;
    size_t s =  socket_addr->get_pack_size((nd_handle)socket_addr, &msg_buf) ;
    //size_t s =(size_t) ntohs(nd_pack_len(msg_buf)) ;
	return iocp_socket_write(iocp_map, msg_buf, s)  ;
}

//发送流式数据
int iocp_socket_write(struct nd_session_iocp *iocp_map, void *data, size_t send_len) 
{
	int ret = 0 ;
	nd_netbuf_t  *send_buf = iocp_send_buf(iocp_map) ;
	size_t free_space = ndlbuf_freespace(send_buf) ;

	if(send_len>ndlbuf_capacity(send_buf) && !ndlbuf_is_auto_inc(send_buf) ){
		nd_assert(0);
		return -1;
	}

	if(nd_atomic_read(&TCPNODE_STATUS(iocp_map))!= ETS_CONNECTED)
		return -1;

	if (free_space <send_len) {
		iocp_map->__client_map.connect_node.myerrno = NDERR_WOULD_BLOCK ;
		return -1;
	}
	ret = ndlbuf_write(send_buf,data, send_len,EBUF_SPECIFIED) ;

	if(0==iocp_unnotified_length(iocp_map))
		if(-1==iocp_write(iocp_map) )
			return -1;
	return ret ;
}


int iocp_cm_lock(struct nd_session_iocp *pclient, struct iocp_lock_info *lockinfo)
{
	NDUINT16 session_id =iocp_session_id(pclient);
	struct nd_srv_node *srv_node;
	struct cm_manager *pmanager ;
	void *addr ;
	//struct iocp_lock_info lock_info ;

	if(session_id==0)
		return -1 ;

	srv_node = (struct nd_srv_node *) pclient->__client_map.connect_node.srv_root ;
	nd_assert(srv_node) ;
	pmanager = &srv_node->conn_manager ;
	lockinfo->session_id = session_id ;
	lockinfo->pmanager = pmanager ;
	addr = pmanager->lock (pmanager,session_id) ;
	if(NULL==addr){
		//need to cancel iocp socket
		return -1 ;
	}
	else if(addr != (void*)pclient) {
		pmanager->unlock (pmanager,session_id) ;
		nd_assert(0);
		return -1 ;
	}
	
	return 0 ;
}

int update_iocp_cm(struct listen_contex *ls_info)
{
	int ret = 0;
	cmlist_iterator_t cm_iterator ;
	struct nd_session_iocp *client;
	struct nd_srv_node *srv_root = &(ls_info->tcp) ;
	struct cm_manager *pmanger = &srv_root->conn_manager ;
	
	for(client = pmanger->lock_first (pmanger,&cm_iterator) ; client; 	client = pmanger->lock_next (pmanger,&cm_iterator) ) {
		if(TCPNODE_STATUS(client)==ETS_DEAD) {
			tcp_release_death_node(client,0) ;		//释放死连接
		}
		else if(TCPNODE_CHECK_RESET(client)) {
			iocp_close_client(client,1) ;
			++ret ;
		}
		else if(TCPNODE_CHECK_CLOSED(client)) {
			if(iocp_unnotified_length(client)==0) {		//check pending data is send ok				
				iocp_close_client(client,0) ;
			}
			else {
				ndtime_t dist = nd_time() - client->schedule_close_time ;
				if(dist >= IOCP_DELAY_CLOSE_TIME) {

					iocp_close_client(client,0) ;
				}
			}
			++ret ;
		}
		else if(check_operate_timeout((nd_handle)client, ((struct nd_tcp_node*)client)->disconn_timeout)) {
			nd_object_seterror((nd_handle)client, NDERR_TIMEOUT) ;
			iocp_close_client(client,0) ;
		}
		client->__client_map.connect_node.update_entry((nd_handle)client) ;
	}
	return ret ;
}