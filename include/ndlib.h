/* file ndlib.h
 *
 * define client connect for other engine 
 * 
 * create by duan 
 *
 * 2015-6-17
 */

#ifndef _NDLIB0_H_
#define _NDLIB0_H_
// 
// #ifdef __UNREAL__
// #define BUILD_AS_THIRD_PARTY 1 
// 
// #if !(PLATFORM_WINDOWS==1)
// #define ND_UNIX
// #endif 
// 
// #endif

#if defined(BUILD_AS_THIRD_PARTY)

#include "nd_common/nd_export_def.h"
#include "nd_common/nd_define.h"
#include "nd_common/nd_handle.h"
#include "nd_net/nd_netioctl.h"

#include "ndapplib/nd_msgpacket.h"
#include "nd_net/byte_order.h"
// 
// #ifndef nd_proxy_info
// struct nd_proxy_info
// {
// 	int proxy_type;
// 	short proxy_port;
// 	char proxy_host[128];
// 
// 	char user[64];
// 	char password[64];
// };
// #endif 

//typedef NDUINT8 ndmsgid_t;
//typedef void *nd_handle;
// 
// #pragma pack(push)
// #pragma pack(1)
// 
// typedef struct nd_packhdr_t
// {
// 	NDUINT16	length;
// 	NDUINT8		version;
// 	NDUINT8		_stuff;
// }nd_packhdr_t;
// 
// typedef struct nd_usermsghdr_t
// {
// 	nd_packhdr_t	packet_hdr ;
// 	ndmsgid_t	maxid;
// 	ndmsgid_t	minid;
// }nd_usermsghdr_t;
// 
// typedef struct nd_usermsgbuf_t
// {
// 	nd_usermsghdr_t msg_hdr;
// 	char			data[0x10000 - sizeof(nd_usermsghdr_t)];
// }nd_usermsgbuf_t;
// #pragma pack(pop)



typedef const char* (*nd_error_convert)(int errcode)  ;

ND_COMMON_API const char *nd_error_desc(int errcode);
ND_COMMON_API nd_error_convert nd_register_error_convert(nd_error_convert func);

#define NDMIN(a,b) ((a) < (b) ? (a) : (b))
#define NDMAX(a,b) ((a) > (b) ? (a) : (b))

#else 


#endif

#include "ndcli/nd_iconn.h"

#endif
