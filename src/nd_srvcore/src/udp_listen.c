/* fiel udp_listen.c
 * 
 * listen udp socket 
 * 
 * all right reserved by neil duan 
 
 * 2010/3/21 9:14:56
 */
// 
// #include "nd_common/nd_common.h"
// #include "nd_srvcore/nd_srvlib.h"
// #include "nd_net/nd_netlib.h"

#include "nd_srvcore/nd_srvlib.h"

//#define USE_UDP_THREAD_POOL

typedef int (*udp_entry) (SOCKADDR_IN *addr,char *buf, size_t read_len, nd_udtsrv *root)  ;
extern int udt_listen_callback(struct listen_contex *listen_info) ;

int create_udp_thpool(struct listen_contex *listen_info,udp_entry data_entry);
int read_datagram(struct nd_netsocket *node, char *buf, size_t buf_size, SOCKADDR_IN *addr );

#define UDP_THREAD_NUM 4 

struct udp_thpool 
{
	volatile int		 used ;
	volatile int		 isexit;
	nd_mutex fd_lock ;
	nd_cond  fd_cond ;
	nd_thsrvid_t thpool_ids[UDP_THREAD_NUM] ;
};



int udp_server_entry(struct listen_contex *listen_info)
{
	ENTER_FUNC()
	ndtime_t tm_start ;
	int ret  ;
	nd_handle context  = nd_thsrv_gethandle(0) ;
	nd_udtsrv *root = &listen_info->udt ;
	udp_entry  udp_func ;

	//struct udp_thpool *pool=NULL;

	SOCKADDR_IN  addr ;
	char readbuf[ND_UDP_PACKET_SIZE] ;

	if(!context){
		nd_logerror("can not get listen service context") ;
		LEAVE_FUNC();
		return -1 ;
	}
	nd_log_screen("start udt-%s listen!\n" AND listen_info->tcp.sock_type==SOCK_DGRAM ? "UDP": "ICMP") ;

	ret = 1 ;
	tm_start = nd_time() ;
	
	if (PROTOCOL_UDT==root->protocol) {		//listen udt protocol
		struct nd_thsrv_createinfo subth_info = {
			SUBSRV_RUNMOD_STARTUP,	//srv_entry run module (ref e_subsrv_runmod)
			(nd_threadsrv_entry)udt_listen_callback,			//service main entry function address
			listen_info,		//param of srv_entry 
			listen_info,		//user data
			("update_connect")			//service name
		};
		listen_info->listen_id = nd_thsrv_createex(&subth_info,NDT_PRIORITY_HIGHT,0) ;
		if(!listen_info->listen_id ){
			LEAVE_FUNC();
			return -1;
		}
		udp_func = (udp_entry) udt_data_handler;
	}
	else if(PROTOCOL_RUDP==root->protocol) {
		udp_func = (udp_entry) NULL; //not surport
	}
	else if(PROTOCOL_OTHER==root->protocol) {
		//udp_func =(udp_entry) root->data_entry ;
		udp_func = NULL ;		//not surport
	}
	nd_assert(udp_func) ;


#ifndef USE_UDP_THREAD_POOL 
	while (nd_thsrv_msghandler(context) >= 0){
		int read_len = 0 ;
		//int sock_len = sizeof(addr) ;

		if (ret > 0){
			read_len = read_datagram((struct nd_netsocket *)root, readbuf, sizeof(readbuf), &addr )  ;
			//read_len  = recvfrom(root->fd, readbuf, sizeof(readbuf), 0, (LPSOCKADDR)&addr, &sock_len )  ;
			if (read_len > 0) {
				udp_func(&addr, readbuf, read_len, root)  ;				
			}
		}
		if(read_len <=0) {
			ret = nd_socket_wait_read(root->fd,30) ;
			if(ret <= 0)
				update_all_socket(root) ; 
		}		
	}
#else

	if(-1==create_udp_thpool(listen_info,udp_func)) {
		LEAVE_FUNC();
		return -1;
	}
	pool = listen_info->th_pool ;

	while (nd_thsrv_msghandler(context)>=0){
		int read_len = 0 ;
		//int sock_len = sizeof(addr) ;

		nd_mutex_lock(&pool->fd_lock) ;		
		while(pool->used && pool->isexit==0) 
			nd_cond_wait(&pool->fd_cond,&pool->fd_lock) ;	
		if (pool->isexit) {	
			nd_mutex_unlock(&pool->fd_lock) ;
			break;
		}
		++pool->used ;
		nd_mutex_unlock(&pool->fd_lock) ;


		read_len = read_datagram((struct nd_netsocket *)root, readbuf, sizeof(readbuf), &addr )  ;
		if (read_len <= 0){
			if(nd_socket_wait_read(root->fd,100) >0) {
				read_len  = read_datagram((struct nd_netsocket *)root, readbuf, sizeof(readbuf), &addr )  ;
			}
		}

		nd_mutex_lock(&pool->fd_lock) ;
			--pool->used ;
			nd_cond_signal(&pool->fd_cond) ;
		nd_mutex_unlock(&pool->fd_lock) ;
		if (read_len > 0) {
			udp_func(&addr, readbuf, read_len, root)  ;				
		}
	}
#endif

	LEAVE_FUNC();
	return 0 ;
}


int udt_listen_callback(struct listen_contex *listen_info)
{
	ENTER_FUNC()
	nd_handle context  = nd_thsrv_gethandle(0) ;
	nd_udtsrv *root = &listen_info->udt ;

	if(!context){
		nd_logerror("can not get listen service context") ;
		LEAVE_FUNC();
		return -1 ;
	}

	while (nd_thsrv_msghandler(context)>=0){
		update_all_socket(root) ;
		if (listen_info->connector_hub) {
			update_connector_hub((nd_listen_handle)listen_info) ;
		}
		nd_sleep(10);
	}

	LEAVE_FUNC();
	return 0 ;
}


int udp_data_handler(struct listen_contex *listen_info)
{
	ENTER_FUNC()
	udp_entry udp_func;
	nd_handle context  = nd_thsrv_gethandle(0) ;
	nd_udtsrv *root = &listen_info->udt ;
	struct udp_thpool *pool = listen_info->th_pool ;
	
	SOCKADDR_IN  addr ;
	char readbuf[ND_UDP_PACKET_SIZE] ;

	if(!context){
		nd_logerror("can not get listen service context") ;
		LEAVE_FUNC();
		return -1 ;
	}

	udp_func = nd_thsrv_getdata(context) ;
	nd_assert(udp_func) ;
	nd_thsrv_setdata(context,listen_info) ;

	while (nd_thsrv_msghandler(context)>=0){
		int read_len = 0 ;
		//int sock_len = sizeof(addr) ;
		
		nd_mutex_lock(&pool->fd_lock) ;		
			while(pool->used && pool->isexit==0) 
				nd_cond_wait(&pool->fd_cond,&pool->fd_lock) ;	
			if (pool->isexit) {	
				nd_mutex_unlock(&pool->fd_lock) ;
				break;
			}
			++pool->used ;
		nd_mutex_unlock(&pool->fd_lock) ;

		read_len = read_datagram((struct nd_netsocket *)root, readbuf, sizeof(readbuf), &addr )  ;
		if (read_len <= 0){
			if(nd_socket_wait_read(root->fd,100) >0) {
				read_len = read_datagram((struct nd_netsocket *)root, readbuf, sizeof(readbuf), &addr )  ;
			}
		}

		nd_mutex_lock(&pool->fd_lock) ;
			--pool->used ;
			nd_cond_signal(&pool->fd_cond) ;
		nd_mutex_unlock(&pool->fd_lock) ;
		if (read_len > 0) {
			udp_func(&addr, readbuf, read_len, root)  ;				
		}

	}

	LEAVE_FUNC();
	return 0 ;
}

int create_udp_thpool(struct listen_contex *listen_info,udp_entry data_entry)
{
	int i;
	struct nd_thsrv_createinfo subth_info = {
		SUBSRV_RUNMOD_STARTUP,	//srv_entry run module (ref e_subsrv_runmod)
		(nd_threadsrv_entry)udp_data_handler,			//service main entry function address
		listen_info		//param of srv_entry 
		//_NDT("udp_data"),			//service name
		//data_entry		//user data
	};
	
	struct udp_thpool *pool = malloc(sizeof(struct udp_thpool)) ;
	if(!pool) {
		return -1 ;
	}
	memset(pool,0, sizeof(*pool)) ;

	nd_mutex_init(&pool->fd_lock) ;
	nd_cond_init(&pool->fd_cond);
	listen_info->th_pool = pool ;

	subth_info.data = data_entry ;
	for(i=0; i<UDP_THREAD_NUM; i++) {
		snprintf(subth_info.srv_name, sizeof(subth_info.srv_name),("udp_data%d"),i+1) ;
		pool->thpool_ids[i] = nd_thsrv_createex(&subth_info,NDT_PRIORITY_HIGHT,0) ;
	}

	return 0;
}

int destroy_udp_thpool(struct listen_contex *listen_info,int flag)
{
	int i;
	struct udp_thpool *pool = listen_info->th_pool;
	if(!pool) {
		return 0 ;
	}


	pool->isexit = 1 ;
	nd_mutex_lock(&pool->fd_lock) ;
		nd_cond_broadcast(&pool->fd_cond) ;
	nd_mutex_unlock(&pool->fd_lock) ;

	for(i=0; i<UDP_THREAD_NUM; i++) {
		if(pool->thpool_ids[i]) {
			nd_thsrv_destroy(pool->thpool_ids[i], flag) ;
			pool->thpool_ids[i] = 0 ;
		}
	}
	
	nd_sleep(10) ;
	nd_cond_destroy(&pool->fd_cond);
	nd_mutex_destroy(&pool->fd_lock) ;
	free(listen_info->th_pool) ;

	listen_info->th_pool = NULL;
	return 0;
}

int read_datagram(struct nd_netsocket *node, char *buf, size_t buf_size, SOCKADDR_IN *addr )
{
	socklen_t sock_len =  sizeof(*addr) ;
	if(node->sock_type == SOCK_DGRAM)
		return (int)recvfrom(node->fd, buf, buf_size, 0, (LPSOCKADDR)addr, &sock_len )  ;
	else {
		return (int)icmp_socket_read((struct nd_netsocket*)node , buf, buf_size, addr, node->bindip,node->port) ;
	}
}
