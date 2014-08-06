/* file : checksum.c
 * test checksum arithmetic
 * 2008-5-16
 * all right reserved
 */
 
#include "nd_common/nd_common.h"
#include "nd_crypt/nd_crypt.h"

void md5test()
{
	//int n =0 ;
	//char *buf = "hello world" ;
	char buf[128], mdres[16],mdtxt[33] ;
	ndprintf("start test MD5 \n input data\n") ;
	
	 fgets( buf, 128, stdin );

	//buf[n] = 0 ;

	MD5CryptStr16(buf,mdres);
	MD5ToString(mdres,mdtxt) ;

	ndprintf("input test=%s \n md5=%s\n", buf, mdtxt) ;

}

void tea_test()
{
	int len, en_len ;
	tea_k k ;
	char buf[128] ;
	ndprintf("start test TEA \n input data\n") ;
	
	fgets( buf, 128, stdin );

	if(0!=tea_key(&k)){
		ndprintf("generate tea key error\n") ;
		return ;
	}
	len = strlen(buf) ;

	en_len =nd_TEAencrypt(buf,len, &k) ;

	nd_TEAdecrypt(buf, en_len,&k) ;
	if(en_len > len) {
		nd_assert(buf[en_len-1]==(en_len-len)) ;
		buf[len] = 0 ;
	}
	ndprintf("after encrypt and decrypt buf=%s", buf) ;

}


//rsaº”√‹/Ω‚√‹
void test_rsa()
{

	int len ,outlen, inlen;
	char buf[128],en_buf[128], de_buf[128] ;
		
	ND_RSA_CONTEX  __rsa_contex ;

	ndprintf("start test RSA \n input data\n") ;
	
	if(nd_RSAInit(&__rsa_contex))
		abort()  ;


	fgets( buf, 128, stdin );
	
	len = strlen(buf) ;

	printf("input buf=%s\n",buf);
	
	if(0!=nd_RSAPublicEncrypt(en_buf, &outlen, buf, len, &__rsa_contex)) {
		ndprintf("generate rsa encrypt error\n") ;
		abort() ;
	}
	
	if(0!=nd_RSAPrivateDecrypt(de_buf, &inlen, en_buf, outlen,&__rsa_contex)) {
		ndprintf("generate rsa encrypt error\n") ;
		abort() ;
	}
/*
	if(0!=nd_RSAPrivateEncrypt(en_buf, &outlen, de_buf, inlen, &__rsa_contex)) {
		ndprintf("generate rsa encrypt error\n") ;
		abort() ;
	}
	
	if(0!=nd_RSAPublicDecrypt(de_buf, &inlen, en_buf, outlen,&__rsa_contex)) {
		ndprintf("generate rsa encrypt error\n") ;
		abort() ;
	}

*/
	de_buf[len] = 0 ;
	ndprintf("after encrypt and decrypt buf=%s\n", de_buf) ;
	nd_RSAdestroy(&__rsa_contex);
}

int crypt_test()
{
	test_rsa();
//	md5test() ;
//	tea_test() ;
	return 0;
}
