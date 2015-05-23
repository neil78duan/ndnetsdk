/* file nd_node_mgr.h
 * 
 * memory block node manager 
 *
 * all right reserved by neil duan 
 */

#ifndef _ND_NODE_MGR_H_
#define _ND_NODE_MGR_H_

/*活动连接的管理节点
 *支持多线程的节点管理器
 * 节点管理器支持[1,65535]
 */
struct node_root;

typedef void *(*node_alloc)(nd_handle alloctor) ;							//从特定分配器上分配一个连接块内存
typedef void (*node_init)(void *node_addr, nd_handle owner) ;				//初始化一个节点owner:节点拥有者
typedef void (*node_dealloc)(void *node_addr,nd_handle alloctor) ;		//从分配器上释放连接块内存
typedef int (*node_freenum)(struct node_root *root) ;											//空闲节点个数
typedef int (*node_capacity)(struct node_root *root) ;											//容量

typedef void(*node_walk_callback)(void *node_addr, void *param) ;
typedef NDUINT16 (*node_accept)(struct node_root *root, void *node_addr);
typedef int (*node_deaccept)(struct node_root *root, NDUINT16 node_id);	//返回0成功，－1失败如果没有返回成功，很可能是引用计数不为0
typedef int (*node_inc_ref)(struct node_root *root, NDUINT16 node_id);	//增加引用次数 返回0成功，－1失败
typedef void (*node_dec_ref)(struct node_root *root, NDUINT16 node_id);	//减少引用次数
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

typedef void* (*node_lock_first)(struct node_root *root,node_iterator *it);	//所住队列中第一个,输入node_id 0
typedef void* (*node_lock_next)(struct node_root *root, node_iterator *it);	//所住下一个,同时释放但前输入node_id但前被锁住的ID,输出node_id已经被所住的下一个ID,并且释放当前被锁住的对象
typedef void (*node_unlock_iterator)(struct node_root *root, node_iterator *it) ;

struct node_info 
{
	ndatomic_t used;			//used status指示此节点是否使用
	ndthread_t  owner;			//拥有者id
	nd_mutex	lock ;			//对udt_socket的互斥
	void *node_addr ;
};

struct node_root
{
	int					max_conn_num;	//最大连接个数
	ndatomic_t			connect_num;	//当前连接数量
	nd_sa_handle		node_alloctor;	//连接分配器(提供连接块内存的分配
	int					base_id ;		//节点起始编号,(可以让多个管理器协同工作)
	struct node_info	*connmgr_addr;	
	nd_handle			mm_pool ;
	size_t				node_size ;

	node_alloc			alloc;
	node_init			init ;		//初始化函数
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

typedef int (*node_create_func)(struct node_root *, int , size_t ,NDUINT16, nd_handle ) ;
typedef void (*node_destroy_func)(struct node_root *) ;

ND_COMMON_API void nd_nodemgr_set(node_create_func create_func,node_destroy_func destroy_func );
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

ND_COMMON_API void nd_node_change_owner(struct node_root *root,ndthread_t owner) ;

#ifdef ND_DEBUG
ND_COMMON_API void nd_node_checkerror(struct node_root *root,NDUINT16 exceptid) ;
#else
#define  nd_node_checkerror //
#endif

#ifdef ND_SOURCE_TRACE
static __INLINE__ int _node_mgr_create_ex(const char *file, int line,struct node_root *root, int max_num, size_t node_size,NDUINT16 start_id,nd_handle mmpool )
{
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

#define nd_node_create nd_node_create_ex
#define nd_node_destroy nd_node_destroy_ex 

#endif
#endif
