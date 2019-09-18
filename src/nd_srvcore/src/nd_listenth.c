/* file nd_thpoolio.c
 * net io by thread pool
 *
 * all right reserved by neil
 */

#include "nd_srvcore/nd_srvlib.h"

extern int destroy_udp_thpool(struct listen_contex *listen_info,int flag) ;
extern int listen_thread_createex(struct thread_pool_info *ic);



extern int listen_thread_createex(struct thread_pool_info *ic);
//打开一个线程服务器
nd_thsrvid_t nd_listener_thread_create(nd_listen_handle h, int session_num)
{
	struct listen_contex * lc = (struct listen_contex *) h;
	struct nd_netth_context  *piocp;

	if (nd_listener_get_threads(h) >= ND_MAX_THREAD_NUM) {
		nd_object_seterror(h, NDERR_LIMITED);
		nd_assert(0);
		return 0;
	}

	piocp = (struct nd_netth_context *) malloc(sizeof(*piocp));
	if (!piocp) {
		return 0;
	}
	NETTH_CONTEXT_INIT(piocp);
	piocp->lh = (struct listen_contex*) h;

	if (_IS_UDT_MOD(lc->io_mod)) {
		nd_threadsrv_entry th_func = (nd_threadsrv_entry)(((struct listen_contex *) h)->listen_id ? _udt_sub_thread : _utd_main_thread);
		if (listen_thread_create(piocp, th_func) != 0) {
			free(piocp);
			return 0;
		}
	}
	else
#ifdef ND_UNIX
		if (ND_LISTEN_OS_EXT == lc->io_mod) {
			if (listen_thread_createex(piocp) != 0) {
				free(piocp);
				return 0;
			}
		}
		else
#endif
		{
			nd_threadsrv_entry th_func = (nd_threadsrv_entry)(((struct listen_contex *) h)->listen_id ? _nd_thpool_sub : _nd_thpool_main);
			if (listen_thread_create(piocp, th_func) != 0) {
				free(piocp);
				return 0;
			}
		}
	list_add_tail(&piocp->list, &piocp->lh->list_thread);

	if (0 == lc->listen_id) {
		lc->listen_id = piocp->thid;
	}
	nd_thsrv_timer(piocp->thid, (nd_timer_entry)close_session_in_thread, piocp, 0, ETT_DESTROY);
	return piocp->thid;
}


int close_session_in_thread(struct nd_netth_context *thpi)
{
	int  ret = 0;
	struct list_head *pos, *next;

	list_for_each_safe(pos, next, &thpi->sessions_list) {
		struct nd_session_tcp  *client = list_entry(pos, struct nd_session_tcp, map_list);
		nd_session_close((nd_handle)client, 1);
		++ret;
	}
	return ret;
}

int update_session_in_thread(struct cm_manager *pmanger, struct nd_netth_context *thpi)
{
	int  ret, sleep = 0;
	struct list_head *pos, *next;
	struct listen_contex *lc = (struct listen_contex *)(thpi->lh);

	list_for_each_safe(pos, next, &thpi->sessions_list) {
		struct nd_session_tcp  *client = list_entry(pos, struct nd_session_tcp, map_list);

		if (!client || !nd_handle_checkvalid((nd_handle)client, NDHANDLE_TCPNODE)) {
			nd_session_close((nd_handle)client, 1);
			continue;
		}
		if (0 == tryto_close_tcpsession((nd_handle)client, client->connect_node.disconn_timeout)) {
			++sleep;
		}
		else {
			ret = tcp_session_do_msg(client, &lc->tcp);
			if (ret > 0) {
				++sleep;
			}
			else if (-1 == ret) {
				//nd_session_tcp_close(client,1) ;
				nd_session_close((nd_handle)client, 1);
				++sleep;
			}
		}
	}

	list_for_each_safe(pos, next, &thpi->sessions_list) {
		struct nd_session_tcp  *client = list_entry(pos, struct nd_session_tcp, map_list);

		if (nd_connector_valid((nd_netui_handle)client)) {
			_tcpnode_push_sendbuf(&client->connect_node);
			client->connect_node.update_entry((nd_handle)client);
		}
	}
	return sleep;

}



//把sessio添加到线程池中
int addto_thread_pool(struct nd_session_tcp *client, struct nd_netth_context * pthinfo)
{
	pthinfo->session_num++;
	if (client->connect_node.type == NDHANDLE_UDPNODE) {
		struct nd_session_udt *udtcli = (struct nd_session_udt *) client;
		list_add_tail(&udtcli->map_list, &pthinfo->sessions_list);
	}
	else {
		list_add_tail(&client->map_list, &pthinfo->sessions_list);
	}

#ifdef ND_UNIX
	if (pthinfo->lh->io_mod == ND_LISTEN_OS_EXT) {
		attach_to_listen(pthinfo, client);
	}
#endif 
	nd_logdebug("client %d add to %d thread server\n", nd_session_getid((nd_handle)client), nd_thread_self());
	return 0;
}

int delfrom_thread_pool(struct nd_session_tcp *client, struct thread_pool_info * pthinfo)
{

	if (client->connect_node.type == NDHANDLE_UDPNODE) {
		struct nd_session_udt *udtcli = (struct nd_session_udt *) client;
		list_del_init(&udtcli->map_list);
	}
	else {
		list_del_init(&client->map_list);
	}

	--pthinfo->session_num;

#ifdef ND_UNIX
	if (pthinfo->lh->io_mod == ND_LISTEN_OS_EXT) {
		deattach_from_listen(pthinfo, client);
	}
#endif
	return 0;
}

//static int _delfrom_thread_pool(NDUINT16 sid, struct thread_pool_info * pthinfo) ;
/* @thread_num 线程个数
 * @th_sessions 每个线程的连接数
 */
int create_listen_thread_pool(struct listen_contex *handle, int pre_thnum, int session_num)
{
	int i;
	struct thread_pool_info  *piocp ;
	struct list_head *pos ;

	for(i=0; i<pre_thnum; i++) {
		if(0==nd_listener_thread_create((nd_listen_handle) handle, session_num) ) {
			break ;
		}
	}

	pos = handle->list_thread.next;
	if(pos == &handle->list_thread) {
		return -1 ;
	}
	piocp = list_entry(pos,struct thread_pool_info,list) ;

	if (handle->listen_id) {
		nd_thsrv_timer(handle->listen_id,(nd_timer_entry)update_connector_hub, handle,50, ETT_LOOP) ;
	}
	else {
		//nd_logwarn("not add update_connector_hub() ti timer") ;
	}
	
	do 	{
		pos = pos->next ;
		nd_thsrv_resume(piocp->thid) ;
		piocp = list_entry(pos,struct thread_pool_info,list) ;;
	} while (pos != &handle->list_thread);

	return 0;

}

int destroy_listen_thread_pool(struct listen_contex *handle,int flag)
{
	
	struct thread_pool_info  *piocp ;
	struct list_head *pos,*next ;

#if !defined(ND_UNIX)
	if(ND_LISTEN_OS_EXT==handle->io_mod ) {
		if (handle->listen_id) {
			nd_thsrv_destroy(handle->listen_id,flag) ;
			handle->listen_id = 0 ;
		}
		return 0;
	}

#endif
	list_for_each_safe(pos,next,&handle->list_thread) {
		piocp = list_entry(pos,struct thread_pool_info,list) ;
		list_del_init(pos) ;

		nd_thsrv_destroy(piocp->thid,0) ;
		if (piocp->connector_hub){
			cm_destroy(piocp->connector_hub) ;
			free(piocp->connector_hub) ;
			piocp->connector_hub = 0 ;
		}
		free(piocp);
	}
	INIT_LIST_HEAD(&handle->list_thread) ;
	handle->listen_id = 0;
	return 0 ;
}

int listen_thread_create(struct thread_pool_info *ic,nd_threadsrv_entry th_func)
{
	static int index = 0 ;
	nd_handle hth ;
	struct nd_thsrv_createinfo subth_info = {
		SUBSRV_RUNMOD_STARTUP,	//srv_entry run module (ref e_subsrv_runmod)
		(nd_threadsrv_entry)th_func,			//service main entry function address
		ic,		//param of srv_entry 
		ic 
	};

	ndsnprintf(subth_info.srv_name, sizeof(subth_info.srv_name), "listen%d", ++index);
	ic->thid = nd_thsrv_createex(&subth_info,NDT_PRIORITY_HIGHT,1) ;

	if(!ic->thid ) {
		return -1 ;
	}
	hth = nd_thsrv_gethandle(ic->thid) ;
	nd_assert(hth) ;

	init_netthread_msg( hth ) ;

	//nd_sleep(100);
	nd_thsrv_resume_force(ic->thid) ;
	return 0;
}

int nd_listener_thread_destroy(nd_listen_handle h,nd_thsrvid_t sid)  
{
	struct thread_pool_info  *piocp =nd_thpool_get_info( h,  sid);
	if (!piocp){
		return -1;
	}
	list_del_init(&piocp->list) ;
	nd_thsrv_destroy(piocp->thid,0) ;
	free(piocp);

	return 0;

}

//为session找一个相对空闲的线程
int nd_session_loadbalancing(nd_listen_handle h,NDUINT16 sessionid)
{
	ndthread_t self_id = nd_thread_self() ;
	struct thread_pool_info  *piocp ,*minpool=NULL;
	struct list_head *pos ;
	struct listen_contex *handle = (struct listen_contex *)h;
	
	struct cm_manager *pmanger = nd_listener_get_session_mgr(h) ;	
	
	pos = handle->list_thread.next ;
	
#ifndef ND_UNIX
	if (handle->io_mod == ND_LISTEN_OS_EXT) {
		nd_logerror("this platform not support this function \n") ;
		return -1 ;
	}
#endif
	
	if(pmanger->connect_num < SESSION_IN_THREAD_LOW_NUM ) { //low requirment 
		return 0 ;
	}
		
	while(pos != &handle->list_thread) {
		piocp = list_entry(pos,struct thread_pool_info,list) ;
		pos = pos->next ;
		if (!minpool){
			minpool = piocp ;
		}
		else if (minpool->session_num > piocp->session_num)	{
			minpool = piocp ;
		}
	}
	if (minpool==NULL || minpool->thid == self_id){
		return -1;
	}
	return nd_session_switch( h, sessionid,  minpool->thid) ;
}




int _nd_thpool_main(struct thread_pool_info *thip)
{
	ENTER_FUNC()
	//NDUINT16 session_id = 0;
	int ret ;
	struct cm_manager *pmanger  ;
	struct listen_contex *listen_info = (struct listen_contex *)thip->lh ;
	
	nd_handle context = nd_thsrv_gethandle(0) ;
	nd_assert(context) ;
	pmanger = nd_listener_get_session_mgr((nd_listen_handle)listen_info) ;	
	
	while (!nd_thsrv_isexit(context)){
		int sleep = LISTEN_INTERVAL;
		int msgret ;
		if (listen_info->pre_update){
			listen_info->pre_update((nd_handle)listen_info, context) ;
		}
		msgret = nd_thsrv_msghandler(context) ;
		if(-1== msgret) {		//处理线程消息
			if (listen_info->end_update){
				listen_info->end_update((nd_handle)listen_info, context) ;
			}
			break ;
		}
		else if(msgret)
			sleep = 0 ;	

		if (listen_info->close_accept==0){
			ret =nd_socket_wait_read(listen_info->tcp.fd,0) ;
			if(ret> 0) {
				struct nd_session_tcp*accepted = accetp_client_connect(listen_info,get_listen_fd(listen_info)) ;
				if(accepted) {
					addto_thread_pool(accepted, thip) ;
					sleep = 0 ;
				}
			}
			//listen port list
			if (!list_empty(&listen_info->list_ext_ports) ){
				struct listen_port_node *node ;
				list_for_each_entry(node, &listen_info->list_ext_ports, struct listen_port_node, list) {
					
					ret =nd_socket_wait_read(node->fd,0) ;
					if(ret> 0) {
						struct nd_session_tcp*accepted = accetp_client_connect(listen_info,node->fd) ;
						if(accepted) {
							addto_thread_pool(accepted, thip) ;
							sleep = 0 ;
						}
					}
				}
			}
		}
		if(update_session_in_thread(pmanger,thip) > 0) {
			sleep = 0;
		}
		if (thip->connector_hub) {
			if (update_connectors(thip->connector_hub) > 0)
				sleep = 0;
		}

		if (listen_info->end_update){
			listen_info->end_update((nd_handle)listen_info, context) ;
		}
		if(sleep) {
			nd_sleep(sleep) ;
		}
	}
	//close_session_in_thread(thip) ;
	LEAVE_FUNC();
	return 0 ;
}


/* entry of  listen service */
int _nd_thpool_sub(struct thread_pool_info *thip)
{
	ENTER_FUNC()
	struct cm_manager *pmanger  ;
	struct listen_contex *listen_info = (struct listen_contex *)thip->lh ;

	nd_handle context = nd_thsrv_gethandle(0) ;
	nd_assert(context) ;
	pmanger = nd_listener_get_session_mgr((nd_listen_handle)listen_info) ;	

	while (!nd_thsrv_isexit(context)){
		int sleep = LISTEN_INTERVAL;
		int msgret ;
		if (listen_info->pre_update){
			listen_info->pre_update((nd_handle)listen_info, context) ;
		}
		msgret = nd_thsrv_msghandler(context) ;
		if(-1== msgret) {		//处理线程消息
			if (listen_info->end_update){
				listen_info->end_update((nd_handle)listen_info, context) ;
			}
			break ;
		}
		else if(msgret)
			sleep = 0 ;	

		if(update_session_in_thread(pmanger,thip)> 0 ) {
			sleep = 0 ;
		}
		if (thip->connector_hub){
			if(update_connectors(thip->connector_hub) > 0) 
				sleep = 0 ;
		}

		if (listen_info->end_update){
			listen_info->end_update((nd_handle)listen_info, context) ;
		}
		if (sleep)
			nd_sleep(sleep) ;
	}
	//close_session_in_thread(thip) ;
	LEAVE_FUNC();
	return 0 ;
}

struct thread_pool_info *nd_thpool_get_info(nd_listen_handle h, nd_thsrvid_t thid)
{
	struct thread_pool_info *pthinfo = NULL;
	struct list_head *pos ;

	struct listen_contex *listen_info = (struct listen_contex *) h ;

	if (thid==0)	{
		thid = nd_thread_self() ;
	}

	pos = listen_info->list_thread.next ;
	while(pos!= &listen_info->list_thread) {
		pthinfo = list_entry(pos,struct thread_pool_info ,list) ;
		if (pthinfo->thid == thid)	{
			return pthinfo ;
		}
		pos = pos->next ;
		pthinfo = NULL;
	}
	return NULL;
}


int nd_listener_get_threads(nd_listen_handle h) 
{
	int num = 0 ;
	struct thread_pool_info *node = NULL;
	struct listen_contex *listen_info = (struct listen_contex *) h ;
	
	list_for_each_entry(node, &listen_info->list_thread, struct thread_pool_info, list) {
		++num ;
	}
	return  num ;
}

int nd_listener_fetch_sessions(nd_listen_handle h, ndthread_t *threadid_buf, int *count_buf, int size)
{
	int num = 0 ;
	struct thread_pool_info *node = NULL;
	struct listen_contex *listen_info = (struct listen_contex *) h ;
	
	list_for_each_entry(node, &listen_info->list_thread, struct thread_pool_info, list) {
		if (num > size) {
			return num ;
		}
		*threadid_buf = node->thid ;
		++threadid_buf ;
		*count_buf = node->session_num ;
		++count_buf ;
		
		++num ;
	}
	return  num ;
	
}

//////////////////////////////////////////////////////////////////////////
