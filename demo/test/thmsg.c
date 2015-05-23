/* file thmsg.c
 * test thread server message
 *
 * 2009-5
 */


#include "nd_common/nd_common.h"
#include "nd_srvcore/nd_srvlib.h"


#define READ_FILE "client_test"
#define WRITE_FILE "client_test-out-msg"


nd_thsrvid_t _recv_id, _send_id ;
nd_handle __s_timer, __r_timer ;
ndtimer_t __g_timer;

static void _srv_clean(void) 
{
	ndprintf(_NDT("eixt %d\n"),nd_thread_self()) ;
}



static int sender(void *p) 
{
	int ret = 0 ;
	static int times= 0 ;
	size_t send_len = 0 ;
	ndchar_t buf[1024] ;

	int nReadNum = rand() %1000 + 10;
	FILE *pf = fopen(READ_FILE, "r+b") ;
	if(!pf) {
		printf("open file error!\n") ;
		return 0;
	}
	while(nReadNum=fread(buf,1,nReadNum,pf)){

		send_len += nReadNum ;
		ret = nd_thsrv_send(_recv_id,1,buf, nReadNum) ;
		//nd_assert(ret==0) ;
		//nd_sleep(10) ;
		nReadNum = rand() %1000 + 10;
		++times ;
		
	}
	
	nd_thsrv_send(_recv_id,2,0, 0) ;

	fclose(pf);

	return -1;

}


static int _msg_func(struct nd_thread_msg *msg)
{
	static int recved = 0 ;
	static FILE *wf ; 
	if(!wf) {
		wf = fopen(WRITE_FILE, "w+b") ;
		if(!wf)
			return -1 ;
	}

	if(msg->msg_id==1) 
		fwrite(msg->data,msg->data_len,1,wf) ;
	else if(2==msg->msg_id) {
		fclose(wf) ;
		return -1 ;
	}
	

	return 0 ;
}

int thmsg_test()
{
	struct nd_thsrv_createinfo srv_info = {0} ;
	
	srv_info.run_module =SUBSRV_MESSAGE;					//srv_entry run module (ref e_subsrv_runmod)
	srv_info.srv_entry =NULL;		//service entry
	srv_info.srv_param = NULL ;					//param of srv_entry 
	
	ndstrcpy(srv_info.srv_name, "receiver") ;

	_recv_id = nd_thsrv_create(&srv_info) ;
	if(-1==_recv_id) {
		nd_logfatal("create recv server error !") ;
		exit(1) ;
	}
	
	srv_info.run_module =SUBSRV_RUNMOD_LOOP;					//srv_entry run module (ref e_subsrv_runmod)
	srv_info.srv_entry =sender;		//service entry
	srv_info.srv_param = NULL ;					//param of srv_entry 
	
	ndstrcpy(srv_info.srv_name, "sender") ;
	_send_id = nd_thsrv_create(&srv_info);
	if(-1== _send_id) {
		nd_logfatal("create sender server error !") ;
		exit(1) ;
	}
	nd_thsrv_hook(nd_thsrv_gethandle(_send_id),_msg_func) ;
	nd_thsrv_hook(nd_thsrv_gethandle(_recv_id),_msg_func) ;

	nd_thsrv_wait(_send_id) ;
	nd_thsrv_wait(_recv_id) ;
	/*context = nd_thsrv_gethandle(_send_id) ;
	if(context->th_handle)
		nd_waitthread(context->th_handle) ;

	//fprintf(stderr, "wait for received thread!\n") ;
	//nd_waitthread(_recv_id) ;
	context = nd_thsrv_gethandle(_recv_id) ;
	if(context)
		nd_waitthread(context->th_handle) ;
	*/

	ndprintf("wait received success exit!\n") ;
	
	nd_thsrv_release_all() ;
	
	nd_timer_destroy(__s_timer,0) ;
	nd_timer_destroy(__r_timer,0) ;
//	nd_common_release() ;
	
//	getch();
	return 0;
}
