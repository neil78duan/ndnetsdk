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

#define ND_NAME_SIZE 40

/*client connect map on server*/
struct nd_client_map
{
	struct nd_tcp_node connect_node;		//socket connect node 
	struct list_head map_list;
	char __sendbuf[ND_NETBUF_SIZE];
	char __recvbuf[ND_NETBUF_SIZE];
};

/*client connect map is server*/
struct nd_udtcli_map
{
	nd_udt_node connect_node;		//socket connect node 
	struct list_head map_list;
	char __sendbuf[ND_NETBUF_SIZE];
	char __recvbuf[ND_NETBUF_SIZE];
};

#if !defined(ND_UNIX)  

#if _MSC_VER < 1300 // 1200 == VC++ 6.0
#else 
#include "winsock2.h"

#endif

struct nd_client_map_iocp;

struct ND_OVERLAPPED_PLUS
{
	WSAOVERLAPPED overlapped;
	struct nd_client_map_iocp* client_addr; //Pointer to client
};

struct nd_client_map_iocp
{
	struct nd_client_map  __client_map;		//common client map ,MUST in first 	
	size_t			total_send;				//write buf total
	size_t			send_len;				// had been send length
	int				__wait_buffers;		//waiting sending buffer number
	ndtime_t		schedule_close_time;				//set close time 
	ndatomic_t		in_sending;			//between WSASend () and iocp_callback()
	struct ND_OVERLAPPED_PLUS  ov_read;
	struct ND_OVERLAPPED_PLUS  ov_write;
	WSABUF			wsa_readbuf;			// Temporary buffer used for reading
	WSABUF			wsa_writebuf;			// Temporary buffer used for writing
};

#endif
// 
// enum ePlayerState {
// 	EPS_NONE = 0 ,
// 	EPS_CONNECT,
// 	EPS_LOGIN
// };
//header of playerdata
// typedef struct player_header{
// 	nd_netui_handle		h_session ;		//handle of connection session
// 	NDUINT32			id ;
// 	int					status ;		//ref ePlayerState
// }player_header_t ;


//typedef void *nd_cli_handle;
typedef struct netui_info *nd_climap_handle;


ND_SRV_API struct list_head *get_self_list(nd_climap_handle cli_handle);

ND_SRV_API  void nd_tcpcm_init(struct nd_client_map *client_map, nd_handle h_listen);
ND_SRV_API void nd_client_map_destroy(struct nd_client_map *client_map);

ND_SRV_API size_t nd_getclient_hdr_size(int iomod);

ND_SRV_API void *nd_session_getdata(nd_netui_handle session);
//-----------------end client map file

/*
 * we called client map as session
 */

typedef struct netui_info  *nd_session_handle ;

//close tcp connect
int tcp_client_close(struct nd_client_map* cli_map, int force) ;


#define tcp_release_death_node(c, f) tcp_client_close((struct nd_client_map*)c, f) 
/* close connect*/
ND_SRV_API int nd_session_close(nd_handle session_handle, int force);
ND_SRV_API int nd_session_closeex(NDUINT16 session_id,nd_handle listen_handle);

static __INLINE__ NDUINT16  nd_session_getid(nd_handle session_handle) 
{
	return ((nd_session_handle)session_handle)->session_id ; 
}
//send message 
static __INLINE__ int  nd_session_sendex(nd_handle session_handle,nd_packhdr_t  *msg_buf, int flag) 
{
	return nd_connector_send(session_handle, msg_buf, flag) ;
}

static __INLINE__ int nd_sessionmsg_sendex(nd_handle session_handle,nd_usermsghdr_t  *msg, int flag)
{
	ND_USERMSG_SYS_RESERVED(msg) = 0 ; //session , connector not allow send system message 
	nd_assert(session_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(session_handle, (nd_packhdr_t *)msg, flag) ;
}

#define nd_session_raw_write(session, data, size)	nd_connector_raw_write(session, data, size)
#define nd_sessionmsg_send(session,msg) nd_sessionmsg_sendex((nd_handle)(session),(nd_usermsghdr_t*)(msg),ESF_NORMAL) 
#define nd_sessionmsg_writebuf(session,msg) nd_sessionmsg_sendex((nd_handle)(session),(nd_usermsghdr_t*)(msg),ESF_WRITEBUF) 
#define nd_sessionmsg_send_urgen(session,msg) nd_sessionmsg_sendex((nd_handle)(session),(nd_usermsghdr_t*)(msg),ESF_URGENCY) 
#define nd_sessionmsg_post(session,msg) nd_sessionmsg_sendex((nd_handle)(session),(nd_usermsghdr_t*)(msg),ESF_POST)
#define nd_session_valid nd_connector_valid

ND_SRV_API int nd_session_flush_sendbuf(nd_handle session_handle, int flag)  ;

//broadcast netmessage 
// @send_sid session id of sender 
//ND_SRV_API int nd_session_broadcast(nd_handle listen_handle, nd_usermsghdr_t *msg) ;

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
