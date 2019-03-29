/* file nd_sock.h
 * define my socket type 
 *
 * neil
 * 2005-11-23
 */

#ifndef _ND_SOCK_H_
#define _ND_SOCK_H_

#if  !defined(ND_UNIX) 
#ifndef _WINDOWS_
//#include <WINSOCK2.H>
#endif 
	typedef signed int 				ndsocket_t;
	typedef int						socklen_t ;
	#ifndef ESOCKETTIMEOUT
	#define	ESOCKETTIMEOUT				WSAEWOULDBLOCK
	#endif
	#define nd_socket_last_error		WSAGetLastError		
	typedef char* sock_opval_t ;
	#include <Ws2tcpip.h>  
#else 
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	typedef signed int 				ndsocket_t;
	typedef struct sockaddr_in  SOCKADDR_IN ;
	typedef struct sockaddr_in6  SOCKADDR_IN6;
	typedef struct hostent HOSTENT,*LPHOSTENT;
	typedef struct sockaddr SOCKADDR, *LPSOCKADDR ;
	#define  ioctlsocket  ioctl 
	#define nd_socket_last_error()	errno
	#define ESOCKETTIMEOUT	EWOULDBLOCK 
	typedef void* sock_opval_t ;
#endif //win32

//close socket
ND_NET_API void nd_socket_close(ndsocket_t s) ;

//send data from socket 
ND_NET_API int nd_socket_tcp_write(ndsocket_t fd, void *write_buf, size_t len);
//reand tcp data from socket fd 
ND_NET_API int nd_socket_tcp_read(ndsocket_t fd, void *buf, size_t buflen) ;

/* write data to net 
 * return value : write data len 
 */
ND_NET_API int nd_socket_udp_write(ndsocket_t fd, const char* data, size_t data_len ,SOCKADDR_IN* to_addr);

/* read date from net 
 * input : udp fd , date buffer address and buffer size 
 * output : from_addr
 * return value: read datalen 
 */
ND_NET_API int nd_socket_udp_read(ndsocket_t fd ,char *buf, size_t size, SOCKADDR_IN* from_addr);



/*get host address from host name*/
ND_NET_API int get_sockaddr_in(const char *host_name, short port, SOCKADDR_IN* sock_addr);

/* create a server socket and bind 
 * @port listened port
 * @type = SOCK_DGRAM create udp port
 * @type = SOCK_STREAM create udp tcp 
 * @out_addr out put address of the socket
 * return -1 error else return socket
 */
ND_NET_API ndsocket_t nd_socket_openport(int port, int type, int protocol, int listen_nums, const char*bindip);
ND_NET_API ndsocket_t nd_socket_openport_v6(int port, int type,int protocol, int listen_nums);
ND_NET_API ndsocket_t nd_socket_openport_v4(int port, int type, int protocol, int listen_nums);
/* connect to server
 * @out put address of remote host
 * return -1 on error else return socket fd
 */

ND_NET_API ndsocket_t nd_socket_connect(const char *host_name, short port,int sock_type, SOCKADDR_IN *out_addr) ;
#define nd_socket_tcp_connect(h, p, addr) nd_socket_connect(h, p,SOCK_STREAM, addr)
#define nd_socket_udp_connect(h, p, addr) nd_socket_connect(h, p,SOCK_DGRAM, addr)
//ND_NET_API ndsocket_t nd_socket_udp_connect(char *host_name, short port,SOCKADDR_IN *out_addr);

ND_NET_API NDUINT16 nd_checksum(NDUINT16 *buf,size_t length) ;
/*
 * wait socket writablity.
 * return -1 error 
 * return 0 timeout 
 * return 1 socket fd is writablity
 */
ND_NET_API int nd_socket_wait_writablity(ndsocket_t fd,int timeval) ;
ND_NET_API int nd_socket_wait_read(ndsocket_t fd,int timeval);

//compare two address of internet 
//return 0 equate
ND_NET_API int nd_sockadd_in_cmp(SOCKADDR_IN *src_addr, SOCKADDR_IN *desc_addr);

ND_NET_API int nd_socket_nonblock(ndsocket_t fd, int cmd) ; //set socket nonblock or blocl

ND_NET_API const char *nd_inet_ntoa (ndip_t in, char *buff,size_t size) ;
ND_NET_API ndip_t nd_inet_aton(const char *ipaddr) ;
ND_NET_API int nd_sockadd_to_ndip(SOCKADDR_IN *sockaddr, ndip_t *ip);
#define ND_INET_NTOA(in,buff) nd_inet_ntoa(in,buff, sizeof(buff))

ND_NET_API NDUINT64 nd_hton64(NDUINT64 h) ;
ND_NET_API NDUINT64 nd_ntoh64(NDUINT64 h) ;

/*raw socket*/

typedef void (*parse_ip) (char *buf, int len, SOCKADDR_IN *from) ;
//open raw socket and set IPHDRINCL
//ND_NET_API ndsocket_t open_raw( int protocol);
#define open_raw(proto) nd_socket_openport(0, SOCK_RAW,proto,0, 0)

//send syn pocket 
ND_NET_API int send_tcp_syn(ndsocket_t fd, SOCKADDR_IN *src, SOCKADDR_IN *dest);

ND_NET_API int send_raw_udp(ndsocket_t fd, char *data, int len, SOCKADDR_IN *src, SOCKADDR_IN *dest);
ND_NET_API int set_raw_iphdr(ndsocket_t fd, int flag) ;
ND_NET_API int send_ping(ndsocket_t fd,  SOCKADDR_IN *dest,int seq_no, char *data, int len) ;
//ND_NET_API int icmp_data_send(ndsocket_t fd, char *data, int len, SOCKADDR_IN *dest,int seq_no) ;
//ND_NET_API int icmp_data_recv(ndsocket_t fd,char *buf, int size, SOCKADDR_IN *from ) ;
//ND_NET_API int send_icmp(ndsocket_t fd, char *data, int len, SOCKADDR_IN *dest,int seq_no,int icmptype, unsigned short pid );
//ND_NET_API int send_icmp(ndsocket_t fd, char *data, int len, SOCKADDR_IN *dest,int seq_no) ;

ND_NET_API int udp_packet_make(void *buf, char *data, int len, SOCKADDR_IN *src, SOCKADDR_IN *dest) ;
//ND_NET_API int icmp_packet_make(void *buf, char *data, int len, SOCKADDR_IN *dest,int seq_no,int icmptype) ;
ND_NET_API int test_remote_host(char *host);
ND_NET_API int raw_set_recvall(ndsocket_t raw_fd) ;
ND_NET_API int send_icmp(ndsocket_t fd, char *data, int len, SOCKADDR_IN *dest,int seq_no,int type, int code) ;

ND_NET_API ndip_t nd_sock_getip(ndsocket_t fd);
ND_NET_API ndport_t nd_sock_getport(ndsocket_t fd);

ND_NET_API ndip_t nd_sock_getpeerip(ndsocket_t fd);
ND_NET_API ndport_t nd_sock_getpeerport(ndsocket_t fd);

//get localhost IP
ND_NET_API ndip_t nd_get_ip() ;
//proxy 

#define PROXY_URL_LEN		128
#define PROXY_USER_SIZE		64
enum e_proxy_type{
	ND_PROXY_NOPROXY,
	ND_PROXY_SOCKS4,
	ND_PROXY_SOCKS4A,
	ND_PROXY_SOCKS5,
	ND_PROXY_HTTP11
} ;

struct nd_proxy_info
{
	int proxy_type ;
	short proxy_port;
	char proxy_host[PROXY_URL_LEN];

	char user[PROXY_USER_SIZE];
	char password[PROXY_USER_SIZE] ;
};

ND_NET_API ndsocket_t nd_proxy_connect(const char *host, short port,SOCKADDR_IN *out_addr, struct nd_proxy_info *proxy, int is_udp)  ;

//send udp data through proxy
ND_NET_API int nd_proxy_sendtoex(ndsocket_t fd, const char *data, size_t size, const char *remotehost, short port,SOCKADDR_IN *proxy);

ND_NET_API int nd_proxy_sendto(ndsocket_t fd, void *data, size_t size,  SOCKADDR_IN *remoteaddr,SOCKADDR_IN *proxy);

//ip cmp, 192.168.1.% is EQ 192.168.0.1 because 192.168.1.255 is transfer to 192.168.1.% 
//return 0 success
ND_NET_API int nd_sock_cmp_ip(ndip_t src, ndip_t desc, NDUINT32 ipmask) ;
#endif
