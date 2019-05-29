/* file : nd_connector.cpp
 * connector class of nd engine 
 *
 * all right reserved by neil duan 
 * 2009-5-8 23:25 
 */

#include "ndapplib/nd_connector.h"
#include "ndstl/nd_utility.h"
//////////////////////////////////////////////////////////////////////////
//class NDConnector

//NDConnector * htoConnector(nd_handle h)
//{
//	return static_cast<NDConnector*>(((nd_netui_handle)h)->user_data  );
//}
//
//NDObject * htoNDObject(nd_handle h)
//{
//	return static_cast<NDObject*>(((nd_netui_handle)h)->user_data);
//}

NDConnector::NDConnector(int maxmsg_num, int maxid_start) : NDBaseConnector()
{
	msg_kinds = maxmsg_num;
	msg_base = maxid_start;
	m_open = 0 ;
}

NDConnector::~NDConnector() 
{
}

void NDConnector::SetMsgNum(int maxmsg_num , int maxid_start) 
{
	msg_kinds = maxmsg_num;
	msg_base = maxid_start;
}
#ifndef DND_CLIENT_ONLY
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
		return -1;
	}
	
	OnInitilize() ;
	m_open = 1 ;
	return 0 ;

}

int NDConnector::Close(int force)
{
	ND_TRACE_FUNC();
	if(m_objhandle && m_open ) {
		char iptext[64];
		nd_logwarn("connector %s closed eror = %d \n", ND_INET_NTOA(GetPeerip(), iptext), LastError() ) ;
		int ret =  nd_connector_close(m_objhandle, force) ;
		if(ret==0) 
			OnClose() ;
		m_open = 0 ;
		return ret;
	}
	return -1 ;
}
#endif

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

	//((nd_netui_handle)m_objhandle)->user_data =(void*) this ;
	((nd_netui_handle)m_objhandle)->msg_caller =(void*) this ;
	

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
	if(m_objhandle && ((nd_netui_handle)m_objhandle)->msg_caller ==(void*) this){
		Close(flag) ;
		OnDestroy() ;
		nd_msgtable_destroy(m_objhandle, 0);
		nd_object_destroy(m_objhandle, 0) ;
		m_objhandle = 0 ;
	}
}
//
//int NDConnector::SendMsg(NDSendMsg &msg, int flag)
//{
//	return SendMsg((nd_usermsghdr_t*) (msg.GetMsgAddr()),  flag) ;
//}
//
//int NDConnector::SendMsg(nd_usermsghdr_t *msghdr, int flag)
//{
//	ND_TRACE_FUNC();
//	int ret;
//	nd_assert(m_objhandle);
//	if (!m_objhandle ) {
//		nd_logwarn("try to send data error , connector is not created\n") ;
//		return -1 ;
//	}
//	ND_USERMSG_SYS_RESERVED(msghdr) = 0 ;
//	ret = nd_connector_send(m_objhandle,&msghdr->packet_hdr, flag) ;
//	if (ret > 0 && (flag & ESF_URGENCY)) {
//		if (m_objhandle->type == NDHANDLE_TCPNODE){
//			nd_tcpnode_flush_sendbuf((nd_netui_handle)m_objhandle) ;
//		}
//	}
//	else if(ret == -1 && nd_object_lasterror(m_objhandle) != NDERR_WOULD_BLOCK) {
//		nd_logwarn("Send data error errorcode =%d\n", LastError() ) ;
//		Close(0);
//	}
//	return ret ;
//}
//
//int NDConnector::ResendMsg(NDIStreamMsg &resendmsg, int flag)
//{
//	return SendMsg((nd_usermsghdr_t*) (resendmsg.GetMsgAddr()),  flag) ;
//}
//
//int NDConnector::SendRawData(void *data , size_t size)
//{
//	ND_TRACE_FUNC();
//	int ret ;
//	nd_assert(m_objhandle) ;
//	ret = nd_connector_raw_write(m_objhandle,data,size) ;
//	if (ret > 0 ) {
//		if (m_objhandle->type == NDHANDLE_TCPNODE){
//			nd_tcpnode_flush_sendbuf((nd_netui_handle)m_objhandle) ;
//		}
//	}
//	else if(ret == -1 && nd_object_lasterror(m_objhandle) != NDERR_WOULD_BLOCK) {
//		Close(0);
//	}
//	return ret ;
//
//}
//
//
//int NDConnector::RecvRawData(void *buf, size_t size, ndtime_t waittm)
//{
//	ND_TRACE_FUNC();
//	nd_assert(m_objhandle) ;
//	return nd_connector_raw_waitdata(m_objhandle, buf, size, waittm) ;
//}

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
//
//int NDConnector::WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time)
//{
//	ND_TRACE_FUNC();
//	return nd_connector_waitmsg(m_objhandle, (nd_packetbuf_t *)msgbuf,wait_time);
//}
void NDConnector::InstallMsgFunc( nd_conn_msg_entry func, ndmsgid_t maxid, ndmsgid_t minid,const char *msgname)
{
	if(m_objhandle)
		nd_msgentry_install(m_objhandle,(nd_usermsg_func)func,  maxid,  minid,EPL_CONNECT,msgname) ;
}

void NDConnector::SetDftMsgHandler(nd_conn_msg_entry func)
{
	if(m_objhandle)
		nd_msgentry_def_handler(m_objhandle, (nd_usermsg_func)func) ;
	
}


//
//int NDConnector::CheckValid()
//{
//	if(!m_objhandle)
//		return 0 ;
//	return 	nd_connector_valid((nd_netui_handle)m_objhandle) ;
//}
//
//ndip_t NDConnector::Getip()
//{
//	ndip_t ret = ND_IP_INIT;
//	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
//	if (fd){
//		ret = nd_sock_getip(fd) ;
//	}
//	return ret;
//}
//
//ndport_t NDConnector::GetPort()
//{
//	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
//	if (fd){
//		return nd_sock_getport(fd) ;
//	}
//	return 0 ;
//}
//
//ndip_t NDConnector::GetPeerip()
//{
//	ndip_t ret = ND_IP_INIT;
//	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
//	if (fd){
//		ret = nd_sock_getpeerip(fd) ;
//	}
//	return ret;
//
//}
//ndport_t NDConnector::GetPeerPort()
//{
//	ndsocket_t fd = ((nd_netui_handle)m_objhandle)->fd ;
//	if (fd){
//		return nd_sock_getpeerport(fd) ;
//	}
//	return 0 ;
//}
//
//void NDConnector::SetConnectTimeOut(int seconds) 
//{
//	ND_TRACE_FUNC();
//	nd_connector_set_timeout((nd_netui_handle)m_objhandle,seconds) ;
//}
//
//
//int NDConnector::Ioctl(int cmd, void *val, int *size) 
//{
//	if (m_objhandle && nd_connector_valid((nd_netui_handle)m_objhandle))	{
//		return  nd_net_ioctl((nd_netui_handle)m_objhandle,  cmd, val, size) ;
//	}
//	return 0;
//}

//////////////////////////////////////////////////////////////////////////

NDSafeConnector::NDSafeConnector(int maxmsg_num, int maxid_start) : NDConnector(maxmsg_num, maxid_start)
{
	INIT_LIST_HEAD(&m_undeliveried_list);
}

NDSafeConnector::~NDSafeConnector()
{
	if (!list_empty(&m_undeliveried_list))	{
		struct list_head *pos, *next;
		list_for_each_safe(pos, next, &m_undeliveried_list) {
			resend_data *data = list_entry(pos, resend_data, list);
			free(data);
		}
		INIT_LIST_HEAD(&m_undeliveried_list);
	}
}

int NDSafeConnector::SendSafe(NDSendMsg &msg, int flag)
{
	return SendSafe(&(msg.GetMsgAddr()->msg_hdr), flag);
}

int NDSafeConnector::SendSafe(nd_usermsghdr_t *msghdr, int flag)
{
	int ret = 0;
	if (CheckValid()){
		ret = SendMsg(msghdr, flag);
	}
	if (ret > 0){
		return ret;
	}

	int size = ND_USERMSG_LEN(msghdr);

	resend_data *pdata = (resend_data *)malloc(size + sizeof(resend_data));
	if (!pdata)	{
		return -1;
	}

	pdata->flag = flag;

	INIT_LIST_HEAD(&pdata->list);
	memcpy(&pdata->msg_hdr, msghdr, size);

	list_add_tail(&pdata->list, &m_undeliveried_list);

	return 0;
}

int NDSafeConnector::TrytoResennd()
{
	int ret = 0;
	if (!list_empty(&m_undeliveried_list))	{
		struct list_head *pos, *next;
		list_for_each_safe(pos, next, &m_undeliveried_list) {
			resend_data *data = list_entry(pos, resend_data, list);

			if (SendMsg((nd_usermsghdr_t *)&data->msg_hdr, data->flag) > 0) {
				list_del(pos);
				free(data);
				++ret;
			}
			else {
				return ret;
			}
		}
	}
	return ret;
}
