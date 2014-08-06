/* file nd_netui.h
 * define net user interface
 * 
 *
 * neil duan 
 * all right reserved 2008
 */

/* 定义统一的网络用户接口,主要是针对每个连接的
 * 服务器端的启动结束接口不在此范围.
 * 目的是为了让udt和tcp使用统一的接口函数
 * 使用统一的消息结构.
 * 
 * 这里的定义都是面向网络层的,用户在使用消息发送函数时,
 * 请勿直接使用这里定的 nd_connector_send() 函数,这里发送的是封包,
 * 请使用 nd_msgentry.h 定义的 nd_connectmsg_send** 系列函数
 */
#ifndef _ND_NETUI_H_
#define _ND_NETUI_H_

#include "nd_net/byte_order.h"
#include "nd_net/nd_sock.h"
#include "nd_net/nd_tcp.h"
#include "nd_net/nd_udthdr.h"
#include "nd_net/nd_udt.h"
#include "nd_net/nd_netobj.h"
//#include "nd_common/nd_common.h"

/*enum ND_NET_PROTOCOL
{
	ND_TCP_STREAM = 0 ,		//使用tcp协议连接
	ND_UDT_STREAM 	//,		//使用udt的stream协议
	//ND_UDT_DATAGRAM			//使用udt的datagram协议
};*/


/*从连接句柄得到网络接口信息*/
static __INLINE__ struct netui_info *nd_get_netui(nd_handle handle)
{
	nd_assert(((struct tag_nd_handle*)handle)->type==NDHANDLE_TCPNODE || ((struct tag_nd_handle*)handle)->type==NDHANDLE_UDPNODE) ;	
	return (struct netui_info *)handle ;
};

//把udt的stream变成nd的网络消息
//ND_NET_API 

/* 发送网络消息 
 * @net_handle 网络连接的句柄,指向struct nd_tcp_node(TCP连接)
 *		或者ndudt_socket(UDT)节点
 * @nd_msgui_buf 发送消息内容
 * @flag ref send_flag
 * return value: 
				on error return -1 ,else return send data len ,
				send-data-len = msg_buf->data_len+sizeof(msgid + param).
				It is data length in effect
 */
ND_NET_API int nd_connector_send(nd_handle net_handle, nd_packhdr_t *msg_hdr, int flag) ;

/*直接在nd_handle 打开的socket上写数据
 *不能和nd_connector_send混和用,因为它会破坏nd_packhdr_t 格式
 * 这个函数主要处理不用nd_packhdr_t 封装的数据
 */
ND_NET_API int nd_connector_raw_write(nd_handle net_handle , void *data, size_t size) ;
/* connect remote host 
 * @port remote port
 * @host host name
 * @protocol connect protocol(TCP/UDT) reference enum ND_NET_PROTOCOL
 * on error return NULL ,else return net connect handle 
 */
//ND_NET_API nd_handle nd_connector_open(char *host,int port, int protocol) ;

//处理接受好的网络数据
ND_NET_API int handle_recv_data(nd_netui_handle node, nd_handle h_listen) ;


int _packet_handler(nd_netui_handle node,nd_packhdr_t *msg, nd_handle h_listen) ;
ND_NET_API int nd_dft_packet_handler(nd_netui_handle node,void *data , size_t data_len , nd_handle h_listen) ;

/*
 * 把已经创建的句柄连接到主机host的端口 port上
 * 用法: 
			nd_handle connector =  nd_object_create("tcp-connector") // or  nd_object_create("udt-connector") ;
			if(!connector) {
				//error ;
			}
			if(-1==nd_connector_open(connector, host, port) )
				// error
			nd_connector_close(connector, 0 ) ; // or nd_object_destroy(connector) ;

 */
ND_NET_API int nd_connector_open(nd_handle net_handle,char *host, int port, struct nd_proxy_info *proxy);

/*
 * 重新连接到一个新的服务器
 */
ND_NET_API int nd_reconnect(nd_handle net_handle, ndip_t ip, int port, struct nd_proxy_info *proxy) ;
ND_NET_API int nd_reconnectex(nd_handle net_handle, char *host, int port, struct nd_proxy_info *proxy) ;

/*close connect (only used in client (connect)) */
ND_NET_API int nd_connector_close(nd_handle net_handle, int force) ;

ND_NET_API ndsocket_t nd_connector_fd(nd_handle net_handle);

ND_NET_API int nd_connector_valid(nd_netui_handle net_handle) ;
/* reset connector
 * 关闭网络连接并重新初始化连接状态,但保留用户设置信息(消息处理函数,加密密钥)
 */
ND_NET_API int nd_connector_reset(nd_handle net_handle) ;

/*销毁连接器*/
int _connector_destroy(nd_handle net_handle, int force) ;
/*更新,驱动网络模块, 处理连接消息
 * 主要是用在处理connect端,server端不在次定义
 * 出错放回-1,网络需要被关闭
 * 返回0等待超时
 * on error return -1,check errorcode , 
 * return 0 nothing to be done(time out)
 * else success
 * if return -1 connect need to be closed
 */
ND_NET_API int nd_connector_update(nd_handle net_handle, ndtime_t timeout) ;

/* 得到发送缓冲的空闲长度*/
ND_NET_API size_t nd_connector_sendlen(nd_handle net_handle);

ND_NET_API void nd_connector_set_crypt(nd_handle net_handle, void *key, int size);
ND_NET_API void* nd_connector_get_crypt(nd_handle net_handle, int *size);

ND_NET_API  int nd_connector_check_crypt(nd_handle net_handle) ;
/*等待一个网络消息消息
 *如果有网络消息到了则返回消息的长度(整条消息的长度,到来的数据长度)
 *超时,出错返回-1.网络被关闭返回0
 *
 */
ND_NET_API int nd_connector_waitmsg(nd_handle net_handle, nd_packetbuf_t *msg_hdr, ndtime_t tmout);


/*在connect上等待原始的网络数据
 *
 * return value : 0 connect closed ,-1 error ,else data len
 */
ND_NET_API int nd_connector_raw_waitdata(nd_handle net_handle, void *buf, size_t size, ndtime_t timeout) ;

ND_NET_API int nd_packet_encrypt(nd_handle net_handle, nd_packetbuf_t *msgbuf);
ND_NET_API int nd_packet_decrypt(nd_handle net_handle, nd_packetbuf_t *msgbuf);

//设置数据处理完毕
//@size 被处理的数据长度
ND_NET_API int nd_connector_handled_data(nd_handle net_handle, size_t size) ;


//设置网络数据处理函数
//please careful return value of data_entry
ND_NET_API data_in_entry nd_hook_data(nd_handle h, data_in_entry data_entry) ;

/*设置封包拦截函数
*如果用户想自行处理网络封包,就可以拦截封包处理函数
* 然后在 hook_func 中处理网络封包
* nd_msgentry_install 函数将不起作用
*/
ND_NET_API net_msg_entry nd_hook_packet(nd_handle h, net_msg_entry msg_entry) ;

ND_NET_API int nd_connector_set_timeout(nd_netui_handle net_handle, int seconds) ;

/*得到IP和端口*/
static __INLINE__ ndip_t nd_net_getip(nd_handle h)
{
	return nd_sock_getip(((struct nd_netsocket*)h)->fd);
}
static __INLINE__ ndport_t nd_net_getport(nd_handle h)
{
	return nd_sock_getport(((struct nd_netsocket*)h)->fd);

}

ND_NET_API ndip_t nd_net_peer_getip(nd_handle h);
ND_NET_API ndport_t nd_net_peer_getport(nd_handle h) ;

//设置当接收到没有注册的消息时是否关闭连接
ND_NET_API void nd_net_set_unregmsg_handler(nd_handle h, int isclosed) ;
//设置当接收到没有授权的消息时是否关闭连接
ND_NET_API void nd_net_set_unauthorize_handler(nd_handle h, int isclosed) ;

ND_NET_API void nd_connector_set_userdata(nd_netui_handle net_handle, void *p);

ND_NET_API void* nd_connector_get_userdata(nd_netui_handle net_handle);

int nd_net_sysmsg_hander(nd_netui_handle node, nd_sysresv_pack_t *pack);


#endif 

