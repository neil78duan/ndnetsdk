/* file nd_netthread.h
 *
 * net listen thread server
 *
 * create by duan 
 *
 * 2017.11.1
 *
 */
 
#ifndef _ND_NET_THREAD_H_
#define _ND_NET_THREAD_H_

#include "nd_srvcore/client_map.h"
#include "nd_srvcore/nd_session.h"
#include "nd_srvcore/nd_listensrv.h"

#include "nd_common/nd_common.h"

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


//int nd_session_mgr_create(struct node_root *root, int max_num, size_t node_size, NDUINT16 start_id, nd_handle mmpool);
//void nd_session_mgr_destroy(struct node_root *root);

#endif
