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
#include "nd_msg.h"

static  int _session_update(nd_handle h)
{
	NDBaseSession *pSe = NDGetSession(h);
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
		((nd_session_handle)m_objhandle)->update_entry = m_handle_update;
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

int NDBaseSession::BigDataSend(int maxID, int minID, void *data, size_t datalen)
{
	NDUINT32 hiDword = -1;
	NDUINT32 lowDord = ND_MAKE_DWORD(maxID, minID);
	NDUINT64 param = ND_MAKE_QWORD(hiDword, lowDord);

	return BigDataAsyncSend(m_objhandle, data, datalen, param, NULL);
}

int NDBaseSession::CryptPackage(nd_usermsgbuf_t *msgBuf)
{
	if (msgBuf->msg_hdr.packet_hdr.encrypt) {
		return 0;
	}
	return nd_packet_encrypt(m_objhandle, (nd_packetbuf_t*)msgBuf) > 0 ? 0 : -1;

}
//得到网络地址
const char* NDBaseSession::GetInetAddr()
{
	static char buf[20];
	SOCKADDR_IN *addr = &(((nd_netui_handle)m_objhandle)->remote_addr);
	return (const char*)nd_inet_ntoa(addr->sin_addr.s_addr, buf);

}

NDUINT16 NDBaseSession::GetSessionID()
{
	nd_assert(m_objhandle);
	if (m_objhandle){
		return ((nd_netui_handle)m_objhandle)->session_id;
	}
	return (NDUINT16)-1;
}
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

int NDBaseSession::SendRawData(void *data, size_t size)
{
	int ret = nd_session_raw_write(m_objhandle, data, size);
	if (ret == -1){
		nd_logmsg("SendRawData() send error ret = -1\n");
	}
	return ret;
}

void NDBaseSession::Initilize(nd_handle hsession, nd_handle listen)
{
	ND_TRACE_FUNC();
	m_objhandle = hsession;
	m_handle_update = ((nd_session_handle)hsession)->update_entry;
	((nd_session_handle)hsession)->update_entry = _session_update;
	NDAlarm::Create();
	OnInitilize();
}

int NDBaseSession::Close(int flag)
{
	if (m_handle_update) {
		((nd_session_handle)m_objhandle)->update_entry = m_handle_update;
		m_handle_update = 0;
	}
	NDAlarm::Destroy(flag);
	return nd_session_close(m_objhandle, flag);
}
NDObject* NDBaseSession::GetParent()
{
	return NDGetListener(((nd_netui_handle)m_objhandle)->srv_root);
}

void NDBaseSession::SetPrivilege(int level)
{
	nd_connect_level_set(m_objhandle, (NDUINT32)level);
}
int NDBaseSession::GetPrivilege()
{
	return (int)nd_connect_level_get(m_objhandle);
}

ndip_t NDBaseSession::Getip()
{
	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd;
	if (fd){
		return nd_sock_getip(fd);
	}
	return 0;
}

ndport_t NDBaseSession::GetPort()
{
	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd;
	if (fd){
		return nd_sock_getport(fd);
	}
	return 0;
}

ndip_t NDBaseSession::GetPeerip()
{

	return nd_net_peer_getip(m_objhandle);

}
ndport_t NDBaseSession::GetPeerPort()
{
	return nd_net_peer_getport(m_objhandle);
}

int NDBaseSession::Ioctl(int cmd, void *val, int *size)
{
	if (m_objhandle && nd_connector_valid((nd_netui_handle)m_objhandle))	{
		return  nd_net_ioctl((nd_netui_handle)m_objhandle, cmd, val, size);
	}
	return -1;
}

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
logfunc NDSession::g_redirectOrgFunc =NULL;

static void redirect_log_entry(const char* text)
{
	if (NDSession::g_redirect_log_object)	{
		NDOStreamMsg omsg(ND_MAIN_ID_SYS, ND_MSG_SYS_REDIRECT_SRV_LOG_OUTPUT);
		omsg.Write(text);
		NDSession::g_redirect_log_object->SendMsg(omsg);
	}
	else if (NDSession::g_redirectOrgFunc) {
		NDSession::g_redirectOrgFunc(text);
	}
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
}

int NDSession::Send(int maxId, int minId, void *data, size_t len)
{
	if (!data || len == 0) {
		nd_usermsghdr_t header;
		nd_usermsghdr_init(&header);
		header.maxid = maxId;
		header.minid = minId;
		return ::nd_sessionmsg_sendex(GetHandle(), &header, ESF_NORMAL);
	}
	else {
		NDOStreamMsg omsg(maxId, minId);
		omsg.WriteStream((char*)data, len);

		return Send(omsg);
	}
}

int NDSession::Send(NDUINT16 messageId, void *data, size_t len)
{
	return Send((int)ND_HIBYTE(messageId), (int)ND_LOBYTE(messageId), data, len);
}

int NDSession::SendMsg(NDSendMsg &smsg, int flag)
{
	return ::nd_sessionmsg_sendex(GetHandle() , (nd_usermsghdr_t *)(smsg.GetMsgAddr()), flag) ;
}
int NDSession::SendMsg(nd_usermsghdr_t *msghdr, int flag)
{
	return ::nd_sessionmsg_sendex(GetHandle() , msghdr, flag) ;
}
int NDSession::ResendMsg(NDIStreamMsg &resendmsg, int flag)
{
	return ::nd_sessionmsg_sendex(GetHandle() , (nd_usermsghdr_t *)(resendmsg.GetMsgAddr()), flag) ;
}


#undef ND_NEW_REDEFINE
