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
//#include "nd_common/nd_common.h"

//版本2 把 typedef NDUINT16	ndmsgid_t  该成 typedef NDUINT8	ndmsgid_t 
#define NDNETMSG_VERSION		2

#define ND_PACKET_SIZE C_BUF_SIZE

#pragma pack(push)
#pragma pack(1)

/*定义消息包头部*/
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
	ESF_NORMAL = 0 ,		//正常发送
	ESF_WRITEBUF =1,		//写入发送缓冲
	ESF_URGENCY = 2,		//紧急发送
	ESF_POST	= 4,		//不可靠的发送(可能回丢失)
	ESF_ENCRYPT = 8			//加密(可以和其他位连用|)
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

///*convert header from host to net*/
//static __INLINE__ void nd_hdr_hton(nd_packhdr_t *hdr)
//{
//	//nothing to be done !
//	return ;
//}
//
///*convert header from net to host*/
//static __INLINE__ void nd_hdr_ntoh(nd_packhdr_t *hdr)
//{
//	//nothing to be done !
//	return ;
//}

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

static __INLINE__ void nd_make_alive_pack(nd_sysresv_pack_t *pack)
{
	nd_hdr_init(&pack->hdr) ;
	pack->hdr.length = sizeof(nd_sysresv_pack_t) ;
	pack->hdr.ndsys_msg = 1;
	pack->hdr.stuff_len = 5 ;
	pack->msgid = ERSV_ALIVE_ID ;
	pack->checksum = 0;
	pack->checksum = nd_checksum((NDUINT16 *)pack,sizeof(nd_sysresv_pack_t) ) ;
}
//#define  msg_hton //nothing
//#define msg_ntoh //nothing

static __INLINE__ void nd_packet_hton(nd_packhdr_t *hdr)
{
    hdr->length = htons(hdr->length) ;
}

static __INLINE__ void nd_packet_ntoh(nd_packhdr_t *hdr)
{
    hdr->length = ntohs(hdr->length) ;
}

#define packet_hton(p) nd_packet_hton((nd_packhdr_t *)(p))
#define packet_ntoh(p) nd_packet_ntoh((nd_packhdr_t *)(p))

#endif
