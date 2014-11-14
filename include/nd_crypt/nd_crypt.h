/* file nd_crypt.h
 * define crypt function of nd engine
 * all right reserved 
 * neil duan
 * 2008-5
 * version 1.0
 */

/*
 * This module include MD5, tea and rsa.

  MD5ToString �Ѽ��ܺõ�MD5ת���ɿɴ�ӡ���ַ�
 * char* MD5ToString(unsigned char src[16], unsigned char desc[33]);

  ���ַ������ܳ�MD5
 * char *MD5CryptStr16(char *input, char output[16]) ;

  ��2���Ƽ��ܳ�MD5
 * char *MD5Crypt16(char *input, int inlen, char output[16]);

  ����һ��tea����Կ
 * int tea_key(tea_k *k);

  tea����
 * void tea_enc(tea_k *k, tea_v *v);

  tea����
 * void  tea_dec(tea_k *k, tea_v *v);

  rsa����/����
  ����0�ɹ�
	int nd_RSAcreate(RSA_HANDLE); ����һ�Լ��ܽ�����Կ
 void nd_RSAdestroy(RSA_HANDLE *h_rsa);	//���ټ��ܽ�����Կ
 //���ܻ��߽��ܺ���,������Կ���ܵ�ֻ����˽����Կ����,��֮��Ȼ
 int nd_RSAPublicEncrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa);
 int nd_RSAPrivateEncrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa);
 int nd_RSAPublicDecrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa);
 int nd_RSAPrivateEncrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa);
  
*/

#ifndef _ND_CRYPT_
#define _ND_CRYPT_

#include "nd_common/nd_comcfg.h"

#ifndef CPPAPI
	#ifdef __cplusplus
	#define CPPAPI extern "C" 
	#else 
	#define CPPAPI 
	#endif
#endif 
//#define RSA_COMPILE_AS_DLL	1		//	�����dll

#if  defined(ND_COMPILE_AS_DLL) && (defined(_WINDOWS)  || defined(WIN32) || defined(WIN64))
	#if  defined(ND_CRYPT_EXPORTS) 
	# define ND_CRYPT_API 				CPPAPI __declspec(dllexport)
	#else
	# define ND_CRYPT_API 				CPPAPI __declspec(dllimport)
	#endif
#else 
	# define ND_CRYPT_API 				CPPAPI
#endif 

#include "nd_crypt/tea.h"

#include "nd_crypt/rsah/global.h"
#include "nd_crypt/rsah/rsaref.h"
#include "nd_crypt/rsah/rsa.h"


ND_CRYPT_API int nd_TEAencrypt(unsigned char *data, int data_len, tea_k *key) ;
ND_CRYPT_API int nd_TEAdecrypt(unsigned char *data, int data_len, tea_k *key) ;
ND_CRYPT_API int nd_TEAGenKey(tea_k *key, char *seed) ;		//ʹ����������һ����Կ,�������볤��

/* mix-in data */
ND_CRYPT_API char *crypt_stuff(char *src, int datalen, int stufflen ) ;	/*stuff data to align*/


/* convert crypt string to output string 
 * add by duan !
 */
ND_CRYPT_API char* MD5ToString(unsigned char src[16], unsigned char desc[33]);

/*���ܿɴ�ӡ���ַ�(\0���ַ���)*/
ND_CRYPT_API char *MD5CryptStr16(char *input, char output[16]) ;

/* �����ַ��Ƕ������ַ�
 * @inlen input length
 * @input data address of input
 * @output buffer address char[16]
 */
ND_CRYPT_API char *MD5Crypt16(char *input, int inlen, char output[16]);
/* ����md5,����ɴ�ӡ���ַ�*/
ND_CRYPT_API char *MD5CryptToStr32(char *input, int inlen, char output[33]);
ND_CRYPT_API int MD5cmp(char src[16], char desc[16]) ;

/* rsa crypt*/

typedef struct  {	
	R_RSA_PUBLIC_KEY publicKey;                          /* new RSA public key */
	R_RSA_PRIVATE_KEY privateKey;                       	/* new RSA private key */
	R_RANDOM_STRUCT randomStruct;                        /* random structure */
}ND_RSA_CONTEX ;
typedef ND_RSA_CONTEX *RSA_HANDLE ;

#define RSA_KEY_BITS 2048					/* rsa ��Կ����*/
#define RSA_CRYPT_BUF_SIZE 256				/* rsaÿ�μ�����Ҫ�Ļ��峤��*/

ND_CRYPT_API int RSAinit_random(R_RANDOM_STRUCT *rStruct);
ND_CRYPT_API void RSAdestroy_random(R_RANDOM_STRUCT *rStruct);
ND_CRYPT_API int nd_RSAInit(RSA_HANDLE h_rsa, int keyBits);
ND_CRYPT_API void nd_RSAdestroy(RSA_HANDLE h_rsa);
/* rsa de/encrypt*/
ND_CRYPT_API int nd_RSAPublicEncrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa);
ND_CRYPT_API int nd_RSAPrivateEncrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa);

ND_CRYPT_API int nd_RSAPublicDecrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa);
ND_CRYPT_API int nd_RSAPrivateDecrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa);

ND_CRYPT_API int rsa_pub_encrypt(char *outbuf, int *outlen, char *inbuf, int inlen,R_RSA_PUBLIC_KEY *key);
ND_CRYPT_API int rsa_priv_encrypt(char *outbuf, int *outlen, char *inbuf, int inlen,R_RSA_PRIVATE_KEY *key);

ND_CRYPT_API int rsa_pub_decrypt(char *outbuf, int *outlen, char *inbuf, int inlen,R_RSA_PUBLIC_KEY *key);
ND_CRYPT_API int rsa_priv_decrypt(char *outbuf, int *outlen, char *inbuf, int inlen,R_RSA_PRIVATE_KEY *key);

ND_CRYPT_API int rsa_get_crypt_len(R_RSA_PUBLIC_KEY *key);


ND_CRYPT_API int nd_rsa_privkey_output(R_RSA_PRIVATE_KEY *priv_key, const char *bin_file) ;
ND_CRYPT_API int nd_rsa_privkey_input(R_RSA_PRIVATE_KEY *priv_key, const char *bin_file) ;

ND_CRYPT_API int nd_rsa_pubkey_output(R_RSA_PUBLIC_KEY *pub_key, const char *bin_file) ;
ND_CRYPT_API int nd_rsa_pubkey_input(R_RSA_PUBLIC_KEY *pub_key, const char *bin_file) ;

ND_CRYPT_API int nd_rsa_read_key(R_RSA_PRIVATE_KEY *key , const char * buf, int bufsize, int is_private) ;

ND_CRYPT_API int nd_rsa_test(R_RSA_PRIVATE_KEY *priv_key, R_RSA_PUBLIC_KEY *pub_key) ;

#endif	//_ND_CRYPT_
