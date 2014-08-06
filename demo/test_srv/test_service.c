/* file test_service.c
 * test register/unregister server 
 *
 * 2007-10
 */


#pragma comment(lib,"nd_common.lib")
#pragma comment(lib,"nd_srvcore.lib")

#include "nd_common/nd_common.h"
#include "nd_srvcore/nd_srvlib.h"

service_t _recv_id, _send_id ;
ndtimer_t __g_timer ;
void _srv_clean(void) 
{
	ndprintf(_NDT("eixt %d\n"),nd_thread_self()) ;
}


void timer_func2(ndtimer_t timer_id, void *param)
{
	printf("timer function timer_id=%d\n exit host", timer_id) ;
	nd_host_eixt() ;
}

void timer_func(ndtimer_t timer_id, void *param)
{
	static int s1 = 0 ;
	printf("timer start %d timer_id=%d\n", s1++,timer_id) ;
	if(s1==10){
		nd_thsrv_timer_del(__g_timer, 0) ;
		printf("timer %d is deleted %d\n", timer_id) ;
		__g_timer = nd_thsrv_timer(timer_func2, NULL,1000,0) ;
		
	}
}

int sender(void *p) 
{
	NDUINT16 msgid = 1 ;
	static int times= 0 ;
	ndchar_t buf[128] ;

	if(0==times){
		__g_timer = nd_thsrv_timer(timer_func, NULL,1000,0) ;
		if (!__g_timer)
		{
			printf("create timer error!\n") ;
		}

	}
	times++ ;
	
	//if(times>2){
	//	msgid = 2 ;
		//nd_thsrv_send(_recv_id,2,buf, ndstrlen(buf)+ sizeof(ndchar_t));
	//	nd_srv_exit() ;
	//	nd_threadexit(0) ;
	//}

	ndsprintf(buf, "send times=%d" ,times) ;

	nd_thsrv_send(_recv_id,msgid,buf, (NDUINT16)(ndstrlen(buf)+ sizeof(ndchar_t))) ;

//	ndprintf("send run %d...\n",times) ;
	
	nd_threadsched() ;
	nd_sleep(100) ;
	return 0 ;

}
int recver(void *p)
{
/*	__g_timer = nd_thsrv_timer(timer_func, NULL,1000,0) ;
	if (!__g_timer)
	{
		printf("create timer error!\n") ;
	}
	*/
	while(0==nd_host_check_exit() ){
		//ndprintf("recver run here ...\n");
		nd_thsrv_msghandler(0) ;
		nd_threadsched() ;
		nd_sleep(100);
	}
	ndprintf("%d exit\n", nd_thread_self());
	return 0 ;

}


int _msg_func(struct nd_thread_msg *msg)
{
	static int recved = 0 ;
	//ndfprintf(stderr, _NDT("%d received message from %d\n datalen=%d data=%s\n"), 
	//	msg->recv_id,msg->from_id,msg->data_len, msg->data) ;
	//fprintf(stderr, "received message!\n");
	//fflush(stderr);
	recved++ ;
	//if(100==recved) {
	//	nd_host_eixt() ;
	//}
	return 0 ;
}

int main()
{
	nd_thsrv_context_t *context;
	struct nd_thsrv_createinfo srv_info = {0} ;
	
	nd_common_init() ;
	srv_info.run_module =SUBSRV_RUNMOD_STARTUP;					//srv_entry run module (ref e_subsrv_runmod)
	srv_info.srv_entry =recver;		//service entry
	srv_info.srv_param = NULL ;					//param of srv_entry 
	srv_info.msg_entry = _msg_func;				//handle received message from other thread!
	srv_info.cleanup_entry= _srv_clean ;		//clean up when server is terminal
	
	ndstrcpy(srv_info.srv_name, "receiver") ;

	_recv_id = nd_thsrv_create(&srv_info) ;
	if(-1==_recv_id) {
		nd_logfatal("create recv server error !") ;
		exit(1) ;
	}
	
	srv_info.run_module =SUBSRV_RUNMOD_LOOP;					//srv_entry run module (ref e_subsrv_runmod)
	srv_info.srv_entry =sender;		//service entry
	srv_info.srv_param = NULL ;					//param of srv_entry 
	srv_info.msg_entry = _msg_func;				//handle received message from other thread!
	srv_info.cleanup_entry= NULL ;		//clean up when server is terminal
	
	ndstrcpy(srv_info.srv_name, "sender") ;
	_send_id = nd_thsrv_create(&srv_info);
	if(-1== _send_id) {
		nd_logfatal("create sender server error !") ;
		exit(1) ;
	}

	context = nd_thsrv_gethandle(_send_id) ;
	if(context->th_handle)
		nd_waitthread(context->th_handle) ;

	//fprintf(stderr, "wait for received thread!\n") ;
	//nd_waitthread(_recv_id) ;
	context = nd_thsrv_gethandle(_recv_id) ;
	if(context)
		nd_waitthread(context->th_handle) ;

	fprintf(stderr, "wait received success exit!\n") ;
	
	nd_thsrv_release_all() ;
	
	nd_common_release() ;
	
	getch();
	return 0;
}
