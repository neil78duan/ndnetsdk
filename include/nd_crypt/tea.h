/*
   TEA - Tiny Encryption Algorithm
   author neil duan 
   
   modified : 2004-2-4 13
   version 1.0
   2004-2-4 
   
 */

#ifndef _MYTEA_H_
#define _MYTEA_H_

#define ROUNDS 32

typedef unsigned int TEAint32;

typedef struct _tea_k{ TEAint32 k[4];} tea_k;		/* tea key*/
typedef struct _tea_v{ TEAint32 v[2];} tea_v;		/* tea value (en/decrypt code buf)*/

/* generate tea key*/
ND_CRYPT_API int tea_key(tea_k *k);

/**********************************************************
   Input values: 	k[4]	128-bit key
			v[2]    64-bit plaintext block
   Output values:	v[2]    64-bit ciphertext block 
 **********************************************************/
ND_CRYPT_API void tean(TEAint32 *k, TEAint32 *v, int N) ;

/* tea enctypt data 
 * input :  k 128bit encryptKEY 
 			v 64bit plaintext block
 * output : v ciphertext block 
 */
static __inline void tea_enc(tea_k *k, tea_v *v)
{
	tean((TEAint32 *)k, (TEAint32 *)v, ROUNDS) ;
}

/* tea dectypt data 
 * input :  k 128bit encryptKEY 
 			v 64bit plaintext block
 * output : v ciphertext block 
 */
static __inline void  tea_dec(tea_k *k, tea_v *v)
{
	tean((TEAint32 *)k, (TEAint32 *)v, -ROUNDS) ;
}


#endif 
