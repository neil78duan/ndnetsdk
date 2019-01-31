/* file nd_node_mgr.h
 * 
 * memory block node manager 
 *
 * all right reserved by neil duan 
 */

#ifndef _ND_NODE_MGR_H_
#define _ND_NODE_MGR_H_

#include "nd_common/nd_export_def.h"
#include "nd_common/nd_static_alloc.h"

/*
 * resource node manager 
 * node id is [1,65535]
 */
struct node_root;

typedef void *(*node_alloc)(nd_handle alloctor) ;							//alloc node 
typedef void (*node_init)(void *node_addr, nd_handle owner) ;				//init node
typedef void (*node_dealloc)(void *node_addr,nd_handle alloctor) ;		// free node 
typedef int (*node_freenum)(struct node_root *root) ;											//get free number
typedef int (*node_capacity)(struct node_root *root) ;											//get capacity

typedef void(*node_walk_callback)(struct node_root *root, NDUINT16 node_id, void *param);
typedef NDUINT16 (*node_accept)(struct node_root *root, void *node_addr);
typedef int (*node_deaccept)(struct node_root *root, NDUINT16 node_id);	//accept a node , like register 
typedef int (*node_inc_ref)(struct node_root *root, NDUINT16 node_id);	//add reference count 
typedef void (*node_dec_ref)(struct node_root *root, NDUINT16 node_id);	//decement reference count
typedef void *(*node_lock)(struct node_root *root, NDUINT16 node_id);
typedef void *(*node_trylock)(struct node_root *root, NDUINT16 node_id);
typedef void (*node_unlock)(struct node_root *root, NDUINT16 node_id);
typedef void (*node_walk_node)(struct node_root *root,node_walk_callback cb_entry, void *param);
typedef void *(*node_search)(struct node_root *root, NDUINT16 node_id);

typedef void (*node_set_owner)(struct node_root *root,NDUINT16 node_id , ndthread_t owner) ;
typedef ndthread_t (*node_get_owner)(struct node_root *root,NDUINT16 node_id ) ;

typedef struct node_iterator
{
	NDUINT16 node_id ;
	NDUINT16 numbers ;
	NDUINT16 total ;
}node_iterator ;

typedef void* (*node_lock_first)(struct node_root *root,node_iterator *it);	//get first node 
typedef void* (*node_lock_next)(struct node_root *root, node_iterator *it);	//get next node 
typedef void (*node_unlock_iterator)(struct node_root *root, node_iterator *it) ;


typedef int(*node_create_func)(struct node_root *root, int max_num, size_t node_size, NDUINT16 start_id, nd_handle mmpool);
typedef void(*node_destroy_func)(struct node_root *);

struct node_info
{
	ndatomic_t used;			//used status flag
	ndthread_t  owner;			//owner thread id
	void *node_addr;
};

struct node_root
{
	int					max_conn_num;	//capacity
	ndatomic_t			connect_num;	//current number
	int					base_id;		//start index
	int					param;			//user define patam
	nd_sa_handle		node_alloctor;	//alloctor
	struct node_info	*connmgr_addr;	
	nd_handle			mm_pool ;
	size_t				node_size ;

	node_create_func	root_creator;
	node_destroy_func	root_destroy;

	node_alloc			alloc;
	node_init			init ;		
	node_dealloc		dealloc ;
	
	node_freenum		free_num;
	node_capacity		capacity;

	//define connect manager function 
	node_accept			accept ;
	node_deaccept		deaccept ;
	node_inc_ref		inc_ref ;
	node_dec_ref		dec_ref ;
	node_lock			lock;
	node_trylock		trylock ;
	node_unlock			unlock ;
	node_walk_node		walk_node ;
	node_search			search;

	node_lock_first		lock_first ;
	node_lock_next		lock_next ;
	node_unlock_iterator	unlock_iterator;
	node_lock			safe_lock;
	node_set_owner		set_owner ;
	node_get_owner		get_owner ;
};

ND_COMMON_API int nd_node_preinit(struct node_root *root, node_create_func creator, node_destroy_func destroy);
ND_COMMON_API int nd_node_create_ex(struct node_root *root, int max_num, size_t node_size,NDUINT16 start_id,nd_handle mmpool ) ;
ND_COMMON_API void nd_node_destroy_ex(struct node_root *root) ;
ND_COMMON_API void nd_node_set_allocator(struct node_root *root,nd_handle allocator,node_alloc alloc,node_dealloc dealloc);

static __INLINE__ void nd_node_set_owner(struct node_root *root,NDUINT16 node_id , ndthread_t owner) 
{
	root->set_owner(root, node_id, owner) ;
}
static __INLINE__  ndthread_t  nd_node_get_owner(struct node_root *root,NDUINT16 node_id ) 
{
	return root->get_owner(root,node_id);
}
static __INLINE__ int nd_node_free_num(struct node_root *root)
{
	return root->free_num (root) ;
}
static __INLINE__ int nd_node_capacity(struct node_root *root)
{
	return root->capacity(root) ;
}

static __INLINE__  void nd_node_checkerror(struct node_root *root, NDUINT16 exceptid)
{

}

ND_COMMON_API void nd_node_change_owner(struct node_root *root,ndthread_t owner) ;

#ifdef ND_SOURCE_TRACE
static __INLINE__ int _node_mgr_create_ex(const char *file, int line,struct node_root *root, int max_num, size_t node_size,NDUINT16 start_id,nd_handle mmpool )
{
	nd_node_preinit(root,NULL,NULL);
	if(0==nd_node_create_ex(root,  max_num,  node_size, start_id, mmpool ) ) {
		_source_log((void *)root ,"create node mgr","node mgr not release", file, line) ;
		return 0 ;
	}
	return -1 ;
}
static __INLINE__  void _node_mgr_destroy_ex(struct node_root *root) 
{
	_source_release((void*)root) ;
	nd_node_destroy_ex(root) ;
}
#define nd_node_create(root, max_num, node_size,start_id,mmpool ) _node_mgr_create_ex(__FILE__,__LINE__,root, max_num, node_size,start_id,mmpool)
#define nd_node_destroy(root) _node_mgr_destroy_ex(root) 
#else 

static __INLINE__ int nd_node_create(struct node_root *root, int max_num, size_t node_size,NDUINT16 start_id,nd_handle mmpool )
{
	nd_node_preinit(root,NULL,NULL);
	return nd_node_create_ex(root,  max_num,  node_size, start_id, mmpool ) ;
}
#define nd_node_destroy nd_node_destroy_ex 

#endif
#endif
