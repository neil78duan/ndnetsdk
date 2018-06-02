/* file :nd_listensrv.h
 * define a sub service of net listen 
 * may be you would like to use yourself listen sub service
 *
 * all right reserved by neil duan 2007
 */

/*创建 net listen 的服务(线程)
 */
#ifndef _ND_LISTENSRV_H_
#define _ND_LISTENSRV_H_

//#include "nd_srvcore/nd_srvlib.h"
#include "nd_srvcore/client_map.h"
#include "nd_srvcore/nd_session.h"
//#include "nd_net/nd_udt.h"

#define CONNECTION_TIMEOUT		60		//time out between twice read/write (second)
#define LISTEN_INTERVAL			30		//update 30hz
#define PRE_THREAD_CREATED		4		//如果使用TCP模式,预先创建的线程数
#define SESSION_PER_THREAD		512		//池中每个线程默认连接数
#define CONNECTORS_IN_LISTEN	64

//#define SURPORT_SINGLE_THREAD_MOD			//支持单线程模式

enum ND_LISTEN_MOD{
	ND_LISTEN_COMMON = 0 ,
	ND_LISTEN_OS_EXT,
	ND_LISTEN_UDT_STREAM			//使用UDT服务STREAM
};

#define _IS_UDT_MOD(iomod) ((iomod)==ND_LISTEN_UDT_STREAM )

typedef int (*listen_thread_update)(nd_handle h_listen, nd_handle th_handle) ;

typedef int(*session_valid_func)(nd_handle session);


//listen more than one port
struct listen_port_node
{
	struct list_head list;
	ndsocket_t fd ;
};

/* listen service contex*/
struct listen_contex
{
	union {
		struct nd_srv_node	tcp;		//server port
		nd_udtsrv			udt;		//udt socket port
	};
	int io_mod ;						//listen module
	int close_accept;					//do not open 'accept' function
	ndtime_t	operate_timeout ;		//timeout between twice net read/write (if timeout session would be close)
	ndtime_t	empty_conn_timeout;		//time out of stat ==1 
	volatile nd_thsrvid_t  listen_id ;			//listen thread server id
	//volatile nd_thsrvid_t  sub_id ;				//sub thread server id
	void	 *th_pool;							//thread pool data
	void	 *user_data ;						//user data of listener
	struct node_root  *connector_hub ;			//connector manager
	struct list_head list_thread;				//list of thread
	listen_thread_update pre_update ;
	listen_thread_update end_update ;
	session_valid_func check_valid_func;		//check session is valid
	struct list_head list_ext_ports ;
} ;

#ifdef IMPLEMENT_LISTEN_HANDLE
	typedef struct listen_contex *nd_listen_handle ;
#else 
	typedef nd_handle	nd_listen_handle ;
#endif

/*打开网络,并且启动监听线程*/
ND_SRV_API int nd_listensrv_open(int is_ipv6, int port, nd_listen_handle handle,int thread_num, const char* bindip) ;
/*关闭网络关闭监听线程*/
ND_SRV_API int nd_listensrv_close(nd_listen_handle handle, int force) ;
ND_SRV_API int nd_listensrv_checkvalid(nd_listen_handle handle) ;
/*设置对应连接的相关属性并分配内存*/
ND_SRV_API int nd_listensrv_session_info(nd_listen_handle handle, int max_client,size_t session_size) ;
/*设置连接进入和退出的回调函数*/
ND_SRV_API void nd_listensrv_set_entry(nd_listen_handle handle, accept_callback income,  deaccept_callback outcome) ;

ND_SRV_API nd_handle nd_listensrv_get_cmallocator(nd_listen_handle handle) ;

//add another port to listen list 
ND_SRV_API int nd_listensrv_add_port(nd_listen_handle handle , int port, const char* bindip ) ;

//得到连接管理器
ND_SRV_API struct cm_manager *nd_listensrv_get_cmmamager(nd_listen_handle handle) ;

//得到监听器容量
static __INLINE__ int nd_listensrv_capacity(nd_listen_handle handle) 
{
	return  nd_srv_capacity((struct nd_srv_node *)handle) ; 
}

static __INLINE__ void nd_listensrv_set_accept(nd_listen_handle handle, int bClosed) 
{
	((struct listen_contex *)handle)->close_accept = bClosed ;
}

ND_SRV_API session_valid_func nd_listensrv_set_valid_func(nd_listen_handle h_listen, session_valid_func func);

ND_SRV_API int nd_listensrv_freenum(nd_listen_handle handle) ;

//在监听器上建立定时器
//只有在单线程模式的时候才有必要在监听器上创建定时
//这样可以保证定时器和消息处理都在一个线程上执行
//如果是多线程模式可以建立单独的线程来执行定时器函数这样时间会更精确一些
ND_SRV_API ndtimer_t nd_listensrv_timer(nd_listen_handle h_listen,nd_timer_entry func,void *param,ndtime_t interval, int run_type ) ;
ND_SRV_API void nd_listensrv_del_timer(nd_listen_handle h_listen, ndtimer_t timer_id ) ;

/* 释放已经被关闭,但是还在使用的绘话*/
//void release_dead_cm(struct listen_contex * lc) ;
//得到listen file description
static __INLINE__ ndsocket_t get_listen_fd(struct listen_contex * handle)
{
	return handle->tcp.fd;
}

struct nd_client_map * accetp_client_connect(struct listen_contex *listen_info,ndsocket_t sock_fd) ;


/*deal with received net message*/
/*ND_SRV_API*/ int nd_do_netmsg(struct nd_client_map *cli_map,struct nd_srv_node *srv_node) ;

ND_SRV_API void udt_clientmap_init(struct nd_udtcli_map *node, nd_handle h_listen ) ; 

static __INLINE__ void nd_set_connection_timeout(nd_listen_handle h, int second)
{
	((struct listen_contex*)h)->operate_timeout = second * 1000 ;
}

static __INLINE__ ndtime_t nd_get_connection_timeout(nd_listen_handle h)
{
	return ((struct listen_contex*)h)->operate_timeout;
}

static __INLINE__ void nd_listensrv_set_empty_conntimeout(nd_listen_handle h, int second) 
{
	((struct listen_contex*)h)->empty_conn_timeout = second * 1000 ;
}


//为listen句柄handle 创建一个管理连接session的线程
ND_SRV_API nd_thsrvid_t nd_listensrv_thread_alloc(nd_listen_handle h);
ND_SRV_API int nd_listensrv_thread_free(nd_listen_handle h,nd_thsrvid_t thid);
//设置每个线程最多管理的连接数,默认为1024个,在nd_listensrv_open之前使用
ND_SRV_API int nd_thpool_set_sessions(nd_listen_handle h,int num_per_session);
ND_SRV_API int nd_thpool_get_sessions(nd_listen_handle h);

//把一个连接器添加中监听器中,让监听器来处理网络事件
ND_SRV_API NDUINT16 nd_listensrv_attach(nd_listen_handle h_listen, nd_handle h_connector, nd_thsrvid_t thid) ;
ND_SRV_API int nd_listensrv_deattach(nd_listen_handle h_listen, nd_handle h_connector,nd_thsrvid_t thid) ;
ND_SRV_API nd_thsrvid_t nd_listensrv_getowner(nd_listen_handle h_listen, nd_handle session) ;
typedef int (*connector_close_entry)(nd_handle net_handle,int flag) ;		//connector attach到listener上时connector关闭后的回调函数
ND_SRV_API int nd_listensrv_set_connector_close(nd_listen_handle h_listen,connector_close_entry ce) ;
ND_SRV_API int nd_close_all_session(nd_listen_handle listen_info) ;

ND_SRV_API int nd_listensrv_set_update(nd_listen_handle h_listen,listen_thread_update pre_entry, listen_thread_update end_entry) ;
//更新attach到listen_handle上的连接
int update_connector_hub(nd_listen_handle listen_info) ;
int update_connectors(struct node_root *pmanger) ;


#endif
