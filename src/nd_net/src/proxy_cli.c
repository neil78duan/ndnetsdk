/* file proxy_cli.c
 * raw socket , send syn or icmp packet
 * 
 * all right reserved by neil duan 
 */

#include "nd_net/nd_netlib.h"
//#include "nd_common/nd_common.h"

enum prox_ack_code{

	PROXYERROR_NOERROR ,
	PROXYERROR_NOCONN ,			//Can't connect to proxy server, use GetLastError for more information
	PROXYERROR_REQUESTFAILED ,	//Request failed, can't send data
	PROXYERROR_AUTHREQUIRED ,	//Authentication required
	PROXYERROR_AUTHTYPEUNKNOWN ,//Authtype unknown or not supported
	PROXYERROR_AUTHFAILED ,		//Authentication failed
	PROXYERROR_AUTHNOLOGON ,
	PROXYERROR_CANTRESOLVEHOST 
};

//Error codes
int send_init_proxy(ndsocket_t fd,const char *host, short port, struct nd_proxy_info *proxy, int is_udp,SOCKADDR_IN *proxy_addr) ;
//int base64_encode( const char * source, int len, char * destination_string ) ;

ndsocket_t nd_proxy_connect(const char *host, short port,SOCKADDR_IN *out_addr, struct nd_proxy_info *proxy, int is_udp)
{
	ndsocket_t fd;
	SOCKADDR_IN proxy_addr ;
	
	fd = nd_socket_connect(proxy->proxy_host,  proxy->proxy_port,SOCK_STREAM, &proxy_addr) ;
	//fd = nd_socket_tcp_connect(proxy->proxy_host,  proxy->proxy_port,out_addr);
	if(fd <= 0) {
		return -1;
	}
	if( 0!=send_init_proxy(fd, host, port, proxy,is_udp,out_addr) )  {
		nd_socket_close(fd) ;
		return -1 ;
	}
	if (out_addr && proxy->proxy_type==ND_PROXY_SOCKS5) {
		out_addr->sin_family = proxy_addr.sin_family ;
	}
	return fd ;
}


int send_init_proxy(ndsocket_t fd,const char *host, short port, struct nd_proxy_info *proxy, int is_udp,SOCKADDR_IN *proxy_addr)
{
	int len ,ret ;
	char command[1024] ;
	if (proxy->proxy_type==ND_PROXY_SOCKS4 || proxy->proxy_type==ND_PROXY_SOCKS4A) { 
		 SOCKADDR_IN sock_addr ;
		//SOCKS4 proxy
		 sock_addr.sin_family = AF_INET;

		command[0]=4;
		command[1]=1; //CONNECT or BIND request
		*(short*)(&command[2]) = htons(port);

		if(-1 == get_sockaddr_in(host, port,&sock_addr) ) {			
			command[4]=0;
			command[5]=0;
			command[6]=0;
			command[7]=1;
			ndstrcpy(&command[9],host);
			len = (int)ndstrlen(host)+10;
		}
		else {
			*(unsigned long *)&command[4] = sock_addr.sin_addr.s_addr ;
			len = 9;
		}
	}
	else if (proxy->proxy_type==ND_PROXY_SOCKS5) { 
		command[0]=5;
		//cleartext username/password (if set) logon
		command[1]=1; //Number of logon types
		command[2]=0; //2=user/pass, 0=no logon
		len = 3; //length of request
	}
	else if (proxy->proxy_type==ND_PROXY_HTTP11) {

		ndsprintf(command, "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n", host, port,host, port);

		if (!proxy->user[0])
			ndstrcat(command, "\r\n") ;
		else	{
			char userpass[PROXY_USER_SIZE<<1];
			char base64str[PROXY_USER_SIZE<<1];
			ndsprintf(userpass, "%s:%s", proxy->user, proxy->password[0]? proxy->password :"");

			base64_encode(userpass, (int)ndstrlen(userpass), base64str) ;			
			ndstrcat(command, "Authorization: Basic ");
			ndstrcat(command, base64str);
			ndstrcat(command, "\r\nProxy-Authorization: Basic ");
			ndstrcat(command, base64str);
			ndstrcat(command, "\r\n\r\n");
		}
		len =(int) ndstrlen(command);		
	}
	else
		return -1;
	
	//send data and wait ack
	ret = nd_socket_tcp_write(fd, command, len) ;
	if (ret != len) {
		return -1;
	}
	if(nd_socket_wait_read(fd, 10*1000) <= 0) {
		return -1 ;
	}
	ret = nd_socket_tcp_read(fd, command, sizeof(command)) ;
	if(ret <=0 || ret >= sizeof(command))
		return -1 ;

	//parse ack
	if (proxy->proxy_type==ND_PROXY_SOCKS4 || proxy->proxy_type==ND_PROXY_SOCKS4A) { 
		if (command[1]!=90 || command[0]!=0)
			return -1;

	}
	else if (proxy->proxy_type==ND_PROXY_SOCKS5) { 
		SOCKADDR_IN sock_addr ;
		if (command[0]!=5)
			return -1;

		if(PROXYERROR_AUTHREQUIRED==command[1]) {
			char userpass[PROXY_USER_SIZE<<1];
			if(!proxy->user[0]) {
				return -1 ;
			}
			userpass[0] = 5;
			ndstrcpy(userpass+1, proxy->user) ;
			len =(int) ndstrlen(proxy->user) ;
			userpass[1] = len ;
			len += 1 ;

			if(proxy->password[0]) {
				int pwdlen =(int) ndstrlen(proxy->password) ;
				userpass[len++] = pwdlen ;
				ndstrcpy(userpass+len, proxy->password) ;
				len += pwdlen ;
			}
			else {
				userpass[len++] = 0 ;
			}
			ret = nd_socket_tcp_write(fd, userpass, len) ;
			if (ret != len) {
				return -1;
			}
			//wait auth ack 
			if(nd_socket_wait_read(fd, 10*1000) <= 0) {
				return -1 ;
			}
			ret = nd_socket_tcp_read(fd, command, sizeof(command)) ;
			if(ret <=0)
				return -1 ;
			if (command[0]!=5 || command[1] != 0 )
				return -1;
		}
		else if(command[1])
			return -1 ;		
		//connect		
		command[0]=5;			
		command[1] = is_udp ? 3:1;		//1 connect ,2 bind listen , 3 udp
		command[2]=0;	

		if (is_udp)	{
			command[3]=1;		//ipv4 address
			*(int*)&command[4] = nd_sock_getip(fd).ip ;
			*(short *) &command[8] = htons(port) ;
			//*(int*)&command[4] = 0 ;
			//*(short *) &command[8] = 0 ;
			len = 10 ;
		}
		else  if(0 == get_sockaddr_in(host, port,&sock_addr) ) {
			command[3]=1;		//ipv4 address
			*(int*)&command[4] = sock_addr.sin_addr.s_addr ;
			*(short *) &command[8] = htons(port) ;
			len = 10 ;
		}
		else {
			command[3]=3;	
			len = 4 ;
			command[len]=(int)ndstrlen(host);
			ndstrcpy(&command[len+1], host);
			len += command[len] + 1;

			*(short *) &command[len] = htons(port) ;
			len += 2 ;
		}

		//send connect
		ret = nd_socket_tcp_write(fd, command, len) ;
		if (ret != len) {
			return -1;
		}
		if(nd_socket_wait_read(fd, 10*1000) <= 0) {
			return -1 ;
		}
		ret = nd_socket_tcp_read(fd, command, sizeof(command)) ;
		if(ret <=0)
			return -1 ;

		if (command[0]!=5 || command[1] != 0 || ret < 10)
			return -1;
		//get socket address in proxy server
		proxy_addr->sin_addr.s_addr = *(int *) &command[4]  ;
		proxy_addr->sin_port = *(short *) &command[8]  ;
	}
	else if (proxy->proxy_type==ND_PROXY_HTTP11) {
		char *p = command + len - 4 ;
		int len =(int) ndstrlen(command) ;			// tail is "\r\n\r\n"
		if(len >= sizeof(command) || len <8 ) {
			return -1 ;
		}
		if (p[0]==13 && p[1]==10 && p[2]==13 && p[3]==10) {
			//return 0 ;
		}
		else 
			return -1 ;
		
	}
	return 0;
}

//send udp data through proxy
int nd_proxy_sendtoex(ndsocket_t fd, const char *data, size_t size, const char *remotehost, short port, SOCKADDR_IN *proxy)
{
	int len ;
	char buf[MAX_UDP_LEN] ;


	buf[0] = buf[1] =buf[2] = 0 ;
	buf[3]=3;
	
	len = 4 ;
	buf[len]= (unsigned char)ndstrlen(remotehost);
	ndstrcpy(&buf[len+1], remotehost);
	len += buf[len] + 1;

	*(short *) &buf[len] = htons(port) ;
	len += 2 ;

	if(size >= MAX_UDP_LEN - len)
		return -1;
	memcpy(buf+10, data, size) ;
	return nd_socket_udp_write(fd,(char*)buf, size+len ,proxy) ;
	//return send(fd, data, size,0) ;
}

int nd_proxy_sendto(ndsocket_t fd, void *data, size_t size, SOCKADDR_IN *remoteaddr, SOCKADDR_IN *proxy)
{
	unsigned char buf[MAX_UDP_LEN] ;

	nd_assert(size < MAX_UDP_LEN -10);
	//if(size >= MAX_UDP_LEN -10)
	//	return -1;

	buf[0] = buf[1] =buf[2] = 0 ;
	buf[3]=1;
	*(int*) &buf[4] =(int) remoteaddr->sin_addr.s_addr ;
	*(short*) &buf[8] =(int) remoteaddr->sin_port ;

	memcpy(buf+10, data, size) ;
	
	return nd_socket_udp_write(fd,(char*)buf, size+10 ,proxy) ;
	//return send(fd,(char*) buf, size+10 ,0) ;
}
