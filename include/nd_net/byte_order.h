/* file byte_order.h
 * define host byte order 
 * little endian or big endian
 * 2008-9-3
 * author duan
 */
 
#ifndef _BYTE_ORDER_H_
#define _BYTE_ORDER_H_

/*
大端和小端

Big Endian:  高位字节放到内存的低位地址，反之亦然。以太网网络传输字节序, PowerPC, UltraSparc一类的处理器采用大端。

Little Endian: Intel的IA-32架构采用。高位字节放到内存高位地址。记得学X86结构是有一记忆口令：High high, low low

比特位顺序一般和字节序的端模式相同，但是这涉及硬件连线方式，一般软件设计不需要管。
注意，C语言中的位域结构也要遵循端模式。
例如：
struct  edtest
{
uchar a : 2;
uchar b : 6;
}
该位域结构占1个字节，假设赋值 a = 0x01; b=0x02;
大端机器上该字节为： (01)(000010)
小端机器上：                 (000010)(01)
因此在编写可移植代码时，需要加条件编译。

网络发送是使用的是大位数,从左到右的顺序发送
bit0是最右端的，bit7是最左端的
*/

#define ND_L_ENDIAN  1
#define ND_B_ENDIAN  0


#ifdef __ND_ANDROID__

	#if defined(__mips__) || defined(__mips64__)
		#define ND_BYTE_ORDER 0
	#else
		#define ND_BYTE_ORDER 1
	#endif

#else
	#define ND_BYTE_ORDER 1
#endif //end android



#ifndef u_8
typedef unsigned char u_8;
#endif 

#ifndef u_16 
typedef unsigned short u_16;
#endif 

#ifndef u_32
typedef unsigned int u_32;
#endif 

//get byte order , return 0 bigend 
static __inline int nd_byte_order() 
{
	int a = 1 ;
	char *p = (char*)&a ;
	return (int)p[0] ;
}

//big endian to little endian
#define nd_btols(_a)    ((unsigned short)( \
	(((unsigned short)(_a) & (unsigned short)0x00ff) << 8) | \
	(((unsigned short)(_a) & (unsigned short)0xff00) >> 8) ))



#define nd_btoll(_a) 	((unsigned int)( \
	(((unsigned int)(_a) & (unsigned int)0x000000ff) << 24) | \
	(((unsigned int)(_a) & (unsigned int)0x0000ff00) << 8) | \
	(((unsigned int)(_a) & (unsigned int)0x00ff0000) >> 8) | \
	(((unsigned int)(_a) & (unsigned int)0xff000000) >> 24) ))


static __inline unsigned long long  nd_btolll(unsigned long long val)
{
	union {
		unsigned long long val;
		char _data[8];
	}v1 , v2 ;
	v2.val = val;

	v1._data[7] = v2._data[0];
	v1._data[6] = v2._data[1];
	v1._data[5] = v2._data[2];
	v1._data[4] = v2._data[3];

	v1._data[3] = v2._data[4];
	v1._data[2] = v2._data[5];
	v1._data[1] = v2._data[6];
	v1._data[0] = v2._data[7];

	return v1.val;

}

//little to big
#define nd_ltobs(x) 	nd_btols(x)
        
#define nd_ltobl(x) 	nd_btoll(x) 

#define nd_order_change_short(x) nd_btols(x)
#define nd_order_change_long(x) nd_btoll(x)
#define nd_order_change_longlong(x) nd_btolll(x)


#endif 
