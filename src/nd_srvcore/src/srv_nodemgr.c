/* file srv_nodemgr.c
 *
 * manager node source 
 *
 * create by duan 
 * 2011/3/2 10:05:10
 */

/* file nd_node_mgr.c
* 
* memory block node manager 
*
* all right reserved by neil duan 
*/
#include "nd_srvcore/nd_srvlib.h"

#if !defined (USE_NEW_MODE_LISTEN_THREAD)

#define WAIT_LOOPS 2000
#include "nd_common/nd_common.h"

static void _nd_node_init(void *socket_node, nd_handle h) ;
static NDUINT16 _nd_node_accept(struct node_root *root, void *socket_node);
static int _nd_node_deaccept(struct node_root *root, NDUINT16 node_id);
static void *_nd_node_lock(struct node_root *root, NDUINT16 node_id);
static void *_nd_node_trylock(struct node_root *root, NDUINT16 node_id);
static void _nd_node_unlock(struct node_root *root, NDUINT16 node_id);
static void _nd_node_walk_node(struct node_root *root,node_walk_callback cb_entry, void *param);
static void *_nd_node_search(struct node_root *root, NDUINT16 node_id);
static void* _nd_node_lock_first(struct node_root *root,node_iterator *it);
static void* _nd_node_lock_next(struct node_root *root,node_iterator *it) ;
static void _nd_node_unlock_iterator(struct node_root *root, node_iterator *it) ;
static int _nd_node_inc_ref(struct node_root *root, NDUINT16 node_id);
static void _nd_node_dec_ref(struct node_root *root, NDUINT16 node_id);
static void *_nd_node_safe_lock(struct node_root *root, NDUINT16 node_id);
static int _nd_node_free_num(struct node_root *root);
static int _nd_node_capacity(struct node_root *root);

static void _nd_node_set_owner(struct node_root *root,NDUINT16 node_id , ndthread_t owner) ;
static ndthread_t  _nd_node_get_owner(struct node_root *root,NDUINT16 node_id ) ;


struct srvnode_info 
{
	ndatomic_t used;			//used status指示此节点是否使用
	ndthread_t  owner;			//拥有者id
	ndthread_t locked_id;		//LOCKED BY THREAD
	void *node_addr ;
};


//只能使用静态分配器
#define USER_STATIC_ALLOC 1

#ifndef USER_STATIC_ALLOC
static  void *_srv_node_alloc(nd_handle alloctor) 
{
	struct node_root *root = (struct node_root *)alloctor ;
	return nd_pool_alloc(root->mm_pool,root->node_size) ;
}

static  void _srv_node_dealloc(void *node_addr,nd_handle alloctor) 
{
	struct node_root *root = (struct node_root *)alloctor ;
	nd_pool_free(root->mm_pool,node_addr) ;

}
#endif 


int nd_srvnode_create(struct node_root *root, int max_num, size_t node_size,NDUINT16 start_id, nd_handle mmpool) 
{
	if (!mmpool){
		mmpool = nd_global_mmpool() ;
		nd_assert(mmpool) ;
	}

	root->max_conn_num = max_num ;
	nd_atomic_set(&(root->connect_num),0) ;
	root->base_id = start_id ;
	root->node_size = node_size ;

#ifdef USER_STATIC_ALLOC
	if (node_size> 0) {
		root->node_alloctor = nd_sa_create(max_num, node_size, mmpool);
		if(!root->node_alloctor ) {
			nd_logerror("create session allocator error!\n") ;
			return -1 ;
		}
	}
#else
	root->node_alloctor = (nd_sa_handle)root ;
#endif 

	root->mm_pool =	mmpool ;
	root->connmgr_addr = (struct node_info *) nd_pool_alloc(mmpool,max_num*sizeof(struct srvnode_info)) ;

	if(!root->connmgr_addr) {
		nd_sa_destroy(root->node_alloctor,1) ;
		root->node_alloctor = 0 ;
		nd_logerror("malloc error!\n") ;
		return -1 ;
	}
	memset(root->connmgr_addr, 0, max_num*sizeof(struct srvnode_info)) ;


#define SET_FUNC(a,name) if((a)-> name==0)(a)-> name = _nd_node_##name
	//SET_FUNC(root,alloc) ;
	SET_FUNC(root,init) ;
	//SET_FUNC(root,dealloc) ;
	SET_FUNC(root,accept) ;
	SET_FUNC(root,deaccept) ;
	SET_FUNC(root,lock) ;
	SET_FUNC(root,trylock) ;
	SET_FUNC(root,unlock) ;
	SET_FUNC(root,walk_node) ;
	SET_FUNC(root,search) ;
	SET_FUNC(root,lock_first) ;
	SET_FUNC(root,lock_next) ;
	SET_FUNC(root,unlock_iterator) ;
	SET_FUNC(root,inc_ref) ;
	SET_FUNC(root,dec_ref) ;
	SET_FUNC(root,free_num) ;
	SET_FUNC(root,capacity) ;
	SET_FUNC(root,safe_lock) ;

#undef SET_FUNC

	root->set_owner = _nd_node_set_owner ;
	root->get_owner = _nd_node_get_owner ;

#ifdef USER_STATIC_ALLOC
	root->alloc = nd_sa_alloc ;
	root->dealloc = nd_sa_free ;
#else 
	root->alloc = _srv_node_alloc ;
	root->dealloc = _srv_node_dealloc;

#endif 


	return 0 ;
}

void nd_srvnode_destroy(struct node_root *root)
{
	nd_assert(root->connmgr_addr) ;
	if(!root->connmgr_addr) {		
		return ;
	}
	
	nd_pool_free(root->mm_pool,root->connmgr_addr) ;
	root->connmgr_addr = 0 ;
#ifdef USER_STATIC_ALLOC
	if (root->node_alloctor)	{
		nd_sa_destroy(root->node_alloctor,1) ;
	}
#endif 
	root->node_alloctor = 0 ;
}

//减少运用计数
static void __dec_ref(struct node_root *root, struct srvnode_info *node)
{
	ndatomic_t v = 0;
	ndthread_t self = nd_thread_self();
	while ( (v=nd_atomic_read(&node->used)) > 0) 	{
		if (node->locked_id != self || v==1)	{
			break;
		}

		if(nd_compare_swap(&node->used, v,v-1) ){
			if (v==1) {
				node->node_addr = NULL ;
				node->owner = 0 ;
				node->locked_id = 0;
				//nd_atomic_dec(&(root->connect_num)) ;
			}
			else if(2==v) {
				node->locked_id = 0 ;
			}
			break ;
		}
	}
}

static __INLINE__ void* _lock_node(struct srvnode_info *node) 
{
	ndthread_t self = nd_thread_self() ;
	if (self != node->owner || nd_atomic_read(&node->used)==0){
		return 0 ;
	}
	//nd_assert(node->used ==1);
	if(nd_compare_swap(&node->used,1,2)) {		//only can be lock when used==1
		if(ND_ALLOC_MM_VALID(node->node_addr) ) {
			node->locked_id = self ;
			return node->node_addr ;
		}
		nd_atomic_dec(&node->used) ;
	}
	else if (nd_atomic_read(&node->used)>1) {	
		if (node->locked_id == self && ND_ALLOC_MM_VALID(node->node_addr)){
			nd_atomic_inc(&node->used) ;

			//nd_assert(node->used < 3);
			return node->node_addr ;
		}
	}
	return NULL;
}

static __INLINE__ void _unlock_node(struct node_root *root,struct srvnode_info *node )
{
	__dec_ref(root, node) ;
	
}

NDUINT16 _nd_node_accept(struct node_root *root, void *socket_node)
{	
	int i ;
	struct srvnode_info *node;

	if(!root)
		return 0 ;
	node = (struct srvnode_info*) root->connmgr_addr;

	for (i=0; i<root->max_conn_num; i++, node++) {
		if(nd_compare_swap(&node->used,0,2)) {
			nd_assert(node->owner==0);
			node->owner = nd_thread_self() ;
			node->locked_id = node->owner ;
			node->node_addr = socket_node ;
			nd_atomic_inc(&(root->connect_num)) ;
			return i+ root->base_id;
		}
	}
	return 0 ;
}


int _nd_node_deaccept(struct node_root *root, NDUINT16 node_id)
{
	int index = node_id - root->base_id  ;
	struct srvnode_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return -1;

	node = (struct srvnode_info*) root->connmgr_addr  ;
	node += index ;

	node->node_addr = 0 ;
	node->owner = 0 ;
	node->locked_id = 0;
	nd_atomic_set(&node->used, 0);

	nd_atomic_dec(&(root->connect_num)) ;

	return 0;
}

//增加引用次数
int _nd_node_inc_ref(struct node_root *root, NDUINT16 node_id)
{
	ndatomic_t tmp ;
	int index = node_id  - root->base_id ;
	struct srvnode_info *node ;
	
	if(index<0 || index >= root->max_conn_num)
		return -1;

	node =(struct srvnode_info *) root->connmgr_addr  ;
	node += index ;

	while((tmp = nd_atomic_read(&(node->used))) > 0 ) {
		if(nd_compare_swap(&node->used,tmp,tmp+1)) {
			return 0 ;
		}
	}
	return -1 ;
}
//减少引用次数
void _nd_node_dec_ref(struct node_root *root, NDUINT16 node_id)
{
	int index = node_id - root->base_id  ;
	struct srvnode_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return ;

	node = (struct srvnode_info *)root->connmgr_addr  ;
	node += index ;
	__dec_ref(root,node) ;
}

void *_nd_node_lock(struct node_root *root, NDUINT16 node_id)
{
	int index = node_id - root->base_id ;
	struct srvnode_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return NULL;

	node = (struct srvnode_info *)root->connmgr_addr  ;
	node += index ;

	return _lock_node(node) ;
}

void *_nd_node_safe_lock(struct node_root *root, NDUINT16 node_id)
{
	void *paddr  ;
	int index = node_id - root->base_id ;
	struct srvnode_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return NULL;

	node =(struct srvnode_info *) root->connmgr_addr  ;
	node += index ;

	paddr = _lock_node(node) ;
	if (paddr){
		return paddr ;
	}
	if(nd_compare_swap(&node->used,1,2)) {		//only can be lock when used==1
		return node->node_addr ; 
	}
	return NULL;

}

void *_nd_node_trylock(struct node_root *root, NDUINT16 node_id)
{
	return _nd_node_lock(root,node_id) ;

}

void _nd_node_unlock(struct node_root *root, NDUINT16 node_id)
{
	int index =  node_id  - root->base_id ;
	struct srvnode_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return ;

	node =(struct srvnode_info *) root->connmgr_addr  ;
	node += index ;
	_unlock_node(root, node) ;
}

void _nd_node_walk_node(struct node_root *root,node_walk_callback cb_entry, void *param)
{
	int i;
	struct srvnode_info *node = (struct srvnode_info *) root->connmgr_addr ;
	int num = nd_atomic_read(&root->connect_num);
	
	for (i=0; i<root->max_conn_num && num>0; i++,node++){

		if(nd_atomic_read(&(node->used)) >0) {
			--num ;
			cb_entry(root,node->node_addr, param);
		}
	}
}

void *_nd_node_search(struct node_root *root, NDUINT16 node_id)
{

	int index = node_id - root->base_id  ;
	struct srvnode_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return NULL;

	node =(struct srvnode_info *) root->connmgr_addr  ;
	node += index ;

#ifdef USER_SYS_LOCK
#else
	if(0==nd_atomic_read(&(node->used))) 
		return NULL;
#endif
	return node->node_addr ;
}


void _nd_node_init(void *socket_node, nd_handle h)
{
	nd_msgbox("please set initialization function of client manager", "error" ) ;
	nd_logerror("please set initialization function of client manager") ;
	return ;
}

void* _nd_node_lock_first(struct node_root *root,node_iterator *it)
{
	//ndatomic_t v = 0 ;
	int i ;
	struct srvnode_info *node;
	if (nd_atomic_read(&(root->connect_num)) < 1){
		return NULL;
	}

	node =(struct srvnode_info *) root->connmgr_addr  ;
	it->node_id = root->base_id ;
	it->numbers = 0 ;
	it->total = (NDUINT16 )root->connect_num ;

	for (i=0; i<root->max_conn_num; i++, node++,(it->node_id)++ ) {

		if(nd_atomic_read(&(node->used)) >0) {
			++(it->numbers) ; 
			if (_lock_node(node))	{
				return node->node_addr ;
			}
		}
	}
	it->node_id = 0 ;
	return NULL ;
}

void _nd_node_unlock_iterator(struct node_root *root, node_iterator *it) 
{
	_nd_node_unlock(root, it->node_id) ;

	it->node_id = 0 ;
	it->numbers = 0 ;
	it->total = 0 ;
}

void* _nd_node_lock_next(struct node_root *root,node_iterator *it)
{
	//ndatomic_t v = 0 ;
	int i = (it->node_id) - root->base_id ;

	nd_assert(i>=0 && i<root->max_conn_num) ;

	_nd_node_unlock(root, it->node_id) ;
	++i ;
	++(it->node_id) ;

	for ( ;i<root->max_conn_num && it->numbers < it->total; i++, (it->node_id)++) {
		struct srvnode_info *node =(struct srvnode_info *) root->connmgr_addr   ;
		node += i ;

		if(nd_atomic_read(&(node->used)) >0) {
			++(it->numbers) ; 
			if (_lock_node(node))	{
				return node->node_addr ;
			}
		}
	}
	it->node_id = 0 ;
	return NULL ;
}

int _nd_node_free_num(struct node_root *root)
{
#ifdef USER_STATIC_ALLOC
	return nd_sa_freenum(root->node_alloctor) ;
#else 
	return root->max_conn_num - root->connect_num ;
#endif
}
int _nd_node_capacity(struct node_root *root)
{
#ifdef USER_STATIC_ALLOC
	return nd_sa_capacity(root->node_alloctor) ;
#else 
	return root->max_conn_num ;
#endif

}

void _nd_node_set_owner(struct node_root *root,NDUINT16 node_id , ndthread_t owner) 
{
	struct srvnode_info *node = (struct srvnode_info *)root->connmgr_addr ;
	int index = node_id - root->base_id ;

	if(index<0 || index >= root->max_conn_num)
		return ;

	node[index].owner = owner;
}

ndthread_t  _nd_node_get_owner(struct node_root *root,NDUINT16 node_id ) 
{
	struct srvnode_info *node = (struct srvnode_info *)root->connmgr_addr ;
	int index = node_id - root->base_id ;

	if(index<0 || index >= root->max_conn_num)
		return 0;

	return node[index].owner ;
}
#endif


