/* file : nd_sock.c
 * socket operate function
 * read/write create/close etc..
 *
 * author : neil
 * date : 2005-11-24
 * version : 1.0
 * modified :
 */

#include "nd_net/nd_netlib.h"
//#include "nd_common/nd_common.h"


extern int set_raw_sockopt(ndsocket_t fd) ;
//send data from socket
int nd_socket_tcp_write(ndsocket_t fd, void *write_buf, size_t len)
{
	nd_assert(fd) ;
	nd_assert(len>0) ;
	return (int)send(fd, write_buf, (int) len, 0) ;
}
//reand tcp data from socket fd
int nd_socket_tcp_read(ndsocket_t fd, void *buf, size_t buflen)
{
	nd_assert(fd) ;
	nd_assert(buflen>0) ;
	return (int)recv(fd, buf,(int)  buflen, 0);
}

/* write data to net
 * return value : write data len
 */
int nd_socket_udp_write(ndsocket_t fd, const char* data, size_t data_len ,SOCKADDR_IN* to_addr)
{
	int ret = 0 ;
	int sock_len = sizeof(struct sockaddr_in);
	if (to_addr->sin_family == AF_INET6) {
		sock_len = sizeof(struct sockaddr_in6);
	}
	nd_assert(data && data_len > 0 && to_addr) ;
	
	ret = (int)sendto(fd, data,(int)  data_len,0,(LPSOCKADDR)to_addr, sock_len) ;
	return ret ;
}
/* read date from net
 * input : udp fd , date buffer address and buffer size
 * output : from_addr
 * return value: read datalen
 */
int nd_socket_udp_read(ndsocket_t fd ,char *buf, size_t size, SOCKADDR_IN* from_addr)
{
	int read_len;
	
	socklen_t sock_len = (socklen_t) sizeof(struct sockaddr_in6);
	struct sockaddr_in6 remote_addr = { 0 };

	nd_assert(buf && size > 0) ;

	read_len  = (int)recvfrom(fd, buf,(int)  size, 0, (LPSOCKADDR)&remote_addr, &sock_len )  ;
	if (read_len > 0) {
		memcpy(from_addr, &remote_addr, sock_len);
	}
	return read_len  ;
}


//close a net socket of tcp or udp
void nd_socket_close(ndsocket_t s)
{
	if(s==0 || (ndsocket_t)-1==s) {
		return ;
	}
#if !defined(ND_UNIX)
	shutdown(s, SO_DONTLINGER);
	closesocket(s) ;
#else
	close(s) ;
#endif
}

/*get host address from host name
 * input host name in string (char*) and port
 * output socket address
 */
int get_sockaddr_in(const char *host_name, short port, SOCKADDR_IN* sock_addr)
{
	struct addrinfo *rp;
	struct addrinfo hints;
	char port_buf[20];
	ndsnprintf(port_buf, sizeof(port_buf), "%d", port);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = sock_addr->sin_family;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = 0; /* Datagram socket */
	hints.ai_flags = 0;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
		
	if (getaddrinfo(host_name, port_buf, &hints, &rp) != 0) {
		nd_logerror("getaddrinfo: %s\n", nd_last_error());
		return -1;
	}
	if (rp) {
		memcpy(sock_addr, rp->ai_addr, rp->ai_addrlen);
		sock_addr->sin_family = rp->ai_family;
		sock_addr->sin_port = htons(port);		
		freeaddrinfo(rp); 
		return 0;
	}
	return -1;
}

//get localhost ip
ndip_t nd_get_ip()
{
	ndip_t ipval = ND_IP_INIT;
	char hostname[256];
	int ret = gethostname(hostname,   sizeof(hostname));
	if(0==ret) {
		HOSTENT *host ;

		host = gethostbyname((const char *)(hostname ));
		if(host){
			memcpy(ipval.ip6, host->h_addr, host->h_length);
			ipval.sin_family = host->h_addrtype;
		}
	}
	return ipval;
}

int nd_sock_cmp_ip(ndip_t srcIP, ndip_t destIP, NDUINT32 ipmask)
{
	if (srcIP.sin_family != destIP.sin_family) {
		return 1;
	}

	if (srcIP.sin_family == AF_INET) {
		NDUINT32 src = srcIP.ip;
		NDUINT32 dest = destIP.ip;

		if (src == dest) {
			return 0;
		}
		if (ipmask) {
			src = src & ipmask;
			dest = dest & ipmask;
			return !(src == dest);
		}
		else {
			int n;
			unsigned char *srcip = (unsigned char*)&src;
			unsigned char *destip = (unsigned char*)&dest;
			for (n = 0; n < 4; n++) {
				if (destip[n] != 0xff && srcip[n] != 0xff) {
					if (destip[n] != srcip[n]) {
						return 1;
					}
				}
			}
		}
	}
	else {
		if (srcIP.netIp[1] == 0) {
			if (srcIP.netIp[0] == destIP.netIp[0]) {
				return 0;
			}
		}
		else {
			return  (srcIP.netIp[0] == destIP.netIp[0] && srcIP.netIp[1] == destIP.netIp[1]) ? 0 : 1;
		}
	}	
	return 0 ;
}

ndsocket_t nd_socket_openport_ex(int af_netType, int port, int type, int protocol, int listen_nums, const char* bindip)
{
	ndsocket_t listen_fd = -1;
	//char port_buf[20];
	int re_usr_addr = 1;

	listen_fd = (ndsocket_t) socket(af_netType, type, 0);
	if (listen_fd == -1)
		return -1;

#ifdef __ND_MAC__
	re_usr_addr = 1;
	if (-1 == setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, (void*)&re_usr_addr, sizeof(re_usr_addr))) {
		goto OPEN_ERROR;
	}
#endif
	re_usr_addr = 1;
	if (-1 == setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&re_usr_addr, sizeof(re_usr_addr))) {
		goto OPEN_ERROR;
	}

	if (AF_INET6 == af_netType) {
		struct sockaddr_in6 svraddr = { 0 };

		svraddr.sin6_family = AF_INET6;
		svraddr.sin6_port = htons(port);		

		if (bindip && *bindip) {
			inet_pton(AF_INET6, bindip, &svraddr.sin6_addr);
		}
		else {
			svraddr.sin6_addr = in6addr_any;
		}
		if (0 != bind(listen_fd, (SOCKADDR*)&svraddr, sizeof(svraddr))) {
			nd_logerror("bind error :%s\n", nd_last_error());
			goto OPEN_ERROR;
		}

	}
	else  {
		SOCKADDR_IN serv_addr;		/* socket address */

		serv_addr.sin_port = htons(port);
		serv_addr.sin_family = AF_INET;
		if (bindip && *bindip)
			serv_addr.sin_addr.s_addr = inet_addr(bindip);
		else
			serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

		if (-1 == bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) {
			nd_logerror("bind error :%s\n", nd_last_error());
			goto OPEN_ERROR;
		}
	}
	
	if (-1 == nd_socket_nonblock(listen_fd, 1)) {
		goto OPEN_ERROR;
	}

	if (type == SOCK_STREAM) {
		if (-1 == listen(listen_fd, listen_nums)) {
			goto OPEN_ERROR;
		}
	}
	else if (type == SOCK_RAW) {
		if (protocol != IPPROTO_ICMP) {
			if (-1 == set_raw_iphdr(listen_fd, 1))
				goto OPEN_ERROR;
		}
		if (-1 == set_raw_sockopt(listen_fd))
			goto OPEN_ERROR;
	}

	nd_logmsg("   OPEN port : %d SUCCESS!!!\n",  port);
	return listen_fd;

OPEN_ERROR:
	if (listen_fd>0) {
		nd_showerror();
		nd_socket_close(listen_fd);
	}
	return -1;
}

ndsocket_t nd_socket_openport(int port, int type, int protocol, int listen_nums, const char*bindip)
{
	return  nd_socket_openport_ex(AF_UNSPEC, port, type, protocol, listen_nums, bindip);
}

ndsocket_t nd_socket_openport_v4(int port, int type, int protocol, int listen_nums)
{
	return  nd_socket_openport_ex(AF_INET, port, type, protocol, listen_nums, NULL); 
}

ndsocket_t nd_socket_openport_v6(int port, int type, int protocol, int listen_nums)
{
	return  nd_socket_openport_ex(AF_INET6, port, type, protocol, listen_nums, NULL);
}

ndsocket_t nd_socket_connect(const char *host_name, short port, int sock_type, SOCKADDR_IN *out_addr)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	ndsocket_t sfd = -1;
	int s;
	char port_buf[20];
	//struct sockaddr_storage peer_addr;
	//socklen_t peer_addr_len;
	ndsnprintf(port_buf, sizeof(port_buf), "%d", port);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = sock_type; /* Datagram socket */
	hints.ai_flags = 0;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */

	s = getaddrinfo(host_name, port_buf, &hints, &result);
	if (s != 0) {
		//nd_logfatal("getaddrinfo: %s\n" AND gai_strerror(s));
		nd_logerror("getaddrinfo: %s\n", nd_last_error()) ;
		return -1;
	}

	/* getaddrinfo() returns a list of address structures.
	Try each address until we successfully bind(2).
	If socket(2) (or bind(2)) fails, we (close the socket
	and) try the next address. */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
			if (out_addr ) {
				memcpy(out_addr, rp->ai_addr, rp->ai_addrlen);
			}
			break;                  /* Success */
		}
		nd_socket_close(sfd);
		sfd = -1;
	}

	freeaddrinfo(result);           /* No longer needed */
	if (sfd == -1) {               /* No address succeeded */
		nd_logfatal("create socket and connected error\n");
		return -1;
	}
	nd_logmsg("   CONNECTED --> %s : %d SUCCESS!!!\n", host_name, port);
	return sfd;
}

//get ip-address from in
const char *nd_inet_ntoa (ndip_t in, char *buffer, size_t size)
{
	if (in.sin_family == AF_INET6) {
		return inet_ntop(AF_INET6, in.ip6, buffer,(socklen_t) size);
	}
	else {
		struct in_addr inaddr;
		inaddr.s_addr = in.ip;
		if (!buffer) {
			return inet_ntoa(inaddr);
		}
		else {
			return ndstrncpy(buffer, inet_ntoa(inaddr), size);
		}
	}	
}

int nd_sockadd_to_ndip(SOCKADDR_IN *sockaddr, ndip_t *ip)
{
	ip->sin_family = sockaddr->sin_family;
	if (sockaddr->sin_family == AF_INET) {
		ip->ip = sockaddr->sin_addr.s_addr;
		return 0;
	}
	else if (sockaddr->sin_family == AF_INET6) {
		struct sockaddr_in6 *paddr6 = (struct sockaddr_in6 *)sockaddr;
		memcpy(ip->ip6, &paddr6->sin6_addr, sizeof(ip->ip6));
		return 0;
	}
	return -1;
}

ndip_t nd_inet_aton(const char *ipaddr)
{
	ndip_t ipret = ND_IP_INIT;
	if (ndstrchr(ipaddr, ':')) {
		inet_pton(AF_INET6, ipaddr, &ipret.ip6);
		ipret.sin_family = AF_INET6;
	}
	else {
		ipret.ip = inet_addr(ipaddr); 
	}
	return ipret;
}


NDUINT64 nd_hton64(NDUINT64 h)
{
    if (nd_byte_order()==ND_B_ENDIAN) {
        return h ;
    }
    else {

        NDUINT64 ret ;
        NDUINT8 *src  = (NDUINT8 *) &h ;
        NDUINT8 *dest = (NDUINT8 *) &ret ;
        int i;
        for( i=0 ; i<8 ; i++) {
            dest[i] = src[7-i] ;
        }
        return ret ;
    }
}

NDUINT64 nd_ntoh64(NDUINT64 h)
{
    if (nd_byte_order()==ND_B_ENDIAN) {
        return h ;
    }
    else {

        NDUINT64 ret ;
        NDUINT8 *src  = (NDUINT8 *) &h ;
        NDUINT8 *dest = (NDUINT8 *) &ret ;
        int i;
        for( i=0 ; i<8 ; i++) {
            dest[i] = src[7-i] ;
        }
        return ret ;
    }

}

/*
 * wait socket writable
 * return value : 0 time out , -1 error ,else writablity
 */
int nd_socket_wait_writablity(ndsocket_t fd,int timeval)
{
	ENTER_FUNC()
	int ret;
	fd_set rfds;
	struct timeval tmvel ;

	FD_ZERO(&rfds) ;
	FD_SET(fd,&rfds) ;

	if(-1==timeval){
		ret = select (fd+1, NULL, &rfds,  NULL, NULL) ;
	}
	else {
		tmvel.tv_sec = timeval/1000 ;
		tmvel.tv_usec = (timeval%1000) * 1000;
		ret = select (fd+1, NULL, &rfds,  NULL, &tmvel) ;
	}

	if(ret <=0 ) {
		LEAVE_FUNC() ;
		return ret ;
	}

	LEAVE_FUNC() ;
	return FD_ISSET(fd, &rfds) ;
}

ndip_t nd_sock_getip(ndsocket_t fd)
{
	ENTER_FUNC()
	ndip_t ret = ND_IP_INIT;
	struct sockaddr_in6 saddr = {0};
	socklen_t addr_len=sizeof(saddr);
	if(-1==getsockname(fd,(struct sockaddr *)&saddr,&addr_len) ) {
		nd_logdebug("get ip error: %s\n" AND nd_last_error()) ;
		LEAVE_FUNC() ;
		return ret ;
	}

	ret.sin_family = saddr.sin6_family;
	if (saddr.sin6_family == AF_INET6) {
		memcpy(ret.ip6, &saddr.sin6_addr, sizeof(ret.ip6));
	}
	else {
		ret.ip = ((struct sockaddr_in*)&saddr)->sin_addr.s_addr;
	}
	LEAVE_FUNC() ;
	return ret;
}

ndport_t nd_sock_getport(ndsocket_t fd)
{
	struct sockaddr_in6 saddr = {0};
	socklen_t addr_len=sizeof(saddr);
	if(-1==getsockname(fd,(struct sockaddr *)&saddr,&addr_len) ) {
		nd_logdebug("getsockname error: %s\n" AND nd_last_error()) ;
		return 0 ;
	}
	return saddr.sin6_port ;
}

ndip_t nd_sock_getpeerip(ndsocket_t fd)
{
	ENTER_FUNC()
	ndip_t ret = ND_IP_INIT;
	struct sockaddr_in6 saddr = { 0 };
	socklen_t addr_len = sizeof(saddr);
	if (-1 == getpeername(fd, (struct sockaddr *)&saddr, &addr_len)) {
		nd_logdebug("get ip error: %s\n" AND nd_last_error());
		LEAVE_FUNC();
		return ret;
	}

	ret.sin_family = saddr.sin6_family;
	if (saddr.sin6_family == AF_INET6) {
		memcpy(ret.ip6, &saddr.sin6_addr, sizeof(ret.ip6));
	}
	else {
		ret.ip = ((struct sockaddr_in*)&saddr)->sin_addr.s_addr;
	}
	LEAVE_FUNC();
	return ret;
}

ndport_t nd_sock_getpeerport(ndsocket_t fd)
{
	struct sockaddr_in6 saddr = {0};
	socklen_t addr_len=sizeof(saddr);
	if(-1==getpeername(fd,(struct sockaddr *)&saddr,&addr_len) ) {
		nd_logdebug("getpeername error: %s\n" AND nd_last_error()) ;
		return 0 ;
	}
	return saddr.sin6_port ;
}
/*
 * wait socket readable
 * return value : 0 time out , -1 error ,else readable
 */
int nd_socket_wait_read(ndsocket_t fd,int timeval)
{
	ENTER_FUNC()
	int ret;
	fd_set rfds;
	struct timeval tmvel ;

	FD_ZERO(&rfds) ;
	FD_SET(fd,&rfds) ;

	if(-1==timeval){
		ret = select (fd+1, &rfds,NULL,   NULL, NULL) ;
	}
	else {
		tmvel.tv_sec = timeval/1000 ;
		tmvel.tv_usec = (timeval%1000) * 1000;
		ret = select (fd+1, &rfds,  NULL, NULL, &tmvel) ;
	}

	if(ret <=0 ) {
		LEAVE_FUNC() ;
		return ret ;
	}

	LEAVE_FUNC() ;
	return FD_ISSET(fd, &rfds) ;
};


int nd_sockadd_in_cmp(SOCKADDR_IN *src_addr, SOCKADDR_IN *desc_addr)
{
	if (src_addr->sin_family != desc_addr->sin_family) {
		return 1;
	}
	else if (src_addr->sin_family == AF_INET) {
		return !((src_addr->sin_port == desc_addr->sin_port) &&
			((u_long)src_addr->sin_addr.s_addr == (u_long)desc_addr->sin_addr.s_addr));
	}
	else if (src_addr->sin_family == AF_INET6) {
		SOCKADDR_IN6 *src6 =(SOCKADDR_IN6 *) src_addr;
		SOCKADDR_IN6 *dest6 = (SOCKADDR_IN6 *)desc_addr;

		if (src6->sin6_port != src6->sin6_port) {
			return 1;
		}

		NDUINT64 *psrc = (NDUINT64 *)&src6->sin6_addr;
		NDUINT64 *pdest = (NDUINT64 *)&dest6->sin6_addr;
		
		if ( psrc[0] != pdest[0] || psrc[1] != pdest[1]) {
			return 1;
		}

		return 0;
	}
	return 1;
}

int nd_socket_nonblock(ndsocket_t fd, int cmd)
{
	int val = cmd;
	return ioctlsocket(fd,FIONBIO ,&val) ;
}

//get checksum
NDUINT16 nd_checksum(NDUINT16 *buf,size_t length)
{
	ENTER_FUNC()
	register size_t len=length>>1;
	register unsigned long  sum= 0;

	for(sum=0;len>0;len--){
		sum += *buf++;
	}
	if(length&1){
		sum += *(NDUINT8*)buf ;	
	}
	sum=(sum>>16)+(sum&0xffff);
	sum+=(sum>>16);

	LEAVE_FUNC() ;
	return (NDUINT16)(~sum);
}
