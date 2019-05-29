/* file nd_new.cpp
 *
 * implemention of new/delete 
 *
 * create by duan 
 * 2011/7/22 11:14:11
 */

#include "nd_common/nd_common.h"
//#define ND_IMPLETE_MEMPOOL	1		//use nd-memory pool system
#include "ndstl/nd_new.h"
#include "ndstl/nd_allocator.h"
#include "ndapplib/nd_object.h"

#ifdef _ND_MEMORY_H_
// must use libc malloc
#error do not include nd_alloc.h
#endif

//static nd_handle _s_pool_for_new = 0 ;

//static int __isdebug = 0 ;
//static int __look_size = 0 ;

#ifdef ND_UNUSE_STDC_ALLOC
#include "nd_common/nd_mempool.h"
//implemention of new
void *_alloc_new(size_t size,nd_handle pool)
{
	//ND_TRACE_FUNC() ;
	if (!pool)
		pool = nd_global_mmpool();

	nd_assert(pool) ;
	if(size == 0)
		size = 1 ;

	return nd_pool_alloc(pool ,  size); ;
}

//implemention of delete
void _free_delete(void *p,nd_handle pool)
{
	//ND_TRACE_FUNC() ;
	if (ND_ADDR_VALID(p)){
		nd_pool_free(NULL, p) ;
	}
}
#else 
void *_alloc_new(size_t size,nd_handle pool)
{
	ND_TRACE_FUNC() ;
	if(size == 0)
		size = 1 ;

	return ::malloc(size); 
}

//implemention of delete
void _free_delete(void *p,nd_handle pool)
{
	ND_TRACE_FUNC() ;
	if (ND_ADDR_VALID(p)){
		free(p) ;
	}
}
#endif

void _init_pool_for_new() 
{
}

void _destroy_pool_for_new() 
{
}


#ifdef ND_OVER_RIDE_NEW

void *operator new(size_t size) __new_throw
{
	return _alloc_new( size, NULL);
}
void *operator new[](size_t size)  __new_throw
{
	return _alloc_new( size, NULL);
}
void operator delete(void *p) throw()
{
	_free_delete(p, NULL) ;
}
void operator delete[](void *p) throw()
{
	_free_delete(p, NULL) ;
}
#endif

