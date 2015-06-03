 /* file test_raw.c
 * test raw socket 
 *
 * neil duan
 * 2008-9-7
 */

#ifdef ND_DEBUG
#pragma comment(lib,"nd_common_dbg.lib")
#pragma comment(lib,"nd_net_dbg.lib")
#pragma comment(lib,"nd_crypt_dbg.lib")

#else 
#pragma comment(lib,"nd_common.lib")
#pragma comment(lib,"nd_net.lib")
#pragma comment(lib,"nd_crypt.lib")
#endif 

#pragma comment(lib, "Ws2_32.lib")

#include "nd_common/nd_common.h"

#include "nd_net/nd_netlib.h"
#include "nd_net/nd_netui.h"

/*
int sendTcp(unsigned short desPort, unsigned long desIP)
{
	WSADATA WSAData;
	SOCKET sock;
	SOCKADDR_IN addr_in;
	IPHEADER ipHeader;
	TCPHEADER tcpHeader;
	PSDHEADER psdHeader;
	
	char szSendBuf[MAX_LEN] = { 0 };
	BOOL flag;
	int rect, nTimeOver;
	
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
	printf("WSAStartup Error!\n");
	return false;
	}
	
	if ((sock = WSASocket(AF_INET, SOCK_RAW, IPPROTO_RAW, NULL, 0,
	WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
	printf("Socket Setup Error!\n");
	return false;
	}
	flag = true;
	if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (char*) &flag, sizeof(flag)) ==SOCKET_ERROR)
	{
	printf("setsockopt IP_HDRINCL error!\n");
	return false;
	}
	
	nTimeOver = 1000;
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*) &nTimeOver, sizeof
	(nTimeOver)) == SOCKET_ERROR)
	{
	printf("setsockopt SO_SNDTIMEO error!\n");
	return false;
	}
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(desPort);
	addr_in.sin_addr.S_un.S_addr = inet_addr(desIP);
	
	//填充IP报头
	ipHeader.h_verlen = (4 << 4 sizeof(ipHeader) / sizeof(unsigned long));
	// ipHeader.tos=0;
	ipHeader.total_len = htons(sizeof(ipHeader) + sizeof(tcpHeader));
	ipHeader.ident = 1;
	ipHeader.frag_and_flags = 0;
	ipHeader.ttl = 128;
	ipHeader.proto = IPPROTO_TCP;
	ipHeader.checksum = 0;
	ipHeader.sourceIP = inet_addr("localhost");
	ipHeader.destIP = desIP;
	
	//填充TCP报头
	tcpHeader.th_dport = htons(desPort);
	tcpHeader.th_sport = htons(SOURCE_PORT); //源端口号
	tcpHeader.th_seq = htonl(0x12345678);
	tcpHeader.th_ack = 0;
	tcpHeader.th_lenres = (sizeof(tcpHeader) / 4 << 4 0);
	tcpHeader.th_flag = 2; //标志位探测，2是SYN
	tcpHeader.th_win = htons(512);
	tcpHeader.th_urp = 0;
	tcpHeader.th_sum = 0;
	
	psdHeader.saddr = ipHeader.sourceIP;
	psdHeader.daddr = ipHeader.destIP;
	psdHeader.mbz = 0;
	psdHeader.ptcl = IPPROTO_TCP;
	psdHeader.tcpl = htons(sizeof(tcpHeader));
	
	//计算校验和
	memcpy(szSendBuf, &psdHeader, sizeof(psdHeader));
	memcpy(szSendBuf + sizeof(psdHeader), &tcpHeader, sizeof(tcpHeader));
	tcpHeader.th_sum = checksum((unsigned short*)szSendBuf, sizeof(psdHeader) + sizeof
	(tcpHeader));
	
	memcpy(szSendBuf, &ipHeader, sizeof(ipHeader));
	memcpy(szSendBuf + sizeof(ipHeader), &tcpHeader, sizeof(tcpHeader));
	memset(szSendBuf + sizeof(ipHeader) + sizeof(tcpHeader), 0, 4);
	ipHeader.checksum = checksum((unsigned short*)szSendBuf, sizeof(ipHeader) + sizeof
	(tcpHeader));
	
	memcpy(szSendBuf, &ipHeader, sizeof(ipHeader));
	
	rect = sendto(sock, szSendBuf, sizeof(ipHeader) + sizeof(tcpHeader), 0,
	(struct sockaddr*) &addr_in, sizeof(addr_in));
	if (rect == SOCKET_ERROR)
	{
	printf("send error!:%d\n", WSAGetLastError());
	return false;
	}
	else
	printf("send ok!\n");
	
	closesocket(sock);
	WSACleanup();
	
	return rect;
} 
*/

int ping(char *host)
{
	unsigned short pid ;
	int ret ,i, sock_len;
	int timeout ;
	ndsocket_t raw_fd;
	SOCKADDR_IN dest, from  ;
	char recv_buf[RAW_SYSTEM_BUF];

#ifndef  WIN32
	//setuid(getpid());
#endif
	raw_fd = open_raw(IPPROTO_ICMP) ;
	if(-1==raw_fd) {
		nd_showerror() ;
		printf("error open raw") ;
		getch() ;
		exit(1) ;
	}
	timeout=2000;
	//设置接受延迟
	setsockopt(raw_fd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout));
	
	get_sockaddr_in(host, 0, &dest) ;
	pid = (u_16)nd_processid() ;
	
	for (i=0; i<4; i++){
		int start ;
		//ret = send_icmp(raw_fd,NULL,0, &dest, i,ICMP_ECHO,pid);		
		ret =  send_ping(raw_fd,  &dest,i,NULL,0);
		if(-1==ret) {
			nd_showerror();
			goto EXIT_PING ;
		}
		start = nd_time() ;
		while (1)
		{
			if((nd_time()-start)>=1000 * 3){
				printf("Request timed out.\n");
				break;
			}
			sock_len = sizeof(from) ;
			ret=recvfrom(raw_fd,recv_buf,RAW_SYSTEM_BUF,0,(struct sockaddr *) &from, &sock_len);
			if(ret>=(int)((sizeof(icmp_hdr) + sizeof(ip_hdr)))) {
				int icmp_len = 0 ;
				ndtime_t timelast ;
				ip_hdr * ip = (ip_hdr*)recv_buf;
				icmp_hdr  * icmp = (icmp_hdr*) (recv_buf + ip->ip_hl *4) ;

				if(icmp->icmp_type!=ICMP_ECHOREPLY || ntohs(icmp->icmp_id) != pid) {
					printf("other Process Icmp data\n") ;
					break ;
				}

				timelast = nd_time() - icmp->icmp_timestamp ;
				icmp_len = ret - ip->ip_hl *4 - sizeof(icmp_hdr) ;

				printf("%d Reply from %s len=%d time=%d ttl=%d\n", 
					icmp->icmp_sequence,inet_ntoa(from.sin_addr) ,
					icmp_len, timelast,ip->ip_ttl ) ;
				break ;
			}	//end if 
		}

	}


EXIT_PING :
	printf("send data %d\npress any key to continue\n", ret) ;
	getch() ;

	nd_socket_close(raw_fd) ;

	return 0 ;
}

int main(int argc ,char *argv[])
{
	int ret ;

	nd_arg(argc, argv);
	nd_common_init() ;
	nd_net_init() ;
#if 1

	ret = test_remote_host("192.168.0.1") ;
	if(ret >=0 ) {
		printf("test host success packet_travel_time=%d\n",ret) ;
	}
	else {
		printf("test host error ret=%d\n",ret) ;
	}
	ping("192.168.0.1") ;
#else 
	{
		ndsocket_t raw_fd;
		SOCKADDR_IN dest, src ;
		char *ptext = "hello";

		raw_fd = open_raw(IPPROTO_UDP) ;
		//raw_fd = open_raw(IPPROTO_ICMP) ;
		if(-1==raw_fd) {
			nd_showerror() ;
			printf("error open raw\n") ;
			getch() ;
			exit(1) ;
		}
		dest.sin_family = AF_INET ;
		dest.sin_port = 0;
		dest.sin_addr.s_addr = inet_addr("192.168.1.12") ;

		src.sin_family= AF_INET ;
		src.sin_port = 0 ;
		src.sin_addr.s_addr = inet_addr("192.168.1.2") ;

		ret = send_raw_udp(raw_fd, ptext,strlen(ptext), &src, &dest) ;
		//ret = icmp_data_send(raw_fd,ptext,strlen(ptext),&dest, 0) ;
		if(-1==ret) {
			//WSAEWOULDBLOCK
			nd_showerror();
		}
		//nd_assert(ret==12) ;

		nd_socket_close(raw_fd) ;

		printf("send data %d\npress any key to continue\n", ret) ;
	}
	
#endif 

	getch() ;

	nd_net_destroy();

	nd_common_release() ;
	return 0;
}
