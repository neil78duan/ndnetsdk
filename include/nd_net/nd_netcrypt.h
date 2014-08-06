/* file nd_netcrypt.h
 * define io crpyt function
 * 
 *
 * neil duan 
 * all right reserved 
 * 2008-5-18
 */

#ifndef _ND_NETCRPYT_ 
#define _ND_NETCRPYT_

#include "nd_common/nd_common.h"

#define CRYPT_KEK_SIZE 16

/*define nd net crypt function and struct*/
typedef struct _crypt_key
{
	int size ;
	unsigned char key[CRYPT_KEK_SIZE] ;
}nd_cryptkey;

static __INLINE__ void init_crypt_key(nd_cryptkey *key)
{
	key->size = 0;
}

static __INLINE__ int is_valid_crypt(nd_cryptkey *key)
{
	return (key->size > 0) ;
}
/* ����/���ܺ���
 * input : @data �����ܵ�����
 *			@len ���������ݵĳ���
 *			@key������Կ
 * output : @data ���ܺ������
 * ����ʱ������ݳ��Ȳ������ܵ�Ҫ��,���ڱ����ܵ����ݺ��油��ո�,���һ���ַ���¼������ݵĳ���
 * ������ݸպ�,����Ҫ����κζ���.
 * return value :on error return 0, else return data length of encrypted
 * ע��:���ܺ����ݲ���Ƚ���ǰ��
 */
typedef int (*nd_netcrypt)(unsigned char *data, int len, void *key) ;

/*���ü��ܺ����ͼ��ܵ�Ԫ����*/
ND_NET_API void nd_net_set_crypt(nd_netcrypt encrypt_func, nd_netcrypt decrypt_func,int crypt_unit) ;

#endif
