/* file nd_connector_open.h
 * define connect struct over TCP
 * connect by net message on tcp/udp
 *
 * neil duan 
 * all right reserved 2007
 */

#ifndef _ND_TCP_H_
#define _ND_TCP_H_

#include "nd_net/byte_order.h"
#include "nd_net/nd_sock.h"
//#include "nd_common/nd_common.h"
#include "nd_net/nd_netcrypt.h"

#include "nd_net/nd_netpack.h"

#include "nd_net/nd_netobj.h"

#define SENDBUF_PUSH	128		/* if buffer data length is more than this ,force flush*/
#define SENDBUF_TMVAL	50		/**/
#define ALONE_SEND_SIZE 64		/*if send data length is more than this ,not store to buffer*/

/* 
 * WAIT_WRITABLITY_TIME wait socket writable timeout value
 */
#ifdef ND_DEBUG
#define WAIT_WRITABLITY_TIME	-1		/*wait infinite*/
#else 
#define WAIT_WRITABLITY_TIME    1000	/*wait socket writable timeout value*/
#endif

//TCP connect statu 
enum ETCP_CONNECT_STATUS{
	ETS_DEAD = 0 ,			//dead
	ETS_ACCEPT,				//accept in 
	ETS_CONNECTED ,			// success
	ETS_TRYTO_CLOSE	,		//need to send data can be receive data
	ETS_RESET,				//need to be reset (force close). socket would be set reset status on error when write data
	ETS_CLOSED				//closed
};

/* socket connect info struct */
struct nd_tcp_node{
	ND_OBJ_BASE ;
	ND_SOCKET_OBJ_BASE ;
	ND_CONNECTOR_BASE ;
};

#define check_connect_valid(node) (((struct nd_tcp_node*)(node))->fd > 0 && ((struct nd_tcp_node*)(node))->status==ETS_CONNECTED)

//connect to host
ND_NET_API int nd_tcpnode_connect(const char *host, int port, struct nd_tcp_node *node,struct nd_proxy_info *proxy);
ND_NET_API int nd_tcpnode_close(struct nd_tcp_node *node,int force);
ND_NET_API int nd_tcpnode_send(struct nd_tcp_node *node, nd_packhdr_t *msg_buf, int flag) ;	
ND_NET_API int nd_tcpnode_read(struct nd_tcp_node *node);		//read ,0 timeout ,-1 error or closed ,else datalen
ND_NET_API int nd_tcpnode_stream_send(struct nd_tcp_node *node, void*data, size_t len, int flag);	//send stream data  flag ref send_flag

ND_NET_API int _tcpnode_push_sendbuf(struct nd_tcp_node *conn_node) ;	//flush send buffer
ND_NET_API int _tcpnode_push_force(struct nd_tcp_node *conn_node);	//
ND_NET_API int nd_tcpnode_tryto_flush_sendbuf(struct nd_tcp_node *conn_node) ;	//
static __INLINE__ int nd_tcpnode_flush_sendbuf(nd_netui_handle node)
{
	int ret = 0;
	nd_send_lock(node);
	ret = _tcpnode_push_sendbuf((struct nd_tcp_node *)node);
	nd_send_unlock(node);
	return ret;
}
static __INLINE__ int nd_tcpnode_flush_sendbuf_force(nd_netui_handle node)
{
	int ret = 0;
	nd_send_lock(node);
	ret = _tcpnode_push_force((struct nd_tcp_node *)node);
	nd_send_unlock(node);
	return ret;

}

ND_NET_API void nd_tcpnode_init(struct nd_tcp_node *conn_node) ;	

ND_NET_API void nd_tcpnode_deinit(struct nd_tcp_node *conn_node)  ;
//ND_NET_API int nd_tcpnode_sendlock_init(struct nd_tcp_node *conn_node) ;
//ND_NET_API void nd_tcpnode_sendlock_deinit(struct nd_tcp_node *conn_node) ;

ND_NET_API void nd_tcpnode_reset(struct nd_tcp_node *conn_node)  ;
ND_NET_API int nd_socket_wait_writablity(ndsocket_t fd,int timeval) ;
ND_NET_API void _tcp_connector_init(struct nd_tcp_node *conn_node) ;

/* set ndnet connector default options */
void _set_ndtcp_conn_dft_option(ndsocket_t sock_fd);
/* set ndnet connector default options */
void _set_ndtcp_session_dft_option(ndsocket_t sock_fd);


int _wait_read_msg(struct nd_tcp_node *node, ndtime_t tmout);
int _wait_write_msg(struct nd_tcp_node *node, ndtime_t tmout);

#define TCPNODE_FD(conn_node) (((struct nd_tcp_node*)(conn_node))->fd )
#define TCPNODE_READ_AGAIN(conn_node) (conn_node)->read_again
#define CURRENT_IS_CRYPT(conn_node) (((struct nd_tcp_node*)(conn_node))->is_crypt_packet )

//TCP node operate macro
#define TCPNODE_STATUS(conn_node) (((struct nd_tcp_node*)(conn_node))->status )
#define TCPNODE_SET_OK(conn_node) ((struct nd_tcp_node*)(conn_node))->status = ETS_CONNECTED
#define TCPNODE_IS_OK(conn_node) (((struct nd_tcp_node*)(conn_node))->status == ETS_CONNECTED)
#define TCPNODE_CHECK_OK(conn_node) (((struct nd_tcp_node*)(conn_node))->status == ETS_CONNECTED)
#define TCPNODE_CHECK_CLOSED(conn_node) (((struct nd_tcp_node*)(conn_node))->status == ETS_TRYTO_CLOSE)
#define TCPNODE_SET_CLOSED(conn_node) (((struct nd_tcp_node*)(conn_node))->status = ETS_TRYTO_CLOSE)

#define TCPNODE_TRY_CALLBACK_WRITE(conn_node) do { \
	net_writable_callback w_callback = ((struct nd_tcp_node*)(conn_node))->writable_callback ; \
	if(w_callback) {		\
		nd_userdata_t _param = ((struct nd_tcp_node*)(conn_node))->writable_param ; \
		if(nd_socket_wait_writablity(TCPNODE_FD(conn_node),0) > 0) {\
			w_callback((nd_handle)conn_node,_param) ;	\
		}\
	}\
}while(0)


#if 0
#define CONNECT_CLOSED 0
#define TCPNODE_SET_RESET(conn_node) \
	do { (((struct nd_tcp_node*)(conn_node))->status = ETS_RESET); \
	nd_assert(CONNECT_CLOSED) ;}while(0)
#else 
#define TCPNODE_SET_RESET(conn_node) (((struct nd_tcp_node*)(conn_node))->status = ETS_RESET)
#endif 

#define TCPNODE_CHECK_RESET(conn_node) (((struct nd_tcp_node*)(conn_node))->status == ETS_RESET)

// 
// ND_NET_API int nd_set_wait_writablity_time(int newtimeval) ;
// ND_NET_API int nd_get_wait_writablity_time() ;


/*wait message ,
 * return 0 timeout,
 * on error return -1 
 * 
 */
//ND_NET_API int tcpnode_wait_msg(struct nd_tcp_node *node, ndtime_t tmout);

//ND_NET_API int _sys_socket_write(struct nd_tcp_node *node,void *data , size_t len) ;
#endif

