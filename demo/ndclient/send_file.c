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
#include <time.h>

char *__send_file, *__recv_file ;
#define _INPUT_FILE __send_file
#define _OUTPUT_FILE __recv_file
#define _READ_SIZE  NETDATA_SIZE

volatile int __exit = 0;

int do_client_msg(struct nd_tcp_node *conn, void *param)
{
		
	int ret;
	nd_assert(conn) ;
	nd_assert(check_connect_valid(conn) );
RE_READ:
	ret = nd_tcpnode_read(conn) ;
	if(ret < 0) {
		//error , close 
		//tcp_client_close(cli_map,1) ;
		//would be blocked
		//printf("last erron %s\n" , nd_last_error());
		return ret ;
	}
	else if(0==ret){
		//connect  end 
		nd_tcpnode_close(conn,0) ;
		return ret ;
	}
	else {
		if(-1==tcpnode_parse_recv_msg(conn, param) ){
			nd_tcpnode_close(conn,1) ;
			ret = 0 ;
		}
		if(TCPNODE_READ_AGAIN(conn)) {
			/*read buf is to small , after parse data , read again*/
			goto RE_READ;
		}
	}
	return ret ;
}


int msg_entry1(struct nd_tcp_node*node,struct ndnet_msg *msg,void *param)
{
	FILE **wf1 = (FILE**)param;
	FILE *wf = *wf1 ;
	nd_assert(wf) ;

	ndprintf(_NDT("\tcurrent recv=%d\n"),msg->msg_hdr.data_length + ND_MSGHRD_LEN);
	//ndprintf(_NDT("received message %d "),NDNET_MSGID(msg));
	if(NDNET_DATALEN(msg)>0 && 1==NDNET_MSGID(msg)) {
		nd_assert(wf);
		fwrite(NDNET_DATA(msg),NDNET_DATALEN(msg),1,wf) ;
		//NDNET_DATA(msg) + NDNET_DATALEN(msg) = 0 ; 
		//msg->buf[NDNET_DATALEN(msg)] = 0 ;

		//ndprintf(_NDT("%s\n"),NDNET_DATA(msg));
	}
	else if(2==NDNET_MSGID(msg)){
		fflush(wf);
		fclose(wf);
		__exit = 1;
	}
	else if(3==NDNET_MSGID(msg)) {
		FILE **open_fp = param ;
		*open_fp=fopen(msg->buf, "w+b") ;
		nd_assert(*open_fp);
	}
	return 0 ;
}

int send_file(struct nd_tcp_node *conn_node)
{
	static int _send_times= 0;
	int ret = 0;
	struct ndnet_msg msg_buf;

	int nReadNum = rand() %(_READ_SIZE-10) +1 ;
	char *paddr ;

	FILE *pf = fopen(_INPUT_FILE, "r+b") ;
	
	if(!pf) {
		printf("open file error!\n") ;
		return -1;
	}

	_init_ndmsg_buf(&msg_buf);

	NDNET_MSGID(&msg_buf) = 3 ;
	paddr = NDNET_DATA(&msg_buf);
	ndstrcpy(paddr,_OUTPUT_FILE) ;
	NDNET_DATALEN(&msg_buf) = ndstrlen(_OUTPUT_FILE)+1 ;
	msg_buf.msg_hdr.param = 0 ;
	nd_tcpnode_send(conn_node,&msg_buf,ESF_URGENCY) ;
	
	NDNET_MSGID(&msg_buf) = 1 ;
	
	paddr = NDNET_DATA(&msg_buf);
	while(nReadNum=fread(paddr,1,nReadNum,pf)) {
		
		NDNET_DATALEN(&msg_buf) = nReadNum ;

		//getch();
		//Sleep(10);
		++(msg_buf.msg_hdr.param) ;
		ret = nd_tcpnode_send(conn_node,&msg_buf,ESF_URGENCY) ;

		nd_assert(ret==(nReadNum +ND_MSGHRD_LEN ));
		nReadNum = rand() %(_READ_SIZE-10) +1 ;
		ndprintf("%d,send one message success  sendlen=%d\n", ++_send_times,ret);
	}
	NDNET_MSGID(&msg_buf) = 2 ;
	NDNET_DATALEN(&msg_buf) = 0 ;

	fclose(pf) ;
	++(msg_buf.msg_hdr.param) ;
	nd_tcpnode_send(conn_node,&msg_buf,ESF_URGENCY) ;

	return 0;
}

static  void* recv_msgt(void *arg)
{
	struct nd_tcp_node *conn_node = arg ;
	//FILE *wf = fopen(_OUTPUT_FILE, "w+b") ;
	FILE *wf ;
	conn_node->msg_entry = msg_entry1;
	while (0==__exit){
		do_client_msg(conn_node,&wf);
	}
//	fflush(wf);
//	fclose(wf);
	return (void *) 0;
} 


int main(int argc , char *argv[])
{	
	ndthread_t recv_id;
	ndth_handle hRecv;
	struct nd_tcp_node conn_node ;

	if(argc != 5) {
		ndprintf("usage: %s host port send_file recv_file\n",argv[0]) ;
		getch();
		exit(1) ;
	}
	_INPUT_FILE = argv[3] ;
	_OUTPUT_FILE = argv[4];
	srand( (unsigned)time( NULL ) );

	nd_common_init() ;
	
	nd_net_init() ;

	nd_tcpnode_init(&conn_node) ;
	if(-1==nd_tcpnode_connect(argv[1], atoi(argv[2]),&conn_node )){
		ndprintf(_NDT("connect error :%s!"), nd_last_error()) ;
		getch();
		exit(1);
	}
	
	nd_socket_nonblock(conn_node.fd,0);

	hRecv = nd_createthread(recv_msgt, &conn_node, &recv_id, 0) ;

	//send_file("localhost",7828);
	send_file(&conn_node) ;
	//test_send(argv[1],atoi(argv[2])) ;
	
	fprintf(stderr, "client test exit!\n") ;
	
	nd_waitthread(hRecv) ;
	nd_close_handle(hRecv) ;

	nd_tcpnode_close(&conn_node,0);

	nd_net_destroy() ;
	nd_common_release() ;
	
	getch();
	return 0;
}