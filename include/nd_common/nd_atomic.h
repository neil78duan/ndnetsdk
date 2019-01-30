/* file : nd_atomic.h
 * define atomic operation
 *
 * version 1.0 
 * all right reserved by neil duan 
 * 2007 -10
 */

#ifndef _ND_ATOMIC_H_
#define _ND_ATOMIC_H_

#include "nd_common/nd_comcfg.h"
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
#if defined(__ND_WIN__) 

#if _MSC_VER < 1300 // 1200 == VC++ 6.0

typedef LONG ndatomic_t ;
#else 
typedef  LONG ndatomic_t ;
#endif

//atomic operate
__INLINE__ int nd_compare_swap(ndatomic_t *lpDest,ndatomic_t lComp,ndatomic_t lExchange)
{
#if _MSC_VER < 1300 // 1200 == VC++ 6.0
	return (int) (lComp==(LONG)_InterlockedCompareExchange((PVOID*)lpDest, (PVOID)lExchange,(PVOID)lComp) );
#else 
	return (int) (lComp==(LONG)_InterlockedCompareExchange(lpDest, lExchange, lComp) );
#endif
//#endif
}
#define nd_atomic_inc(p)	_InterlockedIncrement(p) 
#define nd_atomic_dec(p)	_InterlockedDecrement(p) 
#define nd_atomic_add(p,step)	_InterlockedExchangeAdd((long*)(p),step) 
#define nd_atomic_sub(p,step)	_InterlockedExchangeAdd((long*)(p),-(step))
#define nd_atomic_swap(p, val)	_InterlockedExchange(p,val) 
#define nd_testandset(p)	_InterlockedExchange(p,1) 

#define nd_atomic_set(p, val)    _InterlockedExchange((p), val)
#define nd_atomic_read(p)        (*(p))



#elif defined(__ND_LINUX__) || defined(__ND_ANDROID__)
typedef int ndatomic_t ;

#define nd_atomic_inc(x) __sync_add_and_fetch((x),1)  
#define nd_atomic_dec(x) __sync_sub_and_fetch((x),1)  
#define nd_atomic_add(x,y) __sync_fetch_and_add((x),(y))
#define nd_atomic_sub(x,y) __sync_fetch_and_sub((x),(y))
#define nd_compare_swap(dest, cmp, exchval) (int)__sync_bool_compare_and_swap(dest, cmp, exchval)
static __INLINE__ ndatomic_t nd_atomic_swap(volatile ndatomic_t *p ,ndatomic_t exch)
{
    int oldval;
    do {
        oldval = *p ;
    }while (!nd_compare_swap(p, oldval, exch)) ;
    return oldval ;
}
#define nd_testandset(p) __sync_fetch_and_add((p),1)
#define nd_atomic_set(p, val)    nd_atomic_swap(p,val)
#define nd_atomic_read(p)        (*(p))



#elif defined(__ND_BSD__)

typedef int ndatomic_t ;

#include <sys/types.h>
#include <machine/atomic.h>

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

#elif defined(__ND_MAC__) || defined(__ND_IOS__)
//#define OSATOMIC_USE_INLINED 1
//#include <libkern/OSAtomic.h>
typedef int ndatomic_t ;
#ifdef __cplusplus
#include <atomic>
typedef std::atomic<int32_t> _OSAtomic_int32_t;
static __INLINE__ bool NDAtomicCompareAndSwap32(int32_t __oldValue, int32_t __newValue,
						 volatile int32_t *__theValue)
{
	return (std::atomic_compare_exchange_strong_explicit)(
			(volatile _OSAtomic_int32_t*)__theValue, &__oldValue, __newValue, std::memory_order_relaxed,std::memory_order_relaxed);
}
static __INLINE__ int32_t NDAtomicAdd32Barrier(int32_t __theAmount, volatile int32_t *__theValue)
{
	return std::atomic_fetch_add_explicit((volatile _OSAtomic_int32_t*) __theValue, __theAmount,
										  std::memory_order_seq_cst) + __theAmount;
}

#else
#include <stdatomic.h>
typedef _Atomic(int32_t) _OSAtomic_int32_t;
static __INLINE__ int NDAtomicCompareAndSwap32(int32_t __oldValue, int32_t __newValue,
												volatile int32_t *__theValue)
{
	return (int)atomic_compare_exchange_strong_explicit((volatile _OSAtomic_int32_t*)__theValue, &__oldValue, __newValue, memory_order_relaxed, memory_order_relaxed);
}
static __INLINE__ int32_t NDAtomicAdd32Barrier(int32_t __theAmount, volatile int32_t *__theValue)
{
	return atomic_fetch_add_explicit((volatile _OSAtomic_int32_t*) __theValue, __theAmount,
										  memory_order_seq_cst) + __theAmount;
}
#endif



//define for xcode
#define nd_compare_swap(p,compare,exchange) NDAtomicCompareAndSwap32(compare, exchange,p)
static inline  int nd_testandset(volatile ndatomic_t *p) {return !nd_compare_swap(p,0,1);}
static inline int nd_atomic_swap(volatile ndatomic_t *p ,ndatomic_t exch)
{
	ndatomic_t oldval;
	do {
		oldval = *p ;
	}while (!nd_compare_swap(p, oldval, exch)) ;
	return (int)oldval ;
}
#define nd_atomic_add(p,val)  NDAtomicAdd32Barrier(val, p)
#define nd_atomic_sub(p,val)  NDAtomicAdd32Barrier(-val, p)


#define nd_atomic_inc(p) NDAtomicAdd32Barrier(1,p)
#define nd_atomic_dec(p) NDAtomicAdd32Barrier(-1,p)

static inline void nd_atomic_set( ndatomic_t *p, ndatomic_t val)
{
	*p = val ;
}
#define nd_atomic_read(p)        (*(p))


#else 
#error unknow platform!

#endif

#endif
