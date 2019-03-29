/* file nd_udp.c
 * 
 * translate udp data to nd net message 
 * 
 * all right reserved 
 *
 * 2010/3/21 10:06:38
 */

#define ND_IMPLEMENT_HANDLE
typedef struct nd_udp_node *nd_handle;

//#include "nd_common/nd_common.h"
//#include "nd_common/nd_alloc.h"
#include "nd_net/nd_netlib.h"


int nd_udp_connect(struct nd_udp_node*node,const char *host, int port,struct nd_proxy_info *proxy)
{
	node->fd = nd_socket_udp_connect(host, port, (SOCKADDR_IN*)&node->remote_addr6);
	if (node->fd <= 0) {
		node->myerrno = NDERR_OPENFILE;
		return -1;
	}

	if (node->remote_addr6.sin6_family == AF_INET6) {
		node->is_ipv6 = 1;
	}
	nd_socket_nonblock(node->fd, 1) ;
	return 0 ;
}

int nd_udp_close(struct nd_udp_node*node,int flag) 
{
	nd_assert(node) ;	
	if (node->fd) {
		nd_socket_close(node->fd) ;
		node->fd = 0 ;
		node->status = 0 ;
	}
	return 0 ;
}


int nd_udp_send(struct nd_udp_node*node,const char *data, size_t len)  
{
	int ret =0;
// 	if (node->protocol != PROTOCOL_OTHER ){
// 		ndudp_header  *packet = (ndudp_header  *) data ;
// 		if (UDP_POCKET_PROTOCOL(packet) == node->protocol) {
// 			if(UDP_POCKET_CHECKSUM(packet)==0){
// 				UDP_POCKET_CHECKSUM(packet) = nd_checksum((NDUINT16*)packet,len) ;
// 			}		;
// 		}
// 	}

	//ret = _socket_sendto(node, data, len, &node->remote_addr);
	ret = nd_socket_udp_write(node->fd, data, len, &node->remote_addr);
	if(ret > 0){
		node->last_push = nd_time() ;
		node->send_len += len ;
	}
	else if(-1==ret) {
		if(ESOCKETTIMEOUT==nd_last_errno())
			node->myerrno = NDERR_WOULD_BLOCK;
		else 
			node->myerrno = NDERR_WRITE ;
	}

	return ret;
}
// 
// int nd_udp_sendto(struct nd_udp_node*node,const char *data, size_t len, SOCKADDR_IN* to_addr)
// {
// 	int ret= nd_socket_udp_write(node->fd, data, len, to_addr);
// 
// 	if(ret > 0) {
// 		node->send_len += len ;
// 
// 		node->last_push = nd_time() ;
// 	}
// 	else if(-1==ret) {
// 		if(ESOCKETTIMEOUT==nd_last_errno())
// 			node->myerrno = NDERR_WOULD_BLOCK;
// 		else 
// 			node->myerrno = NDERR_WRITE ;
// 	}
// 
// 	return ret;
// }
// 
// int nd_udp_sendtoex(struct nd_udp_node*node,const char *data, size_t len, char *host, int port)
// {
// 	int ret ;
// 
// 	struct sockaddr_in6 to_addr;
// 	if (node->is_ipv6) {
// 		to_addr.sin6_family = AF_INET6;
// 	}
// 	else {
// 		to_addr.sin6_family = AF_INET;
// 	}
// 
// 	if (-1 == get_sockaddr_in(host, port, (SOCKADDR_IN*)&to_addr)) {
// 		node->myerrno = NDERR_INVALID_INPUT;
// 		return -1;
// 	}
// 
// 	ret = nd_socket_udp_write(node, data, len, &to_addr);
// 
// 	if(ret > 0)
// 		node->send_len += len ;
// 	else if(-1==ret) {
// 		if(ESOCKETTIMEOUT==nd_last_errno())
// 			node->myerrno = NDERR_WOULD_BLOCK;
// 		else 
// 			node->myerrno = NDERR_WRITE ;
// 	}
// 	return ret ;
// }

//read udp socket data
int nd_udp_read(struct nd_udp_node*node , char *buf, size_t buf_size, ndtime_t outval) 
{

	int ret ;
	int readlen = 0;

	nd_assert(buf && buf_size>0);

	TCPNODE_READ_AGAIN(node) = 0;

	if(outval ) {
		ret = nd_socket_wait_read(node->fd, outval) ;
		if(ret <= 0) {
			node->myerrno = (ret == 0) ? NDERR_WOULD_BLOCK: NDERR_IO;
			return ret  ;
		}
	}

	readlen = nd_socket_udp_read(node->fd, buf, (int)buf_size, &node->last_read);

	if (-1 == readlen) {
		node->sys_error = nd_last_errno();
		node->myerrno = NDERR_READ;
		nd_logdebug("recvfrom : %s\n", nd_last_error());
		return -1;
	}
	TCPNODE_READ_AGAIN(node) = 1;
	node->last_recv = nd_time();
	node->recv_len += readlen;

// 	if(readlen>=ND_UDP_PACKET_SIZE){
// 		node->myerrno = NDERR_BADPACKET ;
// 		return -1 ;
// 	}

// 
// 	if (node->protocol != PROTOCOL_OTHER){
// 		NDUINT16 checksum,calc_cs;
// 		ndudp_header  *packet = (ndudp_header  *) buf ;
// 
// 		if (UDP_POCKET_PROTOCOL(packet)!= node->protocol) {
// 			node->myerrno = NDERR_BADPACKET ;
// 			return -1 ;
// 		}
// 
// 		checksum = UDP_POCKET_CHECKSUM(packet) ;
// 		if(checksum) {
// 			UDP_POCKET_CHECKSUM(packet) = 0 ;
// 			calc_cs = nd_checksum((NDUINT16*)buf, readlen) ;
// 			if(checksum!=calc_cs){
// 				node->myerrno = NDERR_BADPACKET ;
// 				return -1 ;
// 			}
// 		}
// 	}

	return readlen ;
}


//get last read data address
struct sockaddr_in* nd_udp_read_addr(struct nd_udp_node*node) 
{
	return &node->last_read ;
}
//parse udp data 
int nd_udp_parse(struct nd_udp_node*node, char *buf,size_t len )
{
	int ret =0;
	ENTER_FUNC()
	if (node->protocol==PROTOCOL_OTHER){
		ret = node->data_entry( (nd_handle)node, buf, len,(nd_handle) &node->last_read ) ;
	}
	else  {
		ndudp_header  *packet = (ndudp_header  *) buf ;
		nd_assert(node->protocol_entry) ;
		if(len < sizeof(*packet)) {
			node->myerrno = NDERR_BADPACKET ;
		}
		else { 
			ret = node->protocol_entry((nd_handle) node , packet, len , &node->last_read) ;
		}
	}

	LEAVE_FUNC();
	return ret ;
}


void udp_node_init(struct nd_udp_node* node) 
{
	nd_assert(node) ;

	memset(node, 0, sizeof(*node)) ;

	node->type = NDHANDLE_UDPNODE ;
	node->size = sizeof(struct nd_udp_node) ;
	node->sock_write = (socket_write_entry) nd_udp_send ;
	node->close_entry = (nd_close_callback) nd_udp_close;
	node->sock_read = (socket_read_entry)nd_udp_read;
	node->status = ETS_DEAD;				/*socket state in game 0 not read 1 ready*/
	node->protocol = PROTOCOL_OTHER;
	node->sock_type = SOCK_DGRAM ;
	node->start_time =nd_time() ;		
	node->last_push = nd_time() ;
	node->disconn_timeout = ND_DFT_DISSCONN_TIMEOUT ;
	init_crypt_key(&node->crypt_key);

	nd_net_connbuf_init((struct netui_info*)node) ;
}

#undef ND_IMPLEMENT_HANDLE
