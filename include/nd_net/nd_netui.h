/* file nd_netui.h
 * define net user interface
 * 
 *
 * neil duan 
 * all right reserved 2008
 */

/* 
 * net ui function 
 * 
 */
#ifndef _ND_NETUI_H_
#define _ND_NETUI_H_

#include "nd_net/byte_order.h"
#include "nd_net/nd_sock.h"
#include "nd_net/nd_tcp.h"
#include "nd_net/nd_udthdr.h"
#include "nd_net/nd_udt.h"
#include "nd_net/nd_netobj.h"
#include "nd_crypt/nd_crypt.h"
//#include "nd_common/nd_common.h"



/*from handle to struct */
static __INLINE__ struct netui_info *nd_get_netui(nd_handle handle)
{
	nd_assert(((struct tag_nd_handle*)handle)->type==NDHANDLE_TCPNODE || ((struct tag_nd_handle*)handle)->type==NDHANDLE_UDPNODE) ;	
	return (struct netui_info *)handle ;
};

/* send packet 
 * @net_handle 
 * @nd_msgui_buf message data 
 * @flag ref send_flag
 * return value: 
				on error return -1 ,else return send data len ,
				send-data-len = msg_buf->data_len+sizeof(msgid + param).
				It is data length in effect
 */
ND_NET_API int nd_connector_send(nd_handle net_handle, nd_packhdr_t *msg_hdr, int flag) ;

/* send common data stream , not ND message struct data 
 * but only for tcp
 */

ND_NET_API int nd_connector_send_stream(nd_handle net_handle, void* data, size_t len, int flag);

/*
 * write raw-data to socket, can not used with nd_connector_send.
 */
ND_NET_API int nd_connector_raw_write(nd_handle net_handle , void *data, size_t size) ;
/* connect remote host 
 * @port remote port
 * @host host name
 * @protocol connect protocol(TCP/UDT) reference enum ND_NET_PROTOCOL
 * on error return NULL ,else return net connect handle 
 */
//ND_NET_API nd_handle nd_connector_open(char *host,int port, int protocol) ;

//transfer and call user install message handler 
ND_NET_API int handle_recv_data(nd_netui_handle node, nd_handle h_listen) ;


ND_NET_API int _packet_handler(nd_netui_handle node,nd_packhdr_t *msg, nd_handle h_listen) ;
ND_NET_API int nd_dft_packet_handler(nd_netui_handle node,void *data , size_t data_len , nd_handle h_listen) ;

/*
 * connect remote host  
			nd_handle connector =  nd_object_create("tcp-connector") // or  nd_object_create("udt-connector") ;
			if(!connector) {
				//error ;
			}
			if(-1==nd_connector_open(connector, host, port) )
				// error
			nd_connector_close(connector, 0 ) ; // or nd_object_destroy(connector) ;

 */
ND_NET_API int nd_connector_open(nd_handle net_handle,const char *host, int port, struct nd_proxy_info *proxy);

/*
 * reconnect
 */
ND_NET_API int nd_reconnect(nd_handle net_handle, ndip_t ip, int port, struct nd_proxy_info *proxy) ;
ND_NET_API int nd_reconnectex(nd_handle net_handle, const char *host, int port, struct nd_proxy_info *proxy) ;

/*close connect (only used in client (connect)) */
ND_NET_API int nd_connector_close(nd_handle net_handle, int force) ;

ND_NET_API ndsocket_t nd_connector_fd(nd_handle net_handle);

ND_NET_API int nd_connector_valid(nd_netui_handle net_handle) ;
/* reset connector
 * reset connect status ,but the crypt key will not destroy
 */
ND_NET_API int nd_connector_reset(nd_handle net_handle) ;

int _connector_destroy(nd_handle net_handle, int force) ;
/*
 * update client-connector
 * on error return -1
 * return 0 time 
 * else return recv data length
 * on error return -1,check errorcode , 
 * return 0 nothing to be done(time out)
 * else success
 * if return -1 connect need to be closed
 */
ND_NET_API int nd_connector_update(nd_handle net_handle, ndtime_t timeout) ;

/* get send buffer free space*/
ND_NET_API size_t nd_connector_sendlen(nd_handle net_handle);
ND_NET_API size_t nd_connector_data_in_window(nd_handle net_handle);

ND_NET_API void nd_connector_set_crypt(nd_handle net_handle, void *key, int size);
ND_NET_API void* nd_connector_get_crypt(nd_handle net_handle, int *size);

ND_NET_API  int nd_connector_check_crypt(nd_handle net_handle) ;
/*
 * wait net message 
 * on error return -1 
 * return 0 timeout 
 * else return the data length
 *
 */
ND_NET_API int nd_connector_waitmsg(nd_handle net_handle, nd_packetbuf_t *msg_hdr, ndtime_t tmout);


/* reveive data from socket ,without format
 *
 * return value : 0 connect closed ,-1 error ,else data len
 */
ND_NET_API int nd_connector_raw_waitdata(nd_handle net_handle, void *buf, size_t size, ndtime_t timeout) ;

ND_NET_API int nd_packet_encrypt(nd_handle net_handle, nd_packetbuf_t *msgbuf);
ND_NET_API int nd_packet_decrypt(nd_handle net_handle, nd_packetbuf_t *msgbuf);

ND_NET_API int nd_packet_encrypt_key(nd_cryptkey *pcrypt_key, nd_packetbuf_t *msgbuf);
ND_NET_API int nd_packet_decrypt_key(nd_cryptkey *pcrypt_key,nd_packetbuf_t *msgbuf);

ND_NET_API void nd_teaKeyToNetorder(tea_k *outkey, tea_k *fromHostkey) ;
ND_NET_API void nd_teaKeyToHostorder(tea_k *outkey, tea_k *fromNetKey) ;

//set data had handled 
//@size handled data length
ND_NET_API int nd_connector_handled_data(nd_handle net_handle, size_t size) ;


//hook raw data 
//please careful return value of data_entry
ND_NET_API data_in_entry nd_hook_data(nd_handle h, data_in_entry data_entry) ;

/*hook package 
*/
ND_NET_API net_msg_entry nd_hook_packet(nd_handle h, net_msg_entry msg_entry) ;
// set timeout 
ND_NET_API int nd_connector_set_timeout(nd_netui_handle net_handle, int seconds) ;

ND_NET_API ndip_t nd_net_getip(nd_handle h);
ND_NET_API ndport_t nd_net_getport(nd_handle h);

ND_NET_API ndip_t nd_net_peer_getip(nd_handle h);
ND_NET_API ndport_t nd_net_peer_getport(nd_handle h) ;

//set close or ignore when get unregister message 
ND_NET_API void nd_net_set_unregmsg_handler(nd_handle h, int isclosed) ;

ND_NET_API void nd_net_set_unauthorize_handler(nd_handle h, int isclosed) ;

ND_NET_API void nd_connector_set_userdata(nd_netui_handle net_handle, void *p);

ND_NET_API void* nd_connector_get_userdata(nd_netui_handle net_handle);

ND_NET_API int nd_net_sysmsg_hander(nd_netui_handle node, nd_sysresv_pack_t *pack);

// add function call when close
static __INLINE__ int nd_connector_add_close_callback(nd_handle handle,nd_object_destroy_callback callback, void *param) 
{
	return nd_object_add_destroy_cb(handle, callback, param, 1) ;
}

static __INLINE__  int nd_connector_del_close_callback(nd_handle handle,nd_object_destroy_callback callback, void *param) 
{
	return nd_object_del_destroy_cb(handle, callback, param) ;
}

#endif 

