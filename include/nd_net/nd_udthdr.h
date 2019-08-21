/* file nd_udthdr.h
 * define pocket header of nd UDP data transfer (UDT POROTOCAL)   
 *
 * author : neil 
 * 2005-11-23
 * version 1.0 
 */

#ifndef _ND_UDTHDR_H_
#define _ND_UDTHDR_H_

#include "nd_net/byte_order.h"
#include "nd_net/nd_udp.h"

#define NDUDT_VERSION 			1
#define NDUDT_FRAGMENT_LEN		1400  //reference rfc 1122 not include header
#define NDUDT_BUFFER_SIZE		NDUDT_FRAGMENT_LEN //封包缓冲

//#define CHECK_LOST_PACKET		//检测丢包情况
//定义UDT报文类型
enum eUdtMsgType{
	NDUDT_DATA=0,
	NDUDT_SYN,
	NDUDT_ALIVE,
	NDUDT_ACK,
	NDUDT_LOST,				//通知对方丢包
	NDUDT_FIN,
	NDUDT_RESET,
	NDUDT_DGRAM				//不可靠的报文,对UDP简单的封装
};

#define NOT_ACK_SPACE		//不使用ack_seq作为发送数据的缓冲

#pragma pack(push)
#pragma pack(1)

//udt协议报头
//32bits
struct ndudt_header
{	
	
#  if ND_BYTE_ORDER == ND_L_ENDIAN
	//0~15 bit
	u_8	version:4;			//版本信息(没有使用,主要是为了保留对齐而以)
	u_8	protocol:4;			//协议类型

	u_8	udt_type:4;			//报文类型
	u_8	stuff:1;			//否为了加密而填充(udt中不使用
	u_8	crypt:1;			//是否加密(udt中不使用
	u_8	ack:1;				//是否包含回复
	u_8	isresend:1;			//this packet is retranslated
#else 
	//0~15 bit
	u_8	protocol:4;			//协议类型
	u_8	version:4;			//版本信息(没有使用,主要是为了保留对齐而以)

	u_8	isresend : 1;		//this packet is retranslated
	u_8	ack:1;				//是否包含回复
	u_8	crypt:1;			//是否加密(udt中不使用
	u_8	stuff:1;			//否为了加密而填充(udt中不使用
	u_8	udt_type:4;			//报文类型

#endif

	//16~31 bits
	u_16	checksum;
};
#ifdef _MSC_VER
#pragma warning (disable: 4200)
#endif
//16 bytes
//udt封包头部
struct ndudt_pocket
{
	struct ndudt_header header;
	u_16	local_port ;			//(port of udt protocol,local port not system port)
	u_16	window_len;			//slide window lenght
	u_32	sequence ;			//sender sequence 发送系列号(当前封包系列)
	
	//32bits * 3
	u_32	ack_seq;			//确认接收系列
	u_8		data[0];
};

//不包括ack的数据包头
struct _ndudt_unack_packet 
{
	struct ndudt_header header;
	u_16	udt_local_port;			//(port of udt protocol, session id)
	u_16	window_len;			//slide window lenght
	u_32	sequence ;			//sender sequence 发送系列号

};
//udp数据包
struct ndudp_packet
{
	struct ndudt_header header;		//32bits
	u_16	udt_local_port;			//(port of udt protocol, session id)
	//32bits + 16bits
	u_8		data[0] ;
};


#pragma pack(pop)
#define UDP_PACKET_HEAD_SIZE sizeof(struct ndudp_packet)



//get pocket data addr
static __INLINE__ char* pocket_data(struct ndudt_pocket *pocket)
{
	if(pocket->header.ack) {
		return (char*)(pocket->data );
	}else {
		return(char*) &(pocket->ack_seq);
	}
}

static __INLINE__ void set_pocket_ack(struct ndudt_pocket *pocket,u_32 ack)
{
	pocket->header.ack = 1 ;
	pocket->ack_seq = ack ;
}


//get pocket header size
static __INLINE__ int ndt_header_size(struct ndudt_pocket *pocket)
{
	if(pocket->header.ack) {
		return sizeof(struct ndudt_pocket);
	}else {
		return sizeof(struct _ndudt_unack_packet);
	}
}


#pragma pack(push)
#pragma pack(1)

typedef union pocket_buffer
{
	struct ndudt_pocket pocket ;
	u_8		_buffer[2048] ;
}udt_pocketbuf ;

#pragma pack(pop)

struct udt_packet_info
{
	NDUINT32 data_len; //include packet header
	struct sockaddr_in6 addr;
	udt_pocketbuf packet;
};

static __INLINE__ int _udt_packet_info_size(struct udt_packet_info *packet)
{
	return sizeof(*packet) - sizeof(packet->packet) + packet->data_len ;
}

static __INLINE__ void init_udt_header(struct ndudt_header *hdr)
{
	*(u_32 *) hdr = 0;
	hdr->protocol=PROTOCOL_UDT ;
}

static __INLINE__ void init_udt_pocket(struct ndudt_pocket *pocket)
{
	//memset(pocket, 0, sizeof(*pocket));
	init_udt_header(&pocket->header);
	pocket->local_port = 0;
	pocket->window_len = 0;
	pocket->sequence =0 ;
}

#define POCKET_GETPORT(pocket) (pocket)->local_port
#define POCKET_CHECKSUM(pocket) (pocket)->header.checksum
#define POCKET_PROTOCOL(pocket) (pocket)->header.protocol
#define POCKET_TYPE(pocket)		(pocket)->header.udt_type
#define POCKET_ACK(pocket)		(pocket)->header.ack

//SET  pocket type
#define SET_SYN(pocket) POCKET_TYPE(pocket)=NDUDT_SYN 
#define SET_ALIVE(pocket) POCKET_TYPE(pocket)=NDUDT_ALIVE
#define SET_FIN(pocket) POCKET_TYPE(pocket)=NDUDT_FIN
#define SET_RESET(pocket) POCKET_TYPE(pocket)=NDUDT_RESET
#define SET_ACK(pocket) POCKET_TYPE(pocket)=NDUDT_ACK
#define SET_LOST(pocket) POCKET_TYPE(pocket)=NDUDT_LOST

static __INLINE__ void _udt_host2net(struct ndudt_pocket *pock)
{
	pock->header.checksum = htons(pock->header.checksum) ;
	pock->local_port = htons(pock->local_port);
	pock->window_len= htons(pock->window_len);
	pock->sequence= htonl(pock->sequence);
	if(pock->header.ack) {
		pock->ack_seq = htonl(pock->ack_seq);
	}
}

static __INLINE__ void _udt_net2host(struct ndudt_pocket *pock)
{
	pock->header.checksum = ntohs(pock->header.checksum);
	pock->local_port = ntohs(pock->local_port);
	pock->window_len = ntohs(pock->window_len);
	pock->sequence = ntohl(pock->sequence);
	if (pock->header.ack) {
		pock->ack_seq = ntohl(pock->ack_seq);
	}
}

//#if ND_BYTE_ORDER == ND_B_ENDIAN
#define udt_host2net(pocket)  _udt_host2net(pocket)
#define udt_net2host(pocket)  _udt_net2host(pocket)

// #else 
// #define udt_host2net(pocket)  //nothing to be done
// #define udt_net2host(pocket)  //nothing to be done
// 
// #endif

#endif 
