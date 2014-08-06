/* file : nd_srv.h
 * define net function of data struct of server
 * 
 * version 1.0
 * all right reserved by neil duan 2007-10
 */
#ifndef _ND_SRV_H_
#define _ND_SRV_H_

#include "nd_net/nd_sock.h"
//#include "nd_common/nd_common.h"

//#define TCP_SERVER_TYPE ('t'<<8 | 's' )		//����tcp�������

//���ӹ�����
#define cm_init		node_init
#define cm_alloc	node_alloc
#define cm_dealloc	node_dealloc

#define cmlist_iterator_t	node_iterator
#define cm_manager			node_root

//typedef int (*data_callback)(nd_handle session, void *data, size_t size, nd_handle listener) ;	//ԭʼ�������ݴ�����,����-1���ӱ��ر�
typedef int (*accept_callback) (void* income_handle, SOCKADDR_IN *addr, nd_handle listener) ;	//�������ӽ�����ִ�еĻص�����
typedef void (*deaccept_callback) (void* exit_handle, nd_handle listener) ;					//�������ӱ��ͷ�ʱִ�еĻص�����
typedef int (*pre_deaccept_callback) (void* handle, nd_handle listener) ;					//�ͷ�����ǰִ�еĻص�����



struct nd_srv_node
{
	ND_OBJ_BASE ;
	ND_SOCKET_OBJ_BASE ;

	void				*msg_handle ;		//��Ϣ������

	data_in_entry		data_entry;					//���ݴ�����
	net_msg_entry		msg_entry ;					//��Ϣ����
	accept_callback		connect_in_callback ;		//���ӽ���֪ͨ����
//	pre_deaccept_callback	pre_out_callback ;		//������Ҫ�˳�,���˳�֮ǰִ�еĺ���,�������-1�������ͷ�����,��һ�μ���
	deaccept_callback	connect_out_callback ;		//�����˳�֪ͨ����

	SOCKADDR_IN			self_addr ; 				//�Լ��ĵ�ַ
	struct cm_manager	conn_manager ;				/* ���ӹ�����*/
};

static __INLINE__ nd_handle nd_srv_get_allocator(struct nd_srv_node *node)
{
	//return node->conn_manager.cm_alloctor;
	return node->conn_manager.node_alloctor;
}

static __INLINE__ void nd_srv_set_cm_init(struct nd_srv_node *node,cm_init init_func) 
{
	node->conn_manager.init = init_func;
}

static __INLINE__ ndsocket_t nd_srv_getfd(nd_handle h_srv) 
{
	return ((struct nd_srv_node*)h_srv)->fd ;
}

ND_NET_API void nd_srv_set_allocator(struct nd_srv_node *node,nd_handle allocator,cm_alloc alloc,cm_dealloc dealloc);

ND_NET_API void nd_srv_node_init(struct nd_srv_node *node);
#define  nd_tcpsrv_node_init	nd_srv_node_init

#define	nd_srv_open(port,   listen_num,node) nd_net_bind(port, listen_num, (nd_handle)node)

ND_NET_API void nd_srv_close(struct nd_srv_node *node) ; /* close net server*/

//#define nd_udpsrv_open(port, node)	nd_net_bind(port,SOCK_DGRAM, 0, 0, (nd_handle)node)

#define  nd_tcpsrv_close nd_srv_close


//�趨���������
ND_NET_API int cm_listen(struct cm_manager *root, int max_num, int client_size) ;
ND_NET_API void cm_destroy(struct cm_manager *root) ;

//�õ����������
ND_NET_API int nd_srv_capacity(struct nd_srv_node *srvnode) ;

#endif
