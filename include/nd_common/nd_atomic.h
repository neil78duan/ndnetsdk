/* file : nd_atomic.h
 * define atomic operation
 *
 * version 1.0 
 * all right reserved by neil duan 
 * 2007 -10
 */

#ifndef _ND_ATOMIC_H_
#define _ND_ATOMIC_H_

/*
 atomic operation type : ndatomic_t 

//atomic operate
long nd_compare_swap(ndatomic_t *desc,ndatomic_t cmp, ndatomic_t exch)
{
	ndatomic_t tmp = *desc ;
	if(*desc==cmp) {
		*desc = exch ;
		return 1 ;
	}
	
	return 0 ;
}
		
ndatomic_t nd_atomic_inc(ndatomic_t *p)
{
	++(*p ) ;
	return *p ;
}
ndatomic_t nd_atomic_dec(ndatomic_t *p) 
{
	--(*p ) ;
	return *p ;
}	
ndatomic_t nd_atomic_add(ndatomic_t *p,int step) 
{
	ndatomic_t tmp = *desc ;
	
	*desc += step ;
	
	return tmp ;
}
ndatomic_t nd_atomic_sub(ndatomic_t *p,int step)
{
	ndatomic_t tmp = *desc ;
	
	*desc -= step ;
	
	return tmp ;
}
int  nd_atomic_swap(ndatomic_t *p, ndatomic_t val)
{

	ndatomic_t tmp = *desc ;
	
	*desc = val ;
	
	return tmp ;

}

int nd_testandset(ndatomic_t *p)
{
	int old = (int)*p ;
	if(p==0) 
		p =1 ;
	return old;
}
	
void nd_atomic_set(ndatomic_t* p, ndatomic_t val);
void nd_atomic_read(ndatomic_t *p);
  */
#if !defined(ND_UNIX) 

#if _MSC_VER < 1300 // 1200 == VC++ 6.0

typedef LONG ndatomic_t ;
#else 
typedef  LONG ndatomic_t ;
#endif

//atomic operate
__INLINE__ int nd_compare_swap(ndatomic_t *lpDest,ndatomic_t lComp,ndatomic_t lExchange)
{
	//ndatomic_t last = *lpDest ;
//#ifdef InterlockedCompareExchangePointer
//	return (InterlockedCompareExchange(lpDest, lExchange,lComp)==last);
//#else 
#if _MSC_VER < 1300 // 1200 == VC++ 6.0
	return (int) (lComp==(LONG)InterlockedCompareExchange((PVOID*)lpDest, (PVOID)lExchange,(PVOID)lComp) );
#else 
	return (int) (lComp==(LONG)InterlockedCompareExchange(lpDest, lExchange, lComp) );
#endif
//#endif
}
#define nd_atomic_inc(p)	InterlockedIncrement(p) 
#define nd_atomic_dec(p)	InterlockedDecrement(p) 
#define nd_atomic_add(p,step)	InterlockedExchangeAdd((long*)(p),step) 
#define nd_atomic_sub(p,step)	InterlockedExchangeAdd((long*)(p),-(step))
#define nd_atomic_swap(p, val)	InterlockedExchange(p,val) 
#define nd_testandset(p)	InterlockedExchange(p,1) 

#define nd_atomic_set(p, val)    InterlockedExchange((p), val)
#define nd_atomic_read(p)        (*(p))

#elif defined(__LINUX__)
typedef int ndatomic_t ;

#ifdef X86_64

#define nd_atomic_inc(x) __sync_add_and_fetch((x),1)  
#define nd_atomic_dec(x) __sync_sub_and_fetch((x),1)  
#define nd_atomic_add(x,y) __sync_add_and_fetch((x),(y))  
#define nd_atomic_sub(x,y) __sync_sub_and_fetch((x),(y)) 
#define nd_compare_swap(dest, cmp, exchval) (int)__sync_bool_compare_and_swap(dest, cmp, exchval)
static __INLINE__ ndatomic_t nd_atomic_swap(ndatomic_t *ptr, ndatomic_t exval)
{
	ndatomic_t cmpval = *ptr ;
	while(!nd_compare_swap(ptr, cmpval,exval)) {
		cmpval = *ptr ;
	}
	return cmpval ;
};
#define nd_testandset(p) __sync_fetch_and_add((p),1)
#define nd_atomic_set(p, val)    nd_atomic_swap(p,val)
#define nd_atomic_read(p)        (*(p))

#else 
/*
#include <asm/atomic.h>
#ifdef CONFIG_SMP
#define LOCK "lock ; "
#else
#define LOCK ""
#endif
/*/
#define LOCK "lock ; "
/**/
static __INLINE__ int nd_testandset (ndatomic_t  *p)
{
  long int ret;

  __asm__ __volatile__(
       "xchgl %0, %1"
       : "=r"(ret), "=m"(*p)
       : "0"(1), "m"(*p)
       : "memory");

  return ret;
}
static __INLINE__ int nd_compare_swap (ndatomic_t *p, ndatomic_t cmpval, ndatomic_t exchval)
{
  char ret;
  ndatomic_t readval;
  __asm__ __volatile__ (LOCK" cmpxchgl %3, %1; sete %0"
			: "=q" (ret), "=m" (*p), "=a" (readval)
			: "r" (exchval), "m" (*p), "a" (cmpval)
			: "memory");
  return ret;
}

/* Atomically swap memory location [32 bits] with `newval'*/
/*
	ret = *p ;
	*p =newval ;
	return ret ;
*/

static __INLINE__ ndatomic_t  nd_atomic_swap(ndatomic_t * ptr, ndatomic_t x )
{
	__asm__ __volatile__("xchgl %0,%1"
			:"=r" (x)
			:"m" (*ptr), "0" (x)
			:"memory");
		
	return x;
}

/* (*val)++ 
	return *val;
 */
static __INLINE__ int nd_atomic_inc(int *val)
{
	register int oldval;
	do {
		oldval = *val ;
	}while (!nd_compare_swap(val, oldval, oldval+1)) ;
	return oldval+1 ;
}

/* (*val)++
	return *val;
 */
static __INLINE__ int nd_atomic_dec(int *val)
{
	register int oldval;
	do {
		oldval = *val ;
	}while (!nd_compare_swap(val, oldval, oldval-1)) ;
	return oldval-1 ;
}

/* (*val)+= nstep 
	return *val;
 */
static __INLINE__ int nd_atomic_add(int *val, int nstep)
{
	register int oldval;
	do {
		oldval = *val ;
	}while (!nd_compare_swap(val, oldval, oldval+nstep)) ;
	return oldval;
}

/* (*val)-=nstep
	return *val;
 */
static __INLINE__ int nd_atomic_sub(int *val, int nstep)
{
	register int oldval;
	do {
		oldval = *val ;
	}while (!nd_compare_swap(val, oldval, oldval-nstep)) ;
	return oldval;
}

#define nd_atomic_set(p, val)    nd_atomic_swap(p, val)
#define nd_atomic_read(p)        (*(p))
#endif 


#elif defined(__BSD__)

typedef int ndatomic_t ;

#include <sys/types.h>
#include <machine/atomic.h>
//#define nd_atomic_swap(p, val) (atomic_store_rel_int(p,val),*(p)) 
static inline  int nd_testandset(volatile int *p) {return !atomic_cmpset_rel_int(p,0,1);}
#define nd_compare_swap(p,compare,exchange) atomic_cmpset_rel_int(p,compare,exchange)
static inline int nd_atomic_swap(volatile ndatomic_t *p ,ndatomic_t exch)
{
	register int oldval;
	do {
		oldval = *p ;
	}while (!atomic_cmpset_rel_int(p, oldval, exch)) ;
	return oldval ;
}
#define nd_atomic_add(p, val)  atomic_fetchadd_int(p, val)
#define nd_atomic_sub(p,val)  atomic_subtract_int(p, val)

#define nd_atomic_inc(p) (atomic_add_rel_int(p,1),*(p))
#define nd_atomic_dec(p) (atomic_subtract_rel_int(p,1),*(p))

#define nd_atomic_set(p, val)    atomic_set_int(p, val)
#define nd_atomic_read(p)        (*(p))

#elif defined(__MAC_OS__)
#include <libkern/OSAtomic.h>
typedef int ndatomic_t ;
//define for xcode
#define nd_compare_swap(p,compare,exchange) OSAtomicCompareAndSwapInt(compare, exchange,p)

static inline  int nd_testandset(volatile ndatomic_t *p) {return !nd_compare_swap(p,0,1);}
static inline int nd_atomic_swap(volatile ndatomic_t *p ,ndatomic_t exch)
{
    int oldval;
    do {
        oldval = *p ;
    }while (!nd_compare_swap(p, oldval, exch)) ;
    return oldval ;
}
#define nd_atomic_add(p, val)  OSAtomicAdd32(val, p)
#define nd_atomic_sub(p,val)  OSAtomicAdd32(-val, p)


#define nd_atomic_inc(p) OSAtomicIncrement32(p)
#define nd_atomic_dec(p) OSAtomicIncrement32(p)

static inline void nd_atomic_set(volatile ndatomic_t *p, ndatomic_t val)
{
    *p = val ;
}
#define nd_atomic_read(p)        (*(p))
#else 
#error unknow platform!
#endif

#endif
