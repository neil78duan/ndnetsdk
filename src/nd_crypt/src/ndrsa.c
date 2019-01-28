/* file ndrsa.c
 * implemention user function interface of rsa
 * neil duan
 * 2008-8
 */
 
#include "nd_common/nd_common.h"
#include "nd_crypt/rsah/global.h"
#include "nd_crypt/rsah/rsaref.h"
#include "nd_crypt/rsah/rsa.h"
#include "nd_crypt/nd_crypt.h"
#include "nd_crypt/tea.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef _MSC_VER
#pragma warning(disable : 4305)
#endif

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
	return RSAPublicEncrypt((unsigned char*)outbuf,(unsigned int*) outlen, (unsigned char*)inbuf, (unsigned int)inlen, &(h_rsa->publicKey) , &(h_rsa->randomStruct)) ;
}

int nd_RSAPrivateEncrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa)
{
	return RSAPrivateEncrypt((unsigned char*)outbuf, (unsigned int*)outlen, (unsigned char*)inbuf, (unsigned int)inlen, &h_rsa->privateKey) ;
}

int nd_RSAPublicDecrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa)
{
	return RSAPublicDecrypt((unsigned char*)outbuf, (unsigned int*)outlen, (unsigned char*)inbuf, (unsigned int)inlen, &h_rsa->publicKey) ;
}

int nd_RSAPrivateDecrypt(char *outbuf, int *outlen, char *inbuf, int inlen,RSA_HANDLE h_rsa)
{
	return RSAPrivateDecrypt((unsigned char*)outbuf, (unsigned int*)outlen, (unsigned char*)inbuf, (unsigned int)inlen, &h_rsa->privateKey ) ;
}


int rsa_pub_encrypt(char *outbuf, int *outlen, char *inbuf, int inlen,R_RSA_PUBLIC_KEY *key)
{
	R_RANDOM_STRUCT randomStruct; 
	RSAinit_random(&randomStruct) ;

	return RSAPublicEncrypt((unsigned char*)outbuf, (unsigned int*)outlen, (unsigned char*)inbuf, (unsigned int)inlen,key ,&randomStruct) ;
}

int rsa_priv_encrypt(char *outbuf, int *outlen, char *inbuf, int inlen,R_RSA_PRIVATE_KEY *key)
{

	return RSAPrivateEncrypt((unsigned char*)outbuf, (unsigned int*)outlen, (unsigned char*)inbuf, (unsigned int)inlen, key) ;
}

int rsa_pub_decrypt(char *outbuf, int *outlen, char *inbuf, int inlen,R_RSA_PUBLIC_KEY *key)
{
	return RSAPublicDecrypt((unsigned char*)outbuf, (unsigned int*)outlen, (unsigned char*)inbuf, inlen, key) ;
}
int rsa_priv_decrypt(char *outbuf, int *outlen, char *inbuf, int inlen,R_RSA_PRIVATE_KEY *key)
{
	return RSAPrivateDecrypt((unsigned char*)outbuf, (unsigned int*)outlen, (unsigned char*)inbuf, (unsigned int)inlen, key ) ;
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


#define _order_exch_s(_a)    ((unsigned short)( \
	(((unsigned short)(_a) & (unsigned short)0x00ff) << 8) | \
	(((unsigned short)(_a) & (unsigned short)0xff00) >> 8) ))


#define _order_exch_l(_a) 	((unsigned int)( \
(((unsigned int)(_a) & (unsigned int)0x000000ff) << 24) | \
(((unsigned int)(_a) & (unsigned int)0x0000ff00) << 8) | \
(((unsigned int)(_a) & (unsigned int)0x00ff0000) >> 8) | \
(((unsigned int)(_a) & (unsigned int)0xff000000) >> 24) ))


#define WRITE_PARAM(pf, param) \
if(param) {				\
	NDUINT32 val ;		\
	char *offset  ;				\
	*((*(char**)&pf)++) ='T' ;	\
	val = ND_LODWORD(param) ; memcpy(pf,&val, sizeof(val)) ; pf +=sizeof(val) ;	\
	val = ND_HIDWORD(param) ; memcpy(pf,&val, sizeof(val)) ; pf +=sizeof(val) ;	\
	offset = pf ; pf += sizeof(val) ;		\
	val = ndsnprintf(pf, bufsize - (pf - tobuf), "Q%s!  ",(const char*)paramText) ;	\
	val += 1 ;	pf += val ;	*((*(char**)&pf)++) ='Q' ; val += 1 ;		\
	memcpy(offset,&val, sizeof(val)) ;		\
}

#define _TEST_VALID(p, _func)	do {\
	NDUINT32 lowT, hiT ;		\
	NDUINT32 offset ;			\
	memcpy(&lowT, p, sizeof(lowT)) ; p+= sizeof(lowT) ;	\
	memcpy(&hiT, p, sizeof(hiT)) ; p+= sizeof(hiT) ;	\
	memcpy(&offset, p, sizeof(offset)) ; p+= sizeof(offset) ;	\
	GET_LONG(lowT) ;GET_LONG(hiT) ;GET_LONG(offset) ;	\
	if( ND_MAKE_QWORD(hiT, lowT) > _func(0) ) {	p+= offset ;}\
}while(0)

//write key to buf
static int nd_rsa_write_key_ex(R_RSA_PRIVATE_KEY *key ,  char * tobuf, int bufsize, int is_private, unsigned long long param,void *paramText)
{
	char sign ;
	char *pf = tobuf ;
	RSA_HEADER_INIT( rsa_header ) ;

	rsa_header.bits = key->bits ;

	if (pf + sizeof(rsa_header) > tobuf + bufsize) {
		return -1;
	}

	memcpy(pf, &rsa_header, sizeof(rsa_header)) ;
	pf += sizeof(rsa_header) ;

#define COPY_CONTEXT(_sign, _data, _size, _pf) do { \
		int i ;		\
		for(i =0 ;i<_size; i++) {		\
		if(_data[i] ) {				\
			unsigned short write_len= _size - i ; 	\
			unsigned short start_pos= i ; 	\
			if (_pf + write_len + 5 > tobuf + bufsize ) { return -1 ;} \
			*((*(char**)&_pf)++) =*(_sign) ;	\
			memcpy(_pf,&start_pos, sizeof(start_pos)) ; _pf +=sizeof(start_pos) ;\
			memcpy(_pf,&write_len, sizeof(write_len)) ; _pf +=sizeof(write_len) ;\
			memcpy(_pf,&(_data[i]), write_len) ;			\
			_pf+= write_len ; \
			break ;		\
		}				\
		}					\
	}while(0)

	WRITE_PARAM(pf,param) ;

	sign='A' ;
	COPY_CONTEXT(&sign, key->modulus, sizeof(key->modulus), pf) ;

	sign='B' ;
	COPY_CONTEXT(&sign, key->publicExponent, sizeof(key->publicExponent), pf) ;

	if (is_private) {
		sign='C' ;
		COPY_CONTEXT(&sign, key->exponent, sizeof(key->exponent), pf) ;

		sign='D' ;
		COPY_CONTEXT(&sign, key->prime[0], sizeof(key->prime[0]), pf) ;

		sign='E' ;
		COPY_CONTEXT(&sign, key->prime[1], sizeof(key->prime[1]), pf) ;

		sign='F' ;
		COPY_CONTEXT(&sign, key->primeExponent[0], sizeof(key->primeExponent[0]), pf) ;

		sign='G' ;
		COPY_CONTEXT(&sign, key->primeExponent[1], sizeof(key->primeExponent[1]), pf) ;

		sign='H' ;
		COPY_CONTEXT(&sign, key->coefficient, sizeof(key->coefficient), pf) ;
	}

	return (int)(pf-tobuf);
}
int nd_rsa_write_key(R_RSA_PRIVATE_KEY *key ,  char * tobuf, int bufsize, int is_private)
{
	return nd_rsa_write_key_ex(key, tobuf, bufsize,  is_private,0,NULL) ;
}
static int _key_output(R_RSA_PRIVATE_KEY *key, const char *bin_file, int is_private,unsigned long long param,void*pTips)
{
	int len;
	FILE *pf;
	char buf[8192];
	len = nd_rsa_write_key_ex(key, buf, (int) sizeof(buf), is_private,param,pTips);
	if (len == -1 )	{
		return -1;
	}
	pf = fopen(bin_file, "wb");
	if (!pf) {
		return -1 ;
	}
	
	
	len = (int) fwrite(buf,1, len, pf) ;
	fclose(pf);
	return len ? 0 : -1;
	
}

int nd_rsa_read_key(R_RSA_PRIVATE_KEY *key , const char * buf, int bufsize, int is_private)
{
	int ret = 0;
	const char *p ;
	RSA_HEADER_INIT( rsa_header ) ;
	struct rsa_key_header *inheader = (struct rsa_key_header *) buf ;
	if (*(int*)rsa_header.sign != *(int*)inheader->sign ) {
		return -1 ;
	}

	key->bits = rsa_header.byte_order == inheader->byte_order ? inheader->bits : _order_exch_l(inheader->bits) ;
	p = (char*)(inheader + 1);

#define GET_SHORT(_s) _s =  rsa_header.byte_order == inheader->byte_order ? (_s) : _order_exch_s(_s)
#define GET_LONG(_s) _s =  rsa_header.byte_order == inheader->byte_order ? (_s) : _order_exch_l(_s)
	
#define  READ_NODE( _data, _size) \
	do {						\
		unsigned short start_pos = 0 ; /* *((*(unsigned short**)&p)++) ;	*/ \
		unsigned short len =0; /* *((*(unsigned short**)&p)++) ;			*/ \
		memcpy(&start_pos, p, sizeof(start_pos)) ; p+= sizeof(start_pos) ;		\
		memcpy(&len, p, sizeof(len)) ; p+= sizeof(len) ;		\
		GET_SHORT(start_pos) ;	\
		GET_SHORT(len) ;	\
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
			case 'T':
				_TEST_VALID(p,time) ;
				break ;
			case 'Q':
			{
				nd_logmsg("%s\n", p) ;
				p= buf + bufsize ;
				return -1 ;
			}
			default:
				break;
		}

	}

	if (is_private) {
		return  ret==8 ? 0 : -1 ;
	}
	else {
		return ret ==2 ? 0: -1 ;
	}

}


int nd_rsa_privkey_output(R_RSA_PRIVATE_KEY *key, const char *bin_file,unsigned long long param,void*tip)
{
	return _key_output(key, bin_file, 1,param,tip) ;
}


int nd_rsa_privkey_input(R_RSA_PRIVATE_KEY *priv_key, const char *bin_file)
{
	int ret = -1 ;
	size_t size = 0 ;
	void *data =nd_load_file (bin_file, &size) ;
	if (data) {
		ret =  nd_rsa_read_key(priv_key , (const char *)data,(int) size, 1) ;
		nd_unload_file(data) ;
	}
	return  ret ;

}

int nd_rsa_pubkey_output(R_RSA_PUBLIC_KEY *key, const char *bin_file,unsigned long long param,void*tip)
{
	return _key_output((R_RSA_PRIVATE_KEY*)key, bin_file, 0,param,tip) ;

}
int nd_rsa_pubkey_input(R_RSA_PUBLIC_KEY *priv_key, const char *bin_file)
{
	int ret = -1 ;
	size_t size = 0 ;
	void *data =nd_load_file (bin_file, &size) ;
	if (data) {
		ret =  nd_rsa_read_key((R_RSA_PRIVATE_KEY*)priv_key , (const char *)data, (int)size, 0) ;
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

	ND_RSA_CONTEX rsa_contex ;//= {0} ;
	memset(&rsa_contex,0,sizeof(rsa_contex)) ;
	memcpy((void*)&rsa_contex.privateKey, priv_key, sizeof(rsa_contex.privateKey) );
	memcpy((void*)&rsa_contex.publicKey, pub_key, sizeof(rsa_contex.publicKey) );
	RSAinit_random(&rsa_contex.randomStruct);

	text_size = (int)ndstrlen(p) ;

	//test public encrypt , private decrypt
	if (nd_RSAPublicEncrypt(buf1, &len1, p, text_size, &rsa_contex)){
		return -1 ;
	}
	if (nd_RSAPrivateDecrypt(buf2, &len2, buf1, len1, &rsa_contex)){
		return -1 ;
	}

	buf2[len2] = 0 ;
	if(ndstrcmp(p, buf2)) {
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
	if(ndstrcmp(p, buf2)) {
		return -1 ;
	}
	return 0 ;
}
