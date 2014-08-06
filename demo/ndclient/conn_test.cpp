/* file : conn_test.cpp
 * test connection
 *
 * all right reserved by neil duan 
 * 2009-5-8 23:47
 */
 

#if defined(ND_DEBUG)
#pragma comment(lib,"nd_cliapp_dbg.lib")
#else
#pragma comment(lib,"nd_cliapp.lib")
#endif

#include "nd_cliapp/nd_cliapp.h"

int volatile __exit;

int __netmod = ND_TCP_STREAM ;

#define MAX_CONNECTIONS 1

NDConnector *p_connbuf ;
//nd_handle __conn_buf[MAX_CONNECTIONS] ;
ndatomic_t __conn_num ;

int msg_entry(nd_handle connect_handle , nd_usermsgbuf_t *msg, nd_handle h_listen)
{
	ndtime_t now = nd_time() ;
	ndprintf(_NDT("received data [%s] translate time=%d \n"),  ND_USERMSG_DATA(msg),now-ND_USERMSG_PARAM(msg)) ;
	return 0 ;
}


int msg_bc_entry(nd_handle connect_handle , nd_usermsgbuf_t *msg, nd_handle h_listen)
{
	ndtime_t now = nd_time() ;
	NDIStreamMsg MsgRecv(msg) ;
	char buf[4096] ;

	//MsgRecv >> buf ;
	MsgRecv.Read(buf, sizeof(buf)) ;
	
	ndprintf(_NDT("received data [%s] translate time=%d \n"), buf, now - MsgRecv.MsgParam()) ;
	return 0 ;
}



void *recv_func(void *param)
{
	int i;
	while(0==__exit){
		int num = nd_atomic_read(&__conn_num) ;
		for (i=0; i<MAX_CONNECTIONS; i++){
			if (__exit)
				goto _quit;

			if( p_connbuf[i].CheckValid()){
				p_connbuf[i].Update(10) ;
			}
		}
		//nd_sleep(10) ;
	}
_quit:

	return (void*)0;
}

void *send_func(void *param)
{
	int i;
	while(0==__exit){
		int num = nd_atomic_read(&__conn_num) ;
		for (i=0; i<MAX_CONNECTIONS; i++){
			if (__exit)
				goto _quit;
			
			NDOStreamMsg OsMsg(MAXID_SYS, SYM_BROADCAST) ;
			char buf[128] ;
			OsMsg.MsgParam() = nd_time() ;
			ndsprintf(buf, _NDT("broad cast hello all %d"), i) ;
			OsMsg.Write(buf) ;

			if( p_connbuf[i].CheckValid()){
				p_connbuf[i].SendMsg(&OsMsg) ;
				p_connbuf[i].Update(50) ;
			}
			if(--num<=0){
				break ;
			}
		}
		//nd_sleep(10) ;
	}
_quit:

	return (void*)0;
}


//等待服务的结束
int wait_exit()
{
	int ch;
	while( 0==__exit ){
		if(kbhit()) {
			ch = getch() ;
			if(ND_ESC==ch){
				printf_dbg("you are hit ESC, program eixt\n") ;
				__exit = 1 ;
				break ;
			}
		}
		else {
			nd_sleep(100) ;
		}
	}
	return 0;
}

int init_connector()
{
	int i ;
	struct connect_config *pconf = get_config_info() ; ;
	p_connbuf = new NDConnector[MAX_CONNECTIONS] ;
	
	for (i=0 ; i<MAX_CONNECTIONS; i++) {
		if(-1==p_connbuf[i].Open(pconf->host, pconf->port, pconf->protocol_name) ) {
			nd_assert(0) ;
			break ;
		}
		p_connbuf[i].InstallMsgFunc(msg_bc_entry,MAXID_SYS,SYM_TEST) ;
		p_connbuf[i].InstallMsgFunc(msg_bc_entry,MAXID_SYS,SYM_ECHO) ;
		p_connbuf[i].InstallMsgFunc(msg_bc_entry,MAXID_SYS,SYM_BROADCAST) ;
		++__conn_num;
	}

	return (__conn_num == MAX_CONNECTIONS) ? 0:-1 ;

}

void destroy_conn()
{
	int i ;
	for (i=0; i<MAX_CONNECTIONS; i++)
	{
		if( p_connbuf[i].CheckValid() ){
			p_connbuf[i].Destroy() ;
		}
	}
	delete [] p_connbuf;
}
int main(int argc , char *argv[])
{	
	ndthread_t thid ;
	ndth_handle h  ;
	
	if(nd_cliapp_init(argc, argv)){
		ndprintf(_NDT("connect error :%s!"), nd_last_error()) ;
		getch();
		exit(1);
	}
	
	init_connector() ;

	h = nd_createthread(send_func, NULL, &thid, 0) ;
	
	wait_exit() ;

	if(h)
		nd_waitthread(h) ;
	
	destroy_conn() ;

	nd_cliapp_end(0);
	
	fprintf(stderr, "client test exit!\n") ;
	getch();
	return 0;
}
