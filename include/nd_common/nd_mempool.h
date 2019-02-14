/* fiel nd_mempool.h
 * define a simple memory pool 
 * version 1.0 
 * neil duan 
 * 2008-9
 */

/*
 * memory pool module implemention 
 */
#ifndef _ND_MEMPOOL_H_
#define _ND_MEMPOOL_H_

#include "nd_common/nd_comcfg.h"

#define MIN_ALLOC_SIZE		ALIGN_SIZE

#define _ND_ALINE(_size, _aline) (((_size)+(_aline)-1) & (~((_aline)-1)))
#define ND_ROUNDUP4(_size) _ND_ALINE(_size, 4)
#define ND_ROUNDUP8(_size) _ND_ALINE(_size, 8)
#define ND_ROUNDUP16(_size) _ND_ALINE(_size, 16)

//mempool tyep enum
enum enem_pool_type{
	EMEMPOOL_TINY =  64*1024,		//mini memory pool
	EMEMPOOL_NORMAL = 256*1024,		//common pool
	EMEMPOOL_HUGE	= 1024*1024,		//huge pool
	EMEMPOOL_UNLIMIT = -1			//unlimited pool
};

#ifdef ND_UNUSE_STDC_ALLOC
//callback on memeoy release 
typedef void (*memdestruct_entry)(nd_handle pool, void *addr) ;

//the memory system init / destroy entries .
ND_COMMON_API int nd_mempool_root_init(void) ;
ND_COMMON_API void nd_mempool_root_release(void);

//alloc / free functions 
ND_COMMON_API nd_handle nd_pool_create(size_t size,const char *name ) ;	//on error return NULL
ND_COMMON_API int nd_pool_destroy(nd_handle pool, int flag);		//

ND_COMMON_API void *nd_pool_alloc_real(nd_handle pool , size_t size);	//alloc
ND_COMMON_API void nd_pool_free_real(nd_handle pool ,void *addr) ;		//free
ND_COMMON_API void nd_pool_reset(nd_handle pool) ;					//reset a memory pool
ND_COMMON_API void nd_pool_set_trace(nd_handle pool, int flag) ;					//reset a memory pool
ND_COMMON_API void nd_pool_set_parmament(nd_handle pool);

ND_COMMON_API size_t nd_pool_freespace(nd_handle pool) ;	//get free space 
ND_COMMON_API void nd_pool_destruct_entry(nd_handle pool , memdestruct_entry func) ;

/*get default global pool*/
ND_COMMON_API nd_handle nd_global_mmpool(void) ;

ND_COMMON_API void* nd_pool_realloc(nd_handle pool ,void *oldaddr, size_t newsize) ;


#if  defined(ND_SOURCE_TRACE) 
ND_COMMON_API void nd_mmpool_dump(void) ;
#else 
#define nd_mmpool_dump()		// 
#endif 

#ifdef ND_MEM_CHECK

typedef void *(*nd_alloc_func)(nd_handle _pool,size_t __s) ;		//for debug
typedef void (*nd_free_func)(nd_handle _pool,void *__p) ;			//for debug

ND_COMMON_API void *nd_alloc_check(nd_handle _pool,size_t __n,const char *file, int line, nd_alloc_func allocfn) ;
ND_COMMON_API void nd_free_check(nd_handle _pool,void *__p, nd_free_func freefn) ;

#define nd_pool_alloc(_pool, _size) nd_alloc_check(_pool, _size, __FILE__ , __LINE__ , nd_pool_alloc_real) 
#define nd_pool_free(_pool, _addr)  nd_free_check(_pool, _addr,nd_pool_free_real)


#else 
#define nd_pool_alloc nd_pool_alloc_real
#define nd_pool_free  nd_pool_free_real

#endif 

#ifdef ND_MEM_STATICS

//statics memory used info 
ND_COMMON_API int nd_mm_statics_start(void) ;
ND_COMMON_API int nd_mm_statics_end(void) ;

#else 

static __INLINE__ int nd_mm_statics_start(void) {return 0;}
static __INLINE__ int nd_mm_statics_end(void) {return 0;}

#endif 

#else //ND_UNUSE_STDC_ALLOC
//////////////////////////////////////////////////////////////////////////

//
typedef void (*memdestruct_entry)(nd_handle pool, void *addr) ;

//
static __INLINE__ int nd_mempool_root_init(void) {return 0;}
static __INLINE__ void nd_mempool_root_release(void){}
ND_COMMON_API nd_handle nd_pool_create(size_t size,const char *name );
ND_COMMON_API  int nd_pool_destroy(nd_handle pool, int flag);
ND_COMMON_API  void *nd_pool_alloc_real(nd_handle pool , size_t size);
ND_COMMON_API void nd_pool_free_real(nd_handle pool ,void *addr) ;

ND_COMMON_API void nd_pool_reset(nd_handle pool) ;

static __INLINE__   void nd_pool_set_parmament(nd_handle pool) {}
static __INLINE__  void nd_pool_set_trace(nd_handle pool, int flag)
{

}
// static __INLINE__ void *nd_pool_alloc_l(nd_handle pool , size_t size) 
// {
// 	return nd_pool_alloc_real(pool, size) ;
// }
// static __INLINE__ void nd_pool_free_l(nd_handle pool ,void *addr, size_t size) 
// {
// 	nd_pool_free_real(pool, addr) ;
// }

static __INLINE__ size_t nd_pool_freespace(nd_handle pool) 
{
	return (size_t)-1;
}
ND_COMMON_API void nd_pool_destruct_entry(nd_handle pool , memdestruct_entry func) ;


ND_COMMON_API nd_handle nd_global_mmpool(void) ;

#define nd_pool_alloc nd_pool_alloc_real
#define nd_pool_free  nd_pool_free_real

static __INLINE__ int nd_mm_statics_start(void) {return 0;}
static __INLINE__ int nd_mm_statics_end(void) {return 0;}
#define nd_mmpool_dump (void)0 

#endif


ND_COMMON_API int nd_addr_checkvalid(void *addr) ;			//check the addr is valid in current OS system
ND_COMMON_API int nd_alloc_checkvalid(void *addr) ;	//check addr alloced by memory pool is valid
#define ND_ALLOC_MM_VALID(a) nd_alloc_checkvalid(a) 
#define ND_ADDR_VALID(a) nd_addr_checkvalid(a) 
#endif
