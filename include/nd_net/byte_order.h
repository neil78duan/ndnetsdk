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

#define ND_BYTE_ORDER 1 

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
#define nd_btols(x)    ((short)( \
        (((short)(x) & (short)0x00ff) << 8) | \
        (((short)(x) & (short)0xff00) >> 8) ))
        
#define nd_btoll(x) 	((int)( \
        (((int)(x) & (int)0x000000ff) << 24) | \
        (((int)(x) & (int)0x0000ff00) << 8) | \
        (((int)(x) & (int)0x00ff0000) >> 8) | \
        (((int)(x) & (int)0xff000000) >> 24) ))

//Сβ�� ת����β��
#define nd_ltobs(x) 	((short)( \
        (((short)(x) & (short)0x00ff) << 8) | \
        (((short)(x) & (short)0xff00) >> 8) ))
        
#define nd_ltobl(x) 	((int)( \
        (((int)(x) & (int)0x000000ff) << 24) | \
        (((int)(x) & (int)0x0000ff00) << 8) | \
        (((int)(x) & (int)0x00ff0000) >> 8) | \
        (((int)(x) & (int)0xff000000) >> 24) ))


#endif 
