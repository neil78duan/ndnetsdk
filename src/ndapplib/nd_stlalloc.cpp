/* nd_allocator.cpp
 *
 * allocator of nd app
 *
 *
 */ 

#include "nd_common/nd_common.h"
#include "ndapplib/nd_allocator.h"
#include "nd_common/nd_alloc.h"
#include "ndapplib/nd_object.h"

#include "ndapplib/nd_new.h"

static nd_handle s_stl_mmpool ;

void *_malloc_pool(size_t size,void *pool) 
{
	if (!pool){
		pool = (void*) nd_stlmmpool() ;
	}
	return _alloc_new( size,(nd_handle) pool) ;
	/*
	ND_TRACE_FUNC() ;
	nd_handle h_pool = NULL;
	if (pool){
		h_pool = (nd_handle)pool;
	}
	else {
		h_pool =  nd_stlmmpool()  ;
		if (!h_pool)	{
			return NULL;
		}
	}

#ifdef ND_MEM_CHECK
	{
		char buf[ND_FILE_PATH_SIZE] = "unknow";
		nd_get_callstack_desc(buf, sizeof(buf)) ;
		return nd_alloc_check(h_pool, size, buf , 0,nd_pool_alloc_trace)  ;
	}
#else 
	return nd_pool_alloc(h_pool, size) ;
#endif 
	*/
}


void _free_pool(void *p,void *pool) 
{
	_free_delete(p,(nd_handle) pool) ;
// 	ND_TRACE_FUNC() ;
// 	if (pool){
// 		nd_pool_free((nd_handle)pool,p);
// 	}
// 	else if(s_stl_mmpool) {
// 		nd_pool_free(s_stl_mmpool,p) ;
// 	}
}

int create_stl_allocator() 
{
	ND_TRACE_FUNC() ;
#if defined USE_ND_ALLOCATOR
	s_stl_mmpool = nd_pool_create(EMEMPOOL_UNLIMIT,"stl_pool") ;
	if (s_stl_mmpool)	{
		nd_pool_set_trace(s_stl_mmpool,0) ;
		return 0 ;
	}
	return -1 ;
#else 
	return 0;
#endif 
}

void destroy_stl_allocator() 
{
	ND_TRACE_FUNC() ;
	if(s_stl_mmpool) {
		nd_pool_destroy(s_stl_mmpool,0 ) ;
		s_stl_mmpool = 0 ;
	}
}

nd_handle nd_stlmmpool() 
{
	ND_TRACE_FUNC() ;
	if(!s_stl_mmpool) {
		s_stl_mmpool = nd_pool_create(EMEMPOOL_UNLIMIT,"stl_pool") ;
		if(!s_stl_mmpool) {
			return NULL ;
		}
	}
	return s_stl_mmpool ;
}