/* file nd_netobj.c
 * implemention net object header 
 *
 * all right reserved by neil duan
 * 2009-5-8 23:04 
 */

#define ND_IMPLEMENT_HANDLE
typedef struct netui_info *nd_handle;

//#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
//#include "nd_common/nd_alloc.h"

int net_init_sendlock(nd_netui_handle socket_node)
{
	socket_node->send_lock=malloc(sizeof(nd_mutex )) ;
	if(!socket_node->send_lock)
		return -1 ;
	if(-1==nd_mutex_init(socket_node->send_lock)){
		free(socket_node->send_lock) ;
		socket_node->send_lock = NULL ;
		return -1 ;
	}
	return 0 ;

}

void net_release_sendlock(nd_netui_handle  socket_node)
{
	if(socket_node->send_lock){
		nd_mutex_destroy(socket_node->send_lock) ;
		free(socket_node->send_lock) ;
		socket_node->send_lock = NULL ;
	}
}

//绑定到指定的IP
int nd_net_ipbind(nd_handle net_handle, ndip_t ip) 
{
	struct nd_netsocket *node =(struct nd_netsocket *)net_handle ;
	node->bindip = ip ;
	if(node->fd) {
		SOCKADDR_IN self ;
		self.sin_family = AF_INET;
		if (node->sock_type==SOCK_RAW)
			self.sin_port = 0;
		else 
			self.sin_port = node->port;
		self.sin_addr.s_addr = ip ;
		if(-1==bind(node->fd, (SOCKADDR *)&self, sizeof(self) ) ) {
			nd_showerror();
	 		return -1 ;
	 	}
	}
	return 0;
		
}

int nd_net_bind(int port, int listen_nums,nd_handle net_handle) 
{
	ndsocket_t fd;
	struct nd_netsocket *node = (struct nd_netsocket *)net_handle ;
	SOCKADDR_IN *remote = NULL;

	nd_assert(node) ;
	node->myerrno = NDERR_SUCCESS ;

	fd = nd_socket_openport(port,node->sock_type, node->sock_protocol, node->bindip, listen_nums) ;
	
	if(-1==fd){
		nd_logfatal("open port %s" AND nd_last_error()) ;
		node->myerrno = NDERR_OPENFILE ;
		return -1 ;
	}
	if(0==port && SOCK_DGRAM==node->sock_type) {
		short tmp = nd_sock_getport(fd) ;
		port =(int) ntohs(tmp); 
	}

	node->fd = fd ;
	node->port = htons(port) ;
	node->status = 1 ;
	return 0 ;

}


int nd_net_sendto(nd_handle node,void *data , size_t len,SOCKADDR_IN *to) 
{
	return sendto(((struct nd_netsocket*)node)->fd, data,(int) len,0,(LPSOCKADDR)to, (int) sizeof(*to)) ;
}


int nd_net_ioctl(nd_netui_handle  socket_node, int cmd, void *val, int *size) 
{
	int ret = -1 ;
	NDUINT32 inputVal ;
	switch(cmd)
	{
	case NDIOCTL_SET_SENDVBUF:
		if (*size <4){
			return -1 ;
		}
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			if (!socket_node->is_session){
				inputVal = *(NDUINT32*)val ;
				ret = ndlbuf_realloc (&socket_node->send_buffer,inputVal) ;
			}
		}
		break ;
	case NDIOCTL_GET_SENDBUF:
		if (*size <4){
			return -1 ;
		}
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			if (!socket_node->is_session){
				*(NDUINT32*)val = (NDUINT32)ndlbuf_capacity(&socket_node->send_buffer) ;
				*size = sizeof(NDUINT32) ;
				ret = 0;
			}
		}
		break ;
	case NDIOCTL_SET_RECVVBUF:
		if (*size <4){
			return -1 ;
		}
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			if (!socket_node->is_session){
				inputVal = *(NDUINT32*)val ;
				ret = ndlbuf_realloc (&socket_node->recv_buffer,inputVal) ;
			}
		}
		break ;

	case NDIOCTL_GET_RECVBUF:
		if (*size <4){
			return -1 ;
		}
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			if (!socket_node->is_session){
				*(NDUINT32*)val = (NDUINT32)ndlbuf_capacity(&socket_node->recv_buffer) ;
				*size = sizeof(NDUINT32) ;
				ret = 0;
			}
		}
		break ;
	case NDIOCTL_SET_TCP_RECV_WNDSIZE:
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			if (socket_node->fd){
				ret = setsockopt(socket_node->fd, SOL_SOCKET, SO_RCVBUF, (sock_opval_t)val, *size) ;
			}		
		}
		break ;
	case NDIOCTL_GET_TCP_RECV_WNDSIZE:
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			if (socket_node->fd){
				inputVal = *(NDUINT32*)val ;
				ret = getsockopt(socket_node->fd, SOL_SOCKET, SO_RCVBUF, (sock_opval_t)val, size) ;
			}		
		}
		break ;
	case NDIOCTL_SET_TCP_SEND_WNDSIZE:
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			if (socket_node->fd){
				inputVal = *(NDUINT32*)val ;
				ret = setsockopt(socket_node->fd, SOL_SOCKET, SO_SNDBUF,(sock_opval_t)val, *size) ;
			}		
		}
		break ;
	case NDIOCTL_GET_TCP_SEND_WNDSIZE:
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			if (socket_node->fd){
				inputVal = *(NDUINT32*)val ;
				ret = getsockopt(socket_node->fd, SOL_SOCKET, SO_SNDBUF, (sock_opval_t)val, size) ;
			}		
		}
		break ;
	case NDIOCTL_SET_TIMEOUT:
		socket_node->disconn_timeout = *(ndtime_t*)val ;
		break;
	case NDIOCTL_GET_TIMEOUT:
		*(ndtime_t*)val  = socket_node->disconn_timeout ;
		break ;
    case NDIOCTL_GET_LAST_RECV_TIME:
        *(ndtime_t*)val  = socket_node->last_recv ;
        break ;
        
    case NDIOCTL_GET_LAST_SEND_TIME:
        *(ndtime_t*)val  = socket_node->last_push ;
        break ;
        
    case NDIOCTL_GET_USERDATA:
        *(nd_userdata_t*)val  = socket_node->user_data ;
        break ;
    case NDIOCTL_SET_USERDATA:
        socket_node->user_data = val;
        break ;
        
        ///
        
    case NDIOCTL_GET_PEER_IP:
        *(ndip_t*) val = nd_net_peer_getip(socket_node);
        break;
        
    case NDIOCTL_GET_PEER_PORT:
        *(ndport_t*) val = nd_net_peer_getport(socket_node);
        break;

        
    case NDIOCTL_HOOK_DATA:
        nd_hook_data(socket_node,(data_in_entry) val ) ;
        break ;
        
    case NDIOCTL_HOOK_PACKET:
        nd_hook_packet(socket_node,(net_msg_entry) val ) ;
        break ;
        
    case NDIOCTL_SET_UNREG_MSG_CLOSED: //close peers when received unregister message
        nd_net_set_unregmsg_handler(socket_node, *(int*)val);

        break ;
    case NDIOCTL_SET_UNAUTHORIZE_CLOSE:
        nd_net_set_unauthorize_handler(socket_node, *(int*)val);
        break ;
        

    case NDIOCTL_SET_CRYPT_KEY:
        nd_connector_set_crypt(socket_node,val, *size);
        break ;
    case NDIOCTL_GET_CRYPT_KEY:
        *(nd_userdata_t*)val = nd_connector_get_crypt(socket_node, size) ;
        break;
        
        
	case NDIOCTL_SET_BLOCK:
		ret = nd_socket_nonblock(socket_node->fd, 0) ;
		break ;
	case NDIOCTL_GET_BLOCK:
		ret = nd_socket_nonblock(socket_node->fd, 1) ;
		break ;

	case NDIOCTL_GET_RECV_PACK_NUM:
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			*(NDUINT32*)val = (NDUINT32) nd_atomic_read(&socket_node->recv_pack_times) ;
			*size = sizeof(NDUINT32) ;
			ret = 0 ;
		}
		break ;
	case NDIOCTL_SET_RECV_PACK_NUM:
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			inputVal = *(NDUINT32*)val ;
			nd_atomic_set(&socket_node->recv_pack_times, inputVal );
			ret = 0 ;
		}
		break ;
	case NDIOCTL_GET_SEND_PACK_NUM:
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			*(NDUINT32*)val = (NDUINT32) nd_atomic_read(&socket_node->send_pack_times );
			*size = sizeof(NDUINT32) ;
			ret = 0 ;
		}
		break ;
	case NDIOCTL_SET_SEND_PACK_NUM:
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			inputVal = *(NDUINT32*)val ;
			nd_atomic_set(&socket_node->send_pack_times, inputVal );
			ret = 0 ;
		}
		break ;
	default :
		ret =-1 ;
		break ;
	}
	return ret ;
}


void nd_connector_set_crypt(nd_netui_handle net_handle, void *key, int size)
{
	if(net_handle->type==NDHANDLE_TCPNODE || net_handle->type==NDHANDLE_UDPNODE){
		if(size>0 && size<=CRYPT_KEK_SIZE && key) {
			net_handle->crypt_key.size = size ;
			memcpy(&net_handle->crypt_key.key, key, size) ;
		}
	}
}
void* nd_connector_get_crypt(nd_handle net_handle, int *size)
{
	if (net_handle->crypt_key.size)	{
		*size = net_handle->crypt_key.size ;
		return net_handle->crypt_key.key;
	}
	*size =0;
	return NULL ;
}

int nd_connector_check_crypt(nd_netui_handle net_handle)
{	
	if(net_handle->type==NDHANDLE_TCPNODE || net_handle->type==NDHANDLE_UDPNODE){

		return net_handle->crypt_key.size ? 1:0 ;
	}
	return 0;
}

//get send buf free space
size_t nd_connector_sendlen(nd_netui_handle net_handle)
{
	return ndlbuf_freespace(&net_handle->send_buffer);
}


int nd_connector_set_timeout(nd_netui_handle net_handle, int seconds) 
{
	int ret = net_handle->disconn_timeout ;
	net_handle->disconn_timeout = seconds * 1000 ;
	return ret ;
}

ndip_t nd_net_peer_getip(nd_handle h)
{
	if (h->type ==NDHANDLE_TCPNODE ||  h->type==NDHANDLE_UDPNODE){
		nd_netui_handle handle = (nd_netui_handle)h ;
		return (ndip_t)handle->remote_addr.sin_addr.s_addr ;
	}
	return 0 ;
}
ndport_t nd_net_peer_getport(nd_handle h)
{
	if (h->type ==NDHANDLE_TCPNODE ||  h->type==NDHANDLE_UDPNODE){
		nd_netui_handle handle = (nd_netui_handle)h ;
		return (ndport_t)handle->remote_addr.sin_port ;
 	}
	return 0 ;
}


//设置网络数据处理函数
data_in_entry nd_hook_data(nd_handle h, data_in_entry data_entry) 
{
	data_in_entry ret = NULL ;
	if(h->type==NDHANDLE_TCPNODE || h->type==NDHANDLE_UDPNODE){
		ret = ((struct netui_info *)h)->data_entry ;
		((struct netui_info *)h)->data_entry = data_entry ;
	}
	else if(h->type==NDHANDLE_LISTEN){
		ret = ((struct nd_srv_node *)h)->data_entry ;
		((struct nd_srv_node *)h)->data_entry = data_entry ;
	}
	return ret ;
}

//设置ND格式的封包处理函数
net_msg_entry nd_hook_packet(nd_handle h, net_msg_entry msg_entry) 
{
	net_msg_entry ret = NULL ;
	if(h->type==NDHANDLE_TCPNODE || h->type==NDHANDLE_UDPNODE){
		ret = ((struct netui_info *)h)->msg_entry ;
		((struct netui_info *)h)->msg_entry = msg_entry ;
	}
	else if(h->type==NDHANDLE_LISTEN){
		ret = ((struct nd_srv_node *)h)->msg_entry ;
		((struct nd_srv_node *)h)->msg_entry = msg_entry ;
	}
	return ret ;

}

//设置当接收到没有注册的消息时是否关闭连接
void nd_net_set_unregmsg_handler(nd_handle h, int isclosed) 
{
	((nd_netui_handle)h)->unreg_msg_close = isclosed ? 1: 0 ;
}
//设置当接收到没有授权的消息时是否关闭连接
void nd_net_set_unauthorize_handler(nd_handle h, int isclosed) 
{
	((nd_netui_handle)h)->error_privilage_close = isclosed ? 1: 0 ;
}

void nd_connector_set_userdata(nd_netui_handle net_handle, void *p)
{
	net_handle->user_data = p ;
}

void* nd_connector_get_userdata(nd_netui_handle net_handle)
{
	return net_handle->user_data ;
}

#undef ND_IMPLEMENT_HANDLE