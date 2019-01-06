/* file : client_map.h
 * manage clent map in server
 * 
 * version 1.0
 * all right reserved by neil duan 
 * 2007-10
 */

#ifndef  _CLIENT_MAP_H_
#define _CLIENT_MAP_H_


//#include "nd_net/nd_netlib.h"

#define ND_NAME_SIZE 40


//使用自定义的发送函数
//typedef int (*snedmsg_ext_entry)(void *owner, nd_packhdr_t *msg_buf, int flag) ;	//define extend send function
//client 状态
/*
enum eClientStatus{
	ECS_NONE = 0 ,			//空闲
	ECS_ACCEPTED ,			//accept
	ECS_READY,				//login 
	ESC_CLOSED				//connect closed
} ;
*/
//nd_client_map 结果需要根据各种不同的服务器消息结构来定制
/*client connect map on server*/
struct nd_client_map
{
	struct nd_tcp_node connect_node ;		//socket connect node 
	struct list_head map_list ;
	char __sendbuf[ND_NETBUF_SIZE];
	char __recvbuf[ND_NETBUF_SIZE];
};

/*client connect map is server*/
struct nd_udtcli_map
{
	nd_udt_node connect_node ;		//socket connect node 
	struct list_head map_list ;
	char __sendbuf[ND_NETBUF_SIZE];
	char __recvbuf[ND_NETBUF_SIZE];
};

#if !defined(ND_UNIX)  

#if _MSC_VER < 1300 // 1200 == VC++ 6.0
#else 
#include "winsock2.h"

#endif

struct nd_client_map_iocp ;

struct ND_OVERLAPPED_PLUS  
{
	WSAOVERLAPPED overlapped ;	
	struct nd_client_map_iocp* client_addr; //Pointer to client
};

struct nd_client_map_iocp
{
	struct nd_client_map  __client_map ;		//common client map ,MUST in first 	
	size_t			total_send;				//write buf total
	size_t			send_len;				// had been send length
	int				__wait_buffers ;		//waiting sending buffer number
	ndtime_t		schedule_close_time;				//set close time 
	ndatomic_t		in_sending ;			//between WSASend () and iocp_callback()
	struct ND_OVERLAPPED_PLUS  ov_read;
	struct ND_OVERLAPPED_PLUS  ov_write ;
	WSABUF			wsa_readbuf;			// Temporary buffer used for reading
	WSABUF			wsa_writebuf;			// Temporary buffer used for writing
};

#endif

enum ePlayerState {
	EPS_NONE = 0 ,
	EPS_CONNECT,
	EPS_LOGIN
};
//header of playerdata
typedef struct player_header{
	nd_netui_handle		h_session ;		//handle of connection session
	NDUINT32			id ;
	int					status ;		//ref ePlayerState
}player_header_t ;


//定义client map handle
//typedef void *nd_cli_handle;
typedef struct netui_info *nd_climap_handle ;		//定义客户端连接镜像(客户端连接到服务器时需要建立这样一个镜像)

//从clienthandle中得到相关属性
//ND_SRV_API struct nd_client_info* nd_get_client_info(nd_climap_handle cli_handle) ; 

ND_SRV_API struct list_head *get_self_list(nd_climap_handle cli_handle);	//得到client自己的链表节点

ND_SRV_API  void nd_tcpcm_init(struct nd_client_map *client_map,nd_handle h_listen) ;
ND_SRV_API void nd_client_map_destroy(struct nd_client_map *client_map) ;

ND_SRV_API size_t nd_getclient_hdr_size(int iomod) ;

ND_SRV_API void *nd_session_getdata(nd_netui_handle session) ;

#endif
