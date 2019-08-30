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
		node->sys_error = nd_socket_last_error();
		if (node->sys_error == ESOCKETTIMEOUT) {
			node->myerrno = NDERR_WOULD_BLOCK;
			return 0;
		}
		else {
			node->myerrno = NDERR_READ;
			nd_logdebug("recvfrom : %s\n", nd_last_error());
			return -1;
		}
	}
	TCPNODE_READ_AGAIN(node) = 1;
	node->last_recv = nd_time();
	node->recv_len += readlen;

	return readlen ;
}


//get last read data address
struct sockaddr_in* nd_udp_read_addr(struct nd_udp_node*node) 
{
	return &node->last_read ;
}


void udp_node_init(struct nd_udp_node* node) 
{
	nd_assert(node) ;

	memset(node, 0, sizeof(*node)) ;

	node->type = NDHANDLE_UDPNODE ;
	node->size = sizeof(struct nd_udp_node) ;
	node->sys_sock_write = (socket_sys_entry) nd_udp_send ;
	node->sys_sock_read = (socket_sys_entry)nd_udp_read;
	node->close_entry = (nd_close_callback) nd_udp_close;
	node->status = ETS_DEAD;				/*socket state in game 0 not read 1 ready*/
	node->protocol = PROTOCOL_OTHER;
	node->sock_type = SOCK_DGRAM ;
	node->start_time =nd_time() ;		
	node->last_push = nd_time() ;
	node->disconn_timeout = ND_DFT_DISSCONN_TIMEOUT ;
	node->msg_caller = node;
	init_crypt_key(&node->crypt_key);

	nd_net_connbuf_init((struct netui_info*)node) ;
}

#undef ND_IMPLEMENT_HANDLE
