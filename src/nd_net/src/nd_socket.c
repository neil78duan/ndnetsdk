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
	return send(fd, write_buf, (int) len, 0) ;
}
//reand tcp data from socket fd
int nd_socket_tcp_read(ndsocket_t fd, void *buf, size_t buflen)
{
	nd_assert(fd) ;
	nd_assert(buflen>0) ;
	return recv(fd, buf,(int)  buflen, 0);
}

/* write data to net
 * return value : write data len
 */
int nd_socket_udp_write(ndsocket_t fd, const char* data, size_t data_len ,SOCKADDR_IN* to_addr)
{
	int ret = 0 ;
	nd_assert(data && data_len > 0 && to_addr) ;
	ret = sendto(fd, data,(int)  data_len,0,(LPSOCKADDR)to_addr, (int) sizeof(*to_addr)) ;
	return ret ;
}
/* read date from net
 * input : udp fd , date buffer address and buffer size
 * output : from_addr
 * return value: read datalen
 */
int nd_socket_udp_read(ndsocket_t fd ,char *buf, size_t size, SOCKADDR_IN* from_addr)
{
	int read_len,sock_len = sizeof(SOCKADDR_IN) ;

	nd_assert(buf && size > 0) ;

	read_len  = recvfrom(fd, buf,(int)  size, 0, (LPSOCKADDR)from_addr, &sock_len )  ;
	return read_len  ;

/*	int sock_len, read_len ;
	SOCKADDR_IN *paddr, addr ;

	nd_assert(buf && size > 0) ;

	sock_len = sizeof(addr) ;
	if(from_addr) {
		paddr = from_addr ;
	}
	else {
		memset(&addr, 0, sizeof(addr)) ;
		paddr = &addr ;
	}

	read_len = recvfrom(fd, buf, size, 0, (LPSOCKADDR)paddr, &sock_len )  ;
#ifdef ND_DEBUG
	if(-1==read_len)
		nd_showerror();
#endif
	return read_len  ;
	*/
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
	HOSTENT *host ;

	nd_assert(sock_addr && host_name) ;
	if(!sock_addr ) {
		return -1 ;
	}

	host = gethostbyname((const char *)(host_name ));
	if(!host){
		return -1 ;
	}

	memset(sock_addr, 0, sizeof(sock_addr)) ;

	sock_addr->sin_family = AF_INET ;
	sock_addr->sin_port = htons(port) ;

	sock_addr->sin_addr = *((struct in_addr*)(host->h_addr)) ;
	return 0 ;
}

//get localhost ip
ndip_t nd_get_ip()
{
	char hostname[256];
	int ret = gethostname(hostname,   sizeof(hostname));
	if(0==ret) {
		HOSTENT *host ;

		host = gethostbyname((const char *)(hostname ));
		if(!host){
			return -1 ;
		}
		return *((ndip_t*)(host->h_addr)) ;
	}
	return 0;
}

int nd_sock_cmp_ip(ndip_t src, ndip_t dest, ndip_t ipmask)
{
	if (src == dest){
		return 0;
	}
	if (ipmask) {
		src = src & ipmask ;
		dest = dest & ipmask ;
		return !(src == dest) ;
	}
	else {
		int n;
		unsigned char *srcip = (char*)&src ;
		unsigned char *destip = (char*)&dest ;
		for( n=0; n<4; n++) {
			if (destip[n] != 0xff && srcip[n]!=0xff){
				if (destip[n] != srcip[n]){
					return 1;
				}
			}
		}
	}
	return 0 ;
}

#ifdef __LINUX__

ndsocket_t nd_socket_openport(int port, int type,int protocol,ndip_t bindip, int listen_nums)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int listen_fd = -1, s;
    char port_buf[20] ;
    //struct sockaddr_storage peer_addr;
    //socklen_t peer_addr_len;
    snprintf(port_buf,sizeof(port_buf), "%d", port) ;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = type; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE|AI_NUMERICSERV;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, port_buf, &hints, &result);
    if (s != 0) {
       nd_logfatal( "getaddrinfo: %s\n" AND gai_strerror(s));
       return -1;
    }

    /* getaddrinfo() returns a list of address structures.
      Try each address until we successfully bind(2).
      If socket(2) (or bind(2)) fails, we (close the socket
      and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if(INADDR_ANY != bindip) {
            if((ndip_t)rp->ai_addr->sa_family != htonl(bindip)) {
                continue ;
            }
        }
        listen_fd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
        if (listen_fd == -1)
            continue;

        if (bind(listen_fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(listen_fd);
    }

    if (listen_fd == -1) {               /* No address succeeded */
        nd_logfatal( "create socket and bind error\n");
        return -1;
    }

    freeaddrinfo(result);           /* No longer needed */

    if(-1==nd_socket_nonblock(listen_fd, 1) ) {
		goto OPEN_ERROR ;
	}

	if(type==SOCK_STREAM){
        int re_usr_addr = 1 ;
		if(-1==setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&re_usr_addr, sizeof(re_usr_addr)) ) {
			goto OPEN_ERROR ;
		}
		if(-1==listen(listen_fd, listen_nums)){
			goto OPEN_ERROR ;
		}
	}
	else if(type==SOCK_RAW) {
		if (protocol != IPPROTO_ICMP){
			if(-1==set_raw_iphdr(listen_fd,1) )
				goto OPEN_ERROR ;
		}
		if(-1==set_raw_sockopt(listen_fd) )
			goto OPEN_ERROR ;
    }
    return listen_fd ;

OPEN_ERROR:
	if(listen_fd>0) {
		nd_showerror() ;
		nd_socket_close(listen_fd) ;
	}
	return -1;
}

ndsocket_t nd_socket_connect(const char *host_name, short port,int sock_type, SOCKADDR_IN *out_addr)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd = -1, s;
    char port_buf[20] ;
    //struct sockaddr_storage peer_addr;
    //socklen_t peer_addr_len;
    snprintf(port_buf,sizeof(port_buf), "%d", port) ;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = sock_type; /* Datagram socket */
    hints.ai_flags = 0;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(host_name, port_buf, &hints, &result);
    if (s != 0) {
       nd_logfatal( "getaddrinfo: %s\n" AND gai_strerror(s));
       return -1;
    }

    /* getaddrinfo() returns a list of address structures.
      Try each address until we successfully bind(2).
      If socket(2) (or bind(2)) fails, we (close the socket
      and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            if(out_addr && rp->ai_addrlen <=sizeof(SOCKADDR_IN)) {
                memcpy(out_addr, &rp->ai_addr, rp->ai_addrlen) ;
            }
            break;                  /* Success */
        }
        close(sfd);
    }

    if (sfd == -1) {               /* No address succeeded */
        nd_logfatal( "create socket and bind error\n");
        return -1;
    }


    return sfd ;

}
#else

/* create a socket and bind
 * @port listened port
 * @type = SOCK_DGRAM create udp port
 * @type = SOCK_STREAM create udp tcp
 * @out_addr out put address of the socket
 * return -1 error else return socket
 */
ndsocket_t nd_socket_openport(int port, int type,int protocol,ndip_t bindip, int listen_nums)
{
	short tmp_port = (short)port;
	ndsocket_t listen_fd  ;					/* listen and connect fd */
	int re_usr_addr = 1 ;


	if (type==SOCK_DGRAM && tmp_port==0) {
		ndsocket_t tmp_fd = nd_socket_connect("127.0.0.1", 0, type, NULL) ;
		if(tmp_fd <= 0) {
			return -1 ;
		}
		tmp_port = nd_sock_getport(tmp_fd) ;
		nd_socket_close(tmp_fd) ;
	}
	else {
		tmp_port = htons(tmp_port);
	}

	/* zero all socket slot */
	listen_fd = (ndsocket_t ) socket(AF_INET, type, protocol) ;
	if(-1 == listen_fd){
		return -1;
	}
	if(type != SOCK_RAW && INADDR_NONE!= bindip){
		SOCKADDR_IN serv_addr  ;		/* socket address */
		serv_addr.sin_family = AF_INET ;
		if(bindip)
			serv_addr.sin_addr.s_addr = htonl(bindip) ;
		else
			serv_addr.sin_addr.s_addr = htonl(INADDR_ANY) ;
		serv_addr.sin_port = tmp_port;
		if(-1==bind (listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))){
			goto OPEN_ERROR ;
		}
	}

	if(-1==nd_socket_nonblock(listen_fd, 1) ) {
		goto OPEN_ERROR ;
	}

	if(type==SOCK_STREAM){
		if(-1==setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&re_usr_addr, sizeof(re_usr_addr)) ) {
			goto OPEN_ERROR ;
		}
		if(-1==listen(listen_fd, listen_nums)){
			goto OPEN_ERROR ;
		}
	}
	else if(type==SOCK_RAW) {
		if (protocol != IPPROTO_ICMP){
			if(-1==set_raw_iphdr(listen_fd,1) )
				goto OPEN_ERROR ;
		}
		if(-1==set_raw_sockopt(listen_fd) )
			goto OPEN_ERROR ;
		if(bindip &&  INADDR_NONE!= bindip) {
			SOCKADDR_IN serv_addr  ;		/* socket address */
			serv_addr.sin_family = AF_INET ;
			serv_addr.sin_addr.s_addr = htonl(bindip) ;
			serv_addr.sin_port = 0 ;
			if(-1==bind (listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))){
				goto OPEN_ERROR ;
			}
		}
	}

	return listen_fd ;
OPEN_ERROR:
	if(listen_fd) {
		nd_showerror() ;
		nd_socket_close(listen_fd) ;
	}
	return -1;
}


/* connect to server
 * @out put address of remote host
 * return -1 on error else return socket fd
 */
ndsocket_t nd_socket_connect(const char *host_name, short port,int sock_type, SOCKADDR_IN *out_addr)
{
	ENTER_FUNC()
	ndsocket_t conn_sock ;
	SOCKADDR_IN their_addr = {0} ;
	if(-1==get_sockaddr_in(host_name, port, &their_addr) ) {
		LEAVE_FUNC() ;
		return -1 ;
	}
	conn_sock = (ndsocket_t ) socket (AF_INET, sock_type, 0);
	if(-1==conn_sock) {
		LEAVE_FUNC() ;
		return -1;
	}
	if(-1==connect(conn_sock,(LPSOCKADDR)&their_addr, sizeof(struct sockaddr)) ) {
		LEAVE_FUNC() ;
		return -1 ;
	}

	if(out_addr)
		memcpy((void*)out_addr, (void*)&their_addr, sizeof(their_addr)) ;

	LEAVE_FUNC() ;
	return conn_sock;
}
#endif // __LINUX__


#if 0
/*
 * udp connect do not real connect
 * only create a socket !
 */
ndsocket_t nd_socket_udp_connect(const char *host_name, short port,SOCKADDR_IN *out_addr)
{
	ndsocket_t conn_sock ;

	if(out_addr)
		get_sockaddr_in(host_name, port, out_addr)  ;
	conn_sock = socket(AF_INET, SOCK_DGRAM, 0);

	/*
	 * to realizing p2p do not use system call 'bind' and 'connect'
	 */
	return conn_sock ;
}
#endif

//从ip地址int 到字符串形式
char *nd_inet_ntoa (unsigned int in, char *buffer)
{
	struct in_addr inaddr ;
	inaddr.s_addr = in ;
	if (!buffer) {
		return inet_ntoa(inaddr);
	}
	else {
		return strncpy(buffer, inet_ntoa(inaddr), 20);
	}
	/*
	char *p ;

	unsigned char *bytes = (unsigned char *) &in;
	if (buffer){
		p=buffer ;
	}
	else {
		p = _s_iptext;
	}
	snprintf (p, 20, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
	return p ;
	 */
}

ndip_t nd_inet_aton(const char *ipaddr)
{
	return (ndip_t) inet_addr(ipaddr) ;

	//int inet_aton(const char *cp, struct in_addr *pin);
	/*
	HOSTENT *host ;

	host = gethostbyname((ipaddr));
	if(!host){
		return 0 ;
	}
	return *((ndip_t*)(host->h_addr)) ;
*/
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
 * 等待socket可写
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
	struct sockaddr_in saddr = {0};
	socklen_t addr_len=sizeof(saddr);
	if(-1==getsockname(fd,(struct sockaddr *)&saddr,&addr_len) ) {
		nd_logdebug("get ip error: %s\n" AND nd_last_error()) ;
		LEAVE_FUNC() ;
		return 0 ;
	}
	LEAVE_FUNC() ;
	return (int) saddr.sin_addr.s_addr ;
}

ndport_t nd_sock_getport(ndsocket_t fd)
{
	struct sockaddr_in saddr = {0};
	socklen_t addr_len=sizeof(saddr);
	if(-1==getsockname(fd,(struct sockaddr *)&saddr,&addr_len) ) {
		nd_logdebug("getsockname error: %s\n" AND nd_last_error()) ;
		return 0 ;
	}
	return saddr.sin_port ;
}

ndip_t nd_sock_getpeerip(ndsocket_t fd)
{
	struct sockaddr_in saddr = {0};
	socklen_t addr_len=sizeof(saddr);
	if(-1==getpeername(fd,(struct sockaddr *)&saddr,&addr_len) ) {
		nd_logdebug("get peer ip error: %s\n" AND nd_last_error()) ;
		return 0 ;
	}
	return (int) saddr.sin_addr.s_addr ;
}

ndport_t nd_sock_getpeerport(ndsocket_t fd)
{
	struct sockaddr_in saddr = {0};
	socklen_t addr_len=sizeof(saddr);
	if(-1==getpeername(fd,(struct sockaddr *)&saddr,&addr_len) ) {
		nd_logdebug("getpeername error: %s\n" AND nd_last_error()) ;
		return 0 ;
	}
	return saddr.sin_port ;
}
/*
 * 等待socket可读
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
		ret = select (fd+1, NULL, &rfds,  NULL, NULL) ;
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

int nd_socket_nonblock(ndsocket_t fd, int cmd)
{
	int val = cmd;
	return ioctlsocket(fd,FIONBIO ,&val) ;
}

//校验和计算
NDUINT16 nd_checksum(NDUINT16 *buf,size_t length)
{
	ENTER_FUNC()
	register size_t len=length>>1;
	register unsigned long  sum= 0;

	for(sum=0;len>0;len--){
		sum += *buf++;
	}
	if(length&1){
		//sum+= (*buf&0xff00);		//这里出错了,这个程序是大尾数的,哈哈浪费了我很多时间啊
		sum += *(NDUINT8*)buf ;		//这个好一点,不过大小通吃
	}
	sum=(sum>>16)+(sum&0xffff);
	sum+=(sum>>16);

	LEAVE_FUNC() ;
	return (NDUINT16)(~sum);
}
