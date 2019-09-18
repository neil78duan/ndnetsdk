  /* file : linux_listen.c
 * listen service on linux platform
 *
 * all right reserved by neil duan 2007
 * 2007-10-22
 */

#include "nd_common/nd_common.h"
#include "nd_srvcore/nd_srvlib.h"

#if defined(ND_UNIX)

#if defined( __ND_LINUX__ )
#include <sys/epoll.h>
#elif defined (__ND_MAC__)
#include "nd_srvcore/nd_epoll.h"
#endif

//#if defined(__LINUX__)
//#include <sys/epoll.h>

//static int create_sub_update_thread(struct listen_contex *listen_info) ;
int epoll_main(struct thread_pool_info *thip);
int epoll_sub(struct thread_pool_info *thip) ;
void update_epoll_event(struct epoll_event* ev_node,struct cm_manager *pmanger,struct thread_pool_info *thip);

int epoll_update_session(struct cm_manager *pmanger,struct thread_pool_info *thpi);

int create_event_fd(int maxnumber)
{
    return epoll_create(maxnumber) ;
}

int epoll_main(struct thread_pool_info *thip)
{
	ENTER_FUNC()
	ndsocket_t listen_fd ;
	int  event_num;
	struct epoll_event ev_listen, *ev_buf ;

	nd_handle thread_handle = nd_thsrv_gethandle(0)  ;
	struct listen_contex *listen_info = (struct listen_contex *)thip->lh ;
	struct cm_manager *pmanger = nd_listener_get_session_mgr((nd_listen_handle)listen_info) ;

	nd_assert(thread_handle) ;
	listen_fd = get_listen_fd(listen_info);
	event_num = nd_listener_get_capacity((nd_listen_handle)thip->lh);

	ev_buf = (struct epoll_event*)malloc(event_num * sizeof(*ev_buf)) ;
	if(!ev_buf){
		nd_logerror("alloc memory ") ;
		LEAVE_FUNC();
		return -1 ;
	}

	ev_listen.data.u32 = (__uint32_t)  ((listen_fd<<16) & 0xffff0000);
	ev_listen.events = EPOLLIN ;

	if(-1==epoll_ctl(thip->iopc_handle,EPOLL_CTL_ADD, listen_fd,&ev_listen) ) {
		nd_logerror("epoll ctrl error :%s" AND nd_last_error()) ;
		goto LISTEN_EXIT ;
	}
	
	if (!list_empty(&listen_info->list_ext_ports) ){
		struct listen_port_node *node ;
		list_for_each_entry(node, &listen_info->list_ext_ports, struct listen_port_node, list) {
			
			ev_listen.data.u32 =  ((node->fd<<16) & 0xffff0000) ;
			ev_listen.events = EPOLLIN ;
			if(-1==epoll_ctl(thip->iopc_handle,EPOLL_CTL_ADD, node->fd, &ev_listen) ) {
				nd_logerror("epoll ctrl error :%s" AND nd_last_error()) ;
				goto LISTEN_EXIT ;
			}
			
		}
	}

	while (!nd_thsrv_isexit(thread_handle)){
        int i;
        int msgret ;
        int nfds ;

        if (listen_info->pre_update){
            listen_info->pre_update((nd_handle)listen_info, thread_handle) ;
        }
        msgret = nd_thsrv_msghandler(thread_handle) ;
        if(-1== msgret) {
            if (listen_info->end_update){
                listen_info->end_update((nd_handle)listen_info, thread_handle) ;
            }
            break ;
        }

        nfds = epoll_wait(thip->iopc_handle,ev_buf,event_num,50) ;

		for (i=0; i<nfds; i++){
			if((ev_buf[i].data.u32 & 0xffff)==0) {
				ndsocket_t fd = ev_buf[i].data.u32 >> 16 ;
				
				struct nd_session_tcp *client_map = accetp_client_connect(listen_info,fd) ;
				if(client_map && client_map->connect_node.session_id) {
					nd_socket_nonblock(client_map->connect_node.fd, 1) ;
					if (-1==addto_thread_pool(client_map,thip))	{
						nd_session_tcp_close(client_map,1) ;
					}
					client_map->connect_node.close_entry =(nd_close_callback ) nd_session_tcp_close ;
				}
			}
			else {
				update_epoll_event(&ev_buf[i],pmanger,thip);
			}		//end if
		}			//end for

		epoll_update_session(pmanger,thip) ;

        if (listen_info->end_update){
            listen_info->end_update((nd_handle)listen_info, thread_handle) ;
        }
//		if (listen_info->connector_hub) {
//			update_connector_hub((nd_listen_handle)listen_info);
//		}
	}					//end while

LISTEN_EXIT:
	//close_session_in_thread(thip) ;
	close(thip->iopc_handle) ;
	free(ev_buf) ;
	LEAVE_FUNC();
	return 0 ;
}

int epoll_sub(struct thread_pool_info *thip)
{
	ENTER_FUNC()
	int  event_num;
	struct epoll_event  *ev_buf ;
	nd_handle thread_handle = nd_thsrv_gethandle(0)  ;
	struct listen_contex *listen_info = (struct listen_contex *)thip->lh ;
	struct cm_manager *pmanger = nd_listener_get_session_mgr((nd_listen_handle)listen_info) ;

	nd_assert(thread_handle) ;
	//event_num = thip->max_sessions + 1;
	event_num = nd_listener_get_capacity((nd_listen_handle)thip->lh);
	
	ev_buf = (struct epoll_event*)malloc(event_num * sizeof(*ev_buf)) ;
	if(!ev_buf){
		nd_logerror("alloc memory ") ;
		LEAVE_FUNC();
		return -1 ;
	}

	while (!nd_thsrv_isexit(thread_handle)){
		int i;
        int msgret ;
        int nfds ;

        if (listen_info->pre_update){
            listen_info->pre_update((nd_handle)listen_info, thread_handle) ;
        }
        msgret = nd_thsrv_msghandler(thread_handle) ;
        if(-1== msgret) {
            if (listen_info->end_update){
                listen_info->end_update((nd_handle)listen_info, thread_handle) ;
            }
            break ;
        }

		nfds = epoll_wait(thip->iopc_handle,ev_buf,event_num,50) ;
		for (i=0; i<nfds; i++){
			update_epoll_event(&ev_buf[i],pmanger,thip);
		}			//end for
		epoll_update_session(pmanger,thip) ;


        if (listen_info->end_update){
            listen_info->end_update((nd_handle)listen_info, thread_handle) ;
        }
	}					//end while

LISTEN_EXIT:
	//close_session_in_thread(thip) ;
	close(thip->iopc_handle) ;
	free(ev_buf) ;
	LEAVE_FUNC();
	return 0 ;
}

void update_epoll_event(struct epoll_event* ev_node,struct cm_manager *pmanger,struct thread_pool_info *thip)
{
	ENTER_FUNC() ;
	struct nd_session_tcp *client_map;
	//int fd_tmp =(ev_node->data.u32 >> 16) & 0xffff;
	NDUINT16 session_id =(NDUINT16) (ev_node->data.u32) & 0xffff;

	client_map = pmanger->lock(pmanger,session_id) ;
	
	if(!client_map ) {
		LEAVE_FUNC();
		return ;
	}

	if(TCPNODE_CHECK_CLOSED(client_map)|| !check_connect_valid(& (client_map->connect_node))) {
		nd_session_tcp_close(client_map,1) ;
	}
#ifdef __LINUX__
	else  if(ev_node->events & EPOLLIN){
		int ret = tcp_session_do_msg(client_map, &thip->lh->tcp) ;
		if(ret ==-1){
			nd_session_tcp_close(client_map,1) ;
		}
		else if(ret>0){
			nd_tcpnode_flush_sendbuf(&(client_map->connect_node)) ;
		}
	}
	else if(ev_node->events & EPOLLOUT) {
        nd_tcpnode_flush_sendbuf(&(client_map->connect_node)) ;
	}
	else if(ev_node->events & (EPOLLERR + EPOLLHUP) ) {
		if (ETS_DEAD!=TCPNODE_STATUS(client_map) )	{
			nd_session_tcp_close(client_map,1) ;
		}
		else {
			delfrom_thread_pool(client_map,thip) ;
		}
	}
#else 
	else  {
		int ret = tcp_session_do_msg(client_map, &thip->lh->tcp) ;
		if(ret ==-1){
			nd_session_tcp_close(client_map,1) ;
		}
		else if(ret>0){
			nd_tcpnode_flush_sendbuf((nd_netui_handle)&(client_map->connect_node)) ;
		}
	}
#endif
	pmanger->unlock(pmanger,session_id) ;
	
	LEAVE_FUNC();
}


int attach_to_listen(struct thread_pool_info *thip,struct nd_session_tcp *client_map)
{
	struct epoll_event ev_listen;

	ev_listen.events = EPOLLIN | EPOLLERR | EPOLLHUP| EPOLLRDHUP ;
	ev_listen.data.u32 =(__uint32_t)(client_map->connect_node.session_id) |(client_map->connect_node.fd)<<16 ;
	if (epoll_ctl(thip->iopc_handle, EPOLL_CTL_ADD, client_map->connect_node.fd, &ev_listen) < 0) {
		return -1 ;
	}
	return 0 ;
}

int deattach_from_listen(struct thread_pool_info *thip , struct nd_session_tcp *client_map)
{
	struct epoll_event ev_listen;
	epoll_ctl(thip->iopc_handle, EPOLL_CTL_DEL, client_map->connect_node.fd, &ev_listen);
	return 0 ;
}


//#elif defined(__MAC_OS__)
//
//#include <sys/types.h>
//#include <sys/event.h>
//#include <sys/time.h>
//
//int regeister_event(int event_id, int regid,void *udata)
//{
//    struct kevent changes[1];
//    EV_SET(&changes[0], regid, EVFILT_READ, EV_ADD, 0, 0, NULL);
//    changes[0].udata = udata ;
//    return kevent(event_id, changes, 1, NULL, 0, NULL);
//
//}
//
//void update_epoll_event(struct kevent* ev_node,struct cm_manager *pmanger,struct thread_pool_info *thip)
//{
//	ENTER_FUNC()
//	
//    struct nd_session_tcp *client_map;
//    NDUINT32 udata = (NDUINT32) ev_node->udata ;
//    intptr_t len = ev_node->data;
//
//    int fd_tmp =(udata >> 16) & 0xffff;
//    NDUINT16 session_id =(NDUINT16) (udata & 0xffff);
//	
//	
//    client_map = pmanger->lock(pmanger,session_id) ;
//	if(!client_map ) {		
//		LEAVE_FUNC();
//        return ;
//	}
//    if(0==len || TCPNODE_CHECK_CLOSED(client_map)|| !check_connect_valid(& (client_map->connect_node))) {
//        nd_session_tcp_close(client_map,1) ;
//    }
//    else  {
//        int ret = tcp_session_do_msg(client_map, &thip->lh->tcp) ;
//        if(ret ==-1){
//            nd_session_tcp_close(client_map,1) ;
//        }
//        else if(ret>0){
//            nd_tcpnode_flush_sendbuf((nd_netui_handle)&(client_map->connect_node)) ;
//        }
//    }
//    pmanger->unlock(pmanger,session_id) ;
//	
//	LEAVE_FUNC();
//}
//
//
//int attach_to_listen(struct thread_pool_info *thip,struct nd_session_tcp *client_map)
//{
//    //struct epoll_event ev_listen;
//    void *data =(NDUINT32)(client_map->connect_node.session_id) |(client_map->connect_node.fd)<<16 ;
//    return regeister_event(thip->iopc_handle, client_map->connect_node.fd, data) ;
//
//}
//
//int deattach_from_listen(struct thread_pool_info *thip , struct nd_session_tcp *client_map)
//{
//    int fd = client_map->connect_node.fd ;
//    struct kevent changes[1];
//    EV_SET(&changes[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
//    return kevent(thip->iopc_handle, changes, 1, NULL, 0, NULL);
//
//}
//
//
//int kqueue_main(struct thread_pool_info *thip)
//{
//    ENTER_FUNC()
//    //int ret ;
//    ndsocket_t listen_fd ;
//    int  event_num;
//    struct kevent  *ev_buf ;
//
//    nd_handle thread_handle = nd_thsrv_gethandle(0)  ;
//    struct listen_contex *listen_info = (struct listen_contex *)thip->lh ;
//    struct cm_manager *pmanger = nd_listener_get_session_mgr((nd_listen_handle)listen_info) ;
//
//    nd_assert(thread_handle) ;
//    listen_fd = get_listen_fd(listen_info);
//    event_num = thip->max_sessions + 1;
//
//    ev_buf = (struct kevent *)malloc(event_num * sizeof(*ev_buf)) ;
//    if(!ev_buf){
//        nd_logerror("alloc memory ") ;
//        LEAVE_FUNC();
//        return -1 ;
//    }
//
//    //register
//    if (-1==regeister_event(thip->iopc_handle, listen_fd,NULL)) {
//		nd_logerror("kqueue register fd ") ;
//		LEAVE_FUNC();
//        return -1;
//    }
//
//
//    while (!nd_thsrv_isexit(thread_handle)){
//        struct timespec tmsp = {0, 50 *1000000} ;
//        int i,msgret ,nfds ;
//
//        if (listen_info->pre_update){
//            listen_info->pre_update((nd_handle)listen_info, thread_handle) ;
//        }
//        msgret = nd_thsrv_msghandler(thread_handle) ;
//        if(-1== msgret) {
//            if (listen_info->end_update){
//                listen_info->end_update((nd_handle)listen_info, thread_handle) ;
//            }
//            break ;
//        }
//
//        nfds = kevent(thip->iopc_handle, NULL, 0, ev_buf, event_num, &tmsp);
//
//        for (i=0; i<nfds; i++){
//            uintptr_t sock = ev_buf[i].ident;
//            //intptr_t data = ev_buf[i].data;
//
//            if(sock==listen_fd) {
//                struct nd_session_tcp *client_map = accetp_client_connect(listen_info) ;
//                if(client_map && client_map->connect_node.session_id) {
//                    nd_socket_nonblock(client_map->connect_node.fd, 1) ;
//                    if (-1==addto_thread_pool(client_map,thip))	{
//                        nd_session_tcp_close(client_map,1) ;
//                    }
//                    client_map->connect_node.close_entry =(nd_close_callback ) nd_session_tcp_close ;
//                }
//            }
//            else {
//                update_epoll_event(&ev_buf[i],pmanger,thip);
//            }		//end if
//        }			//end for
//
//        epoll_update_session(pmanger,thip) ;
//
//        if (listen_info->end_update){
//            listen_info->end_update((nd_handle)listen_info, thread_handle) ;
//        }
////		if (listen_info->connector_hub) {
////			update_connector_hub((nd_listen_handle)listen_info);
////		}
//    }					//end while
//
//LISTEN_EXIT:
//    //close_session_in_thread(thip) ;
//    close(thip->iopc_handle) ;
//    free(ev_buf) ;
//    LEAVE_FUNC();
//    return 0 ;
//}
//
//int kqueue_sub(struct thread_pool_info *thip)
//{
//    ENTER_FUNC()
//    int  event_num;
//    struct kevent  *ev_buf ;
//    nd_handle thread_handle = nd_thsrv_gethandle(0)  ;
//    struct listen_contex *listen_info = (struct listen_contex *)thip->lh ;
//    struct cm_manager *pmanger = nd_listener_get_session_mgr((nd_listen_handle)listen_info) ;
//
//    nd_assert(thread_handle) ;
//    event_num = thip->max_sessions + 1;
//
//    ev_buf = (struct kevent*)malloc(event_num * sizeof(*ev_buf)) ;
//    if(!ev_buf){
//        nd_logerror("alloc memory ") ;
//        LEAVE_FUNC();
//        return -1 ;
//    }
//
//    while (!nd_thsrv_isexit(thread_handle)){
//        struct timespec tmsp = {0, 50 *1000000} ;
//        int i,msgret ,nfds ;
//
//        if (listen_info->pre_update){
//            listen_info->pre_update((nd_handle)listen_info, thread_handle) ;
//        }
//        msgret = nd_thsrv_msghandler(thread_handle) ;
//        if(-1== msgret) {
//            if (listen_info->end_update){
//                listen_info->end_update((nd_handle)listen_info, thread_handle) ;
//            }
//            break ;
//        }
//
//        nfds = kevent(thip->iopc_handle, NULL, 0, ev_buf, event_num, &tmsp);
//
//
//        for (i=0; i<nfds; i++){
//            update_epoll_event(&ev_buf[i],pmanger,thip);
//        }			//end for
//        epoll_update_session(pmanger,thip) ;
//
//        if (listen_info->end_update){
//            listen_info->end_update((nd_handle)listen_info, thread_handle) ;
//        }
//    }					//end while
//
//LISTEN_EXIT:
//    //close_session_in_thread(thip) ;
//    close(thip->iopc_handle) ;
//    free(ev_buf) ;
//    LEAVE_FUNC();
//    return 0 ;
//}
//
//int create_event_fd( int maxnumber )
//{
//    return kqueue();
//}
//
//#define epoll_sub kqueue_sub
//#define epoll_main kqueue_main
//
//#endif			//linux

#if defined(ND_UNIX)

//
//int thpoolex_create(struct listen_contex *handle, int pre_thnum, int session_num)
//{
//    int i ;
//    struct thread_pool_info  *piocp ;
//    struct list_head *pos ;
//
//    for(i=0; i<pre_thnum; i++) {
//        if(0==nd_open_listen_thread((nd_listen_handle) handle, session_num) ) {
//            break ;
//        }
//    }
//
//    pos = handle->list_thread.next;
//    if(pos == &handle->list_thread) {
//        return -1 ;
//    }
//    piocp = list_entry(pos,struct thread_pool_info,list) ;
//    //handle->listen_id = piocp->thid ;
//
//    //nd_thsrv_timer(piocp->thid,(nd_timer_entry)update_connector_hub, handle,10, ETT_LOOP) ;
//    do 	{
//        pos = pos->next ;
//        nd_thsrv_resume(piocp->thid) ;
//        piocp = list_entry(pos,struct thread_pool_info,list) ;;
//    } while (pos != &handle->list_thread);
//
//    return 0;
//
//}



int listen_thread_createex(struct thread_pool_info *ic)
{
    int epoll_fd;
    epoll_fd = create_event_fd(nd_listener_get_capacity((nd_listen_handle)ic->lh)) ;
	
	
    if(epoll_fd<=0) {
        nd_showerror() ;
        return -1;
    }

    ic->iopc_handle = epoll_fd;
    nd_threadsrv_entry th_func = (nd_threadsrv_entry)(ic->lh->listen_id ?epoll_sub:epoll_main);
    return listen_thread_create(ic,th_func);
}

int epoll_update_session(struct cm_manager *pmanger,struct thread_pool_info *thpi)
{
	ENTER_FUNC() ;
	int sleep=0;
	
	//flush send buffer
#if !defined (USE_NEW_MODE_LISTEN_THREAD)
	struct nd_session_tcp  *client;
	for (i=thpi->session_num-1; i>=0;i-- ) {
		NDUINT16 session_id = thpi->sid_buf[i];
		client =(struct nd_session_tcp*) pmanger->lock(pmanger,session_id) ;
		if (!client)
			continue ;
		if (0==tryto_close_tcpsession((nd_session_handle)client, client->connect_node.disconn_timeout )){
			++sleep ;
		}
		else if (TCPNODE_IS_OK(client)){
			if(_tcpnode_push_sendbuf(&client->connect_node,0) > 0)
			++sleep ;
			client->connect_node.update_entry((nd_handle)client) ;
		}
		pmanger->unlock(pmanger,session_id) ;
	}
#else
	struct list_head *pos, *next;
	list_for_each_safe(pos, next, &thpi->sessions_list) {
		struct nd_session_tcp  *client = list_entry(pos, struct nd_session_tcp, map_list);
		if (0==tryto_close_tcpsession((nd_handle)client, client->connect_node.disconn_timeout )){
			++sleep ;
		}
		else if (TCPNODE_IS_OK(client)){
			if(_tcpnode_push_sendbuf(&client->connect_node) > 0)
			++sleep ;
			client->connect_node.update_entry((nd_handle)client) ;
		}
		
	}
#endif
	LEAVE_FUNC() ;
	return sleep;
}


#endif

#endif 
