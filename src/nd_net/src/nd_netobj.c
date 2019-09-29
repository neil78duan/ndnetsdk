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

static char *_s_send_stream =NULL;
static char *_s_recv_stream =NULL;

static int _min_packet_len = sizeof(nd_usermsghdr_t);

int nd_net_set_packet_minsize(int minsize)
{
	int ret = _min_packet_len;
	_min_packet_len = minsize;
	return ret;
}

void _release_send_stream()
{
	if (_s_send_stream) {
		free(_s_send_stream);
		_s_send_stream = 0;
	}

	if (_s_recv_stream) {
		free(_s_recv_stream);
		_s_recv_stream = 0;
	}

}
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

int nd_net_bind(int isipv6, int port, int listen_nums,nd_handle net_handle)
{
	ndsocket_t fd;
	struct nd_netsocket *node = (struct nd_netsocket *)net_handle ;
	//SOCKADDR_IN *remote = NULL;

	nd_assert(node) ;
	node->myerrno = NDERR_SUCCESS ;
	if (node->bindip[0]) {
		fd = nd_socket_openport(port, node->sock_type, node->sock_protocol, listen_nums, node->bindip);
	}
	else if (isipv6) {
		fd = nd_socket_openport_v6(port, node->sock_type, node->sock_protocol, listen_nums);
	} 
	else {
		fd = nd_socket_openport_v4(port, node->sock_type, node->sock_protocol, listen_nums);
	}
	
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
	nd_logmsg("Bind socket success PORT = %d\n", port);
	return 0 ;

}


int nd_net_sendto(nd_handle node,void *data , size_t len,SOCKADDR_IN *to) 
{
	return (int)sendto(((struct nd_netsocket*)node)->fd, data,(int) len,0,(LPSOCKADDR)to, (int) sizeof(*to)) ;
}

void* nd_net_object_caller(nd_handle node)
{
	int type = nd_object_get_type(node) ;
	if(type==NDHANDLE_TCPNODE || type==NDHANDLE_UDPNODE || type == NDHANDLE_LISTEN) {
		return node->msg_caller ;
	}
	return NULL ;
	
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
	case NDIOCTL_UNLIMITED_SEND_WNDSIZE:
		if (socket_node->type == NDHANDLE_TCPNODE || socket_node->type == NDHANDLE_UDPNODE) {
			if (*(NDUINT32*)val) {
				ndlbuf_auto_inc_enable(&socket_node->send_buffer);
			}
			else {
				ndlbuf_auto_inc_disable(&socket_node->send_buffer);
			}
			ret = 0;
		}
		break;
	case NDIOCTL_UNLIMITED_RECV_WNDSIZE:
		if (socket_node->type == NDHANDLE_TCPNODE || socket_node->type == NDHANDLE_UDPNODE) {
			if (*(NDUINT32*)val) {
				ndlbuf_auto_inc_enable(&socket_node->recv_buffer);
			}
			else {
				ndlbuf_auto_inc_disable(&socket_node->recv_buffer);
			}
			ret = 0;
		}
		break;

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
				ret = getsockopt(socket_node->fd, SOL_SOCKET, SO_RCVBUF, (sock_opval_t)val, (socklen_t*)size) ;
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
				ret = getsockopt(socket_node->fd, SOL_SOCKET, SO_SNDBUF, (sock_opval_t)val, (socklen_t*)size) ;
			}		
		}
		break ;
	case NDIOCTL_SET_TIMEOUT:
		socket_node->disconn_timeout = (*(ndtime_t*)val) *1000 ;
		ret = 0;
		break;
	case NDIOCTL_GET_TIMEOUT:
		*(ndtime_t*)val  = socket_node->disconn_timeout /1000 ;
		*size = sizeof(ndtime_t) ;
		ret = 0;
		break ;
    case NDIOCTL_GET_LAST_RECV_TIME:
		*(ndtime_t*)val  = socket_node->last_recv ;
		*size = sizeof(ndtime_t) ;
		ret = 0;
        break ;
        
    case NDIOCTL_GET_LAST_SEND_TIME:
		*(ndtime_t*)val  = socket_node->last_push ;
		*size = sizeof(ndtime_t) ;
		ret = 0;
        break ;
        
    case NDIOCTL_GET_USERDATA:
		*(nd_userdata_t*)val  = socket_node->user_data ;
		ret = 0;
        break ;
    case NDIOCTL_SET_USERDATA:
		socket_node->user_data = val;
		ret = 0;
        break ;
        
        ///
        
    case NDIOCTL_GET_PEER_IP:
		*(ndip_t*) val = nd_net_peer_getip(socket_node);
		ret = 0;
        break;
        
    case NDIOCTL_GET_PEER_PORT:
		*(ndport_t*) val = nd_net_peer_getport(socket_node);
		ret = 0;
        break;

        
    case NDIOCTL_HOOK_DATA:
		nd_hook_data(socket_node,(data_in_entry) val ) ;
		ret = 0;
        break ;
        
    case NDIOCTL_HOOK_PACKET:
		nd_hook_packet(socket_node,(net_msg_entry) val ) ;
		ret = 0;
        break ;
        
    case NDIOCTL_SET_UNREG_MSG_CLOSED: //close peers when received unregister message
		nd_net_set_unregmsg_handler(socket_node, *(int*)val);
		ret = 0;

        break ;
    case NDIOCTL_SET_UNAUTHORIZE_CLOSE:
		nd_net_set_unauthorize_handler(socket_node, *(int*)val);
		ret = 0;
        break ;
        

    case NDIOCTL_SET_CRYPT_KEY:
		nd_connector_set_crypt(socket_node,val, *size);
		ret = 0;
        break ;
    case NDIOCTL_GET_CRYPT_KEY:
		{
		int keysize = 0 ;
		void *addr = nd_connector_get_crypt(socket_node, &keysize) ;
		ret = -1 ;
		if (keysize <= *size) {
			memcpy(val, addr, keysize) ;
			*size = keysize ;
			ret = 0 ;
		}
		}
        break;
            
    case NDIOCTL_SET_LEVEL:
		socket_node->level = (NDUINT8) (*(int*)val ) ;
		ret = 0;
        break;
            
    case NDIOCTL_GET_LEVEL:
		*(int*)val = socket_node->level ;
		ret = 0;
        break;

	case NDIOCTL_LOG_SEND_MSG:
		if (*(int*)val){
			socket_node->is_log_send = 1;
		}
		else {
			socket_node->is_log_send = 0;
		}
		ret = 0;
		break;

	case NDIOCTL_LOG_RECV_MSG:
		if (*(int*)val){
			socket_node->is_log_recv = 1;
		}
		else {
			socket_node->is_log_recv = 0;
		}
		ret = 0;
		break;

	case NDIOCTL_SYS_SET_LOG_FUNC:
		nd_setlog_func((nd_log_entry)val);
		break;

	case NDIOCTL_SYS_GET_LOG_FUNC:
	{
		nd_log_entry oldval = nd_setlog_func((nd_log_entry)0);
		nd_log_entry *retval = (nd_log_entry *)val;
		*retval = oldval;
		nd_setlog_func(oldval);
	}
		break;

	case NDIOCTL_GET_SESSIONID:
		*(int*)val = socket_node->session_id ;
		ret = 0;
		break;
	case NDIOCTL_SET_SESSIONID:
		socket_node->session_id = (NDUINT16) (*(int*)val);
		ret = 0;
		break;

	case NDIOCTL_GET_WRITABLE_CALLBACK:
		*(net_writable_callback*)val = socket_node->writable_callback ;
		ret = 0 ;
		break ;
	case NDIOCTL_SET_WRITABLE_CALLBACK:
		socket_node->writable_callback =(net_writable_callback) *(nd_userdata_t*)val ;
		ret = 0 ;
		break ;
		
	case NDIOCTL_GET_WRITABLE_CALLBACK_PARAM:
		*(nd_userdata_t*)val = socket_node->writable_param ;
		ret = 0 ;
		break ;
	case NDIOCTL_SET_WRITABLE_CALLBACK_PARAM:
		socket_node->writable_param = *(nd_userdata_t*)val ;
		ret = 0 ;
		break ;
		

// 	case NDIOCTL_SET_BLOCK:
// 		ret = nd_socket_nonblock(socket_node->fd, 0) ;
// 		break ;
// 	case NDIOCTL_GET_BLOCK:
// 		ret = nd_socket_nonblock(socket_node->fd, 1) ;
// 		break ;

	case NDIOCTL_GET_RECV_PACK_NUM:
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			*(NDUINT32*)val = (NDUINT32) nd_atomic_read(&socket_node->recv_pack_times) ;
			*size = sizeof(NDUINT32) ;
			ret = 0;
		}
		break ;
	case NDIOCTL_SET_RECV_PACK_NUM:
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			inputVal = *(NDUINT32*)val ;
			nd_atomic_set(&socket_node->recv_pack_times, inputVal );
			ret = 0;
		}
		break ;
	case NDIOCTL_GET_SEND_PACK_NUM:
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			*(NDUINT32*)val = (NDUINT32) nd_atomic_read(&socket_node->send_pack_times );
			*size = sizeof(NDUINT32) ;
			ret = 0;
		}
		break ;
	case NDIOCTL_SET_SEND_PACK_NUM:
		if (socket_node->type ==NDHANDLE_TCPNODE|| socket_node->type ==NDHANDLE_UDPNODE ){
			inputVal = *(NDUINT32*)val ;
			nd_atomic_set(&socket_node->send_pack_times, inputVal );
			ret = 0;
		}
		break ;
			
			
			
	case NDIOCTL_LOG_SEND_STRAM_FILE:
			ret = -1 ;
			if (val) {
				socket_node->save_send_stream = 1 ;
				if (_s_send_stream) {
					free(_s_send_stream) ;
				}
				_s_send_stream = (char*) malloc(*size + 1) ;
				if (_s_send_stream) {
					ndstrncpy(_s_send_stream, (const char*)val, *size +1) ;
					ret =  0 ;
				}
			}
			else {
				socket_node->save_send_stream = 0;
				if (_s_send_stream) {
					free(_s_send_stream) ;
				}
				ret = 0 ;
			}
		break ;
	case NDIOCTL_LOG_RECV_STRAM_FILE:
			ret = -1 ;
			if (val) {
				socket_node->save_recv_stream = 1 ;
				if (_s_recv_stream) {
					free(_s_recv_stream) ;
				}
				_s_recv_stream = (char*) malloc(*size + 1) ;
				if (_s_recv_stream) {
					ndstrncpy(_s_recv_stream, (const char*)val, *size +1) ;
					ret =  0 ;
				}
			}
			else {
				socket_node->save_recv_stream = 0;
				if (_s_recv_stream) {
					free(_s_recv_stream) ;
				}
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
		else if (!key || size == 0) {
			net_handle->crypt_key.size = 0;
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

size_t nd_connector_data_in_window(nd_handle net_handle)
{
	return ndlbuf_datalen(&net_handle->send_buffer);
}

int nd_connector_set_timeout(nd_netui_handle net_handle, int seconds) 
{
	int ret = net_handle->disconn_timeout ;
	net_handle->disconn_timeout = seconds * 1000 ;
	return ret ;
}


ndip_t nd_net_getip(nd_handle h)
{
	ndip_t ip = { 0 };
	if (((struct nd_netsocket*)h)->fd) {
		return nd_sock_getip(((struct nd_netsocket*)h)->fd);
	}
	else if (h->type == NDHANDLE_UDPNODE) {
		nd_handle root = (nd_handle) h->srv_root;
		if (root) {
			return nd_net_getip(root);
		}

	}
	return ip;
}
ndport_t nd_net_getport(nd_handle h)
{
	if (((struct nd_netsocket*)h)->fd) {
		return nd_sock_getport(((struct nd_netsocket*)h)->fd);
	}
	else if (h->type == NDHANDLE_UDPNODE) {
		nd_handle root = (nd_handle)h->srv_root;
		if (root) {
			return nd_net_getport(root);
		}

	}
	return 0;
}

ndip_t nd_net_peer_getip(nd_handle h)
{
	if (h->type == NDHANDLE_TCPNODE && h->fd) {
		return nd_sock_getpeerip(h->fd);
	}
	else {
		ndip_t ret = ND_IP_INIT;
		nd_sockadd_to_ndip(&h->remote_addr, &ret);
		return ret;
	}
}
ndport_t nd_net_peer_getport(nd_handle h)
{
	if (h->type == NDHANDLE_TCPNODE && h->fd) {
		return nd_sock_getpeerport(h->fd);
	}
	else {
		return (ndport_t)h->remote_addr.sin_port;
	}
}


//设置网络数据处理函数
data_in_entry nd_hook_data(nd_handle h, data_in_entry data_entry) 
{
	data_in_entry ret = NULL ;
	if(h->type==NDHANDLE_TCPNODE || h->type==NDHANDLE_UDPNODE){
		ret = ((struct netui_info *)h)->data_entry ;
		((struct netui_info *)h)->data_entry = data_entry ;
		((struct netui_info *)h)->user_def_data_hook = 1;
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

#include "nd_crypt/nd_crypt.h"

int _log_net_stream(const char *fileName,   void *data, int size)
{

	//int descLength;
	nd_usermsghdr_t *pmsg = (nd_usermsghdr_t *)data;
	char msg_desc[128];
	FILE *fp = fopen(fileName, "ab+") ;
	if (!fp) {
		return -1 ;
	}

	fseek(fp, 0, SEEK_END);

	NDUINT16 mark = ND_STREAM_MESSAGE_START;
	NDUINT32 now = nd_time() ;

	fwrite(&mark, 1, sizeof(mark),  fp);
	//write time
	fwrite(&now, 1, sizeof(now), fp);

	//write message descript
	mark = ndsnprintf(msg_desc, sizeof(msg_desc), "[%s msg(%d,%d) length=%d time=%d]", nd_get_datetimestr(),
		pmsg->maxid, pmsg->minid, pmsg->packet_hdr.length, now);
	fwrite(&mark, 1, sizeof(mark), fp);
	fwrite(msg_desc, 1, mark, fp);

	
	//data size
	mark = size ;
	fwrite(&mark, 1, sizeof(mark), fp);
	//WRITE data
	fwrite(data, 1, size, fp);
	
	mark = ND_STREAM_MESSAGE_END ;
	fwrite(&mark, 1, sizeof(mark), fp);
	fflush(fp) ;
	fclose(fp) ;
	
	return 0;

}
int nd_netobj_recv_stream_save(nd_netui_handle net_handle, void *data, int size )
{
	if (!_s_recv_stream || !_s_recv_stream[0]) {
		return -1;
	}
	return _log_net_stream(_s_recv_stream, data, size) ;
}

int nd_netobj_send_stream_save(nd_netui_handle net_handle,  void *data, int size )
{
	if (!_s_send_stream || !_s_send_stream[0]) {
		return -1;
	}
	return _log_net_stream(_s_send_stream, data, size) ;
	
}

int nd_netobj_update(nd_netui_handle node)
{
	return node->update_entry(node);
}


int nd_netobj_wait_readable(nd_netui_handle node, int tmout)
{
	return node->wait_readable(node, tmout);
}

int nd_netobj_packet_recv(nd_netui_handle node, int tmout)
{
	return node->recv_data(node, tmout);
}

int nd_netobj_wait_writable(nd_netui_handle node, int tmout)
{
	return node->wait_writable(node, tmout);
}

int nd_netobj_close(nd_netui_handle node, int flag)
{
	return node->sys_sock_close(node, flag);
}

int nd_netobj_read(nd_netui_handle node, void *buffer, int size)
{
	return node->sys_sock_read(node, buffer, size);
}

int nd_netobj_write(nd_netui_handle node, void *data, int len)
{
	ENTER_FUNC();
	int ret;
	ret = (int)node->sys_sock_write(node, data, len);
	if (ret > 0) {
		node->send_len += ret;
		node->last_push = nd_time();
	}
	else if (-1 == ret) {
		node->sys_error = nd_socket_last_error();
		if (node->sys_error == ESOCKETTIMEOUT) {
			node->myerrno = NDERR_WOULD_BLOCK;
		}
		else {
			node->myerrno = NDERR_IO;
		}
	}
	else if (ret == 0) {
		node->myerrno = NDERR_WOULD_BLOCK;
	}
	LEAVE_FUNC();
	return ret;
}

//fetch recvd message in nd_packhdr_t format
static int __net_fetch_msg(nd_netui_handle socket_addr, nd_packhdr_t *msgbuf)
{
	ENTER_FUNC()
		int data_len = 0, valid_len = 0;
	nd_packhdr_t  tmp_hdr;
	nd_netbuf_t  *pbuf;
	nd_packhdr_t *stream_data;

	if (!nd_connector_valid(socket_addr)) {
		LEAVE_FUNC();
		return -1;
	}

RE_FETCH:
	pbuf = &(socket_addr->recv_buffer);
	data_len = (int)ndlbuf_datalen(pbuf);

	if (data_len < ND_PACKET_HDR_SIZE) {
		LEAVE_FUNC();
		return 0;
	}

	stream_data = (nd_packhdr_t *)ndlbuf_data(pbuf);

	ND_HDR_SET(&tmp_hdr, stream_data);
	//nd_hdr_ntoh(&tmp_hdr) ;

	//check incoming data legnth
	valid_len = (int)socket_addr->get_pack_size(socket_addr, &tmp_hdr);
	//valid_len =  nd_pack_len(&tmp_hdr) ;

	if (valid_len > ND_PACKET_SIZE || valid_len < _min_packet_len) {
		socket_addr->myerrno = NDERR_BADPACKET;
		LEAVE_FUNC();
		return -1;	//incoming data error 
	}
	else if (valid_len > data_len) {
		LEAVE_FUNC();
		return 0;	//not enough
	}

	if (socket_addr->user_define_packet) {
		//user define packet data
		nd_atomic_inc(&socket_addr->recv_pack_times);
		ndlbuf_read(pbuf, msgbuf, valid_len, EBUF_SPECIFIED);
		//nd_packet_ntoh(msgbuf) ;
		return valid_len;
	}

	/*if ((int)NDNETMSG_VERSION != nd_pack_version(&tmp_hdr)){
		nd_net_message_version_error(socket_addr);
		return -1;
	}
	else*/ if (tmp_hdr.ndsys_msg) {
		if (-1 == nd_net_sysmsg_hander(socket_addr, (nd_sysresv_pack_t *)stream_data)) {
			LEAVE_FUNC();
			return -1;
		}
		ndlbuf_sub_data(pbuf, valid_len);
		valid_len = 0;
		goto RE_FETCH;
	}

	if (msgbuf) {
		ndlbuf_read(pbuf, msgbuf, valid_len, EBUF_SPECIFIED);

		nd_packet_ntoh(msgbuf);

		nd_assert(valid_len == nd_pack_len(msgbuf));
		if (msgbuf->encrypt) {
			int new_len;
			new_len = nd_packet_decrypt(socket_addr, (nd_packetbuf_t *)msgbuf);
			if (new_len == 0) {
				socket_addr->myerrno = NDERR_BADPACKET;
				LEAVE_FUNC();
				return -1;
			}
			nd_pack_len(msgbuf) = (NDUINT16)new_len;
			valid_len = new_len;
			CURRENT_IS_CRYPT(socket_addr) = 1;
		}
		else {
			CURRENT_IS_CRYPT(socket_addr) = 0;
		}
	}
	nd_atomic_inc(&socket_addr->recv_pack_times);
	LEAVE_FUNC();
	return valid_len;
}

int nd_net_fetch_msg(nd_netui_handle socket_addr, nd_packhdr_t *msgbuf)
{
	int readlen = __net_fetch_msg(socket_addr, msgbuf);
	if (readlen > 0 && socket_addr->is_log_recv) {
		if (socket_addr->is_log_recv) {
			nd_logmsg("received (%d,%d) len=%d\n\n", ND_USERMSG_MAXID(msgbuf), ND_USERMSG_MINID(msgbuf), ND_USERMSG_LEN(msgbuf));
		}
		if (socket_addr->save_recv_stream) {
			if (CURRENT_IS_CRYPT(socket_addr)) {
				msgbuf->encrypt = 1;
				nd_netobj_recv_stream_save(socket_addr, msgbuf, readlen);
				msgbuf->encrypt = 0;
			}
			else {
				nd_netobj_recv_stream_save(socket_addr, msgbuf, readlen);
			}
		}

	}
	return readlen;
}

#undef ND_IMPLEMENT_HANDLE
