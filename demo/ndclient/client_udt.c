/* file : test_client.c
 * client connect test 
 * 2007-10
 */

#pragma comment(lib,"nd_common.lib")
#pragma comment(lib,"nd_net.lib")

#include "nd_common/nd_common.h"
#include "nd_srvcore/nd_srvlib.h"
#include "nd_srvcore/nd_listensrv.h"
#include "nd_net/nd_netlib.h"
#include "nd_net/nd_udt.h"
#include <time.h>

char *__send_file, *__recv_file ;
#define _INPUT_FILE __send_file
#define _OUTPUT_FILE __recv_file
#define _READ_SIZE  NETDATA_SIZE

volatile int __exit = 0;


int datagram_entry(nd_udt_node *new_socket, void *data, size_t len) 
{
	if(0==len) {
		return 0 ;
	}
	else {
		char *p = data ;
		p[len]=0;
		printf("recv DATAGRAM len =%d\n [%s]\n" , len, p) ;
	}
	return 0;
}

int test_connect(char *host, int port)
{
	int ret =0;
	nd_udt_node *sock_node = udt_cli_socket(AF_INET,SOCK_DGRAM,0) ;
	if(!sock_node) 
		return -1 ;
	if(-1==udt_cli_connect(sock_node,(short)port,host) ) {
		//udt_cli_close(sock_node);
		ret = -1 ;
		goto OUT_1;
	}

OUT_1:
	udt_cli_close(sock_node,0);
	return ret ;
}
int main(int argc , char *argv[])
{	
	int val = 0;
	int ret ,i;
	u_32 *p_entry =(u_32 *) datagram_entry;
//	ndthread_t recv_id;
//	ndth_handle hRecv;
	//struct nd_tcp_node conn_node ;

	nd_udt_node socket_node ;

	char buf[1024] ;
	if(argc < 3) {
		ndprintf("usage: %s host port \n",argv[0]) ;
		getch();
		exit(1) ;
	}
	NDTRAC("START NET\n") ;
	//_INPUT_FILE = argv[3] ;
	//_OUTPUT_FILE = argv[4];
	srand( (unsigned)time( NULL ) );

	nd_common_init() ;
	
	nd_net_init() ;
	//ret = test_connect(argv[1], atoi(argv[2])) ;
	//printf("connect %s press any key to exit!" ,ret?"fail":"success") ;
	//getch();


	if(!udt_connect( argv[1] , (short) atoi(argv[2]),&socket_node)){
		//nd_assert(0) ;
		ndprintf("connect error[%s]\n",nd_last_error()) ;
		getch();
		return -1;
	}
	printf("connect success press any key to close connect!") ;
	getch();

	udt_ioctl(&socket_node,UDT_IOCTRL_DRIVER_MOD,&val);

	udt_ioctl(&socket_node,UDT_IOCTRL_SET_DATAGRAM_ENTRY,p_entry);
	
	udt_send(&socket_node,"hello world",13) ;

	//ret = udt_recv(&socket_node,buf, 1024) ;
	//update_socket(&socket_node);
	i= 0 ;
	while ((ret=udt_recv(&socket_node,buf, 1024)) > 0){
		buf[ret] = 0 ;
		printf("recv data len=%d\n[%s]\n", ret, buf) ;
		sprintf(buf,"hello world %d",i) ;
		i++ ;
		if(100==i)
			break ;
		//if(i%2)
			udt_send(&socket_node,buf, strlen(buf)) ;
		//else 
		//	ndudp_sendto(&socket_node,buf, strlen(buf)) ;
		
	//	if(i==20)
	//		nd_assert(0) ;
	}

	if(0==ret) {
		printf("close by peer data=%d\n", ret) ;
	}

	udt_close(&socket_node, 0);
	
	nd_net_destroy() ;
	nd_common_release() ;	
	
	printf("press any key to continue!\n") ;
	getch();
	return 0;
}