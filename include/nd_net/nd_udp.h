/* file nd_udp.h
 * 
 * translate udp data to nd net message 
 * 
 * all right reserved 
 *
 * 2010/3/21 10:06:38
 */


#ifndef _ND_UDP_H_
#define _ND_UDP_H_

#include "nd_net/byte_order.h"
#include "nd_net/nd_sock.h"
//#include "nd_common/nd_common.h"
#include "nd_net/nd_netcrypt.h"
#include "nd_net/nd_netpack.h"
#include "nd_net/nd_netobj.h"

#define ND_UDP_PACKET_SIZE		0x10000

enum endudt_protocol{
	PROTOCOL_OTHER = 0,		//UNKNOW POROTOCOL
	PROTOCOL_UDT,			//udt protocol( udp data transfer protocol)
	PROTOCOL_RUDP			//reliable udp 
};

#define PROTOCOL_UDP  (PROTOCOL_OTHER+3)

#pragma pack(push)
#pragma pack(1)

//UDP protocol header
typedef struct ndudp_header
{
#  if ND_BYTE_ORDER == ND_L_ENDIAN
	u_8		version:4;			//protocl version
	u_8		protocol:4;			//protocol tyep
#else 
	u_8		protocol:4;			
	u_8		version:4;			
#endif 

	u_8		reserved ;
	u_16	checksum;
}ndudp_header;

#pragma pack(pop)


#define UDP_POCKET_CHECKSUM(pocket) ((ndudp_header*)(pocket))->checksum
#define UDP_POCKET_PROTOCOL(pocket) ((ndudp_header*)(pocket))->protocol


//upd data handle 
typedef int (*udp_protocol_entry)( nd_handle h, ndudp_header *data, size_t read_len,SOCKADDR_IN *addr) ;

struct udp_proxy_info
{
	ndsocket_t proxy_fd ;					//proxy socket fd
	short remote_port ;	
	SOCKADDR_IN proxy_addr;					//proxy address
	char remote_name[256] ;
};

//udp connector
typedef struct nd_udp_node 
{
	ND_OBJ_BASE ;
	ND_SOCKET_OBJ_BASE ;
	ND_CONNECTOR_BASE ;
	struct udp_proxy_info *prox_info ;
	union {
		struct sockaddr_in last_read;
		struct sockaddr_in6 last_read6;
	};
}nd_udp_node ;

//open udp connect
ND_NET_API int nd_udp_connect(nd_handle net_handle,const char *host, int port,struct nd_proxy_info *proxy) ;

//close udp connect
ND_NET_API int nd_udp_close(nd_handle net_handle,int flag)  ;

ND_NET_API int nd_udp_send(nd_handle net_handle,const char *data, size_t len)  ;
//ND_NET_API int nd_udp_sendto(nd_handle net_handle,const char *data, size_t len, SOCKADDR_IN* to_addr)  ;
//ND_NET_API int nd_udp_sendtoex(nd_handle net_handle,const char *data, size_t len, char *host, int port)  ;
ND_NET_API int nd_udp_read(struct nd_udp_node*node , char *buf, size_t buf_size, ndtime_t outval) ;

//ND_NET_API int nd_udp_parse(nd_handle net_handle, char *buf,size_t len );

//get addr
ND_NET_API struct sockaddr_in* nd_udp_read_addr(nd_handle net_handle) ;

void udp_node_init(struct nd_udp_node* node)  ;

#endif
