/* file test_listen.c
 * test net server listen 
 *
 * 2007-10
 */


#pragma comment(lib,"nd_common.lib")
#pragma comment(lib,"nd_net.lib")
#pragma comment(lib,"nd_srvcore.lib")

#include "nd_net/nd_netlib.h"
#include "nd_common/nd_common.h"
#include "nd_srvcore/nd_srvlib.h"
#include "nd_srvcore/nd_listensrv.h"
#include "nd_net/nd_udt.h"

nd_udtsrv *root ;

int datagram_entry(nd_udt_node *new_socket, void *data, size_t len,void *param) ;

int accept_entry(nd_udt_node *new_socket) 
{
	u_32 *val =(u_32 *) datagram_entry ;
	fprintf(stderr,"accept new connext \n") ;
	udt_ioctl(new_socket,UDT_IOCTRL_SET_DATAGRAM_ENTRY, val) ;
	return 0 ;
}


int release_node_entry(nd_udt_node *new_socket) 
{
	fprintf(stderr,"release a closed connext \n") ;
	
	return 0 ;
}


int datagram_entry(nd_udt_node *new_socket, void *data, size_t len,void *param) 
{
	if(0==len) {
		return 0 ;
	}
	else {
		char *p = data ;
		p[len]=0;
		fprintf(stderr,"recv DATAGRAM len =%d\n [%s]\n" , len, p) ;
		ndudp_sendto(new_socket,data,len);
		//udt_close(new_socket,0) ;
	}
	return 0;
}

int data_recv_entry(nd_udt_node *new_socket, void *data, size_t len) 
{
	if(0==len) {
		fprintf(stderr,"close by remote\n") ;
	}
	else {
		char *p = data ;
		p[len]=0;
		fprintf(stderr,"recv data len =%d\n [%s]\n" , len, p) ;
		udt_send(new_socket,data,len);
		//udt_close(new_socket,0) ;
	}
	return 0;
}

int run_net()
{
	root = udt_open_srv(7828,10,10);

	if(!root)
		return -1 ;

	if(-1==nd_cm_allocator_create(10, 0,ND_LISTEN_UDT_STREAM) ) {
		fprintf(stderr, "create cm error!\n") ;
		exit(1);
	}

	udt_install_connect_manager(root,nd_cm_node_alloc ,nd_cm_node_free) ;

	if(-1==udt_bindio_handler(root, accept_entry, data_recv_entry,release_node_entry)){
		udt_close_srv(root,1) ;
		return -1 ;
	}


	for(;;){
		doneIO(root,100) ;
		if(kbhit()) {
			int ch = getch() ;
			if(ND_ESC==ch){
				printf_dbg("you are hit ESC, program eixt\n") ;
				break ;
			}
		}
	}
	udt_close_srv(root,1) ;
	
	return 0;
}
int main()
{	
	nd_common_init() ;
	
	nd_net_init() ;


	run_net() ;
	
	nd_net_destroy() ;
	nd_common_release() ;
	
	getch();
	return 0;
}
