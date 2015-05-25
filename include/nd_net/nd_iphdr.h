/* file nd_iphdr.h
 * define tcp/ip udp icmp header 
 * 2008-9-6
 * neil duan 
 * all right reserved 
 */
 
#ifndef _ND_IPHDR_H_
#define _ND_IPHDR_H_

#include "nd_net/byte_order.h"

#define RAW_SYSTEM_BUF	0x2000			//8K

#define	ND_IPVERSION	4               /* IP version number */
#define	ND_IP_MAXPACKET	65535			/* maximum packet size */
#define ND_DFT_ICMP_PID 65534

enum {

    ND_IPPROTO_IP = 0,	   /* Dummy protocol for TCP.  */
    ND_IPPROTO_ICMP = 1,	   /* Internet Control Message Protocol.  */
    ND_IPPROTO_IGMP = 2,	   /* Internet Group Management Protocol. */
    ND_IPPROTO_TCP = 6,	   /* Transmission Control Protocol.  */
    ND_IPPROTO_UDP = 17	   /* User Datagram Protocol.  */
} ;


enum {
	ICMP_ECHOREPLY= 0, 

	ICMP_DESTUNREACH = 3,
	ICMP_SRCQUENCH  = 4,
	ICMP_REDIRECT =  5,
	ICMP_ECHO = 8 ,
	ICMP_TIMEOUT = 11,
	ICMP_PARMERR  = 12
};

#define ICMP_MIN 8
#define ND_TTL	 128

#pragma pack(push)
#pragma pack(1)
typedef struct _iphdr
{
#  if ND_BYTE_ORDER == ND_L_ENDIAN
	u_8 ip_hl:4;			/* header length */
	u_8 ip_v:4;				/* version */
#elif ND_BYTE_ORDER == ND_B_ENDIAN
	u_8 ip_v:4;				/* version */
	u_8 ip_hl:4;			/* header length */
#endif
	u_8 ip_tos;				/* type of service */
	u_16 ip_len;			/* total length */
	u_16 ip_id;				/* identification */
	u_16 ip_off;			/* fragment offset field */
#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
	u_8 ip_ttl;					/* time to live */
	u_8 ip_p;					/* protocol */
	u_16 ip_sum;				/* checksum */
	u_32 ip_src;
	u_32 ip_dst;		/* source and dest address */
}ip_hdr;
//tcp头

typedef struct nd_tcphdr
 {
    u_16 source;
    u_16 dest;
    u_32 seq;
    u_32 ack_seq;
#  if ND_BYTE_ORDER == ND_L_ENDIAN
    u_8 res1:4;
    u_8  doff:4;
    u_8  fin:1;
    u_8  syn:1;
    u_8  rst:1;
    u_8  psh:1;
    u_8  ack:1;
    u_8  urg:1;
    u_8  res2:2;
#  elif ND_BYTE_ORDER == ND_B_ENDIAN
    u_8  doff:4;
    u_8  res1:4;
    u_8  res2:2;
    u_8  urg:1;
    u_8  ack:1;
    u_8  psh:1;
    u_8  rst:1;
    u_8  syn:1;
    u_8  fin:1;
#  else
#   error "unknow CPU arch"
#  endif
    u_16 window;
    u_16 check;
    u_16 urg_ptr;
}tcp_hdr;

//udp头
typedef struct _udp_hdr
{
    u_16 src_port;       // Source port no.
    u_16 dest_portno;       // Dest. port no.
    u_16 length;       // Udp packet length
    u_16 checksum;     // Udp checksum (optional)
} udp_hdr;


// ICMP header
typedef struct _icmp_hdr
{
    u_8   icmp_type;
    u_8   icmp_code;
    u_16  icmp_checksum;
    u_16  icmp_id;
    u_16  icmp_sequence;
    u_32   icmp_timestamp;
} icmp_hdr;

//udp伪头部,用来计算CHECK SUM
typedef struct _psuedo_udp_hdr
{
	u_32 src_addr;
	u_32 dest_addr;
	u_8 reserved;
	u_8 protocol;
	u_16 udp_length;
	udp_hdr udp ;
}psuedo_udp;

//tcp伪头部,用来计算CHECK SUM
typedef struct pseudo_tcp_header
{
	u_32 src_addr;
	u_32 dest_addr;
	u_8 reserved;
	u_8 protocol;
	u_16 tcp_length;
	tcp_hdr tcp;
} pseudo_tcp;

//tcp伪头部,用来计算CHECK SUM
typedef struct pseudo_icmp_header
{
	u_32 src_addr;
	u_32 dest_addr;
	u_8 reserved;
	u_8 protocol;
	u_16 icmp_length;
	icmp_hdr icmp;
} pseudo_icmp;

#pragma pack(pop)

#endif
