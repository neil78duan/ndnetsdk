/* file nd_iocp.h
 * head file of windows iocp interface
 * 
 * version 1.0 
 * all right reserved by neil duan 2007-11
 */

#ifndef _ND_IOCP_H_
#define _ND_IOCP_H_

#if !defined(ND_UNIX) 

//#define USER_THREAD_POLL		//是否使用线程模式

#define IOCP_DELAY_CLOSE_TIME		(10*1000)	//延迟10秒关闭
#include "nd_srvcore/nd_session.h"
#include "nd_srvcore/nd_srvobj.h"
//#include "nd_srvcore/nd_srvlib.h"
typedef BOOL (WINAPI *BindIoCPCallback)(
	HANDLE FileHandle,                         // handle to file
	LPOVERLAPPED_COMPLETION_ROUTINE Function,  // callback
	ULONG Flags                                // reserved
);


struct iocp_lock_info
{
	NDUINT16 session_id ;
	struct cm_manager *pmanager ;
};
int iocp_cm_lock(struct nd_session_iocp *pclient, struct iocp_lock_info *lockinfo);
static __INLINE__ void iocp_cm_unlock(struct iocp_lock_info *lockinfo) 
{	
	lockinfo->pmanager->unlock (lockinfo->pmanager,lockinfo->session_id) ;
}


#if 0
struct send_buffer_node
{
	struct list_head list ;
	size_t size ;
	//struct ndnet_msg msg_buf ;
	//nd_packhdr_t msg_buf;
	char msg_buf[0] ;
};
#endif 

int nd_iocp_node_init(struct nd_session_iocp *iocp_map,nd_handle h_listen) ;
int nd_init_iocp_client_map(struct nd_session_iocp *iocp_map,int listen_fd);

#define _SEND_LIST(iocp_map) &((iocp_map)->wait_send_list)

static __INLINE__ NDUINT16 iocp_session_id(struct nd_session_iocp *iocp_map)
{
	return iocp_map->__client_map.connect_node.session_id  ;
}

/*
static __INLINE__ int check_send_list_empty(struct nd_session_iocp *iocp_map)
{
	return iocp_map->wait_send_list.next == &iocp_map->wait_send_list ;
}
*/
static __INLINE__ int iocp_unnotified_length(struct nd_session_iocp *iocp_map)
{
	return (int)(iocp_map->total_send - iocp_map->send_len) ;
}
static __INLINE__ nd_netbuf_t * iocp_send_buf(struct nd_session_iocp *cli_map)
{
	return &(cli_map->__client_map.connect_node.send_buffer) ;
}

static __INLINE__ nd_netbuf_t * iocp_recv_buf(struct nd_session_iocp *cli_map)
{
	return &(cli_map->__client_map.connect_node.recv_buffer) ;
}

#define IOCP_MAP_FD(node) (node)->__client_map.connect_node.fd
void CALLBACK iocp_callback(DWORD dwErrorCode, DWORD dwByteCount,LPOVERLAPPED lpOverlapped) ;

//iocp_write /iocp_read 在socket级别上read write
ND_SRV_API int iocp_write(struct nd_session_iocp *iocp_map) ;
ND_SRV_API int iocp_read(struct nd_session_iocp *iocp_map) ;
ND_SRV_API int iocp_close_client(struct nd_session_iocp *iocp_map, int force) ;

//在 nd_tcp_node 级别上的发送主要是使用WSASend 来代替send
ND_SRV_API int _iocp_write2sock(struct nd_tcp_node *node,void *data , size_t len);
//发送一个完整的消息
ND_SRV_API int _iocp_write2sock_wait(struct nd_tcp_node *node,void *data , size_t len) ;

//在client_map节点上发送
ND_SRV_API int nd_iocp_sendmsg(struct nd_session_iocp *iocp_map,nd_packhdr_t *msg_buf, int flag) ;

int iocp_socket_write(struct nd_session_iocp *iocp_map, void *data, size_t send_len) ;

int check_repre_accept(struct listen_contex *) ;	//得到当前需要preaccept的个数
//the following function used is iocp model
int pre_accept(struct nd_srv_node *srv_node) ;
int iocp_accept(struct nd_session_iocp *node);
//int iocp_parse_msgex(struct nd_session_iocp *iocp_map,NDNET_MSGENTRY msg_entry );
int data_income(struct nd_session_iocp *pclient, DWORD dwRecvBytes );

int update_iocp_cm(struct listen_contex *srv_root) ;
#endif
#endif
