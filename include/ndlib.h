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

struct nd_proxy_info
{
	int proxy_type;
	short proxy_port;
	char proxy_host[128];

	char user[64];
	char password[64];
};

#include "nd_define.h"
#include "nd_comdef.h"

typedef unsigned int ndip_t;
typedef NDUINT8 ndmsgid_t;
typedef void *nd_handle;

#pragma pack(push)
#pragma pack(1)

typedef struct nd_packhdr_t
{
	NDUINT16	length;
	NDUINT8		version;
	NDUINT8		_stuff;
}nd_packhdr_t;

typedef struct nd_usermsghdr_t
{
	nd_packhdr_t	packet_hdr ;
	ndmsgid_t	maxid;
	ndmsgid_t	minid;
}nd_usermsghdr_t;

typedef struct nd_usermsgbuf_t
{
	nd_usermsghdr_t msg_hdr;
	char			data[0x10000 - sizeof(nd_usermsghdr_t)];
}nd_usermsgbuf_t;
#pragma pack(pop)


#ifdef __cplusplus
#define CPPAPI extern "C"
#else 
#define CPPAPI 
#endif

#define ND_COMMON_API 				CPPAPI 
#define ND_CONNCLI_API 				CPPAPI 


#include "nd_msgpacket.h"
#include "nd_iconn.h"

#else 

#include "ndcli/nd_iconn.h"

#endif


#endif
