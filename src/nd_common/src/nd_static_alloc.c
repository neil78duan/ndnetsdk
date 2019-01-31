/* file : nd_static_alloc.h
 *
 * static memory allocator
 *
 * create by neil duan, all right reserved 
 */

//#define ND_IMPLEMENT_HANDLE

#include "nd_common/nd_common.h"
#include "nd_common/nd_static_alloc.h"

/*
 * node manager-alloctor
 */
typedef struct nd_static_alloc_t
{
	ND_OBJ_BASE ;

	size_t node_size ;
	int capacity ;
	
	int allocated_num ;
	
	nd_handle mem_pool;
	
	nd_mutex  list_lock;
	struct list_head __free_list;

	char addr[0] ;

}nd_static_alloc_t;

#define GET_SA_ALLOC_MIN_SIZE(size) max(sizeof(struct list_head),size)

nd_sa_handle nd_sa_create(int node_num, size_t node_size, nd_handle mempool)
{
	int i ;
	size_t raw_len;
	char *addr ;
	nd_static_alloc_t *allocator ;
	nd_handle pool ;

	node_size = GET_SA_ALLOC_MIN_SIZE(node_size) ;
		
	raw_len = node_num * node_size + sizeof(nd_static_alloc_t);
	
	if(mempool) 
		pool =mempool ;
	else 
		pool = nd_global_mmpool() ;

	nd_assert(pool) ;
	
	allocator = (nd_static_alloc_t *)nd_pool_alloc(pool, raw_len ) ;

	if(!allocator) {
		return NULL;
	}

	allocator->size = (NDUINT32) raw_len;						
	allocator->type =NDHANDLE_STATICALLOCATOR;					
	allocator->close_entry =(nd_close_callback) nd_sa_destroy ;	
	allocator->myerrno = NDERR_SUCCESS ;
	allocator->capacity =node_num;	
	allocator->node_size = node_size ;
	allocator->allocated_num = 0 ;
	allocator->mem_pool = pool ;
	
	nd_mutex_init(&allocator->list_lock);
	INIT_LIST_HEAD(&allocator-> __free_list);

	addr = allocator->addr ;

	for (i=0; i<node_num; i++){
		struct list_head *list = (struct list_head *)addr ;
		
		INIT_LIST_HEAD(list);
		list_add(list,&(allocator->__free_list)) ;
		addr += node_size ;
	}
	return (nd_sa_handle)allocator ;
}


void* nd_sa_alloc(nd_sa_handle sa_handle) 
{
	struct list_head *pos;
	nd_static_alloc_t * allocator =(nd_static_alloc_t *) sa_handle ;

	allocator->myerrno = NDERR_SUCCESS;
	nd_mutex_lock(&allocator->list_lock);
	pos = allocator->__free_list.next ;

	if(pos== &allocator->__free_list){
		allocator->myerrno = NDERR_LIMITED ;
		nd_mutex_unlock(&allocator->list_lock);
		return NULL ;
	}

	list_del_init(pos) ;
	++(allocator->allocated_num );	
	nd_mutex_unlock(&allocator->list_lock);

	return (void*)pos;
}

void nd_sa_free(void* addr,nd_sa_handle sa_handle ) 
{

	nd_static_alloc_t * allocator =(nd_static_alloc_t * ) sa_handle ;
	struct list_head *head ;
	if(!addr || !sa_handle) {
		return ;
	}

	head = (struct list_head *) addr ;

	INIT_LIST_HEAD(head) ;

	nd_mutex_lock(&allocator->list_lock);
		list_add_tail(head,&(allocator->__free_list)) ;
		--(allocator->allocated_num );
	nd_mutex_unlock(&allocator->list_lock);
}

int nd_sa_capacity(nd_sa_handle sa_handle)  
{
	return ((nd_static_alloc_t *)sa_handle)->capacity;
}		

int nd_sa_freenum(nd_sa_handle sa_handle) 
{
	return ((nd_static_alloc_t *)sa_handle)->capacity - ((nd_static_alloc_t *)sa_handle)->allocated_num;
}

int nd_sa_destroy(nd_sa_handle sa_handle, int flag) 
{
	nd_static_alloc_t *alloc =(nd_static_alloc_t *) sa_handle;
	if(!sa_handle) {
		return -1;
	}

	nd_mutex_destroy(&alloc->list_lock);
	
	nd_pool_free(alloc->mem_pool ,alloc) ;	
	//free(allocator) ;
	
	return 0 ;
}


size_t nd_sa_getsize(nd_sa_handle sa_handle) 
{
	return ((nd_static_alloc_t *)sa_handle)->node_size ;
}

//#undef  ND_IMPLEMENT_HANDLE
