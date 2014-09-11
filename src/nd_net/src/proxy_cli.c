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
int base64_encode( const char * source, int len, char * destination_string ) ;

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
		out_addr->sin_family = AF_INET ;
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

		command[0]=4;
		command[1]=1; //CONNECT or BIND request
		*(short*)(&command[2]) = htons(port);

		if(-1 == get_sockaddr_in(host, port,&sock_addr) ) {			
			command[4]=0;
			command[5]=0;
			command[6]=0;
			command[7]=1;
			strcpy(&command[9],host);
			len = (int)strlen(host)+10;
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

		sprintf(command, "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n", host, port,host, port);

		if (!proxy->user[0])
			strcat(command, "\r\n") ;
		else	{
			char userpass[PROXY_USER_SIZE<<1];
			char base64str[PROXY_USER_SIZE<<1];
			sprintf(userpass, "%s:%s", proxy->user, proxy->password[0]? proxy->password :"");

			base64_encode(userpass, (int)strlen(userpass), base64str) ;			
			strcat(command, "Authorization: Basic ");
			strcat(command, base64str);
			strcat(command, "\r\nProxy-Authorization: Basic ");
			strcat(command, base64str);
			strcat(command, "\r\n\r\n");
		}
		len =(int) strlen(command);		
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
			strcpy(userpass+1, proxy->user) ;
			len =(int) strlen(proxy->user) ;
			userpass[1] = len ;
			len += 1 ;

			if(proxy->password[0]) {
				int pwdlen =(int) strlen(proxy->password) ;
				userpass[len++] = pwdlen ;
				strcpy(userpass+len, proxy->password) ;
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
			*(int*)&command[4] = nd_sock_getip(fd) ;
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
			command[len]=(int)strlen(host);
			strcpy(&command[len+1], host);
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
		int len =(int) strlen(command) ;			// tail is "\r\n\r\n"
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
	buf[len]= (unsigned char)strlen(remotehost);
	strcpy(&buf[len+1], remotehost);
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

#define END_OF_BASE64_ENCODED_DATA           ('=')
#define BASE64_END_OF_BUFFER                 (0xFD)
#define BASE64_IGNORABLE_CHARACTER           (0xFE)
#define BASE64_UNKNOWN_VALUE                 (0xFF)
#define BASE64_NUMBER_OF_CHARACTERS_PER_LINE (72)

int base64_encode( const char * source, int len, char * destination_string )
{

	const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	int loop_index                = 0;
	int number_of_bytes_to_encode = len;

	NDUINT8 byte_to_add = 0;
	NDUINT8 byte_1      = 0;
	NDUINT8 byte_2      = 0;
	NDUINT8 byte_3      = 0;

	NDUINT32 number_of_bytes_encoded = (NDUINT32) ( (double) number_of_bytes_to_encode / (double) 0.75 ) + 1;
	char * destination;

	number_of_bytes_encoded += (NDUINT32)( ( ( number_of_bytes_encoded / BASE64_NUMBER_OF_CHARACTERS_PER_LINE ) + 1 ) * 2 );

	destination = destination_string;

	number_of_bytes_encoded = 0;

	while( loop_index < number_of_bytes_to_encode ) {
		// Output the first byte
		byte_1 = source[ loop_index ];
		byte_to_add = alphabet[ ( byte_1 >> 2 ) ];

		destination[ number_of_bytes_encoded ] =  byte_to_add ;
		number_of_bytes_encoded++;

		loop_index++;

		if ( loop_index >= number_of_bytes_to_encode ) {
			// We're at the end of the data to encode
			byte_2 = 0;
			byte_to_add = alphabet[ ( ( ( byte_1 & 0x03 ) << 4 ) | ( ( byte_2 & 0xF0 ) >> 4 ) ) ];

			destination[ number_of_bytes_encoded ] = byte_to_add;
			number_of_bytes_encoded++;

			destination[ number_of_bytes_encoded ] =  END_OF_BASE64_ENCODED_DATA;
			number_of_bytes_encoded++;

			destination[ number_of_bytes_encoded ] =  END_OF_BASE64_ENCODED_DATA;

			destination[ number_of_bytes_encoded + 1 ] = 0;

			return 0;
		}
		else{
			byte_2 = source[ loop_index ];
		}

		byte_to_add = alphabet[ ( ( ( byte_1 & 0x03 ) << 4 ) | ( ( byte_2 & 0xF0 ) >> 4 ) ) ];

		destination[ number_of_bytes_encoded ] = byte_to_add;
		number_of_bytes_encoded++;

		loop_index++;

		if ( loop_index >= number_of_bytes_to_encode )
		{
			// We ran out of bytes, we need to add the last half of byte_2 and pad
			byte_3 = 0;

			byte_to_add = alphabet[ ( ( ( byte_2 & 0x0F ) << 2 ) | ( ( byte_3 & 0xC0 ) >> 6 ) ) ];

			destination[ number_of_bytes_encoded ] = byte_to_add;
			number_of_bytes_encoded++;

			destination[ number_of_bytes_encoded ] = END_OF_BASE64_ENCODED_DATA;

			destination[ number_of_bytes_encoded + 1 ] = 0;

			return 0;
		}
		else
		{
			byte_3 = source[ loop_index ];
		}

		loop_index++;

		byte_to_add = alphabet[ ( ( ( byte_2 & 0x0F ) << 2 ) | ( ( byte_3 & 0xC0 ) >> 6 ) ) ];

		destination[ number_of_bytes_encoded ] = byte_to_add;
		number_of_bytes_encoded++;

		byte_to_add = alphabet[ ( byte_3 & 0x3F ) ];

		destination[ number_of_bytes_encoded ] = byte_to_add;
		number_of_bytes_encoded++;

		if ( ( number_of_bytes_encoded % BASE64_NUMBER_OF_CHARACTERS_PER_LINE ) == 0 )
		{
			destination[ number_of_bytes_encoded ] = 13;		//»Ø³µreturn
			number_of_bytes_encoded++;

			destination[ number_of_bytes_encoded ] = 10;		//»»ÐÐ
			number_of_bytes_encoded++;
		}
	}

	destination[ number_of_bytes_encoded ] = END_OF_BASE64_ENCODED_DATA;

	destination[ number_of_bytes_encoded + 1 ] = 0;

	return 0;
}