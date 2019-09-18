/* file : srv_netio.c
 * impelemention of nd-server net io interface
 * 
 * version 1.0
 * all right reserved by neil duan 
 * 2008-4
 */

#define ND_IMPLEMENT_HANDLE
typedef struct netui_info *nd_handle;

#include "nd_srvcore/nd_srvlib.h"
#if !defined(ND_UNIX) 
#include "../win_iocp/nd_iocp.h"
#endif 


/*deal with received net message
 * return -1 connect closed
 * return 0 nothing to be done
 * else return received data length
 */
int tcp_session_do_msg(struct nd_session_tcp *cli_map, struct nd_srv_node *srv_node)
{
	ENTER_FUNC()
		int read_len, ret = 0;
	nd_assert(cli_map);
	nd_assert(check_connect_valid(&(cli_map->connect_node)));
	if (!nd_handle_checkvalid((nd_handle)cli_map, NDHANDLE_TCPNODE)) {
		LEAVE_FUNC();
		return -1;
	}

	if (NDERR_USER_BREAK == cli_map->connect_node.myerrno) {
		//用户需要暂停消息处理
		LEAVE_FUNC();
		return 0;
	}

	cli_map->connect_node.myerrno = NDERR_SUCCESS;

RE_READ:
	read_len = nd_tcpnode_read(&(cli_map->connect_node));

	if (-1 == read_len) {
		if (cli_map->connect_node.myerrno == NDERR_WOULD_BLOCK) {
			LEAVE_FUNC();
			return 0;
		}
		else {
			LEAVE_FUNC();
			return -1;		//need closed
		}
	}
	else if (read_len > 0) {

		ret += read_len;
		if (-1 == handle_recv_data((nd_netui_handle)cli_map, (nd_handle)srv_node)) {
			LEAVE_FUNC();
			return -1;
		}
		if (TCPNODE_READ_AGAIN(&(cli_map->connect_node)) && cli_map->connect_node.myerrno == NDERR_SUCCESS) {
			/*read buf is to small , after parse data , read again*/
			goto RE_READ;
		}

	}
	if (NDERR_USER_BREAK == cli_map->connect_node.myerrno) {
		cli_map->connect_node.myerrno = NDERR_SUCCESS;
	}
	LEAVE_FUNC();
	return ret;

}


int nd_session_tcp_close(struct nd_session_tcp* cli_map, int force)
{	
	ENTER_FUNC()
	int ret = 0 ;
	NDUINT16  session = cli_map->connect_node.session_id ;
	struct nd_srv_node *root = (struct nd_srv_node *) (cli_map->connect_node.srv_root);
	struct thread_pool_info *thpi ;
	if (TCPNODE_STATUS(cli_map) == ETS_DEAD ) {
		LEAVE_FUNC();
		return 0 ;
	}
	
	if(root->connect_out_callback)	
		root->connect_out_callback(cli_map,(nd_handle)root) ;
	nd_tcpnode_close(&(cli_map->connect_node) , force) ;		//关闭socket
	
	thpi = nd_thpool_get_info((nd_listen_handle) root, 0) ;
	if (thpi) {
		delfrom_thread_pool(cli_map, thpi) ;
	}
	ret = root->conn_manager.deaccept (&root->conn_manager, session) ;
	if(0!=ret) {
		TCPNODE_STATUS(cli_map) = ETS_DEAD ;
		LEAVE_FUNC();
		return ret ;
	}
	nd_session_tcp_destroy(cli_map) ;
	
	root->conn_manager.dealloc (cli_map, nd_srv_get_allocator(root));
	LEAVE_FUNC();
	return 0 ;
}

int nd_session_close(nd_handle cli_handle, int force)
{
	int ret ;
	ENTER_FUNC()
//	nd_log_screen("close)
	nd_assert(cli_handle && cli_handle->close_entry) ;
	if(cli_handle->myerrno==NDERR_SUCCESS) {
		cli_handle->myerrno = NDERR_CLOSED ;
	}

	nd_unreg_handle(cli_handle) ;
	
	_nd_object_on_destroy(cli_handle, 1) ;
	
	ret = cli_handle->close_entry(cli_handle, force) ;
	
	LEAVE_FUNC();
	return ret ;
}

int nd_session_closeex(NDUINT16 session_id,nd_handle listen_handle)
{
	ndthread_t thid ;
	struct listen_contex *lc = (struct listen_contex*)listen_handle ;
	struct cm_manager *pmanger  ;

	pmanger = nd_listener_get_session_mgr((nd_listen_handle)lc) ;
	if (!pmanger){
		return -1 ;
	}
	thid = nd_node_get_owner(pmanger,session_id) ;
	if (thid == nd_thread_self() ) {
		nd_handle client = (nd_handle) pmanger->lock(pmanger,session_id) ;
		if(!client) 
			return -1 ;		
		nd_session_close(client,0) ;
		pmanger->unlock(pmanger,session_id);
	}
	else {
		nd_thsrv_send(thid,E_THMSGID_CLOSE_SESSION,&session_id, sizeof(session_id)) ;
	}	
	return 0 ;
}

int nd_session_flush_sendbuf(nd_handle cli_handle, int flag)
{
	ENTER_FUNC()
	int ret = 0 ;
	nd_netui_handle h_header =(nd_netui_handle)cli_handle ;
	nd_assert(h_header) ;
	h_header->myerrno = NDERR_SUCCESS ;
	if(h_header->type==NDHANDLE_TCPNODE){
		struct nd_session_tcp *cli_map =(struct nd_session_tcp *)cli_handle;
		if(0== flag) 
			ret = _tcpnode_push_sendbuf(&(cli_map->connect_node)) ;
		else if(1==flag)
			ret = _tcpnode_push_force(&(cli_map->connect_node)) ;
		else if(2==flag)
			ret = nd_tcpnode_tryto_flush_sendbuf(&(cli_map->connect_node)) ;
		
	}

	LEAVE_FUNC();
	return ret ;
	
}

int check_operate_timeout(nd_handle nethandle, ndtime_t tmout)
{

	struct listen_contex *lc ;
	ndtime_t interval = nd_time() - nethandle->last_recv;
	if(interval > tmout) {
		return 1 ;
	}

	//禁止空连接
	lc =(struct listen_contex *) nethandle->srv_root ;
	if (lc && lc->empty_conn_timeout && nethandle->level <= EPL_CONNECT){
		interval = nd_time() - nethandle->start_time;
		if(interval > lc->empty_conn_timeout) {
			return 1 ;
		}
	}
	return 0 ;
}

//return 0 nothing to be done 
// -1 nethandle closed and node freed 
// else socket in error wait to close next time
int tryto_close_tcpsession(nd_handle nethandle, ndtime_t connect_tmout )
{
	ENTER_FUNC()
	int ret = 0 ;
	if(TCPNODE_STATUS(nethandle)==ETS_DEAD) {
		tcp_release_death_node(nethandle,0) ;		//释放死连接
	}
	else if(TCPNODE_CHECK_RESET(nethandle)) {
		nd_session_close(nethandle,1) ;
	}
	else if(TCPNODE_CHECK_CLOSED(nethandle)) {
		nd_session_close(nethandle,0) ;
	}
	else if(check_operate_timeout(nethandle, connect_tmout)) {
		nd_object_seterror(nethandle, NDERR_TIMEOUT) ;
		nd_session_close(nethandle,0) ;
	}
	else {
		ret = 1;
	}
	LEAVE_FUNC();
	return ret ;
}



int _tcp_session_update(nd_handle handle)
{
	int ret = _tcpnode_push_sendbuf((struct nd_tcp_node *)handle);
	if (ret <= 0) {
		if (nd_netobj_is_alive((nd_netui_handle)handle)) {
			ndtime_t now = nd_time();
			TCPNODE_TRY_CALLBACK_WRITE(handle);
			if (now - handle->last_push > ND_ALIVE_TIMEOUT) {
				nd_sysresv_pack_t alive;
				nd_make_alive_pack(&alive);
				ret = nd_connector_send(handle, &alive.hdr, ESF_URGENCY);
			}
		}
	}
	return ret ;
}



#define INIT_SESSION_BUFF(session) \
	(session)->connect_node.send_buffer.is_alloced = 0 ;	\
	(session)->connect_node.recv_buffer.is_alloced = 0 ;	\
	(session)->connect_node.send_buffer.__buf = (session)->__sendbuf ;	\
	(session)->connect_node.recv_buffer.__buf = (session)->__recvbuf ;	\
	(session)->connect_node.send_buffer.buf_capacity = sizeof((session)->__sendbuf );	\
	(session)->connect_node.recv_buffer.buf_capacity = sizeof((session)->__recvbuf );	\
	ndlbuf_reset(&((session)->connect_node.send_buffer)) ;				\
	ndlbuf_reset(&((session)->connect_node.recv_buffer)) 

void nd_session_tcp_init(struct nd_session_tcp *client_map, nd_handle h_listen)
{
	memset(client_map, 0, sizeof(struct nd_session_tcp));
	_tcp_connector_init(&(client_map->connect_node));
	//init buff
	INIT_SESSION_BUFF(client_map);

	INIT_LIST_HEAD(&(client_map->map_list));
	client_map->connect_node.is_session = 1;
	client_map->connect_node.size = sizeof(struct nd_session_tcp);

	client_map->connect_node.disconn_timeout = ((struct listen_contex*)h_listen)->operate_timeout;
	client_map->connect_node.close_entry = (nd_close_callback)nd_session_tcp_close;

	client_map->connect_node.msg_entry = ((struct nd_srv_node*)h_listen)->msg_entry;

	client_map->connect_node.data_entry = ((struct nd_srv_node*)h_listen)->data_entry;
	client_map->connect_node.update_entry = (net_update_entry)_tcp_session_update;
}
void nd_session_tcp_destroy(struct nd_session_tcp *client_map)
{
	ndlbuf_destroy(&client_map->connect_node.send_buffer);
	ndlbuf_destroy(&client_map->connect_node.recv_buffer);
}

void nd_session_udt_init(struct nd_session_udt *node, nd_handle h_listen)
{
	memset(node, 0, sizeof(*node));
	_udt_connector_init(&node->connect_node);

	INIT_SESSION_BUFF(node);

	INIT_LIST_HEAD(&(node->map_list));
	node->connect_node.is_session = 1;
	node->connect_node.size = sizeof(struct nd_session_udt);

	node->connect_node.disconn_timeout = ((struct listen_contex*)h_listen)->operate_timeout;
	node->connect_node.close_entry = (nd_close_callback)udt_close;

	node->connect_node.data_entry = ((struct nd_srv_node*)h_listen)->data_entry;

	node->connect_node.msg_entry = ((struct nd_srv_node*)h_listen)->msg_entry;
	//node->connect_node.update_entry = (net_update_entry)_tcp_session_update ;
}

/* get client header size*/
size_t nd_session_hdr_size(int iomod)
{
	size_t ret;
	switch (iomod)
	{
		//case ND_LISTEN_UDT_DATAGRAM:
	case ND_LISTEN_UDT_STREAM:
		ret = sizeof(struct nd_session_udt);
		break;
	case ND_LISTEN_OS_EXT:
#if !defined(ND_UNIX)
		ret = sizeof(struct nd_session_iocp);
		break;
#endif
	case 	ND_LISTEN_COMMON:
	default:
		ret = sizeof(struct nd_session_tcp);
		break; ;
	}
	return ret;
}


void *nd_session_getdata(nd_handle session)
{
	size_t size = session->size;
	if (size & 7) {
		size += 8;
		size &= ~7;
	}
#ifdef ND_DEBUG
	//nd_assert(nd_session_valid(session)) ;
#endif
	return (void*)(((char*)session) + size);
}
