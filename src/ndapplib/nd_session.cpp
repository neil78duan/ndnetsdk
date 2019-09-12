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
//#include "ndapplib/nd_datatransfer.h"
#include "nd_msg.h"

static  int _session_update(nd_handle h)
{
	NDBaseSession *pSe = dynamic_cast<NDBaseSession*>( NDObject::FromHandle(h));
	if (pSe) {
		pSe->baseUpdate() ;
	}
	return  0;
}

//////////////////////////////////////////////////////////////////////////

NDBaseSession::NDBaseSession()
{

	m_handle_update = 0;
}

NDBaseSession::~NDBaseSession()
{
	if (m_objhandle &&m_handle_update) {
		((nd_netui_handle)m_objhandle)->update_entry = m_handle_update;
		m_handle_update = 0;
	}
	NDAlarm::Destroy();
}


void NDBaseSession::baseUpdate()
{
	if (m_handle_update) {
		m_handle_update(m_objhandle);
	}
	NDAlarm::Update();
}

int NDBaseSession::sendPack(nd_packhdr_t *msghdr, int flag)
{
	return ::nd_sessionmsg_sendex(GetHandle() ,(nd_usermsghdr_t*)msghdr, flag) ;
}

//
//int NDBaseSession::BigDataSend(int maxID, int minID, void *data, size_t datalen)
//{
//	NDUINT32 hiDword = -1;
//	NDUINT32 lowDord = ND_MAKE_DWORD(maxID, minID);
//	NDUINT64 param = ND_MAKE_QWORD(hiDword, lowDord);
//
//	return BigDataAsyncSend(m_objhandle, data, datalen, param, NULL);
//}
//
//int NDBaseSession::CryptPackage(nd_usermsgbuf_t *msgBuf)
//{
//	if (msgBuf->msg_hdr.packet_hdr.encrypt) {
//		return 0;
//	}
//	return nd_packet_encrypt(m_objhandle, (nd_packetbuf_t*)msgBuf) > 0 ? 0 : -1;
//
//}
//
//NDUINT16 NDBaseSession::GetSessionID()
//{
//	nd_assert(m_objhandle);
//	if (m_objhandle){
//		return ((nd_netui_handle)m_objhandle)->session_id;
//	}
//	return (NDUINT16)-1;
//}
//增加使用计数
int NDBaseSession::IncRefCount()
{
	struct nd_srv_node *root = (struct nd_srv_node *) (((nd_netui_handle)m_objhandle)->srv_root);
	if (!root) {
		return -1;
	}
	return root->conn_manager.inc_ref(&root->conn_manager, GetSessionID());

}

//减少使用计数
void NDBaseSession::DecRefCount()
{
	struct nd_srv_node *root = (struct nd_srv_node *) (((nd_netui_handle)m_objhandle)->srv_root);
	if (!root) {
		return;
	}
	root->conn_manager.dec_ref(&root->conn_manager, GetSessionID());

}
//
//int NDBaseSession::SendRawData(void *data, size_t size)
//{
//	int ret = nd_session_raw_write(m_objhandle, data, size);
//	if (ret == -1){
//		nd_logmsg("SendRawData() send error ret = -1\n");
//	}
//	return ret;
//}
//
//bool NDBaseSession::FlushSendBuf(bool bForce)
//{
//	if (bForce) {
//		return nd_tcpnode_flush_sendbuf_force((nd_netui_handle)GetHandle()) > 0;
//	}
//	else {
//		return nd_tcpnode_flush_sendbuf((nd_netui_handle)GetHandle()) > 0;
//	}
//}

void NDBaseSession::Initilize(nd_handle hsession, nd_handle listen)
{
	ND_TRACE_FUNC();
	m_objhandle = hsession;
	nd_netui_handle hs =  (nd_netui_handle)hsession ;
	m_handle_update = hs->update_entry;
	hs->update_entry = _session_update;
	
	//hs->user_data = this;
	hs->msg_caller = this;
	
	
	NDAlarm::Create();
	OnInitilize();
}

int NDBaseSession::Close(int flag)
{
	if (m_handle_update) {
		((nd_netui_handle)m_objhandle)->update_entry = m_handle_update;
		m_handle_update = 0;
	}
	NDAlarm::Destroy(flag);
	return nd_session_close(m_objhandle, flag);
}
NDObject* NDBaseSession::GetParent()
{
	return GetListener() ;
	//return NDGetListener(((nd_netui_handle)m_objhandle)->srv_root);
}

NDListener* NDBaseSession::GetListener()
{
	nd_handle hl = nd_session_getlisten( m_objhandle);
	if(!hl){
		return NULL;
	}
	return dynamic_cast<NDListener*>(NDObject::FromHandle(hl)) ;
}

//
//void NDBaseSession::SetPrivilege(int level)
//{
//	nd_connect_level_set(m_objhandle, (NDUINT32)level);
//}
//int NDBaseSession::GetPrivilege()
//{
//	return (int)nd_connect_level_get(m_objhandle);
//}
//
//ndip_t NDBaseSession::Getip()
//{
//	return nd_net_getip(m_objhandle);
//	//return nd_sock_getip(((nd_netui_handle)m_objhandle)->fd);
//}
//
//ndport_t NDBaseSession::GetPort()
//{
//	return nd_net_getport(m_objhandle);
//}
//
//ndip_t NDBaseSession::GetPeerip()
//{
//	return nd_net_peer_getip(m_objhandle);
//}
//ndport_t NDBaseSession::GetPeerPort()
//{
//	return nd_net_peer_getport(m_objhandle);
//}
//
//int NDBaseSession::Ioctl(int cmd, void *val, int *size)
//{
//	if (m_objhandle /*&& nd_connector_valid((nd_netui_handle)m_objhandle)*/)	{
//		return  nd_net_ioctl((nd_netui_handle)m_objhandle, cmd, val, size);
//	}
//	return -1;
//}

int NDBaseSession::SetDelayClose()
{
	TCPNODE_SET_RESET(m_objhandle);
	return 0;
}

int NDBaseSession::LoadBalance()
{
	NDObject *pobj = GetParent();
	nd_assert(pobj);
	return nd_session_loadbalancing((nd_listen_handle)pobj->GetHandle(), GetSessionID());

}

//////////////////////////////////////////////////////////////////////////
//class NDSession

bool m_bRedirctLog;
NDSession *NDSession::g_redirect_log_object=NULL;
nd_log_entry NDSession::g_redirectOrgFunc = NULL;

static int redirect_log_entry(const char* text)
{
	if (NDSession::g_redirect_log_object)	{
		NDOStreamMsg omsg(ND_MAIN_ID_SYS, ND_MSG_SYS_REDIRECT_SRV_LOG_OUTPUT);
		omsg.Write(text);
		return NDSession::g_redirect_log_object->SendMsg(omsg);
	}
	else if (NDSession::g_redirectOrgFunc) {
		return NDSession::g_redirectOrgFunc(text);
	}
	return 0;
}

NDSession::NDSession () 
{
	m_id = 0;
	m_type = 0;
	m_bRedirctLog = false;
}

NDSession::~NDSession() 
{
	if (m_bRedirctLog && g_redirect_log_object == this) {
		nd_setlog_func(NDSession::g_redirectOrgFunc);
		NDSession::g_redirectOrgFunc = 0;
	}

}


bool NDSession::RedirectLogToMe()
{
	if (m_bRedirctLog)	{
		return true;
	}
	m_bRedirctLog = true;
	g_redirect_log_object = this;
	if (!NDSession::g_redirectOrgFunc){
		NDSession::g_redirectOrgFunc = nd_setlog_func(redirect_log_entry);
	}
	else {
		nd_setlog_func(redirect_log_entry);
	}
	return true;
}

//send message in script
//int NDSession::Send(NDOStreamMsg &omsg)
//{
//	return ::nd_sessionmsg_sendex(GetHandle() , (nd_usermsghdr_t *)(omsg.GetMsgAddr()), ESF_NORMAL) ;
//}
//
//int NDSession::Send(int maxId, int minId, void *data, size_t len)
//{
//	if (!data || len == 0) {
//		nd_usermsghdr_t header;
//		nd_usermsghdr_init(&header);
//		header.maxid = maxId;
//		header.minid = minId;
//		return ::nd_sessionmsg_sendex(GetHandle(), &header, ESF_NORMAL);
//	}
//	else {
//		NDOStreamMsg omsg(maxId, minId);
//		omsg.WriteStream((char*)data, len);
//
//		return Send(omsg);
//	}
//}
//
//int NDSession::Send(NDUINT16 messageId, void *data, size_t len)
//{
//	return Send((int)ND_HIBYTE(messageId), (int)ND_LOBYTE(messageId), data, len);
//}
//
//int NDSession::SendMsg(NDSendMsg &smsg, int flag)
//{
//	return ::nd_sessionmsg_sendex(GetHandle() , (nd_usermsghdr_t *)(smsg.GetMsgAddr()), flag) ;
//}
//int NDSession::SendMsg(nd_usermsghdr_t *msghdr, int flag)
//{
//	return ::nd_sessionmsg_sendex(GetHandle() , msghdr, flag) ;
//}
//int NDSession::ResendMsg(NDIStreamMsg &resendmsg, int flag)
//{
//	return ::nd_sessionmsg_sendex(GetHandle() , (nd_usermsghdr_t *)(resendmsg.GetMsgAddr()), flag) ;
//}


#undef ND_NEW_REDEFINE
