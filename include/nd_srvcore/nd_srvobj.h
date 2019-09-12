/* file :nd_listensrv.h
 * define a sub service of net listen 
 * may be you would like to use yourself listen sub service
 *
 * all right reserved by neil duan 2007
 */

#ifndef _ND_LISTENSRV_H_
#define _ND_LISTENSRV_H_

//#include "nd_srvcore/nd_srvlib.h
#include "nd_srvcore/nd_session.h"
//#include "nd_net/nd_udt.h"

#define CONNECTION_TIMEOUT		60		//time out between twice read/write (second)
#define LISTEN_INTERVAL			30		//update 30hz
#define PRE_THREAD_CREATED		4		// default thread number 
#define SESSION_PER_THREAD		512		//session number for one thread 
#define CONNECTORS_IN_LISTEN	64

enum ND_LISTEN_MOD{
	ND_LISTEN_COMMON = 0 ,
	ND_LISTEN_OS_EXT,
	ND_LISTEN_UDT_STREAM
};

struct nd_netth_context
{
	ndthread_t thid;
	int session_num;
	HANDLE iopc_handle;
	struct listen_contex *lh;
	struct list_head list;
	struct node_root  *connector_hub;			//connector manager
	struct list_head  sessions_list;
};

#define NETTH_CONTEXT_INIT(netthcont) \
	(netthcont)->thid = 0 ;	\
	(netthcont)->session_num = 0 ;	\
	(netthcont)->iopc_handle = 0 ;	\
	(netthcont)->lh = 0 ;	\
	(netthcont)->connector_hub = 0 ;	\
	INIT_LIST_HEAD(&(netthcont)->list) ;\
	INIT_LIST_HEAD(&(netthcont)->sessions_list) ;


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
	void	 *th_pool;							//thread pool data
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

/* open port to listen */
ND_SRV_API int nd_listener_open(int is_ipv6, int port, nd_listen_handle handle,int thread_num, const char* bindip) ;
//close port 
ND_SRV_API int nd_listener_close(nd_listen_handle handle, int force) ;
ND_SRV_API int nd_listener_checkvalid(nd_listen_handle handle) ;
/* set session info */
ND_SRV_API int nd_listener_set_capacity(nd_listen_handle handle, int max_client,size_t session_size) ;
/* set accept and close callback*/
ND_SRV_API void nd_listener_set_callback(nd_listen_handle handle, accept_callback income,  deaccept_callback outcome) ;

ND_SRV_API nd_handle nd_listener_get_session_allocator(nd_listen_handle handle) ;

//add another port to listen list 
ND_SRV_API int nd_listener_add_port(nd_listen_handle handle , int port, const char* bindip ) ;

//get session manager 
ND_SRV_API struct cm_manager *nd_listener_get_session_mgr(nd_listen_handle handle) ;

static __INLINE__ int nd_listener_get_capacity(nd_listen_handle handle) 
{
	return  nd_srv_capacity((struct nd_srv_node *)handle) ; 
}

static __INLINE__ void nd_listener_set_accept(nd_listen_handle handle, int bClosed) 
{
	((struct listen_contex *)handle)->close_accept = bClosed ;
}

ND_SRV_API session_valid_func nd_listener_set_valid_func(nd_listen_handle h_listen, session_valid_func func);

ND_SRV_API int nd_listener_freenum(nd_listen_handle handle) ;

//add timer to listener 
ND_SRV_API ndtimer_t nd_listener_add_timer(nd_listen_handle h_listen,nd_timer_entry func,void *param,ndtime_t interval, int run_type ) ;
ND_SRV_API void nd_listener_del_timer(nd_listen_handle h_listen, ndtimer_t timer_id ) ;

// get listen socket fd
static __INLINE__ ndsocket_t get_listen_fd(struct listen_contex * handle)
{
	return handle->tcp.fd;
}

struct nd_session_tcp * accetp_client_connect(struct listen_contex *listen_info,ndsocket_t sock_fd) ;

static __INLINE__ void nd_listener_set_timeout(nd_listen_handle h, int second)
{
	((struct listen_contex*)h)->operate_timeout = second * 1000 ;
}

static __INLINE__ ndtime_t nd_listener_get_timeout(nd_listen_handle h)
{
	return ((struct listen_contex*)h)->operate_timeout;
}

static __INLINE__ void nd_listener_set_empty_timeout(nd_listen_handle h, int second) 
{
	((struct listen_contex*)h)->empty_conn_timeout = second * 1000 ;
}

static __INLINE__ int nd_listener_get_empty_timeout(nd_listen_handle h)
{
	return ((struct listen_contex*)h)->empty_conn_timeout / 1000;
}


//attach connector to listener 
ND_SRV_API NDUINT16 nd_listener_attach(nd_listen_handle h_listen, nd_handle h_connector, nd_thsrvid_t thid) ;
ND_SRV_API int nd_listener_deattach(nd_listen_handle h_listen, nd_handle h_connector,nd_thsrvid_t thid) ;
ND_SRV_API nd_thsrvid_t nd_listener_getowner(nd_listen_handle h_listen, nd_handle session) ;
typedef int (*connector_close_entry)(nd_handle net_handle,int flag) ;		
ND_SRV_API int nd_listener_set_connector_close(nd_listen_handle h_listen,connector_close_entry ce) ;
ND_SRV_API int nd_listener_close_all(nd_listen_handle listen_info) ;

ND_SRV_API int nd_listener_set_update(nd_listen_handle h_listen,listen_thread_update pre_entry, listen_thread_update end_entry) ;
//update connectors be attached to listener
int update_connector_hub(nd_listen_handle listen_info) ;
int update_connectors(struct node_root *pmanger) ;


#endif
