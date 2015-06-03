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
nd_udt_node *alloc_listen_socket(nd_udtsrv *root);
int pump_insrv_udt_data(nd_udtsrv *root,struct ndudt_pocket *pocket, int len,SOCKADDR_IN *addr);

//更新每一个节点
// return -1 ,closed , source already release
static int update_udt_session(nd_udt_node *node)
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

	else if(NETSTAT_ESTABLISHED != node->status) {
		ndtime_t now = nd_time() ;
		if((now - node->last_resend_tm) > (node->retrans_timeout*2)) {
			return -1;
		}
	}
	
	return 0;
}

void update_all_socket(nd_udtsrv *root) 
{
	ENTER_FUNC()

	cmlist_iterator_t cm_iter;
	struct cm_manager *conn_manager  = &root->conn_manager ;
	nd_udt_node *node ;
	
	for(node = conn_manager->lock_first(conn_manager,&cm_iter) ; node; 
	node = conn_manager->lock_next(conn_manager,&cm_iter) ) {
		if(-1==update_udt_session(node) ) {
			release_dead_node(node,1) ;
		}
	} 

	LEAVE_FUNC();
}


//udt数据处理函数,listen 端用的
int udt_data_handler(SOCKADDR_IN *addr, struct ndudt_pocket*pocket, size_t read_len, nd_udtsrv *root) 
{
	NDUINT16 calc_cs,checksum;
	if(read_len > MAX_UDP_LEN){
		return -1 ;
	}

	udt_net2host(pocket) ;

	checksum = POCKET_CHECKSUM(pocket) ;	
	POCKET_CHECKSUM(pocket) = 0 ;

	calc_cs = nd_checksum((NDUINT16*)pocket,read_len) ;
	if(checksum!=calc_cs){
		return 0 ;
	}

	if(POCKET_PROTOCOL(pocket)==PROTOCOL_UDT) {
		return pump_insrv_udt_data(root, pocket, (int)read_len, addr);
		
	}
	else {
		nd_assert(0);
		return 0;		//incoming data error
	}
}


//处理进入服务器的消息
int pump_insrv_udt_data(nd_udtsrv *root,struct ndudt_pocket *pocket, int len,SOCKADDR_IN *addr)
{
	ENTER_FUNC()
	int ret = 0;
	nd_udt_node *socket_node = NULL;
	u_16 session_id = POCKET_SESSIONID(pocket) ;
	if(NDUDT_SYN==POCKET_TYPE(pocket)) {
		if(0==session_id) {
			socket_node = alloc_listen_socket(root) ;
			if(!socket_node){
				LEAVE_FUNC();
				return 0 ;
			}
			socket_node->session_id = root->conn_manager.accept(&root->conn_manager,socket_node);

			if(!socket_node->session_id) {
				release_dead_node(socket_node, 1) ;
				LEAVE_FUNC();
				return 0 ;
			}
			
			nd_assert(socket_node->status==NETSTAT_LISTEN);
			memcpy(&socket_node->remote_addr,addr,sizeof(*addr)) ;
			socket_node->last_recv = nd_time() ;
			socket_node->update_tm = socket_node->last_recv ;
			session_id = socket_node->session_id ;
		}
		else {
			socket_node = root->conn_manager.lock(&root->conn_manager,session_id) ;
			if(!socket_node){
				LEAVE_FUNC();
				return 0;
			}
			socket_node->last_recv = nd_time() ;
			socket_node->update_tm = socket_node->last_recv ;
		}
		
		if(-1==_handle_syn(socket_node,pocket) ) {
			udt_reset( socket_node, 0) ;
			release_dead_node(socket_node,1) ;
		}
	}
	else{
		socket_node = root->conn_manager.lock(&root->conn_manager,session_id) ;
		if(!socket_node){
			LEAVE_FUNC();
			return -1 ;
		}
		socket_node->last_recv = nd_time() ;		//record received data time 

		if(socket_node->check_entry(socket_node,pocket,len,addr) ) {
			ret = _udt_packet_handler(socket_node,pocket,len)  ;
		}  
		
		if(-1== ret){
			release_dead_node(socket_node,1) ;
		}
		else {
			socket_node->last_recv = nd_time() ;
			if(-1==update_socket(socket_node) )
				release_dead_node(socket_node,1) ;
		}

	}
	if(socket_node) {
		socket_node->recv_len += len ;
		root->conn_manager.unlock(&root->conn_manager,session_id) ;
	}
	LEAVE_FUNC();
	return ret ;
}

nd_udt_node *alloc_listen_socket(nd_udtsrv *root)
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
	else 
		nd_udtnode_init(node);

	node->status = NETSTAT_LISTEN;
	node->is_accept = 1;
	node->srv_root =(nd_handle) root;
	node->fd = root->fd ;		//use same socket fd with root
	return node ;
}


//释放accept端一个已经关闭的连接
void release_dead_node(nd_udt_node *socket_node,int needcallback)
{
	nd_udtsrv *root =(nd_udtsrv *) socket_node->srv_root ;
	struct cm_manager *cm = &root->conn_manager ;
	
	nd_assert(root && cm) ;
	if(!root || 0==socket_node->session_id)
		return ;

	if( root->connect_out_callback && socket_node->status & NETSTAT_ESTABLISHED) {
		root->connect_out_callback(socket_node,(nd_handle)root) ;
		socket_node->status &= ~NETSTAT_ESTABLISHED ;
	}

	cm->deaccept(cm, socket_node->session_id);
	
	_deinit_udt_socket(socket_node) ;
	cm->dealloc ((void*)socket_node,nd_srv_get_allocator((struct nd_srv_node*)root));

}
