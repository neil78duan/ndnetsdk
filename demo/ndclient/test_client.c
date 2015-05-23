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

int msg_entry(struct nd_tcp_node*node,struct ndnet_msg *msg,void *param)
{
	ndprintf(_NDT("received message %d "),NDNET_MSGID(msg));
	if(NDNET_DATALEN(msg)>0) {
		//NDNET_DATA(msg) + NDNET_DATALEN(msg) = 0 ; 
		msg->buf[NDNET_DATALEN(msg)] = 0 ;
		ndprintf(_NDT("%s\n"),NDNET_DATA(msg));
	}
	return 0 ;
}

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
int test_send(char *host, int port)
{

	int i ;
	struct nd_tcp_node conn_node ;
	struct ndnet_msg msg_buf;

	
	nd_tcpnode_init(&conn_node) ;

	conn_node.msg_entry = msg_entry;
	//i=atoi(argv[2]) ;
	if(-1==nd_tcpnode_connect(host, port,&conn_node )){
	//if(-1==nd_tcpnode_connect("localhost", 7828,&conn_node )){
		ndprintf(_NDT("connect error :%s!"), nd_last_error()) ;
		getch();
		exit(1);
	}

	nd_socket_nonblock(conn_node.fd,0);
	_init_ndmsg_buf(&msg_buf);
	NDNET_MSGID(&msg_buf) = 1 ;
	for (i=0 ; i<100 ; i++){
		ndsprintf(NDNET_DATA(&msg_buf), _NDT("send %d time =%d"), i, nd_time()) ;
		NDNET_DATALEN(&msg_buf) =ndstrlen(NDNET_DATA(&msg_buf)) ;
		nd_tcpnode_send(&conn_node,&msg_buf,ESF_URGENCY) ;
		do_client_msg(&conn_node,NULL);
	}
	return 0;
}


int main(int argc , char *argv[])
{	
	ndthread_t recv_id;
	ndth_handle hRecv;
	struct nd_tcp_node conn_node ;

	if(argc != 3) {
		ndprintf("usage: %s host port\n",argv[0]) ;
		getch();
		exit(1) ;
	}

	

	nd_common_init() ;
	
	nd_net_init() ;


	//send_file("localhost",7828);
	//send_file(&conn_node) ;
	test_send(argv[1],atoi(argv[2])) ;
	
	fprintf(stderr, "client test exit!\n") ;
	
	nd_net_destroy() ;
	nd_common_release() ;
	
	getch();
	return 0;
}