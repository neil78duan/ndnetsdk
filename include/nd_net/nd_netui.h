/* file nd_netui.h
 * define net user interface
 * 
 *
 * neil duan 
 * all right reserved 2008
 */

/* ����ͳһ�������û��ӿ�,��Ҫ�����ÿ�����ӵ�
 * �������˵����������ӿڲ��ڴ˷�Χ.
 * Ŀ����Ϊ����udt��tcpʹ��ͳһ�Ľӿں���
 * ʹ��ͳһ����Ϣ�ṹ.
 * 
 * ����Ķ��嶼������������,�û���ʹ����Ϣ���ͺ���ʱ,
 * ����ֱ��ʹ�����ﶨ�� nd_connector_send() ����,���﷢�͵��Ƿ��,
 * ��ʹ�� nd_msgentry.h ����� nd_connectmsg_send** ϵ�к���
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
	ND_TCP_STREAM = 0 ,		//ʹ��tcpЭ������
	ND_UDT_STREAM 	//,		//ʹ��udt��streamЭ��
	//ND_UDT_DATAGRAM			//ʹ��udt��datagramЭ��
};*/


/*�����Ӿ���õ�����ӿ���Ϣ*/
static __INLINE__ struct netui_info *nd_get_netui(nd_handle handle)
{
	nd_assert(((struct tag_nd_handle*)handle)->type==NDHANDLE_TCPNODE || ((struct tag_nd_handle*)handle)->type==NDHANDLE_UDPNODE) ;	
	return (struct netui_info *)handle ;
};

//��udt��stream���nd��������Ϣ
//ND_NET_API 

/* ����������Ϣ 
 * @net_handle �������ӵľ��,ָ��struct nd_tcp_node(TCP����)
 *		����ndudt_socket(UDT)�ڵ�
 * @nd_msgui_buf ������Ϣ����
 * @flag ref send_flag
 * return value: 
				on error return -1 ,else return send data len ,
				send-data-len = msg_buf->data_len+sizeof(msgid + param).
				It is data length in effect
 */
ND_NET_API int nd_connector_send(nd_handle net_handle, nd_packhdr_t *msg_hdr, int flag) ;

/*ֱ����nd_handle �򿪵�socket��д����
 *���ܺ�nd_connector_send�����,��Ϊ�����ƻ�nd_packhdr_t ��ʽ
 * ���������Ҫ������nd_packhdr_t ��װ������
 */
ND_NET_API int nd_connector_raw_write(nd_handle net_handle , void *data, size_t size) ;
/* connect remote host 
 * @port remote port
 * @host host name
 * @protocol connect protocol(TCP/UDT) reference enum ND_NET_PROTOCOL
 * on error return NULL ,else return net connect handle 
 */
//ND_NET_API nd_handle nd_connector_open(char *host,int port, int protocol) ;

//������ܺõ���������
ND_NET_API int handle_recv_data(nd_netui_handle node, nd_handle h_listen) ;


int _packet_handler(nd_netui_handle node,nd_packhdr_t *msg, nd_handle h_listen) ;
ND_NET_API int nd_dft_packet_handler(nd_netui_handle node,void *data , size_t data_len , nd_handle h_listen) ;

/*
 * ���Ѿ������ľ�����ӵ�����host�Ķ˿� port��
 * �÷�: 
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
 * �������ӵ�һ���µķ�����
 */
ND_NET_API int nd_reconnect(nd_handle net_handle, ndip_t ip, int port, struct nd_proxy_info *proxy) ;
ND_NET_API int nd_reconnectex(nd_handle net_handle, char *host, int port, struct nd_proxy_info *proxy) ;

/*close connect (only used in client (connect)) */
ND_NET_API int nd_connector_close(nd_handle net_handle, int force) ;

ND_NET_API ndsocket_t nd_connector_fd(nd_handle net_handle);

ND_NET_API int nd_connector_valid(nd_netui_handle net_handle) ;
/* reset connector
 * �ر��������Ӳ����³�ʼ������״̬,�������û�������Ϣ(��Ϣ������,������Կ)
 */
ND_NET_API int nd_connector_reset(nd_handle net_handle) ;

/*����������*/
int _connector_destroy(nd_handle net_handle, int force) ;
/*����,��������ģ��, ����������Ϣ
 * ��Ҫ�����ڴ���connect��,server�˲��ڴζ���
 * ����Ż�-1,������Ҫ���ر�
 * ����0�ȴ���ʱ
 * on error return -1,check errorcode , 
 * return 0 nothing to be done(time out)
 * else success
 * if return -1 connect need to be closed
 */
ND_NET_API int nd_connector_update(nd_handle net_handle, ndtime_t timeout) ;

/* �õ����ͻ���Ŀ��г���*/
ND_NET_API size_t nd_connector_sendlen(nd_handle net_handle);

ND_NET_API void nd_connector_set_crypt(nd_handle net_handle, void *key, int size);
ND_NET_API void* nd_connector_get_crypt(nd_handle net_handle, int *size);

ND_NET_API  int nd_connector_check_crypt(nd_handle net_handle) ;
/*�ȴ�һ��������Ϣ��Ϣ
 *�����������Ϣ�����򷵻���Ϣ�ĳ���(������Ϣ�ĳ���,���������ݳ���)
 *��ʱ,������-1.���类�رշ���0
 *
 */
ND_NET_API int nd_connector_waitmsg(nd_handle net_handle, nd_packetbuf_t *msg_hdr, ndtime_t tmout);


/*��connect�ϵȴ�ԭʼ����������
 *
 * return value : 0 connect closed ,-1 error ,else data len
 */
ND_NET_API int nd_connector_raw_waitdata(nd_handle net_handle, void *buf, size_t size, ndtime_t timeout) ;

ND_NET_API int nd_packet_encrypt(nd_handle net_handle, nd_packetbuf_t *msgbuf);
ND_NET_API int nd_packet_decrypt(nd_handle net_handle, nd_packetbuf_t *msgbuf);

//�������ݴ������
//@size ����������ݳ���
ND_NET_API int nd_connector_handled_data(nd_handle net_handle, size_t size) ;


//�����������ݴ�����
//please careful return value of data_entry
ND_NET_API data_in_entry nd_hook_data(nd_handle h, data_in_entry data_entry) ;

/*���÷�����غ���
*����û������д���������,�Ϳ������ط��������
* Ȼ���� hook_func �д���������
* nd_msgentry_install ��������������
*/
ND_NET_API net_msg_entry nd_hook_packet(nd_handle h, net_msg_entry msg_entry) ;

ND_NET_API int nd_connector_set_timeout(nd_netui_handle net_handle, int seconds) ;

/*�õ�IP�Ͷ˿�*/
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

//���õ����յ�û��ע�����Ϣʱ�Ƿ�ر�����
ND_NET_API void nd_net_set_unregmsg_handler(nd_handle h, int isclosed) ;
//���õ����յ�û����Ȩ����Ϣʱ�Ƿ�ر�����
ND_NET_API void nd_net_set_unauthorize_handler(nd_handle h, int isclosed) ;

ND_NET_API void nd_connector_set_userdata(nd_netui_handle net_handle, void *p);

ND_NET_API void* nd_connector_get_userdata(nd_netui_handle net_handle);

int nd_net_sysmsg_hander(nd_netui_handle node, nd_sysresv_pack_t *pack);


#endif 

