/* file nd_thmsg.c
 *
 * message handler of thread 
 *
 * create by duan 
 * 2011/3/3 15:30:29
 */

// #include "nd_common/nd_common.h"
#include "nd_srvcore/nd_srvlib.h"
// #include "nd_net/nd_netlib.h"
// #include "nd_common/nd_alloc.h"
// #include "nd_srvcore/nd_thpool.h"

#ifdef _MSC_VER
#define check_thread_switch(lc) do {\
	if(lc->io_mod != ND_LISTEN_COMMON) {return -1;} }while(0) 
#else 
#define check_thread_switch(lc) do{} while(0) 
#endif

//通过SESSIONid把消息发送给客户端
int nd_send_tocliet(NDUINT16 sessionid,nd_usermsghdr_t *data, nd_handle listen_handle) 
{
	ndthread_t thid ;
	struct listen_contex *lc = (struct listen_contex *)listen_handle ;

	struct cm_manager *pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)listen_handle) ;	
	
	check_thread_switch(lc) ;
	
	if(!pmanger||sessionid==0 ) 
		return -1 ;
	thid = nd_node_get_owner(pmanger,sessionid ) ;
	if(0==thid) 
		return -1 ;
	if(thid == nd_thread_self() ) {
		int ret ;
		nd_handle client = (nd_handle) pmanger->lock(pmanger,sessionid) ;
		if(!client) 
			return -1 ;
		ret = nd_sessionmsg_send(client,data) ;
		pmanger->unlock(pmanger,sessionid);
		return ret ;
	}
	else {
		NDUINT32  size = ND_USERMSG_LEN(data) ;
		if(size==0 || size>ND_PACKET_SIZE) {
			return -1 ;
		}
		ND_USERMSG_LEN(data) = sessionid ;

		return nd_thsrv_send(thid,E_THMSGID_SENDTO_CLIENT,data, size) ;
	}
}

//把消息交给session处理
int nd_netmsg_handle(NDUINT16 sessionid,nd_usermsghdr_t *data, nd_handle listen_handle) 
{
	ndthread_t thid ;
	struct listen_contex *lc = (struct listen_contex *)listen_handle ;

	struct cm_manager *pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)listen_handle) ;	
	
	check_thread_switch(lc) ;
	
	if(!pmanger) 
		return -1 ;
	thid = nd_node_get_owner(pmanger,sessionid ) ;
	if(0==thid) 
		return -1 ;
	if(thid == nd_thread_self() ) {
		NDUINT8 tmp ;
		int ret ;
		nd_netui_handle  client = (nd_netui_handle) pmanger->lock(pmanger,sessionid) ;
		if(!client) 
			return -1 ;
		//ret = client->msg_entry((nd_handle)client, data, ND_USERMSG_LEN(data), listen_handle) ;
		tmp = data->packet_hdr.ndsys_msg ;
		data->packet_hdr.ndsys_msg =1 ;
		ret = client->msg_entry((nd_handle)client,(nd_packhdr_t *) data, listen_handle) ; 
		data->packet_hdr.ndsys_msg = tmp ;
		pmanger->unlock(pmanger,sessionid);
		return ret ;
	}
	else {
		NDUINT32  size = ND_USERMSG_LEN(data) ;
		if(size==0 || size>ND_PACKET_SIZE) 
			return -1 ;
	
		ND_USERMSG_LEN(data) = sessionid ;
		return nd_thsrv_send(thid,E_THMSGID_NETMSG_HANDLE,data, size) ;
	}

}

//把session添加到其他线程
int _session_addto(NDUINT16 sessionid, nd_handle listen_handle,ndthread_t thid) 
{
	struct listen_contex *lc = (struct listen_contex *)listen_handle ;
	nd_netui_handle client ;
	struct cm_manager *pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)listen_handle) ;	
	
	check_thread_switch(lc) ;
	
	if(!pmanger) 
		return -1 ;
	if(thid == nd_thread_self())
		return -1;

	client = (nd_netui_handle) pmanger->lock(pmanger,sessionid) ;
	if(!client) {
		return -1 ;
	}
	nd_node_set_owner(pmanger,sessionid,thid) ;
	pmanger->unlock(pmanger,sessionid);

	nd_thsrv_send(thid,E_THMSGID_ADDTO_THREAD,&sessionid, sizeof(sessionid)) ;
	return 0;

}
int _session_delfrom(NDUINT16 sessionid, nd_handle listen_handle,ndthread_t thid)
{
	nd_thsrv_send(thid,	E_THMSGID_DELFROM_THREAD,&sessionid, sizeof(sessionid)) ;
	return 0 ;
}
int nd_session_switch(nd_listen_handle h,NDUINT16 sessionid, nd_thsrvid_t aimid)
{
	int ret ;
	struct listen_contex *listen_info = (struct listen_contex *) h ;

	ndthread_t thid = nd_thread_self() ;

	struct thread_pool_info *pthinfo ;
	struct cm_manager *pmanger;
	struct nd_client_map *client;

	if (thid == aimid) {
		return 0 ;
	}
	pthinfo = get_thread_poolinf(h,thid) ;
	if(!pthinfo) {
		return -1 ;
	}
	
	pmanger = nd_listensrv_get_cmmamager(h) ;
	if(!pmanger) 
		return -1 ;	
	client = (struct nd_client_map *) pmanger->lock(pmanger,sessionid) ;
	if(!client) {
		return -1 ;
	}
	ret = delfrom_thread_pool(client,pthinfo) ;
	pmanger->unlock(pmanger,sessionid);
	if (ret==-1)
		return -1;

	if (aimid){
		return _session_addto(sessionid, (nd_handle) h, aimid) ; 
	}
	else 
		return 0;
}

//////////////////////////////////////////////////////////////////////////
//消息处理函数
int session_add_handler(nd_thsrv_msg *msg)
{
	NDUINT16 sessionid;
	nd_netui_handle client ;
	struct cm_manager *pmanger;
	struct thread_pool_info *pthinfo =(struct thread_pool_info *) msg->th_userdata ;

	struct listen_contex *lc = (struct listen_contex *)pthinfo->lh ;
	
	sessionid = *(NDUINT16*)(msg->data);

	pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)lc) ;	
	if(!pmanger) 
		return 0 ;	
	client = (nd_netui_handle) pmanger->trylock(pmanger,sessionid) ;
	if(!client) {
		return -1 ;//如果不能lock就保留一下此消息
	}	
	pmanger->unlock(pmanger,sessionid);
	addto_thread_pool((struct nd_client_map *)client,pthinfo) ;
	return 0;
}

int session_deattach_handler(nd_thsrv_msg *msg)
{
	NDUINT16 sessionid;
	nd_netui_handle client ;
	struct cm_manager *pmanger;
	struct thread_pool_info *pthinfo =(struct thread_pool_info *) msg->th_userdata ;

	struct listen_contex *lc = (struct listen_contex *)pthinfo->lh ;

	sessionid = *(NDUINT16*)(msg->data);

	pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)lc) ;	
	if(!pmanger) 
		return 0 ;	
	client = (nd_netui_handle) pmanger->lock(pmanger,sessionid) ;
	if(!client) {
		return 0 ;
	}	
	pmanger->unlock(pmanger,sessionid);
	delfrom_thread_pool((struct nd_client_map *)client,pthinfo) ;
	return 0;
}

int session_close_handler(nd_thsrv_msg *msg)
{
	NDUINT16 sessionid;
	nd_netui_handle client ;
	struct cm_manager *pmanger;
	struct thread_pool_info *pthinfo =(struct thread_pool_info *) msg->th_userdata ;

	struct listen_contex *lc = (struct listen_contex *)pthinfo->lh ;

	sessionid = *(NDUINT16*)(msg->data);

	pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)lc) ;	
	if(!pmanger) 
		return 0 ;	
	client = (nd_netui_handle) pmanger->lock(pmanger,sessionid) ;
	if(!client) {
		return 0 ;
	}
	nd_session_close((nd_handle)client,0) ;
	pmanger->unlock(pmanger,sessionid);
	return 0;
}

int msg_sendto_client_handler(nd_thsrv_msg *msg)
{
	NDUINT16  session_id  ;
	nd_netui_handle client ;

	struct thread_pool_info *pthinfo =(struct thread_pool_info *) msg->th_userdata ;
	struct listen_contex *lc = (struct listen_contex *)pthinfo->lh ;

	struct cm_manager *pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)lc) ;		
	nd_usermsgbuf_t *net_msg ;

	if(!pmanger) 
		return 0 ;

	if(msg->data_len < ND_USERMSG_HDRLEN) {
		return 0;
	}
	net_msg = (nd_usermsgbuf_t *)msg->data ;

	session_id = ND_USERMSG_LEN(net_msg) ;
	ND_USERMSG_LEN(net_msg) = (NDUINT16) msg->data_len ;
	
	if (session_id){
		client = (nd_netui_handle) pmanger->lock(pmanger,session_id) ;
		if(!client) 
			return 0 ;
		nd_sessionmsg_send(client,net_msg) ;
		pmanger->unlock(pmanger,session_id);
	}
	else {
		nd_handle client;
		NDUINT8 priv_level = net_msg->msg_hdr.packet_hdr.version ;
		net_msg->msg_hdr.packet_hdr.version = NDNETMSG_VERSION ;

		if (lc->io_mod == ND_LISTEN_COMMON)	{
			int i;
			for (i=pthinfo->session_num-1; i>=0;i-- ) {
				NDUINT16 session_id = pthinfo->sid_buf[i];
				client =(nd_handle) pmanger->lock(pmanger,session_id) ;
				if (!client)
					continue ;
				if (nd_connect_level_get(client)>= priv_level )	{
					nd_sessionmsg_post(client,net_msg) ;
				}
				pmanger->unlock(pmanger,session_id) ;
			}
		}
		else {
			cmlist_iterator_t cm_iterator ;
			for(client = pmanger->lock_first (pmanger,&cm_iterator) ; client; 
				client = pmanger->lock_next (pmanger,&cm_iterator) ) {
					if (nd_connect_level_get(client)>= priv_level )	{
						nd_sessionmsg_post(client,net_msg) ;
					}
			}
		}
		net_msg->msg_hdr.packet_hdr.version = priv_level;		
	}	
	return 0 ;
}

int netmsg_recv_handler(nd_thsrv_msg *msg)
{
	NDUINT8 tmp;
	NDUINT16  session_id  ;
	nd_netui_handle client ;

	struct thread_pool_info *pthinfo =(struct thread_pool_info *) msg->th_userdata ;
	struct listen_contex *lc = (struct listen_contex *)pthinfo->lh ;

	struct cm_manager *pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)lc) ;		
	nd_usermsgbuf_t *net_msg ;
	
	if(!pmanger) 
		return -1 ;

	if(msg->data_len < ND_USERMSG_HDRLEN) {
		return 0;
	}
	net_msg = (nd_usermsgbuf_t *)msg->data ;

	session_id = ND_USERMSG_LEN(net_msg) ;
	
	ND_USERMSG_LEN(net_msg) = (NDUINT16) msg->data_len ;
	client = (nd_netui_handle) pmanger->lock(pmanger,session_id) ;
	if(!client) 
		return 0 ;
	tmp = net_msg->msg_hdr.packet_hdr.ndsys_msg ;
	net_msg->msg_hdr.packet_hdr.ndsys_msg =1 ;
	client->msg_entry((nd_handle)client,(nd_packhdr_t*)net_msg, (nd_handle)lc) ;
	net_msg->msg_hdr.packet_hdr.ndsys_msg = tmp;
	//client->data_entry((nd_handle)client, net_msg, msg->data_len,(nd_handle)lc) ;
	pmanger->unlock(pmanger,session_id);
	
	return 0;
}

void init_netthread_msg( nd_handle  thhandle)
{
	nd_thsrv_install_msg(  thhandle,E_THMSGID_ADDTO_THREAD, session_add_handler ) ;	
	nd_thsrv_install_msg(  thhandle,E_THMSGID_SENDTO_CLIENT, msg_sendto_client_handler ) ;	
	nd_thsrv_install_msg(  thhandle,E_THMSGID_NETMSG_HANDLE, netmsg_recv_handler ) ;
	nd_thsrv_install_msg(  thhandle,E_THMSGID_CLOSE_SESSION, session_close_handler ) ;
}

