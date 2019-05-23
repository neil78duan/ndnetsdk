/* file nd_udt.h
 * udp data translate porotocal
 *
 * neil
 * 2005-11-23
 */

#ifndef _NDUDT_H_
#define _NDUDT_H_

//#include "nd_net/nd_netlib.h"
//#include "nd_common/nd_common.h"
#include "nd_net/nd_udthdr.h"
#include "nd_net/nd_netcrypt.h"
#include "nd_net/nd_netobj.h"
#include "nd_net/nd_netioctl.h"

#define MAX_ATTEMPT_SEND	5 	//tryto retranslate times 
//#define LISTEN_BUF_SIZE		16	//listen number 

//#define UDT_MAX_PACKET		32	//�����Է���32��δ��ȷ�ϵķ��

#define RETRANSLATE_TIME		10000 //��������ʱ��ms
#define WAIT_CONNECT_TIME		15000 //�ȴ��������ӵ�ʱ��
#define TIME_OUT_BETA			2	//������Ȩ����,����ʱ���ڴ�ֵ * ����ʱ�����Ϊ�ǳ�ʱ��
#define DELAY_ACK_TIME			10 //ms
#define ACTIVE_TIME				1000*30 //20 seconds 
#define CONNECT_TIMEOUT			5000 //S
#define UPDATE_TIMEVAL			100 //MS ��������״̬��ʱ����
//#define WAIT_RELEASE_TIME		150000	//�ر�ʱ�ȴ���ʱ

#define MAX_UDP_LEN				ND_UDP_PACKET_SIZE
/*��������״̬*/
enum _enetstat {
	NETSTAT_CLOSED = 0 ,
	NETSTAT_LISTEN =1,
	NETSTAT_SYNSEND =2,
	NETSTAT_SYNRECV =4,
	NETSTAT_ACCEPT =8,		//�Ѿ����ӳɹ��ȴ��û�accept
	NETSTAT_ESTABLISHED =0x10,	//���ӳɹ�
	NETSTAT_FINSEND =0x20,	//����(���Ͷ˱��ر�)
	NETSTAT_SENDCLOSE = 0x40, //д���ݹر�
	NETSTAT_RECVCLOSE =0x80, //�����ݹر�
	NETSTAT_RESET	  = 0x100
};

//��¼��ʱ�ش�������ƽ��ʱ���ƫ��
struct nd_rtt
{
	int average  ;		//������Ȩƽ��ֵ
	int deviation ;		//��������
};


struct _udt_packet_node{
	struct list_head list;
	ndtime_t recvTm;
	int size;
	struct ndudt_pocket pack;
};

typedef struct _s_udt_socket nd_udt_node ;

//���UDT���ݰ��Ƿ�Ϸ�
typedef int (*check_udt_packet)(nd_udt_node *node, struct ndudt_pocket *pocket, int len,SOCKADDR_IN *addr) ;
typedef int(*udt_close_entry)(nd_udt_node *node, int flag);

/* �����������ӽڵ�
 * �������ӷ������Ľڵ�
 * ��Ҫ�����Ǵ�����/����/����/
 * ȷ��/��ʱ�ش���
 */

struct _s_udt_socket
{
	ND_OBJ_BASE ;
	ND_SOCKET_OBJ_BASE ;
	ND_CONNECTOR_BASE ;
	struct udp_proxy_info *prox_info ;
	union {
		struct sockaddr_in last_read;
		struct sockaddr_in6 last_read6;
	};
	//////////////////////////////////////////////////////////////////////////
	check_udt_packet check_entry;
	udt_close_entry udt_close_entry;

	u_16	is_accept:4;				//0 connect , 1 accept
	u_16	is_reset:1;
	u_16	nonblock:1;					//0 block 1 nonblock
	u_16	need_ack:1 ;				////����һ�˷������ݵ�ʱ���Ƿ���Ҫ����ȷ��
	u_16	iodrive_mod:1 ;				//����ģ��0Ĭ��,��send��recv�������Զ�����,1��Ҫ�û���ʾ��ʹ��update_socket)
	u_16	is_retranslate:1;			//�Ƿ����ش�ģʽ
	u_16	user_data_in : 1;			// received user data 
	u_16	resend_times ;				//�ط��Ĵ���
	u_16	local_port;					//port for udt
	
	ndtime_t retrans_timeout ;			//��ʱ�ش�ʱ��͵ȴ��ر�ʱ��(��¼ʱ�������Ǿ���ʱ��)
	ndtime_t last_resend_tm;			//�ϴ��ش�ʱ��(ֻ������ݱ�)
	ndtime_t last_active ;				//��һ�η��ͷ��ʱ��(����ʱ�����̫���ͳ�ʱ,���߷���alive��)
	ndtime_t update_tm ;				//time point of update 
	
	u_32 send_sequence ;				//��ǰ���͵ĵ�ϵ�к�
	u_32 acknowledged_seq ;				//�Ѿ����Է�ȷ�ϵ�ϵ�к� (send_sequence - acknowledged_seq)����û�з��ʹ�����δ��ȷ�ϵ�
	u_32 received_sequence ;			//���ܵ��ĶԷ���ϵ�к�
	u_32 retrans_seq;					//�ش�ϵ�к�(λ��[acknowledged_seq,send_sequence])
	size_t window_len ;					//�Է����մ��ڵĳ���
	struct nd_rtt    _rtt ;				//��¼��������ʱ��
	struct list_head pre_list;			//��ǰ�����İ�
} ;

typedef int(*udt_data_proc)(nd_handle hsrv, struct udt_packet_info *packet);
typedef int(*udt_connected_proc)(nd_handle hsrv, nd_udt_node *socket_node, SOCKADDR_IN *addr);


struct udt_listen_node
{
	struct nd_srv_node base;
	udt_data_proc data_proc;
	udt_connected_proc accept_proc;
	//struct list_head wait_rease;

};
typedef struct udt_listen_node nd_udtsrv;

#define UDTSO_SET_RESET(udt_socket)  (udt_socket)->is_reset = 1
#define UDTSO_IS_RESET(udt_socket)  (udt_socket)->is_reset

#define UDT_RECV_USER_DATA(node) (node)->user_data_in 

static __INLINE__ size_t send_window(nd_udt_node *socket_node)
{
	size_t size = ndlbuf_free_capacity(&socket_node->recv_buffer) ;
	if (size > 0xffff) {
		return 0xffff;
	}
	return size;
}


static __INLINE__ void set_socket_ack(nd_udt_node *socket_node, int flag) 
{
	socket_node->need_ack = flag ;
}
#define get_socket_ack(socket_node ) (socket_node)->need_ack 


static __INLINE__ int udt_is_reset(nd_udt_node *socket_node)
{
	return socket_node->status == NETSTAT_RESET;
}

int _handle_ack(nd_udt_node *socket_node, u_32 ack_seq);

int _handle_fin(nd_udt_node* socket_node, struct ndudt_pocket *pocket);

int _udt_syn(nd_udt_node *socket_node) ;

int _test_checksum(struct ndudt_header *data, int size); //return 0 error ,1 success 
void _set_checksum(struct ndudt_header *data,int size);

//int _handle_income_data(nd_udt_node* socket_node, struct ndudt_pocket *pocket, size_t len);

int _udt_fin(nd_udt_node *socket_node);

//Wait and read data from nd_udt_node
//return 0 no data in coming, -1 error check error code ,else return lenght of in come data
int _wait_data(nd_udt_node *socket_node,udt_pocketbuf* buf,ndtime_t outval) ;

int write_pocket_to_socket(nd_udt_node *socket_node, struct ndudt_pocket *pocket, size_t len);
int read_packet_from_socket(nd_udt_node *socket_node, char *buf, size_t size, ndtime_t tmout);

int _handle_syn(nd_udt_node *socket_node,struct ndudt_pocket *pocket);
/*����udtЭ���*/
int _udt_packet_handler(nd_udt_node *socket_node,struct ndudt_pocket *pocket,size_t len);

int udt_send_ack(nd_udt_node *socket_node) ;

char *send_window_start(nd_udt_node* socket_node, size_t *sendlen) ;

//void udt_conn_timeout(nd_udt_node* socket_node) ;

ndtime_t calc_timeouval(struct nd_rtt *rtt, int measuerment) ;

//void send_reset_packet(nd_udt_node* socket_node) ;

int _tryto_fetch_prelist(nd_udt_node *socket_node);
int _addto_pre_list(nd_udt_node *socket_node, struct ndudt_pocket *packet, int len);
int _update_prelist(nd_udt_node *socket_node);

ND_NET_API int update_udt_session(nd_udt_node *node);
ND_NET_API void _udt_connector_init(nd_udt_node *socket_node) ;
ND_NET_API void nd_udtnode_init(nd_udt_node *socket_node);

ND_NET_API void _deinit_udt_socket(nd_udt_node *socket_node) ;
/*����UDT����:�رյ�ǰ���Ӻ�,��ջ���͸���״̬;
 * ���Ǳ����û����������,��Ϣ�������ͼ�����Կ
 */
ND_NET_API void nd_udtnode_reset(nd_udt_node *socket_node) ;


/*����UDT,�����ӳٷ���,����ȷ��,��ʱ�ش�,�������Ӵ���
 * return value : -1 error check error code
 * return 0 , received data over, closed by remote peer
 * else return > 0
 */
ND_NET_API int update_socket(nd_udt_node* socket_node) ;

//����һ������
ND_NET_API void udt_reset(nd_udt_node* socket_node,int issend_reset) ;

//�ر�һ������
ND_NET_API int udt_close(nd_udt_node* socket_node,int force);

//���ӵ�������
ND_NET_API nd_udt_node* udt_connect(nd_udt_node *socket_node,const char *host, short port,struct nd_proxy_info *proxy) ;

//���Ϳɿ�����ʽЭ��
ND_NET_API int udt_send(nd_udt_node* socket_node,void *data, int len ) ;

//����UDT nd_packhdr_t ��
//ND_NET_API int udt_connector_send(nd_udt_node* socket_addr, nd_packhdr_t *msg_buf, int flag) ;									

//listen �����
//�ͷ�һ���Ѿ��رյ�����
ND_NET_API void release_dead_node(nd_udt_node *socket_node,int needcallback) ;

//����fin������ر�����
//void _close_listend_socket(nd_udt_node* socket_node) ;

//����ÿ��udt_socket��״̬
//��ʱ����ÿ������
ND_NET_API void update_all_socket(nd_udtsrv *root) ;


//ND_NET_API int pump_insrv_udt_data(nd_udtsrv *root, struct ndudt_pocket *pocket, int len, SOCKADDR_IN *addr);
ND_NET_API int pump_insrv_udt_data(nd_udtsrv *root, struct udt_packet_info *pack_buf);

//����udt����
//ND_NET_API int udt_data_handler(SOCKADDR_IN *addr, struct ndudt_pocket*pocket, size_t read_len, nd_udtsrv *root)  ;

//ND_NET_API void udt_icmp_init(nd_udt_node *socket_node) ;
//ND_NET_API void _udticmp_connector_init(nd_udt_node *socket_node) ;
#endif
