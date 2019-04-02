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
//extern int udt_listen_callback(struct listen_contex *listen_info) ;

//int create_udp_thpool(struct listen_contex *listen_info,udp_entry data_entry);
int read_datagram(struct nd_netsocket *node, char *buf, size_t buf_size, SOCKADDR_IN *addr );

static int update_udt_sessions(struct cm_manager *pmanger, struct nd_netth_context *thpi);

#define UDP_THREAD_NUM 4 

struct udp_thpool 
{
	volatile int		 used ;
	volatile int		 isexit;
	nd_mutex fd_lock ;
	nd_cond  fd_cond ;
	nd_thsrvid_t thpool_ids[UDP_THREAD_NUM] ;
};



int udt_session_data_handle(nd_udt_node *socket_node, struct udt_packet_info* pack_buf)
{
	int ret = 0;
	UDT_RECV_USER_DATA(socket_node) = 0;
	if (socket_node->check_entry(socket_node, &pack_buf->packet.pocket, pack_buf->data_len,(SOCKADDR_IN*)&pack_buf->addr)) {
		ret = _udt_packet_handler(socket_node, &pack_buf->packet, pack_buf->data_len);
		if (-1 == ret) {
			release_dead_node(socket_node, 1);
		}
		else if (UDT_RECV_USER_DATA(socket_node)) {
			handle_recv_data((nd_netui_handle)socket_node, (nd_handle)socket_node->srv_root);
		}
	}
	return ret;
}

static int data_handle(nd_handle hsrv, struct udt_packet_info* pack_buf)
{
	NDUINT16 localport = pack_buf->packet.pocket.local_port;
	int ret = 0;
	struct cm_manager *pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)hsrv);
	nd_udt_node *socket_node = pmanger->lock(pmanger, localport);
	
	if (socket_node) {		
		ret = udt_session_data_handle(socket_node, pack_buf);
	}
	else {
		ndthread_t  thid = nd_node_get_owner(pmanger, localport);
		if (0 == thid || thid == nd_thread_self()) {
			return 0;
		}
		int len = _udt_packet_info_size(pack_buf);
		ret = nd_thsrv_send(thid, E_THMSGID_SEND_UDP_DATA, pack_buf, len); 
	} 

	return ret;
}

int _utd_main_thread(struct thread_pool_info *thip)
{
	ENTER_FUNC();
	int ret;
	struct cm_manager *pmanger;
	struct listen_contex *listen_info = (struct listen_contex *)thip->lh;

	struct udt_packet_info pack_buf;

	nd_handle context = nd_thsrv_gethandle(0);
	nd_assert(context);
	pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)listen_info);

	listen_info->udt.data_proc = data_handle;

	while (!nd_thsrv_isexit(context)) {
		int sleep = LISTEN_INTERVAL;
		int msgret;
		if (listen_info->pre_update) {
			listen_info->pre_update((nd_handle)listen_info, context);
		}
		msgret = nd_thsrv_msghandler(context);
		if (-1 == msgret) {		//处理线程消息
			if (listen_info->end_update) {
				listen_info->end_update((nd_handle)listen_info, context);
			}
			break;
		}
		else if (msgret)
			sleep = 0;

		ret = 0;
		if (sleep) {
			if (nd_socket_wait_read(listen_info->udt.base.fd, 30) > 0) {
				ret = read_datagram((struct nd_netsocket *)listen_info,
					(char*)&pack_buf.packet, sizeof(pack_buf.packet), &pack_buf.addr);
			}
		}
		else {

			ret = read_datagram((struct nd_netsocket *)listen_info,
				(char*)&pack_buf.packet, sizeof(pack_buf.packet), &pack_buf.addr);

		}

		if (ret > 0) {
			//udt_data_handler(&addr, readbuf, ret, (nd_handle)listen_info);
			pack_buf.data_len = ret;
			pump_insrv_udt_data((nd_handle)listen_info, &pack_buf);
		}		

		if (update_udt_sessions(pmanger, thip) > 0) {
			sleep = 0;
		}

		if (listen_info->end_update) {
			listen_info->end_update((nd_handle)listen_info, context);
		}
	}
	LEAVE_FUNC();
	return 0;
}

/* entry of  listen service */
int _udt_sub_thread(struct thread_pool_info *thip)
{
	ENTER_FUNC();
	struct cm_manager *pmanger;
	struct listen_contex *listen_info = (struct listen_contex *)thip->lh;

	nd_handle context = nd_thsrv_gethandle(0);
	nd_assert(context);
	pmanger = nd_listensrv_get_cmmamager((nd_listen_handle)listen_info);

	while (!nd_thsrv_isexit(context)) {
		int sleep = LISTEN_INTERVAL;
		int msgret;
		if (listen_info->pre_update) {
			listen_info->pre_update((nd_handle)listen_info, context);
		}
		msgret = nd_thsrv_msghandler(context);
		if (-1 == msgret) {		//处理线程消息
			if (listen_info->end_update) {
				listen_info->end_update((nd_handle)listen_info, context);
			}
			break;
		}
		else if (msgret)
			sleep = 0;

		if (update_udt_sessions(pmanger, thip) > 0) {
			sleep = 0;
		}
		if (thip->connector_hub) {
			if (update_connectors(thip->connector_hub) > 0)
				sleep = 0;
		}

		if (listen_info->end_update) {
			listen_info->end_update((nd_handle)listen_info, context);
		}
		if (sleep)
			nd_sleep(sleep);
	}
	//close_session_in_thread(thip) ;
	LEAVE_FUNC();
	return 0;
}


int update_udt_sessions(struct cm_manager *pmanger, struct nd_netth_context *thpi)
{
	ENTER_FUNC();
	struct list_head *pos, *next;
	struct listen_contex *lc = (struct listen_contex *)(thpi->lh);

	list_for_each_safe(pos, next, &thpi->sessions_list) {
		struct nd_udtcli_map  *client = list_entry(pos, struct nd_client_map, map_list);

		if (!client || !nd_handle_checkvalid((nd_handle)client, NDHANDLE_TCPNODE)) {
			nd_session_close((nd_handle)client, 1);
			continue;
		}
		update_udt_session((nd_udt_node*)client);
	}

	LEAVE_FUNC();
	return 0;

}



int read_datagram(struct nd_netsocket *node, char *buf, size_t buf_size, SOCKADDR_IN *addr )
{
	ENTER_FUNC();
	int len = nd_socket_udp_read(node->fd, buf,buf_size, addr);

	if (len <= 0) {
		LEAVE_FUNC();
		return len;
	}

	if (!_test_checksum((struct ndudt_header*)buf, len)) {
		node->myerrno = NDERR_BADPACKET;
		LEAVE_FUNC();
		return 0;
	}
	udt_net2host((struct ndudt_header*)buf);

	if (POCKET_PROTOCOL(((struct ndudt_pocket*)buf)) == PROTOCOL_UDT) {
		LEAVE_FUNC();
		return len;
	}
	else {
		LEAVE_FUNC();
		return 0;
	}
}
