/* file : nd_session.cpp
 * define net connection session of nd 
 * 
 * 2009-4-25 0:57
 */
#define ND_NEW_REDEFINE

#include "nd_net/nd_netlib.h"
#include "nd_srvcore/nd_srvlib.h"
#include "ndapplib/nd_session.h"
#include "ndapplib/nd_listener.h"
#include "ndapplib/nd_datatransfer.h"

//////////////////////////////////////////////////////////////////////////
//class NDSession
NDSession::NDSession () 
{
}

NDSession::~NDSession() 
{
}

#if 0
void *NDSession::operator new(size_t size, void *addr)  throw (std::bad_alloc)
{
	return addr ;
}
void NDSession::operator delete(void *p) 
{
}

void *  NDSession::operator new(size_t size) throw (std::bad_alloc) 
{
	throw std::bad_alloc() ;
}
#endif 


//send message in script
int NDSession::Send(NDOStreamMsg &omsg) 
{
	return ::nd_sessionmsg_sendex(GetHandle() , (nd_usermsghdr_t *)(omsg.GetMsgAddr()), ESF_NORMAL) ;	
// 	int ret = ::nd_sessionmsg_sendex(GetHandle() , (nd_packhdr_t *)(omsg.GetMsgAddr()), ESF_URGENCY) ;	
// 	if (ret==-1){
// 		nd_logmsg("nd_sessionmsg_sendex() send error ret = -1\n") ;
// 	}
// 	return ret ;
}

int NDSession::SendMsg(NDSendMsg &smsg, int flag)
{
	return ::nd_sessionmsg_sendex(GetHandle() , (nd_usermsghdr_t *)(smsg.GetMsgAddr()), flag) ;	
// 	int ret = ::nd_sessionmsg_sendex(GetHandle() , (nd_packhdr_t *)(smsg.GetMsgAddr()), flag) ;	
// 	if (ret==-1){
// 		nd_logmsg("nd_sessionmsg_sendex() send error ret = -1\n") ;
// 	}
// 	return ret ;
}
int NDSession::SendMsg(nd_usermsghdr_t *msghdr, int flag)
{
	return ::nd_sessionmsg_sendex(GetHandle() , msghdr, flag) ;	
// 	int ret = ::nd_sessionmsg_sendex(GetHandle() , &msghdr->packet_hdr, flag) ;	
// 	if (ret==-1){
// 		nd_logmsg("nd_sessionmsg_sendex() send error ret = -1\n") ;
// 	}
// 	return ret ;
}
int NDSession::ResendMsg(NDIStreamMsg &resendmsg, int flag)
{
	return ::nd_sessionmsg_sendex(GetHandle() , (nd_usermsghdr_t *)(resendmsg.GetMsgAddr()), flag) ;	
// 	int ret = ::nd_sessionmsg_sendex(GetHandle() , (nd_packhdr_t *)(resendmsg.GetMsgAddr()), flag) ;	
// 	if (ret==-1){
// 		nd_logmsg("nd_sessionmsg_sendex() send error ret = -1\n") ;
// 	}
// 	return ret ;
}

int NDSession::BigDataSend(int maxID, int minID, void *data, size_t datalen) 
{
	NDUINT32 hiDword = -1 ;
	NDUINT32 lowDord = ND_MAKE_DWORD(maxID, minID) ;
	NDUINT64 param = ND_MAKE_QWORD(hiDword, lowDord);
	
	return BigDataAsyncSend(m_objhandle, data, datalen,  param, NULL) ;
}

int NDSession::CryptPackage(nd_usermsgbuf_t *msgBuf) 
{	
	if (msgBuf->msg_hdr.packet_hdr.encrypt) {
		return 0;
	}
	return nd_packet_encrypt(m_objhandle, (nd_packetbuf_t*)msgBuf) > 0 ? 0 : -1 ;
	
}
//得到网络地址
const char* NDSession::GetInetAddr()
{
	static char buf[20] ;
	SOCKADDR_IN *addr =& (((nd_netui_handle)m_objhandle)->remote_addr );
	return (const char*) nd_inet_ntoa( addr->sin_addr.s_addr, buf );

}

NDUINT16 NDSession::GetSessionID() 
{
	nd_assert(m_objhandle) ;
	if (m_objhandle){
		return ((nd_netui_handle)m_objhandle)->session_id ;
	}
	return (NDUINT16)-1 ;
}
//增加使用计数
int NDSession::IncRefCount()
{
	struct nd_srv_node *root = (struct nd_srv_node *) (((nd_netui_handle)m_objhandle)->srv_root );
	if(!root) {
		return -1 ;
	}
	return root->conn_manager.inc_ref(&root->conn_manager, GetSessionID()) ;

}

//减少使用计数
void NDSession::DecRefCount()
{
	struct nd_srv_node *root = (struct nd_srv_node *) (((nd_netui_handle)m_objhandle)->srv_root );
	if(!root) {
		return  ;
	}
	root->conn_manager.dec_ref(&root->conn_manager, GetSessionID()) ;

}

int NDSession::SendRawData(void *data, size_t size) 
{
	int ret =  nd_session_raw_write(m_objhandle, data, size) ;
	if (ret==-1){
		nd_logmsg("SendRawData() send error ret = -1\n") ;
	}
	return ret ;
}

void NDSession::Initilize(nd_handle hsession,nd_handle listen)
{
	ND_TRACE_FUNC();
	m_objhandle = hsession ;
	OnInitilize() ;
}

int NDSession::Close(int flag ) 
{	
	return nd_session_close(m_objhandle, flag) ;
}
NDObject* NDSession::GetParent() 
{
	return NDGetListener(((nd_netui_handle)m_objhandle)->srv_root) ;
}

void NDSession::SetPrivilege(int level) 
{
	nd_connect_level_set(m_objhandle,(NDUINT32)level) ;
}
int NDSession::GetPrivilege() 
{
	return (int) nd_connect_level_get(m_objhandle) ;
}

ndip_t NDSession::Getip() 
{
	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
	if (fd){
		return nd_sock_getip(fd) ;
	}
	return 0 ;
}

ndport_t NDSession::GetPort() 
{
	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
	if (fd){
		return nd_sock_getport(fd) ;
	}
	return 0 ;
}

ndip_t NDSession::GetPeerip() 
{

	return nd_net_peer_getip(m_objhandle);
// 	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
// 	if (fd){
// 		return nd_sock_getpeerip(fd) ;
// 	}
// 	return 0 ;

}
ndport_t NDSession::GetPeerPort() 
{
	return nd_net_peer_getport(m_objhandle);
// 	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
// 	if (fd){
// 		return nd_sock_getpeerport(fd) ;
// 	}
// 	return 0 ;
}

int NDSession::Ioctl(int cmd, void *val, int *size) 
{
	if (m_objhandle && nd_connector_valid((nd_netui_handle)m_objhandle))	{
		return  nd_net_ioctl((nd_netui_handle)m_objhandle,  cmd, val, size) ;
	}
	return -1;
}

int NDSession::SetDelayClose() 
{
	TCPNODE_SET_RESET(m_objhandle) ;
	return 0;
}

int NDSession::LoadBalance() 
{
	NDObject *pobj = GetParent() ;
	nd_assert(pobj) ;
	return nd_session_loadbalancing((nd_listen_handle)pobj->GetHandle(), GetSessionID());
	
}


#undef ND_NEW_REDEFINE
