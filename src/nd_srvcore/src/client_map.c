/* file : client_map.c
 * manage clent map in server
 * 
 * version 1.0
 * all right reserved by neil duan 
 * 2007-10
 */
// 
// #include "nd_common/nd_common.h"
// #include "nd_net/nd_netlib.h"

#include "nd_srvcore/nd_srvlib.h"

#if !defined(ND_UNIX) 
#include "../win_iocp/nd_iocp.h"
#endif 

#define INIT_SESSION_BUFF(session) \
	(session)->connect_node.send_buffer.is_alloced = 0 ;	\
	(session)->connect_node.recv_buffer.is_alloced = 0 ;	\
	(session)->connect_node.send_buffer.__buf = (session)->__sendbuf ;	\
	(session)->connect_node.recv_buffer.__buf = (session)->__recvbuf ;	\
	(session)->connect_node.send_buffer.buf_capacity = sizeof((session)->__sendbuf );	\
	(session)->connect_node.recv_buffer.buf_capacity = sizeof((session)->__recvbuf );	\
	ndlbuf_reset(&((session)->connect_node.send_buffer)) ;				\
	ndlbuf_reset(&((session)->connect_node.recv_buffer)) 

void nd_tcpcm_init(struct nd_client_map *client_map, nd_handle h_listen)
{	
	memset(client_map, 0, sizeof(struct nd_client_map )) ;
	_tcp_connector_init(&(client_map->connect_node)) ;
	//init buff
	INIT_SESSION_BUFF(client_map) ;

	INIT_LIST_HEAD(&(client_map->map_list)) ;
	client_map->connect_node.is_session = 1;
	client_map->connect_node.size = sizeof(struct nd_client_map) ;

	client_map->connect_node.disconn_timeout = ((struct listen_contex*)h_listen)->operate_timeout;
	client_map->connect_node.close_entry = (nd_close_callback ) tcp_client_close ;

	client_map->connect_node.msg_entry = ((struct nd_srv_node*)h_listen)->msg_entry ;

	client_map->connect_node.data_entry = ((struct nd_srv_node*)h_listen)->data_entry ;
	client_map->connect_node.update_entry = (net_update_entry)_tcp_session_update ;
}
void nd_client_map_destroy(struct nd_client_map *client_map)
{
	ndlbuf_destroy(&client_map->connect_node.send_buffer) ;
	ndlbuf_destroy(&client_map->connect_node.recv_buffer) ;
}

void udt_clientmap_init(struct nd_udtcli_map *node, nd_handle h_listen)
{
	memset(node, 0, sizeof(*node )) ;
	_udt_connector_init(&node->connect_node);

	INIT_SESSION_BUFF(node) ;

	INIT_LIST_HEAD(&(node->map_list)) ;
	node->connect_node.is_session = 1;
	node->connect_node.size = sizeof(struct nd_udtcli_map) ;

	node->connect_node.disconn_timeout = ((struct listen_contex*)h_listen)->operate_timeout;
	node->connect_node.close_entry =(nd_close_callback ) udt_close ;

	node->connect_node.data_entry = ((struct nd_srv_node*)h_listen)->data_entry ;

	node->connect_node.msg_entry = ((struct nd_srv_node*)h_listen)->msg_entry ;
	//node->connect_node.update_entry = (net_update_entry)_tcp_session_update ;
}

void udt_icmp_cm_init(struct nd_udtcli_map *node, nd_handle h_listen)
{
	memset(node, 0, sizeof(*node )) ;
	//udt_icmp_init(&node->connect_node);

	_udticmp_connector_init((nd_udt_node*)node);

	INIT_SESSION_BUFF(node) ;

	INIT_LIST_HEAD(&(node->map_list)) ;
	node->connect_node.is_session = 1;
	node->connect_node.size = sizeof(struct nd_udtcli_map) ;

	node->connect_node.disconn_timeout = ((struct listen_contex*)h_listen)->operate_timeout;
	node->connect_node.close_entry =(nd_close_callback ) udt_close ;

	node->connect_node.data_entry = ((struct nd_srv_node*)h_listen)->data_entry ;

	node->connect_node.msg_entry = ((struct nd_srv_node*)h_listen)->msg_entry ;

	ndstrncpy(node->connect_node.bindip ,((struct nd_netsocket*)h_listen)->bindip, sizeof(node->connect_node.bindip));
	node->connect_node.port = ((struct nd_netsocket*)h_listen)->port ;
	//node->connect_node.update_entry = (net_update_entry)_tcp_session_update ;
}

/* get client header size*/
size_t nd_getclient_hdr_size(int iomod)
{
	size_t ret ;
	switch(iomod)
	{
	//case ND_LISTEN_UDT_DATAGRAM:
	case ND_LISTEN_UDT_STREAM:
		ret = sizeof(struct nd_udtcli_map) ;
		break ;
	case ND_LISTEN_OS_EXT:
#if !defined(ND_UNIX)
		ret = sizeof(struct nd_client_map_iocp) ;
		break ;
#endif
	case 	ND_LISTEN_COMMON :
	default :
		ret = sizeof(struct nd_client_map) ;
		break ; ;
	}
	return ret ;
}


void *nd_session_getdata(nd_netui_handle session) 
{
	size_t size =  session->size ;
	if (size & 7){
		size += 8 ;
		size &= ~7 ;
	}
#ifdef ND_DEBUG
	//nd_assert(nd_session_valid(session)) ;
#endif
	return (void*) (((char*)session) + size);
}
