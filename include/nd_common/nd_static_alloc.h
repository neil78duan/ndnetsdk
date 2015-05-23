/* file : nd_static_alloc.h
 *
 * static memory allocator
 *
 * create by neil duan, all right reserved 
 */

#ifndef _ND_STATIC_ALLOC_H_
#define _ND_STATIC_ALLOC_H_

typedef nd_handle nd_sa_handle ;		//static allocator handle 
/*
 * create a memory alloc that only allocate specified size and max number
 */

ND_COMMON_API nd_sa_handle nd_sa_create(int node_num, size_t node_size, nd_handle mempool);

ND_COMMON_API void* nd_sa_alloc(nd_sa_handle sa_handle) ;

ND_COMMON_API void nd_sa_free(void* addr, nd_sa_handle sa_handle ) ;

ND_COMMON_API int nd_sa_capacity(nd_sa_handle sa_handle)  ;			

ND_COMMON_API int nd_sa_freenum(nd_sa_handle sa_handle) ;	

ND_COMMON_API int nd_sa_destroy(nd_sa_handle sa_handle, int flag) ;

ND_COMMON_API size_t nd_sa_getsize(nd_sa_handle sa_handle) ;

#endif
