/* file nd_pubkey.h
 * RSA public key 
 * create by auto_tool please DO NOT modify 
 * datetime: 2011-10-13 11:16:4
 */ 

#ifndef _ND_PUBKEY_H_

#include "nd_crypt/nd_crypt.h"


ND_CRYPT_API int nd_get_public_certificate_version(void);
ND_CRYPT_API char* nd_calc_publickey_md5(char text[33]);
ND_CRYPT_API R_RSA_PUBLIC_KEY *nd_get_publickey(void) ;
ND_CRYPT_API void nd_set_publickey(const R_RSA_PUBLIC_KEY *key,int version);

#endif 
