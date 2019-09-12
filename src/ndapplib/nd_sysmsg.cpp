/* file nd_sysmsg.cpp
* message handler of system for ndnet
*
* create by duan
*
*/


#include "ndapplib/applib.h" 



MSG_ENTRY_INSTANCE(nd_echo_handler)
{
	ND_TRACE_FUNC();
	
	netconn->SendMsg(msg) ;
	//ND_MSG_SEND(nethandle, (nd_usermsghdr_t *)msg, h_listen);
	nd_logdebug("receive echo message ECHO length=%d\n", ND_USERMSG_LEN(msg));
	return 0;
}

MSG_ENTRY_INSTANCE(nd_broadcast_handler)
{
	//ND_BROAD_CAST(nethandle,(nd_usermsghdr_t *)msg,ESF_URGENCY,0) ;
	return 0;
}



MSG_ENTRY_INSTANCE(nd_transfer_to_msgproc)
{
	ND_TRACE_FUNC();
	NDUINT16 sid;
	NDIStreamMsg inmsg(msg);

	if (-1 == inmsg.Read(sid) || sid == 0) {

		nd_logmsg("transfer-message error sessionid ==0\n");
		return 0;
	}
	nd_handle h_listen = netconn->GetListenerHandle() ;
	if (!h_listen) {
		h_listen = getbase_inst()->GetDeftListener()->GetHandle();
		nd_assert(h_listen);
	}

	NDOStreamMsg omsg(inmsg.MsgMaxid(), inmsg.MsgMinid());
	inmsg.Read(omsg);

	if (-1 == nd_netmsg_handle(sid, (nd_usermsghdr_t*)omsg.GetMsgAddr(), h_listen)) {
		nd_logmsg("nd_netmsg_handle() to %d error\n", sid);
	}
	return 0;
}

MSG_ENTRY_INSTANCE(nd_transfer_to_client)
{
	ND_TRACE_FUNC();
	NDUINT16 sid;
	NDIStreamMsg inmsg(msg);

	if (-1 == inmsg.Read(sid) || sid == 0) {
		nd_logmsg("transfer-message error sessionid ==0\n");
		return 0;
	}
	nd_handle h_listen = netconn->GetListenerHandle() ;
	if (!h_listen) {
		h_listen = getbase_inst()->GetDeftListener()->GetHandle();
		nd_assert(h_listen);
	}
	NDOStreamMsg omsg(inmsg.MsgMaxid(), inmsg.MsgMinid());
	inmsg.Read(omsg);

	if (-1 == nd_session_msg_send_id(sid, (nd_usermsghdr_t*)omsg.GetMsgAddr(), h_listen)) {
		nd_logmsg("nd_netmsg_handle() to %d error\n", sid);
	}
	return 0;
}

MSG_ENTRY_INSTANCE(nd_get_message_name_handler)
{
	ND_TRACE_FUNC();
	NDIStreamMsg inmsg(msg);
	NDOStreamMsg omsg(inmsg.MsgMaxid(), inmsg.MsgMinid());
	NDUINT16 maxID, minID;

	if (-1 == inmsg.Read(maxID)) {
		return 0;
	}
	if (-1 == inmsg.Read(minID)) {
		return 0;
	}
	nd_handle h_listen = netconn->GetListenerHandle() ;
	if (!h_listen) {
		h_listen = getbase_inst()->GetDeftListener()->GetHandle();
		nd_assert(h_listen);
	}
	const char *p = nd_msgentry_get_name(h_listen, (ndmsgid_t)maxID, (ndmsgid_t)minID);

	omsg.Write(maxID);

	omsg.Write(minID);

	omsg.Write((NDUINT8*)p);

	netconn->SendMsg(omsg) ;
	//ND_MSG_SEND(nethandle, omsg.GetMsgAddr(), h_listen);
	return 0;
}

MSG_ENTRY_INSTANCE(nd_get_app_ver_handler)
{
	ND_TRACE_FUNC();
	NDOStreamMsg omsg(ND_USERMSG_MAXID(msg), ND_USERMSG_MINID(msg));

	NDInstanceBase *pinst =  getbase_inst();
	omsg.Write((NDUINT32)pinst->GetVersionID());
	if (getbase_inst()->CheckIsDeveVer()) {
		omsg.Write((NDUINT8)1);
	}
	else {
		omsg.Write((NDUINT8)0);
	}
	omsg.Write((NDUINT8*)pinst->GetVersionDesc());

	netconn->SendMsg(omsg) ;
	//ND_MSG_SEND(nethandle, omsg.GetMsgAddr(), h_listen);
	return 0;
}

MSG_ENTRY_INSTANCE(nd_get_sys_time)
{
	ND_TRACE_FUNC();
	NDIStreamMsg inmsg(msg);
	NDOStreamMsg omsg(inmsg.MsgMaxid(), inmsg.MsgMinid());
	NDUINT64 tm1 = (NDUINT64)time(NULL);
	omsg.Write(tm1);

	netconn->SendMsg(omsg) ;
	//nd_connector_send(nethandle, (nd_packhdr_t*)(omsg.GetMsgAddr()), ESF_URGENCY);

	return 0;
}
MSG_ENTRY_INSTANCE(nd_get_game_time)
{
	ND_TRACE_FUNC();
	NDIStreamMsg inmsg(msg);
	NDOStreamMsg omsg(inmsg.MsgMaxid(), inmsg.MsgMinid());
	NDUINT64 tm1 = (NDUINT64)app_inst_time(NULL);
	omsg.Write(tm1);

	netconn->SendMsg(omsg) ;
	//nd_connector_send(nethandle, (nd_packhdr_t*)(omsg.GetMsgAddr()), ESF_URGENCY);

	return 0;
}

MSG_ENTRY_INSTANCE(nd_quicken_inst_time)
{
	ND_TRACE_FUNC();
	NDUINT32 hh = 0, mm = 0;
	NDIStreamMsg inmsg(msg);

	inmsg.Read(hh);
	inmsg.Read(mm);
	time_t tm1 = app_inst_time(NULL);
	app_inst_set_hm(hh, mm);
	time_t tm2 = app_inst_time(NULL);
	char buf1[64], buf2[64];
	nd_logmsg("change time from %s to %s\n", nd_get_datetimestr_ex(tm1, buf1, 64), nd_get_datetimestr_ex(tm2, buf2, 64));
	return 0;
}

MSG_ENTRY_INSTANCE(nd_get_server_rlimit)
{
	ND_TRACE_FUNC();
	char buf[1024];
	NDOStreamMsg omsg(ND_USERMSG_MAXID(msg), ND_USERMSG_MINID(msg));

	get_rlimit_info(buf, sizeof(buf));
	omsg.Write((NDUINT8*)buf);

	netconn->Send(omsg) ;
	//ND_MSG_SEND(nethandle, omsg.GetMsgAddr(), h_listen);
	return 0;
}

MSG_ENTRY_INSTANCE(nd_set_netmsg_log)
{
	ND_TRACE_FUNC();
	NDIStreamMsg inmsg(msg);
	NDOStreamMsg omsg(inmsg.MsgMaxid(), inmsg.MsgMinid());
	NDUINT8 maxID, minID;
	NDUINT8 isOpen = 0;

	if (-1 == inmsg.Read(maxID)) {
		return 0;
	}
	if (-1 == inmsg.Read(minID)) {
		return 0;
	}
	if (-1 == inmsg.Read(isOpen)) {
		return 0;
	}
	
	nd_handle h_listen = netconn->GetListenerHandle() ;
	if (!h_listen) {
		h_listen = getbase_inst()->GetDeftListener()->GetHandle();
		nd_assert(h_listen);
	}
	
	int ret = nd_message_set_log(h_listen, (ndmsgid_t)maxID, (ndmsgid_t)minID, (int)isOpen);
	if (-1 == ret) {
		isOpen = 0xff;
	}
	else {
		isOpen = ret;
	}

	omsg.Write(maxID);
	omsg.Write(minID);
	omsg.Write(isOpen);

	netconn->SendMsg(omsg) ;
	//ND_MSG_SEND(nethandle, omsg.GetMsgAddr(), h_listen);
	nd_logmsg("message (%d,%d) will be logged\n", maxID, minID);
	return 0;
}

MSG_ENTRY_INSTANCE(nd_set_netmsg_print)
{
	ND_TRACE_FUNC();
	NDIStreamMsg inmsg(msg);
	NDOStreamMsg omsg(inmsg.MsgMaxid(), inmsg.MsgMinid());
	NDUINT8 maxID, minID;
	NDUINT8 isOpen = 0;

	if (-1 == inmsg.Read(maxID)) {
		return 0;
	}
	if (-1 == inmsg.Read(minID)) {
		return 0;
	}
	if (-1 == inmsg.Read(isOpen)) {
		return 0;
	}
	nd_handle h_listen = netconn->GetListenerHandle() ;
	if (!h_listen) {
		h_listen = getbase_inst()->GetDeftListener()->GetHandle();
		nd_assert(h_listen);
	}
	
	int ret = nd_message_set_print(h_listen, (ndmsgid_t)maxID, (ndmsgid_t)minID, (int)isOpen);
	if (-1 == ret) {
		isOpen = 0xff;
	}
	else {
		isOpen = ret;
	}

	omsg.Write(maxID);
	omsg.Write(minID);
	omsg.Write(isOpen);

	netconn->SendMsg(omsg) ;
	//ND_MSG_SEND(nethandle, omsg.GetMsgAddr(), h_listen);
	nd_logmsg("message (%d,%d) will be print format\n", maxID, minID);
	return 0;
}


MSG_ENTRY_INSTANCE(app_statics_begin)
{
	ND_TRACE_FUNC();
	getbase_inst()->StartStaticsMem();
	return 0;
}

MSG_ENTRY_INSTANCE(app_statics_end)
{
	ND_TRACE_FUNC();
	getbase_inst()->EndStaticsMem();
	return 0;
}

MSG_ENTRY_INSTANCE(default_close_handler)
{
	return -1;
}


MSG_ENTRY_INSTANCE(nd_redirect_msglog_to_me)
{
	ND_TRACE_FUNC();
	NDSession *psession = dynamic_cast<NDSession*>(netconn);
	if (psession) {
		psession->RedirectLogToMe();
		return 0;
	}
	return -1;
}


MSG_ENTRY_INSTANCE(nd_open_log_handler)
{
	ND_TRACE_FUNC();
	NDIStreamMsg inmsg(msg);
	NDUINT8 bIsopen, logLevel;

	if (-1 == inmsg.Read(logLevel)) {
		return 0;
	}

	if (-1 == inmsg.Read(bIsopen)) {
		return 0;
	}

	if (bIsopen) {
		nd_log_open((edg_ID)logLevel);
	}
	else {
		nd_log_close((edg_ID)logLevel);
	}
	return 0;
}


MSG_ENTRY_INSTANCE(error_ack_message)
{
	ND_TRACE_FUNC();
	NDOStreamMsg omsg(ND_MAIN_ID_SYS,ND_MSG_SYS_ERROR);
	omsg.Write((NDUINT32)NDERR_FUNCTION_CLOSED);
	netconn->SendMsg(omsg) ;
	//ND_MSG_SEND(nethandle, omsg.GetMsgAddr(), h_listen);
	return 0;
}


MSG_ENTRY_INSTANCE(nd_close_exist_msg_handler)
{
	ND_TRACE_FUNC();
	NDIStreamMsg inmsg(msg);
	NDUINT8 maxID, minID;

	if (-1 == inmsg.Read(maxID)) {
		return 0;
	}

	if (-1 == inmsg.Read(minID)) {
		return 0;
	}
	nd_handle h_listen = netconn->GetListenerHandle() ;
	if (!h_listen) {
		h_listen = getbase_inst()->GetDeftListener()->GetHandle();
		nd_assert(h_listen);
	}
	nd_msgentry_install(h_listen, (nd_usermsg_func)error_ack_message, maxID, minID, EPL_READY, NULL);
	return 0;
}
