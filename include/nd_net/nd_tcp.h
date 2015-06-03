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

#define SENDBUF_PUSH	128		/*发送缓冲上限,如果数据达到这个数即可发送*/
#define SENDBUF_TMVAL	50		/*两次缓冲清空时间间隔,超过这个间隔将清空*/
#define ALONE_SEND_SIZE 64		/*发送数据长度大于这个数字将不缓冲*/

/* 
 * WAIT_WRITABLITY_TIME 等待一个连接变为可写的时间.
 * 如果设置太长可能会使系统挂起,影响整个服务器的吞吐率.
 * 可以使用nd_set_wait_writablity_time 函数来重新设置等待时间
 */
#ifdef ND_DEBUG
#define WAIT_WRITABLITY_TIME	-1		/*无限等待*/
#else 
#define WAIT_WRITABLITY_TIME    1000	/*等待socke可写的时间*/
#endif

//TCP连接状态
enum ETCP_CONNECT_STATUS{
	ETS_DEAD = 0 ,			//无连接(或者等待被释放)
	ETS_ACCEPT,				//等待连接进入(IOCP)
	ETS_CONNECTED ,			//连接成功
	ETS_TRYTO_CLOSE	,		//等待关闭连接need to send data can be receive data
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
ND_NET_API int nd_tcpnode_connect(const char *host, int port, struct nd_tcp_node *node,struct nd_proxy_info *proxy);	//连接到主机
ND_NET_API int nd_tcpnode_close(struct nd_tcp_node *node,int force);				//关闭连接
ND_NET_API int nd_tcpnode_send(struct nd_tcp_node *node, nd_packhdr_t *msg_buf, int flag) ;	//发送网络消息 flag ref send_flag
ND_NET_API int nd_tcpnode_read(struct nd_tcp_node *node) ;		//读取数据,0 timeout ,-1 error or closed ,else datalen
ND_NET_API int _tcpnode_push_sendbuf(struct nd_tcp_node *conn_node, int force) ;	//发送缓冲中的数据
ND_NET_API int nd_tcpnode_tryto_flush_sendbuf(struct nd_tcp_node *conn_node) ;	//发送缓冲中的数据

ND_NET_API int tcpnode_parse_recv_msgex(struct nd_tcp_node *node,NDNET_MSGENTRY msg_entry , void *param) ;
ND_NET_API void nd_tcpnode_init(struct nd_tcp_node *conn_node) ;	//初始化连接节点

ND_NET_API void nd_tcpnode_deinit(struct nd_tcp_node *conn_node)  ;
ND_NET_API int nd_tcpnode_sendlock_init(struct nd_tcp_node *conn_node) ;
ND_NET_API void nd_tcpnode_sendlock_deinit(struct nd_tcp_node *conn_node) ;

/*重置TCP连接:关闭当前连接和,清空缓冲和各种状态;
 * 但是保存用户的相关设置,消息处理函数和加密密钥
 */
ND_NET_API void nd_tcpnode_reset(struct nd_tcp_node *conn_node)  ;
ND_NET_API int nd_socket_wait_writablity(ndsocket_t fd,int timeval) ;
ND_NET_API  int _set_socket_addribute(ndsocket_t sock_fd) ;
ND_NET_API void _tcp_connector_init(struct nd_tcp_node *conn_node) ;

/* set ndnet connector default options */
void _set_ndtcp_conn_dft_option(ndsocket_t sock_fd);
/* set ndnet connector default options */
void _set_ndtcp_session_dft_option(ndsocket_t sock_fd);

/*以下函数锁住发送
 * 在用户程序中不需要显式的调用
 */


#define TCPNODE_FD(conn_node) (((struct nd_tcp_node*)(conn_node))->fd )
#define TCPNODE_READ_AGAIN(conn_node) (conn_node)->read_again
#define CURRENT_IS_CRYPT(conn_node) (((struct nd_tcp_node*)(conn_node))->is_crypt_packet )

//TCP状态操作宏
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

static __INLINE__ void nd_tcpnode_flush_sendbuf(nd_netui_handle node)	
{
	nd_send_lock(node) ;
	_tcpnode_push_sendbuf((struct nd_tcp_node *)node,0) ;
	nd_send_unlock(node) ;
}

static __INLINE__ void nd_tcpnode_flush_sendbuf_force(nd_netui_handle node)	
{
	nd_send_lock(node) ;
	_tcpnode_push_sendbuf((struct nd_tcp_node *)node,1) ;
	nd_send_unlock(node) ;
}

ND_NET_API int nd_set_wait_writablity_time(int newtimeval) ;
ND_NET_API int nd_get_wait_writablity_time() ;


/*等待一个网络消息消息
 *如果有网络消息到了则返回消息的长度(整条消息的长度,到来的数据长度)
 *超时,出错返回-1,此时网络需要被关闭
 *返回0表示无数据到了
 */
ND_NET_API int tcpnode_wait_msg(struct nd_tcp_node *node, ndtime_t tmout);

ND_NET_API int _socket_send(struct nd_tcp_node *node,void *data , size_t len) ;
#endif

