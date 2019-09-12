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
#include "ndapplib/nd_connector.h"
//#include "pg_config.h"

static ndatomic_t __current_num = 0 ;


//通过监听句柄得到NDListener类
//NDListener *NDGetListener(nd_handle h_listen)
//{
//	ND_TRACE_FUNC();
//	if (h_listen) {
//		return static_cast<NDListener*>(((struct nd_srv_node *)h_listen)->user_data);
//	}
//	return NULL;
//}

//通过句柄得到绘话
//NDBaseSession *NDGetSession(nd_handle session, NDListener * Listener)
//{
//	ND_TRACE_FUNC();
//	if (!session /*|| !check_connect_valid(session)*/){
//		return NULL ;
//	}
//	NDObject *pobj = (NDObject *) nd_session_getdata((nd_netui_handle )session) ;
//	if (pobj){
//		return dynamic_cast<NDBaseSession*>(pobj);
//	}
//	return NULL ;
//}
static  int _check_session_valid(nd_handle session)
{
	NDObject *pobj = (NDObject *)nd_session_getdata((nd_handle)session);
	if (pobj){
		if (dynamic_cast<NDBaseSession*>(pobj)) {
			return 1;
		}
	}
	return 0;	
}

static int on_accept_entry(nd_handle nethandle, SOCKADDR_IN *addr, nd_handle h_listen) 
{
	ND_TRACE_FUNC();
	NDListener *pListener ;
	NDBaseSession *newSession = NULL;
	char buf[64] ;
	const char  *pszTemp= inet_ntop(addr->sin_family, &addr->sin_addr, buf, sizeof(buf));

	pListener = (NDListener *)NDObject::FromHandle( h_listen)  ;

	int old_num ;
	((nd_netui_handle )nethandle)->level = EPL_CONNECT ;		//set privilage
	void * session_addr = nd_session_getdata(nethandle) ;

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

	NDBaseSession *newSession = NULL;
	//pListener =NDGetListener( h_listen)  ;
	
	pListener = (NDListener *)NDObject::FromHandle( h_listen)  ;
	
	if(!nethandle|| !pListener)
		return ;

	nd_assert(((nd_netui_handle )nethandle)->level != EPL_NONE);
	NDSessionMgr cm_iterator(pListener) ;

	nd_atomic_dec(&__current_num);

	newSession = pListener->htoSession(nethandle);
	//newSession = NDGetSession(nethandle);
	newSession->OnClose() ;

	//nd_assert(NDERR_WOULD_BLOCK != nd_object_lasterror(nethandle));
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
	//NDConnector *pconn = htoConnector(connector);
	NDObject *pconn = NDObject::FromHandle(connector);
	if(pconn) 
		pconn->OnClose() ;
	return 0 ;
}
//////////////////////////////////////////////////////////////////////////


NDListener::NDListener(nd_fectory_base* sf) : session_fectory(sf)
{
	m_msg_kinds = ND_MAIN_MSG_CAPACITY;
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



NDBaseSession *NDListener::ConstructSession(void *addr)
{
	return static_cast<NDBaseSession*>(session_fectory->construct(addr));
}
void NDListener::DestructSession(NDBaseSession *psession)
{
	return session_fectory->destruct(psession);
}


NDBaseSession *NDListener::GetSession(NDUINT16 sessionId)
{
	NDSessionMgr tmpmgr(this) ;
	return tmpmgr.TryLock(sessionId) ;
}

NDBaseSession *NDListener::htoSession(nd_handle h_session)
{
	return static_cast<NDBaseSession*>(nd_session_getdata(h_session));
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

	ss = session_size + nd_session_hdr_size(((struct listen_contex*)listen_handle)->io_mod)  + 8;

	if(-1==nd_listener_set_capacity(listen_handle, session_num, ss) ) {
		nd_logfatal((char*)"create client map allocator!\n") ;
		return -1 ;
	}

	nd_listener_set_callback(listen_handle,(accept_callback)on_accept_entry,(deaccept_callback)on_close_entry) ;
	nd_listener_set_valid_func(listen_handle, _check_session_valid);
	
	OnCreate() ;
	
	nd_msgtable_create(listen_handle, m_msg_kinds, m_msg_base) ;

	m_objhandle = listen_handle ;

	//((struct nd_srv_node *)m_objhandle)->user_data = this ;
	((struct nd_srv_node *)m_objhandle)->msg_caller = this ;
	m_session_mgr.SetMgr(nd_listener_get_session_mgr(listen_handle));
	
	nd_logmsg("Create %s listen object success %d sessions in buffer list \n", listen_name, session_num);
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

int NDListener::Open(int port, int thNum, const char *bindIp , int isIpv6)
{
	ND_TRACE_FUNC();
	nd_assert(m_objhandle) ;
	if(-1==nd_listener_open(isIpv6, port, m_objhandle, thNum,bindIp)  ) {
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
	if (m_objhandle && nd_listener_checkvalid(m_objhandle)) {
		int ret = nd_listener_close(m_objhandle, force);
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
	return nd_listener_get_capacity(m_objhandle) ;
}

int NDListener::CloseAllConnects() 
{
	ND_TRACE_FUNC();

	nd_assert(m_objhandle) ;
	return nd_listener_close_all(m_objhandle) ;
}
void NDListener::SetEmptyConnTimeout(int seconds) 
{
	nd_assert(m_objhandle) ;
	nd_listener_set_empty_timeout(m_objhandle,  seconds) ; 
}

void NDListener::SetAccept(int bClose) 
{
	nd_listener_set_accept(m_objhandle, bClose) ;
}
int NDListener::GetAllocatorFreenum()
{
	nd_assert(m_objhandle) ;
	return nd_listener_freenum(m_objhandle) ;

}

int NDListener::OnAccept(NDBaseSession *pSession, SOCKADDR_IN *addr)
{
	return 0 ;
}


void NDListener::InstallMsgFunc( nd_conn_msg_entry func, ndmsgid_t maxid, ndmsgid_t minid,int level , const char *msgname)
{
	::nd_msgentry_install(m_objhandle,(nd_usermsg_func) func,  maxid,  minid, level,msgname) ;
}
void NDListener::setScriptEngine(void *script_engine, nd_msg_script_entry entry)
{
	::nd_message_set_script_engine(m_objhandle, script_engine, entry);
}

int NDListener::Attach(NDObject &conn, nd_thsrvid_t thid )
{
	ND_TRACE_FUNC();
	if (m_objhandle) {
		if((NDUINT16)-1==nd_listener_attach(m_objhandle,conn.GetHandle(),thid) ) {
			return -1 ;
		}
		nd_listener_set_connector_close(m_objhandle,on_connector_closed) ;
		return 0 ;
	}
	return -1 ;
}
int NDListener::Deattach(NDObject &conn,nd_thsrvid_t thid)
{
	ND_TRACE_FUNC();
	if (m_objhandle) {
		return nd_listener_deattach(m_objhandle,conn.GetHandle(), thid) ;
	}
	return -1 ;
}


int NDListener::AttachPort(int port, const char* bindIP)
{
	if(-1==nd_listener_add_port((nd_listen_handle) m_objhandle, port, bindIP) ) {
		return -1 ;
	}
	return 0;
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
	return nd_listener_thread_create((nd_listen_handle) GetHandle(),session_num)  ;
}
int NDListener::SwitchTothread(NDBaseSession *session, ndthread_t aimth)
{
	return nd_session_switch((nd_listen_handle) GetHandle(),session->GetSessionID(),  aimth);
}

int NDListener::GetClientsInThreads(ndthread_t *threadid_buf, int *count_buf, int size) 
{
	return nd_listener_fetch_sessions((nd_listen_handle)GetHandle(), threadid_buf, count_buf, size) ;
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

int NDSafeListener::OnAccept(NDBaseSession *pSession, SOCKADDR_IN*addr)
{
	if (m_inst){
		ndip_t peerIp = ND_IP_INIT;
		if (0 == nd_sockadd_to_ndip(addr, &peerIp)) {
			if (!m_inst->CheckReliableHost(peerIp)) {
				char  peer_buf[64];
				nd_logwarn("[IP FORBID ] closed connection from %s unreliabled ip\n", ND_INET_NTOA(peerIp, peer_buf));
				return -1;
			}
		}
		else {
			return -1;
		}
		
	}
	return 0 ;
}

