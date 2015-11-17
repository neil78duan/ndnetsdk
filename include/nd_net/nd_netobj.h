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
	ndip_t				bindip;				\
	int					sys_error

struct nd_netsocket
{
	ND_OBJ_BASE ;
	ND_SOCKET_OBJ_BASE ;
};

typedef void *nd_userdata_t ;

//发送数据函数
typedef int (*socket_write_entry)(nd_handle node,void *data , size_t len) ; 

typedef int (*socket_read_entry)(nd_handle node,void *data , size_t size, ndtime_t tmout) ; 

//封包发送函数
typedef int (*packet_write_entry)(nd_handle net_handle, nd_packhdr_t *msg_buf, int flag) ;	//define extend send function

/*网络原始数据接收处理
 * @node连接节点
 * @data 需要处理的数据
 * @len 数据长度
 * @listen_h node所属的监听器
 * return value : 返回实际被处理的数据长度,被处理的数据将被删除, -1出错,连接被关闭
 */
typedef int (*data_in_entry)(nd_handle node,void *data , size_t len,nd_handle listen_h) ;

//net message handle entry 
typedef int (*net_msg_entry)(nd_handle  handle, nd_packhdr_t *msg , nd_handle listener);


typedef int (*net_update_entry)(nd_handle h) ;

typedef int (*net_writable_callback) (nd_handle h,nd_userdata_t param) ; //when socket can be writable call 

typedef size_t (*net_get_packet_size)(nd_handle  handle, void *data) ;


#define  ND_CONNECTOR_BASE \
	NDUINT8		level ;			\
	NDUINT8		read_again:1;	\
	NDUINT8		is_crypt_packet:1;\
	NDUINT8		is_session:1;	\
	NDUINT8		is_log_send:1;	\
	NDUINT8		is_log_recv:1;	\
	NDUINT16	session_id;		\
	ndtime_t	start_time ;	\
	ndtime_t	last_recv ;		\
	size_t		send_len ;		\
	size_t		recv_len ;		\
	nd_handle 	msg_handle ;	\
	nd_handle 	srv_root;		\
	nd_userdata_t 		user_data ;	\
	nd_mutex			*send_lock;		\
	packet_write_entry	write_entry;	\
	socket_write_entry	sock_write;		\
	socket_read_entry	sock_read;		\
	data_in_entry		data_entry;		\
	net_msg_entry		msg_entry ;		\
	net_update_entry	update_entry ;	\
	net_get_packet_size get_pack_size;	\
	nd_userdata_t		writable_param ;	\
	net_writable_callback writable_callback; \
	nd_cryptkey			crypt_key ;		\
	struct sockaddr_in 	remote_addr ;	\
	ndtime_t			last_push ;		\
	ndtime_t			disconn_timeout;\
	ndatomic_t			send_pack_times,recv_pack_times;\
	nd_netbuf_t 		recv_buffer, send_buffer

#define ND_NETBUF_SIZE	0x10000
#define ND_ALIVE_TIMEOUT 30000		//发送alive包时间间隔
#define ND_DFT_DISSCONN_TIMEOUT	(30*60*1000)	//默认规定时间内么有收到对方的包就断开连接

//定义网络缓冲
/*typedef struct nd_netbuf 
{	
	unsigned int buf_capacity ;
	char *__start, *__end ;	
	char buf[ND_NETBUF_SIZE] ;
}nd_netbuf_t;
*/
typedef struct nd_linebuf nd_netbuf_t ;


/*网络连接或绘话句柄,成员内容只读*/
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


int net_init_sendlock(nd_netui_handle socket_node);
void net_release_sendlock(nd_netui_handle  socket_node);
size_t nd_net_getpack_size(nd_handle  handle, void *data) ;

//设置每个网络封包长度最小值,如果小于这个值,认为接收网络数据错误
ND_NET_API int nd_net_set_packet_minsize(int minsize) ;

/*
 * 设置网络接口相关参数
 * @cmd 命令参考ND_IOCTRL_CMD 
 * @val 变量值 输入/输出
 * @val 变量长度 输入/输出
*/
ND_NET_API int nd_net_ioctl(nd_netui_handle  socket_node, int cmd, void *val, int *size) ;

/*  如果成功等待一个网络消息,
* 那么现在可以使用nd_net_get_msg函数从消息缓冲中提取一个消息
*/
ND_NET_API nd_packhdr_t* nd_net_get_msg(nd_netui_handle node) ;

/*删除已经处理过的消息*/
ND_NET_API void nd_net_del_msg(nd_netui_handle node, nd_packhdr_t *msgaddr) ;


/*从udt-stream的缓冲中提取一个完整的udt消息
 *此函数用在等待消息的模块中
 *在等待消息之前提取一次,防止上次的遗漏,
 *等待网络数据到了以后在提取一次
 */
ND_NET_API int nd_net_fetch_msg(nd_netui_handle socket_addr, nd_packhdr_t *msgbuf) ;

/*从udt-stream的缓冲中提取一个完整的udt消息
*此函数用在等待消息的模块中
*在等待消息之前提取一次,防止上次的遗漏,
*等待网络数据到了以后在提取一次
*/
ND_NET_API int nd_net_fetch_msg(nd_netui_handle socket_addr, nd_packhdr_t *msgbuf) ;

ND_NET_API int nd_net_bind(int port, int listen_nums,nd_handle net_handle) ;

ND_NET_API int nd_net_sendto(nd_handle node,void *data , size_t len,SOCKADDR_IN *to) ;
//绑定到指定的IP
ND_NET_API int nd_net_ipbind(nd_handle net_handle, ndip_t ip) ;
ND_NET_API int icmp_socket_read(struct nd_netsocket*node , char *buf, size_t buf_size, struct sockaddr_in *addr, ndip_t destip, NDUINT16 destport);

#endif
