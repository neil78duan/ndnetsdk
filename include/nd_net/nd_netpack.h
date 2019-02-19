/* file : nd_netpack.h
 * define nd engine net packet
 *
 * 2007-12-17 
 *
 * author neil
 * all right reserved 
 *
 */

#ifndef _ND_NETPACK_H_
#define _ND_NETPACK_H_

#include "nd_net/byte_order.h"
#include "nd_common/nd_define.h"
#include "nd_common/nd_export_def.h"
#include <stdlib.h>
#include <stdio.h>
//#include "nd_common/nd_common.h"

//version 2
#define NDNETMSG_VERSION		2

#define ND_PACKET_SIZE 0x10000

#pragma pack(push)
#pragma pack(1)

/* package header struct define */
typedef struct packet_hdr
{
	NDUINT16	length ;				/*length of packet*/
	NDUINT8		version ;				/*version of net protocol*/
#if ND_BYTE_ORDER == ND_L_ENDIAN
	NDUINT8		ndsys_msg:1;			/* system message*/
	NDUINT8		encrypt:1;				/* the package is crypt */
	NDUINT8		stuff:1;				/* is filled data on tail when crypt*/
	NDUINT8		stuff_len:5;			/* fill length*/
#else 
	NDUINT8		stuff_len:5;			
	NDUINT8		stuff:1;				
	NDUINT8		encrypt:1;				
	NDUINT8		ndsys_msg:1;			
#endif
	
} nd_packhdr_t;

//packet header size 
#define ND_PACKET_HDR_SIZE  sizeof(nd_packhdr_t)
#define ND_PACKET_DATA_SIZE (ND_PACKET_SIZE - ND_PACKET_HDR_SIZE)

//packet header
typedef struct nd_net_packet_buf
{
	nd_packhdr_t	hdr ;
	char			data[ND_PACKET_DATA_SIZE] ;
}nd_packetbuf_t;

//nd common packet
typedef struct nd_sysresv_pack
{
	nd_packhdr_t hdr ;
	NDUINT16 msgid ;
	NDUINT16 checksum;
}nd_sysresv_pack_t;
// the message id reserved 
enum {
	ERSV_ALIVE_ID = 0xfb0e,
	ERSV_VERSION_ERROR = 0xfb0f
};
#pragma pack(pop)

/* send type , flag */
enum send_flag {
	ESF_NORMAL = 0 ,		//normal send data
	ESF_WRITEBUF =1,		//write data to send buffer
	ESF_URGENCY = 2,		//send data right now
	ESF_POST	= 4,		//post 
	ESF_ENCRYPT = 8			//crypt send
};

static __INLINE__ void nd_hdr_init(nd_packhdr_t *hdr)
{
	*((NDUINT32*)hdr) = 0 ;
	hdr->version=NDNETMSG_VERSION ;
}

static __INLINE__ int nd_pack_version(nd_packhdr_t *hdr)
{
	return (int) hdr->version ;
}


/*convert header from net to host*/
#define nd_pack_len(hdr) (hdr)->length 

/*convert header from net to host*/
static __INLINE__ void nd_set_pack_len(nd_packhdr_t *hdr, NDUINT16 len)
{
	hdr->length = len;
}

// hdrdest = hdrsrc
#define ND_HDR_SET(hdrdest, hdrsrc) *((NDUINT32*)hdrdest) = *((NDUINT32*)hdrsrc)  

typedef int (*NDNET_MSGENTRY)(void *connect, nd_packhdr_t *msg_hdr, void *param) ;	//packet handle function 


//------------------------------begin define message data struct------------------------------------------


//client connect status 
enum  privilege_level {
	EPL_NONE = 0,			//none, not connect
	EPL_CONNECT,			//connected in
	EPL_LOGIN,				//login success
	EPL_READY,				//start game
	EPL_HIGHT				//hight privelete
};

#define SUB_MSG_NUM		64		//message sub class number
#define MAX_MAIN_NUM	256		//message mian class capacity
#define MAX_SCRIPT_NAME 64		//message script name size
//#define ND_DFT_MAXMSG_NUM 32	//default main message number

typedef NDUINT8	ndmsgid_t;		//message id type

//#define ND_UNSUPPORT_SCRIPT_MSG		//

#pragma pack(push)
#pragma pack(1)
//net message package hander of nd-protocol 
typedef struct nd_usermsghdr_t
{
	nd_packhdr_t	packet_hdr;		//消息包头
	ndmsgid_t		maxid;		//主消息号 8bits
	ndmsgid_t		minid;		//次消息号 8bits
//	ndmsgparam_t	param;		//消息参数
}nd_usermsghdr_t;

#define ND_USERMSG_HDRLEN sizeof(nd_usermsghdr_t)
//user data capacity in nd-prorocol
#define ND_USERMSG_DATA_CAPACITY  (ND_PACKET_SIZE-sizeof(nd_usermsghdr_t) )		
//message buffer 
typedef struct nd_usermsgbuf_t
{
	nd_usermsghdr_t msg_hdr;
	char			data[ND_USERMSG_DATA_CAPACITY];
}nd_usermsgbuf_t;

#pragma pack(pop)

static __INLINE__ void nd_usermsghdr_init(nd_usermsghdr_t *hdr)
{
	memset(hdr, 0, sizeof(*hdr));
	hdr->packet_hdr.length = ND_USERMSG_HDRLEN;
	hdr->packet_hdr.version = NDNETMSG_VERSION;
}

#define ND_USERMSG_INITILIZER {{ND_USERMSG_HDRLEN,NDNETMSG_VERSION,0,0,0},0,0,0} 
#define nd_netmsg_hton(m)		//net message byte order to host  
#define nd_netmsg_ntoh(m)		//host to net

#define ND_USERMSG_LEN(m)	((nd_packhdr_t*)m)->length
#define ND_USERMSG_MAXID(m)	((nd_usermsghdr_t*)m)->maxid 
#define ND_USERMSG_MINID(m)	((nd_usermsghdr_t*)m)->minid 
#define ND_USERMSG_PARAM(m)	
#define ND_USERMSG_DATA(m)	(((nd_usermsgbuf_t*)m)->data)
#define ND_USERMSG_DATALEN(m)	(((nd_packhdr_t*)m)->length - ND_USERMSG_HDRLEN)
#define ND_USERMSG_SYS_RESERVED(m) ((nd_packhdr_t*)m)->ndsys_msg 

#endif
