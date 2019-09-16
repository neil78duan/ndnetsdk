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
#include "nd_srvcore/nd_session.h"
#include "nd_srvcore/nd_srvobj.h"

#define USE_NEW_MODE_LISTEN_THREAD 1 

#define thread_pool_info nd_netth_context

#ifdef ND_DEBUG
#define SESSION_IN_THREAD_LOW_NUM 4 //if session number in current thread more than this , it will be switch to other thread 
#else
#define SESSION_IN_THREAD_LOW_NUM 32 //if session number in current thread more than this , it will be switch to other thread 
#endif

ND_SRV_API nd_thsrvid_t nd_listener_thread_create(nd_listen_handle h, int session_num);
ND_SRV_API int nd_listener_thread_destroy(nd_listen_handle h, nd_thsrvid_t sid);
ND_SRV_API int nd_listener_fetch_sessions(nd_listen_handle h, ndthread_t *threadid_buf, int *count_buf, int size);
int listen_thread_create(struct thread_pool_info *ic, nd_threadsrv_entry th_func);

//close all session after delay rand ms(0~65536ms) for per-session
ND_SRV_API int nd_rand_delay_cloase_all(nd_handle listen_info);
ND_SRV_API int nd_netmsg_handle(NDUINT16 sessionid, nd_usermsghdr_t *data, nd_handle listen_handle);
ND_SRV_API int nd_netmsg_2all_handle(nd_usermsghdr_t *data, nd_handle listen_handle, int priv_level);

ND_SRV_API int nd_session_switch(nd_listen_handle h, NDUINT16 sessionid, nd_thsrvid_t aimid);
ND_SRV_API int nd_session_loadbalancing(nd_listen_handle h, NDUINT16 sessionid);

ND_SRV_API int nd_listener_get_threads(nd_listen_handle h);
ND_SRV_API struct thread_pool_info *nd_thpool_get_info(nd_listen_handle h, nd_thsrvid_t thid);


int udt_session_data_handle(nd_udt_node *socket_node, struct udt_packet_info* pack_buf);
int _udt_sub_thread(struct thread_pool_info *thip);
int _utd_main_thread(struct thread_pool_info *thip);

int close_session_in_thread(struct thread_pool_info *thpi);
int update_session_in_thread(struct cm_manager *pmanger, struct thread_pool_info *thpi);
int addto_thread_pool(struct nd_session_tcp *client, struct thread_pool_info * pthinfo);
int delfrom_thread_pool(struct nd_session_tcp *client, struct thread_pool_info * pthinfo);



void init_netthread_msg( nd_handle  thhandle);
int thpoolex_destroy(struct listen_contex *handle) ;

int attach_to_listen(struct thread_pool_info *thip,struct nd_session_tcp *client_map) ;
int deattach_from_listen(struct thread_pool_info *thip , struct nd_session_tcp *client_map)  ;

int _nd_thpool_main(struct thread_pool_info *listen_info);
int _nd_thpool_sub(struct thread_pool_info *listen_info);



#endif
