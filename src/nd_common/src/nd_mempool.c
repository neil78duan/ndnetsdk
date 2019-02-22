/* fiel nd_mempool.c
 * define a simple memory pool
 * version 1.0
 * neil duan
 * 2008-9
 */


#define ND_IMPLETE_MEMPOOL	1		//need implete memmory pool
#define ND_IMPLEMENT_HANDLE 1

#include "nd_common/nd_os.h"
#include "nd_common/list.h"
#include "nd_common/nd_logger.h"
#include "nd_common/_source_trace.h"

#ifdef ND_UNUSE_STDC_ALLOC	//not use std alloc
typedef struct nd_mm_pool *nd_handle ;
#include "nd_common/nd_handle.h"
#include "nd_common/nd_mempool.h"


#define ND_DEFAULT_ALINE_SIZE	16
#define MIN_SIZE				16
#define ALIGN_SIZE				16

/*
#ifdef X86_64
#define ND_DEFAULT_ALINE_SIZE	8
#define MIN_SIZE				16
#define ALIGN_SIZE				16
#else
#define MIN_SIZE				8
#define ND_DEFAULT_ALINE_SIZE	8
#define ALIGN_SIZE				8
#endif
*/

#define SIZE_ALINE(s)			max(_ND_ALINE(s,ALIGN_SIZE), MIN_SIZE)

#define BIG_SIZE				0x400			// if bigger than this ,is big data 
#define LITTLE_CHUNK_NUM		((BIG_SIZE/ALIGN_SIZE)- MIN_SIZE/ALIGN_SIZE +1)	//little block list (16~256) step is 16
#define CHUNK_INDEX(size)		(((size)/ALIGN_SIZE) - MIN_SIZE/ALIGN_SIZE )	//chunk index 
#define GET_LITTLE_SIZE(index)	(((index)+1) *ALIGN_SIZE + MIN_SIZE-ALIGN_SIZE)
#define LITTLE_ROUND_LOWWER(size)	((size) & ~(ALIGN_SIZE-1))

#define BIG_CHUNK_NUM			64	//big block chunk (1k , 2k ..., 16k)
#define BIG_ALINE(size)			max(_ND_ALINE(size,BIG_SIZE), BIG_SIZE)
#define BIG_INDEX(size)			(size /(BIG_SIZE) -1 )	//
#define GET_BIG_SIZE(index)		(((index)+1) *BIG_SIZE)
#define BIG_ROUND_LOWWER(size)	((size) & ~(BIG_SIZE-1))

#define ALLOCATOR_PAGE_BITMASK  0xFFF
#define POOL_SIZE_BITS			16			//lower 16 bits (not used)

#define DEFAULT_PAGE_SIZE		getgranularity()  //(1024*32)
#define MIN_PAGE_SIZE			getgranularity()
#define SYS_PAGE_SIZE			getpagesize()
#define ROUND_PAGE_SIZE(s)		((s) + MIN_PAGE_SIZE -1) & (~((size_t)MIN_PAGE_SIZE-1))
#define PAGE_ALINE(s)			max(_ND_ALINE(s,SYS_PAGE_SIZE), MIN_PAGE_SIZE)
#define DIRECT_ALLOC_SIZE		(MIN_PAGE_SIZE-SYS_PAGE_SIZE)
typedef size_t allocheader_t ;

#define UNLIMITED_SIZE		((allocheader_t)-1)

#pragma pack(push, ND_DEFAULT_ALINE_SIZE)

// alloced address info
struct alloc_node {
	allocheader_t size ;
	char data[0];
};

//alloc from system info
struct big_chunk_list
{
	void *pool ;
	struct list_head list ;
	allocheader_t size ;
	char data[0];
};

struct free_node {
	allocheader_t size ;
	struct free_node *next ;
};
 struct nd_mm_pool ;
//sub allocator
typedef struct mm_sub_allocator
{
	allocheader_t size ;
	NDUINT16 type ;
	NDUINT16 myerrno;
	allocheader_t allocated_size ;					//already alloc size 
	struct nd_mm_pool *parent ;					//
	char *start, *end ;									//alloced memory start address 
	struct list_head self_list;							//self_list in parent-list
	struct free_node *big_list[BIG_CHUNK_NUM];			// big-block list [BIG_SIZE]
	struct free_node *free_littles[LITTLE_CHUNK_NUM] ;	//little-block list (the first two elemets not used,so I store *BIG_SIZE* list in free_littles[0])
}nd_sub_allocator;

//memory pool info
typedef struct nd_mm_pool
{
	ND_OBJ_BASE ;
	allocheader_t capacity ;							//original capacity
	allocheader_t allocated_size ;						//allocated memory size 
//#ifdef ND_MEM_CHECK
	unsigned int alloc_granularity:16;					//sub allocator granularity  *64k
	unsigned int free_allocator_num:8 ;					// free-allocator numbers	
	unsigned int in_used:1 ;							//state is in used 
	unsigned int permament : 1;							//it would be release untill all memeory in this pool is dealloc
//#endif
	memdestruct_entry destruct_func ;					//destruct function when memory free
	struct list_head self_list;							//list node 
	struct list_head allocator_list;					//using sub allocator
	struct list_head free_allocator;					//sub allocator
	struct list_head page_list;							//mem page size > 64k -sizeof(nd_sub_allocator)
	nd_sub_allocator *cur_allocator;					//current allocator
	nd_mutex lock ;										//lock

	nd_sub_allocator _allocator[0] ;
}nd_mmpool_t;


#pragma pack(pop)

struct pool_root_allocator{
	int init ;
	int is_destroied;
	nd_mutex lock ;										//lock
	size_t free_size ;
	struct list_head inuser_list;						//active memory pool
} ;
nd_mmpool_t *s_common_mmpool = NULL;						// default global memory pool
static struct pool_root_allocator  __mem_root ;		//root of pools system

#ifdef ND_MEM_STATICS
void _erase_mmstatics(void *addr) ;
void _insertinto_mmstatics(nd_handle pool, void *addr, size_t size )  ;
void remove_statics(nd_mmpool_t *pool) ;
#define TRYTO_MMSTATIC(_pool, _addr, _size)	_insertinto_mmstatics(_pool, _addr, _size)
#define ERASE_MMSTATICS(_addr) _erase_mmstatics(_addr)
#else
#define TRYTO_MMSTATIC(_pool, _addr, _size) (void) 0
#define ERASE_MMSTATICS(_addr)  (void) 0
#endif


nd_mmpool_t *nd_global_mmpool(void)
{
	if (!s_common_mmpool){
		nd_mempool_root_init() ;
	}
    
	return s_common_mmpool ;
}

typedef void (*walk_trunk_node) (nd_mmpool_t *pool, void *startaddr, size_t size) ;
static void _walk_alloced_inpool(nd_mmpool_t *pool, walk_trunk_node func ) ;

//destruct all memory
//static void safe_destruct(nd_mmpool_t *pool) ;
static void destruct_mm_entry(nd_mmpool_t *pool, void *startaddr, size_t size) ;
#define safe_destruct(_pool) _walk_alloced_inpool(_pool,destruct_mm_entry)
//dump mem leak
static void _dump_trunk_leak(nd_mmpool_t *pool, void *startaddr, size_t size) ;
#define dump_memleak(_pool) _walk_alloced_inpool(_pool,_dump_trunk_leak)

//static void dump_memleak(nd_mmpool_t *pool) ;
static void *_get_user_addr(nd_mmpool_t *pool, void *p) ;
struct alloc_node *_user_addr_2sys( void *useraddr, size_t *user_len, size_t *alloc_len) ;
static size_t __get_real_size(struct alloc_node *alloc_addr) ;
static size_t __get_need_size(size_t user_size);
static int _tryto_desctroy(nd_mmpool_t *pool) ;
static __INLINE__ struct alloc_node * __user_addr_2allocated(void *addr)
{
	return  (((struct alloc_node *)addr) - 1);
}

//static __INLINE__ void* __allocated_2user_addr(struct alloc_node *alloc_addr)
//{
//	return (void*)(alloc_addr->data);
//}


#ifdef ND_DEBUG
static __INLINE__ void freeFillMem(void *p, size_t size)
{
    memset(p, 'c', size) ;
}
static __INLINE__ void allocFillMem(void *p, size_t size)
{
    memset(p, 0, size) ;
}
#else
static __INLINE__ void allocFillMem(void *p, size_t size)
{
}
static __INLINE__ void freeFillMem(void *p, size_t size)
{
}
#endif

#ifdef _MSC_VER
#define nd_mmap(s)     VirtualAlloc(NULL, (s) ,MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE) ;
#define nd_munmap(p,s)	VirtualFree((p),0, MEM_RELEASE);
int getgranularity(void)
{
	static int granularity = 0;
	if (granularity == 0) {
		SYSTEM_INFO system_info;
		GetSystemInfo(&system_info);
		granularity = max(system_info.dwPageSize,	system_info.dwAllocationGranularity);
	}
	return granularity;
}
int getpagesize(void)
{
	static int pagesize = 0;
	if (pagesize == 0) {
		SYSTEM_INFO system_info;
		GetSystemInfo(&system_info);
		pagesize = system_info.dwPageSize;
	}
	return pagesize;
}
static void * __sys_alloc(size_t size)
{
	void *ret = HeapAlloc(GetProcessHeap(), 0, (DWORD)size);
	if (!ret){
		NDUINT32 lsterr = nd_last_errno() ;
		nd_logerror("HeapAlloc(%d) ,errcode =%d :%s\n" AND size AND lsterr AND nd_str_error(lsterr)) ;
	}
	return ret ;
}
static void __sys_free(void *addr)
{
	if(!HeapFree(GetProcessHeap(),0,addr)) {
		nd_showerror() ;
	}
}

#else
#include <sys/mman.h>

#if defined(__ND_MAC__) || defined(__ND_IOS__)
#define MAP_ANONYMOUS MAP_ANON
#endif

#define nd_mmap(s)     mmap(0, (s), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)
#define nd_munmap(p,s) munmap((p), (s))
int getgranularity() {	return (64*1024) ;}
static void *__sys_alloc(size_t size )
{
    void *p = malloc(size) ;
	if (!p) {
		nd_logerror("malloc(%d) ,error :%s\n" AND size  AND nd_last_error()) ;
		return 0;
	}
    allocFillMem(p, size) ;
    return  p ;
}
static void __sys_free(void *p)
{
    free(p) ;
}

#endif
// page alloc / free
static void* __sys_page_alloc(size_t size)
{
	struct alloc_node *ret ;
	size = PAGE_ALINE(size) ;
	ret = (struct alloc_node *) nd_mmap( size ) ;
    //ret = (struct alloc_node *) malloc( size ) ;
	if (!ret){
		NDUINT32 lsterr = nd_last_errno() ;
		nd_logerror("nd_mmap(%x) ,errcode =%d :%s\n" AND size AND lsterr AND nd_str_error(lsterr)) ;
	}
	else {
        allocFillMem(ret, size) ;
		ret->size = size ;
	}
    nd_assert(0==((size_t)ret & ALLOCATOR_PAGE_BITMASK)) ;
    
	return ret ;
}

static void __sys_page_free(void *addr)
{
	struct alloc_node *p = (struct alloc_node *)addr ;
    size_t size = p->size ;
    freeFillMem(addr, size) ;
    
	nd_munmap(p,size) ;
    
    nd_assert(0==((size_t)p & ALLOCATOR_PAGE_BITMASK)) ;
    //free(p) ;
}


int nd_mempool_root_init(void)
{
	if (__mem_root.init==0) {
		INIT_LIST_HEAD(&__mem_root.inuser_list);
		nd_mutex_init(&__mem_root.lock);
		__mem_root.init = 1;
		__mem_root.is_destroied = 0;
	}

	if (!s_common_mmpool) {
		s_common_mmpool = nd_pool_create(EMEMPOOL_UNLIMIT, "nd_global_pool");
		nd_assert(s_common_mmpool);
		if (!s_common_mmpool) {
			nd_logerror("Init memory pool error \n");
		}
		else {
			nd_pool_set_trace(s_common_mmpool, 1);
			s_common_mmpool->permament = 1;
		}
	}
	return 0 ;
}

void nd_pool_set_parmament(nd_mmpool_t *pool)
{
	pool->permament = 1;
}

void nd_mempool_root_release(void)
{
	struct list_head *pos, *next ;
	nd_mmpool_t *pool;
	if (__mem_root.init==0){
		return ;
	}
	if (s_common_mmpool) {
		if (_tryto_desctroy(s_common_mmpool)) {
			s_common_mmpool = NULL;
		}
	}

	list_for_each_safe(pos,next,&__mem_root.inuser_list) {
		pool = list_entry(pos,struct nd_mm_pool, self_list ) ;
		list_del_init(&pool->self_list);
		_tryto_desctroy(pool);
		//nd_pool_destroy(pool,0) ;
	}

	nd_mutex_destroy(&__mem_root.lock) ;
	__mem_root.init = 0 ;

	__mem_root.is_destroied = 1;
}

static __INLINE__ void __adjust_freeaddr(nd_sub_allocator *sub_allocator)
{
	size_t free_size = sub_allocator->end - sub_allocator->start ;
	if (free_size){
		struct alloc_node *free_addr = (struct alloc_node *) sub_allocator->start ;
		free_addr->size = free_size | 1;
	}
}

int __sub_allocator_init(nd_sub_allocator *pool, size_t size, nd_mmpool_t *parent)
{
	pool->size = (NDUINT32)size  ;
	pool->myerrno = 0 ;
	pool->type = NDHANDLE_SUB_ALLOCATOR ;
	pool->allocated_size = 0 ;
	pool->start = (char*)(pool+1);
	pool->end = ((char*) pool )+ size;
	pool->parent = parent ;
	INIT_LIST_HEAD(&pool->self_list);

	memset(&pool->big_list, 0, sizeof(pool->big_list));
	memset(&pool->free_littles, 0, sizeof(pool->free_littles));

	list_add(&pool->self_list, &parent->allocator_list) ;
	__adjust_freeaddr(pool) ;
	return 0;
}

static nd_sub_allocator* __alloc_sub_allocator(nd_mmpool_t *pool,size_t size)
{
	size_t alloc_size = 0;
	nd_sub_allocator *p = 0 ;
	if (list_empty(&pool->free_allocator) ) {
		alloc_size = pool->alloc_granularity ;
		alloc_size *= MIN_PAGE_SIZE ;
		alloc_size = max(alloc_size, size) ;
RE_ALLOC:
		p = __sys_page_alloc(alloc_size) ;
		if (!p) {
			alloc_size =alloc_size>>1 ;
			if (alloc_size > size)	{
				goto RE_ALLOC;
			}
			return 0 ;
		}
	}
	else {
		struct list_head *pos = pool->free_allocator.next ;
		list_del_init(pos) ;
		p = list_entry(pos, struct mm_sub_allocator ,self_list );
		--pool->free_allocator_num ;
		alloc_size = p->size ;
	}
	__sub_allocator_init( p,  alloc_size,pool) ;
	return p;
}

static void __free_sub_allocator(nd_mmpool_t *pool, nd_sub_allocator* sub_allocator)
{
	list_del_init(&sub_allocator->self_list);
	if (pool->free_allocator_num >=2){
		__sys_page_free(sub_allocator) ;
	}
	else {
		list_add(&sub_allocator->self_list,&pool->free_allocator) ;
		++(pool->free_allocator_num) ;
	}
}

static int __check_in_sub(nd_sub_allocator *sub,void *p)
{
	if (p > (void*) sub && (char*)p< (char*)(sub+1) + sub->size) {
		return 1 ;
	}
	return 0 ;
}

static __INLINE__ size_t _get_max_free_chunk(nd_sub_allocator *sub)
{
	return sub->end - sub->start ;
}

nd_handle nd_pool_create(size_t maxsize ,const char *name )
{
	unsigned int _gran = 1;
	allocheader_t size ;
	nd_handle pool ;

	if(	!__mem_root.init ){
		nd_mempool_root_init() ;
	}
	if(0==maxsize) {
		return NULL ;
	}
	else if(-1==maxsize) {
		size = DEFAULT_PAGE_SIZE * 64 ;
		_gran = 64 ;
	}
	else if(maxsize<=DEFAULT_PAGE_SIZE) {
		size = DEFAULT_PAGE_SIZE ;
		maxsize = DEFAULT_PAGE_SIZE ;
		_gran =1 ;
	}
	else if (maxsize<=EMEMPOOL_NORMAL){
		size = DEFAULT_PAGE_SIZE ;
		maxsize = DEFAULT_PAGE_SIZE ;
	}
	else {
		size = EMEMPOOL_HUGE ;
		maxsize = ROUND_PAGE_SIZE(maxsize) ;
	}

	pool = (nd_handle)__sys_page_alloc(size) ;
	if(!pool) {
		nd_logerror("create mempool error on __sys_page_alloc(): %s\n", nd_last_error()) ;
		return NULL ;
	}

	pool->type = NDHANDLE_MMPOOL;
	pool->size = (NDUINT32) size ;
	pool->capacity = (allocheader_t)maxsize ;
	pool->allocated_size = size ;
	pool->myerrno = NDERR_SUCCESS ;
	pool->destruct_func = NULL;
	pool->alloc_granularity = _gran ;
	pool->free_allocator_num = 0;
//#ifdef ND_MEM_CHECK
	//pool->trace_mem =1 ;					
	pool->in_used = 0 ;
//#endif
	nd_object_set_instname(pool,name? name: "unknow_pool") ;

	nd_mutex_init(&pool->lock) ;
	INIT_LIST_HEAD(&pool->self_list);
	INIT_LIST_HEAD(&pool->page_list) ;
	INIT_LIST_HEAD(&pool->allocator_list);
	INIT_LIST_HEAD(&pool->free_allocator);

	pool->close_entry = (nd_close_callback )nd_pool_destroy ;

	__sub_allocator_init(pool->_allocator, size-sizeof(*pool),pool);
	pool->cur_allocator = pool->_allocator;

	//add memory allocator root
	nd_mutex_lock(&__mem_root.lock) ;
		list_add(&pool->self_list, &__mem_root.inuser_list) ;
	nd_mutex_unlock(&__mem_root.lock) ;
	return pool ;
}

int _tryto_desctroy(nd_mmpool_t *pool)
{
	if (pool->permament == 0) {
		return nd_pool_destroy(pool, 0);
	}
	return -1;
}
int nd_pool_destroy(nd_mmpool_t *pool, int flag)
{
	allocheader_t size ;
	struct alloc_node *node ;
	if(!pool)
		return -1;
	nd_pool_reset(pool) ;

	nd_mutex_lock(&__mem_root.lock) ;
		list_del_init(&pool->self_list) ;
	nd_mutex_unlock(&__mem_root.lock) ;

	nd_mutex_destroy(&pool->lock);
	size = pool->size ;
	node = (struct alloc_node *) pool  ;
	node->size = size ;

	__sys_page_free(pool) ;
	
	return 0 ;
}

void* nd_pool_realloc(nd_mmpool_t *pool ,void *oldaddr, size_t newsize)
{
	size_t user_len =0, alloc_len = 0, size ;
	struct alloc_node *allocaddr ;

	if(! ND_ALLOC_MM_VALID (oldaddr) ) {
		nd_logerror("pool realloc error : old addr(%x) is not valid ", oldaddr) ;
		return NULL;
	}
	allocaddr = _user_addr_2sys( oldaddr, &user_len, &alloc_len) ;
	size = __get_need_size(newsize+ (alloc_len - user_len)) ;

	if (alloc_len >= size) {
		return oldaddr ;
	}
	else {
		void *newaddr = nd_pool_alloc(pool, newsize) ;
		if (newaddr){
			memcpy(newaddr,oldaddr, user_len) ;
			nd_pool_free(pool,oldaddr) ;
			return newaddr ;
		}
	}
	return NULL ;
}

void nd_pool_reset(nd_mmpool_t *pool)
{
	struct list_head *pos, *list_next ;
	ND_LIST_HEAD(sub_list) ;
	ND_LIST_HEAD(page) ;
	//struct page_node *chunk= 0,*next =0 ;
	if(!pool)
		return ;

#ifdef ND_MEM_STATICS
	remove_statics(pool) ;
#endif

	nd_mutex_lock(&pool->lock);

#ifdef ND_MEM_CHECK
	if (/*pool->trace_mem &&*/pool->in_used){
		//dump memory leak
		dump_memleak(pool);
	}
#endif
	if (pool->destruct_func && pool->in_used){
		safe_destruct(pool);
	}

	pool->allocated_size = pool->size ;
	list_del_init(&pool->_allocator[0].self_list) ;

	list_add(&sub_list,&pool->allocator_list) ;
	list_del_init(&pool->allocator_list) ;

	list_add(&page,&pool->page_list) ;
	list_del_init(&pool->page_list) ;

	__sub_allocator_init(pool->_allocator, pool->size - sizeof(*pool),pool);
	pool->cur_allocator = pool->_allocator;

	nd_mutex_unlock(&pool->lock);

	list_for_each_safe(pos,list_next, &sub_list) {
		struct mm_sub_allocator *chunk = list_entry(pos,struct mm_sub_allocator, self_list) ;
		if (chunk == pool->_allocator )	{
			continue;
		}
		__sys_page_free(chunk) ;
	}

	list_for_each_safe(pos,list_next, &page) {
		struct big_chunk_list *chunk = list_entry(pos,struct big_chunk_list, list) ;
		struct alloc_node *addr = (struct alloc_node *)chunk;
		
		size_t size = chunk->size & ~3;
		size = size + sizeof(struct big_chunk_list) - sizeof(allocheader_t) ;
		
		addr->size = size ;
		
		__sys_page_free(addr) ;
	}
}


static void pool_add_free(nd_sub_allocator *pool , struct free_node *insert_node)
{
	size_t index ;
	size_t size =insert_node->size & ~3 ;
	// find from free list
	insert_node->size |= 1 ;

	if ( size >= BIG_SIZE) {
		size = BIG_ROUND_LOWWER(size) ;

		index = BIG_INDEX(size) ;
		nd_assert(index < BIG_CHUNK_NUM) ;
		insert_node->next = pool->big_list[index] ;
		pool->big_list[index] = insert_node;
	}
	else {
		size = LITTLE_ROUND_LOWWER(size) ;

		index = CHUNK_INDEX(size) ;
		insert_node->next = pool->free_littles[index] ;
		pool->free_littles[index] = insert_node;
	}
}

static void add_freeto_list(nd_sub_allocator *pool)
{
	size_t free_size = pool->end - pool->start ;
	if (free_size > BIG_SIZE){
		size_t size = BIG_ROUND_LOWWER(free_size) ;
		struct free_node *tmp_chunk = (struct free_node *)pool->start ;
		tmp_chunk->size = size ;
		pool_add_free(pool, tmp_chunk) ;
		pool->start += size;
		free_size -= size ;
	}
	if(free_size>=MIN_SIZE) {
		struct free_node *tmp_chunk = (struct free_node *)pool->start ;
		tmp_chunk->size = free_size ;
		pool_add_free(pool, tmp_chunk) ;
		//pool->start = pool->end;
	}
	pool->start = pool->end;
}


static struct free_node *_find_free_chunk(nd_sub_allocator *pool , size_t min_size)
{
	int i;
	size_t chunk_size ;
	struct free_node  **chunk_root=0;
	// find from free list

	for( i=BIG_CHUNK_NUM-1; i>=0; i--) {
		chunk_size = GET_BIG_SIZE(i) ;
		if (chunk_size < min_size) {
			return NULL ;
		}
		if (pool->big_list[i]) {
			chunk_root  = &(pool->big_list[i]) ;
			break ;
		}
	}
	if (!chunk_root && min_size <BIG_SIZE){

		for( i=LITTLE_CHUNK_NUM-1; i>=0; i--) {
			chunk_size = GET_LITTLE_SIZE(i) ;
			if (chunk_size < min_size) {
				return NULL ;
			}
			if (pool->free_littles[i]) {
				chunk_root  = &(pool->free_littles[i]) ;
				break ;
			}
		}
	}
	if (chunk_root && *chunk_root){
		//find ok , alloc from freelist
		struct free_node *alloc_chunk = *chunk_root ;
		*chunk_root = alloc_chunk->next ;
		return alloc_chunk ;
	}
	return NULL ;
}

static void __alloc_addr_size2index(nd_sub_allocator *sub_allocator, struct alloc_node *alloc_addr)
{
	size_t index = ((size_t)sub_allocator) & (~((size_t) ALLOCATOR_PAGE_BITMASK));
	size_t size = alloc_addr->size ;
	if (size >= BIG_SIZE) {
		size = BIG_INDEX(size) ;
		size = size<<3 ;
		size |= 4 ;
	}
	else {
		size = CHUNK_INDEX(size) ;
		size = size<<3 ;
	}
    nd_assert(0==(size & (~((size_t) ALLOCATOR_PAGE_BITMASK)) ));
	alloc_addr->size = index | (size & (size_t) ALLOCATOR_PAGE_BITMASK) ;

}

static nd_sub_allocator * __index2_alloc_size(struct alloc_node *alloc_addr)
{
	nd_sub_allocator *sub_allocator = (nd_sub_allocator *) (alloc_addr->size & (~((size_t) ALLOCATOR_PAGE_BITMASK)));
	size_t size = alloc_addr->size & (size_t) ALLOCATOR_PAGE_BITMASK;
	size = size >> 3 ;

	if (alloc_addr->size & 4){
		size = GET_BIG_SIZE(size) ;
	}
	else {
		size = GET_LITTLE_SIZE(size) ;
	}
	alloc_addr->size = size;

	if (sub_allocator->type ==NDHANDLE_MMPOOL){
		nd_mmpool_t *pool = (nd_mmpool_t *)sub_allocator ;
		sub_allocator = pool->_allocator;
	}
	nd_assert(sub_allocator->type==NDHANDLE_SUB_ALLOCATOR ) ;
	return sub_allocator ;
}

size_t __get_real_size(struct alloc_node *alloc_addr)
{
	size_t ret = 0;
	struct alloc_node tmp = *alloc_addr ;
	__index2_alloc_size(alloc_addr) ;
	ret = alloc_addr->size ;
	*alloc_addr = tmp ;
	return ret ;
}

size_t __get_need_size(size_t user_size)
{
	user_size += sizeof(struct alloc_node) ;
	if (user_size > DIRECT_ALLOC_SIZE ){
		return PAGE_ALINE(user_size) ;
	}

	user_size = SIZE_ALINE(user_size) ;

	// find from free list
	if (user_size >= BIG_SIZE) {
		return BIG_ALINE(user_size) ;
	}
	return user_size ;
}


static struct alloc_node *__allocator_alloc(nd_sub_allocator *sub_allocator, size_t alloc_size)
{
	size_t index  ;
	size_t  free_size,size;
	struct alloc_node *alloc_addr;
	struct free_node  **chunk_root=0;
	nd_sub_allocator *pool = sub_allocator ;

	alloc_size += sizeof(struct alloc_node) ;
	size = SIZE_ALINE(alloc_size) ;

	// find from free list
	if (size >= BIG_SIZE) {
		size = BIG_ALINE(alloc_size) ;
		index =  BIG_INDEX(size) ;
		nd_assert(index < BIG_CHUNK_NUM) ;
		chunk_root = &(pool->big_list[index] );
	}
	else {
		index = CHUNK_INDEX(size) ;
		chunk_root = &(pool->free_littles[index]) ;
	}

	if (chunk_root && *chunk_root){
		//find ok , alloc from freelist
		struct free_node *alloc_chunk = *chunk_root ;
		*chunk_root = alloc_chunk->next ;

		alloc_addr = (struct alloc_node *)alloc_chunk ;
		alloc_addr->size &= ~1;
		nd_assert(alloc_addr->size);
		//return (void*)(alloc_addr->data) ;
		return alloc_addr ;
	}

	free_size = pool->end - pool->start ;
	if(free_size < size){			//left memory not enough

		struct free_node *chunk = _find_free_chunk(pool,size) ;
		if(chunk) {
			nd_assert(chunk->size>=size+1) ;

			if(free_size >= MIN_SIZE){
				add_freeto_list(pool) ;
			}
			chunk->size = (chunk->size & ~3) ;
			pool->start = (char*)chunk ;
			pool->end = chunk->size + pool->start ;
		}
		else {
			return NULL;
		}
	}

	alloc_addr = (struct alloc_node *)pool->start ;
	alloc_addr->size = size ;
	pool->start += size ;

	free_size = pool->end - pool->start ;
// 	if(free_size<MIN_SIZE) {
// 		alloc_addr->size += free_size ; 
// 		pool->start = pool->end;
// 	}
	nd_assert(alloc_addr->size >= size) ;
	//return (void*)(alloc_addr->data) ;
	return alloc_addr ;
}

static void __allocator_free(nd_sub_allocator *pool , struct free_node *chunk)
{
	allocheader_t free_size ;

	size_t size = chunk->size & ~(ALIGN_SIZE-1) ;

	char *p = ((char*)chunk) + size;
	nd_assert(chunk->size > 0) ;

	if (p == pool->start){
		pool->start = (char *) chunk ;
		return ;
	}
	free_size = pool->end - pool->start ;

	if(free_size==0) {
		pool->start = (char*)chunk ;
		pool->end = (char*)chunk + size ;
		return ;
	}
	else if(chunk->size > (free_size +3) ) {
		add_freeto_list(pool);

		pool->start = (char*)chunk ;
		pool->end = (char*)chunk + size ;
	}
	else {
		pool_add_free(pool, chunk) ;
	}
}
void *nd_allocator_alloc(nd_sub_allocator *sub_allocator, size_t alloc_size)
{
	struct alloc_node *node = __allocator_alloc(sub_allocator, alloc_size) ;
	nd_assert(sub_allocator->start<= sub_allocator->end) ;
	if (node) {
		sub_allocator->allocated_size += (NDUINT32) node->size ;
		__alloc_addr_size2index(sub_allocator, node)  ;
		__adjust_freeaddr(sub_allocator);
		return (void*)(node->data );
	}
	return NULL;
}

int nd_allocator_free(nd_sub_allocator *sub_allocator, struct alloc_node*free_addr)
{
	sub_allocator->allocated_size -= free_addr->size & ~3 ;
	__allocator_free(sub_allocator, (struct free_node*)free_addr) ;
	__adjust_freeaddr(sub_allocator) ;
	nd_assert(sub_allocator->start<= sub_allocator->end) ;
	if (sub_allocator->allocated_size ==0){
		return 0;
	}
	return 1;
}

void *_pool_alloc_real(nd_mmpool_t *pool , size_t size)
{
	void *addr =0;
	nd_assert(pool) ;
	nd_assert(size>0) ;
	if (!ND_ALLOC_MM_VALID(pool)){
		nd_logerror("pool alloc error: input pool is invalid (%x)\n", pool) ;
		return NULL;
	}

	pool->myerrno = NDERR_SUCCESS ;
	if (size==0){
		pool->myerrno = NDERR_INVALID_INPUT ;
		nd_logerror("pool alloc error: malloc size is zero\n") ;
		return 0;
	}
	else if (pool->allocated_size > pool->capacity){
		pool->myerrno = NDERR_NOSOURCE ;
		nd_logerror("pool alloc error:  malloc size is too much \n") ;
		return 0;
	}

	if ((size + sizeof(struct alloc_node) ) > DIRECT_ALLOC_SIZE ){
		//directly alloc from system for big-request
		size_t alloc_size ;
		struct big_chunk_list *alloc_addr ;
		struct alloc_node *new_chunk = __sys_page_alloc(size + sizeof(struct big_chunk_list ) ) ;
		if(!new_chunk){
			pool->myerrno = NDERR_NOSOURCE ;
			nd_logerror("pool alloc error: no memory \n") ;
			return 0;
		}
		alloc_size = new_chunk->size ;
		alloc_addr = (struct big_chunk_list *)new_chunk ;

		alloc_addr->size = alloc_size - sizeof(struct big_chunk_list) + sizeof(allocheader_t) ;
		alloc_addr->pool = pool;
		INIT_LIST_HEAD(&alloc_addr->list);
		

		nd_mutex_lock(&pool->lock);
			pool->allocated_size += alloc_addr->size ;
			alloc_addr->size |= 2 ;
			addr = alloc_addr->data ;
			list_add(&alloc_addr->list, &pool->page_list) ;
			pool->in_used = 1;
		nd_mutex_unlock(&pool->lock);
	}
	else {
		nd_mutex_lock(&pool->lock);
		if (pool->cur_allocator){
			addr = nd_allocator_alloc(pool->cur_allocator,size) ;
		}
		else {
			struct list_head *pos,*list_next ;
			list_for_each_safe(pos,list_next, &pool->allocator_list) {
				struct mm_sub_allocator *chunk = list_entry(pos,struct mm_sub_allocator, self_list) ;
				if (chunk == pool->cur_allocator){
					continue;
				}
				addr = nd_allocator_alloc(chunk,size) ;
				if (addr){
					pool->cur_allocator = chunk;
					break ;
				}
			}
		}

		if (!addr){
			struct mm_sub_allocator *sub = __alloc_sub_allocator(pool,size) ;
			if (sub){
				addr = nd_allocator_alloc(sub,size) ;
				if (_get_max_free_chunk(sub) > _get_max_free_chunk(pool->cur_allocator) ){
					pool->cur_allocator = sub;
				}
			}
		}
		pool->in_used = 1;
		nd_mutex_unlock(&pool->lock);
	}

	//TRYTO_MMSTATIC(pool, addr, size) ;

	return addr ;
}

void _pool_free_real(/*nd_mmpool_t *pool ,*/void *addr)
{
	nd_mmpool_t *pool = NULL;
	struct alloc_node*free_addr ;
	nd_assert(addr) ;
	if (!ND_ALLOC_MM_VALID(addr)){
		return ;
	}

	//free_addr = ((struct alloc_node *)addr ) -1 ;
	free_addr = __user_addr_2allocated(addr);

	if (free_addr->size & 2){
		size_t size = free_addr->size & ~3;
		struct big_chunk_list *chunk = (struct big_chunk_list *)addr ;
		--chunk ;

		pool = chunk->pool ;
		if(!ND_ALLOC_MM_VALID(pool) ) {
			nd_logdebug("error in pool_free(): invalid input %d\n" AND addr) ;
			return ;
		}

		nd_atomic_sub((ndatomic_t*)&pool->allocated_size,(ndatomic_t)(size)) ;

		nd_mutex_lock(&pool->lock);
			list_del_init(&chunk->list) ;
		nd_mutex_unlock(&pool->lock);

		free_addr = (struct alloc_node *)chunk;
		free_addr->size = size + sizeof(struct big_chunk_list) - sizeof(allocheader_t) ;
		__sys_page_free((void*)free_addr) ;
	}
	else {
		nd_sub_allocator *sub_alloc = __index2_alloc_size(free_addr) ;
		nd_assert(ND_ALLOC_MM_VALID(sub_alloc)) ;
		nd_assert(__check_in_sub(sub_alloc,addr)) ;
		if (!ND_ALLOC_MM_VALID(sub_alloc)|| !__check_in_sub(sub_alloc,addr)) {
			return ;
		}
		pool = sub_alloc->parent ;
		nd_mutex_lock(&pool->lock);
		if( 0==nd_allocator_free(sub_alloc,free_addr) ) {
			if (sub_alloc!=pool->_allocator && sub_alloc!=pool->cur_allocator)	{
				__free_sub_allocator(pool, sub_alloc) ;
			}
		}
		else {
			if (_get_max_free_chunk(sub_alloc) > _get_max_free_chunk(pool->cur_allocator) ){
				pool->cur_allocator = sub_alloc;
			}
		}
		nd_mutex_unlock(&pool->lock);
	}
}

void *nd_pool_alloc_real(nd_mmpool_t *pool , size_t size)
{
	void *ret = _pool_alloc_real(pool ,  size) ;
	TRYTO_MMSTATIC(pool, ret, size) ;
	return ret ;
}
void nd_pool_free_real(nd_mmpool_t *pool ,void *addr)
{
	_pool_free_real(addr);
	ERASE_MMSTATICS(addr) ;
}

//////////////////////////////////////////////////////////////////////////
//border test
#ifdef ND_MEM_CHECK
struct __alloc_header {
	NDUINT32 __magic: 16;
	NDUINT32 __type_size:16;
	NDUINT32 _M_size;
	NDUINT32 line;
	char file[40] ;
}; // that is 8 bytes for sure
// Sunpro CC has bug on enums, so extra_before/after set explicitly
enum { __pad=8, __magic=0xdeba, __deleted_magic = 0xdebd,
	__shred_byte= 0xb8, __release_byte = 0xc5
};
enum { __extra_before = 64, __extra_after = 8 };

// nd_alloc_check nd_free_check  check allocfn return addr is overflow
void *nd_alloc_check(nd_handle _pool,size_t __n,const char *file, int line, nd_alloc_func allocfn)
{
	size_t filesize ;
	char *pfile ;
	size_t __real_n = __n + __extra_before + __extra_after;
	struct __alloc_header *__result =
		(struct __alloc_header *)allocfn(_pool,__real_n);
	if(__result) {
		memset((char*)__result, __shred_byte, __real_n*sizeof(char));
		__result->__magic = __magic;
		__result->__type_size = sizeof(char);
		__result->_M_size = (NDUINT32)__n;
		__result->line = line ;

		pfile = (char*)file ;
		filesize = ndstrlen(file) ;
		if(filesize > 40) {
			pfile += filesize - 40 ;
		}
		ndstrncpy(__result->file,pfile,40) ;

		return ((char*)__result) + (long)__extra_before;
	}
	else {
		return NULL ;
	}
}

// _destroy_chunkpool for debug
void nd_free_check(nd_handle _pool,void *__p, nd_free_func freefn)
{
	unsigned char* __tmp;
	struct __alloc_header * __real_p ;
	size_t __real_n ;
	if(__p==NULL) {
		nd_assert(0) ;
		return ;
	}
	
	if(	__mem_root.init ==0 )
		return  ;
	
	__real_p = (struct __alloc_header*)((char *)__p -(long)__extra_before);

	__real_n= __real_p->_M_size + __extra_before + __extra_after;
	// check integrity
	nd_assert(__real_p->__magic != __deleted_magic) ;
	nd_assert(__real_p->__magic == __magic);
	nd_assert(__real_p->__type_size == 1);
	//nd_assert(__real_p->_M_size == __n);
	// check pads on both sides
	for (__tmp= (unsigned char*)(__real_p+1); __tmp < (unsigned char*)__p; __tmp++) {
		nd_assert(*__tmp==__shred_byte) ;
	}


	for (__tmp= ((unsigned char*)__p)+__real_p->_M_size*sizeof(char*);
	__tmp < ((unsigned char*)__real_p)+__real_n ; __tmp++) {
		nd_assert(*__tmp==__shred_byte) ;
	}

	// that may be unfortunate, just in case
	__real_p->__magic=__deleted_magic;
	//memset((char*)__p, __release_byte, __real_p->_M_size*sizeof(char));

    memset((char*)__real_p,__release_byte, __real_n) ;
	freefn(_pool, __real_p);
}
/*
//alloc with trace
void *nd_pool_alloc_trace(nd_mmpool_t *pool , size_t size, char *file, int line)
{
	char *paddr ;
	int len ;
	NDUINT32 file_size ;

	len = (int) ndstrlen(file) ;
	if (len >255){
		len = 255 ;
	}
	file_size = (len + 3) & (~3) ;


	paddr = nd_pool_alloc_real( pool ,  size + file_size + 12 );
	if (!paddr)	{
		return NULL ;
	}

	*((NDUINT32*)paddr) = (NDUINT32)line ;
	paddr += sizeof(NDUINT32) ;

	*((NDUINT32*)paddr) = file_size ;
	paddr += sizeof(NDUINT32) ;
	if (len < file_size)++len;

	memcpy(paddr, file,len) ;

	paddr += file_size;
	*((NDUINT32*)paddr) = file_size ;
	return (void*)(paddr + sizeof(NDUINT32)) ;
}


void nd_pool_free_trace(nd_mmpool_t *pool , void *p)
{
	char *paddr ;
	NDUINT32 file_size ;

// 	if (!pool->trace_mem){
// 		nd_pool_free_real(pool, p) ;
// 		return ;
// 	}
	paddr =(char*)p - sizeof(NDUINT32) ;
	file_size = *(NDUINT32*)paddr ;
	nd_assert(file_size<=ND_FILE_PATH_SIZE) ;
	paddr -= file_size + sizeof(NDUINT32)  ;

	nd_assert(file_size == *(NDUINT32*)paddr ) ;
	paddr -= sizeof(NDUINT32)  ;

	nd_pool_free_real(pool,paddr) ;
}
*/
//get the user address from a memory 
void *_get_user_addr(nd_mmpool_t *pool, void *p)
{
	//NDUINT32 fill_size ;
	char *user_addr = (char*) p;

// 	user_addr = (char*) p  + sizeof(NDUINT32);
// 	fill_size = *(NDUINT32*)user_addr ;
// 	user_addr +=  fill_size + 8 ;
	return user_addr + __extra_before;
}

//from user address fetch real-alloced address
struct alloc_node *_user_addr_2sys( void *useraddr, size_t *user_len, size_t *alloc_len)
{
	struct alloc_node *ret = (struct alloc_node *)((char*)useraddr-__extra_before);
	--ret ;

	if (ret->size & 2){
		*alloc_len = (ret->size & ~3) ;
		ret = (struct alloc_node *) ((char*)ret - sizeof(struct big_chunk_list) + sizeof(allocheader_t) );

	}
	else {
		*alloc_len =  __get_real_size(ret) ;
	}
	*user_len = *alloc_len - sizeof(struct alloc_node) - __extra_before - __extra_after;
	return ret ;
	/*struct alloc_node *ret ;

	NDUINT32 *p = (NDUINT32*) ((char*)useraddr - __extra_before) ;
	--p ;
	p = (NDUINT32*) ((char*)p -  *p) ;//file test
	--p ; //file size ;
	--p ; //line
	ret = (struct alloc_node *) p ;

	--ret ;

	if (ret->size & 2){
		*alloc_len = ret->size & ~3 ;
		*user_len = *alloc_len - ((char*)useraddr - (char*)ret) - __extra_after;
		ret = (struct alloc_node *) ((char*)ret - sizeof(struct big_chunk_list) + sizeof(allocheader_t) );
	}
	else {
		*alloc_len =  __get_real_size(ret);
		*user_len = *alloc_len - ((char*)useraddr - (char*)ret) - __extra_after;
	}
	return ret ;
	*/
}

void _dump_trunk_leak(nd_mmpool_t *pool, void *startaddr, size_t size)
{

	struct __alloc_header * p ;
// 	size_t __real_n ;
// 	if(startaddr==NULL) {
// 		nd_assert(0) ;
// 		return ;
// 	}
	p = (struct __alloc_header*)startaddr ;
#ifdef ND_SOURCE_TRACE
	_logmsg(__FUNC__,p->file, p->line, ND_ERROR,"%s MEMORY LEAK size=%d\n", pool->inst_name[0]?pool->inst_name:"unknow_pool" AND p->_M_size) ;
#endif
    
// 	char tmp ;
// 	NDUINT32 *p =  (NDUINT32*) startaddr ;
// 	NDUINT32 line =  *p++ ;
// 	NDUINT32 filename_l = *p++ ;
// 	char *file =(char*) p ;
//
// 	tmp = file[filename_l] ;
// 	file[filename_l] = 0 ;
// 	_logmsg(__FUNC__,file, (int)line, ND_ERROR,"%s MEMORY LEAK\n", pool->inst_name[0]?pool->inst_name:"unknow_pool") ;
// 	file[filename_l] = tmp ;
}

#else
void *_get_user_addr(nd_mmpool_t *pool, void *p)
{
	return p;
}

struct alloc_node *_user_addr_2sys( void *useraddr, size_t *user_len, size_t *alloc_len)
{
	struct alloc_node *ret = (struct alloc_node *)useraddr;
	--ret ;

	if (ret->size & 2){
		*alloc_len = (ret->size & ~3) ;
		ret = (struct alloc_node *) ((char*)ret - sizeof(struct big_chunk_list) + sizeof(allocheader_t) );

	}
	else {
		*alloc_len =  __get_real_size(ret) ;
	}
	*user_len = *alloc_len - sizeof(struct alloc_node) ;
	return ret ;
}

#endif
//
// size_t nd_pool_freespace(nd_mmpool_t *pool)
// {
// 	return pool->end - pool->start ;
// }


void nd_pool_destruct_entry(nd_mmpool_t *pool , memdestruct_entry func)
{
	pool->destruct_func = func ;
}

void nd_pool_set_trace(nd_handle pool, int flag)		//reset a memory pool
{
// #ifdef ND_MEM_CHECK
// 	if (pool->in_used==0)	{
// 		//pool->trace_mem = flag ? 1 : 0 ;
// 	}
// #endif
}

//ÄÚ´æÎö¹¹
void destruct_mm_entry(nd_mmpool_t *pool, void *startaddr, size_t size)
{
	pool->destruct_func(pool,_get_user_addr(pool, startaddr)) ;
}

//walk memory trunk
void _walk_trunk(void *start, void *end,nd_sub_allocator *pool, walk_trunk_node cb_func)
{
	struct alloc_node *header = (struct alloc_node *)start ;

	while(header < (struct alloc_node *)end) {
		allocheader_t size;
		if (header->size & 1){
			size = header->size & (~3) ;
		}
		else {
			size = __get_real_size(header);
		}

		if (!(header->size & 1)){
			cb_func(pool->parent,header + 1, size ) ;
		}
		header =(struct alloc_node *) ((char*)header + size) ;
	}
}

//walk memory
void _walk_alloced_inpool(nd_mmpool_t *pool, walk_trunk_node func )
{
	struct list_head *pos, *list_next ;
	void *pstart, *pend ;
	if (!pool->destruct_func){
		return  ;
	}

	list_for_each_safe(pos,list_next, &pool->allocator_list) {
		struct mm_sub_allocator *sub_allocator = list_entry(pos,struct mm_sub_allocator, self_list) ;
		pstart = (void*)(sub_allocator + 1) ;
		pend = (void*) (sub_allocator->size + (char*)sub_allocator );
		_walk_trunk(pstart, pend, sub_allocator, func) ;
	}

	list_for_each_safe(pos,list_next, &pool->page_list) {
		struct big_chunk_list *chunk = list_entry(pos,struct big_chunk_list, list) ;
		func(pool, chunk->data, chunk->size & ~3) ;
	}
	/*struct list_head *pos, *list_next ;
	void *pstart, *pend ;
	if (!pool->destruct_func){
		return  ;
	}
	pstart =(void*) (pool + 1) ;
	pend = (char*)pool + pool->size   ;

	_walk_trunk(pstart, pend, pool,func);

	if (pool->original_list){
		struct page_node *chunk,*next ;

		chunk = pool->original_list ;
		while (chunk){
			next = chunk->next ;
			pstart = (void*)(chunk +1) ;
			pend = (void*)((char*)chunk + chunk->size) ;
			_walk_trunk(pstart, pend, pool,func);
			chunk = next ;
		}
	}

	list_for_each_safe(pos,list_next, &pool->big_list_chunk) {
		struct big_chunk_list *big_chunk = list_entry(pos,struct big_chunk_list, list) ;
		func(pool, big_chunk->data, big_chunk->size & ~3) ;
	}
	*/
}

#ifdef ND_MEM_STATICS
#include "nd_common/nd_bintree.h"
struct mmstatics_header
{
	//struct list_head list ;
	struct nd_rb_node tree;
	size_t size ;
	void *mmaddr ;
	nd_handle pool ;
	char pool_name[32] ;
	char func_stack[1] ;//include '\0'
};

struct mmstatics_root
{
	//struct list_head header ;
	struct nd_rb_root tree_header ;
	nd_mutex lock ;
};

static ndatomic_t _s_mmstatic_created =0 ;
static struct mmstatics_root *_s_mmstatics ;
int nd_mm_statics_start(void)
{
	struct mmstatics_root *proot ;
	if (0!=nd_testandset(&_s_mmstatic_created)){
		return 0;
	}

	proot = (struct mmstatics_root*) __sys_alloc(sizeof(struct mmstatics_root)) ;
	if(!proot) {
		nd_atomic_set(&_s_mmstatic_created, 0) ;
		return -1 ;
	}
	//INIT_LIST_HEAD(&proot->header) ;
	proot->tree_header.rb_node = NULL ;
	nd_mutex_init(&proot->lock) ;
	_s_mmstatics = proot ;
	return 0;
}

int nd_mm_statics_end(void)
{
	struct mmstatics_root *proot ;

	if(!nd_compare_swap(&_s_mmstatic_created,1,0) ) {
		return 0 ;
	}

	if (!_s_mmstatics){
		return -1;
	}
	nd_mutex_lock(&_s_mmstatics->lock) ;

	proot = _s_mmstatics ;

	if (proot->tree_header.rb_node){
		struct nd_rb_node *cur_node = rb_first(&proot->tree_header) ;
		while ( cur_node) {
			struct mmstatics_header *node  = rb_entry(cur_node, struct mmstatics_header, tree) ;
			cur_node = rb_next(cur_node) ;
			nd_assert(node) ;
			nd_logwarn("memleak addr=%d size=%d in %s pool | stack %s\n" AND node->mmaddr AND node->size
				AND node->pool_name AND node->func_stack) ;
			rb_erase(&node->tree,&proot->tree_header ) ;
			__sys_free(node) ;
		}
		proot->tree_header.rb_node = 0;
	}
	_s_mmstatics = 0 ;
	nd_mutex_unlock(&proot->lock) ;
	nd_mutex_destroy(&proot->lock) ;

	__sys_free(proot) ;
	return 0;
}

void _insertinto_mmstatics(nd_handle pool, void *addr, size_t size )
{
	int len ;
	struct mmstatics_header *node ;
	char callstack[1024] ;
	if(nd_atomic_read(&_s_mmstatic_created)==0 || !_s_mmstatics ) {
		return ;
	}

	if(nd_get_callstack_desc(callstack,sizeof(callstack)) ) {
		len = (int) ndstrlen(callstack) + 1;
	}
	else {
		ndstrncpy(callstack, "unknow_stack", 1024) ;
		len = 13;
	}

	node = __sys_alloc(sizeof(struct mmstatics_header) + len) ;
	if(!node) {
		nd_logerror("__sys_alloc error \n") ;
		return ;
	}

	//INIT_LIST_HEAD(&node->list) ;
	rb_init_node(&node->tree) ;
	node->size = size ;
	node->mmaddr = addr ;
	node->pool = pool ;
#ifdef ND_SOURCE_TRACE
	ndstrncpy(node->pool_name, pool->inst_name, sizeof(node->pool_name)) ;
#else
	ndstrncpy(node->pool_name, "mmpool", sizeof(node->pool_name)) ;
#endif
	ndstrncpy(node->func_stack, callstack, len) ;

	nd_mutex_lock(&_s_mmstatics->lock) ;
	{
		struct nd_rb_root *root = &_s_mmstatics->tree_header;
		struct nd_rb_node **new_node = &(root->rb_node), *parent = NULL;

		while (*new_node) {
			struct mmstatics_header *cur_node = rb_entry(*new_node,struct mmstatics_header, tree);
			parent = *new_node;
			if (addr < cur_node->mmaddr )
				new_node = &((*new_node)->rb_left);
			else if (addr > cur_node->mmaddr )
				new_node = &((*new_node)->rb_right);
			else {
				nd_assert(0=="memory address already existed");
				nd_mutex_unlock(&_s_mmstatics->lock) ;
				__sys_free(node) ;
				return ;
			}
		}
		rb_link_node(&node->tree, parent, new_node);
		rb_insert_color(&node->tree, root);

	}
	nd_mutex_unlock(&_s_mmstatics->lock) ;
}

static struct mmstatics_header *find_mmstatics(void *key)
{
	struct nd_rb_node *node = _s_mmstatics->tree_header.rb_node;
	while (node) {
		struct mmstatics_header *data =  rb_entry(node, struct mmstatics_header, tree) ;
		if (key < data->mmaddr)
			node = node->rb_left;
		else if (key > data->mmaddr)
			node = node->rb_right;
		else {
			return data;
		}
	}
	return NULL ;
}

void _erase_mmstatics(void *addr)
{
	//struct list_head *pos,*next ;
	struct mmstatics_header *node ;

	if(nd_atomic_read(&_s_mmstatic_created)==0 || !_s_mmstatics) {
		return ;
	}
	nd_mutex_lock(&_s_mmstatics->lock) ;
	node = find_mmstatics(addr) ;
	if (node) {
		rb_erase(&node->tree,&_s_mmstatics->tree_header ) ;
		__sys_free(node) ;
	}
// 	list_for_each_safe(pos, next, &_s_mmstatics->header){
// 		node = list_entry(pos,struct mmstatics_header ,list) ;
// 		if (node->mmaddr == addr){
// 			list_del_init(&node->list) ;
// 			__sys_free(node) ;
// 			break;
// 		}
// 	}
	nd_mutex_unlock(&_s_mmstatics->lock) ;
}
void _erase_statics_entry(nd_mmpool_t *pool, void *startaddr, size_t size)
{
	_erase_mmstatics(startaddr) ;
}
void remove_statics(nd_mmpool_t *pool)
{
	if(nd_atomic_read(&_s_mmstatic_created)==0 || !_s_mmstatics) {
		return ;
	}
	_walk_alloced_inpool(pool,_erase_statics_entry) ;
}

#endif

#if  defined(ND_SOURCE_TRACE) && defined(ND_UNUSE_STDC_ALLOC)
//dump all memory used info in memory-pool-system
void nd_mmpool_dump(void)
{
	struct list_head *pos ;
	nd_mutex_lock(&__mem_root.lock) ;
	list_for_each(pos,&__mem_root.inuser_list) {
		nd_mmpool_t *pool = list_entry(pos,struct nd_mm_pool, self_list) ;
		nd_logmsg("pool %s alloc size =%x \n" AND pool->inst_name AND pool->allocated_size) ;
	}
	nd_mutex_unlock(&__mem_root.lock) ;
}
#endif


#else //do not use memory pool system

typedef struct nd_mm_pool *nd_handle ;
#include "nd_common/nd_handle.h"
#include "nd_common/nd_mempool.h"
 struct nd_mm_pool
{
	ND_OBJ_BASE ;
	memdestruct_entry destruct_func ;					//
	struct list_head used_list;
	nd_mutex lock ;
};

nd_handle __pool_create(size_t size,const char *name,struct nd_mm_pool *pool )
{
	memset(pool, 0, sizeof(*pool) ) ;
	pool->type = NDHANDLE_MMPOOL;
	pool->size = (NDUINT32) size ;
	pool->myerrno = 0 ;
	pool->__created = 1 ;
	pool->destruct_func = 0 ;
	INIT_LIST_HEAD(&pool->used_list) ;

	nd_mutex_init(&pool->lock );
	return (nd_handle)pool ;
}

static struct nd_mm_pool  _static_pool  ;
nd_handle nd_global_mmpool(void)
{
	if (!_static_pool.__created){
		__pool_create(-1,"global_mmpool",&_static_pool ) ;
	}
	return (nd_handle) &_static_pool ;
}

nd_handle nd_pool_create(size_t size,const char *name )
{
	struct nd_mm_pool *pool = malloc(sizeof(struct nd_mm_pool));
	if (pool)	{
		__pool_create(size,name, pool) ;
	}
	return pool;
}

int nd_pool_destroy(nd_handle pool, int flag)
{
	if (!ND_ALLOC_MM_VALID(pool) ){
		return -1;
	}
	nd_pool_reset(pool) ;
	nd_mutex_destroy(&pool->lock );
	free(pool) ;
	return 0;
}
void *nd_pool_alloc_real(nd_handle pool , size_t size)
{
	if (!ND_ALLOC_MM_VALID(pool) ){
		return NULL;
	}

// 	if (!pool->destruct_func){
// 		return malloc(size) ;
// 	}
// 	else
	{
		struct list_head* header = malloc(size+sizeof(struct list_head)) ;
		if(header) {
			INIT_LIST_HEAD(header);
			nd_mutex_lock(&pool->lock) ;
				list_add(header,&pool->used_list) ;
			nd_mutex_unlock(&pool->lock) ;
			return (void*) (header + 1 );
		}
		return NULL ;
	}
}
void nd_pool_free_real(nd_handle pool ,void *addr)
{
	if (!ND_ALLOC_MM_VALID(pool) ){
		return ;
	}
	if (!ND_ALLOC_MM_VALID(addr) ){
		return ;
	}
// 	if (!pool->destruct_func){
// 		free(addr) ;
// 	}
// 	else
	{
		struct list_head* header = ((struct list_head* )addr ) - 1;
		if (!ND_ALLOC_MM_VALID(header) || !ND_ALLOC_MM_VALID(header->next) || !ND_ALLOC_MM_VALID(header->prev) ){
			return ;
		}

		nd_mutex_lock(&pool->lock) ;
		list_del_init(header) ;
		nd_mutex_unlock(&pool->lock) ;

		free(header) ;
	}
}

void nd_pool_reset(nd_handle pool)
{
	struct list_head *pos, *next ;
	//if (pool->destruct_func)
	{
		nd_mutex_lock(&pool->lock );
		list_for_each_safe(pos, next , &pool->used_list) {
			list_del_init(pos) ;
			if(pool->destruct_func)
				pool->destruct_func(pool, pos+1) ;
			free(pos) ;
		}
		INIT_LIST_HEAD(&pool->used_list) ;
		nd_mutex_unlock(&pool->lock );

		pool->destruct_func = 0;
	}
}
void nd_pool_destruct_entry(nd_handle pool , memdestruct_entry func)
{
	nd_assert(pool) ;
// 	if (pool->destruct_func && !func){
// 		nd_mutex_destroy(&pool->lock );
// 	}
// 	else
// 	if(!pool->destruct_func) {
// 		nd_mutex_init(&pool->lock );
// 	}
	pool->destruct_func = func ;

}

#if  defined(ND_SOURCE_TRACE) && defined(ND_UNUSE_STDC_ALLOC)
void nd_mmpool_dump(void)
{
}
#endif

#endif

#undef ND_IMPLEMENT_HANDLE


#ifdef _MSC_VER
int nd_addr_checkvalid(void *addr)
{
	if (addr <(void*) 0x10000)	{
		return 0 ;
	}
#ifdef X86_64
	else if (addr >(void*)0x7FFFFFFFFFF) {
		return 0 ;
	}
#else
	else if (addr >(void*)0x7FFFFFFF) {
		return 0 ;
	}
#endif
	return 1;
}

#elif defined(__ND_LINUX__) && !defined(__ND_ANDROID__)
int nd_addr_checkvalid(void *addr)
{
	if (addr <(void*) 0x10000)	{
		return 0;
	}
#ifdef X86_64
	else if (addr >(void*)0x7fffffffffff) {
		return 0;
	}
#else
	else if (addr >(void*)0xC0000000) {
		return 0;
	}
#endif
	return 1 ;
}
#else

inline int nd_addr_checkvalid(void *addr)
{
    return addr!=NULL;
}
#endif

int nd_alloc_checkvalid(void *addr)
{
	if(!nd_addr_checkvalid(addr))
		return 0 ;
	if ((size_t)addr & (size_t) 3){ //ALINE 4
		return 0 ;
	}
	return 1 ;
}
