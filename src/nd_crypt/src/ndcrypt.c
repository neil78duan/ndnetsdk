/* file ndcrypt.c
 * implemention user function interface of nd crypt
 * neil duan
 * 2008-5
 */

//#include "nd_common/nd_common.h"
#include "nd_crypt/nd_crypt.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
#if defined _DEBUG || defined DEBUG
#pragma comment(lib, "nd_common_dbg.lib")
#else 
#pragma comment(lib, "nd_common.lib")

#endif
*/

/* stuff src 
 * input :  src stuffed data 
 			dataln current data length
 			stufflen target data len 
 * return : data start address 
 */
#define STUFF_ELE	(char)0x20
char *crypt_stuff(char *src, int datalen, int stufflen )
{
	int i ;
	char *p = src ;
	if( stufflen <1 || stufflen <= datalen){
		return 0 ;
	}
	src += datalen ;
	stufflen -= datalen ;
	//memset(src, STUFF_ELE , stufflen - datalen ) ;	//stuff STUFF_ELE to buffer between src+datalen and src+datalen
	//p[stufflen-1] = (char)(stufflen - datalen) ;	//recrod stuff length 
	
	for(i=0; i<stufflen -1; i++){
		*src = STUFF_ELE ;src++ ;
	}
	*src = stufflen ;	//recrod stuff length 
	return p ;
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
int nd_TEAencrypt(unsigned char *data, int data_len, tea_k *key) 
{
	int vlen = sizeof(tea_v) ;			/*加密单元的长度*/
	int stuff_len ,i,n,new_len;
	
	tea_v *v = (tea_v *)data;

	stuff_len = data_len % vlen ;
	n = data_len / vlen ;
	
	if(stuff_len){
		++n ;
		new_len = n * vlen;
		crypt_stuff(data, data_len, new_len ) ;
	}
	else {
		new_len = data_len ;
	}
	for(i=0; i<n; i++){
		tea_enc(key, v) ;
		++v ;
	}
	return new_len ;
}

int nd_TEAdecrypt(unsigned char *data, int data_len, tea_k *key) 
{
	int vlen = sizeof(tea_v) ;			/*加密单元的长度*/
	int i,n;
	
	tea_v *v = (tea_v *)data;
	
#if 1 //defined _DEBUG || defined DEBUG
	int stuff_len = data_len % vlen ;
	if(stuff_len){
		//nd_assert(0);		
		return -1 ;
	}
#endif

	n = data_len / vlen ;
	
	for(i=0; i<n; i++){
		tea_dec(key, v) ;
		++v ;
	}
	
	return data_len ;
}

int nd_TEAGenKey(tea_k *key, char *seed)
{
	//nd_assert(sizeof(tea_k)==16) ;
	MD5CryptStr16(seed,(char*)key) ; 
	return 16 ;
}
/* convert crypt string to output string 
 * add by duan !
 */
char* MD5ToString(unsigned char src[16], unsigned char desc[33])
{
	unsigned int i ;
	char *p = desc ;
	for(i=0; i<16; i++)
	{
		sprintf(p, "%02x",src[i]) ;
		p += 2 ;
	}
	desc[32] = 0 ;
	return desc ;
}


/*加密可打印的字符(\0的字符串)*/
char *MD5CryptStr16(char *input, char output[16]) 
{
	MD5_CTX context;
	unsigned int len = (unsigned int) strlen (input);
	
	MD5Init (&context);
	MD5Update (&context, input, len);
	MD5Final (output, &context);
	
	return output ;
}

/* 输入字符是二进制字符
 * @inlen input length
 * @input data address of input
 * @output buffer address char[16]
 */
char *MD5Crypt16(char *input, int inlen, char output[16])
{
	MD5_CTX context;
	
	MD5Init (&context);
	MD5Update (&context, input, inlen);
	MD5Final (output, &context);
	
	return output ;
}

char *MD5CryptToStr32(char *input, int inlen, char output[33])
{
	char tmp[16] ;
	MD5_CTX context;
	
	MD5Init (&context);
	MD5Update (&context, input, inlen);
	MD5Final (tmp, &context);
	
	MD5ToString(tmp, output);
	return output ;
}

int MD5cmp(char src[16], char desc[16])
{
	int ret = 0 ;
	UINT4 *s4 =(UINT4 *) src ;
	UINT4 *d4 =(UINT4 *) desc ;

	ret = s4[0] - d4[0] ;
	if(ret)
		return ret ;
	ret = s4[1] - d4[1] ;
	if(ret)
		return ret ;
	ret = s4[2] - d4[2] ;
	if(ret)
		return ret ;
	ret = s4[3] - d4[3] ;
	return ret ;
}
