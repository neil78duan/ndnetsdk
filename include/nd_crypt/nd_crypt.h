/* file nd_crypt.h
 * define crypt function of nd engine
 * all right reserved 
 * neil duan
 * 2008-5
 * version 1.0
 */

/*
 * This module include MD5, tea and rsa.

  MD5ToString transfer MD5 key to printable text
 * char* MD5ToString(unsigned char src[16], unsigned char desc[33]);

  get md5 key from text
 * char *MD5CryptStr16(char *input, char output[16]) ;

  get md5 key from binary
 * char *MD5Crypt16(char *input, int inlen, char output[16]);

  generate a tea key
 * int tea_key(tea_k *k);

  tea encrypt
 * void tea_enc(tea_k *k, tea_v *v);

  tea decrypt
 * void  tea_dec(tea_k *k, tea_v *v);

  rsa
  return 0 success
	int nd_RSAcreate(RSA_HANDLE); generate rsa key set
 void nd_RSAdestroy(RSA_HANDLE *h_rsa);	//destroy res key set
 //rsa encrypt/decrypt
 int nd_RSAPublicEncrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa);
 int nd_RSAPrivateEncrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa);
 int nd_RSAPublicDecrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa);
 int nd_RSAPrivateEncrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa);
  
*/

#ifndef _ND_CRYPT_
#define _ND_CRYPT_

#include "nd_common/nd_comcfg.h"
#include "nd_common/nd_export_def.h"

#include "nd_crypt/tea.h"

#include "nd_crypt/rsah/global.h"
#include "nd_crypt/rsah/rsaref.h"
#include "nd_crypt/rsah/rsa.h"


ND_CRYPT_API int nd_TEAencrypt(unsigned char *data, int data_len, tea_k *key) ;
ND_CRYPT_API int nd_TEAdecrypt(unsigned char *data, int data_len, tea_k *key) ;
ND_CRYPT_API int nd_TEAGenKey(tea_k *key, char *seed) ;		//create key by seed


/* mix-in data */
ND_CRYPT_API char *crypt_stuff(char *src, int datalen, int stufflen ) ;	/*stuff data to align*/


/* convert crypt string to output string 
 * add by duan !
 */
ND_CRYPT_API char* MD5ToString(unsigned char src[16], unsigned char desc[33]);

ND_CRYPT_API char *MD5CryptStr16(const char *in_text, char output[16]);
ND_CRYPT_API char *MD5CryptStr32(const char *in_text, char output[33]);

/* 
 * @inlen input length
 * @input data address of input
 * @output buffer address char[16]
 */
ND_CRYPT_API char *MD5Crypt16(const void *inbuf, int inlen, char output[16]);

ND_CRYPT_API char *MD5Crypt32(const void *in_buf, int inlen, char output[33]);
ND_CRYPT_API int MD5cmp(char src[16], char desc[16]) ;

ND_CRYPT_API char *MD5file(const char *filepath, char output[33]);

ND_CRYPT_API int base64_encode( const char * source, int len, char * destination_string );

ND_CRYPT_API int base64_decode( unsigned char *input, unsigned int input_len, unsigned char *output, unsigned int *output_len ) ;
#define nd_base64_encode base64_encode
#define nd_base64_decode base64_decode

/* rsa crypt*/

typedef struct  {	
	R_RSA_PUBLIC_KEY publicKey;                          /* new RSA public key */
	R_RSA_PRIVATE_KEY privateKey;                       	/* new RSA private key */
	R_RANDOM_STRUCT randomStruct;                        /* random structure */
}ND_RSA_CONTEX ;
typedef ND_RSA_CONTEX *RSA_HANDLE ;

#define RSA_KEY_BITS 2048					/* rsa key length*/
#define RSA_CRYPT_BUF_SIZE 256				/* rsa buffer data length , means rsa handle capacity once*/

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


ND_CRYPT_API int nd_rsa_privkey_output(R_RSA_PRIVATE_KEY *priv_key, const char *bin_file,unsigned long long param,void*tip) ;
ND_CRYPT_API int nd_rsa_privkey_input(R_RSA_PRIVATE_KEY *priv_key, const char *bin_file) ;

ND_CRYPT_API int nd_rsa_pubkey_output(R_RSA_PUBLIC_KEY *pub_key, const char *bin_file,unsigned long long param,void*tip) ;
ND_CRYPT_API int nd_rsa_pubkey_input(R_RSA_PUBLIC_KEY *pub_key, const char *bin_file) ;

ND_CRYPT_API int nd_rsa_read_key(R_RSA_PRIVATE_KEY *key , const char * buf, int bufsize, int is_private) ;

// key to stream , return length of stream
ND_CRYPT_API int nd_rsa_write_key(R_RSA_PRIVATE_KEY *key ,  char * tobuf, int bufsize, int is_private);

ND_CRYPT_API int nd_rsa_test(R_RSA_PRIVATE_KEY *priv_key, R_RSA_PUBLIC_KEY *pub_key) ;

#endif	//_ND_CRYPT_
