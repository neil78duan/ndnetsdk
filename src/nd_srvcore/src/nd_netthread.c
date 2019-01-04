/* file nd_netthread.c
 *
 * net listen thread server
 *
 * create by duan 
 *
 * 2017.11.1
 *
 */


#include "nd_srvcore/nd_srvlib.h"
#if defined(USE_NEW_MODE_LISTEN_THREAD)
#include "nd_srvcore/nd_netthread.h"


extern int listen_thread_createex(struct thread_pool_info *ic);
//打开一个线程服务器
nd_thsrvid_t nd_open_listen_thread(nd_listen_handle h, int session_num)
{
	struct listen_contex * lc = (struct listen_contex *) h;
	struct nd_netth_context  *piocp;

	if (nd_listen_get_threads(h) >= ND_MAX_THREAD_NUM) {
		nd_object_seterror(h, NDERR_LIMITED);
		nd_assert(0);
		return 0;
	}

	piocp = (struct nd_netth_context *) malloc(sizeof(*piocp) );
	if (!piocp) {
		return 0;
	}
	NETTH_CONTEXT_INIT(piocp);	
	piocp->lh =(struct listen_contex*) h;
	
#ifdef ND_UNIX
	if (listen_thread_createex(piocp) != 0) {
		free(piocp);
		return 0;
	}
#else 
	nd_threadsrv_entry th_func = (nd_threadsrv_entry)(((struct listen_contex *) h)->listen_id ? _nd_thpool_sub : _nd_thpool_main);
	if (listen_thread_create(piocp, th_func) != 0) {
		free(piocp);
		return 0;
	}
#endif

	list_add_tail(&piocp->list, &piocp->lh->list_thread);

	if (0 == lc->listen_id){
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
		struct nd_client_map  *client = list_entry(pos, struct nd_client_map, map_list);
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
		struct nd_client_map  *client = list_entry(pos, struct nd_client_map, map_list);

		if (!client || !nd_handle_checkvalid((nd_handle)client, NDHANDLE_TCPNODE)) {
			nd_session_close((nd_handle)client, 1);
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
	}

	list_for_each_safe(pos, next, &thpi->sessions_list) {
		struct nd_client_map  *client = list_entry(pos, struct nd_client_map, map_list);
		
		if (nd_connector_valid((nd_netui_handle)client)){
			_tcpnode_push_sendbuf(&client->connect_node);
			client->connect_node.update_entry((nd_handle)client);
		}
	}
	return sleep;

}



//把sessio添加到线程池中
int addto_thread_pool(struct nd_client_map *client, struct nd_netth_context * pthinfo)
{
	pthinfo->session_num++;
	list_add_tail(&client->map_list, &pthinfo->sessions_list);
#ifdef ND_UNIX
	attach_to_listen(pthinfo, client);
#endif 
	nd_logdebug("client %d add to %d thread server\n", nd_session_getid((nd_handle)client), nd_thread_self());
	return 0;
}

int delfrom_thread_pool(struct nd_client_map *client, struct thread_pool_info * pthinfo)
{
	list_del_init(&client->map_list);
	--pthinfo->session_num;

#ifdef ND_UNIX
	if (pthinfo->lh->io_mod == ND_LISTEN_OS_EXT) {
		deattach_from_listen(pthinfo, client);
	}
#endif
	return 0;
}

//////////////////////////////////////////////////////////////////////////


#endif
