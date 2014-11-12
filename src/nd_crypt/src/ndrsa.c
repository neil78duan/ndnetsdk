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


int nd_RSAInit(RSA_HANDLE r_contex,int keyBits)
{	
	int ret ;
	R_RSA_PROTO_KEY protoKey;                            /* RSA prototype key */
	
	if(RSAinit_random(&r_contex->randomStruct)) {
		return -1 ;
	}
	if (keyBits == 0) {
		keyBits = RSA_KEY_BITS ;
	}
	else {
		keyBits = (keyBits + 127)  & ~127 ;
		if (keyBits>4096) {
			keyBits =4096 ;
		}
	}
	r_contex->publicKey.bits = keyBits ;
	r_contex->privateKey.bits = keyBits ;
	protoKey.bits = keyBits;
	
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


int rsa_get_crypt_len(R_RSA_PUBLIC_KEY *key)
{
	return 	((key->bits + 7) / 8) - 11 ;
}

//read key from file

#include "nd_common/nddir.h"
static int _byte_order()
{
	int a = 1 ;
	char *p = (char*)&a ;
	return (int)p[0] ;
}

#pragma pack(push ,1 )
struct rsa_key_header
{
	char sign[4] ;
	char byte_order ; // 1 little end , 0 big end ;
	unsigned short bits ;
};
#pragma pack(pop )


#define  RSA_HEADER_INIT(name) struct rsa_key_header name = { {'n','d','r','k'} ,_byte_order()}

#define WRITE_CONTEXT(_sign, _data, _size, _pf) do { \
	int i ;		\
	for(i =0 ;i<_size; i++) {		\
		if(_data[i] ) {				\
			unsigned short write_len= _size - i ; 	\
			unsigned short start_pos= i ; 	\
			fwrite(_sign,1, 1, _pf) ;				\
			fwrite(&start_pos,1, sizeof(start_pos), _pf) ;				\
			fwrite(&write_len,1, sizeof(write_len), _pf) ;	\
			fwrite(&(_data[i]),1, write_len, _pf) ;			\
			break ;		\
		}				\
	}					\
}while(0)

int _key_output(R_RSA_PRIVATE_KEY *key, const char *bin_file, int is_private)
{
	int ret = 0;
	char sign ;
	RSA_HEADER_INIT( rsa_header ) ;
	FILE *pf = fopen(bin_file, "w") ;
	if (!pf) {
		return -1 ;
	}
	rsa_header.bits = key->bits ;

	ret = (int) fwrite(&rsa_header,1, sizeof(rsa_header), pf) ;
	if(-1==ret) {
		fclose(pf);
		return -1;
	}

	sign='A' ;
	WRITE_CONTEXT(&sign, key->modulus, sizeof(key->modulus), pf) ;

	sign='B' ;
	WRITE_CONTEXT(&sign, key->publicExponent, sizeof(key->publicExponent), pf) ;

	if (is_private) {
		sign='C' ;
		WRITE_CONTEXT(&sign, key->exponent, sizeof(key->exponent), pf) ;

		sign='D' ;
		WRITE_CONTEXT(&sign, key->prime[0], sizeof(key->prime[0]), pf) ;

		sign='E' ;
		WRITE_CONTEXT(&sign, key->prime[1], sizeof(key->prime[1]), pf) ;

		sign='F' ;
		WRITE_CONTEXT(&sign, key->primeExponent[0], sizeof(key->primeExponent[0]), pf) ;

		sign='G' ;
		WRITE_CONTEXT(&sign, key->primeExponent[1], sizeof(key->primeExponent[1]), pf) ;

		sign='H' ;
		WRITE_CONTEXT(&sign, key->coefficient, sizeof(key->coefficient), pf) ;
	}

	fclose(pf) ;
	return 0;
	
}

int nd_rsa_read_key(R_RSA_PRIVATE_KEY *key , const char * buf, int bufsize, int is_private)
{
	int ret = 0;
	char *p ;
	RSA_HEADER_INIT( rsa_header ) ;
	struct rsa_key_header *inheader = (struct rsa_key_header *) buf ;
	if (*(int*)rsa_header.sign != *(int*)inheader->sign ) {
		return -1 ;
	}
	if (rsa_header.byte_order != inheader->byte_order) {
		return  -2 ;
	}

	key->bits = inheader->bits ;
	p = (char*) (inheader + 1 ) ;

#define  READ_NODE( _data, _size) \
	do {						\
		unsigned short start_pos = *((*(unsigned short**)&p)++) ;	\
		unsigned short len = *((*(unsigned short**)&p)++) ;			\
		if(start_pos>= _size || len > _size) {						\
			return -1 ;			\
		}						\
		memcpy(&(_data[start_pos]), p,  len) ;	\
		p += len ;				\
		++ret;					\
	}while(0)

	while (p < buf + bufsize) {
		char sign = *p++ ;
		switch (sign) {
			case 'A':
				READ_NODE(key->modulus,sizeof(key->modulus)) ;
				break;

			case 'B':
				READ_NODE(key->publicExponent,sizeof(key->publicExponent)) ;
				break;
			case 'C':
				if (is_private==0) {
					break ;
				}
				READ_NODE(key->exponent,sizeof(key->exponent)) ;
				break;
			case 'D':
				if (is_private==0) {
					break ;
				}
				READ_NODE(key->prime[0],sizeof(key->prime[0])) ;
				break;
			case 'E':
				if (is_private==0) {
					break ;
				}
				READ_NODE(key->prime[1],sizeof(key->prime[1])) ;
				break;
			case 'F':
				if (is_private==0) {
					break ;
				}
				READ_NODE(key->primeExponent[0],sizeof(key->primeExponent[0])) ;
				break;
			case 'G':
				if (is_private==0) {
					break ;
				}
				READ_NODE(key->primeExponent[1],sizeof(key->primeExponent[1])) ;
				break;
			case 'H':
				if (is_private==0) {
					break ;
				}
				READ_NODE(key->coefficient,sizeof(key->coefficient)) ;
				break;

			default:
				break;
		}

	}

	if (is_private) {
		return  ret==8 ? 0 : -1 ;
	}
	else {
		return ret == 2 ? 0: -1 ;
	}

}


int nd_rsa_privkey_output(R_RSA_PRIVATE_KEY *key, const char *bin_file)
{
	return _key_output(key, bin_file, 1) ;

}


int nd_rsa_privkey_input(R_RSA_PRIVATE_KEY *priv_key, const char *bin_file)
{
	int ret = -1 ;
	size_t size = 0 ;
	void *data =nd_load_file (bin_file, &size) ;
	if (data) {
		ret =  nd_rsa_read_key(priv_key , (const char *)data, size, 1) ;
		nd_unload_file(data) ;
	}
	return  ret ;

}

int nd_rsa_pubkey_output(R_RSA_PUBLIC_KEY *key, const char *bin_file)
{
	return _key_output((R_RSA_PRIVATE_KEY*)key, bin_file, 0) ;

}
int nd_rsa_pubkey_input(R_RSA_PUBLIC_KEY *priv_key, const char *bin_file)
{
	int ret = -1 ;
	size_t size = 0 ;
	void *data =nd_load_file (bin_file, &size) ;
	if (data) {
		ret =  nd_rsa_read_key((R_RSA_PRIVATE_KEY*)priv_key , (const char *)data, size, 0) ;
		nd_unload_file(data) ;
	}
	return  ret;

}


//test
int nd_rsa_test(R_RSA_PRIVATE_KEY *priv_key, R_RSA_PUBLIC_KEY *pub_key)
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