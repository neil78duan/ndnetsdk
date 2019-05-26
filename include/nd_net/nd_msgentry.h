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


/* message hande function
 * return -1 the connection would closed .
 */
typedef int (*nd_usermsg_func)(nd_handle  handle, nd_usermsgbuf_t *msg );

typedef int(*nd_msg_script_entry)(void *script_engine,nd_handle  handle, nd_usermsgbuf_t *msg, const char *script);

/*packet data function
 */
//typedef int (*nd_packet_func)(nd_handle  handle, nd_packhdr_t *msg );


static __INLINE__ void nd_make_alive_pack(nd_sysresv_pack_t *pack)
{
	nd_hdr_init(&pack->hdr);
	pack->hdr.length = sizeof(nd_sysresv_pack_t);
	pack->hdr.ndsys_msg = 1;
	pack->hdr.stuff_len = 5;
	pack->msgid = ERSV_ALIVE_ID;
	pack->checksum = 0;
	pack->checksum = nd_checksum((NDUINT16 *)pack, sizeof(nd_sysresv_pack_t));
}

static __INLINE__ void nd_packet_hton(nd_packhdr_t *hdr)
{
	hdr->length = htons(hdr->length);
}

static __INLINE__ void nd_packet_ntoh(nd_packhdr_t *hdr)
{
	hdr->length = ntohs(hdr->length);
}

#define packet_hton(p) nd_packet_hton((nd_packhdr_t *)(p))
#define packet_ntoh(p) nd_packet_ntoh((nd_packhdr_t *)(p))


/* create message map table 
 * @mainmsg_num main message number
 * @base_msgid  message-start-index
 * return value : 0 success on error return -1
 */
ND_NET_API int nd_msgtable_create(nd_handle  handle, int mainmsg_num, int base_msgid) ;
#define nd_msgtable_open(_h, _main_num) nd_msgtable_create(_h, _main_num, 0)

ND_NET_API void nd_msgtable_destroy(nd_handle  handle, int flag) ;

//set this message is system message which only send by other thread( in order to simulate net message from other thread)
ND_NET_API int nd_message_set_system(nd_handle handle,  ndmsgid_t maxid, ndmsgid_t minid,int issystem) ; 
ND_NET_API int nd_message_set_log(nd_handle handle,  ndmsgid_t maxid, ndmsgid_t minid,int is_log) ;

ND_NET_API int nd_message_set_print(nd_handle nethandle, ndmsgid_t maxId, ndmsgid_t minId, int is_print);
ND_NET_API nd_usermsg_func nd_message_set_print_entry(nd_handle handle, nd_usermsg_func entry);


/*install handler to message map table*/
ND_NET_API int nd_msgentry_install(nd_handle  handle, nd_usermsg_func, ndmsgid_t maxid, ndmsgid_t minid,int level, const char *name) ;

/* install message script-function */
ND_NET_API int nd_msgentry_script_install(nd_handle handle, const char*script, ndmsgid_t maxid, ndmsgid_t minid, int level);
/* set script engine */
ND_NET_API int nd_message_set_script_engine(nd_handle handle, void *script_engine, nd_msg_script_entry entry);

/* get script engine */
ND_NET_API void* nd_message_get_script_engine(nd_handle handle);

/* set default message handler*/
ND_NET_API int nd_msgentry_def_handler(nd_handle handle, nd_usermsg_func func)  ;

ND_NET_API nd_usermsg_func nd_msgentry_get_func(nd_handle handle, ndmsgid_t maxid, ndmsgid_t minid);
ND_NET_API nd_usermsg_func nd_msgentry_get_def_func(nd_handle handle) ;
ND_NET_API int nd_msgentry_is_handled(nd_handle handle, ndmsgid_t maxid, ndmsgid_t minid);

ND_NET_API const char * nd_msgentry_get_name(nd_handle handle, ndmsgid_t maxid, ndmsgid_t minid) ;
ND_NET_API NDUINT32 nd_msgentry_get_id(nd_handle handle, const char *msgname);

ND_NET_API nd_handle nd_get_msg_hadle(nd_handle handle) ;

/* send nd-protocol package */
static __INLINE__ int nd_connectmsg_send(nd_handle  connector_handle, nd_usermsgbuf_t *msg ) 
{
	nd_assert(connector_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(connector_handle, (nd_packhdr_t *)msg, ESF_NORMAL) ;
}

/* post package*/
static __INLINE__ int nd_connectmsg_post(nd_handle  connector_handle, nd_usermsgbuf_t *msg ) 
{
	nd_assert(connector_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(connector_handle, (nd_packhdr_t *)msg, ESF_POST) ;
}

/* send urgen */
static __INLINE__ int nd_connectmsg_send_urgen(nd_handle  connector_handle, nd_usermsgbuf_t *msg ) 
{
	nd_assert(connector_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(connector_handle, (nd_packhdr_t *)msg,  ESF_URGENCY) ;
}

/* sendex*/
static __INLINE__ int nd_connectmsg_sendex(nd_handle  connector_handle, nd_usermsgbuf_t *msg , int flag) 
{
	nd_assert(connector_handle) ;
	nd_assert(msg) ;
	nd_netmsg_hton(msg) ;
	return nd_connector_send(connector_handle, (nd_packhdr_t *)msg,  flag) ;
}

/*
 * translate message to handler that user installed ,
 * @connect_handle connection handle
 * @msg message 
 * this function used in client-connector
 */
ND_NET_API int nd_translate_message(nd_handle  connector_handle, nd_packhdr_t *msg,nd_handle listen_handle ) ;

//ND_NET_API int nd_translate_message_ex(nd_handle owner, nd_packhdr_t *msg, nd_handle listen_handle, nd_handle caller);

/* 
 * used in listener 
 */
ND_NET_API int nd_srv_translate_message(nd_handle  connector_handle, nd_packhdr_t *msg,nd_handle listen_handle ) ;

//set privilege 
ND_NET_API NDUINT32 nd_connect_level_get(nd_handle  connector_handle) ;
ND_NET_API void nd_connect_level_set(nd_handle  connector_handle,NDUINT32 level);

// check message is log 
ND_NET_API int nd_message_is_log(nd_handle nethandle, int maxId, int minId);


// close/disable a group message handler by maxId
ND_NET_API int nd_message_disable_group(nd_handle nethandle, int maxId, int disable);

#endif

