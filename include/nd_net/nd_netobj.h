/* file nd_netobj.h
 * define net object header 
 *
 * all right reserved by neil duan
 * 2009-5-8 23:04 
 */

#ifndef _ND_NETOBJ_H_
#define _ND_NETOBJ_H_

//protocol  socket_type = SOCK_STREAM not set, 
//socket_type=SOCK_RAW  use IPPROTO_ICMP IPPROTO_IGMP
// socket_type = SOCK_DGRAM ref endudt_protocol

#define ND_SOCKET_OBJ_BASE  \
	NDUINT32			status;				/*socket state in game 0 not read 1 ready*/	\
	ndsocket_t			fd ;				/* socket file description */				\
	NDUINT16			port ;				/* listen port */	\
	NDUINT8				sock_type;		/* socket type SOCK_STREAM SOCK_DGRAM or SOCK_RAW*/	\
	NDUINT8				protocol;			/* user protocol*/	\
	NDUINT8				sock_protocol;		/* protocol of socket */	\
	NDUINT8				error_privilage_close:1; /* close on privilage error */		\
	NDUINT8				unreg_msg_close:1; /* close recv unhandler message */		\
	NDUINT8				user_define_packet:1; /* user define packet*/ \
	int					sys_error;			\
	void 				*msg_caller;	\
	char				bindip[64]

struct nd_netsocket
{
	ND_OBJ_BASE ;
	ND_SOCKET_OBJ_BASE ;
};

typedef void *nd_userdata_t ;

//send raw-data function 
//typedef int (*socket_write_entry)(nd_handle node,void *data , size_t len) ; 
typedef int(*socket_sys_entry)(nd_handle node, void *data, size_t len);
typedef int(*socket_wait_entry)(nd_handle  handle, ndtime_t tmout);
typedef int(*socket_close_entry)(nd_handle  handle, int flag);

//typedef int (*socket_read_entry)(nd_handle node,void *data , size_t size, ndtime_t tmout) ; 

//send *ND-protocol* package function 
typedef int (*packet_write_entry)(nd_handle net_handle, nd_packhdr_t *msg_buf, int flag) ;	//define extend send function

/* raw-net-data handle function 
 * @node net handle
 * @data input data
 * @len data length
 * @listen_h node listener
 * return value : the data length real handled 
 */
typedef int (*data_in_entry)(nd_handle node,void *data , size_t len,nd_handle listen_h) ;

//net message handle entry 
typedef int (*net_msg_entry)(nd_handle  handle, nd_packhdr_t *msg , nd_handle listener);


typedef int (*net_update_entry)(nd_handle h) ;

typedef int (*net_writable_callback) (nd_handle h,nd_userdata_t param) ; //when socket can be writable call 

typedef size_t(*net_get_packet_size)(nd_handle  handle, void *data);
typedef int(*wait_message_entry)(nd_handle  handle, ndtime_t tmout);


#define  ND_CONNECTOR_BASE \
	NDUINT8		level:6 ;			\
	NDUINT8		read_again:1;	\
	NDUINT8		is_crypt_packet:1;\
	NDUINT8		is_session:1;	\
	NDUINT8		is_log_send:1;	\
	NDUINT8		is_log_recv:1;	\
	NDUINT8 	save_send_stream:1;\
	NDUINT8 	save_recv_stream:1;\
	NDUINT8 	user_def_data_hook:1;\
	NDUINT8 	is_ipv6:1;\
	NDUINT8 	without_alive:1;\
	NDUINT16	session_id;		\
	ndtime_t	start_time ;	\
	ndtime_t	last_recv ;		\
	size_t		send_len ;		\
	size_t		recv_len ;		\
	nd_handle 	msg_handle ;	\
	nd_handle 	srv_root;		\
	nd_userdata_t 		user_data ;	\
	void				*ssl ;	\
	nd_mutex			*send_lock;		\
	socket_sys_entry    sys_sock_read;	\
	socket_sys_entry    sys_sock_write;	\
	socket_wait_entry	wait_readable ;	\
	socket_wait_entry	wait_writable;	\
	socket_close_entry	sys_sock_close;	\
	socket_wait_entry	recv_data;	\
	packet_write_entry	packet_write;	\
	data_in_entry		data_entry;		\
	net_msg_entry		msg_entry ;		\
	net_update_entry	update_entry ;	\
	net_get_packet_size get_pack_size;	\
	nd_userdata_t		writable_param ;	\
	net_writable_callback writable_callback; \
	nd_cryptkey			crypt_key ;		\
union {									\
	struct sockaddr_in 	remote_addr;	\
	struct sockaddr_in6 remote_addr6;	\
};										\
	ndtime_t			last_push ;		\
	ndtime_t			disconn_timeout;\
	ndatomic_t			send_pack_times,recv_pack_times;\
	nd_netbuf_t 		recv_buffer, send_buffer

#define ND_NETBUF_SIZE	0x10000
#define ND_ALIVE_TIMEOUT 30000					//send alive package time interval
#define ND_DFT_DISSCONN_TIMEOUT	(30*60*1000)	//default timeout 

typedef struct nd_linebuf nd_netbuf_t ;


/*net handle*/
typedef struct netui_info
{
	ND_OBJ_BASE ;
	ND_SOCKET_OBJ_BASE ;
	ND_CONNECTOR_BASE ;
}*nd_netui_handle;

static __INLINE__ void nd_send_lock( nd_netui_handle node)
{
	if(node->send_lock)
		nd_mutex_lock(node->send_lock) ;
}
static __INLINE__ int nd_send_trytolock(nd_netui_handle node)
{
	if(node->send_lock)
		return nd_mutex_trylock(node->send_lock) ;
	else 
		return 0;
}
static __INLINE__ void nd_send_unlock(nd_netui_handle node)
{
	if(node->send_lock)
		nd_mutex_unlock(node->send_lock) ;
}
static __INLINE__ void nd_net_connbuf_init(nd_netui_handle node)
{
	ndlbuf_init(&node->recv_buffer, ND_NETBUF_SIZE) ;
	ndlbuf_init(&node->send_buffer, ND_NETBUF_SIZE) ;
}
static __INLINE__ int nd_netobj_is_session(nd_handle node)
{
	return( (nd_netui_handle)node)->is_session;
}

int net_init_sendlock(nd_netui_handle socket_node);
void net_release_sendlock(nd_netui_handle  socket_node);
size_t nd_net_getpack_size(nd_handle  handle, void *data) ;
void _release_send_stream();

//set net send recv threshold 
ND_NET_API int nd_net_set_packet_minsize(int minsize) ;

/*
 * net handle ioctl
 * @cmd ref ND_IOCTRL_CMD 
 * @val input/output value address
 * @val input/output value length
*/
ND_NET_API int nd_net_ioctl(nd_netui_handle  socket_node, int cmd, void *val, int *size) ;

/*  get message from recv window
* only get message address ,not copy
*/
ND_NET_API nd_packhdr_t* nd_net_get_msg(nd_netui_handle node) ;

/*remove a message */
ND_NET_API void nd_net_del_msg(nd_netui_handle node, nd_packhdr_t *msgaddr) ;


ND_NET_API int nd_netobj_update(nd_netui_handle node);
/* fetch message from RECV-WINDOW
 * and copy message to msgbuf
 */
ND_NET_API int nd_net_fetch_msg(nd_netui_handle socket_addr, nd_packhdr_t *msgbuf) ;

// bind to port
ND_NET_API int nd_net_bind(int isipv6, int port, int listen_nums,nd_handle net_handle) ;

ND_NET_API int nd_net_sendto(nd_handle node,void *data , size_t len,SOCKADDR_IN *to) ;

ND_NET_API int nd_netobj_wait_readable(nd_netui_handle node, int tmout);

ND_NET_API int nd_netobj_wait_writable(nd_netui_handle node, int tmout);

ND_NET_API int nd_netobj_close(nd_netui_handle node, int flag);

ND_NET_API int nd_netobj_read(nd_netui_handle node, void *buffer, int size);

ND_NET_API int nd_netobj_write(nd_netui_handle node, void *data, int len);

ND_NET_API int nd_netobj_packet_recv(nd_netui_handle node, int tmout);

//ND_NET_API int icmp_socket_read(struct nd_netsocket*node , char *buf, size_t buf_size, struct sockaddr_in *addr, ndip_t destip, NDUINT16 destport);

ND_NET_API void* nd_net_object_caller(nd_handle node) ;

ND_NET_API int nd_netobj_recv_stream_save(nd_netui_handle net_handle, void *data, int size );
ND_NET_API int nd_netobj_send_stream_save(nd_netui_handle net_handle, void *data, int size );

static __INLINE__ int nd_netobj_is_ipv6(nd_netui_handle net_handle) 
{
	return net_handle->is_ipv6;
}
static __INLINE__ void nd_netobj_set_ipv6(nd_netui_handle net_handle)
{
	net_handle->is_ipv6 = 1;
}

static __INLINE__ void nd_netobj_close_keep_alive(nd_netui_handle net_handle)
{
	net_handle->without_alive = 1;
}
static __INLINE__ int nd_netobj_is_alive(nd_netui_handle net_handle)
{
	return net_handle->without_alive==0;
}

#endif
