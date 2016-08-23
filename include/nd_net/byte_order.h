/* file byte_order.h
 * define host byte order 
 * little endian or big endian
 * 2008-9-3
 * author duan
 */
 
#ifndef _BYTE_ORDER_H_
#define _BYTE_ORDER_H_

/*
��˺�С��

Big Endian:  ��λ�ֽڷŵ��ڴ�ĵ�λ��ַ����֮��Ȼ����̫�����紫���ֽ���, PowerPC, UltraSparcһ��Ĵ��������ô�ˡ�

Little Endian: Intel��IA-32�ܹ����á���λ�ֽڷŵ��ڴ��λ��ַ���ǵ�ѧX86�ṹ����һ������High high, low low

����λ˳��һ����ֽ���Ķ�ģʽ��ͬ���������漰Ӳ�����߷�ʽ��һ�������Ʋ���Ҫ�ܡ�
ע�⣬C�����е�λ��ṹҲҪ��ѭ��ģʽ��
���磺
struct  edtest
{
uchar a : 2;
uchar b : 6;
}
��λ��ṹռ1���ֽڣ����踳ֵ a = 0x01; b=0x02;
��˻����ϸ��ֽ�Ϊ�� (01)(000010)
С�˻����ϣ�                 (000010)(01)
����ڱ�д����ֲ����ʱ����Ҫ���������롣

���緢����ʹ�õ��Ǵ�λ��,�����ҵ�˳����
bit0�����Ҷ˵ģ�bit7������˵�
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

//�õ���ǰ�ֽ�˳��return 0 bigend 
static __inline int nd_byte_order() 
{
	int a = 1 ;
	char *p = (char*)&a ;
	return (int)p[0] ;
}

//��β��ת��Сβ��
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

//Сβ�� ת����β��
#define nd_ltobs(x) 	nd_btols(x)
        
#define nd_ltobl(x) 	nd_btoll(x) 

#define nd_order_change_short(x) nd_btols(x)
#define nd_order_change_long(x) nd_btoll(x)
#define nd_order_change_longlong(x) nd_btolll(x)


#endif 
