/* file : test_udp.c
 * test udp open port
 * 2008-8
 * all right reserved
 */

#include "nd_common/nd_common.h"

#include "nd_net/nd_netlib.h"
#include "nd_net/nd_netui.h"


#define PACKET_SIZE		1024	//����һ���������󳤶�

#define PACKET_HEAD_SIZE 8   //�������ݰ���ͷ������
#define PACKET_DATA_SIZE (PACKET_SIZE-8)

//����������Ϣ
#define DNM_RENEW		0			//���������������Լ�
#define DNM_REQUEST		1			//����������������Ϣ
#define DNM_CHAT		2			//������Ϣ
#define DNM_EXIT		3			//�뿪��Ϣ

typedef struct tagNetMsg 
{
	WORD checkSum ;			//У����
	WORD msgid ;
	DWORD param ;

	char data[PACKET_SIZE - sizeof(DWORD) *2] ;
}NetMsg_t;


//������Ϣ���
int NetMsgEntry(int fd , NetMsg_t *pMsg, int size, SOCKADDR_IN *addr_in)
{
	int len ;
	switch(pMsg->msgid)
	{
	case	DNM_RENEW:			//�յ��������������ĸ�����Ϣ
		
		break ;

	case DNM_REQUEST:			//�յ�����������������������,
		//UpdateInNet(addr_in,0) ;		//���Լ�����Ϣ���͸��Է�
		strcpy(pMsg->data, "linux") ;
		size = PACKET_HEAD_SIZE + 7 ;
		nd_socket_udp_write(fd, (char*)pMsg, size ,addr_in);
		break ;
	case DNM_CHAT:				//������Ϣ
		
		len = size - PACKET_HEAD_SIZE ;
		if(len>0 && len<=PACKET_DATA_SIZE) {
			pMsg->data[len] = 0 ;
			fprintf(stderr,"chat:%s\n", pMsg->data) ;
		}
		break ;
	case DNM_EXIT:				//
		//DelUser(addr_in);
		break ;
	default :
		break ;
	}
	return 0;
}

int udp_test()
{
	int read_len ;
	SOCKADDR_IN sock_add ;
	ndsocket_t sock_fd;
	char buf[1024] ;

	nd_net_init() ;
	sock_fd = nd_socket_openport(7828, SOCK_DGRAM,0,0,0);
	if(0==sock_fd) {
		return -1 ;
	}
	while( 1){
		if(kbhit()) {
			int ch = getch() ;
			if(ND_ESC==ch){
				printf_dbg("you are hit ESC, program eixt\n") ;
				break ;
			}
		}
		read_len = nd_socket_udp_read(sock_fd ,buf, 1024, &sock_add) ;
		if(read_len>0) {

// 			char buf[32] ;
// 			fprintf(stderr,"reced from %s:%d datalen=%d \n",  
// 				nd_inet_ntoa( sock_add.sin_addr.s_addr,buf ),
// 				htons(sock_add.sin_port) ,read_len);

			
			//NetMsgEntry(sock_fd , (NetMsg_t *)buf, read_len, &sock_add);
			//send_len = nd_socket_udp_write(sock_fd, buf, read_len ,&sock_add);
			//nd_assert(send_len==read_len) ;
		}
	}
	nd_socket_close(sock_fd) ;
	nd_net_destroy() ;
	return 0;
}

#if 0
int test_icmp_data()
{
	int read_len ;
	SOCKADDR_IN sock_add ;
	ndsocket_t sock_fd;
	char buf[1024] ;

	nd_net_init() ;
	

	//SOCKET sniffersock;
	//sniffsock = WSASocket(AF_INET, SOCK_RAW, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
#if !defined(ND_UNIX) 
	{
	struct in_addr  inadd ;
	sock_fd = open_raw(IPPROTO_IP) ;
	if(0==sock_fd) {
		return -1 ;
	}
	nd_socket_nonblock(sock_fd, 0) ;
	inadd.s_addr = nd_get_ip();
	raw_set_recvall(sock_fd, inet_ntoa(inadd) ) ;
	}
#else
	
	sock_fd = open_raw(IPPROTO_ICMP) ;
	if(0==sock_fd) {
		return -1 ;
	}
	nd_socket_nonblock(sock_fd, 0) ;

	raw_set_recvall(sock_fd, "eth0") ;
#endif
	while( 1){
		if(kbhit()) {
			int ch = getch() ;
			if(ND_ESC==ch){
				printf_dbg("you are hit ESC, program eixt\n") ;
				break ;
			}
		}
		//read_len = nd_socket_udp_read(sock_fd ,buf, 1024, &sock_add) ;
		//if(nd_socket_wait_read(sock_fd, 1000) <=0)
		//	continue ;
		read_len =icmp_data_recv(sock_fd ,buf, 1024, &sock_add) ;
		if(read_len>0) {

			char ipbuf[32] ;
			buf[read_len] = 0 ;
			fprintf(stderr,"reced from %s:%d datalen=%d [%s] \n",  
				nd_inet_ntoa( sock_add.sin_addr.s_addr ,ipbuf ),
				htons(sock_add.sin_port) ,read_len, buf);


			//NetMsgEntry(sock_fd , (NetMsg_t *)buf, read_len, &sock_add);
			//send_len = nd_socket_udp_write(sock_fd, buf, read_len ,&sock_add);
			//nd_assert(send_len==read_len) ;
		}
	}
	nd_socket_close(sock_fd) ;
	nd_net_destroy() ;
	return 0;
}
#endif
