/* file nd_thmsg.c
 *
 * message handler of thread 
 *
 * create by duan 
 * 2011/3/3 15:30:29
 */

#include "nd_srvcore/nd_srvlib.h"

#ifdef _MSC_VER
#define check_thread_switch(lc) do {\
	if(lc->io_mod != ND_LISTEN_COMMON) {return -1;} }while(0) 
#else 
#define check_thread_switch(lc) do{} while(0) 
#endif


//Õ®π˝SESSIONid∞—œ˚œ¢∑¢ÀÕ∏¯øÕªß∂À
int nd_send_toclient_ex(NDUINT16 sessionid,nd_usermsghdr_t *data, nd_handle listen_handle,int encrypt) 
{
	int ret =0 ;
	ndthread_t thid ;
	struct cm_manager *pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)listen_handle) ;	
	
	//check_thread_switch(lc) ;
	
	if(!pmanger ) {
		return -1 ;
	}
	else if(0==sessionid) {
		return nd_sendto_all_ex(data, listen_handle, 0,encrypt) ;
	}
	
	thid = nd_node_get_owner(pmanger,sessionid ) ;
	if(0==thid) {
		return -1 ;
	}
	if(thid == nd_thread_self() ) {
		int flag = encrypt ? (ESF_ENCRYPT|ESF_NORMAL): ESF_NORMAL ;
		nd_handle client = (nd_handle) pmanger->lock(pmanger,sessionid) ;
		if(!client) 
			return -1 ;
		ret = nd_sessionmsg_sendex(client,data, flag) ;
		pmanger->unlock(pmanger,sessionid);
	}
	else {
		NDUINT32  size = ND_USERMSG_LEN(data) ;
		if(size==0 || size>ND_PACKET_SIZE) {
			return -1 ;
		}
		ND_USERMSG_LEN(data) = sessionid ;
		if (encrypt) {			
			data->packet_hdr.encrypt = 1 ;
		}

		ret = nd_thsrv_send(thid,E_THMSGID_SENDTO_CLIENT,data, size) ;
		
		data->packet_hdr.encrypt = 0 ;		
		ND_USERMSG_LEN(data) = size ;
	}
	return ret ;
}

int nd_sendto_all_ex(nd_usermsghdr_t *data, nd_handle listen_handle,int priv_level,int encrypt)
{
	NDUINT8 ver ;
	int ret = 0;	
	NDUINT32  size = ND_USERMSG_LEN(data) ;
	
	struct list_head *pos ;
	struct cm_manager *pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)listen_handle) ;	
	struct listen_contex *lc = (struct listen_contex *)listen_handle ;
	
	if(pmanger->connect_num < 1 ) { //low requirment 
		return 0 ;
	}
	
	if(size==0 || size>ND_PACKET_SIZE) {
		return -1 ;
	}
#ifndef ND_UNIX
	if (lc->io_mod == ND_LISTEN_OS_EXT) {	
		int flag = encrypt ? (ESF_ENCRYPT|ESF_POST): ESF_POST ;	
		nd_netui_handle client ;
		cmlist_iterator_t cm_iterator ;				
		for(client = pmanger->lock_first (pmanger,&cm_iterator) ; client; 
			client = pmanger->lock_next (pmanger,&cm_iterator) ) {
			
			if (nd_connect_level_get(client)>= priv_level )	{
				nd_sessionmsg_sendex(client,data, flag) ;
			}
			++ret ;
		}
		return  ret; 
	}
#endif
	
	ND_USERMSG_LEN(data) = 0 ;
	ver = data->packet_hdr.version ;
	data->packet_hdr.version = priv_level ;
	if (encrypt) {		
		data->packet_hdr.encrypt = 1 ;
	}
	
	pos = lc->list_thread.next ;	
	while(pos != &lc->list_thread) {
		struct thread_pool_info  *piocp  = list_entry(pos,struct thread_pool_info,list) ;
		pos = pos->next ;
		
		nd_thsrv_send(piocp->thid,E_THMSGID_SENDTO_CLIENT,data, size) ;
		++ret ;
	}
	
	ND_USERMSG_LEN(data) = size ;
	data->packet_hdr.version = ver;
	data->packet_hdr.encrypt = 0;
	
	return ret ;
}

//∞—œ˚œ¢Ωª∏¯session¥¶¿Ì
int nd_netmsg_handle(NDUINT16 sessionid,nd_usermsghdr_t *data, nd_handle listen_handle) 
{	
	NDUINT8 tmp,ver ;
	int ret = -1;
	
	ndthread_t thid ;
	struct listen_contex *lc = (struct listen_contex *)listen_handle ;

	struct cm_manager *pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)listen_handle) ;	
	NDUINT32  size = ND_USERMSG_LEN(data) ;
	
	if(size==0 || size>ND_PACKET_SIZE) 
		return -1 ;
	
	//check_thread_switch(lc) ;
	
	if(!pmanger) 
		return -1 ;
	else if(0==sessionid) {
		return nd_netmsg_2all_handle(data, listen_handle, 0) ;
	}
	
	thid = nd_node_get_owner(pmanger,sessionid ) ;
	
	if(0==thid) 
		return -1 ;
	
	tmp = data->packet_hdr.ndsys_msg ;
	data->packet_hdr.ndsys_msg =1 ;
		
	if(thid == nd_thread_self() ) {
		nd_netui_handle  client = (nd_netui_handle) pmanger->lock(pmanger,sessionid) ;
		if(client) {			
			ret = client->msg_entry((nd_handle)client,(nd_packhdr_t *) data, listen_handle) ; 
			pmanger->unlock(pmanger,sessionid);
		}		
	}
	else {	
		
		ND_USERMSG_LEN(data) = sessionid ;	
		ver = data->packet_hdr.version ;		
		data->packet_hdr.version = 0 ;
		
		ret = nd_thsrv_send(thid,E_THMSGID_NETMSG_HANDLE,data, size) ;
		
		ND_USERMSG_LEN(data) = size ;
		data->packet_hdr.version =ver ;
	}
	
	data->packet_hdr.ndsys_msg = tmp ;
	
	return ret ;

}

int nd_netmsg_2all_handle(nd_usermsghdr_t *data, nd_handle listen_handle,int priv_level) 
{	
	NDUINT8 tmp,ver ;
	int ret = 0;	
	NDUINT32  size = ND_USERMSG_LEN(data) ;
	
	struct list_head *pos ;
	struct cm_manager *pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)listen_handle) ;	
	struct listen_contex *lc = (struct listen_contex *)listen_handle ;
	
	if(pmanger->connect_num < 1 ) { //low requirment 
		return 0 ;
	}
	
	if(size==0 || size>ND_PACKET_SIZE) {
		return -1 ;
	}	
		
#ifndef ND_UNIX
	if (lc->io_mod == ND_LISTEN_OS_EXT) {		
		nd_netui_handle client ;
		cmlist_iterator_t cm_iterator ;
		NDUINT8 tmp = data->packet_hdr.ndsys_msg ;
		data->packet_hdr.ndsys_msg =1 ;
			
		for(client = pmanger->lock_first (pmanger,&cm_iterator) ; client; 
			client = pmanger->lock_next (pmanger,&cm_iterator) ) {
			client->msg_entry((nd_handle)client,(nd_packhdr_t *) data, listen_handle) ; 
			++ret ;
		}		
		data->packet_hdr.ndsys_msg = tmp ;
		return  ret; 
	}
#endif
	
	
	ND_USERMSG_LEN(data) = 0 ;	
	ver = data->packet_hdr.version ;		
	data->packet_hdr.version = priv_level ;	
	tmp = data->packet_hdr.ndsys_msg ;
	data->packet_hdr.ndsys_msg =1 ;
	
	pos = lc->list_thread.next ;	
	while(pos != &lc->list_thread) {
		struct thread_pool_info  *piocp  = list_entry(pos,struct thread_pool_info,list) ;
		pos = pos->next ;
		
		nd_thsrv_send(piocp->thid,E_THMSGID_NETMSG_HANDLE,data, size) ;			
		++ret ;
	}	
	ND_USERMSG_LEN(data) = size ;
	data->packet_hdr.ndsys_msg = tmp ;
	data->packet_hdr.version =ver ;
	return ret ;

}

//∞—sessionÃÌº”µΩ∆‰À˚œﬂ≥Ã
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
	//struct listen_contex *listen_info = (struct listen_contex *) h ;

	ndthread_t thid = nd_thread_self() ;

	struct thread_pool_info *pthinfo ;
	struct cm_manager *pmanger;
	struct nd_client_map *client;

	nd_assert(h) ;
	nd_assert(sessionid) ;
	nd_assert(aimid);
	
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
//œ˚œ¢¥¶¿Ì∫Ø ˝
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
		return -1 ;//»Áπ˚≤ªƒ‹lockæÕ±£¡Ù“ªœ¬¥Àœ˚œ¢
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
	NDUINT8 iscrypt = 0 ;
	NDUINT8 priv_level = 0 ;
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
	
	priv_level = net_msg->msg_hdr.packet_hdr.version ;
	net_msg->msg_hdr.packet_hdr.version = NDNETMSG_VERSION ;
	
	iscrypt = net_msg->msg_hdr.packet_hdr.encrypt ;
	net_msg->msg_hdr.packet_hdr.encrypt = 0 ;
	
	if (session_id){
		int flag = iscrypt?  (ESF_NORMAL | ESF_ENCRYPT) : ESF_NORMAL;
		client = (nd_netui_handle) pmanger->lock(pmanger,session_id) ;
		
		if(client) {
			nd_sessionmsg_sendex((nd_handle)client,&net_msg->msg_hdr,flag) ;
			pmanger->unlock(pmanger,session_id);			
		}
	}
	else {
		nd_handle client;
		int flag = iscrypt?  ESF_POST: (ESF_POST | ESF_ENCRYPT) ;
		
#ifndef ND_UNIX
		if (lc->io_mod == ND_LISTEN_OS_EXT) {
			cmlist_iterator_t cm_iterator ;
			for(client = pmanger->lock_first (pmanger,&cm_iterator) ; client; 
				client = pmanger->lock_next (pmanger,&cm_iterator) ) {
				if (nd_connect_level_get(client)>= priv_level )	{
					nd_sessionmsg_sendex((nd_handle)client,&net_msg->msg_hdr,flag) ;
				}
			}
		}
		else 
#endif
		if(pthinfo){
			int i;
			for (i=pthinfo->session_num-1; i>=0;i-- ) {
				NDUINT16 session_id = pthinfo->sid_buf[i];
				client =(nd_handle) pmanger->lock(pmanger,session_id) ;
				if (!client)
					continue ;
				if (nd_connect_level_get(client)>= priv_level )	{
					nd_sessionmsg_sendex((nd_handle)client,&net_msg->msg_hdr,flag) ;
				}
				pmanger->unlock(pmanger,session_id) ;
			}
		}	
	}	
	
	net_msg->msg_hdr.packet_hdr.version = priv_level;	
	ND_USERMSG_LEN(net_msg) = session_id;
	net_msg->msg_hdr.packet_hdr.encrypt = iscrypt ;
	
	return 0 ;
}

int netmsg_recv_handler(nd_thsrv_msg *msg)
{
	NDUINT8 tmp,priv_level =0;
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
	
	priv_level = net_msg->msg_hdr.packet_hdr.version ;
	net_msg->msg_hdr.packet_hdr.version = NDNETMSG_VERSION ;
	
	session_id = ND_USERMSG_LEN(net_msg) ;	
	ND_USERMSG_LEN(net_msg) = (NDUINT16) msg->data_len ;
	
	tmp = net_msg->msg_hdr.packet_hdr.ndsys_msg ;
	net_msg->msg_hdr.packet_hdr.ndsys_msg =1 ;
	
	if(session_id) {
		client = (nd_netui_handle) pmanger->lock(pmanger,session_id) ;
		if(client) {
			client->msg_entry((nd_handle)client,(nd_packhdr_t*)net_msg, (nd_handle)lc) ;			
		}
		pmanger->unlock(pmanger,session_id);	
	}
	else {
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
	
	net_msg->msg_hdr.packet_hdr.version = priv_level ;		
	ND_USERMSG_LEN(net_msg) = session_id ;	
	net_msg->msg_hdr.packet_hdr.ndsys_msg = tmp;
	
	return 0;
}

void init_netthread_msg( nd_handle  thhandle)
{
	nd_thsrv_install_msg(  thhandle,E_THMSGID_ADDTO_THREAD, session_add_handler ) ;	
	nd_thsrv_install_msg(  thhandle,E_THMSGID_SENDTO_CLIENT, msg_sendto_client_handler ) ;	
	nd_thsrv_install_msg(  thhandle,E_THMSGID_NETMSG_HANDLE, netmsg_recv_handler ) ;
	nd_thsrv_install_msg(  thhandle,E_THMSGID_CLOSE_SESSION, session_close_handler ) ;
}

