/*
 * file applib.h
 *
 * 服务器程序运用框架，对网络服务器SDK的简单封装
 *
 * create by duan 
 *
 * 2010/12/15 17:38:32
 */


#ifndef __APPLIB_H__
#define __APPLIB_H__

#pragma warning (disable: 4355)

#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"
#include "nd_srvcore/nd_srvlib.h"

#include "ndstl/nd_allocator.h"
#include "ndstl/nd_new.h"


#include "ndapplib/nd_lockhelper.h"
#include "ndapplib/nd_cmmgr.h"
#include "ndapplib/nd_connector.h"
#include "ndapplib/nd_listener.h"
#include "ndapplib/nd_msgpacket.h"
#include "ndapplib/nd_object.h"
#include "ndapplib/nd_objmgr.h"
#include "ndapplib/nd_session.h"
#include "ndapplib/nd_instance.h"
#include "ndapplib/nd_affair.h"
#include "ndapplib/nd_alarms.h"


#include "ndapplib/ndsingleton.h"
#include "ndapplib/nd_iBaseObj.h"
//#include "pg_config.h"

typedef int (*exit_app_func)(int flag) ;

static __INLINE__ void nd_pause()
{
	printf(("press ANY key to continue\n") );
	getch() ;
}

CPPAPI int nd_end_server(int force);

CPPAPI int system_signals_init() ;
CPPAPI int wait_services() ;
//CPPAPI int read_config(const char *file, struct srv_config *readcfg) ;

CPPAPI void exit_instance(int flag) ;
#ifdef ND_UNIX
//CPPAPI int installed_sig_handle(void);
//CPPAPI int wait_signal_entry();
//CPPAPI int block_signal(void) ;
//char *get_signal_desc(int signo);
//CPPAPI int unblock_signal(void) ;
//CPPAPI int ignore_all_signal(void) ;

CPPAPI int nd_wait_terminate_signals();
CPPAPI int nd_signals_init();

#else
CPPAPI BOOL WINAPI winclose_entry(DWORD dwevent);
#endif //_WINDOWS

//set user appliction exit function
//CPPAPI void app_exit_entry(exit_app_func func) ;
CPPAPI void _error_exit(const char *file, int line,const char *stm,...) ;
CPPAPI void nd_set_exit_callback(exit_app_func func) ;
CPPAPI void nd_instance_exit(int flag);

CPPAPI void nd_sys_exit(int exitcode) ;


CPPAPI int send_error_ack(nd_handle hconnect, int errcode);

#if defined(_MSC_VER)
#define error_exit(msg,...) _error_exit(__FILE__, __LINE__ ,msg,__VA_ARGS__)
#else 
#define error_exit(fmt,arg...) _error_exit(__FILE__ , __LINE__ , fmt,##arg)
#endif

CPPAPI int set_mp() ;
//定义消息发送函数
#define ND_SENDEX(nethandle, msg, flag, h_listen)	nd_sessionmsg_sendex(nethandle, msg, flag )
#define ND_MSG_SEND(nethandle, msg,  h_listen)		nd_sessionmsg_send(nethandle, msg )
#define ND_MSG_WRITEBUF(nethandle, msg,  h_listen)	nd_sessionmsg_writebuf(nethandle, msg )
#define ND_MSG_URGEN(nethandle, msg,  h_listen)		nd_sessionmsg_send_urgen(nethandle, msg )
#define ND_MSG_POST(nethandle, msg,  h_listen)		nd_sessionmsg_post(nethandle, msg )

#define ND_CLOSE(nethandle, flag, h_listen)		nd_session_close(nethandle, flag ) 
//#define ND_INSTALL_HANDLER						nd_msgentry_install
#define SET_CRYPT( sid, k,  size, h)			nd_connector_set_crypt( sid, k,  size)
#define ND_BROAD_CAST(h_listen, msg) nd_sendto_all((nd_usermsghdr_t *)msg, h_listen,0)
#define ND_SET_ONCONNECT_ENTRY(h, in,  out)		nd_listensrv_set_entry(h, (accept_callback)in,  (deaccept_callback)out)

#define USER_NETMSG_FUNC	nd_usermsg_func		
#define ND_GET_SESSION_ID(nethandle)		nd_session_getid(nethandle)

static inline int nd_check_coming_our_server(nd_usermsgbuf_t *msg)
{
	return ND_USERMSG_SYS_RESERVED(msg) ? 1: 0 ;
}
//定义消息处理函数
#define MSG_ENTRY_INSTANCE(name) \
	int name (nd_handle nethandle,nd_usermsgbuf_t *msg, nd_handle h_listen) 

//申明消息处理函数
#define MSG_ENTRY_DECLARE(name) \
	CPPAPI int name (nd_handle nethandle,nd_usermsgbuf_t *msg, nd_handle h_listen)


CPPAPI int nd_get_private_certificate_version(void);
CPPAPI char *nd_get_publickey_md5(void);

CPPAPI char* nd_calc_privatekey_md5(char text[33]);
CPPAPI R_RSA_PRIVATE_KEY *nd_get_privatekey(void) ;

#define  ND_SAFE_DESTROY_OBJ(obj)	\
	if(obj) {						\
		obj->Destroy() ;			\
		delete obj ;				\
		obj = 0 ;					\
	}

#include "ndapplib/nd_sysmsg.h"
#endif
