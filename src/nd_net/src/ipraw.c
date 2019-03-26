/* file ipraw.c
 * raw socket , send syn or icmp packet
 * 
 * 2008-9-6 
 * neil duan 
 */

#include "nd_net/nd_netlib.h"

//#include "nd_net/nd_sock.h"
//#include "nd_net/nd_iphdr.h"

/*
 * do not work in windows
 */
#if  !defined(ND_UNIX) 

#if _MSC_VER < 1300 // 1200 == VC++ 6.0
#include <WS2TCPIP.H>
#else 
#define IP_HDRINCL                 2 // Header is included with data.
#endif

#else 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netinet/ip.h> 
#include <netinet/tcp.h> 
#include <netdb.h> 

//#ifndef __MAC_OS__
#if !defined(__ND_MAC__) && !defined(__ND_IOS__)
#include <netpacket/packet.h>
#include <net/ethernet.h>
#endif

#endif

int set_raw_sockopt(ndsocket_t fd, int is_hdrincl)
{

	/* set time out */
#if !defined(ND_UNIX) 
	int opt = 1000;
	if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&opt, sizeof(opt))==-1){
		return -1 ;
	} 
	opt=1000;
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&opt,sizeof(opt)) ;
#else 
	struct timeval timeout ;			
	timeout.tv_sec = 1 ;			// one second 
	timeout.tv_usec = 0 ;
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (void*)&timeout,sizeof(timeout)) ;
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (void*)&timeout,sizeof(timeout)) ;
#endif 

	return 0;
}

int set_raw_iphdr(ndsocket_t fd, int flag)
{
	return setsockopt(fd, IPPROTO_IP, IP_HDRINCL, (char*)&flag, sizeof(flag));
}

int syn_packet_make(void *buf,  SOCKADDR_IN *src, SOCKADDR_IN *dest)
{
	pseudo_tcp *ps_tc ;
	struct ip_packet {
		ip_hdr ip ;
		tcp_hdr  tcp ;
		char data[0] ;
	}*t_packet = buf ;

	memset(t_packet,0, sizeof(*t_packet)) ;

	t_packet->tcp.source = src->sin_port ;
	t_packet->tcp.dest = dest->sin_port ;
	t_packet->tcp.seq =(u_32) rand() ;
	t_packet->tcp.doff = sizeof(tcp_hdr) >> 2;
	t_packet->tcp.syn = 1 ;						//set syn bit 
	t_packet->tcp.window = htons(8192) ;

	ps_tc = (pseudo_tcp*) (t_packet->data - sizeof(pseudo_tcp)) ;

	ps_tc->dest_addr =(u_32) dest->sin_addr.s_addr ;
	ps_tc->src_addr =(u_32) src->sin_addr.s_addr ;

	ps_tc->protocol = IPPROTO_TCP ;
	ps_tc->tcp_length = htons(sizeof(tcp_hdr) );

	ps_tc->tcp.check = nd_checksum((NDUINT16*)ps_tc, sizeof(pseudo_tcp)) ;

	memset((void*)t_packet, 0, sizeof(ip_hdr)) ;

	t_packet->ip.ip_hl = sizeof(ip_hdr)>>2 ;
	t_packet->ip.ip_v = ND_IPVERSION ;
	t_packet->ip.ip_id =(u_16) nd_processid() ;
	t_packet->ip.ip_tos =  0;
	t_packet->ip.ip_len = sizeof(t_packet);
	t_packet->ip.ip_ttl = ND_TTL ;
	t_packet->ip.ip_p = IPPROTO_TCP;
	t_packet->ip.ip_src = (u_32) src->sin_addr.s_addr ;
	t_packet->ip.ip_dst = (u_32) dest->sin_addr.s_addr ; 

	t_packet->ip.ip_sum = nd_checksum((NDUINT16*)&t_packet, sizeof(ip_hdr)) ;
	return sizeof(*t_packet);
}

int udp_packet_make(void *buf, char *data, int len, SOCKADDR_IN *src, SOCKADDR_IN *dest)
{
	u_16 check_sum ;
	size_t packet_size ;
	psuedo_udp *ps_udp ;
	struct packet {
		ip_hdr ip ;
		udp_hdr  udp ;
		char data[0] ;
	}*u_packet = buf ;

	memset((void*)u_packet, 0, sizeof(ip_hdr)+sizeof(udp_hdr));

	if(len<=0 || len > RAW_SYSTEM_BUF)
		return -1 ;

	ps_udp = (psuedo_udp*) (u_packet->data - sizeof(psuedo_udp)) ;

	ps_udp->dest_addr =(u_32) dest->sin_addr.s_addr ;
	ps_udp->src_addr =(u_32) src->sin_addr.s_addr ;
	ps_udp->protocol = IPPROTO_UDP ;

	ps_udp->udp.src_port = src->sin_port ;
	ps_udp->udp.dest_portno = dest->sin_port ;
	ps_udp->udp.length = ntohs((int)sizeof(udp_hdr) + len ) ;

	ps_udp->udp_length = ps_udp->udp.length ;
	memcpy(u_packet->data, data, len) ;


	check_sum = nd_checksum((NDUINT16*)ps_udp, sizeof(psuedo_udp) + len ) ;

	memset((void*)ps_udp, 0, sizeof(*ps_udp)-sizeof(udp_hdr)) ;

	packet_size = sizeof(ip_hdr)+sizeof(udp_hdr) + len ;

	u_packet->ip.ip_hl = sizeof(ip_hdr)>>2 ;
	u_packet->ip.ip_v = ND_IPVERSION ;
	u_packet->ip.ip_id = 0;		//unique identifier : set to 0
	u_packet->ip.ip_tos =  0;
	u_packet->ip.ip_len = htons((NDUINT16)packet_size) ;
	u_packet->ip.ip_ttl = ND_TTL ;
	u_packet->ip.ip_p = IPPROTO_UDP;
	u_packet->ip.ip_src = (u_32) src->sin_addr.s_addr ;
	u_packet->ip.ip_dst = (u_32) dest->sin_addr.s_addr ; 

	u_packet->ip.ip_sum = nd_checksum((NDUINT16*)u_packet, sizeof(ip_hdr)) ;

	u_packet->udp.checksum = check_sum ;

	return (int)packet_size;

}
//向指定的地点发送IP包,并且把源地址设定位src
int send_raw_udp(ndsocket_t fd, char *data, int len, SOCKADDR_IN *src, SOCKADDR_IN *dest)
{
	size_t packet_size ;
	char buf[RAW_SYSTEM_BUF] ;

	packet_size = udp_packet_make((void*)buf, data, len, src, dest);
	if(-1==packet_size)
		return -1;
	return (int)sendto(fd,buf,(int) packet_size, 0,(SOCKADDR*)dest,(int)sizeof(*dest)) ;
}


int send_icmp(ndsocket_t fd, char *data, int len, SOCKADDR_IN *dest,int seq_no, int type, int code)
{
	struct packet {
		icmp_hdr icmp ;
		char data[RAW_SYSTEM_BUF] ;
	}u_packet  ;

	memset((void*)&u_packet, 0, sizeof(icmp_hdr));

	u_packet.icmp.icmp_code = code ;
	u_packet.icmp.icmp_type = type; //ICMP_ECHO ;
	u_packet.icmp.icmp_id = htons((u_16)nd_processid()) ;
	u_packet.icmp.icmp_sequence = seq_no ;
	u_packet.icmp.icmp_checksum = 0 ;
	u_packet.icmp.icmp_timestamp = nd_time() ;

	if(data && len >0  && len<RAW_SYSTEM_BUF) {
		memcpy(u_packet.data, data, len) ;
	}

	len +=  sizeof(icmp_hdr) ;
	u_packet.icmp.icmp_checksum = nd_checksum((NDUINT16*)&u_packet, len) ;
	return (int)sendto(fd,(char*)&u_packet, len,0,(SOCKADDR*)dest,sizeof(*dest)) ;
}

//发送标准的PING 包
int send_ping(ndsocket_t fd,  SOCKADDR_IN *dest,int seq_no, char *data, int data_len)
{
	return send_icmp(fd, data,  data_len, dest, seq_no, ICMP_ECHO, 0) ;
}


//test remotehost is alive
//return <0 on error 
//else return time travel 
int test_remote_host(char *host)
{
	unsigned short pid ;
	int ret =-1,i;
	socklen_t sock_len;
	ndsocket_t raw_fd;
	SOCKADDR_IN dest, from  ;
	char recv_buf[RAW_SYSTEM_BUF];

#if defined(ND_UNIX)
	//setuid(getpid());
#endif
	raw_fd = open_raw(IPPROTO_ICMP) ;
	if(-1==raw_fd) {
		return -1;
	}
	
	dest.sin_family = AF_INET;
	get_sockaddr_in(host, 0, &dest) ;

	pid = (u_16)nd_processid();
	for (i=0; i<4; i++){
		int start ;
		ret =  send_ping(raw_fd,  &dest,i,NULL, 0);
		if(-1==ret) {
			goto EXIT_PING ;
		}
		start = nd_time() ;
		while (1) {
			if((nd_time()-start)>=1000 * 5){
				nd_logmsg("test host %s time out.\n" AND host);
				break;
			}
			sock_len = (socklen_t)sizeof(from) ;
			ret=(int)recvfrom(raw_fd,recv_buf,RAW_SYSTEM_BUF,0,(struct sockaddr *) &from, &sock_len);
			if(ret >= (int)sizeof(icmp_hdr)) {
				int icmp_len = 0 ;
				ndtime_t timelast ;
				ip_hdr * ip = (ip_hdr*)recv_buf;
				icmp_hdr  * icmp = (icmp_hdr*) (recv_buf + ip->ip_hl *4) ;

				if (ip->ip_p != IPPROTO_ICMP){
					continue ;
				}

				if(icmp->icmp_type!=ICMP_ECHOREPLY || ntohs(icmp->icmp_id) != pid) {
					continue ;
				}

				timelast = nd_time() - icmp->icmp_timestamp ;
				icmp_len = ret - ip->ip_hl *4 - sizeof(icmp_hdr) ;

				ret = timelast ;

				goto EXIT_PING ;
			}	//end if 
		}

	}
EXIT_PING :
	nd_socket_close(raw_fd) ;
	return ret;
}

//test remote tcp port is open 
//return 0 success
int test_remote_tcp_port(char *host, short port)
{
	int ret =0,i;
	socklen_t sock_len;
	int timeout ;
	ndsocket_t raw_fd;
	SOCKADDR_IN dest, from  ;
	struct ip_packet {
		ip_hdr ip ;
		tcp_hdr  tcp ;
		char data[0];
	}tcppacket ;
	char recv_buf[RAW_SYSTEM_BUF];

#if defined(ND_UNIX) 
	//setuid(getpid());
#endif
	raw_fd = open_raw(IPPROTO_TCP) ;
	if(-1==raw_fd) {
		return -1;
	}
	timeout=2000;
	//设置接受延迟
	setsockopt(raw_fd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout));


	dest.sin_family = AF_INET; 

	get_sockaddr_in(host, port, &dest) ;
	from.sin_family = AF_INET ;
	from.sin_port = htons(65534) ;
	from.sin_addr.s_addr = nd_get_ip().ip ;

	syn_packet_make(&tcppacket,  &from, &dest);

	for (i=0; i<3; i++){
		int start ;
		ret =  (int)sendto(raw_fd,(char*)&tcppacket,sizeof(tcppacket), 0, (SOCKADDR*)&dest,sizeof(dest)) ;
		if(-1==ret) {
			nd_showerror() ;
			goto EXIT_PING ;
		}
		start = nd_time() ;
		ret = -1 ;
		while (1)
		{
			if((nd_time()-start)>=1000 * 5){
				nd_logmsg("test host %s time out.\n" AND host);
				ret = -1 ;
				break;
			}
			sock_len = (socklen_t)sizeof(from) ;
			if(nd_socket_wait_read(raw_fd, 1000) <=0)
				continue ;
			ret=(int)recvfrom(raw_fd,recv_buf,RAW_SYSTEM_BUF,0,(struct sockaddr *) &from, &sock_len);
			nd_log_screen("reaceive data ret=%d\n" AND ret) ;
			if(ret>=(int)((sizeof(tcp_hdr) + sizeof(ip_hdr)))) {
				ip_hdr * ip = (ip_hdr*)recv_buf;
				tcp_hdr  * tcp = (tcp_hdr*) (recv_buf + ip->ip_hl *4) ;
				if(ip->ip_p != IPPROTO_TCP ) {
					ret = -1 ;
					continue ;
				}

				if(tcp->dest!=tcppacket.tcp.source || tcp->source != tcppacket.tcp.dest) {
					nd_log_screen("receive data address error dest-port=%d src-port=%d!\n" AND ntohs(tcp->dest) AND ntohs( tcp->source)) ;
					ret = -1 ;
					continue  ;
				}
				if(tcp->ack_seq == tcppacket.tcp.seq + 1 ) {
					pseudo_tcp *ps_tc , tmp;

					ps_tc = (pseudo_tcp*) (tcppacket.data - sizeof(pseudo_tcp)) ;

					tcppacket.tcp.seq++ ;
					tcppacket.tcp.ack =1 ;
					tcppacket.tcp.ack_seq = tcp->seq + 1 ;
					tcppacket.tcp.rst = 1 ;
					
					memcpy(&tmp,ps_tc,sizeof(tmp)) ;
					ps_tc->dest_addr =(u_32) dest.sin_addr.s_addr ;
					ps_tc->src_addr =(u_32) from.sin_addr.s_addr ;
					ps_tc->tcp.check = nd_checksum((NDUINT16*)ps_tc, sizeof(pseudo_tcp)) ;
					memcpy(&tmp,ps_tc,sizeof(tmp)) ;

					tcppacket.ip.ip_sum = nd_checksum((NDUINT16*)&tcppacket, sizeof(ip_hdr)) ;

					sendto(raw_fd,(char*)&tcppacket,sizeof(tcppacket), 0, (SOCKADDR*)&dest,sizeof(dest)) ;
					ret = 0;
					goto EXIT_PING ;	
				}
			}	//end if 
			else {
				ret = -1 ;
			}
		}

	}
EXIT_PING :
	nd_socket_close(raw_fd) ;
	return ret;
}

int raw_set_recvall(ndsocket_t raw_fd)
{

#if !defined(ND_UNIX) 
	
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)
// 	NDUINT32 lpvBuffer = 1;
// 	NDUINT32 lpcbBytesReturned = 0 ;
// 
// 	if(-1==WSAIoctl(raw_fd, SIO_RCVALL, &lpvBuffer, sizeof(lpvBuffer), NULL, 0, &lpcbBytesReturned, NULL, NULL) ) {
// 		return -1;
// 	}

#else
	/*struct ifreq ifr;

	ndstrncpy(ifr.ifr_name, eth_name,sizeof(ifr.ifr_name));
	if((ioctl(raw_fd, SIOCGIFFLAGS, &ifr) == -1))  {
		return -1;
	}

	ifr.ifr_flags |= IFF_PROMISC; 

	if(ioctl(raw_fd, SIOCSIFFLAGS, &ifr) == -1 ){
		return -1;
	}*/
#endif
	return 0;
}
