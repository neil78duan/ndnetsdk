/* file udt_icmp.c
 * 
 * implemention udt by icmp
 *
 * all right reserved by neil duan
 *
 * 2010-4-20 
 */

//#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"

//static short _sequence_id ;

// ICMP header
typedef struct _udt_icmp_hdr
{
	u_8   icmp_type;
	u_8   icmp_code;
	u_16  icmp_checksum;
	u_16  receive_id;				//接收这PROCESS id
	u_16  receive_port;				// port(like udp/tcp port)
	u_16  send_id;
	u_16  send_port;
} udt_icmp_hdr;


nd_udt_node* udt_icmp_connect(nd_udt_node *socket_node,char *host, short port, struct nd_proxy_info *proxy)
{
	int opt = 1;
	if(proxy && proxy->proxy_type) {
		socket_node->myerrno = NDERR_INVALID_INPUT ;
		return NULL ;
	}
	memset(&socket_node->remote_addr,0,sizeof(socket_node->remote_addr));

	if(-1==get_sockaddr_in(host, port, &socket_node->remote_addr) ) {
		return 0;
	}

	if(-1==nd_net_bind(0,0,(nd_handle) socket_node) ) {
		return NULL ;
	}

	//set handle ip header by user
	if(-1==set_raw_iphdr(socket_node->fd,1) ) 
		return NULL;

	if(-1==nd_net_ipbind((nd_handle)socket_node,nd_get_ip()) ){
		return NULL;
	}
	socket_node->port = socket_node->fd & 0xffff; 

	raw_set_recvall(socket_node->fd) ;
	return socket_node ;

}


static int icmp_udt_make(void *buf, char *data, int len, SOCKADDR_IN *src,SOCKADDR_IN *dest,udt_icmp_hdr *icmp_info)
{
	pseudo_icmp *ps_icmp ;
	struct packet {
		ip_hdr ip ;
		udt_icmp_hdr icmp ;
		char data[0] ;
	}*u_packet = buf ;

	memset((void*)u_packet, 0, sizeof(ip_hdr)+sizeof(icmp_hdr));
	ps_icmp = (pseudo_icmp*) (u_packet->data - sizeof(pseudo_icmp)) ;

	ps_icmp->dest_addr =(u_32) dest->sin_addr.s_addr ;
	ps_icmp->src_addr =(u_32) src->sin_addr.s_addr ;
	ps_icmp->protocol = IPPROTO_ICMP ;

	u_packet->icmp.icmp_code = 0 ;
	u_packet->icmp.icmp_type = ICMP_ECHOREPLY ;
	u_packet->icmp.icmp_checksum =0;
	u_packet->icmp.receive_id =icmp_info->receive_id;				//接收这PROCESS id
	u_packet->icmp.receive_port =icmp_info->receive_port;				// port(like udp/tcp port)
	u_packet->icmp.send_id =icmp_info->send_id;
	u_packet->icmp.send_port =icmp_info->send_port;	
	

	if(data && len >0  && len<RAW_SYSTEM_BUF) {
		memcpy(u_packet->data, data, len) ;
	}
	len +=  sizeof(icmp_hdr)  ;
	ps_icmp->icmp_length = ntohs( len ) ;

	u_packet->icmp.icmp_checksum = nd_checksum((NDUINT16*)ps_icmp, len) ;

	//////////////////////////////////////////////////////////////////////////
	memset((void*)u_packet, 0, sizeof(ip_hdr));

	len += sizeof(ip_hdr)  ;

	u_packet->ip.ip_hl = sizeof(ip_hdr)>>2 ;
	u_packet->ip.ip_v = ND_IPVERSION ;
	u_packet->ip.ip_id = 0;		//unique identifier : set to 0
	u_packet->ip.ip_tos =  0;
	u_packet->ip.ip_len = htons(len) ;
	u_packet->ip.ip_ttl = ND_TTL ;
	u_packet->ip.ip_p = IPPROTO_ICMP;
	u_packet->ip.ip_src = (u_32) src->sin_addr.s_addr ;
	u_packet->ip.ip_dst = (u_32) dest->sin_addr.s_addr ; 

	u_packet->ip.ip_sum = nd_checksum((NDUINT16*)u_packet, sizeof(ip_hdr)) ;

	return len;

}
static int _raw_send(nd_udt_node *node, char *data, int len, SOCKADDR_IN *dest)
{
	int send_len = len;
	static NDUINT16 s_pid ;
	SOCKADDR_IN self ;
	udt_icmp_hdr icmp_info ;
	char buf[RAW_SYSTEM_BUF] ;

	if(len > (RAW_SYSTEM_BUF-sizeof(ip_hdr)- sizeof(icmp_hdr) ) )
		return -1;

	if(s_pid==0)
		s_pid = htons((NDUINT16) nd_processid() );
	self.sin_family = AF_INET ;
	self.sin_addr.s_addr = node->bindip ;

	icmp_info.receive_id = *(u_16*)(dest->sin_zero);
	icmp_info.receive_port = dest->sin_port;

	icmp_info.send_port = node->port ;
	icmp_info.send_id = s_pid ;

	len = icmp_udt_make(buf, data, len,&self, dest, &icmp_info );

	if(sendto(node->fd, buf, len,0,(SOCKADDR*)dest,sizeof(*dest))==len) {
		return send_len ;
	}
	return -1;
}

int udt_icmp_write(nd_udt_node* node,char *data, size_t len) 
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

	ret =_raw_send(node, data, (int)len, &node->remote_addr) ;
	
	if(-1==ret) {
		if(ESOCKETTIMEOUT==nd_last_errno())
			node->myerrno = NDERR_WUOLD_BLOCK ;
		else 
			node->myerrno = NDERR_WRITE ;
	}
	return (int)len;
}

/* read data from raw socket */
int icmp_socket_read(struct nd_netsocket*node , char *buf, size_t buf_size, struct sockaddr_in *addr, ndip_t destip, NDUINT16 destport)
{
	static NDUINT16 s_pid ;
	int ret , sock_len;
	char recv_buf[RAW_SYSTEM_BUF] ;

	if(s_pid==0)
		s_pid = htons((NDUINT16) nd_processid() );

RE_READ:
	sock_len = sizeof(*addr) ;
	ret=recvfrom(node->fd,recv_buf,RAW_SYSTEM_BUF,0,(struct sockaddr *)addr, &sock_len);
	if(ret>(int)((sizeof(udt_icmp_hdr) + sizeof(ip_hdr)))) {
		int icmp_len = 0 ;
		ip_hdr *ip = (ip_hdr*)recv_buf;
		udt_icmp_hdr  *icmp = (udt_icmp_hdr*) (recv_buf + ip->ip_hl *4) ;

		if (ip->ip_p != IPPROTO_ICMP){
			goto RE_READ;
		}

		if(icmp->icmp_type!=ICMP_ECHOREPLY  ) {			
			goto RE_READ;
		}

		//check ipaddress and 'ICMP PORT'
		if(ip->ip_dst != destip || icmp->receive_port != destport) {
			goto RE_READ;
		}

		//check process id
		if (icmp->receive_id && icmp->receive_id != s_pid) 	{
			//get receive id 
			goto RE_READ;
		}

		icmp_len = ret - ip->ip_hl *4 - sizeof(udt_icmp_hdr ) ;

		addr->sin_port = icmp->send_port ;
		*(u_16*)(addr->sin_zero) = icmp->send_id ;

		++icmp;
		if (icmp_len>0){
			icmp_len = min((int)buf_size, icmp_len) ;
			ret = icmp_len ;
			memcpy(buf,(void*)icmp, ret) ;
		}
		else {
			ret = 0;
			goto RE_READ;
		}
	}
	else {
		ret = (ret <=0) ? ret: -1;
	}

	return ret ;
}

int udt_icmp_read(struct nd_udp_node*node , char *buf, size_t buf_size, ndtime_t outval) 
{
	int ret;
RE_READ:
	ret = icmp_socket_read((struct nd_netsocket*)node , buf,  buf_size, &node->last_read, node->bindip,node->port) ;
	if(ret<=0) {
		if((int)outval > 0 ) {
			ndtime_t start = nd_time() ;
			if(nd_socket_wait_read(node->fd, outval)  <= 0) {
				node->myerrno =  NDERR_WUOLD_BLOCK;
				return 0;
			}
			outval -= (nd_time() - start) ;
			goto RE_READ ;
		}
		else {
			node->myerrno = NDERR_WUOLD_BLOCK ;
			return -1;
		}
	}

	if(ret > 0) {
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
				calc_cs = nd_checksum((NDUINT16*)buf, ret) ;
				if(checksum!=calc_cs){
					node->myerrno = NDERR_BADPACKET ;
					return -1 ;
				}
			}
		}
	}
	return ret;

}

int check_income_icmp_packet(nd_udt_node *node, struct ndudt_pocket *pocket, int len,SOCKADDR_IN *addr)
{
	u_16 *pid = (u_16*)node->remote_addr.sin_zero;

	if (*pid==0) {
		*pid = *(u_16*)addr->sin_zero ;
	}
	else if(*pid != *(u_16*)addr->sin_zero ) {
		return 0 ;
	}

	if(node->remote_addr.sin_addr.s_addr==addr->sin_addr.s_addr &&
		node->remote_addr.sin_port == addr->sin_port) {
			return 1 ;
	}
	return 0 ;
}

void _udticmp_connector_init(nd_udt_node *socket_node)
{
	_udt_connector_init(socket_node);
	socket_node->sock_write = (socket_write_entry) udt_icmp_write;
	socket_node->sock_read = (socket_read_entry)udt_icmp_read;

	socket_node->sock_type = SOCK_RAW ;
	socket_node->sock_protocol = IPPROTO_ICMP ;

	socket_node->check_entry = check_income_icmp_packet ;
}

void udt_icmp_init(nd_udt_node *socket_node)
{
	nd_udtnode_init(socket_node) ;

	socket_node->sock_write = (socket_write_entry) udt_icmp_write;
	socket_node->sock_read = (socket_read_entry)udt_icmp_read;

	socket_node->sock_type = SOCK_RAW ;
	socket_node->sock_protocol = IPPROTO_ICMP ;

	socket_node->check_entry = check_income_icmp_packet ;
}