/* file: ND_memory.h
 * define memory manager 
 * version 1.0
 * neil
 * 2005-11-15
 */

#ifndef _ND_MEMORY_H_
#define _ND_MEMORY_H_

#include "nd_common/nd_common.h"
#include "nd_common/nd_comcfg.h"
#include "nd_common/nd_mempool.h"

#ifdef ND_UNUSE_STDC_ALLOC

#ifdef ND_IMPLETE_MEMPOOL
#error can not include this file in current c-file
#endif 

#undef free 
#undef malloc
#undef realloc


static __INLINE__ void *nd_malloc(size_t size, const char *file, int line)
{
#ifdef ND_MEM_CHECK
	void *p = nd_alloc_check(nd_global_mmpool(),size,file, line, nd_pool_alloc_real )  ;
#else
	void *p = nd_pool_alloc(nd_global_mmpool(),size) ;
#endif
	return p ;
}

static __INLINE__ void nd_free(void *p)
{
// 	if(p) {

#ifdef ND_MEM_CHECK
		nd_free_check(nd_global_mmpool(),p,nd_pool_free_real ) ;
#else
		nd_pool_free(nd_global_mmpool(),p) ;
#endif
// 	}
}

static __INLINE__ void *nd_realloc(void *oldp, size_t newsize)
{
	if(oldp) {
		return nd_pool_realloc(nd_global_mmpool(), oldp, newsize) ;
	}
	return 0;
}


#define malloc(__n)			nd_malloc(__n,__FILE__, __LINE__) 
#define free(__p)			nd_free(__p)
#define realloc(__p,__s)	nd_realloc(__p,__s)

#else 
#endif	//	ND_UNUSE_STDC_ALLOC

#endif	// end header file 
