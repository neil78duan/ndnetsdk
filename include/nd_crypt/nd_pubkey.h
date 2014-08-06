/* file nd_pubkey.h
 * RSA public key 
 * create by auto_tool please DO NOT modify 
 * datetime: 2011-10-13 11:16:4
 */ 

#ifndef _ND_PUBKEY_H_

#include "nd_crypt/nd_crypt.h"


ND_CRYPT_API int CliGetCertificateVersion(void);
ND_CRYPT_API char *GetPrivatekeyMd5(void);
ND_CRYPT_API char* CalcPublickeyMd5(char text[33]);
ND_CRYPT_API R_RSA_PUBLIC_KEY *GetPgcliRsaPubkey(void) ;

#endif 
