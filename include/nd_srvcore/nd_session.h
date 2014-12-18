/* file : nd_srvnetio.h
 * nd server net io interface
 * 
 * version 1.0
 * all right reserved by neil duan 
 * 2008-4
 */

#ifndef _ND_SRVNETIO_H_
#define _ND_SRVNETIO_H_

//#include "nd_net/nd_netlib.h"
#include "nd_srvcore/client_map.h"

/*
 * 当每个客户端连接到服务器以后,会在服务器上产生一个对于的连接,
 * 我把这个连接称为一个session.
 */

typedef struct netui_info  *nd_session_handle ;

//close tcp connect
int tcp_client_close(struct nd_client_map* cli_map, int force) ;

//释放死连接
#define tcp_release_death_node(c, f) tcp_client_close((struct nd_client_map*)c, f) 
/* close connect*/
ND_SRV_API int nd_session_close(nd_handle session_handle, int force);
ND_SRV_API int nd_session_closeex(NDUINT16 session_id,nd_handle listen_handle);

static __INLINE__ NDUINT16  nd_session_getid(nd_handle session_handle) 
{
	return ((nd_session_handle)session_handle)->session_id ; 
}
//服务器的连接节点向客户端发送数据
//发送一个封包格式的数据
static __INLINE__ int  nd_session_sendex(nd_handle session_handle,nd_packhdr_t  *msg_buf, int flag) 
{
	return nd_connector_send(session_handle, msg_buf, flag) ;
}

//发送消息个数的数据
static __INLINE__ int nd_sessionmsg_sendex(nd_handle session_handle,nd_usermsghdr_t  *msg, int flag)
{
	ND_USERMSG_SYS_RESERVED(msg) = 0 ; //session , connector not allow send system message 
	nd_assert(session_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(session_handle, (nd_packhdr_t *)msg, flag) ;
}

//通话会话句柄发送非格式化的数据
#define nd_session_raw_write(session, data, size)	nd_connector_raw_write(session, data, size)
/*向客户端发送网络消息*/
#define nd_sessionmsg_send(session,msg) nd_sessionmsg_sendex((nd_handle)(session),(nd_usermsghdr_t*)(msg),ESF_NORMAL) 
#define nd_sessionmsg_writebuf(session,msg) nd_sessionmsg_sendex((nd_handle)(session),(nd_usermsghdr_t*)(msg),ESF_WRITEBUF) 
#define nd_sessionmsg_send_urgen(session,msg) nd_sessionmsg_sendex((nd_handle)(session),(nd_usermsghdr_t*)(msg),ESF_URGENCY) 
#define nd_sessionmsg_post(session,msg) nd_sessionmsg_sendex((nd_handle)(session),(nd_usermsghdr_t*)(msg),ESF_POST)
#define nd_session_valid nd_connector_valid

/* 发送缓冲操作*/
ND_SRV_API int nd_session_flush_sendbuf(nd_handle session_handle, int flag)  ;

//broadcast netmessage 
// @send_sid session id of sender 
ND_SRV_API int nd_session_broadcast(nd_handle listen_handle, nd_usermsghdr_t *msg) ;

ND_SRV_API int nd_session_broadcast_ex(nd_handle listen_handle, nd_usermsghdr_t *msg, NDUINT16 except_id, NDUINT8 privage_level) ;

#define nd_session_flush(session)		nd_session_flush_sendbuf((nd_handle)session,0)
#define nd_session_flush_force(session)	nd_session_flush_sendbuf((nd_handle)session,1)
#define nd_session_tryto_flush(session)	nd_session_flush_sendbuf((nd_handle)session,2)

static __INLINE__ nd_handle nd_session_getlisten(nd_handle session_handle)
{
	return (nd_handle) (((struct netui_info*)session_handle)->srv_root );
}


/* check connection is timeout return 1 timeout need to be close*/
int check_operate_timeout(nd_handle nethandle, ndtime_t tmout) ;

int tryto_close_tcpsession(nd_session_handle nethandle, ndtime_t connect_tmout ) ;
int _tcp_session_update(nd_session_handle handle);
#endif
