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
/* 加密/解密函数
 * input : @data 被加密的数据
 *			@len 被加密数据的长度
 *			@key加密密钥
 * output : @data 加密后的数据
 * 加密时如果数据长度不够加密的要求,则在被加密的数据后面补充空格,最后一个字符记录填充数据的长度
 * 如果数据刚好,则不需要填充任何东西.
 * return value :on error return 0, else return data length of encrypted
 * 注意:解密后数据不会比解密前长
 */
typedef int (*nd_netcrypt)(unsigned char *data, int len, void *key) ;

/*设置加密函数和加密单元长度*/
ND_NET_API void nd_net_set_crypt(nd_netcrypt encrypt_func, nd_netcrypt decrypt_func,int crypt_unit) ;

#endif
