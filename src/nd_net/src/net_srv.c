/* file :net_srv.c
 * release net service function
 * 
 * version 1.0 
 * all right reserved by neil 2007-10
 */

#include "nd_net/nd_netlib.h"



int nd_srv_capacity(struct nd_srv_node *srvnode) 
{
	return srvnode->conn_manager.capacity(&srvnode->conn_manager) ;
}

int cm_listen(struct cm_manager *root, int max_num, int client_size)
{
	client_size += 8 ;
	client_size &= ~7 ;
	
	return nd_node_create_ex(root, max_num, client_size, 1024, NULL);
}
void cm_destroy(struct cm_manager *root) 
{
	nd_node_destroy_ex(root) ;
}


void nd_srv_close(struct nd_srv_node *node)
{
	nd_assert(node) ;
	nd_socket_close(node->fd) ;
	node->status = 0 ;
	node->fd = 0 ;
}

#undef  GET_SESSION_ID
