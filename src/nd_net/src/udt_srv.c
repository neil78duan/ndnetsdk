/* file : udt_server
 * implete UDP data transfer
 * server program socket bind accept listen
 * neil 
 * 2007-11-17
 * all right reserved by neil duan
 */

//#include "nd_common/nd_common.h"
//#include "nd_common/nd_alloc.h"
#include "nd_net/nd_netlib.h"

int udt_send_fin(nd_udt_node *socket_node);
nd_udt_node *alloc_listen_socket(struct nd_srv_node *root);

//关闭服务器端的socket
int _close_listend_socket(nd_udt_node* socket_node, int force)
{
	struct nd_srv_node *root = (struct nd_srv_node *) socket_node->srv_root;
	nd_assert(socket_node->is_accept);

	if (force || ((NETSTAT_ESTABLISHED & socket_node->status) &&
		!(socket_node->status & NETSTAT_SENDCLOSE))) {
		udt_send_fin(socket_node);
	}
	else {
		udt_reset(socket_node, 1);
	}
	
	if (root && socket_node->local_port) {

		if (root->connect_out_callback && socket_node->status & NETSTAT_ESTABLISHED) {
			root->connect_out_callback(socket_node, (nd_handle)root);
			socket_node->status &= ~NETSTAT_ESTABLISHED;
		}
	}
	return 0;
}

//更新每一个节点
// return -1 ,closed , source already release
int update_udt_session(nd_udt_node *node)
{	
	if(NETSTAT_ESTABLISHED==node->status){
		if (-1==update_socket(node)) {
			udt_reset(node,0) ;
			//nd_assert(0) ;
			return 0 ;
		}
	}
	
	else if(node->status<NETSTAT_ESTABLISHED ){
		//not accepted 
		ndtime_t val = nd_time() - node->last_recv ;
		if(val > node->retrans_timeout) {
			udt_reset(node,0) ;
			return 0 ;		
		}	
	}
	
	else if(NETSTAT_RESET & node->status) {
		return -1 ;
	}
	

	else if(NETSTAT_FINSEND & node->status &&  !(NETSTAT_SENDCLOSE&node->status)) {
		if(node->resend_times<2) {
			ndtime_t now = nd_time() ;
			if((now - node->last_resend_tm) > node->retrans_timeout) {
				udt_send_fin(node);
			}
		}
		else {
			return -1 ;
		}
	}
	
	return 0;
}


//处理进入服务器的消息
int pump_insrv_udt_data(nd_udtsrv *root, struct udt_packet_info *pack_buf)
{
	ENTER_FUNC();
	int ret = 0; 
	int len = pack_buf->data_len;
	SOCKADDR_IN *addr =(SOCKADDR_IN *) &pack_buf->addr;
	struct ndudt_pocket *pocket = &pack_buf->packet.pocket;
		
	u_16 localport = pocket->local_port;

	if (NDUDT_SYN == POCKET_TYPE(pocket)) {
		nd_udt_node *socket_node = NULL;
		if(0==localport) {
			socket_node = alloc_listen_socket(&root->base) ;
			if(!socket_node){
				LEAVE_FUNC();
				return 0 ;
			}
			socket_node->local_port = root->base.conn_manager.accept(&root->base.conn_manager,socket_node);

			if(!socket_node->local_port) {
				release_dead_node(socket_node, 1) ;
				LEAVE_FUNC();
				return 0 ;
			}
			
			nd_assert(socket_node->status==NETSTAT_LISTEN);
			memcpy(&socket_node->remote_addr,addr,sizeof(*addr)) ;
			socket_node->last_recv = nd_time() ;
			socket_node->update_tm = socket_node->last_recv ;
			localport = socket_node->local_port;
			socket_node->session_id = socket_node->local_port;
		}
		else {
			socket_node = root->base.conn_manager.lock(&root->base.conn_manager,localport) ;
			if(!socket_node){
				ret = root->data_proc((nd_handle)root, pack_buf);
				LEAVE_FUNC();
				return ret;
			}
			socket_node->last_recv = nd_time() ;
			socket_node->update_tm = socket_node->last_recv ;
		}
		
		if(-1==_handle_syn(socket_node,pocket) ) {
			udt_reset( socket_node, 0) ;
			release_dead_node(socket_node,1) ;
			ret = -1;
		}

		socket_node->recv_len += len;
		root->base.conn_manager.unlock(&root->base.conn_manager, localport);

	}
	else {
		ret = root->data_proc((nd_handle)root, pack_buf);

	}
	LEAVE_FUNC();
	return ret ;
}
nd_udt_node *alloc_listen_socket(struct nd_srv_node *root)
{
	nd_udt_node* node ;
	nd_assert(root->conn_manager.alloc);
	node = root->conn_manager.alloc(nd_srv_get_allocator((struct nd_srv_node*)root)) ;
	if (!node){
		return NULL;
	}
	if(root->conn_manager.init ) {
		root->conn_manager.init(node,(nd_handle) root) ;
	}
	else {
		nd_udtnode_init(node);
	}

	node->close_entry = (nd_close_callback)_close_listend_socket;
	node->status = NETSTAT_LISTEN;
	node->is_accept = 1;
	node->srv_root =(nd_handle) root;
	node->fd = root->fd ;		//use same socket fd with root
	return node ;
}


//释放accept端一个已经关闭的连接
void release_dead_node(nd_udt_node *socket_node,int needcallback)
{
	struct nd_srv_node *root =(struct nd_srv_node *) socket_node->srv_root;
	struct cm_manager *cm = &root->conn_manager ;
	
	nd_assert(root && cm) ;
	if(!root || 0==socket_node->local_port)
		return ;

	if( root->connect_out_callback && socket_node->status & NETSTAT_ESTABLISHED) {
		root->connect_out_callback(socket_node,(nd_handle)root) ;
		socket_node->status &= ~NETSTAT_ESTABLISHED ;
	}

	cm->deaccept(cm, socket_node->local_port);
	
	_deinit_udt_socket(socket_node) ;
	cm->dealloc ((void*)socket_node,nd_srv_get_allocator((struct nd_srv_node*)root));

}
