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


int testReadKey(RSA_HANDLE _h_rsa)
{
	char md5text1[33] ;
	char md5text2[33] ;
	char md5text3[33] ;
	char md5text4[33] ;

	R_RSA_PRIVATE_KEY priv_key = {0} ;
	R_RSA_PUBLIC_KEY pub_key = {0} ;


	if(-1==nd_rsa_privkey_input(&priv_key, "./private_key.bin") )  {
		printf("read private key bin error \n") ;
		exit(1) ;
	}

	if(-1==nd_rsa_pubkey_input(&pub_key, "./public_key.bin") )  {
		printf("read private key bin error \n") ;
		exit(1) ;
	}

	MD5Crypt32(&_h_rsa->publicKey,sizeof(_h_rsa->publicKey),md5text1);
	MD5Crypt32(&pub_key,sizeof(pub_key),md5text2);


	MD5Crypt32(&_h_rsa->privateKey,sizeof(_h_rsa->privateKey),md5text3);
	MD5Crypt32(&priv_key,sizeof(priv_key),md5text4);

	if (strcmp(md5text1,md5text2) ) {
		printf("read public key data error not match \n") ;
		exit(1) ;
	}

	if (strcmp(md5text3,md5text4) ) {
		printf("read private key data error not match \n") ;
		exit(1) ;
	}

	//test crpyt
	size_t size = 0 ;

	void *data = nd_load_file("./public_key.bin", &size) ;
	if (!data ) {
		nd_logfatal(" load public key error ./public_key.bin \n") ;
		return -1 ;
	}

	//crypt public using embed private key , after client recv use embed public key decrypt
	//R_RSA_PRIVATE_KEY *embedKey = &priv_key ;

	ND_RSA_CONTEX rsa_contex = {0} ;
	memcpy((void*)&rsa_contex.privateKey, &priv_key, sizeof(rsa_contex.privateKey) );
	memcpy((void*)&rsa_contex.publicKey, &pub_key, sizeof(rsa_contex.publicKey) );
	RSAinit_random(&rsa_contex.randomStruct);

	char buf[1024] ;
	int crpyt_size = sizeof(buf) ;

	if(0!=nd_RSAPrivateEncrypt(buf, &crpyt_size, (char*)data, size, &rsa_contex) ) {
		nd_unload_file(data) ;

		nd_logfatal(" rsa crypt public key error  \n") ;
		return -1 ;
	}
	if(0!=rsa_priv_encrypt(buf, &crpyt_size, (char*)data, size, &priv_key) ) {
		nd_unload_file(data) ;

		nd_logfatal(" rsa crypt public key error  \n") ;
		return -1 ;
	}

	nd_unload_file(data) ;

	fprintf(stdout," rsa crypt test success \n") ;
	return 0 ;
	
}

int TestRsa(R_RSA_PRIVATE_KEY *priv_key)
{
	int i =0;
	int len1, len2 ;
	int text_size ;
	char *p  ;
	char buf1[8192] ,buf2[8192];
	char text_buf[4096] ;

	len1 =sizeof(buf1) ;
	len2 =sizeof(buf2) ;

	ND_RSA_CONTEX rsa_contex = {0} ;
	memcpy((void*)&rsa_contex.privateKey, priv_key, sizeof(rsa_contex.privateKey) );
	memcpy((void*)&rsa_contex.publicKey, priv_key, sizeof(rsa_contex.publicKey) );
	RSAinit_random(&rsa_contex.randomStruct);

	text_size = 0 ;
	p = text_buf ;
	while (p< (text_buf + 4000) ) {
		int size = snprintf(p, 100, "%d hell world. ", i) ;
		i++ ;
		p += size ;
		text_size += size ;
	}

	p=text_buf ;

	//test public encrypt , private decrypt
	if (nd_RSAPublicEncrypt(buf1, &len1, p, text_size, &rsa_contex)){
		printf("RSA text error \n") ;
		return -1;
	}
	if (nd_RSAPrivateDecrypt(buf2, &len2, buf1, len1, &rsa_contex)){
		printf("RSA text error \n") ;
		return -1;
	}

	nd_assert(text_size == len2) ;
	buf2[len2] = 0 ;
	if(strcmp(p, buf2)) {
		printf("RSA text error \n") ;
		return -1;
	}

	len1 =sizeof(buf1) ;
	len2 =sizeof(buf2) ;

	//test private encrypt , public decrypt
	if (nd_RSAPrivateEncrypt(buf1, &len1, p, text_size, &rsa_contex)){
		printf("RSA text error \n") ;
		return -1;
	}

	if (nd_RSAPublicDecrypt(buf2, &len2, buf1, len1, &rsa_contex)){
		printf("RSA text error \n") ;
		return -1;
	}

	nd_assert(text_size == len2) ;
	buf2[len2] = 0 ;
	if(strcmp(p, buf2)) {
		printf("RSA text error \n") ;
		return -1;
	}

	if(-1==nd_rsa_privkey_output(&rsa_contex.privateKey, "./private_key.bin",0,NULL) )  {
		printf("out put private key bin error \n") ;
		exit(1) ;
	}

	if(-1==nd_rsa_pubkey_output(&rsa_contex.publicKey, "./public_key.bin",0,NULL) )  {
		printf("out put private key bin error \n") ;
		exit(1) ;
	}

	if(-1==testReadKey(&rsa_contex) ){
		printf("read key error\n") ;
		exit(1) ;

	}

	return 0 ;
}

//rsa¼ÓÃÜ/½âÃÜ
void test_rsa()
{

		
	ND_RSA_CONTEX  __rsa_contex ;

	ndprintf("start test RSA \n input data\n") ;
	
	if(nd_RSAInit(&__rsa_contex, 500))
		abort()  ;

	TestRsa(&__rsa_contex.privateKey) ;

	nd_RSAdestroy(&__rsa_contex);
}

int crypt_test()
{
	test_rsa();
//	md5test() ;
//	tea_test() ;
	return 0;
}
