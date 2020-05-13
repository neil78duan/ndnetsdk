/* file nd_new.h
 *
 * implemention of new/delete 
 *
 * create by duan 
 * 2011/7/22 11:14:11
 */

#ifndef _ND_NEW_H_
#define _ND_NEW_H_

#include "nd_common/nd_comcfg.h"
CPPAPI void *_alloc_new(size_t size,nd_handle pool);
CPPAPI void _free_delete(void *p,nd_handle pool);
CPPAPI void _init_pool_for_new() ;
CPPAPI void _destroy_pool_for_new() ;

#include <new>
//#pragma auto_inline
//#if defined(ND_UNUSE_STDC_ALLOC)
#pragma optimize( "", off )
#pragma auto_inline (off)

#if defined(_MSC_VER)
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
static inline void * operator new(size_t, void *&_P)
{
	return (_P); 
}
static inline void * operator new(size_t, void *_P)
{
	return (_P); 
}

static inline void  operator delete(void *, void *)
{
	return; 
}
#endif
#define __new_throw
#elif defined(__ND_LINUX__)
#define __new_throw throw (std::bad_alloc)
#else
#define __new_throw 
#endif 

#ifdef ND_OVER_RIDE_NEW
inline void *operator new(size_t size,nd_handle pool) __new_throw
{
	return _alloc_new( size, pool);
}
inline void *operator new[](size_t size,nd_handle pool)  __new_throw
{
	return _alloc_new( size, pool);
}

inline void operator delete(void *p,nd_handle pool) throw()
{
	_free_delete(p, pool) ;
}
inline void operator delete[](void *p,nd_handle pool) throw()
{
	_free_delete(p, pool) ;
}

#pragma auto_inline (on)
#pragma optimize( "", on ) 

void *operator new(size_t size) __new_throw;
void *operator new[](size_t size)  __new_throw;

void operator delete(void *p) throw();
void operator delete[](void *p) throw();
#endif 
//#endif //ND_UNUSE_STDC_ALLOC

#endif 
