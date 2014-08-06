/* file : nd_appdef.h
 * define function of application
 *
 * all right reserved by neil duan
 *
 * 2009
 */

#ifndef _ND_APPDEF_H_
#define _ND_APPDEF_H_

/* 单线程模式使用的是sessionid ,普通模式使用的是连接句柄
 * 并且发送和处理函数格式上有少许不同,为了统一编程接口,
 * 在这里加了条件编译,主要就是为了解决这个问题
 */
//#define SINGLE_THREAD_MOD 1			//使用单线程模式

#ifdef SINGLE_THREAD_MOD
	//定义连接标识符
	typedef NDUINT16 ND_SESSION_T ;
	//定义消息处理函数
	#define MSG_ENTRY_INSTANCE(name) \
	int name (NDUINT16 nethandle,nd_usermsgbuf_t *msg, nd_handle h_listen) 
	#define MSG_ENTRY_DECLARE(name) \
	CPPAPI int name (NDUINT16 nethandle,nd_usermsgbuf_t *msg, nd_handle h_listen)

	//定义消息发送函数
	#define ND_SENDEX(nethandle, msg, flag, h_listen)	nd_st_send(nethandle, (nd_usermsghdr_t *) (msg), flag, h_listen )
	#define ND_MSG_SEND(nethandle, msg,  h_listen)		nd_st_send(nethandle, (nd_usermsghdr_t *) (msg), ESF_NORMAL, h_listen )
	#define ND_MSG_WRITEBUF(nethandle, msg,  h_listen)	nd_st_send(nethandle, (nd_usermsghdr_t *) (msg), ESF_WRITEBUF, h_listen )
	#define ND_MSG_URGEN(nethandle, msg,  h_listen)		nd_st_send(nethandle, (nd_usermsghdr_t *) (msg), ESF_URGENCY, h_listen )
	#define ND_MSG_POST(nethandle, msg,  h_listen)		nd_st_send(nethandle, (nd_usermsghdr_t *) (msg), ESF_POST, h_listen )

	#define ND_CLOSE(nethandle, flag, h_listen)		nd_st_close(nethandle, flag, h_listen ) 
	#define ND_INSTALL_HANDLER						nd_srv_msgentry_install_st
	#define SET_CRYPT( sid, k,  size, h)			nd_st_set_crypt( sid, k,  size, h)

	#define ND_BROAD_CAST(h_listen, msg, sid)		nd_session_broadcast(h_listen, (nd_usermsghdr_t *)(msg), sid) 
	#define ND_SET_ONCONNECT_ENTRY(h, in,  out) nd_listensrv_set_entry_st(h, in, out)
	#define USER_NETMSG_FUNC	nd_usermsg_func1
	#define ND_GET_SESSION_ID(nethandle)		(nethandle)
#else 
	typedef nd_handle ND_SESSION_T ;
	
	#define MSG_ENTRY_INSTANCE(name) \
	int name (nd_handle nethandle,nd_usermsgbuf_t *msg, nd_handle h_listen) 
	#define MSG_ENTRY_DECLARE(name) \
	CPPAPI int name (nd_handle nethandle,nd_usermsgbuf_t *msg, nd_handle h_listen)

	//定义消息发送函数
	#define ND_SENDEX(nethandle, msg, flag, h_listen)	nd_sessionmsg_sendex(nethandle, msg, flag )
	#define ND_MSG_SEND(nethandle, msg,  h_listen)		nd_sessionmsg_send(nethandle, msg )
	#define ND_MSG_WRITEBUF(nethandle, msg,  h_listen)	nd_sessionmsg_writebuf(nethandle, msg )
	#define ND_MSG_URGEN(nethandle, msg,  h_listen)		nd_sessionmsg_send_urgen(nethandle, msg )
	#define ND_MSG_POST(nethandle, msg,  h_listen)		nd_sessionmsg_post(nethandle, msg )

	#define ND_CLOSE(nethandle, flag, h_listen)		nd_session_close(nethandle, flag ) 
	#define ND_INSTALL_HANDLER						nd_msgentry_install
	#define SET_CRYPT( sid, k,  size, h)			nd_connector_set_crypt( sid, k,  size)
	#define ND_BROAD_CAST(h_listen, msg, hsession)	nd_session_broadcast(h_listen, (nd_usermsghdr_t *)(msg), ((nd_netui_handle)hsession)->session_id) 
	#define ND_SET_ONCONNECT_ENTRY(h, in,  out) nd_listensrv_set_entry(h, (accept_callback)in,  (deaccept_callback)out)
	
	#define USER_NETMSG_FUNC	nd_usermsg_func		
	#define ND_GET_SESSION_ID(nethandle)		nd_session_getid(nethandle)
#endif

#endif
