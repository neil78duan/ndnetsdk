/* file max_connect.c
 * test max connection
 *
 * neil duan
 * 2008-8-14 
 */

#if defined(ND_DEBUG)
#pragma comment(lib,"nd_cliapp_dbg.lib")
#pragma comment(lib,"vs10_static_cli_dbg.lib")
#else
#pragma comment(lib,"nd_cliapp.lib")
#pragma comment(lib,"vs10_static_cli.lib")
#endif

#include "nd_cliapp/nd_cliapp.h"

int volatile __exit;

char *__host;
int __port ;

#define MAX_CONNECTIONS 100

nd_mutex __conn_mutex[MAX_CONNECTIONS] ;
nd_handle __conn_buf[MAX_CONNECTIONS] ;
ndatomic_t __conn_num ;

int update_connect(nd_handle connect_handle)
{
	//return nd_connector_update(connect_handle,10) ;

	if(-1==nd_connector_update(connect_handle,0)) {
		nd_connector_close(connect_handle,0) ;
		return -1 ;
	}
	return 0 ;
	
}

int msg_entry(nd_handle connect_handle , nd_usermsgbuf_t *msg, nd_handle h_listen)
{
	ndtime_t now = nd_time() ;
	ndprintf(_NDT("received data [%s] translate  \n"),  ND_USERMSG_DATA(msg)) ;
	return 0 ;
}


int send_test(nd_handle connect_handle, int index,int param)
{
	int ret = 0;
	nd_usermsgbuf_t msg_buf = ND_USERMSG_INITILIZER;
	char *paddr ;

	ND_USERMSG_MAXID(&msg_buf) = MAXID_SYS ;
	ND_USERMSG_MINID(&msg_buf) = SYM_ECHO ;
	paddr = ND_USERMSG_DATA(&msg_buf) ;
	
	ndsprintf(paddr, _NDT("hello world %d"), index) ;
	ND_USERMSG_LEN(&msg_buf) = ndstrlen(paddr)+1 + ND_USERMSG_HDRLEN;
	//ND_USERMSG_PARAM(&msg_buf) = nd_time() ;
	return nd_connectmsg_send_urgen(connect_handle,&msg_buf) ;
	
	
}


int send_broadcast(nd_handle connect_handle, int index)
{
	int ret = 0;
	nd_usermsgbuf_t msg_buf = ND_USERMSG_INITILIZER;
	char *paddr ;

	ND_USERMSG_MAXID(&msg_buf) = MAXID_SYS ;
	ND_USERMSG_MINID(&msg_buf) = SYM_BROADCAST ;
	//ND_USERMSG_MINID(&msg_buf) = SYM_ECHO ;
	paddr = ND_USERMSG_DATA(&msg_buf) ;
	
	ndsprintf(paddr, _NDT("broad cast hello all %d"), index) ;
	ND_USERMSG_LEN(&msg_buf) = ndstrlen(paddr)+1 + ND_USERMSG_HDRLEN;
	//ND_USERMSG_PARAM(&msg_buf) = nd_time() ;
	return nd_connectmsg_send_urgen(connect_handle,&msg_buf) ;
	
	
}

int send_logout(nd_handle connect_handle, int index)
{
	int ret = 0;
	nd_usermsgbuf_t msg_buf = ND_USERMSG_INITILIZER;

	ND_USERMSG_MAXID(&msg_buf) = MAXID_START_SESSION ;
	ND_USERMSG_MINID(&msg_buf) = SSM_LOGOUT ;
	
	return nd_connectmsg_send_urgen(connect_handle,&msg_buf) ;
	
	
}
/*
int _msg_entry(nd_handle connect_handle, NETMSG_T mid,NDUINT32 param, NDUINT8 *data, int data_len)
{
	ndprintf(_NDT("id=[%d] received data %s \n"), param, data) ;
	return 0 ;
}
*/
int msg_wait(nd_handle connect_handle)
{

	int ret;
	//nd_msgui_buf msg_recv;
	if(connect_handle->type==NDHANDLE_UDPNODE) {
		nd_usermsgbuf_t msg_recv;

		ret = nd_connector_waitmsg(connect_handle, (nd_packetbuf_t *)&msg_recv,10);
		if(ret > 0) {			
			//msg_entry(connect_handle, &msg_recv) ;
			nd_translate_message(connect_handle, (nd_packhdr_t*)&msg_recv,0) ;
		}		

		else if(-1==ret) {
			ndprintf(_NDT("closed by remote ret = 0\n")) ;
			return -1 ;
		}
		else {
			ndprintf(_NDT("wait time out ret = %d\npress any key to continue\n"), ret) ;
			return -1;		
		}
		
		return 0;
	}
	else
		return nd_connector_update(connect_handle,10) ;
}

int open_net() 
{
	int i, __times = 0;
	for (i=0; i<MAX_CONNECTIONS; i++){
		__conn_buf[i] =  create_connector() ;
		if(!__conn_buf[i])
			return -1;
		if(-1==nd_mutex_init(&__conn_mutex[i]) ) {
			return -1 ;
		}	

		nd_msgentry_install(__conn_buf[i],msg_entry,MAXID_SYS,SYM_ECHO,0) ;
		nd_msgentry_install(__conn_buf[i],msg_entry,MAXID_SYS,SYM_BROADCAST,0) ;
		nd_atomic_inc(&__conn_num);

	}
	return 0 ;
}

int close_net()
{
	int i;
	for (i=0; i<MAX_CONNECTIONS ; i++){
		if(__conn_buf[i]){
			end_session(__conn_buf[i]);
			nd_connector_update(__conn_buf[i],10) ;
			//msg_wait(__conn_buf[i]) ;
		}
	}

	for (i=0; i<MAX_CONNECTIONS ; i++){
		destroy_connect(__conn_buf[i]) ;
		nd_mutex_destroy(&__conn_mutex[i]) ;
	}
	return 0 ;
}
void *wait_func(void *param)
{

	int i ;
	int sleep_tm ;
	while(0==__exit){
		int num = nd_atomic_read(&__conn_num) ;
		int ret = 0 ;
		sleep_tm = 10;
		for (i=0; i<MAX_CONNECTIONS; i++){
			if (__exit)
				return (void*)0;

			if(0==nd_mutex_trylock(&__conn_mutex[i])) {
				if (__conn_buf[i]) {
					ret =update_connect(__conn_buf[i]) ;
					if(ret == -1) {
						destroy_connect(__conn_buf[i]) ;
						__conn_buf[i] = NULL;
					}
				}
				nd_mutex_unlock(&__conn_mutex[i]) ;
			}
		}
		//if(sleep_tm)
		nd_sleep(10) ;
	}
	return (void*)0;	
}

void *send_func(void *param)
{
	int i, __times = 0;
	for (i=0; i<MAX_CONNECTIONS; i++){
		nd_mutex_lock(&__conn_mutex[i]) ;
		start_session(__conn_buf[i]) ;
		nd_mutex_unlock(&__conn_mutex[i]) ;
	}

	while(0==__exit){
		int num = nd_atomic_read(&__conn_num) ;
		int ret = 0 ;
		for (i=0; i<MAX_CONNECTIONS; i++){
			if (__exit)
				return (void*)0;

			if(0==nd_mutex_trylock(&__conn_mutex[i])) {
				if (__conn_buf[i]){
					send_broadcast(__conn_buf[i], i) ;
					//send_test(__conn_buf[i], i,0);
				}
				nd_mutex_unlock(&__conn_mutex[i]) ;
			}
		}
		nd_sleep(100) ;

	}

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

int main(int argc , char *argv[])
{	
	ndthread_t thid, waitid ;
	ndth_handle h, waith  ;
	
	if(nd_cliapp_init(argc, argv)){
		ndprintf(_NDT("connect error :%s!"), nd_last_error()) ;
		getch();
		exit(1);
	}

	if(-1==open_net()  ) {
		ndprintf(_NDT("open net error \n press any key to exit")) ;
		getch();
		exit(1);
	}

	h = nd_createthread(send_func, NULL, &thid, 0) ;
	waith = nd_createthread(wait_func, NULL, &waitid, 0) ;
	
	nd_assert(h) ;
	nd_assert(waith);

	wait_exit() ;

	//if(h)
		nd_waitthread(h) ;

	//if(waith)
		nd_waitthread(waith) ;

	close_net() ;

	nd_cliapp_end(0);
	
	fprintf(stderr, "client test exit!\n") ;
	getch();
	return 0;
}
