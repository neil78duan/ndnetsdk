/* file test_app.c
 * test client connect application
 *
 * neil duan
 * 2008-6
 */

#include "nd_common/nd_common.h"

#if defined(ND_UNICODE)
#pragma comment(lib,"nd_cliapp_uni.lib")
#else
#pragma comment(lib,"nd_cliapp.lib")
#endif

#include "nd_cliapp/nd_cliapp.h"

int recv_msg_handler(nd_handle connect_handle , nd_usermsgbuf_t *msg)
{
	ndprintf("echo message %s\n", ND_USERMSG_DATA(msg)) ;
	return 0;
}
int __exit ;

int send_func(nd_handle connect_handle)
{
	int _send_times= 0;
	int ret = 0;
	nd_usermsgbuf_t msg_buf = ND_USERMSG_INITILIZER ; 
	//nd_msgui_buf msg_buf;
	//struct ndnet_msg msg_buf;
	char *paddr ;

	//_init_ndmsg_buf(&msg_buf);

	//MSGUI_MSGID(&msg_buf) = MAKE_MSGID(MAXID_SYS,SYM_ECHO) ;

	while ( __exit==0 && _send_times<10){
		paddr = ND_USERMSG_DATA(&msg_buf);
		ndsprintf(paddr, _NDT("hello world %d"), _send_times++) ;
		ND_USERMSG_MAXID(&msg_buf) = MAXID_SYS ;
		ND_USERMSG_MINID(&msg_buf) = SYM_ECHO ;
		ND_USERMSG_LEN(&msg_buf) = ndstrlen(paddr)+1 + ND_USERMSG_HDRLEN;
		ND_USERMSG_PARAM(&msg_buf) = 0 ;
		//nd_connector_send(connect_handle,&msg_buf,ESF_POST) ;	
		nd_connectmsg_send(connect_handle,&msg_buf) ;
		//memset(&msg_buf,0,sizeof(msg_buf)) ;
		ret = nd_connector_waitmsg(connect_handle,(nd_packetbuf_t*) &msg_buf,10000);
		
		if(ret > 0) {
			nd_translate_message(connect_handle,(nd_packhdr_t*) &msg_buf) ;
			//run_cliemsg(connect_handle, &msg_buf) ;
			//msg_handle(connect_handle, msg_buf.msgid,msg_buf.param,
			//		msg_buf._data, msg_buf.data_len);
		}
		else if(-1==ret) {
			ndprintf(_NDT("closed by remote ret = 0\n")) ;
			break ;
		}
		else {
			ndprintf(_NDT("wait time out ret = %d\npress any key to continue\n"), ret) ;
//			getch() ;
			
		}
		
	}
	end_session(connect_handle) ;
//	__exit = 1 ;
	
	return 0;
}

int main(int argc , char *argv[])
{	
	if(argc != 5) {
		ndprintf("usage: %s host port send_file recv_file\n",argv[0]) ;
		getch();
		exit(1) ;
	}

	if(nd_cliapp_init(argc, argv)){
		ndprintf(_NDT("connect error :%s!"), nd_last_error()) ;
		getch();
		exit(1);
	}
	
	nd_msgentry_install(get_connect_handle(),recv_msg_handler,MAXID_SYS,SYM_ECHO,0) ;
	if(-1==start_encrypt(get_connect_handle()) ) {
			
		fprintf(stderr, "press any key to continue!\n") ;
		getch();
		exit(1) ;
	}
	else {
		ndprintf(_NDT("start encrypt success!\n press ANY KEY to continue\n")) ;
		getch() ;
	}

	//send_file("localhost",7828);
	//send_file(connect_handle) ;
	send_func(get_connect_handle());
	//test_send(argv[1],atoi(argv[2])) ;
	
	
//	nd_waitthread(hRecv) ;
//	nd_close_handle(hRecv) ;

	while(0==__exit ) {
		int ret = nd_connector_update(get_connect_handle(),1000) ;
		if(0==ret )
			break ;
	}
	
	nd_cliapp_end(0);
	
	fprintf(stderr, "client test exit!\n") ;
	getch();
	return 0;
}
