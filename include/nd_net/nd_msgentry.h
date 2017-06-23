/* file : nd_msgentry.h
 * define net message entry of nd engine 
 *
 * author : neil 
 * luckduan@126.com
 *
 * all right reserved by neil 2008 
 */

#ifndef _ND_MSGENTRY_H_
#define _ND_MSGENTRY_H_

#include "nd_net/nd_netpack.h"
#include "nd_net/nd_netui.h"

enum  privalige_level{
	EPL_NONE = 0 ,			//没有任何权限
	EPL_CONNECT,			//普通权限
	EPL_LOGIN ,				//登陆权限
	EPL_READY,				//start game
	EPL_HIGHT				//特区
};

#define SUB_MSG_NUM		64		//每一类消息可以有多少个子消息号
#define MAX_MAIN_NUM	256		//最多可以拥有的消息类型
#define MAX_SCRIPT_NAME 256		//脚本名字长度
#define ND_DFT_MAXMSG_NUM 32	//默认主消息类型数量

typedef NDUINT8	ndmsgid_t ;		//消息编号

#define ND_UNSUPPORT_SCRIPT_MSG		//不支持脚本方式的消息函数

#pragma pack(push)
#pragma pack(1)
//用户消息类型 头
typedef struct nd_usermsghdr_t
{
	nd_packhdr_t	packet_hdr ;		//消息包头
	ndmsgid_t		maxid ;		//主消息号 8bits
	ndmsgid_t		minid ;		//次消息号 8bits
//	ndmsgparam_t	param;		//消息参数
}nd_usermsghdr_t ;

#define ND_USERMSG_HDRLEN sizeof(nd_usermsghdr_t)
//用户数据长度
#define ND_USERMSG_DATA_CAPACITY  (ND_PACKET_SIZE-sizeof(nd_usermsghdr_t) )		
//定义用户缓冲
typedef struct nd_usermsgbuf_t
{
	nd_usermsghdr_t msg_hdr ;
	char			data[ND_USERMSG_DATA_CAPACITY] ;
}nd_usermsgbuf_t;

#pragma pack(pop)

static __INLINE__ void nd_usermsghdr_init(nd_usermsghdr_t *hdr)
{
	memset(hdr, 0, sizeof(*hdr)) ;
	hdr->packet_hdr.length = ND_USERMSG_HDRLEN ;
	hdr->packet_hdr.version = NDNETMSG_VERSION ;
}
#define ND_USERMSG_INITILIZER {{ND_USERMSG_HDRLEN,NDNETMSG_VERSION,0,0,0},0,0,0} 
#define nd_netmsg_hton(m)		//把网络消息转变成主机格式
#define nd_netmsg_ntoh(m)		//把主机格式转变成网络消息

#define ND_USERMSG_LEN(m)	((nd_packhdr_t*)m)->length
#define ND_USERMSG_MAXID(m)	((nd_usermsghdr_t*)m)->maxid 
#define ND_USERMSG_MINID(m)	((nd_usermsghdr_t*)m)->minid 
#define ND_USERMSG_PARAM(m)	
#define ND_USERMSG_DATA(m)	(((nd_usermsgbuf_t*)m)->data)
#define ND_USERMSG_DATALEN(m)	(((nd_packhdr_t*)m)->length - ND_USERMSG_HDRLEN)
#define ND_USERMSG_SYS_RESERVED(m) ((nd_packhdr_t*)m)->ndsys_msg 
/* 用户自定义消息处理函数
 * 在网络数据到达时会根据消息号执行相应的处理函数.
 * 函数返回值: 出错时返回-1, 系统会自动关闭对应的连接.
 * 需要关闭连接时,只需要在处理函数中返回-1即可
 */
typedef int (*nd_usermsg_func)(nd_handle  handle, nd_usermsgbuf_t *msg , nd_handle listener);

typedef int(*nd_msg_script_entry)(void *script_engine,nd_handle  handle, nd_usermsgbuf_t *msg, const char *script);

/*拦截封包的函数
 *如果使用了拦截函数那么用户不在需要 nd_msgtable_create()和 nd_msgentry_install()
 *而是自行处理被拦截的封包
 */
typedef int (*nd_packet_func)(nd_handle  handle, nd_packhdr_t *msg , nd_handle listener);

/* 为连接句柄创建消息入口表
 * @mainmsg_num 主消息的个数(有多数类消息
 * @base_msgid 主消息开始号
 * return value : 0 success on error return -1
 */
ND_NET_API int nd_msgtable_create(nd_handle  handle, int mainmsg_num, int base_msgid) ;
#define nd_msgtable_open(_h, _main_num) nd_msgtable_create(_h, _main_num, 0)

ND_NET_API void nd_msgtable_destroy(nd_handle  handle, int flag) ;

//set this message is system message which only send by other thread( in order to simulate net message from other thread)
ND_NET_API int nd_message_set_system(nd_handle handle,  ndmsgid_t maxid, ndmsgid_t minid,int issystem) ; 
ND_NET_API int nd_message_set_log(nd_handle handle,  ndmsgid_t maxid, ndmsgid_t minid,int is_log) ;

/*在handle句柄上安装网络消息处理函数*/
ND_NET_API int nd_msgentry_install(nd_handle  handle, nd_usermsg_func, ndmsgid_t maxid, ndmsgid_t minid,int level, const char *name) ;

/* install message script-function */
ND_NET_API int nd_msgentry_script_install(nd_handle handle, const char*script, ndmsgid_t maxid, ndmsgid_t minid, int level);
/* set script engine */
ND_NET_API int nd_message_set_script_engine(nd_handle handle, void *script_engine, nd_msg_script_entry entry);


/* 设置默认消息处理函数*/
ND_NET_API int nd_msgentry_def_handler(nd_handle handle, nd_usermsg_func func)  ;

ND_NET_API nd_usermsg_func nd_msgentry_get_func(nd_handle handle, ndmsgid_t maxid, ndmsgid_t minid);
ND_NET_API nd_usermsg_func nd_msgentry_get_def_func(nd_handle handle) ;
ND_NET_API const char * nd_msgentry_get_name(nd_handle handle, ndmsgid_t maxid, ndmsgid_t minid) ;
ND_NET_API NDUINT32 nd_msgentry_get_id(nd_handle handle, const char *msgname);

ND_NET_API nd_handle nd_get_msg_hadle(nd_handle handle) ;

/* 正常发送消息 */
static __INLINE__ int nd_connectmsg_send(nd_handle  connector_handle, nd_usermsgbuf_t *msg ) 
{
	nd_assert(connector_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(connector_handle, (nd_packhdr_t *)msg, ESF_NORMAL) ;
}

/*投递消息,不可靠的发送,消息可能会丢失*/
static __INLINE__ int nd_connectmsg_post(nd_handle  connector_handle, nd_usermsgbuf_t *msg ) 
{
	nd_assert(connector_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(connector_handle, (nd_packhdr_t *)msg, ESF_POST) ;
}

/* 发送紧急数据 */
static __INLINE__ int nd_connectmsg_send_urgen(nd_handle  connector_handle, nd_usermsgbuf_t *msg ) 
{
	nd_assert(connector_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(connector_handle, (nd_packhdr_t *)msg,  ESF_URGENCY) ;
}

/* 发送扩展函数*/
static __INLINE__ int nd_connectmsg_sendex(nd_handle  connector_handle, nd_usermsgbuf_t *msg , int flag) 
{
	nd_assert(connector_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(connector_handle, (nd_packhdr_t *)msg,  flag) ;
}

/*
 * 把网络接口过来的消息传递给用户定义的消息,
 * 一般是网络解析层调用
 * @connect_handle 进入的消息句柄
 * @msg 消息内容
 * @callback_param 用户输入参数
 */
ND_NET_API int nd_translate_message(nd_handle  connector_handle, nd_packhdr_t *msg,nd_handle listen_handle ) ;

ND_NET_API int nd_translate_message_ex(nd_handle owner, nd_packhdr_t *msg, nd_handle listen_handle, nd_handle caller);

/* 
 * 服务器端消息传递函数
 * 把网络接口过来的消息传递给用户定义的消息,
 * 一般是网络解析层调用
 */
ND_NET_API int nd_srv_translate_message(nd_handle  connector_handle, nd_packhdr_t *msg,nd_handle listen_handle ) ;

//使用sessionid 模式
//ND_NET_API int nd_srv_translate_message1(NDUINT16 session_id, nd_packhdr_t *msg , nd_handle listen_handle,int session_privilege) ;

//权限等级
ND_NET_API NDUINT32 nd_connect_level_get(nd_handle  connector_handle) ;

//权限等级
ND_NET_API void nd_connect_level_set(nd_handle  connector_handle,NDUINT32 level);

// check message is log 
ND_NET_API int nd_message_is_log(nd_handle nethandle, int maxId, int minId);

#endif

