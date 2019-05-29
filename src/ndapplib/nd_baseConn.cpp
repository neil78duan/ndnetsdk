/* file nd_baseConn.cpp
 *
 * net connector base class
 *
 * create by duan 
 *
 * 2019.5.24
 *
 */


#include "ndapplib/nd_baseConn.h"
#include "ndstl/nd_utility.h"

NDBaseConnector::NDBaseConnector()
{
	
}
NDBaseConnector::~NDBaseConnector()
{
	
}

int NDBaseConnector::Send(NDOStreamMsg &omsg)
{
	return sendPack((nd_packhdr_t*) (omsg.GetMsgAddr()),  ESF_NORMAL) ;
}
int NDBaseConnector::SendMsg(NDSendMsg &msg, int flag)
{
	return sendPack((nd_packhdr_t*) (msg.GetMsgAddr()),  flag) ;
}

int NDBaseConnector::SendMsg(nd_usermsghdr_t *msghdr, int flag)
{
	return sendPack( &msghdr->packet_hdr,  flag) ;
}

int NDBaseConnector::SendMsg(nd_usermsgbuf_t *msg, int flag)
{
	return sendPack( &msg->msg_hdr.packet_hdr,  flag) ;
}


int NDBaseConnector::SendMsg(NDIStreamMsg &resendmsg, int flag)
{
	return sendPack((nd_packhdr_t*) (resendmsg.GetMsgAddr()),  flag) ;
}

int NDBaseConnector::sendPack(nd_packhdr_t *msghdr, int flag)
{
	ND_TRACE_FUNC();
	int ret;
	nd_assert(m_objhandle);
	if (!m_objhandle ) {
		return -1 ;
	}
	
	ND_USERMSG_SYS_RESERVED(msghdr) = 0;
	
	ret = nd_connector_send(m_objhandle,msghdr, flag) ;
	if (ret > 0 && (flag & ESF_URGENCY)) {
		if (m_objhandle->type == NDHANDLE_TCPNODE){
			nd_tcpnode_flush_sendbuf((nd_netui_handle)m_objhandle) ;
		}
	}
	else if(ret == -1 && nd_object_lasterror(m_objhandle) != NDERR_WOULD_BLOCK) {
		Close(0);
	}
	return ret ;
}

int NDBaseConnector::SendRawData(void *data , size_t size)
{
	ND_TRACE_FUNC();
	nd_assert(m_objhandle) ;
	return nd_connector_raw_write(m_objhandle,data,size) ;
}


int NDBaseConnector::RecvRawData(void *buf, size_t size, ndtime_t waittm)
{
	ND_TRACE_FUNC();
	nd_assert(m_objhandle) ;
	return nd_connector_raw_waitdata(m_objhandle, buf, size, waittm) ;
}

int NDBaseConnector::CryptPackage(nd_usermsgbuf_t *msgBuf)
{
	if (msgBuf->msg_hdr.packet_hdr.encrypt) {
		return 0;
	}
	return nd_packet_encrypt(m_objhandle, (nd_packetbuf_t*)msgBuf) > 0 ? 0 : -1;
	
}

bool NDBaseConnector::FlushSendBuf(bool bForce)
{
	if (bForce) {
		return nd_tcpnode_flush_sendbuf_force((nd_netui_handle)GetHandle()) > 0;
	}
	else {
		return nd_tcpnode_flush_sendbuf((nd_netui_handle)GetHandle()) > 0;
	}
}

int NDBaseConnector::WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time)
{
	ND_TRACE_FUNC();
	return nd_connector_waitmsg(m_objhandle, (nd_packetbuf_t *)msgbuf,wait_time);
}

int NDBaseConnector::CheckValid()
{
	if(!m_objhandle)
		return 0 ;
	return 	nd_connector_valid((nd_netui_handle)m_objhandle) ;
}

ndip_t NDBaseConnector::Getip()
{
	ndip_t ret = ND_IP_INIT;
	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
	if (fd){
		ret = nd_sock_getip(fd) ;
	}
	return ret;
}

ndport_t NDBaseConnector::GetPort()
{
	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
	if (fd){
		return nd_sock_getport(fd) ;
	}
	return 0 ;
}

ndip_t NDBaseConnector::GetPeerip()
{
	ndip_t ret = ND_IP_INIT;
	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
	if (fd){
		ret = nd_sock_getpeerip(fd) ;
	}
	return ret;
	
}
ndport_t NDBaseConnector::GetPeerPort()
{
	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
	if (fd){
		return nd_sock_getpeerport(fd) ;
	}
	return 0 ;
}

NDUINT16 NDBaseConnector::GetSessionID()
{
	nd_assert(m_objhandle);
	if (m_objhandle){
		return ((nd_netui_handle)m_objhandle)->session_id;
	}
	return (NDUINT16)-1;
}

nd_handle NDBaseConnector::GetListenerHandle()
{
	return (nd_handle) (((struct netui_info*)m_objhandle)->srv_root );
}

void NDBaseConnector::SetConnectTimeOut(int seconds)
{
	ND_TRACE_FUNC();
	nd_connector_set_timeout((nd_netui_handle)m_objhandle,seconds) ;
}


int NDBaseConnector::Ioctl(int cmd, void *val, int *size)
{
	if (m_objhandle && nd_connector_valid((nd_netui_handle)m_objhandle))	{
		return  nd_net_ioctl((nd_netui_handle)m_objhandle,  cmd, val, size) ;
	}
	return 0;
}


void NDBaseConnector::SetPrivilege(int level)
{
	nd_connect_level_set(m_objhandle, (NDUINT32)level);
}
int NDBaseConnector::GetPrivilege()
{
	return (int)nd_connect_level_get(m_objhandle);
}

void *NDBaseConnector::GetScriptEngine()
{
	if (m_objhandle) {
		return nd_message_get_script_engine(m_objhandle);
	}
	return NULL;
}


void *NDBaseConnector::GetUserData()
{
	if (m_objhandle){
		return nd_connector_get_userdata((nd_netui_handle)m_objhandle);
	}
	return NULL;
}
void NDBaseConnector::SetUserData(void *pData)
{
	if (m_objhandle){
		return nd_connector_set_userdata((nd_netui_handle)m_objhandle,pData);
	}
	
}

