/* file : nd_listener.cpp
 * server listener 
 * 
 * 2009-4-25 1:14
 */

#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_srvcore/nd_srvlib.h"

#include "ndapplib/nd_listener.h"
#include "ndapplib/nd_session.h"
#include "ndapplib/nd_cmmgr.h"
//#include "pg_config.h"

static ndatomic_t __current_num = 0 ;


//通过监听句柄得到NDListener类
NDListener *NDGetListener(nd_handle h_listen) 
{
	ND_TRACE_FUNC();
	return static_cast<NDListener*>(((struct listen_contex *)h_listen)->user_data)  ;
}

//通过句柄得到绘话
NDSession *NDGetSession(nd_handle session, NDListener * Listener)
{
	ND_TRACE_FUNC();
	if (!check_connect_valid(session)){
		return NULL ;
	}
	NDObject *pobj = (NDObject *) nd_session_getdata((nd_netui_handle )session) ;
	if (pobj){
		return dynamic_cast<NDSession*>(pobj)  ;
	}
	return NULL ;
}

static int on_accept_entry(nd_handle nethandle, SOCKADDR_IN *addr, nd_handle h_listen) 
{
	ND_TRACE_FUNC();
	NDListener *pListener ;
	NDSession *newSession = NULL ;
	char buf[32] ;
	char  *pszTemp = nd_inet_ntoa( addr->sin_addr.s_addr ,buf);

	pListener =NDGetListener( h_listen)  ;
	//pListener =(NDListener *) (((struct listen_contex *)h_listen)->user_data)  ;

	int old_num ;
	((nd_netui_handle )nethandle)->level = EPL_CONNECT ;		//set privilage
	void * session_addr = nd_session_getdata((nd_netui_handle )nethandle) ;

	nd_assert(session_addr) ;
	newSession = pListener->ConstructSession(session_addr);
	newSession->Initilize(nethandle) ;
	newSession->OnCreate() ;
	
	if(-1==pListener->OnAccept(newSession, addr) ) {
		pListener->DestructSession(newSession) ;
		return -1 ;
	}
	NDSessionMgr cm_iterator(pListener) ;
	old_num = nd_atomic_read(&__current_num) ;
	nd_atomic_inc(&__current_num);

	//need to lock
	if (__current_num > pListener->m_max_onlines){
		pListener->m_max_onlines = __current_num ;
	}

	nd_logdebug(("Connect from [%s:%d]\t connection_num=%d free=%d active=%d\n") AND
		pszTemp AND htons(addr->sin_port) AND __current_num AND
		pListener->GetAllocatorFreenum() AND cm_iterator.GetActiveNum() );
	
	return 0 ;
}

static  void on_close_entry(nd_handle nethandle, nd_handle h_listen) 
{
	ND_TRACE_FUNC();
	NDListener *pListener ;

	NDSession *newSession = NULL ;
	pListener =NDGetListener( h_listen)  ;
	
	if(!nethandle|| !pListener)
		return ;

	nd_assert(((nd_netui_handle )nethandle)->level != EPL_NONE);
	NDSessionMgr cm_iterator(pListener) ;

	nd_atomic_dec(&__current_num);

	newSession = pListener->htoSession(nethandle);
	//newSession = NDGetSession(nethandle);
	newSession->OnClose() ;

	nd_logdebug(("net CLOSED error =%s\tconnect_num=%d free=%d active=%d \n") AND 
		nd_object_errordesc((nd_handle)nethandle) AND  __current_num AND 
		pListener->GetAllocatorFreenum() AND cm_iterator.GetActiveNum()-1 );
	
	((nd_netui_handle )nethandle)->level = EPL_NONE ;		//set privilage
	
	//need to lock
	nd_netui_handle netobject = (nd_netui_handle)nethandle ;
	ndtime_t onlinetm = nd_time() - netobject->start_time ;
	pListener->m_total_recv += netobject->recv_len ;
	pListener->m_total_send += netobject->send_len ;
	pListener->m_total_online += onlinetm ;

	newSession->OnDestroy() ;
	pListener->DestructSession(newSession);

}
static int on_connector_closed(nd_handle connector, int flag)
{
	ND_TRACE_FUNC();
	nd_assert(connector) ;
	NDConnector *pconn = htoConnector(connector);
	if(pconn) 
		pconn->OnClose() ;
	return 0 ;
}
//////////////////////////////////////////////////////////////////////////


NDListener::NDListener(nd_fectory_base* sf) : session_fectory(sf)
{
	m_msg_kinds = ND_DFT_MAXMSG_NUM;
	m_msg_base = 0;

	m_total_send = 0 ;		//总发送长度
	m_total_recv = 0 ;		//总接收长度
	m_total_online = 0 ;	//总在线时间
	m_max_onlines = 0 ;		//最大在线人数
	m_inst = 0 ;
}
NDListener::~NDListener() 
{
	if (session_fectory){
		session_fectory->selfdestroy();
		session_fectory = 0 ;
	}
}


NDListener &NDListener::operator = (nd_handle h)
{
	m_objhandle = h ;
	return *this ;
}



NDSession *NDListener::ConstructSession(void *addr)
{
	return static_cast<NDSession*>(session_fectory->construct(addr) );
}
void NDListener::DestructSession(NDSession *psession)
{
	return session_fectory->destruct(psession);
}

NDSession *NDListener::htoSession(nd_handle h_session) 
{
	return static_cast<NDSession*>(nd_session_getdata((nd_netui_handle )h_session))  ;
}

int NDListener::Create(const char *listen_name, int session_num, size_t session_size)
{
	ND_TRACE_FUNC() ;
	size_t ss =0;
	//open net server 
	nd_handle listen_handle = nd_object_create(listen_name) ;
	if(!listen_handle) {
		nd_logfatal((char*)"create object!\n") ;
		return -1 ;
	}

	ss = session_size + nd_getclient_hdr_size(((struct listen_contex*)listen_handle)->io_mod)  + 8;

	if(-1==nd_listensrv_session_info(listen_handle, session_num, ss) ) {
		nd_logfatal((char*)"create client map allocator!\n") ;
		return -1 ;
	}

	nd_listensrv_set_entry(listen_handle,(accept_callback)on_accept_entry,(deaccept_callback)on_close_entry) ;
	
	OnCreate() ;
	
	nd_msgtable_create(listen_handle, m_msg_kinds, m_msg_base) ;

	m_objhandle = listen_handle ;

	((struct listen_contex *)m_objhandle)->user_data = this ;
	m_session_mgr.SetMgr(nd_listensrv_get_cmmamager(listen_handle));
	return 0 ;
}


void NDListener::Destroy(int flag) 
{
	ND_TRACE_FUNC();
	
	if(m_objhandle) {
		int recv_rate = 0, send_rate =0;
		m_total_online /= 1000 ;
		if (m_total_online>0){
			recv_rate = (int)(m_total_recv / m_total_online) ;
			send_rate = (int)(m_total_send / m_total_online) ;
		}
		nd_logmsg("listener destroy\n==================================LOG NET STREAM ===================================\n"
			" max_online_num = %d \n total_send= %d total_recv=%d total_online_time = %d s\n"
			" receive_rates = %d byte/s , send_rates = %d byte/s stream = %d\n" 
			"==================================LOG NET STREAM ===================================\n"
			 AND m_max_onlines AND m_total_send AND m_total_recv  AND  m_total_online AND 
			recv_rate  AND  send_rate  AND  (recv_rate+ send_rate)) ;
		OnDestroy();
		nd_msgtable_destroy(m_objhandle,0) ;
		nd_object_destroy(m_objhandle,0) ;
		m_objhandle = 0 ;
	}

}

int NDListener::Open(int port,int thread_num)
{
	ND_TRACE_FUNC();
	nd_assert(m_objhandle) ;
	if(-1==nd_listensrv_open(port,   m_objhandle, 0, thread_num)  ) {
		nd_logfatal((char*)"open port!\n") ;
		return -1 ;
	}	
	((struct listen_contex *)m_objhandle)->tcp.msg_entry =(net_msg_entry) nd_srv_translate_message ;
	OnInitilize() ;
	return 0 ;

}

int NDListener::Close(int force)
{
	ND_TRACE_FUNC();
	if (m_objhandle && nd_listensrv_checkvalid(m_objhandle)) {
		int ret = nd_listensrv_close(m_objhandle, force);
		if(0==ret) {
			OnClose() ;
		}
		return ret;
	}	
	return -1 ;
}

int NDListener::GetAllocatorCapacity()
{
	ND_TRACE_FUNC();

	nd_assert(m_objhandle) ;
	return nd_listensrv_capacity(m_objhandle) ;
}

int NDListener::CloseAllConnects() 
{
	ND_TRACE_FUNC();

	nd_assert(m_objhandle) ;
	return nd_close_all_session(m_objhandle) ;
}
void NDListener::SetEmptyConnTimeout(int seconds) 
{
	nd_assert(m_objhandle) ;
	nd_listensrv_set_empty_conntimeout(m_objhandle,  seconds) ; 
}

void NDListener::SetAccept(int bClose) 
{
	nd_listensrv_set_accept(m_objhandle, bClose) ;
}
int NDListener::GetAllocatorFreenum()
{
	nd_assert(m_objhandle) ;
	return nd_listensrv_freenum(m_objhandle) ;

}

int NDListener::OnAccept(NDSession *pSession, SOCKADDR_IN *addr)
{
	return 0 ;
}


void NDListener::InstallMsgFunc(nd_usermsg_func func, ndmsgid_t maxid, ndmsgid_t minid,int level , const char *msgname)
{
	::nd_msgentry_install(m_objhandle, func,  maxid,  minid, level,msgname) ;
}

int NDListener::Attach(NDConnector &conn, nd_thsrvid_t thid )
{
	ND_TRACE_FUNC();
	if (m_objhandle) {
		if((NDUINT16)-1==nd_listensrv_attach(m_objhandle,conn.GetHandle(),thid) ) {
			return -1 ;
		}
		nd_listensrv_set_connector_close(m_objhandle,on_connector_closed) ;
		return 0 ;
	}
	return -1 ;
}
int NDListener::Deattach(NDConnector &conn,nd_thsrvid_t thid)
{
	ND_TRACE_FUNC();
	if (m_objhandle) {
		return nd_listensrv_deattach(m_objhandle,conn.GetHandle(), thid) ;
	}
	return -1 ;
}
ndthread_t NDListener::GetListenThid() 
{
	ND_TRACE_FUNC();
	if (m_objhandle) {
		return ((listen_contex *) m_objhandle)->listen_id ;
	}
	return 0;
}

ndthread_t NDListener::OpenListenThread(int session_num) 
{
	return nd_open_listen_thread((nd_listen_handle) GetHandle(),session_num)  ;
}
int NDListener::SwitchTothread(NDSession *session, ndthread_t aimth) 
{
	return nd_session_switch((nd_listen_handle) GetHandle(),session->GetSessionID(),  aimth);
}


NDSafeListener::NDSafeListener(nd_fectory_base *sf ) : NDListener(sf)
{

}
void NDSafeListener::Destroy(int flag) 
{
	NDListener::Destroy(flag) ;
	if (session_fectory){
		session_fectory->Destroy(flag) ;
		delete session_fectory ;
		session_fectory = 0 ;
	}

}

int NDSafeListener::OnAccept(NDSession *pSession, SOCKADDR_IN*addr)
{
	if (m_inst){
		if(!m_inst->CheckReliableHost(addr->sin_addr.s_addr) ) {
			return -1 ;
		}
	}
	return 0 ;
}

