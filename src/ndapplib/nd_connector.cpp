/* file : nd_connector.cpp
 * connector class of nd engine 
 *
 * all right reserved by neil duan 
 * 2009-5-8 23:25 
 */

#include "ndapplib/nd_connector.h"
//////////////////////////////////////////////////////////////////////////
//class NDConnector

NDConnector * htoConnector(nd_handle h)
{
	//return (NDConnector*) (((nd_netui_handle)h)->user_data  );
	return static_cast<NDConnector*>(((nd_netui_handle)h)->user_data  );
}

static int data_in_ofconnect(nd_handle h,void *data , size_t len,nd_handle listen_h)
{
	NDConnector *pconn = htoConnector(h) ;
	if (pconn){
		return pconn->_data_func(data,len) ;
	}
	return 0;
}

NDConnector::NDConnector(int maxmsg_num , int maxid_start) 
{
	m_objhandle = NULL ;
	msg_kinds = maxmsg_num;
	msg_base = maxid_start;
	m_old_in_func = 0;
	m_open = 0 ;
}

NDConnector::~NDConnector() 
{
	//Destroy() ;
}

//设置消息映射表大小,必须在OPEN函数前调用
void NDConnector::SetMsgNum(int maxmsg_num , int maxid_start) 
{
	msg_kinds = maxmsg_num;
	msg_base = maxid_start;
}

int NDConnector::_data_func(void *data, size_t size) 
{
	ND_TRACE_FUNC();
	if (!data || 0==size){
		//OnClose() ;
	}
	else if(m_old_in_func) {
		return m_old_in_func(m_objhandle,data,size,NULL) ;
	}
	return 0;
}

int NDConnector::Open(const char *host, int port, const char *protocol_name,nd_proxy_info *proxy)
{
	ND_TRACE_FUNC();
	if(!m_objhandle) {
		//return -1 ;
		if(-1==Create(protocol_name) ) {
			return -1;
		}
		if (!m_objhandle){
			return -1;
		}
	}
	
	if(-1==nd_connector_open( m_objhandle, host,  port,proxy ) ) {
		nd_logerror("connect error :%s!" AND nd_last_error()) ;
		//nd_object_destroy(m_objhandle,1) ;
		//m_objhandle = NULL ;
		return -1;
	}

	m_old_in_func = ((struct netui_info*)m_objhandle)->data_entry ;
	((struct netui_info*)m_objhandle)->data_entry = data_in_ofconnect ;

	OnInitilize() ;
	m_open = 1 ;
	return 0 ;

}

int NDConnector::Close(int force)
{
	ND_TRACE_FUNC();
	if(m_objhandle && m_open ) {
		nd_logerror("connector %s closed eror = %d \n", nd_inet_ntoa(GetPeerip(), NULL), LastError() ) ;
		int ret =  nd_connector_close(m_objhandle, force) ;
		if(ret==0) 
			OnClose() ;
		m_open = 0 ;
		return ret;
	}
	return -1 ;
}


int NDConnector::Create(const char *protocol_name)
{
	ND_TRACE_FUNC();
	//connect to host 
	if (m_objhandle) {
		Destroy() ;
		m_objhandle = NULL;
	}
	m_objhandle = nd_object_create(protocol_name? protocol_name: (char*)"tcp-connector"  ) ;

	if(!m_objhandle){		
		nd_logerror((char*)"connect error :%s!" AND nd_last_error()) ;
		return -1;
	}
	((nd_netui_handle)m_objhandle)->user_data =(void*) this ;

	//set message handle	
	if (msg_kinds > 0){
		if(-1==nd_msgtable_create(m_objhandle, msg_kinds, msg_base) ) {
			nd_object_destroy(m_objhandle, 0) ;
		}
	}
	OnCreate() ;
	return 0 ;

}
void NDConnector::Destroy(int flag)
{
	ND_TRACE_FUNC();
	if(m_objhandle && ((nd_netui_handle)m_objhandle)->user_data ==(void*) this){
		Close(flag) ;
		//nd_connector_close(m_objhandle, 0) ;
		OnDestroy() ;
		nd_msgtable_destroy(m_objhandle, 0);
		nd_object_destroy(m_objhandle, 0) ;
		m_objhandle = 0 ;
	}
}

int NDConnector::SendMsg(NDSendMsg &msg, int flag)
{
	return SendMsg((nd_usermsghdr_t*) (msg.GetMsgAddr()),  flag) ;
	/*
	ND_TRACE_FUNC();
	int ret ;
	if (!m_objhandle || !nd_connector_valid((nd_netui_handle)m_objhandle )) {
		return -1 ;
	}
	ret = nd_connector_send(m_objhandle,(nd_packhdr_t*) (msg.GetMsgAddr()), flag) ;
	if (ret > 0 && (flag & ESF_URGENCY)) {
		if (m_objhandle->type == NDHANDLE_TCPNODE){
			nd_tcpnode_flush_sendbuf((nd_netui_handle)m_objhandle) ;
		}
	}
	else if(ret == -1 && nd_object_lasterror(m_objhandle) != NDERR_WUOLD_BLOCK) {
		Close(0);
	}
	return ret ;
	 */
}

int NDConnector::SendMsg(nd_usermsghdr_t *msghdr, int flag)
{
	ND_TRACE_FUNC();
	int ret ;
	if (!m_objhandle || !nd_connector_valid((nd_netui_handle)m_objhandle )) {
		nd_logerror("try to send data error , connector is invalid\n") ;
		return -1 ;
	}
	nd_assert(m_objhandle) ;
	ret = nd_connector_send(m_objhandle,&msghdr->packet_hdr, flag) ;
	if (ret > 0 && (flag & ESF_URGENCY)) {
		if (m_objhandle->type == NDHANDLE_TCPNODE){
			nd_tcpnode_flush_sendbuf((nd_netui_handle)m_objhandle) ;
		}
	}
	else if(ret == -1 && nd_object_lasterror(m_objhandle) != NDERR_WUOLD_BLOCK) {
		nd_logerror("Send data error errorcode =%d\n", LastError() ) ;
		Close(0);
	}
	return ret ;
}

int NDConnector::ResendMsg(NDIStreamMsg &resendmsg, int flag)
{
	return SendMsg((nd_usermsghdr_t*) (resendmsg.GetMsgAddr()),  flag) ;
	/*
	ND_TRACE_FUNC();
	int ret ;
	nd_assert(m_objhandle) ;
	ret = nd_connector_send(m_objhandle,(nd_packhdr_t*) (resendmsg.GetMsgAddr()), flag) ;
	if (ret > 0 && (flag & ESF_URGENCY)) {
		if (m_objhandle->type == NDHANDLE_TCPNODE){
			nd_tcpnode_flush_sendbuf((nd_netui_handle)m_objhandle) ;
		}
	}
	else if(ret == -1 && nd_object_lasterror(m_objhandle) != NDERR_WUOLD_BLOCK) {
		Close(0);
	}
	return ret ;
	 */
}

int NDConnector::SendRawData(void *data , size_t size) 
{
	ND_TRACE_FUNC();
	int ret ;
	nd_assert(m_objhandle) ;
	ret = nd_connector_raw_write(m_objhandle,data,size) ;
	if (ret > 0 ) {
		if (m_objhandle->type == NDHANDLE_TCPNODE){
			nd_tcpnode_flush_sendbuf((nd_netui_handle)m_objhandle) ;
		}
	}
	else if(ret == -1 && nd_object_lasterror(m_objhandle) != NDERR_WUOLD_BLOCK) {
		Close(0);
	}
	return ret ;

}

int NDConnector::RecvRawData(void *buf, size_t size, ndtime_t waittm) 
{
	ND_TRACE_FUNC();
	nd_assert(m_objhandle) ;
	return nd_connector_raw_waitdata(m_objhandle, buf, size, waittm) ;
}

int NDConnector::Update(ndtime_t wait_time)
{
	ND_TRACE_FUNC();

	int ret;
	//nd_msgui_buf msg_recv;
	if(m_objhandle->type==NDHANDLE_UDPNODE) {
		nd_usermsgbuf_t msg_recv;
RE_WAIT:
		ret = nd_connector_waitmsg(m_objhandle, (nd_packetbuf_t *)&msg_recv,wait_time);
		if(ret > 0) {			
			//msg_entry(connect_handle, &msg_recv) ;
			nd_translate_message(m_objhandle, (nd_packhdr_t*)&msg_recv, 0) ;
			wait_time = 0;
			goto RE_WAIT;
			//return 0;
		}
		else {
			return ret ;
		}
		
	}
	else {
		return nd_connector_update(m_objhandle,wait_time) ;
	}
}

int NDConnector::WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time)
{
	ND_TRACE_FUNC();
	return nd_connector_waitmsg(m_objhandle, (nd_packetbuf_t *)msgbuf,wait_time);
}
void NDConnector::InstallMsgFunc(nd_usermsg_func func, ndmsgid_t maxid, ndmsgid_t minid)
{
	if(m_objhandle)
		nd_msgentry_install(m_objhandle, func,  maxid,  minid,EPL_CONNECT) ;
}

int NDConnector::CheckValid()
{
	if(!m_objhandle)
		return 0 ;
	return 	nd_connector_valid((nd_netui_handle)m_objhandle) ;
}

ndip_t NDConnector::Getip() 
{
	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
	if (fd){
		return nd_sock_getip(fd) ;
	}
	return 0 ;
}

ndport_t NDConnector::GetPort() 
{
	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
	if (fd){
		return nd_sock_getport(fd) ;
	}
	return 0 ;
}

ndip_t NDConnector::GetPeerip() 
{
	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
	if (fd){
		return nd_sock_getpeerip(fd) ;
	}
	return 0 ;

}
ndport_t NDConnector::GetPeerPort() 
{
	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
	if (fd){
		return nd_sock_getpeerport(fd) ;
	}
	return 0 ;
}

void NDConnector::SetConnectTimeOut(int seconds) 
{
	ND_TRACE_FUNC();
	//((nd_netui_handle)m_objhandle)->disconn_timeout = tmval;
	nd_connector_set_timeout((nd_netui_handle)m_objhandle,seconds) ;
}


int NDConnector::Ioctl(int cmd, void *val, int *size) 
{
	if (m_objhandle && nd_connector_valid((nd_netui_handle)m_objhandle))	{
		return  nd_net_ioctl((nd_netui_handle)m_objhandle,  cmd, val, size) ;
	}
	return 0;
}