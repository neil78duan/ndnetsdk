/**********************************************************
   TEA - Tiny Encryption Algorithm
   author neil duan 
   porting for ndengine 
   modified : 2008-8
   version 1.0
   2008-8
   
 **********************************************************/
#include "nd_common/nd_comcfg.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#ifdef ND_UNIX
#include <unistd.h>
#endif

#include "nd_crypt/nd_crypt.h"

#define DELTA 0x9e3779b9 /* sqr(5)-1 * 2^31 */
/**********************************************************
   Input values: 	k[4]	128-bit key
			v[2]    64-bit plaintext block
   Output values:	v[2]    64-bit ciphertext block 
 **********************************************************/
void tean(TEAint32 *k, TEAint32 *v, int N)
{
	register TEAint32 y=v[0], z=v[1];
	register TEAint32 limit,sum=0;
	if(N>0) { 
		/* ENCRYPT */
		limit=DELTA*N;
		while(sum!=limit) {
			y+=((z<<4)^(z>>5)) + (z^sum) + k[sum&3];
			sum+=DELTA;
			z+=((y<<4)^(y>>5)) + (y^sum) + k[(sum>>11)&3];
		}
	} 
	else { 
		/* DECRYPT */
		sum=DELTA*(-N);
		while(sum) {
			z-=((y<<4)^(y>>5)) + (y^sum) + k[(sum>>11)&3];
			sum-=DELTA;
			y-=((z<<4)^(z>>5)) + (z^sum) + k[sum&3];
		}
	}
	v[0]=y; v[1]=z;
}


/*generate tea crypt key */
static void tea_init_rnd()
{
	static int rnd_inited = 0 ;
	
	
	//long time();
	unsigned short seed[3];
	int _getpid( void );
	
	if(1==rnd_inited){
		return ;
	}
	else {
		rnd_inited = 1 ;
	}
	seed[0] = time((time_t *)0) & 0xFFFF;
#if !defined(ND_UNIX) 
	seed[1] = _getpid() & 0xFFFF;
	seed[2] = (time((time_t *)0) >> 16) & 0xFFFF;
	{
		unsigned int *nseed = (unsigned int *) &seed[0] ;
		srand(*nseed) ;
	}
#else
	seed[1] = ((long) getpid()) & 0xFFFF;
	seed[2] = (time((long *)0) >> 16) & 0xFFFF;
	(void)seed48( seed );
#endif
}

#if !defined(ND_UNIX) 
#include <windows.h>
int tea_key(tea_k *k)
{
	TEAint32 *np = (TEAint32*)k ;
	LARGE_INTEGER curtim ;
	
	tea_init_rnd() ;
	
	QueryPerformanceCounter(&curtim) ;
	np[0] = rand() ^ (long)curtim.QuadPart ;
	
	QueryPerformanceCounter(&curtim) ;
	np[2] = rand() & (long)curtim.QuadPart;
	
	np[1] = rand()* (long)curtim.QuadPart ;
	
	QueryPerformanceCounter(&curtim) ;
	np[3] = rand() ^ (long)(~curtim.QuadPart) ;
	return 0 ;
}
#else
#include <sys/time.h>
int tea_key(tea_k *k)
{
	TEAint32 *np = (TEAint32 *)k ;
	struct timeval tv ;
	
	tea_init_rnd() ;
	
	gettimeofday(&tv, NULL);
	np[0] =(int) lrand48() ^ tv.tv_usec ;
	
	gettimeofday(&tv, NULL);
	np[2] =(int) lrand48() & tv.tv_usec ;
	
	np[1] = (int)(lrand48() * tv.tv_sec) | tv.tv_usec;
	
	gettimeofday(&tv, NULL);
	np[3] =(int) (lrand48() * tv.tv_usec) & tv.tv_sec ;
	return 0 ;
}
#endif 

