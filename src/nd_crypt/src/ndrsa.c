/* file ndrsa.c
 * implemention user function interface of rsa
 * neil duan
 * 2008-8
 */
 
//#include "nd_common/nd_common.h"
#include "nd_crypt/rsah/global.h"
#include "nd_crypt/rsah/rsaref.h"
#include "nd_crypt/rsah/rsa.h"
#include "nd_crypt/nd_crypt.h"
#include "nd_crypt/tea.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#pragma warning(push)
#pragma warning(disable : 4305)
extern int tea_key(tea_k *k) ;

int RSAinit_random(R_RANDOM_STRUCT *rStruct)
{
	int i;
	unsigned char p_randblock[RANDOM_BYTES_NEEDED+1]  ;
	tea_k *paddr ;
	if(R_RandomInit(rStruct)){
		return -1;
	}
	paddr = (tea_k *)p_randblock ;
	for(i=0; i<RANDOM_BYTES_NEEDED/sizeof(tea_k); i++) {
		tea_key(paddr) ;
		paddr++ ;
	}


	R_RandomUpdate(rStruct, p_randblock, sizeof(p_randblock)) ;
	
	return 0;
}
#pragma warning(pop)

void RSAdestroy_random(R_RANDOM_STRUCT *rStruct)
{
	R_RandomFinal(rStruct) ;
}


int nd_RSAInit(RSA_HANDLE r_contex)
{	
	int ret ;
	R_RSA_PROTO_KEY protoKey;                            /* RSA prototype key */
	
	if(RSAinit_random(&r_contex->randomStruct)) {
		return -1 ;
	}
	
	r_contex->publicKey.bits = RSA_KEY_BITS ; r_contex->privateKey.bits = RSA_KEY_BITS ; protoKey.bits = RSA_KEY_BITS;
	ret = R_GeneratePEMKeys (&r_contex->publicKey, &r_contex->privateKey, &protoKey, &r_contex->randomStruct) ;
	if(ret ) {
		return -1 ;
	}

	memset(&protoKey,0, sizeof(protoKey)) ;
	return 0 ;
}

void nd_RSAdestroy(RSA_HANDLE h_rsa)
{
	ND_RSA_CONTEX *r_contex = (ND_RSA_CONTEX *)h_rsa ;
	R_RandomFinal(&r_contex->randomStruct) ;
	
}

int nd_RSAPublicEncrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa)
{
	return RSAPublicEncrypt(outbuf, outlen, inbuf, inlen, &(h_rsa->publicKey) , &(h_rsa->randomStruct)) ;
}

int nd_RSAPrivateEncrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa)
{
	return RSAPrivateEncrypt(outbuf, outlen, inbuf, inlen, &h_rsa->privateKey) ;
}

int nd_RSAPublicDecrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa)
{
	return RSAPublicDecrypt(outbuf, outlen, inbuf, inlen, &h_rsa->publicKey) ;
}

int nd_RSAPrivateDecrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa)
{
	return RSAPrivateDecrypt(outbuf, outlen, inbuf, inlen, &h_rsa->privateKey ) ;
}

int rsa_pub_encrypt(char *outbuf, int *outlen, char *inbuf, int inlen,R_RSA_PUBLIC_KEY *key)
{
	R_RANDOM_STRUCT randomStruct; 
	RSAinit_random(&randomStruct) ;

	return RSAPublicEncrypt(outbuf, outlen, inbuf, inlen,key ,&randomStruct) ;
}

int rsa_priv_encrypt(char *outbuf, int *outlen, char *inbuf, int inlen,R_RSA_PRIVATE_KEY *key)
{
	return RSAPrivateEncrypt(outbuf, outlen, inbuf, inlen, key) ;
}

int rsa_pub_decrypt(char *outbuf, int *outlen, char *inbuf, int inlen,R_RSA_PUBLIC_KEY *key)
{
	return RSAPublicDecrypt(outbuf, outlen, inbuf, inlen, key) ;
}
int rsa_priv_decrypt(char *outbuf, int *outlen, char *inbuf, int inlen,R_RSA_PRIVATE_KEY *key)
{
	return RSAPrivateDecrypt(outbuf, outlen, inbuf, inlen, key ) ;
}

int nd_rsa_text(R_RSA_PRIVATE_KEY *priv_key, R_RSA_PUBLIC_KEY *pub_key)
{
	int len1=1024, len2 =1024;
	int text_size ;
	char *p = "hello world" ;
	char buf1[1024] ,buf2[1024];

	ND_RSA_CONTEX rsa_contex = {0} ;
	memcpy((void*)&rsa_contex.privateKey, priv_key, sizeof(rsa_contex.privateKey) );
	memcpy((void*)&rsa_contex.publicKey, pub_key, sizeof(rsa_contex.publicKey) );
	RSAinit_random(&rsa_contex.randomStruct);

	text_size = (int)strlen(p) ;

	//test public encrypt , private decrypt
	if (nd_RSAPublicEncrypt(buf1, &len1, p, text_size, &rsa_contex)){
		return -1 ;
	}
	if (nd_RSAPrivateDecrypt(buf2, &len2, buf1, len1, &rsa_contex)){
		return -1 ;
	}

	buf2[len2] = 0 ;
	if(strcmp(p, buf2)) {
		return -1 ;
	}

	//test private encrypt , public decrypt
	if (nd_RSAPrivateEncrypt(buf1, &len1, p, text_size, &rsa_contex)){
		return -1 ;
	}
	if (nd_RSAPublicDecrypt(buf2, &len2, buf1, len1, &rsa_contex)){
		return -1 ;
	}

	buf2[len2] = 0 ;
	if(strcmp(p, buf2)) {
		return -1 ;
	}
	return 0 ;
}