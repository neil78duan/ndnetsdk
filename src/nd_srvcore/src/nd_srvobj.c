/* file srv_listen.h
 * listen socket (common version and listen entry)
 * version 1.0 all right reserved by neil
 * 2007-10
 */
#define IMPLEMENT_LISTEN_HANDLE
// 
// #include "nd_common/nd_common.h"
// #include "nd_srvcore/nd_srvlib.h"
// #include "nd_net/nd_netlib.h"
// #include "nd_common/nd_alloc.h"
#include "nd_srvcore/nd_srvlib.h"

#if defined(__ND_WIN__)
extern int nd_start_iocp_listen(struct listen_contex *listen_info) ;
#elif defined(__ND_LINUX__)
extern int _linux_listen_srv(struct listen_contex *listen_info) ;
#endif
void host_congest(ndsocket_t fd) ;

extern int udp_server_entry(struct listen_contex *listen_info) ;

extern int create_listen_thread_pool(struct listen_contex *handle, int pre_thnum, int session_num) ;
extern int destroy_listen_thread_pool(struct listen_contex *handle, int flag);

void nd_listen_contex_init(nd_listen_handle handle)
{
	bzero(handle,sizeof(struct listen_contex )) ;
	handle->tcp.size = sizeof(struct listen_contex );
	handle->tcp.type = NDHANDLE_LISTEN ;

	handle->operate_timeout = CONNECTION_TIMEOUT * 1000 * 60; 

	handle->tcp.data_entry = (data_in_entry) nd_dft_packet_handler ;
	handle->tcp.msg_entry =(net_msg_entry) nd_srv_translate_message ;
	handle->tcp.msg_caller = handle;


#if !defined (USE_NEW_MODE_LISTEN_THREAD)
	nd_node_preinit(&handle->tcp.conn_manager, nd_srvnode_create, nd_srvnode_destroy);
#endif
	nd_srv_set_cm_init(&handle->tcp,(cm_init )nd_session_tcp_init) ;
	
	INIT_LIST_HEAD(&handle->list_ext_ports) ;
}

int nd_listener_close(nd_listen_handle handle, int flag) 
{
	ENTER_FUNC()
	if (! nd_listener_checkvalid(handle)){
		LEAVE_FUNC() ;
		return 0;
	}

	if(handle->listen_id) {
		nd_thsrv_destroy(handle->listen_id, 0) ;
	}
	
	if (!list_empty(&handle->list_ext_ports)){
		struct listen_port_node *node, *next ;
		list_for_each_entry_safe(node, next, &handle->list_ext_ports, struct listen_port_node, list) {
			if (node->fd) {
				nd_socket_close(node->fd) ;
				node->fd =  0 ;
				free(node) ;
			}
		}
		INIT_LIST_HEAD(&handle->list_ext_ports) ;
	};
	
	destroy_listen_thread_pool(handle,0);

	nd_listener_close_all(handle) ;
	nd_srv_close(&handle->tcp) ;

	cm_destroy(nd_listener_get_session_mgr(handle)) ;
	
	if (handle->connector_hub){
		nd_node_destroy(handle->connector_hub);
		free(handle->connector_hub) ;
		handle->connector_hub = NULL;
	}

	LEAVE_FUNC();
	return 0 ;
}

int nd_listener_add_port(nd_listen_handle handle , int port, const char* bindip )
{
	struct listen_port_node *node = malloc(sizeof(struct listen_port_node)) ;
	if (!node) {
		return -1;
	}
	memset(node, 0, sizeof(*node)) ;
	INIT_LIST_HEAD(&node->list) ;
	
	node->fd = nd_socket_openport( port,  handle->tcp.sock_type, handle->tcp.protocol , 10,bindip) ;
	if (node->fd) {
		list_add_tail(&node->list, &handle->list_ext_ports) ;
		
		
		return node->fd;
	}
	else {
		free(node) ;
		return -1 ;
	}
	
}

int nd_listener_checkvalid(nd_listen_handle handle) 
{
	return get_listen_fd((struct listen_contex *)handle) ;
}
int nd_listener_set_capacity(nd_listen_handle handle, int max_client,size_t session_size) 
{
	handle->tcp.myerrno = NDERR_SUCCESS;
	if(0==session_size) {
		session_size = nd_session_hdr_size(handle->io_mod) ;
	}

	return cm_listen(&handle->tcp.conn_manager, max_client, (int)session_size);

}

session_valid_func nd_listener_set_valid_func(nd_listen_handle h_listen, session_valid_func func)
{
	session_valid_func ret = h_listen->check_valid_func;
	h_listen->check_valid_func = func;
	return ret;
}

int nd_listener_set_update(nd_listen_handle h_listen,listen_thread_update pre_entry, listen_thread_update end_entry) 
{
	h_listen->pre_update = pre_entry ;
	h_listen->end_update = end_entry ;
	return 0;
}
int nd_listener_open(int is_ipv6, int port, nd_listen_handle handle, int thread_num, const char* bindip)
{
	int ret ,io_mode;
	nd_assert(handle) ;

	io_mode = handle->io_mod ;
	//handle->tcp.bindip = bindip ;
	handle->tcp.bindip[0] = 0;
	if (bindip && *bindip) {
		ndstrncpy(handle->tcp.bindip, bindip, sizeof(handle->tcp.bindip));
	}
	ret = nd_srv_open(is_ipv6, port,10, &handle->tcp ) ;
	if(-1==ret) {
		handle->tcp.myerrno = NDERR_OPENFILE ;
		return -1 ;
	}
	nd_socket_nonblock(get_listen_fd(handle),1) ;
// 	if(_IS_UDT_MOD(io_mode)) {
// 		struct nd_thsrv_createinfo ls_info = {SUBSRV_RUNMOD_STARTUP,NULL,handle,handle,	("listen")	};
// 		ls_info.srv_entry = (nd_threadsrv_entry )udp_server_entry ;
// 
// 		handle->listen_id = nd_thsrv_createex(&ls_info,NDT_PRIORITY_HIGHT,0) ;
// 		return handle->listen_id?0:-1;
// 	}
//Windows iocp
#if defined(_MSC_VER)
	if(ND_LISTEN_OS_EXT==io_mode) {
		handle->tcp.status = 1;


		struct nd_thsrv_createinfo ls_info = {SUBSRV_RUNMOD_STARTUP,NULL,handle,handle,	("listen")	};
		ls_info.srv_entry = nd_start_iocp_listen ;
		handle->listen_id = nd_thsrv_createex(&ls_info,NDT_PRIORITY_HIGHT,0) ;
		return handle->listen_id?0:-1;
	}
#endif

	return create_listen_thread_pool( handle,thread_num? thread_num: nd_getcpu_num(),nd_listener_get_capacity(handle)) ;

}

void nd_listener_set_callback(nd_listen_handle handle, accept_callback income, deaccept_callback outcome) 
{	
	if(income)
		handle->tcp.connect_in_callback = income ;
	if(outcome)
		handle->tcp.connect_out_callback = outcome ;
	//if(pre_close)
	//	handle->tcp.pre_out_callback = pre_close ;
}


/* accept incoming  connect*/
struct nd_session_tcp * accetp_client_connect(struct listen_contex *listen_info, ndsocket_t sock_fd)
{
	ENTER_FUNC()
	NDUINT16  session_id ;
	ndsocket_t newconnect_fd ;
	socklen_t cli_len ;				/* client socket lenght */
	struct sockaddr_in6 client_addr ;
	
	struct nd_session_tcp *client_map ;
	struct cm_manager *pmanger  = nd_listener_get_session_mgr(listen_info) ;

	//cli_len = sizeof(*client_map);

	cli_len = sizeof (client_addr);
	newconnect_fd = (ndsocket_t)accept(sock_fd, (struct sockaddr*)&client_addr, &cli_len);
	if(newconnect_fd < 0){
		LEAVE_FUNC();
		nd_logdebug("system accept error : %s\n", nd_last_error());
		return 0 ;
	}

	if (listen_info->close_accept){
		host_congest(newconnect_fd) ;
		LEAVE_FUNC();
		return 0;
	}
	
	//alloc a connect node struct 	
	client_map =(struct nd_session_tcp*) pmanger->alloc (pmanger->node_alloctor) ;
	if(!client_map){
		host_congest(newconnect_fd) ;
		LEAVE_FUNC();
		return 0;
	}
	if(pmanger->init )
		pmanger->init (client_map, (nd_handle)listen_info) ;
	else 
		nd_session_tcp_init(client_map,(nd_handle)listen_info);

	if(-1== nd_socket_nonblock(newconnect_fd,1)) {
		nd_socket_close(newconnect_fd);
		pmanger->dealloc (client_map,pmanger->node_alloctor);
		LEAVE_FUNC();
		return 0 ;
	}
	//_set_socket_addribute(newconnect_fd) ;
	_set_ndtcp_session_dft_option(newconnect_fd) ;

	client_map->connect_node.fd = newconnect_fd ;
	memcpy(&(client_map->connect_node.remote_addr),&client_addr, cli_len) ;
	TCPNODE_SET_OK(&client_map->connect_node) ;
	client_map->connect_node.srv_root = (nd_handle)&(listen_info->tcp) ;

	client_map->connect_node.session_id = session_id = pmanger->accept (pmanger,client_map);
	if(0==client_map->connect_node.session_id) {
		nd_socket_close(newconnect_fd);
		pmanger->dealloc (client_map,pmanger->node_alloctor);
		LEAVE_FUNC();
		return 0;
	}

	if(listen_info->tcp.connect_in_callback){		
		if(-1==listen_info->tcp.connect_in_callback(client_map,(SOCKADDR_IN*)&client_addr,(nd_handle)listen_info) ){
			nd_socket_close(newconnect_fd);
			pmanger->deaccept (pmanger,client_map->connect_node.session_id);
			
			pmanger->unlock (pmanger,session_id);
			pmanger->dealloc (client_map,pmanger->node_alloctor);
			LEAVE_FUNC();
			return 0 ;
		}
	}
	client_map->connect_node.start_time = nd_time();
	client_map->connect_node.last_recv = nd_time(); 
	INIT_LIST_HEAD(&client_map->map_list) ;

	pmanger->unlock (pmanger,client_map->connect_node.session_id);
	
	nd_reg_handle((nd_handle)client_map) ;
	LEAVE_FUNC();
	return client_map ;

}

/*通知client服务器拥挤,并且关闭*/
void host_congest(ndsocket_t fd)
{
	nd_logerror("connect is limits\n") ;
	nd_socket_close(fd) ;
}

nd_handle nd_listener_get_session_allocator(struct listen_contex * handle) 
{
	return   nd_srv_get_allocator(&handle->tcp) ;
}

//得到连接管理器
struct cm_manager *nd_listener_get_session_mgr(struct listen_contex * handle) 
{
	return &(handle->tcp.conn_manager );
}

nd_thsrvid_t nd_listener_getowner(struct listen_contex * handle, nd_handle session) 
{
	struct cm_manager *pmmr = nd_listener_get_session_mgr(handle) ;
	return nd_node_get_owner(pmmr,nd_session_getid(session)) ;
}

int nd_listener_freenum(struct listen_contex * handle) 
{
	return handle->tcp.conn_manager.free_num(&handle->tcp.conn_manager) ;
}
//在监听器上建立定时器
ndtimer_t nd_listener_add_timer(nd_listen_handle h_listen,nd_timer_entry func,void *param,ndtime_t interval, int run_type ) 
{
	if(h_listen->listen_id) {
		return nd_thsrv_timer(h_listen->listen_id, func, param, interval, run_type) ;
	}
	return -1 ;
}

void nd_listener_del_timer(nd_listen_handle h_listen, ndtimer_t timer_id ) 
{
	if(h_listen->listen_id) {
		nd_thsrv_del_timer(h_listen->listen_id, timer_id) ;
	}	
}

//把一个连接器添加中监听器中,让监听器来处理网络事件
NDUINT16 nd_listener_attach(nd_listen_handle h_listen, nd_handle h_connector, nd_thsrvid_t thid)
{
	struct node_root **mgr_addr = NULL;
	struct node_root  *mgr ;
 	struct nd_tcp_node *conn_node = (struct nd_tcp_node *)h_connector;
	if(!h_connector)
		return 0;
	if(	conn_node->session_id )
		return conn_node->session_id  ;

	if (thid ==0 ){
		mgr_addr = &h_listen->connector_hub ;
		thid = h_listen->listen_id;
	}
	else {
		//find listen thread 
		struct thread_pool_info  *piocp=NULL;
		struct list_head *pos ;
		pos = h_listen->list_thread.next ;
		while(pos != &h_listen->list_thread) {
			piocp = list_entry(pos,struct thread_pool_info,list) ;
			pos = pos->next ;
			if (piocp->thid == thid){
				mgr_addr = &piocp->connector_hub;
				break ;
			}
		}
		if (!piocp || piocp->thid != thid){
			return 0;
		}
	}
	if (*mgr_addr==NULL){
		mgr = (struct node_root  *) malloc(sizeof(struct node_root)) ;
		if(!mgr)
			return 0 ;
		memset(mgr , 0, sizeof(struct node_root)) ;
		if (-1==nd_node_create(mgr,CONNECTORS_IN_LISTEN,0,1,NULL) ) {
			free(mgr) ;
			return 0 ;
		}
		nd_node_set_allocator(mgr, NULL, NULL,NULL) ;
		*mgr_addr = mgr ;
	}
	else {
		mgr = *mgr_addr ;
	}
	
	conn_node->session_id = mgr->accept(mgr, (void*)h_connector) ;
	if(conn_node->session_id) {
		mgr->unlock(mgr,conn_node->session_id) ;
	}
	nd_node_set_owner(mgr,conn_node->session_id, thid) ;
	return conn_node->session_id ;
}

int nd_listener_set_connector_close(nd_listen_handle h_listen,connector_close_entry ce) 
{
	if(!h_listen->connector_hub) {
		return -1 ;
	}
	h_listen->connector_hub->alloc = (node_alloc)ce ;
	return 0 ;
}
int nd_listener_deattach(nd_listen_handle h_listen, nd_handle h_connector,nd_thsrvid_t thid) 
{
	struct node_root **mgr_addr = NULL;
	struct node_root  *mgr =NULL;
	struct nd_tcp_node *conn_node = (struct nd_tcp_node *)h_connector;
	if(!h_connector || !conn_node->session_id )
		return 0 ;

	if (thid ==0 ){
		mgr_addr = &h_listen->connector_hub ;
		thid = h_listen->listen_id;
	}
	else {
		//find listen thread 
		struct thread_pool_info  *piocp=NULL;
		struct list_head *pos ;
		pos = h_listen->list_thread.next ;
		while(pos != &h_listen->list_thread) {
			piocp = list_entry(pos,struct thread_pool_info,list) ;
			pos = pos->next ;
			if (piocp->thid == thid){
				mgr_addr = &piocp->connector_hub;
				break ;
			}
		}
		if (!piocp || piocp->thid != thid){
			return 0;
		}
	}
	if (*mgr_addr==NULL) {
		return -1;
	}
	mgr = *mgr_addr ;	
	if (mgr->lock(mgr,conn_node->session_id)){
		mgr->deaccept(mgr, conn_node->session_id) ;
		mgr->unlock(mgr,conn_node->session_id) ;
		conn_node->session_id = 0;
	}
	return 0 ;
}

int nd_listener_close_all(nd_listen_handle listen_info)
{
	int ret = 0;
	struct nd_session_tcp  *client;
	struct nd_srv_node *srv_root = &(listen_info->tcp) ;
	struct cm_manager *pmanger = &srv_root->conn_manager ;

	if ( /*listen_info->io_mod == ND_LISTEN_COMMON && */!nd_host_check_exit())	{
		int i ;
		struct node_info *pnode = pmanger->connmgr_addr ;
		for( i=0; i<pmanger->capacity(pmanger); i++, pnode++) {
			if (nd_atomic_read(&pnode->used) >0) {
				nd_session_closeex(i+pmanger->base_id, (nd_handle)listen_info) ;
				++ret ;
			}
		}
	}
	else {
		cmlist_iterator_t cm_iterator ;

		nd_node_change_owner(pmanger,nd_thread_self()) ;
		for(client = pmanger->lock_first (pmanger,&cm_iterator) ; client; 	client = pmanger->lock_next (pmanger,&cm_iterator) ) {
			if (nd_handle_checkvalid((nd_handle)client,NDHANDLE_TCPNODE)) {
				continue;
			}
			nd_session_close((nd_handle)client,1) ;
			++ret;
		}
	}
	return ret ;
}

int update_connector_hub(nd_listen_handle listen_info)
{
	if (!listen_info->connector_hub) {
		return -1;
	}
	return update_connectors(listen_info->connector_hub) ; 
}

int update_connectors(struct node_root *pmanger)
{
	ENTER_FUNC()
	int ret=0, read_len  ;
	nd_handle  client;
	cmlist_iterator_t cm_iterator ;

	if(!pmanger|| pmanger->connect_num==0){
		LEAVE_FUNC();
		return -1 ;
	}

	for(client = pmanger->lock_first(pmanger,&cm_iterator) ; client;	client = pmanger->lock_next(pmanger,&cm_iterator) ) {
		if (nd_connector_valid((nd_netui_handle)client)) {
			read_len = nd_connector_update(client,0) ;
			if(read_len > 0)
				++ret ;
			if (-1==read_len) {
				nd_logerror("%s connector update error =%d\n", nd_object_get_instname(client), nd_object_lasterror(client)) ;
				pmanger->deaccept(pmanger, ((nd_netui_handle)client)->session_id);
				((nd_netui_handle)client)->session_id = 0;

				nd_connector_close(client, 0) ;

			}
		}
	}
	LEAVE_FUNC();
	return ret ;
}

#undef  IMPLEMENT_LISTEN_HANDLE
