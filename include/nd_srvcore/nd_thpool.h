/* file nd_thpool.h
 * 
 * define thread pool handle net io
 *
 * create by duan 
 * 2011/3/3 15:09:44
 */

#ifndef _ND_THPOOL_H_
#define _ND_THPOOL_H_

//#include "nd_common/nd_common.h"
#include "nd_srvcore/client_map.h"
#include "nd_srvcore/nd_session.h"
#include "nd_srvcore/nd_listensrv.h"

#pragma  warning(push)
#pragma warning (disable : 4200 )
struct thread_pool_info 
{
	ndthread_t thid ;
	HANDLE iopc_handle ;
	struct listen_contex *lh;
	struct list_head list ;
	int max_sessions ;
	int session_num ;
	struct node_root  *connector_hub ;			//connector manager
	NDUINT16 sid_buf[0] ;

};
#pragma  warning(pop)

#ifdef ND_DEBUG
#define SESSION_IN_THREAD_LOW_NUM 4 //if session number in current thread more than this , it will be switch to other thread 
#else
#define SESSION_IN_THREAD_LOW_NUM 32 //if session number in current thread more than this , it will be switch to other thread 
#endif

int nd_srvnode_create(struct node_root *root, int max_num, size_t node_size,NDUINT16 start_id,nd_handle mmpool) ;
void nd_srvnode_destroy(struct node_root *root);

void init_netthread_msg( nd_handle  thhandle);
//iocp 线程池
//int thpoolex_create(struct listen_contex *handle, int pre_thnum, int session_num) ;
int thpoolex_destroy(struct listen_contex *handle) ;

int attach_to_listen(struct thread_pool_info *thip,struct nd_client_map *client_map) ;
int deattach_from_listen(struct thread_pool_info *thip , struct nd_client_map *client_map)  ;

int addto_thread_pool(struct nd_client_map *, struct thread_pool_info * thpi) ;
int delfrom_thread_pool(struct nd_client_map *, struct thread_pool_info * thpi) ;
int _delfrom_thread_pool(NDUINT16 sid, struct thread_pool_info * pthinfo) ;
int close_session_in_thread(struct thread_pool_info *thpi) ;

ND_SRV_API int nd_send_toclient_ex(NDUINT16 sessionid,nd_usermsghdr_t *data, nd_handle listen_handle,int encrypt) ;
ND_SRV_API int nd_sendto_all_ex(nd_usermsghdr_t *data, nd_handle listen_handle,int priv_level,int encrypt);

//通过SESSIONid把消息发送给客户端
static __INLINE__ int nd_send_tocliet(NDUINT16 sessionid,nd_usermsghdr_t *data, nd_handle listen_handle) 
{
	return nd_send_toclient_ex( sessionid,data, listen_handle, 0) ;
}
//send to all , implement not like broadcast 
static __INLINE__ int nd_sendto_all(nd_usermsghdr_t *data, nd_handle listen_handle,int priv_level) 
{
	return nd_sendto_all_ex(data, listen_handle, priv_level, 0) ;
}


//把消息交给session处理
ND_SRV_API int nd_netmsg_handle(NDUINT16 sessionid,nd_usermsghdr_t *data, nd_handle listen_handle) ;

ND_SRV_API int nd_netmsg_2all_handle(nd_usermsghdr_t *data, nd_handle listen_handle,int priv_level) ;

//把session 添加到其他线程
ND_SRV_API int nd_session_switch(nd_listen_handle h,NDUINT16 sessionid, nd_thsrvid_t aimid);

//为session找一个相对空闲的线程
ND_SRV_API int nd_session_loadbalancing(nd_listen_handle h,NDUINT16 sessionid);
//打开
//ND_SRV_API nd_thsrvid_t nd_open_listen_thread(nd_listen_handle h) ;

ND_SRV_API nd_thsrvid_t nd_open_listen_thread(nd_listen_handle h,int session_number)  ;
ND_SRV_API int nd_close_listen_thread(nd_listen_handle h,nd_thsrvid_t sid)  ;

ND_SRV_API int nd_listen_get_threads(nd_listen_handle h) ;
ND_SRV_API struct thread_pool_info *get_thread_poolinf(nd_listen_handle h, nd_thsrvid_t thid);
ND_SRV_API int nd_fetch_sessions_in_thread(nd_listen_handle h,  ndthread_t *threadid_buf, int *count_buf, int size);

#endif
