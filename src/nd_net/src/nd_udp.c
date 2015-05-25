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

//static int udp_node_rawsend(struct nd_udp_node*node,char *data, size_t len)
//{
//	return  sendto(node->fd, data, (int)len,0,(LPSOCKADDR)&node->remote_addr, sizeof(node->remote_addr)) ;
//}

int nd_udp_connect(struct nd_udp_node*node,const char *host, int port,struct nd_proxy_info *proxy)
{
	if (proxy && proxy->proxy_type) {
		short tmp_port ;
		
		node->fd = nd_socket_openport(0, SOCK_DGRAM ,0,0,0);
		if(node->fd <= 0) {
			node->myerrno = NDERR_OPENFILE ;
			return -1 ;
		}		

		node->prox_info = malloc(sizeof(struct udp_proxy_info));
		if(!node->prox_info) {
			node->myerrno = NDERR_NOSOURCE ;
			return -1 ;
		}
		node->prox_info->remote_port = 0 ;
		tmp_port = nd_sock_getport(node->fd) ;
		tmp_port = ntohs(tmp_port) ;

		node->prox_info->proxy_fd =  nd_proxy_connect(host,tmp_port , &(node->prox_info->proxy_addr), proxy,1)  ;
		if(node->prox_info->proxy_fd <= 0) {
			node->myerrno = NDERR_INVALID_INPUT ;
			free(node->prox_info) ;
			node->prox_info = 0;
			return -1 ;
		}
		
		if(-1==get_sockaddr_in(host,  port, &node->remote_addr)) {
			node->prox_info->remote_port =  port ;
			strncpy(node->prox_info->remote_name, host, sizeof(node->prox_info->remote_name)) ;
		}
		
	}
	else {
		node->fd = nd_socket_udp_connect(host,  port, &node->remote_addr);
		if(node->fd <=0) {
			node->myerrno = NDERR_OPENFILE ;
			return -1;
		}
	}
	nd_socket_nonblock(node->fd, 1) ;
	return 0 ;
}

int nd_udp_close(struct nd_udp_node*node,int flag) 
{
	nd_assert(node) ;
	if (node->prox_info){
		if (node->prox_info->proxy_fd >0){
			nd_socket_close(node->prox_info->proxy_fd) ;
			node->prox_info->proxy_fd = 0 ;
		}
		free(node->prox_info) ;
		node->prox_info = NULL;
	}

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
	if (node->protocol != PROTOCOL_OTHER ){
		ndudp_header  *packet = (ndudp_header  *) data ;
		if (UDP_POCKET_PROTOCOL(packet) == node->protocol) {
			if(UDP_POCKET_CHECKSUM(packet)==0){
				UDP_POCKET_CHECKSUM(packet) = nd_checksum((NDUINT16*)packet,len) ;
			}		;
		}
	}
	if (!node->prox_info){
		ret =  sendto(node->fd, data, (int)len,0,(LPSOCKADDR)&node->remote_addr, (int)sizeof(node->remote_addr)) ;
	}
	else {
		struct udp_proxy_info *prox = node->prox_info ;
		if (prox->remote_port){
			ret = nd_proxy_sendtoex(node->fd,data,len, prox->remote_name,prox->remote_port,&prox->proxy_addr) ;
		}
		else {
			ret = nd_proxy_sendto(node->fd,data,len, &node->remote_addr,&prox->proxy_addr) ;
		}
		if(ret >0)
			ret = (int)len ;
	}
	if(ret > 0){
		node->last_push = nd_time() ;
		node->send_len += len ;
	}
	else if(-1==ret) {
		if(ESOCKETTIMEOUT==nd_last_errno())
			node->myerrno = NDERR_WUOLD_BLOCK ;
		else 
			node->myerrno = NDERR_WRITE ;
	}

	return ret;
}

int nd_udp_sendto(struct nd_udp_node*node,const char *data, size_t len, SOCKADDR_IN* to_addr)
{
	int ret ;

	if (!node->prox_info){
		ret = sendto(node->fd, data, (int)len,0,(LPSOCKADDR)to_addr, (int)sizeof(*to_addr)) ;
	}
	else {
		ret = nd_proxy_sendto(node->fd,data,len, to_addr,&node->prox_info->proxy_addr) ;
	}

	if(ret > 0) {
		node->send_len += len ;

		node->last_push = nd_time() ;
	}
	else if(-1==ret) {
		if(ESOCKETTIMEOUT==nd_last_errno())
			node->myerrno = NDERR_WUOLD_BLOCK ;
		else 
			node->myerrno = NDERR_WRITE ;
	}

	return ret;
}

int nd_udp_sendtoex(struct nd_udp_node*node,const char *data, size_t len, char *host, int port)
{
	int ret ;
	if (!node->prox_info){
		SOCKADDR_IN to_addr ;
		if(-1==get_sockaddr_in(host, port, &to_addr) ) {
			node->myerrno = NDERR_INVALID_INPUT;
			return -1;
		}
		ret = sendto(node->fd, data, (int)len,0,(LPSOCKADDR)&to_addr, (int)sizeof(to_addr)) ;
	}
	else {
		ret =  nd_proxy_sendtoex(node->fd,data,len, node->prox_info->remote_name,node->prox_info->remote_port,&node->prox_info->proxy_addr) ;
	}


	if(ret > 0)
		node->send_len += len ;
	else if(-1==ret) {
		if(ESOCKETTIMEOUT==nd_last_errno())
			node->myerrno = NDERR_WUOLD_BLOCK ;
		else 
			node->myerrno = NDERR_WRITE ;
	}
	return ret ;
}

//read udp socket data
int nd_udp_read(struct nd_udp_node*node , char *buf, size_t buf_size, ndtime_t outval) 
{

	int ret , readlen = 0,sock_len;

	nd_assert(buf && buf_size>0);

	TCPNODE_READ_AGAIN(node) = 0;

	if(outval ) {
		ret = nd_socket_wait_read(node->fd, outval) ;
		if(ret <= 0) {
			node->myerrno = (ret==0) ? NDERR_WUOLD_BLOCK:NDERR_IO ;
			return ret  ;
		}
	}

	//read data
	sock_len = sizeof(SOCKADDR_IN) ;
	readlen = recvfrom(node->fd,buf, (int)buf_size, 0, (LPSOCKADDR)&node->last_read, &sock_len )  ;
	if(readlen <= 0 || readlen>=ND_UDP_PACKET_SIZE){
		node->myerrno = NDERR_READ ;
		return -1 ;
	}

	TCPNODE_READ_AGAIN(node) = 1;
	node->last_recv = nd_time() ;
	node->recv_len += readlen ;

	if (node->prox_info){
		struct udp_proxy_info *prox = node->prox_info ;
		if(buf[0]!=0 || buf[1]!=0 || buf[2]!=0) {
			node->myerrno = NDERR_BADPACKET ;
			return -1;
		}
		if (buf[3]==1)	{
			if(readlen <= 10) {
				node->myerrno = NDERR_BADPACKET ;
				return -1 ;
			}
			node->last_read.sin_addr.s_addr =*(int*) &buf[4] ;
			node->last_read.sin_port = *(short*) &buf[8] ;
			sock_len = 10;
			readlen -= 10 ;
		}
		else if(buf[3]==3){
			sock_len = buf[4] + 5;
			if ( sock_len >= readlen) {
				node->myerrno = NDERR_BADPACKET ;
				return -1 ;
			}

			node->last_read.sin_port = *(short *) &buf[sock_len] ;
			sock_len += 2 ;
			
			if ( sock_len >= readlen) {
				node->myerrno = NDERR_BADPACKET ;
				return -1 ;
			}
			readlen -= sock_len ;

		}
		memcpy(buf, buf+sock_len, readlen) ;
	}

	if (node->protocol != PROTOCOL_OTHER){
		NDUINT16 checksum,calc_cs;
		ndudp_header  *packet = (ndudp_header  *) buf ;

		if (UDP_POCKET_PROTOCOL(packet)!= node->protocol) {
			node->myerrno = NDERR_BADPACKET ;
			return -1 ;
		}

		checksum = UDP_POCKET_CHECKSUM(packet) ;
		if(checksum) {
			UDP_POCKET_CHECKSUM(packet) = 0 ;
			calc_cs = nd_checksum((NDUINT16*)buf, readlen) ;
			if(checksum!=calc_cs){
				node->myerrno = NDERR_BADPACKET ;
				return -1 ;
			}
		}
	}

	return readlen ;
}


//得到上次读取的数据源地址
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