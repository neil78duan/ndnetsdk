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
	EPL_NONE = 0 ,			//û���κ�Ȩ��
	EPL_CONNECT,			//��ͨȨ��
	EPL_LOGIN ,				//��½Ȩ��
	EPL_HIGHT				//����
};

#define SUB_MSG_NUM		64		//ÿһ����Ϣ�����ж��ٸ�����Ϣ��
#define MAX_MAIN_NUM	256		//������ӵ�е���Ϣ����
#define MAX_SCRIPT_NAME 256		//�ű����ֳ���
#define ND_DFT_MAXMSG_NUM 32	//Ĭ������Ϣ��������

typedef NDUINT8	ndmsgid_t ;		//��Ϣ���

#define ND_UNSUPPORT_SCRIPT_MSG		//��֧�ֽű���ʽ����Ϣ����

#pragma pack(push)
#pragma pack(1)
//�û���Ϣ���� ͷ
typedef struct nd_usermsghdr_t
{
	nd_packhdr_t	packet_hdr ;		//��Ϣ��ͷ
	ndmsgid_t		maxid ;		//����Ϣ�� 16bits
	ndmsgid_t		minid ;		//����Ϣ�� 16bits
//	ndmsgparam_t	param;		//��Ϣ����
}nd_usermsghdr_t ;

#define ND_USERMSG_HDRLEN sizeof(nd_usermsghdr_t)
//�û����ݳ���
#define ND_USERMSG_DATA_CAPACITY  (ND_PACKET_SIZE-sizeof(nd_usermsghdr_t) )		
//�����û�����
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
#define nd_netmsg_hton(m)		//��������Ϣת���������ʽ
#define nd_netmsg_ntoh(m)		//��������ʽת���������Ϣ

#define ND_USERMSG_LEN(m)	((nd_packhdr_t*)m)->length
#define ND_USERMSG_MAXID(m)	((nd_usermsghdr_t*)m)->maxid 
#define ND_USERMSG_MINID(m)	((nd_usermsghdr_t*)m)->minid 
#define ND_USERMSG_PARAM(m)	
#define ND_USERMSG_DATA(m)	(((nd_usermsgbuf_t*)m)->data)
#define ND_USERMSG_DATALEN(m)	(((nd_packhdr_t*)m)->length - ND_USERMSG_HDRLEN)
#define ND_USERMSG_SYS_RESERVED(m) ((nd_packhdr_t*)m)->ndsys_msg 
/* �û��Զ�����Ϣ������
 * ���������ݵ���ʱ�������Ϣ��ִ����Ӧ�Ĵ�����.
 * ��������ֵ: ����ʱ����-1, ϵͳ���Զ��رն�Ӧ������.
 * ��Ҫ�ر�����ʱ,ֻ��Ҫ�ڴ������з���-1����
 */
typedef int (*nd_usermsg_func)(nd_handle  handle, nd_usermsgbuf_t *msg , nd_handle listener);

/*���ط���ĺ���
 *���ʹ�������غ�����ô�û�������Ҫ nd_msgtable_create()�� nd_msgentry_install()
 *�������д������صķ��
 */
typedef int (*nd_packet_func)(nd_handle  handle, nd_packhdr_t *msg , nd_handle listener);

/* Ϊ���Ӿ��������Ϣ��ڱ�
 * @mainmsg_num ����Ϣ�ĸ���(�ж�������Ϣ
 * @base_msgid ����Ϣ��ʼ��
 * return value : 0 success on error return -1
 */
ND_NET_API int nd_msgtable_create(nd_handle  handle, int mainmsg_num, int base_msgid) ;
#define nd_msgtable_open(_h, _main_num) nd_msgtable_create(_h, _main_num, 0)

ND_NET_API void nd_msgtable_destroy(nd_handle  handle, int flag) ;
ND_NET_API int nd_message_set_system(nd_handle handle,  ndmsgid_t maxid, ndmsgid_t minid,int issystem) ;

/*��handle����ϰ�װ������Ϣ������*/
ND_NET_API int nd_msgentry_install(nd_handle  handle, nd_usermsg_func, ndmsgid_t maxid, ndmsgid_t minid,int level) ;

/* ����Ĭ����Ϣ������*/
ND_NET_API int nd_msgentry_def_handler(nd_netui_handle handle, nd_usermsg_func func)  ;

#ifndef ND_UNSUPPORT_SCRIPT_MSG

/*��Ϣ������ִ�нű��ĺ���
 * @handle_connect���Ӿ�����߻滭���
 * @msg��Ϣ
 * @script �ű�����
 * �����Լ��Ľű�������,����Ϣ������ʱ��ͻ��ô˺���ȥִ��
 * ��Ҫ���ݲ�ͬ����Ϣע�᲻ͬ�Ľű�����,�ű�����������ע�����������ͬ����Ϣ
 */

typedef int (*nd_msg_script_func)(nd_handle  handle_connect ,nd_usermsgbuf_t *msg , char *script,nd_handle listener) ;
//���ýű���������
ND_NET_API int nd_set_script_engine(nd_handle handle, nd_msg_script_func script_entry) ;

/*��װ�ű���ʽ����Ϣ������*/
ND_NET_API int nd_msg_script_install(nd_handle  handle, char *script_name, ndmsgid_t maxid, ndmsgid_t minid,int level) ;
#endif 

/* ����������Ϣ */
static __INLINE__ int nd_connectmsg_send(nd_handle  connector_handle, nd_usermsgbuf_t *msg ) 
{
	nd_assert(connector_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(connector_handle, (nd_packhdr_t *)msg, ESF_NORMAL) ;
}

/*Ͷ����Ϣ,���ɿ��ķ���,��Ϣ���ܻᶪʧ*/
static __INLINE__ int nd_connectmsg_post(nd_handle  connector_handle, nd_usermsgbuf_t *msg ) 
{
	nd_assert(connector_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(connector_handle, (nd_packhdr_t *)msg, ESF_POST) ;
}

/* ���ͽ������� */
static __INLINE__ int nd_connectmsg_send_urgen(nd_handle  connector_handle, nd_usermsgbuf_t *msg ) 
{
	nd_assert(connector_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(connector_handle, (nd_packhdr_t *)msg,  ESF_URGENCY) ;
}

/* ������չ����*/
static __INLINE__ int nd_connectmsg_sendex(nd_handle  connector_handle, nd_usermsgbuf_t *msg , int flag) 
{
	nd_assert(connector_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(connector_handle, (nd_packhdr_t *)msg,  flag) ;
}

/*
 * ������ӿڹ�������Ϣ���ݸ��û��������Ϣ,
 * һ����������������
 * @connect_handle �������Ϣ���
 * @msg ��Ϣ����
 * @callback_param �û��������
 */
ND_NET_API int nd_translate_message(nd_handle  connector_handle, nd_packhdr_t *msg,nd_handle listen_handle ) ;


/* 
 * ����������Ϣ���ݺ���
 * ������ӿڹ�������Ϣ���ݸ��û��������Ϣ,
 * һ����������������
 */
ND_NET_API int nd_srv_translate_message(nd_handle  connector_handle, nd_packhdr_t *msg,nd_handle listen_handle ) ;

//ʹ��sessionid ģʽ
//ND_NET_API int nd_srv_translate_message1(NDUINT16 session_id, nd_packhdr_t *msg , nd_handle listen_handle,int session_privilege) ;

//Ȩ�޵ȼ�
ND_NET_API NDUINT32 nd_connect_level_get(nd_handle  connector_handle) ;

//Ȩ�޵ȼ�
ND_NET_API void nd_connect_level_set(nd_handle  connector_handle,NDUINT32 level);

#endif

