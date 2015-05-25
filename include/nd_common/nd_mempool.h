/* fiel nd_mempool.h
 * define a simple memory pool 
 * version 1.0 
 * neil duan 
 * 2008-9
 */

/*
 * 这是一个特殊的mempool 
 * 只提供了对特定长度和个数的内存缓冲池.
 * 在需要使用多个同样大小的结构或者内存块的是,可以申请一个内存池,
 * 然后每次使用的时候从这个内存池中申请一个内存块出来,不用了就可以释放给内存池.
 */
#ifndef _ND_MEMPOOL_H_
#define _ND_MEMPOOL_H_

#include "nd_common/nd_comcfg.h"

#define MIN_ALLOC_SIZE		ALIGN_SIZE

//内存池类型
enum emem_pool_type{
	EMEMPOOL_TINY =  64*1024,		//微型内存池
	EMEMPOOL_NORMAL = 256*1024,		//普通大小内存池
	EMEMPOOL_HUGE	= 1024*1024,		//巨型内存池
	EMEMPOOL_UNLIMIT = -1			//无限制内存池
};

#ifdef ND_UNUSE_STDC_ALLOC
//定义内存析构函数
typedef void (*memdestruct_entry)(nd_handle pool, void *addr) ;

//内存池模块初始化/释放函数
ND_COMMON_API int nd_mempool_root_init() ;
ND_COMMON_API void nd_mempool_root_release();

//内存池操作函数
ND_COMMON_API nd_handle nd_pool_create(size_t size,const char *name ) ;	//创建一个内存池,返回内存池地址
ND_COMMON_API int nd_pool_destroy(nd_handle pool, int flag);		//销毁一个内存缓冲池
ND_COMMON_API void *nd_pool_alloc_real(nd_handle pool , size_t size);	//从缓冲池中申请一个内存
ND_COMMON_API void nd_pool_free_real(nd_handle pool ,void *addr) ;		//释放一个内存
ND_COMMON_API void nd_pool_reset(nd_handle pool) ;					//reset a memory pool
ND_COMMON_API void nd_pool_set_trace(nd_handle pool, int flag) ;					//reset a memory pool

ND_COMMON_API size_t nd_pool_freespace(nd_handle pool) ;	//get free space 
ND_COMMON_API void nd_pool_destruct_entry(nd_handle pool , memdestruct_entry func) ;

/*得到全局默认的内存池*/
ND_COMMON_API nd_handle nd_global_mmpool() ;

ND_COMMON_API void* nd_pool_realloc(nd_handle pool ,void *oldaddr, size_t newsize) ;


#if  defined(ND_SOURCE_TRACE) && defined(ND_UNUSE_STDC_ALLOC)
ND_COMMON_API void nd_mmpool_dump() ;
#else 
#define nd_mmpool_dump()		// 
#endif 

#ifdef ND_MEM_CHECK

//带有日志跟踪的分配函数
//ND_COMMON_API void *nd_pool_alloc_trace(nd_handle pool , size_t size, char *file, int line);
//ND_COMMON_API void nd_pool_free_trace(nd_handle pool , void *p);

typedef void *(*nd_alloc_func)(nd_handle _pool,size_t __s) ;		//定义内存申请函数指针
typedef void (*nd_free_func)(nd_handle _pool,void *__p) ;			//定义内存释放函数指针

ND_COMMON_API void *nd_alloc_check(nd_handle _pool,size_t __n,const char *file, int line, nd_alloc_func allocfn) ;
ND_COMMON_API void nd_free_check(nd_handle _pool,void *__p, nd_free_func freefn) ;

#define nd_pool_alloc(_pool, _size) nd_alloc_check(_pool, _size, __FILE__ , __LINE__ , nd_pool_alloc_real) 
#define nd_pool_free(_pool, _addr)  nd_free_check(_pool, _addr,nd_pool_free_real)


#else 
#define nd_pool_alloc nd_pool_alloc_real
#define nd_pool_free  nd_pool_free_real

#endif 

#ifdef ND_MEM_STATICS

//内测统计
ND_COMMON_API int nd_mm_statics_start() ;
ND_COMMON_API int nd_mm_statics_end() ;

#else 

static __INLINE__ int nd_mm_statics_start() {return 0;}
static __INLINE__ int nd_mm_statics_end() {return 0;}

#endif 

#else //ND_UNUSE_STDC_ALLOC
//////////////////////////////////////////////////////////////////////////

//定义内存析构函数
typedef void (*memdestruct_entry)(nd_handle pool, void *addr) ;

//内存池模块初始化/释放函数
static __INLINE__ int nd_mempool_root_init() {return 0;}
static __INLINE__ void nd_mempool_root_release(){} 
ND_COMMON_API nd_handle nd_pool_create(size_t size,const char *name );
ND_COMMON_API  int nd_pool_destroy(nd_handle pool, int flag);
ND_COMMON_API  void *nd_pool_alloc_real(nd_handle pool , size_t size);
ND_COMMON_API void nd_pool_free_real(nd_handle pool ,void *addr) ;

ND_COMMON_API void nd_pool_reset(nd_handle pool) ;
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

/*得到全局默认的内存池*/
ND_COMMON_API nd_handle nd_global_mmpool() ;

#define nd_pool_alloc nd_pool_alloc_real
#define nd_pool_free  nd_pool_free_real

static __INLINE__ int nd_mm_statics_start() {return 0;}
static __INLINE__ int nd_mm_statics_end() {return 0;}
#define nd_mmpool_dump (void)0 

#endif


ND_COMMON_API int nd_addr_checkvalid(void *addr) ;			//检测内存地址是否合法
ND_COMMON_API int nd_alloc_checkvalid(void *addr) ;	//检测malloc分配的地址是否合法
#define ND_ALLOC_MM_VALID(a) nd_alloc_checkvalid(a) 
#define ND_ADDR_VALID(a) nd_addr_checkvalid(a) 
#endif
