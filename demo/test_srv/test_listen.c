/* file test_listen.c
 * test net server listen 
 *
 * 2007-10
 */


#pragma comment(lib,"nd_common.lib")
#pragma comment(lib,"nd_net.lib")
#pragma comment(lib,"nd_srvcore.lib")
#pragma comment(lib,"Ws2_32.lib")

#include "nd_net/nd_netlib.h"
#include "nd_srvcore/nd_srvlib.h"
#include "nd_srvcore/nd_listensrv.h"
#include "nd_common/nd_common.h"
#include "nd_srvcore/nd_session.h"
//#include "winsock2.h"

int listen_netmsg_entry(struct nd_client_map*cli_map,struct ndnet_msg *msg) 
{
	static NDUINT32 s1 = 0 ;
	static FILE *file1 = NULL;
//	if(s1!=msg->msg_hdr.param)
//		nd_assert(0);
	//windows平台只有在这里加了打印才能正确发送数据,难道是多线程切换需要使用printf
	//也行需要提供listen线程的优先级
	ndprintf(_NDT("%d [send %d]received len=%d\n"),s1,msg->msg_hdr.param,
		msg->msg_hdr.data_length + ND_MSGHRD_LEN);
//	ndprintf(_NDT("%d received message %d total recv=%d\n"),s1,NDNET_MSGID(msg), cli_map->connect_node.recv_len);

	switch(1) {
	case 1:
		nd_session_writebuf(cli_map,msg) ;
		break ;
	case 2:
		nd_session_send(cli_map,msg);
		break;
	default :
		nd_session_send_urgen(cli_map,msg) ;
		break ;
	}

	//nd_netmsg_send(cli_map,msg);
	//nd_netmsg_writebuf(cli_map,msg) ;
	//nd_netmsg_urgen(cli_map,msg) ;
	//
	
//	
//	ndprintf(_NDT("\ttotal send=%d\n") ,cli_map->connect_node.send_len);
//	if(rand()%2)
//		nd_netmsg_writebuf(cli_map,msg) ;
//	else
//		nd_netmsg_urgen(cli_map,msg) ;
	//nd_netmsg_writebuf(cli_map,msg) ;
//	nd_netmsg_send(cli_map,msg);
	//nd_netmsg_urgen(cli_map,msg) ;
//	ndprintf(_NDT("\ttotal send=%d\n") ,cli_map->connect_node.send_len);
/*
	if(1==NDNET_MSGID(msg)){
		nd_assert(file1) ;
		//fwrite()
		fwrite(NDNET_DATA(msg),NDNET_DATALEN(msg),1,file1) ;
	}
	else if(2==NDNET_MSGID(msg)) {
		//nd_netmsg_force_flush(cli_map);
		//file1 = fopen(msg->buf,"w+b") ;
		fflush(file1);
		fclose(file1) ;
		file1 = NULL;
	}
	else if(3==NDNET_MSGID(msg)) {
		//open file
		file1 = fopen(msg->buf,"w+b") ;
		nd_assert(file1);
	}
	*/
	s1++;

	return 0 ;

}


int accept_entry(struct nd_client_map*cli_map, SOCKADDR_IN *addr) 
{
	char  *pszTemp = nd_inet_ntoa( addr->sin_addr.s_addr );
	ndprintf(_NDT("Connect from %s:%d\n"), pszTemp, htons(addr->sin_port));
	return 0 ;
}

void close_entry(struct nd_client_map*cli_map) 
{
	char  *pszTemp = nd_inet_ntoa( cli_map->connect_node.remote_addr.sin_addr.s_addr );
	ndprintf(_NDT("%s:%d closed \n"), pszTemp, htons(cli_map->connect_node.remote_addr.sin_port));
}

int main()
{
	int io_mod = ND_LISTEN_OS_EXT;
	service_t listen_id ;
	struct nd_netlisten_entry entries = {0};
	nd_common_init() ;
	
	nd_net_init() ;

	
	/* initialize client map*/
	if(-1==nd_cm_allocator_create(_MAX_CLIENTS, _AFFIX_DATA_LENT,io_mod) ){
		nd_logerror("Create client map error") ;
		return -1 ;
	}

	entries.msg_entry = listen_netmsg_entry; 
	entries.accept_entry = accept_entry ;
	entries.close_entry = close_entry ;
	
	listen_id = create_listen_instance(7828, &entries,io_mod) ;

	if(-1==listen_id) {
		nd_logfatal("create recv server error !") ;
		exit(1) ;
	}
	
	nd_thsrv_wait(listen_id) ;
	fprintf(stderr, "wait received success exit!\n") ;
	
	nd_thsrv_release_all() ;
	
	nd_net_destroy() ;
	nd_common_release() ;
	
	getch();
	return 0;
}
