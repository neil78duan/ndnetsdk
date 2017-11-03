/* file nd_thpoolio.c
 * net io by thread pool
 *
 * all right reserved by neil
 */

#include "nd_srvcore/nd_srvlib.h"

extern int destroy_udp_thpool(struct listen_contex *listen_info,int flag) ;
extern int listen_thread_createex(struct thread_pool_info *ic);


static int _delfrom_thread_pool(NDUINT16 sid, struct thread_pool_info * pthinfo) ;
/* @thread_num 线程个数
 * @th_sessions 每个线程的连接数
 */
int create_listen_thread_pool(struct listen_contex *handle, int pre_thnum, int session_num)
{
	int i;
	struct thread_pool_info  *piocp ;
	struct list_head *pos ;

	for(i=0; i<pre_thnum; i++) {
		if(0==nd_open_listen_thread((nd_listen_handle) handle, session_num) ) {
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

	snprintf(subth_info.srv_name, sizeof(subth_info.srv_name), "listen%d", ++index);
	ic->thid = nd_thsrv_createex(&subth_info,NDT_PRIORITY_HIGHT,1) ;

	if(!ic->thid ) {
		return -1 ;
	}
	hth = nd_thsrv_gethandle(ic->thid) ;
	nd_assert(hth) ;

	init_netthread_msg( hth ) ;
	nd_thsrv_resume(ic->thid) ;
	return 0;
}

int nd_close_listen_thread(nd_listen_handle h,nd_thsrvid_t sid)  
{
	struct thread_pool_info  *piocp =get_thread_poolinf( h,  sid);
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
	
	struct cm_manager *pmanger = nd_listensrv_get_cmmamager(h) ;	
	
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
	pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)listen_info) ;	
	
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
				struct nd_client_map*accepted = accetp_client_connect(listen_info,get_listen_fd(listen_info)) ;
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
						struct nd_client_map*accepted = accetp_client_connect(listen_info,node->fd) ;
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
	pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)listen_info) ;	

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




struct thread_pool_info *get_thread_poolinf(nd_listen_handle h, nd_thsrvid_t thid)
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


int nd_listen_get_threads(nd_listen_handle h) 
{
	int num = 0 ;
	struct thread_pool_info *node = NULL;
	struct listen_contex *listen_info = (struct listen_contex *) h ;
	
	list_for_each_entry(node, &listen_info->list_thread, struct thread_pool_info, list) {
		++num ;
	}
	return  num ;
}

int nd_fetch_sessions_in_thread(nd_listen_handle h, ndthread_t *threadid_buf, int *count_buf, int size)
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


#if !defined(USE_NEW_MODE_LISTEN_THREAD)
//打开一个线程服务器
nd_thsrvid_t nd_open_listen_thread(nd_listen_handle h, int session_num)
{
	struct listen_contex * lc = (struct listen_contex *) h;
	struct thread_pool_info  *piocp;

	if (nd_listen_get_threads(h) >= ND_MAX_THREAD_NUM) {
		nd_object_seterror(h, NDERR_LIMITED);
		nd_assert(0);
		return 0;
	}
#ifndef ND_UNIX
	if (ND_LISTEN_OS_EXT == lc->io_mod && lc->listen_id){
		return lc->listen_id;
	}
#endif 
	piocp = (struct thread_pool_info *) malloc(sizeof(*piocp) + sizeof(NDUINT16) * session_num);
	if (!piocp) {
		return 0;
	}
	bzero(piocp, sizeof(*piocp));

	INIT_LIST_HEAD(&piocp->list);
	piocp->max_sessions = session_num;
	piocp->lh = (struct listen_contex *) h;
	if (ND_LISTEN_OS_EXT == ((struct listen_contex *)h)->io_mod){
#ifdef ND_UNIX
		if (listen_thread_createex(piocp) != 0) {
			free(piocp);
			return 0;
		}
#endif
	}
	else {
		nd_threadsrv_entry th_func = (nd_threadsrv_entry)(((struct listen_contex *) h)->listen_id ? _nd_thpool_sub : _nd_thpool_main);
		if (listen_thread_create(piocp, th_func) != 0) {
			free(piocp);
			return 0;
		}
	}
	list_add_tail(&piocp->list, &piocp->lh->list_thread);

	if (0 == lc->listen_id){
		lc->listen_id = piocp->thid;
	}
	nd_thsrv_timer(piocp->thid, (nd_timer_entry)close_session_in_thread, piocp, 0, ETT_DESTROY);
	return piocp->thid;
}


int close_session_in_thread(struct thread_pool_info *thpi)
{
	int i, ret = 0;
	struct nd_client_map  *client;
	struct listen_contex *lc = (struct listen_contex *)(thpi->lh);
	struct cm_manager *pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)lc);

	for (i = thpi->session_num - 1; i >= 0; i--) {
		NDUINT16 session_id = thpi->sid_buf[i];
		client = (struct nd_client_map*) pmanger->lock(pmanger, session_id);
		if (!client)
			continue;
		nd_session_close((nd_handle)client, 1);
		pmanger->unlock(pmanger, session_id);
		++ret;
	}

	return ret;
}

int update_session_in_thread(struct cm_manager *pmanger, struct thread_pool_info *thpi)
{
	int i, ret, sleep = 0;
	struct nd_client_map  *client;
	struct listen_contex *lc = (struct listen_contex *)(thpi->lh);
	for (i = thpi->session_num - 1; i >= 0; i--) {
		NDUINT16 session_id = thpi->sid_buf[i];
		ret = 0;
		client = (struct nd_client_map*) pmanger->lock(pmanger, session_id);
		if (!client || !nd_handle_checkvalid((nd_handle)client, NDHANDLE_TCPNODE)) {
			_delfrom_thread_pool(session_id, thpi);
			continue;
		}
		if (0 == tryto_close_tcpsession((nd_session_handle)client, client->connect_node.disconn_timeout)){
			++sleep;
		}
		else  {
			ret = nd_do_netmsg(client, &lc->tcp);
			if (ret > 0) {
				++sleep;
			}
			else if (-1 == ret) {
				//tcp_client_close(client,1) ;
				nd_session_close((nd_handle)client, 1);
				++sleep;
			}
		}
		pmanger->unlock(pmanger, session_id);
	}

	//flush send buffer
	for (i = thpi->session_num - 1; i >= 0; i--) {
		NDUINT16 session_id = thpi->sid_buf[i];
		client = (struct nd_client_map*) pmanger->lock(pmanger, session_id);
		if (!client)
			continue;
		if (nd_connector_valid((nd_netui_handle)client)){
			_tcpnode_push_sendbuf(&client->connect_node, 0);
			client->connect_node.update_entry((nd_handle)client);
		}
		pmanger->unlock(pmanger, session_id);
	}
	return sleep;
}

//把sessio添加到线程池中
int addto_thread_pool(struct nd_client_map *client, struct thread_pool_info * pthinfo)
{
	if (pthinfo->session_num >= pthinfo->max_sessions){
		return-1;
	}
	pthinfo->sid_buf[pthinfo->session_num++] = client->connect_node.session_id;
#ifdef ND_UNIX
	if (pthinfo->lh->io_mod == ND_LISTEN_OS_EXT) {
		attach_to_listen(pthinfo, client);
	}

#endif 
	nd_logdebug("client %d add to %d thread server\n", nd_session_getid((nd_handle)client), nd_thread_self());
	return 0;
}

int _delfrom_thread_pool(NDUINT16 sid, struct thread_pool_info * pthinfo)
{
	int i;
	if (pthinfo->session_num == 1) {
		pthinfo->session_num = 0;
		return 0;
	}

	for (i = 0; i < pthinfo->session_num; i++) {
		if (pthinfo->sid_buf[i] == sid)	{
			break;
		}
	}
	if (pthinfo->session_num == i){
		//not find 
		return -1;
	}
	--pthinfo->session_num;
	pthinfo->sid_buf[i] = pthinfo->sid_buf[pthinfo->session_num];


	return 0;
}

int delfrom_thread_pool(struct nd_client_map *client, struct thread_pool_info * pthinfo)
{
	if (0 == _delfrom_thread_pool(client->connect_node.session_id, pthinfo)) {

#ifdef ND_UNIX
		if (pthinfo->lh->io_mod == ND_LISTEN_OS_EXT) {
			deattach_from_listen(pthinfo, client);
		}
#endif
		return 0;
	}

	return -1;
}
#endif 
