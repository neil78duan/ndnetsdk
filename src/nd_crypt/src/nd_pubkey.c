/* file nd_pubkey.c 
 * RSA public key 
 * create by auto_tool please DO NOT modify 
 * datetime: 2018-4-2 16:37:27
 */ 

#include "nd_crypt/nd_crypt.h"
#include "nd_crypt/nd_pubkey.h"
/* publish version */
int __g_pub_publis_num=2 ; 

/* md5 of private key  */
char *__g_privkey_md5="1569093b70d05a7c93e21904649d4f97" ; 


/* RSA public key  data */ 
R_RSA_PUBLIC_KEY __nd_pub_key = 
{	1024,//bits of key
	// data of pub_key.modulus
	{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xca, 0x9c, 0xe0
	, 0x1c, 0xa0, 0x5a, 0x83, 0x6d, 0xd9, 0x29, 0xfc, 0x5b, 0x85
	, 0xc6, 0x88, 0xfb, 0x9, 0x15, 0x7b, 0x9f, 0xd0, 0x7c, 0x94
	, 0x72, 0x49, 0x42, 0xbb, 0x89, 0x3d, 0xb1, 0x6d, 0xa7, 0x71
	, 0x60, 0x8, 0x2f, 0xc1, 0x5, 0xe3, 0x60, 0x82, 0x81, 0xd2
	, 0x9e, 0x9b, 0x9e, 0x33, 0x28, 0x39, 0x48, 0x4b, 0x3, 0x1f
	, 0x79, 0x8c, 0x47, 0xf0, 0x8d, 0xdb, 0xed, 0x54, 0xb9, 0xbb
	, 0x65, 0x0, 0x4a, 0x44, 0x13, 0x7f, 0xc7, 0x99, 0x4e, 0xd9
	, 0x17, 0xbf, 0x97, 0x1, 0xa9, 0xa4, 0xa2, 0x74, 0xbc, 0x2a
	, 0xa8, 0x6e, 0xd4, 0x94, 0x70, 0x95, 0x13, 0xec, 0x85, 0x3c
	, 0x63, 0xd9, 0xd, 0x80, 0x88, 0x51, 0x14, 0xe0, 0xdd, 0xa0
	, 0x11, 0x75, 0x81, 0xcd, 0xe6, 0x87, 0xe5, 0x1c, 0x37, 0x92
	, 0x6, 0x10, 0x1e, 0x50, 0xd0, 0x92, 0x6d, 0x21, 0xea, 0x9d
	, 0x3d, 0x36, 0xbf, 0x53, 0x89},
	 	// data of pub_key.exponent
	{ 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
	, 0x0, 0x0, 0x1, 0x0, 0x1}
} ;

/* RSA public key  data --end */ 

int nd_get_public_certificate_version(void)
{
	return __g_pub_publis_num ;
}

char* nd_calc_publickey_md5(char text[33])
{
	return MD5CryptToStr32((char*)&__nd_pub_key, sizeof(__nd_pub_key), text) ;
}

R_RSA_PUBLIC_KEY *nd_get_publickey(void) 
{
	return &__nd_pub_key ;
}

