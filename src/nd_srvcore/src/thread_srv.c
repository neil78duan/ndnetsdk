/* file srv_entry.h
 * server entry
 * version 1.0 all right reserved by neil
 * 2007-10
 */
//#define TEST_MYMM 1		//定义这个时,用系统的malloc代替我mempool

#ifdef ND_UNIX
#define USER_P_V_LOCKMSG		1		//使用条件变量来互斥消息
#endif 

/*
 * 锟斤拷锟斤拷锟斤拷锟竭程凤拷锟斤拷,锟斤拷装锟斤拷锟竭程猴拷锟竭筹拷之锟斤拷锟酵ㄑ?
 * 锟斤拷每锟斤拷锟竭程筹拷为一锟斤拷锟竭程凤拷锟斤拷锟斤拷
 */
// #include "nd_common/nd_common.h"
// #include "nd_srvcore/nd_srvlib.h"
// #include "nd_common/nd_alloc.h"
#include "nd_srvcore/nd_srvlib.h"

//message entry 
struct thmsg_entry
{
	NDUINT32 msgid ;
	nd_thsrvmsg_func func ;
	struct  list_head list ;
};

/* thread contex*/
typedef struct nd_threadsrv_context{
	ND_OBJ_BASE ;
	nd_thsrvid_t  thid ;				//server id
	ndth_handle th_handle ;				//thread handle
	nd_handle	allocator ;				//memory allocator 
	ndatomic_t __exit ;					//if nonzero eixt 
	int run_module ;					//srv_entry run module (ref e_subsrv_runmod)
	nd_threadsrv_entry srv_entry ;		//service entry
	void *srv_param ;					//param of srv_entry 
	nd_thsrvmsg_func msg_entry ;				//handle received message from other thread!
	nd_threadsrv_clean cleanup_entry;			//clean up when server is terminal
	nd_mutex msg_lock ;
	int send_message_retval ;			//等待对方处理消息后的返回值
#ifdef USER_P_V_LOCKMSG
	nd_cond	 msg_cond ;					//消息等待条件,只有消息线程能用
#endif
	struct list_head list;				//self list
	struct list_head msg_list ;
	struct list_head msg_entry_list;	//message handler entry
	void *_user_data ;					//user data	
	ndatomic_t is_suspend;				//0 run 1 suspend
	ndatomic_t in_suspend;				//0 wait resume, 1 in suspending
	ndsem_t sem_suspend ;					//received start signal
	nd_handle h_timer ;					//handle of timer 
	char srv_name[ND_SRV_NAME] ;	//service name
}nd_thsrv_context_t;

//服务创建参数
struct thsrv_create_param
{
	nd_thsrv_context_t *pcontext ;
	ndsem_t				sem ;

} ;

/*all service module contex*/
static struct nd_srv_entry
{
	int					status ;
	ndatomic_t			__exit ;		//1 exit 
	ndatomic_t			__suspend;
	ndatomic_t			__err_code;		//host errorcode 
	struct list_head	th_context_list ;
}__s_entry;

struct nd_srv_entry *get_srv_entry() ;

int _destroy_service(nd_thsrv_context_t *contex, int wait) ;

static int deft_msg_handler(nd_thsrv_msg *msg)  ;

struct nd_srv_entry *get_srv_entry() 
{
	return &__s_entry;
}

static	void _init_entry_context()
{
	if(1==__s_entry.status)
		return ;
	INIT_LIST_HEAD(&(__s_entry.th_context_list)) ;
	nd_atomic_set(&__s_entry.__exit, 0) ;
	nd_atomic_set(&__s_entry.__suspend, 0); 

	nd_atomic_set(&__s_entry.__err_code, 0);

	__s_entry.status = 1 ;
}

void nd_host_eixt() 
{
	nd_atomic_swap(&(__s_entry.__exit),1) ;
}

void nd_server_host_begin() 
{
	nd_atomic_swap(&(__s_entry.__exit),0) ;
}
int nd_host_check_exit() 
{
	return nd_atomic_read(&(__s_entry.__exit ));
}
int nd_host_set_error(int err)
{
	int ret = nd_atomic_read(&(__s_entry.__err_code));
	nd_atomic_swap(&(__s_entry.__err_code), err);
	return ret;
}

int nd_host_get_error()
{
	int ret = nd_atomic_read(&(__s_entry.__err_code));
	return ret;
}


int nd_thsrv_check_exit(nd_thsrvid_t srv_id)
{
	nd_thsrv_context_t *contex =(nd_thsrv_context_t *)nd_thsrv_gethandle(srv_id) ;
	if(!contex) 
		return 0 ;
	return (nd_host_check_exit() || nd_atomic_read(&(contex->__exit)) );
}

int nd_thsrv_isexit(nd_handle h_srvth )
{
	nd_thsrv_context_t *contex = (nd_thsrv_context_t *)h_srvth ;
	if(!contex) {
		contex = (nd_thsrv_context_t *)nd_thsrv_gethandle(0) ;
		if(!contex)
			return 0 ;
	}
	return (nd_atomic_read(&(__s_entry.__exit )) || nd_atomic_read(&(contex->__exit)) );

}

//专业消息处理线程函数
static int _msg_th_func(nd_thsrv_context_t *contex)
{
	ENTER_FUNC()
	int ret = 0;
	struct nd_thread_msg *node ;
	struct list_head *pos, *next;
	struct list_head header ;
	struct nd_srv_entry *srventry = get_srv_entry()  ;

	nd_mutex_lock(&contex->msg_lock) ;
#ifdef USER_P_V_LOCKMSG
		while(list_empty(&contex->msg_list) ) {
			nd_cond_wait(&contex->msg_cond,&contex->msg_lock) ;
			if(nd_atomic_read(&contex->__exit) || nd_atomic_read(&__s_entry.__exit)) {
				nd_mutex_unlock(&contex->msg_lock) ;
				LEAVE_FUNC();
				return -1 ;
			}
		}
#endif
		if(list_empty(&contex->msg_list)) {
			nd_mutex_unlock(&contex->msg_lock) ;
			LEAVE_FUNC();
			return 0 ;
		}
		list_add(&header, &contex->msg_list);
		list_del_init(&contex->msg_list) ;
	nd_mutex_unlock(&contex->msg_lock) ;
	
	if(contex->msg_entry ) {
		pos = header.next ;
		while(pos !=  &header){
			int hr ;
			next = pos->next ;
			node = list_entry(pos, struct nd_thread_msg, list) ;
			hr = contex->msg_entry(node) ;			
			pos = next ;
			
#ifdef TEST_MYMM 
			free(node) ;
#else 
			nd_pool_free(contex->allocator, node) ;
#endif
			if(-1==hr){
				LEAVE_FUNC();
				return -1 ;
			}
			++ret ;
		}
	}

	
	while(nd_atomic_read(&contex->is_suspend)>0 || nd_atomic_read(&srventry->__suspend) > 0) {
		nd_atomic_set(&contex->in_suspend,1);
		nd_sem_wait(contex->sem_suspend,-1) ;
		nd_atomic_set(&contex->in_suspend,0) ;
		++ret ;
	}
	LEAVE_FUNC();
	return ret ;
}
//received message function entry
static int _msg_entry(nd_thsrv_context_t *contex)
{
	int ret = 0;
	ENTER_FUNC()
	//struct list_head header, unhandled ;
	ND_LIST_HEAD(header) ;
	ND_LIST_HEAD(unhandled) ;
	struct nd_srv_entry *srventry = get_srv_entry()  ;

	if(list_empty(&contex->msg_list) ) {
		goto EXIT_MSG ;
	}
	nd_mutex_lock(&contex->msg_lock) ;
		list_add(&header, &contex->msg_list);
		list_del_init(&contex->msg_list) ;
	nd_mutex_unlock(&contex->msg_lock) ;
	
	if(contex->msg_entry ) {
		struct nd_thread_msg *node ;
		struct list_head *pos, *next;
		list_for_each_safe(pos,next,&header) {
			int result = 0 ;
			list_del_init(pos) ;
			node = list_entry(pos, struct nd_thread_msg, list) ;
			result = contex->msg_entry(node) ;
			++ret ;
			if (result==-1)	{
				list_add_tail(pos,&unhandled) ;
			}
			else {
				if (node->wait_handle ){
					//wakeup sender
					nd_thsrv_context_t *sender =  (nd_thsrv_context_t *) nd_thsrv_gethandle(node->from_id) ;
					if (sender){
						sender->send_message_retval = ret ;
						nd_thsrv_resume(node->from_id) ;
					}
				}
#ifdef TEST_MYMM 
				free(node) ;
#else 
				nd_pool_free(contex->allocator, node) ;
#endif
			}
		}
		if (!list_empty(&unhandled)) {
			nd_mutex_lock(&contex->msg_lock) ;
			list_join(&unhandled, &contex->msg_list);
			nd_mutex_unlock(&contex->msg_lock) ;
		}
	}
EXIT_MSG:
	
	while(nd_atomic_read(&contex->is_suspend)>0 ||  nd_atomic_read(&srventry->__suspend) > 0) {
		nd_atomic_set(&contex->in_suspend,1) ;
		nd_sem_wait(contex->sem_suspend,-1) ;
		nd_atomic_set(&contex->in_suspend,0) ;
		++ret ;
	}

	LEAVE_FUNC();
	return ret ;
}


int nd_thsrv_install_msg(nd_handle  thhandle,NDUINT32 msgid, nd_thsrvmsg_func func ) 
{
	struct thmsg_entry *pthentry ;
	nd_thsrv_context_t *contex = (nd_thsrv_context_t *)thhandle ;
#ifdef TEST_MYMM 
	pthentry = malloc( sizeof(*pthentry) ) ; //(struct nd_thread_msg *)malloc(size) ;
#else 
	pthentry = (struct thmsg_entry *) nd_pool_alloc(contex->allocator, sizeof(*pthentry)) ; //(struct nd_thread_msg *)malloc(size) ;
#endif 
	if(!pthentry) {
		return -1 ;
	}
	INIT_LIST_HEAD(&pthentry->list) ;
	pthentry->msgid = msgid ;
	pthentry->func = func ;
	list_add_tail(&pthentry->list, &contex->msg_entry_list) ;
	return 0 ;
}

int deft_msg_handler(nd_thsrv_msg *msg) 
{
	struct list_head *pos ;
	struct thmsg_entry *entry ;
	nd_thsrv_context_t *contex =(nd_thsrv_context_t *) msg->recv_handle ;
	
	pos = contex->msg_entry_list.next ;
	while (pos != &contex->msg_entry_list) {
		entry = list_entry(pos, struct thmsg_entry , list) ;
		pos = pos->next ;
		if(msg->msg_id == entry->msgid) {
			nd_assert(entry->func) ;
			entry->func(msg) ;
			break ;
		}
	}
	return 0;
}

/* sub service entry */
static void *_srv_entry(void *p)
{
#ifdef ND_CALLSTACK_TRACE
	int __push_func_return_value=-1;
#endif 
	int ret = 0;
	nd_thsrv_context_t * contex;
	
	struct thsrv_create_param *create_param = (struct thsrv_create_param *) p ;
	if(!p) {
		return (void*)-1 ;
	}
	nd_sleep(100) ;
	
	contex = create_param->pcontext ;

#ifdef ND_CALLSTACK_TRACE
	if(contex->srv_name[0] )  {
		__push_func_return_value = push_func(contex->srv_name) ;
	}
	else {
		__push_func_return_value = push_func(__FUNC__) ;
	}	
#endif

	//sem_suspend
	nd_assert(contex) ;
	nd_sem_post(create_param->sem) ;

	nd_sem_wait(contex->sem_suspend,-1) ;		//wait received run command

	nd_log_screen("*** %s server start\n", contex->srv_name) ;
	if(SUBSRV_RUNMOD_LOOP==contex->run_module) {
		while(0==nd_atomic_read(&contex->__exit) && 0==nd_atomic_read(&__s_entry.__exit)) {
			if(-1==contex->srv_entry(contex->srv_param) ) {
				nd_atomic_set(&contex->__exit,1) ;
				ret = -1 ;
				goto	EXIT_SRV ;
			}
			ret = _msg_entry(contex) ;
			if(contex->h_timer) {
				nd_timer_update(contex->h_timer) ;
			}
			if(0==ret)
				nd_sleep(10) ;
		}	//end while
		
		
	}
	else if (SUBSRV_RUNMOD_STARTUP==contex->run_module){
		ret = contex->srv_entry(contex->srv_param) ;
	}
	else if(SUBSRV_MESSAGE==contex->run_module) {		
		//专业的消息处理线程
		while(0==nd_atomic_read(&contex->__exit) && 0==nd_atomic_read(&__s_entry.__exit)) {	
			int hr = _msg_th_func(contex) ;
			if(contex->h_timer) {
				nd_timer_update(contex->h_timer) ;
			}
#ifndef USER_P_V_LOCKMSG
			if(-1== hr )
				break ;
			else if(0==hr) {
				nd_atomic_inc(&contex->is_suspend) ;
				nd_sem_wait(contex->sem_suspend,100) ;
			}
#endif
		}
#ifdef USER_P_V_LOCKMSG
		nd_cond_destroy(&contex->msg_cond) ;
#endif
	}
EXIT_SRV:
	if(contex->cleanup_entry){
		contex->cleanup_entry() ;
		contex->cleanup_entry = NULL ;		//clean up once
	}

	//退出时需要执行
	if(contex->h_timer) {
		nd_timer_destroy(contex->h_timer, 0) ;
		contex->h_timer = 0 ;
	}
	nd_log_screen("%s server is exit\n", contex->srv_name) ;

#ifdef ND_CALLSTACK_TRACE
	if (0 == __push_func_return_value) {
		if (contex->srv_name[0])  {
			pop_func(contex->srv_name);
		}
		else {
			pop_func(__FUNC__);
		}
	}
#endif

	return (void*)(ret) ;
}

/*create a service */
nd_thsrvid_t nd_thsrv_createex(struct nd_thsrv_createinfo* create_info,int priority, int suspend )
{	
	struct thsrv_create_param create_param ;
	nd_thsrv_context_t * contex;
	_init_entry_context () ;
	if(!create_info )
		return 0;
	if(create_info->run_module!=SUBSRV_MESSAGE && NULL==create_info->srv_entry) {
		return 0 ;
	}
	contex =(nd_thsrv_context_t * ) malloc(sizeof (nd_thsrv_context_t) ) ;
	if(!contex)
		return 0 ;
	
	create_param.pcontext = contex ;	
	if(-1==nd_sem_init(create_param.sem)) {
		free(contex) ;
		nd_logfatal("create thread server error for sem init error=%s !\n" AND nd_last_error()) ;
		return 0 ;
	}

	if(-1==nd_sem_init(contex->sem_suspend)) {
		nd_sem_destroy(create_param.sem) ;
		free(contex) ;
		nd_logfatal("create thread server error for sem suspend init error=%s !\n" AND nd_last_error()) ;
		return 0 ;
	}
	contex->size = sizeof(nd_thsrv_context_t) ;
	contex->type = ('t'<< 8) | 'h' ;
	contex->close_entry = (nd_close_callback )_destroy_service;

	contex->in_suspend = 0;
	contex->thid = 0  ;
	contex->th_handle = 0 ;			//thread handle
	contex->myerrno = 0 ;
	nd_atomic_set(&contex->__exit,0);
	contex->srv_entry = create_info->srv_entry ;
	contex->srv_param = create_info->srv_param ;
	contex->msg_entry = deft_msg_handler ;//create_info->msg_entry ;
	contex->cleanup_entry = NULL;//create_info->cleanup_entry;
	contex->_user_data = create_info->data ;
	contex->run_module = create_info->run_module ;
	contex->h_timer = NULL ;
	nd_atomic_swap(&contex->is_suspend ,1) ;
	strncpy(contex->srv_name,create_info->srv_name,sizeof(contex->srv_name));

	INIT_LIST_HEAD(&(contex->list)) ;
	INIT_LIST_HEAD(&(contex->msg_list)) ;
	INIT_LIST_HEAD(&(contex->msg_entry_list)) ;
	nd_mutex_init(&(contex->msg_lock)) ;

#ifdef USER_P_V_LOCKMSG
	if(SUBSRV_MESSAGE==create_info->run_module){
		if(-1==nd_cond_init(&contex->msg_cond)  ) {
			nd_logfatal("init cond error in message thread!\n") ;
			return -1 ;
		}
		//contex->allocator = nd_pool_create(EMEMPOOL_UNLIMIT) ;
	}
	//else 
#else 
#endif
	
	contex->allocator = nd_pool_create(EMEMPOOL_UNLIMIT,create_info->srv_name) ;
	if(NULL==contex->allocator) {
		contex->allocator = nd_global_mmpool() ;
	}

	//create_param 
	contex-> th_handle = nd_createthread(_srv_entry, &create_param, &(contex->thid),priority) ;
	if(!contex-> th_handle ){
		nd_sem_destroy(create_param.sem) ;
		free(contex) ;
		return 0;
	}
	list_add(&(contex->list), &(__s_entry.th_context_list)) ;
	nd_sem_wait(create_param.sem, -1) ;
nd_log_screen("%s server create success id=%d!\n", contex->srv_name, (int) contex->thid) ;	
	nd_sem_destroy(create_param.sem) ;

	if(0==suspend) {
		nd_atomic_swap(&contex->is_suspend ,0) ;
		nd_sem_post(contex->sem_suspend);
	}
	return contex->thid ;
}

int nd_thsrv_suspend(nd_thsrvid_t  srv_id) 
{
	nd_thsrv_context_t *contex = (nd_thsrv_context_t *)nd_thsrv_gethandle(srv_id ) ;
	if(!contex ){
		return -1;
	}
	nd_atomic_inc(&contex->is_suspend) ;
	do 	{
		nd_sleep(20) ;
	} while (nd_atomic_read(&contex->in_suspend)==0);
	return 0 ;
}


int nd_thsrv_suspend_self(nd_handle handle) 
{
	ENTER_FUNC()
	nd_thsrv_context_t *contex = (nd_thsrv_context_t *)handle ;
	//nd_thsrv_context_t *contex = (nd_thsrv_context_t *)nd_thsrv_gethandle(srv_id ) ;
	nd_assert(contex->thid == nd_thread_self()) ;
	nd_assert(SUBSRV_MESSAGE!=contex->run_module) ;

	if ((int)nd_atomic_read(&contex->is_suspend) < 0){
		nd_atomic_set(&contex->is_suspend,0) ;
	}
	nd_atomic_inc(&contex->is_suspend) ;

	do{
		nd_sem_wait(contex->sem_suspend,1000) ;
	}while(nd_atomic_read(&contex->is_suspend)>0)  ;

	LEAVE_FUNC();
	return 0 ;
}

int nd_thsrv_resume(nd_thsrvid_t  srv_id) 
{
	nd_thsrv_context_t *contex = (nd_thsrv_context_t *)nd_thsrv_gethandle(srv_id ) ;
	if(!contex)
		return -1;

	if ((int)nd_atomic_read(&contex->is_suspend) <= 1){
		nd_atomic_swap(&contex->is_suspend,0) ;
	}
	else {
		nd_atomic_dec(&contex->is_suspend) ;
	}
	nd_sem_post(contex->sem_suspend) ;
	return 0 ;
}

int nd_thsrv_suspendall()
{
	nd_thsrvid_t srvid = nd_thread_self() ;
	struct list_head *pos, *next ;
	nd_thsrv_context_t *context ;
	struct nd_srv_entry *srventry  = get_srv_entry()  ;

	nd_atomic_set(&srventry->__suspend,1) ;

	pos = srventry->th_context_list.next ;
	while(pos != &(srventry->th_context_list)) {
		next = pos->next ;
		context = list_entry(pos,nd_thsrv_context_t,list) ;
		if(!nd_thread_equal(context->thid,srvid)) {
			nd_atomic_inc(&context->is_suspend) ;
			do 	{
				nd_sleep(20) ;
			} while (nd_atomic_read(&context->in_suspend)==0);
		}
		pos = next ;
	}
	return 0 ;
}

int nd_thsrv_resumeall() 
{
	nd_thsrvid_t srvid = nd_thread_self() ;
	struct list_head *pos, *next ;
	nd_thsrv_context_t *context ;
	struct nd_srv_entry *srventry = get_srv_entry()  ;

	nd_atomic_set(&srventry->__suspend,0) ;

	pos = srventry->th_context_list.next ;
	while(pos != &(srventry->th_context_list)) {
		next = pos->next ;
		context = list_entry(pos,nd_thsrv_context_t,list) ;
		if(!nd_thread_equal(context->thid,srvid)) {
			if ((int)nd_atomic_read(&context->is_suspend) <= 1){
				nd_atomic_swap(&context->is_suspend,0) ;
			}
			else {
				nd_atomic_dec(&context->is_suspend) ;
			}
			nd_sem_post(context->sem_suspend) ;
		}
		pos = next ;
	}
	return 0 ;
}


int _destroy_service(nd_thsrv_context_t *contex, int wait)
{
	ENTER_FUNC()
	int ret = 0;
	struct list_head *pos;
	struct nd_srv_entry *srventry = get_srv_entry()  ;

	nd_atomic_set(&srventry->__suspend,0) ;
	nd_assert(contex);

	nd_atomic_swap(&contex->__exit,1);
#ifdef USER_P_V_LOCKMSG
	if(SUBSRV_MESSAGE==contex->run_module) {
		nd_mutex_lock(&contex->msg_lock) ;
			nd_cond_signal(&contex->msg_cond) ;
		nd_mutex_unlock(&contex->msg_lock) ;
	}
	else 
#endif 
	{
		while(nd_atomic_read(&contex->is_suspend)>0) {
			nd_atomic_swap(&contex->is_suspend,0) ;
			nd_sem_post(contex->sem_suspend) ;
		}
	}

	if(contex->th_handle && wait){
		nd_log_screen("waiting for %s id=%d\n", contex->srv_name, (int)contex->thid) ;
		ret = nd_waitthread(contex->th_handle) ;
		
		nd_log_screen("success waiting for %s(%d) success ret =%d\n", contex->srv_name,(int)contex->thid, ret) ;
	}
	list_del(&(contex->list)) ;

	//destroy timer 
	if(contex->h_timer) {
		nd_timer_destroy(contex->h_timer, wait) ;
		contex->h_timer = 0 ;
	}
	//destroy message context
	
	pos = contex->msg_list.next ;
	while(pos !=  &contex->msg_list){
		struct nd_thread_msg *node = list_entry(pos, struct nd_thread_msg, list) ;
		pos = pos->next ;
		nd_pool_free(contex->allocator, node) ;
	}
	//destroy message entry 
	pos = contex->msg_entry_list.next ;
	while(pos != &contex->msg_entry_list) {
		struct thmsg_entry *entry = list_entry(pos,struct thmsg_entry, list) ;
		pos = pos->next ;
		nd_pool_free(contex->allocator, entry) ;
	}

	if(contex->cleanup_entry)
		contex->cleanup_entry() ;
	nd_sem_destroy(contex->sem_suspend) ;

	nd_mutex_destroy(&contex->msg_lock) ;
	if(contex->allocator) {
		nd_handle h_alloc = nd_global_mmpool() ;
		if(h_alloc!=contex->allocator) {
			nd_pool_destroy(contex->allocator, 0) ;
		}
		contex->allocator = 0 ;
	}
	free(contex) ;

	LEAVE_FUNC();
	return ret ;
}

int nd_thsrv_destroy(nd_thsrvid_t srvid,int force)
{
	nd_thsrv_context_t *contex = (nd_thsrv_context_t *)nd_thsrv_gethandle(srvid ) ;
	if(!contex)
		return -1;
	
	return _destroy_service(contex, !force);
}

//find service context
nd_handle nd_thsrv_gethandle(nd_thsrvid_t srvid )
{
	struct list_head *pos, *next ;
	nd_thsrv_context_t *context ;
	struct nd_srv_entry *srventry ;
	if(0==srvid) {
		srvid = nd_thread_self() ;
	}

	srventry = get_srv_entry()  ;
	pos = srventry->th_context_list.next ;
	while(pos != &(srventry->th_context_list)) {
		next = pos->next ;
		context = list_entry(pos,nd_thsrv_context_t,list) ;
		if(nd_thread_equal(context->thid,srvid))
			return (nd_handle)context ;
		pos = next ;
	}
	return NULL ;
}

nd_thsrvid_t nd_thsrv_getid(nd_handle handle)
{
	return ((nd_thsrv_context_t *)handle)->thid ;
}

void* nd_thsrv_getdata(nd_handle handle)
{
	return ((nd_thsrv_context_t *)handle)->_user_data ;
}
void nd_thsrv_setdata(nd_handle handle, void *data)
{
	((nd_thsrv_context_t *)handle)->_user_data = data;
}
void nd_thsrv_release_all() 
{
	ENTER_FUNC()
	struct list_head *header ;
	nd_thsrv_context_t *node, *next_node ;
	struct nd_srv_entry *entry = get_srv_entry() ;
	if (__s_entry.status == 0)	{
		LEAVE_FUNC();
		return;
	}
	__s_entry.status = 0;

	header =&( entry->th_context_list);

	list_for_each_entry_safe(node, next_node, header, nd_thsrv_context_t, list){
		_destroy_service(node, 1);
	}
// 
// nd_log_screen("release all thread server\n") ;
// 	pos = entry->th_context_list.next ;
// 	while(pos !=&(entry->th_context_list))  {
// 		next = pos->next ;
// 		node = list_entry(pos,nd_thsrv_context_t,list) ;
// 		_destroy_service(node,1) ;
// 		pos = next ;
// 	}
	INIT_LIST_HEAD(&(__s_entry.th_context_list)) ;

	LEAVE_FUNC();
}


int nd_thsrv_send(nd_thsrvid_t srvid,NDUINT32 msgid,void *data, NDUINT32 data_len) 
{
	return nd_thsrv_sendex( srvid, msgid,data,  data_len,0)  ;
}

int nd_thsrv_sendex(nd_thsrvid_t srvid,NDUINT32 msgid,void *data, NDUINT32 data_len,int iswait) 
{
	int ret = 0;
	ENTER_FUNC()
	size_t size = data_len + sizeof(struct nd_thread_msg );
	struct nd_thread_msg *msg_addr;	
	struct nd_srv_entry *srventry  = get_srv_entry()  ;
	nd_thsrv_context_t *contex =(nd_thsrv_context_t *) nd_thsrv_gethandle(srvid) ;
	if(!contex || nd_atomic_read(&srventry->__suspend) ) {
		LEAVE_FUNC();
		return -1 ;
	}
#ifdef TEST_MYMM 
	msg_addr = malloc(contex->allocator, size) ; //(struct nd_thread_msg *)malloc(size) ;
#else 
	msg_addr = nd_pool_alloc(contex->allocator, size) ; //(struct nd_thread_msg *)malloc(size) ;
#endif 
	nd_assert(msg_addr) ;
	if(!msg_addr){
		LEAVE_FUNC();
		return -1 ;
	}
	msg_addr->data_len = (NDUINT32)data_len ;
	INIT_LIST_HEAD(&(msg_addr->list)) ;


	msg_addr->msg_id = msgid;
	msg_addr->from_id = nd_thread_self();
	//msg_addr->recv_id = srvid;
	msg_addr->recv_handle = (nd_handle)contex ;
	msg_addr->th_userdata = contex->_user_data ;
	msg_addr->wait_handle = iswait;

	if(data_len>0 && data) {
		memcpy(msg_addr->data, data, data_len) ;
		ret = data_len ;
	}
	if (msg_addr->from_id == srvid){
		ret = contex->msg_entry(msg_addr) ;

#ifdef TEST_MYMM 
		free(msg_addr) ;
#else 
		nd_pool_free(contex->allocator, msg_addr) ; 
#endif 
		LEAVE_FUNC();
		return ret ;
	}

#ifdef USER_P_V_LOCKMSG
	if(SUBSRV_MESSAGE==contex->run_module) {
		nd_mutex_lock(&contex->msg_lock) ;
			list_add_tail(&msg_addr->list, &contex->msg_list);
			nd_cond_signal(&contex->msg_cond) ;
		nd_mutex_unlock(&contex->msg_lock) ;
		goto EXIT_MSG ;
		//LEAVE_FUNC();
		//return 0 ;
	}
	else 
#endif
	{
		nd_mutex_lock(&contex->msg_lock) ;
			list_add_tail(&msg_addr->list, &contex->msg_list);
		nd_mutex_unlock(&contex->msg_lock) ;
		
		if(nd_atomic_read(&contex->is_suspend)>0) {
			nd_atomic_swap(&contex->is_suspend,0) ;		//强制唤醒
			nd_sem_post(contex->sem_suspend) ;
		}
		else 
			nd_sem_post(contex->sem_suspend) ;

	}
#ifndef USER_P_V_LOCKMSG
#else 

EXIT_MSG:
#endif

	if (iswait){

		nd_thsrv_context_t *self_contex =(nd_thsrv_context_t *) nd_thsrv_gethandle(0) ;
		if(!self_contex) {
			nd_assert(self_contex) ;
			ret = -1 ;
		}
		else if(-1==nd_thsrv_suspend_self((nd_handle)self_contex)) {
			ret = -1 ;
		}
		else {
			self_contex->send_message_retval ;
		}
	}
	LEAVE_FUNC();
	return ret ;
}

int nd_thsrv_msghandler(nd_handle srv_handle) 
{
	int ret = 0;
	ENTER_FUNC()

	nd_thsrv_context_t *current_contex = (nd_thsrv_context_t *)srv_handle;
	if(!current_contex)
		current_contex = (nd_thsrv_context_t *)nd_thsrv_gethandle(0) ;
	
	nd_assert(current_contex);
	nd_assert(nd_thread_equal(current_contex->thid, nd_thread_self())) ;	
	nd_assert(SUBSRV_RUNMOD_STARTUP==current_contex->run_module) ;
	
	if(nd_atomic_read(&(current_contex->__exit)) || nd_atomic_read(&(__s_entry.__exit))){
		LEAVE_FUNC();
		return -1 ;
	}
	if(current_contex->h_timer) {
		nd_timer_update(current_contex->h_timer) ;
	}
	ret = _msg_entry(current_contex) ;
// 
// 	srventry = get_srv_entry()  ;
// 	while(nd_atomic_read(&current_contex->is_suspend)>0 || nd_atomic_read(&srventry->__suspend) > 0) {
// 		nd_atomic_set(&current_contex->in_suspend,1);
// 		nd_sem_wait(contex->sem_suspend,-1) ;
// 		nd_atomic_set(&current_contex->in_suspend,0) ;
// 	}
	LEAVE_FUNC();
	return ret ;
}

//terminal a service
int nd_thsrv_end(nd_thsrvid_t  srv_id) 
{
	nd_thsrv_context_t *contex = (nd_thsrv_context_t *)nd_thsrv_gethandle(srv_id) ;
	if(contex) {
		nd_atomic_swap(&(contex->__exit),1) ;
		return 0 ;
	}
	return -1 ;
}

int nd_thsrv_wait(nd_thsrvid_t  srv_id)
{
	ENTER_FUNC()
	nd_thsrv_context_t *contex = (nd_thsrv_context_t *)nd_thsrv_gethandle(srv_id) ;
	if(contex) {
		int ret= nd_waitthread(contex->th_handle) ;
		nd_close_handle(contex->th_handle);
	}
	LEAVE_FUNC();
	return -1 ;
}

nd_handle nd_thsrv_local_mempool(nd_handle  thsrv_handle) 
{
	return ((nd_thsrv_context_t*)thsrv_handle)->allocator ;
}

ndtimer_t nd_thsrv_timer(nd_thsrvid_t srv_id,nd_timer_entry func,void *param,ndtime_t interval, int run_type )
{	
	nd_thsrv_context_t *contex = (nd_thsrv_context_t *)nd_thsrv_gethandle(srv_id) ;
	if(!contex) {
		return 0 ;
	}
	if(!contex->h_timer) {
		contex->h_timer = nd_timer_create(contex->allocator) ;
		if(!contex->h_timer)
			return 0 ;
	}
	return nd_timer_add(contex->h_timer, func, param, interval, run_type );
}

void nd_thsrv_del_timer(nd_thsrvid_t srv_id, ndtimer_t timer_id )
{
	nd_thsrv_context_t *contex = (nd_thsrv_context_t *)nd_thsrv_gethandle(srv_id) ;
	if(!contex || !contex->h_timer) {
		return ;
	}
	nd_timer_del(contex->h_timer, timer_id) ;
}


nd_thsrvmsg_func nd_thsrv_hook(nd_handle handle,nd_thsrvmsg_func newfunc) 
{
	nd_thsrvmsg_func ret = ((nd_thsrv_context_t *)handle)->msg_entry ;
	((nd_thsrv_context_t *)handle)->msg_entry = newfunc ;
	return ret ;
}
nd_threadsrv_clean  nd_thsrv_set_clear(nd_handle handle,nd_threadsrv_clean clear_func)
{
	nd_threadsrv_clean ret = ((nd_thsrv_context_t *)handle)->cleanup_entry ;
	((nd_thsrv_context_t *)handle)->cleanup_entry = clear_func;
	return ret ;
}
