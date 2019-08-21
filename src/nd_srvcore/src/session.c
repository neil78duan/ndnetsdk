/* file : srv_netio.c
 * impelemention of nd-server net io interface
 * 
 * version 1.0
 * all right reserved by neil duan 
 * 2008-4
 */

#define ND_IMPLEMENT_HANDLE
typedef struct netui_info *nd_handle;

//#include "nd_common/nd_common.h"
#include "nd_srvcore/nd_srvlib.h"
//#include "nd_net/nd_netlib.h"


int tcp_client_close(struct nd_client_map* cli_map, int force)
{	
	ENTER_FUNC()
	int ret = 0 ;
	NDUINT16  session = cli_map->connect_node.session_id ;
	struct nd_srv_node *root = (struct nd_srv_node *) (cli_map->connect_node.srv_root);
	struct thread_pool_info *thpi ;
	if (TCPNODE_STATUS(cli_map) == ETS_DEAD ) {
		LEAVE_FUNC();
		return 0 ;
	}
	
	if(root->connect_out_callback)	
		root->connect_out_callback(cli_map,(nd_handle)root) ;
	nd_tcpnode_close(&(cli_map->connect_node) , force) ;		//关闭socket
	
	thpi = get_thread_poolinf((nd_listen_handle) root, 0) ;
	if (thpi) {
		delfrom_thread_pool(cli_map, thpi) ;
	}
	ret = root->conn_manager.deaccept (&root->conn_manager, session) ;
	if(0!=ret) {
		TCPNODE_STATUS(cli_map) = ETS_DEAD ;
		LEAVE_FUNC();
		return ret ;
	}
	nd_client_map_destroy(cli_map) ;
	
	root->conn_manager.dealloc (cli_map, nd_srv_get_allocator(root));
	LEAVE_FUNC();
	return 0 ;
}

int nd_session_close(nd_session_handle cli_handle, int force)
{
	int ret ;
	ENTER_FUNC()
//	nd_log_screen("close)
	nd_assert(cli_handle && cli_handle->close_entry) ;
	if(cli_handle->myerrno==NDERR_SUCCESS) {
		cli_handle->myerrno = NDERR_CLOSED ;
	}

	nd_unreg_handle((nd_handle)cli_handle) ;
	
	_nd_object_on_destroy(cli_handle, 1) ;
	
	ret = cli_handle->close_entry(cli_handle, force) ;
	
	LEAVE_FUNC();
	return ret ;
}

int nd_session_closeex(NDUINT16 session_id,nd_handle listen_handle)
{
	ndthread_t thid ;
	struct listen_contex *lc = (struct listen_contex*)listen_handle ;
	struct cm_manager *pmanger  ;

	pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)lc) ;
	if (!pmanger){
		return -1 ;
	}
	thid = nd_node_get_owner(pmanger,session_id) ;
	if (thid == nd_thread_self() ) {
		nd_handle client = (nd_handle) pmanger->lock(pmanger,session_id) ;
		if(!client) 
			return -1 ;		
		nd_session_close(client,0) ;
		pmanger->unlock(pmanger,session_id);
	}
	else {
		nd_thsrv_send(thid,E_THMSGID_CLOSE_SESSION,&session_id, sizeof(session_id)) ;
	}	
	return 0 ;
}

int nd_session_flush_sendbuf(nd_session_handle cli_handle, int flag) 
{
	ENTER_FUNC()
	int ret = 0 ;
	nd_netui_handle h_header =(nd_netui_handle)cli_handle ;
	nd_assert(h_header) ;
	h_header->myerrno = NDERR_SUCCESS ;
	if(h_header->type==NDHANDLE_TCPNODE){
		struct nd_client_map *cli_map =(struct nd_client_map *)cli_handle;
		if(0== flag) 
			ret = _tcpnode_push_sendbuf(&(cli_map->connect_node)) ;
		else if(1==flag)
			ret = _tcpnode_push_force(&(cli_map->connect_node)) ;
		else if(2==flag)
			ret = nd_tcpnode_tryto_flush_sendbuf(&(cli_map->connect_node)) ;
		
	}

	LEAVE_FUNC();
	return ret ;
	
}

int check_operate_timeout(nd_session_handle nethandle, ndtime_t tmout)
{

	struct listen_contex *lc ;
	ndtime_t interval = nd_time() - nethandle->last_recv;
	if(interval > tmout) {
		return 1 ;
	}

	//禁止空连接
	lc =(struct listen_contex *) nethandle->srv_root ;
	if (lc && lc->empty_conn_timeout && nethandle->level <= EPL_CONNECT){
		interval = nd_time() - nethandle->start_time;
		if(interval > lc->empty_conn_timeout) {
			return 1 ;
		}
	}
	return 0 ;
}

//return 0 nothing to be done 
// -1 nethandle closed and node freed 
// else socket in error wait to close next time
int tryto_close_tcpsession(nd_session_handle nethandle, ndtime_t connect_tmout )
{
	ENTER_FUNC()
	int ret = 0 ;
	if(TCPNODE_STATUS(nethandle)==ETS_DEAD) {
		tcp_release_death_node(nethandle,0) ;		//释放死连接
	}
	else if(TCPNODE_CHECK_RESET(nethandle)) {
		nd_session_close(nethandle,1) ;
	}
	else if(TCPNODE_CHECK_CLOSED(nethandle)) {
		nd_session_close(nethandle,0) ;
	}
	else if(check_operate_timeout(nethandle, connect_tmout)) {
		nd_object_seterror(nethandle, NDERR_TIMEOUT) ;
		nd_session_close(nethandle,0) ;
	}
	else {
		ret = 1;
	}
	LEAVE_FUNC();
	return ret ;
}



int _tcp_session_update(nd_session_handle handle)
{
	int ret = _tcpnode_push_sendbuf((struct nd_tcp_node *)handle);
	if (ret <= 0) {
		if (nd_netobj_is_alive((nd_netui_handle)handle)) {
			ndtime_t now = nd_time();
			TCPNODE_TRY_CALLBACK_WRITE(handle);
			if (now - handle->last_push > ND_ALIVE_TIMEOUT) {
				nd_sysresv_pack_t alive;
				nd_make_alive_pack(&alive);
				ret = nd_connector_send(handle, &alive.hdr, ESF_URGENCY);
			}
		}
	}
	return ret ;
}

