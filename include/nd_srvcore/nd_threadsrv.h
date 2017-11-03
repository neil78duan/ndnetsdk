/* file nd_threadsrv.h
 * thread server entry
 * version 1.0 all right reserved by neil
 * 2007-10
 */

#ifndef _NDSRVENGTY_H_
#define _NDSRVENGTY_H_

//#include "nd_common/nd_common.h"

/*
 * 对线程的封装,使它看起来更像一个服务器
 * 服务器功能模块入口:
 * 服务器所执行的功能模块在这里被称为一个子服务.以下称为"线程服务器"
 * 每个服务启动一个线程,有自己的线程上下文,和消息队列,
 * 其他服务或者线程可以通过服务名字或者服务ID找到他们,并且发送消息.
 */
#define MAX_LISTENNUM	2	
#define ND_SRV_NAME 32 

typedef ndthread_t nd_thsrvid_t ;
typedef int (*nd_threadsrv_entry)(void *param) ;		//service entry
typedef void (*nd_threadsrv_clean)(void) ;				//service terminal clean up fucnton 

#pragma  warning(push)
#pragma warning (disable : 4200 )
/* thread message data*/
typedef struct nd_thread_msg{
	NDUINT32			msg_id ;
	NDUINT32			data_len ;			//data length处理过
	int					wait_handle;		//是否等待对方处理完
	nd_thsrvid_t		from_id;	//,recv_id;	//message sender and receiver
	nd_handle			recv_handle;		//接收者handle
	void				*th_userdata ;		//对应 nd_thsrv_createinfo::data
	struct list_head	list ;
	char				data[0] ;			//data address
}nd_thsrv_msg;
#pragma  warning(pop)

typedef int (*nd_thsrvmsg_func)(nd_thsrv_msg *msg) ;	//message handle function

/* 服务函数类型
 * 如果使用SUBSRV_RUNMOD_STARTUP模式需要在自己的程序中处理消息
 * 用法: 
 *  
		service_entry() {
			//do some initilize
			while(!nd_get_exit) {
				//your code 
				...
				
				if(nd_thsrv_msghandler(NULL) ) 
					return ;
				...
			}
			return ;
		}
 */
enum e_thsrv_runmod{
	SUBSRV_RUNMOD_LOOP = 0,		//for(;;) srv_entry() ;
	SUBSRV_RUNMOD_STARTUP,		//return srv_entry() ; 
	SUBSRV_MESSAGE				//创建一个单独的消息处理线程(主要是可以实现一个线程池,只处理消息的线程)
};

struct nd_thsrv_createinfo
{
	int run_module ;					//srv_entry run module (ref e_subsrv_runmod)
	nd_threadsrv_entry srv_entry ;		//service entry
	void *srv_param ;					//param of srv_entry 
	void *data ;						//user data
	char srv_name[ND_SRV_NAME] ;	//service name
};


/*
 * create_service() create a server .
 * @create_info : input service info
 */
ND_SRV_API nd_thsrvid_t nd_thsrv_createex(struct nd_thsrv_createinfo* create_info,int priority, int suspend ) ;
static __INLINE__ nd_thsrvid_t nd_thsrv_create(struct nd_thsrv_createinfo* create_info)
{
	return nd_thsrv_createex(create_info,NDT_PRIORITY_NORMAL, 0) ;
}

/* get service context 
 * if srvid == 0 ,get current service context
 */
ND_SRV_API nd_handle nd_thsrv_gethandle(nd_thsrvid_t srvid );

ND_SRV_API nd_thsrvid_t nd_thsrv_getid(nd_handle);
/*
 * stop service and destroy service context.
 * if force zero wait for service thread exit and return exit code
 * else return 0
 */
ND_SRV_API  int nd_thsrv_destroy(nd_thsrvid_t srvid,int force);

/*release all services in current host*/
ND_SRV_API void nd_thsrv_release_all() ;

/* 让所以线程的宿主退出,所以线程也退出*/
ND_SRV_API void nd_host_eixt() ;
ND_SRV_API void nd_server_host_begin() ;
ND_SRV_API int nd_host_check_exit() ;
ND_SRV_API int nd_host_set_error(int err);
ND_SRV_API int nd_host_get_error();

ND_SRV_API int nd_thsrv_check_exit(nd_thsrvid_t srv_id) ;
ND_SRV_API int nd_thsrv_isexit(nd_handle) ;

ND_SRV_API int nd_thsrv_end(nd_thsrvid_t  srv_id) ;		//terminal a service

ND_SRV_API int nd_thsrv_suspend(nd_thsrvid_t  srv_id) ;		//suspend a service 
ND_SRV_API int nd_thsrv_resume(nd_thsrvid_t  srv_id) ;		//resume a service 
ND_SRV_API int nd_thsrv_suspendall() ;		//suspend all service ,except self
ND_SRV_API int nd_thsrv_resumeall() ;		//suspend all service 

ND_SRV_API int nd_thsrv_wait(nd_thsrvid_t  srv_id) ;		//wait a service exit
//ND_SRV_API struct nd_thread_msg *nd_thsrv_msgnode_create(int datalen) ;

/* 发送消息 推荐使用nd_send_msg() 
 * 如果使用nd_send_msgex() 用法:
	msg = create_thmsg_node(datalen) ;
	//set message id, receiver id, data, and datalen to nd_thread_msg.
	nd_thsrv_sendex(msg);
 */
ND_SRV_API int nd_thsrv_send(nd_thsrvid_t recvid,NDUINT32 msgid,void *data, NDUINT32 data_len) ;
ND_SRV_API int nd_thsrv_sendex(nd_thsrvid_t srvid,NDUINT32 msgid,void *data, NDUINT32 data_len,int iswait)  ;

//ND_SRV_API int nd_thsrv_sendex(struct nd_thread_msg *msg);

/* 处理用户消息
 * 如果是用SUBSRV_RUNMOD_STARTUP 模式来创建服务,则需要在服务程序中自行处理消息
 * 一般用法是在服务循环中调用nd_message_handler() ,参数中指定context是为了提高效率
 * return -1 service(thread) exit 
 * else return numbers of message had been handled
 */
ND_SRV_API int nd_thsrv_msghandler( nd_handle  thsrv_handle) ;	

//install message handle of thread
ND_SRV_API int nd_thsrv_install_msg( nd_handle  thhandle,NDUINT32 msgid, nd_thsrvmsg_func func ) ;	
/* get thread server memory pool 
 * 
 */
ND_SRV_API nd_handle nd_thsrv_local_mempool(nd_handle  thsrv_handle) ;


ND_SRV_API void* nd_thsrv_getdata(nd_handle handle);
ND_SRV_API void nd_thsrv_setdata(nd_handle handle, void *data) ;
/* create timer in service which specified by srv_id, if srv_id == 0 create in current service
 * @entry input : entry of timer 
 * param input : parameter of entry
 * return value timer id 
 * reference ndtimer
 */


ND_SRV_API ndtimer_t nd_thsrv_timer(nd_thsrvid_t srv_id,nd_timer_entry func,void *param,ndtime_t interval, int run_type ) ;
ND_SRV_API void nd_thsrv_del_timer(nd_thsrvid_t srv_id, ndtimer_t timer_id ) ;

//拦截线程消息处理函数
ND_SRV_API nd_thsrvmsg_func nd_thsrv_hook(nd_handle handle,nd_thsrvmsg_func newfunc) ;
ND_SRV_API nd_threadsrv_clean  nd_thsrv_set_clear(nd_handle handle,nd_threadsrv_clean clear_func);
/*线程把自己挂起*/
int nd_thsrv_suspend_self(nd_handle handle) ;

#endif
