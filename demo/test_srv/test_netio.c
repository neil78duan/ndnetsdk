/* file test_netio.c
 * test  user interface of net server io 
 *
 * 2008-4
 */



#include "nd_net/nd_netlib.h"
#include "nd_srvcore/nd_srvlib.h"
#include "nd_srvcore/nd_listensrv.h"
#include "nd_common/nd_common.h"
#include "nd_srvcore/nd_session.h"
#include "nd_crypt/nd_crypt.h"

#pragma comment(lib,"nd_crypt.lib")
#pragma comment(lib,"nd_common.lib")
#pragma comment(lib,"nd_net.lib")
#pragma comment(lib,"nd_srvcore.lib")
#pragma comment(lib,"Ws2_32.lib")

#if defined(ND_UNICODE)
#pragma comment(lib,"nd_app_uni.lib")
#else
#pragma comment(lib,"nd_app.lib")
#endif

//#include "winsock2.h"
//int listen_netmsg_entry(struct nd_client_map*cli_map,struct ndnet_msg *msg) 

int listen_netmsg_entry(nd_cli_handle nethandle,NETMSG_T msgid, NDUINT32 param,NDUINT8 *data, int data_len) 
{
	static NDUINT32 s1 = 0 ;
	static FILE *file1 = NULL;
	nd_msgui_buf msg_buf ;
//	if(s1!=msg->msg_hdr.param)
//		nd_assert(0);
	//windows平台只有在这里加了打印才能正确发送数据,难道是多线程切换需要使用printf
	//也行需要提供listen线程的优先级
	ndprintf(_NDT("%d [send %d]received len=%d\n"),s1,param, data_len);
//	ndprintf(_NDT("%d received message %d total recv=%d\n"),s1,NDNET_MSGID(msg), cli_map->connect_node.recv_len);
	
//	if(msgid==2)
//		nd_assert(0) ;
	create_netui_msg(&msg_buf,msgid,param,data,data_len) ;
	switch(1) {
	case 1:
		//nd_session_writebuf(nethandle,&msg_buf) ;
		nd_session_post(nethandle,&msg_buf);
		break ;
	case 2:
		nd_session_send(nethandle,&msg_buf);
		break;
	default :
		nd_session_send_urgen(nethandle,&msg_buf) ;
		break ;
	}

	s1++;

	return 0 ;

}


int accept_entry(nd_cli_handle nethandle, SOCKADDR_IN *addr) 
{
	char  *pszTemp = nd_inet_ntoa( addr->sin_addr.s_addr );
	ndprintf(_NDT("Connect from %s:%d\n"), pszTemp, htons(addr->sin_port));
	return 0 ;
}

void close_entry(nd_cli_handle nethandle) 
{
	//char  *pszTemp = nd_inet_ntoa( cli_map->connect_node.remote_addr.sin_addr.s_addr );
	//ndprintf(_NDT("%s:%d closed \n"), pszTemp, htons(cli_map->connect_node.remote_addr.sin_port));
	ndprintf(_NDT("net close by remote\n"));
}

int main(int argc, char *argv[])
{
	int ch ;
	int io_mod = ND_LISTEN_UDT_STREAM;//ND_LISTEN_UDT_DATAGRAM;// ; //ND_LISTEN_COMMON;
	service_t listen_id ;
	struct nd_netlisten_entry entries = {0};
	
	nd_arg(argc, argv);
	nd_common_init() ;
	
	nd_net_init() ;
	ND_RUN_HERE() ;
	//create_rsa_key() ;
//printf("udt_head = %d, udt_packet =%d ", sizeof(udtpacket_len),sizeof(struct ndudt_pocket));
	/* initialize client map*/
	if(-1==nd_cm_allocator_create(10, _AFFIX_DATA_LENT,io_mod) ){
		nd_logerror("Create client map error") ;
		return -1 ;
	}
	ND_RUN_HERE() ;

	entries.msg_entry = listen_netmsg_entry; 
	//entries.msg_entry = pki_entry;
	entries.accept_entry = accept_entry ;
	entries.close_entry = close_entry ;
	
	listen_id = create_listen_instance(7828, &entries,io_mod) ;

	if(-1==listen_id) {
		nd_logfatal("create recv server error !") ;
		exit(1) ;
	}
	ND_RUN_HERE() ;
	while( 1 ){
		if(kbhit()) {
			ch = getch() ;
			if(ND_ESC==ch){
				printf_dbg("you are hit ESC, program eixt\n") ;
				break ;
			}
		}
		else {
			nd_sleep(1000) ;
		}
	}

	nd_host_eixt() ;

	nd_thsrv_release_all() ;
	
	nd_cm_allocator_destroy() ;
	nd_net_destroy() ;
	nd_common_release() ;
	
	printf_dbg("Press ANY KEY to continue\n") ;
	getch();
	return 0;
}
