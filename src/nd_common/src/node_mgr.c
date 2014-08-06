/* file nd_node_mgr.c
* 
* memory block node manager 
*
* all right reserved by neil duan 
*/

#include "nd_common/nd_common.h"

#define WAIT_LOOP_TIMES		2000
static void nd_node_init(void *socket_node, nd_handle h) ;
static NDUINT16 nd_node_accept(struct node_root *root, void *socket_node);
static int nd_node_deaccept(struct node_root *root, NDUINT16 node_id);
static void *nd_node_lock(struct node_root *root, NDUINT16 node_id);
static void *nd_node_trylock(struct node_root *root, NDUINT16 node_id);
static void nd_node_unlock(struct node_root *root, NDUINT16 node_id);
static void nd_node_walk_node(struct node_root *root,node_walk_callback cb_entry, void *param);
static void *nd_node_search(struct node_root *root, NDUINT16 node_id);
static void* nd_node_lock_first(struct node_root *root,node_iterator *it);
static void* nd_node_lock_next(struct node_root *root,node_iterator *it) ;
static void nd_node_unlock_iterator(struct node_root *root, node_iterator *it) ;
static int nd_node_inc_ref(struct node_root *root, NDUINT16 node_id);
static void nd_node_dec_ref(struct node_root *root, NDUINT16 node_id);

static int nd_node_free_num(struct node_root *root);
static int nd_node_capacity(struct node_root *root);

static void _nd_node_set_owner(struct node_root *root,NDUINT16 node_id , ndthread_t owner) ;
static ndthread_t  _nd_node_get_owner(struct node_root *root,NDUINT16 node_id ) ;

#define USER_SYS_LOCK

static node_create_func s_create_func ;
static node_destroy_func s_destroy_func;

static int _node_create(struct node_root *root, int max_num, size_t node_size,NDUINT16 start_id,nd_handle mmpool) 
{
	int i;
	struct node_info  *conn_node;

	root->max_conn_num = max_num ;
	nd_atomic_set(&(root->connect_num),0) ;
	root->base_id = start_id ;
	root->node_size = node_size ;

	if (node_size> 0) {
		root->node_alloctor = nd_sa_create(max_num, node_size, mmpool);
		if(!root->node_alloctor ) {
			nd_logerror("create session allocator error!\n") ;
			return -1 ;
		}
	}
	if (!mmpool){
		mmpool = nd_global_mmpool() ;
		nd_assert(mmpool) ;
	}
	root->mm_pool =	mmpool ;

	//root->connmgr_addr = (struct node_info *)malloc(max_num*sizeof(struct node_info)) ;
	root->connmgr_addr = (struct node_info *) nd_pool_alloc(mmpool,max_num*sizeof(struct node_info)) ;
	if(!root->connmgr_addr) {
		nd_sa_destroy(root->node_alloctor,1) ;
		root->node_alloctor = 0 ;
		nd_logerror("malloc error!\n") ;
		return -1 ;
	}

	conn_node = root->connmgr_addr;
	for (i=0; i<root->max_conn_num; i++,conn_node++){
		nd_mutex_init(&(conn_node->lock)) ;
		conn_node->node_addr = NULL ;
		conn_node->owner = 0 ;
		//conn_node->is_mask = 0 ;
		nd_atomic_set(&(conn_node->used),0);
		//conn_node->used = 0;
	}

#define SET_FUNC(a,name) if((a)-> name==0)(a)-> name = nd_node_##name
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

	root->set_owner = _nd_node_set_owner ;
	root->get_owner = _nd_node_get_owner ;

#undef SET_FUNC
	root->alloc = nd_sa_alloc ;
	root->dealloc = nd_sa_free;
	root->safe_lock = root->lock ;
	return 0 ;
}

static void _node_destroy(struct node_root *root)
{
	int i;
	struct node_info  *conn_node;
	if(!root->connmgr_addr) {
		return ;
	}

	conn_node = root->connmgr_addr;
	for (i=0; i<root->max_conn_num; i++,conn_node++){
		nd_mutex_destroy(&(conn_node->lock)) ;
	}

	nd_pool_free(root->mm_pool,root->connmgr_addr) ;
	//free(root->connmgr_addr) ;
	root->connmgr_addr = 0 ;

	if (root->node_alloctor)	{
		nd_sa_destroy(root->node_alloctor,1) ;
		root->node_alloctor = 0 ;
	}
}

int nd_node_create_ex(struct node_root *root, int max_num, size_t node_size,NDUINT16 start_id,nd_handle mmpool) 
{
	if (s_create_func){
		return s_create_func(root,max_num, node_size, start_id, mmpool) ;
	}
	else {
		return _node_create(root,max_num, node_size, start_id, mmpool) ;
	}
}

void nd_node_destroy_ex(struct node_root *root)
{
	if(s_destroy_func) {
		s_destroy_func(root) ;
	}
	else {
		_node_destroy(root) ;
	}
}

void nd_nodemgr_set(node_create_func create_func,node_destroy_func destroy_func )
{
	s_create_func = create_func ;
	s_destroy_func = destroy_func;
}

NDUINT16 nd_node_accept(struct node_root *root, void *socket_node)
{	
#ifdef USER_SYS_LOCK

	int i ;
	struct node_info *node;

	nd_assert(root) ;
	node = root->connmgr_addr;
	for (i=0; i<root->max_conn_num; i++, node++) {
		if(0==nd_mutex_trylock(&node->lock) ) {
			if(node->node_addr==NULL) {
				node->node_addr = socket_node ;
				nd_atomic_inc(&(root->connect_num)) ;
				return i+ root->base_id;
			}
			nd_mutex_unlock(&node->lock) ;
		}
	}
	return 0 ;
#else 
	int i ;
	struct node_info *node;

	if(!root)
		return 0 ;
	node = root->connmgr_addr;

	for (i=0; i<root->max_conn_num; i++, node++) {
		if(nd_compare_swap(&node->__used,0,2)) {
			node->owner = nd_thread_self() ;
			nd_mutex_lock(&node->lock) ;
			node->node_addr = socket_node ;
			nd_atomic_inc(&(root->connect_num)) ;
			return i+ root->base_id;
		}
	}
	return 0 ;
#endif

}
#ifdef ND_DEBUG
void nd_node_checkerror(struct node_root *root,NDUINT16 exceptid) 
{
#ifndef USER_SYS_LOCK
	int index = exceptid - root->base_id  ;
	ndatomic_t v = 0 ;
	int i;
	struct node_info *node = root->connmgr_addr ;
	int num = nd_atomic_read(&root->connect_num);
	ndthread_t self = nd_thread_self() ;

	for (i=0; i<root->max_conn_num && num>0; i++,node++){
		
		if(node->__used >0) {
			--num;
			if (index==i) {
				continue;
			}
			if(self== node->owner) {
				nd_logerror("node unlock id=%d\n" AND i+root->base_id) ;
				nd_assert(0);
			}
		}
	}
#endif

}
#endif

int nd_node_deaccept(struct node_root *root, NDUINT16 node_id)
{
#ifdef USER_SYS_LOCK
	int index = node_id - root->base_id  ;
	struct node_info *node ;
	
	if(index<0 || index >= root->max_conn_num)
		return -1;

	node = root->connmgr_addr  ;
	node += index ;
	
	if (node->node_addr){
		node->node_addr = NULL ;
		nd_atomic_dec(&(root->connect_num)) ;
	}
	return 0;
#else 
	int index = node_id - root->base_id  ;
	struct node_info *node ;
	void *p ;

	if(index<0 || index >= root->max_conn_num)
		return -1;

	node = root->connmgr_addr  ;
	node += index ;
	nd_assert(node->__used>1) ;

	p = node->node_addr ;
	
	node->node_addr = NULL ;
	nd_atomic_dec(&(root->connect_num)) ;
	return 0;
#endif
}

//增加引用次数
int nd_node_inc_ref(struct node_root *root, NDUINT16 node_id)
{
#if 0
	ndatomic_t tmp ;
	int index = node_id  - root->base_id ;
	struct node_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return -1;

	node = root->connmgr_addr  ;
	node += index ;
	nd_assert(node->__used>0) ;

	while((tmp = nd_atomic_read(&(node->__used))) > 0) {
		//防止在read test和swap之间变量被修改
		if(nd_compare_swap(&node->__used,tmp,tmp+1)) {
			return 0 ;
		}
	}
#endif 
	return -1 ;
}
//减少引用次数
void nd_node_dec_ref(struct node_root *root, NDUINT16 node_id)
{
#if 0
	ndatomic_t tmp ;
	int index = node_id - root->base_id  ;
	struct node_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return ;

	node = root->connmgr_addr  ;
	node += index ;
	nd_assert(node->__used>1) ;

	while((tmp = nd_atomic_read(&(node->__used))) > 1) {
		//防止在read test和swap之间变量被修改
		if(nd_compare_swap(&node->__used,tmp,tmp-1)) {
			return  ;
		}
	}
#endif
}

void *nd_node_lock(struct node_root *root, NDUINT16 node_id)
{
#ifdef USER_SYS_LOCK
	
	register int looptimes = WAIT_LOOP_TIMES ;

	int index = node_id - root->base_id ;
	struct node_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return NULL;

	node = root->connmgr_addr  ;
	node += index ;
	nd_mutex_lock(&node->lock);	
	if(node->node_addr) {
		//node->owner = nd_thread_self() ;
		return node->node_addr ;
	}
	nd_mutex_unlock(&node->lock);	
	return NULL;
#else

	register int looptimes = WAIT_LOOP_TIMES ;

	int index = node_id - root->base_id ;
	struct node_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return NULL;

	node = root->connmgr_addr  ;
	node += index ;
	
	do {
		if(nd_compare_swap(&node->__used,1,2)) {		//only can be lock when used==1
			nd_mutex_lock(&node->lock);	
			if(node->node_addr) {
				node->owner = nd_thread_self() ;
				return node->node_addr ;
			}
			nd_atomic_dec(&node->__used) ;
			nd_mutex_unlock(&node->lock);	
			return NULL;
		}
		else if (nd_atomic_read(&node->__used)>1) {	
			if (node->owner == nd_thread_self()&& node->node_addr){
				nd_atomic_inc(&node->__used) ;
				return node->node_addr ;
			}
		}
		else 
			return NULL;
	} while (--looptimes>0);
	
	return NULL ;
#endif
}

void *nd_node_trylock(struct node_root *root, NDUINT16 node_id)
{
#ifdef USER_SYS_LOCK
	int index =  node_id  - root->base_id ;
	struct node_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return NULL;

	node = root->connmgr_addr  ;
	node += index ;
	if(0==nd_mutex_trylock(&node->lock) ){
		if(node->node_addr) {
			return node->node_addr ;
		}
		nd_mutex_unlock(&node->lock);	
	}
	return NULL;
#else
	register int looptimes = WAIT_LOOP_TIMES ;
	int index =  node_id  - root->base_id ;
	struct node_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return NULL;

	node = root->connmgr_addr  ;
	node += index ;
	
	do {
		if(nd_compare_swap(&node->__used,1,2)) {		//only can be lock when used==1
			if(0==nd_mutex_trylock(&node->lock) ){
				if(node->node_addr) {
					node->owner = nd_thread_self() ;
					return node->node_addr ;
				}
				nd_mutex_unlock(&node->lock);	
			}
			nd_atomic_dec(&node->__used) ;
			return NULL;
		}
		else if (nd_atomic_read(&node->__used)>1){
			if (node->owner == nd_thread_self()&& node->node_addr){
				nd_atomic_inc(&node->__used) ;
				return node->node_addr ;
			}
		}
		else 
			return NULL;
	} while (--looptimes>0);

	return NULL ;
#endif
}

void nd_node_unlock(struct node_root *root, NDUINT16 node_id)
{
#ifdef USER_SYS_LOCK
	int index =  node_id  - root->base_id ;
	struct node_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return ;

	node = root->connmgr_addr  ;
	node += index ;

	nd_mutex_unlock(&node->lock) ;


#else 
	ndatomic_t swval ;
	int index =  node_id  - root->base_id ;
	struct node_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return ;

	node = root->connmgr_addr  ;
	node += index ;

	if(node->node_addr==NULL) {
		swval = 0 ;
	}
	else {
		swval = 1;
	}
	
	if(nd_compare_swap(&node->__used,2,swval)) {
		//only lock one times		
		node->owner = 0 ;
		nd_mutex_unlock(&node->lock) ;
	}	
	else {
		//self lock multitimes needn't unlock mutex
		ndatomic_t v = 0 ;
		while ( (v=nd_atomic_read(&node->__used)) > 0) 	{
			if(nd_compare_swap(&node->__used, v,v-1) ){
				break ;
			}
		}
	}
#endif

}

void nd_node_walk_node(struct node_root *root,node_walk_callback cb_entry, void *param)
{
	ndatomic_t v = 0 ;
	int i;
	struct node_info *node = root->connmgr_addr ;
	int num = nd_atomic_read(&root->connect_num);

	for (i=0; i<root->max_conn_num && num>0; i++,node++){

#ifdef USER_SYS_LOCK
		if(0==nd_mutex_trylock(&node->lock) ){
			if(node->node_addr)
				cb_entry(node->node_addr, param);
			nd_mutex_unlock(&node->lock);	
		}
#else
		if(nd_atomic_read(&(node->__used)) >0) {
			--num ;
			if(nd_compare_swap(&node->__used,1,2)) {		//only can be lock when used==1
				if(0==nd_mutex_trylock(&node->lock) ){
					if(node->node_addr)
						cb_entry(node->node_addr, param);
					nd_mutex_unlock(&node->lock);	
				}
				nd_atomic_dec(&node->__used) ;			
				//nd_compare_swap(&node->__used,2,1);
			}

		}
#endif
	}
}

void *nd_node_search(struct node_root *root, NDUINT16 node_id)
{

	int index = node_id - root->base_id  ;
	struct node_info *node ;

	if(index<0 || index >= root->max_conn_num)
		return NULL;

	node = root->connmgr_addr  ;
	node += index ;

#ifdef USER_SYS_LOCK
#else
	if(0==nd_atomic_read(&(node->__used))) 
		return NULL;
#endif
	return node->node_addr ;
}


void nd_node_init(void *socket_node, nd_handle h)
{
	nd_msgbox("please set initialization function of client manager", "error" ) ;
	nd_logerror("please set initialization function of client manager") ;
	return ;
}

void* nd_node_lock_first(struct node_root *root,node_iterator *it)
{
	ndatomic_t v = 0 ;
	int i ;
	void *p ;
	struct node_info *node;
	if (nd_atomic_read(&(root->connect_num)) < 1){
		return NULL;
	}

	node = root->connmgr_addr  ;
	it->node_id = root->base_id ;
	it->numbers = 0 ;
	it->total =(NDUINT16) root->connect_num ;

	for (i=0; i<root->max_conn_num; i++, node++,(it->node_id)++ ) {

#ifdef USER_SYS_LOCK
		if (node->node_addr){
			++(it->numbers) ; 
			p = nd_node_trylock(root,it->node_id) ;
			if (p){
				return  p ;
			}
		}
#else
		if (nd_atomic_read(&node->__used)){
			++(it->numbers) ; 
			p = nd_node_trylock(root,it->node_id) ;
			if (p){
				return  p ;
			}
		}
#endif
	}
	it->node_id = 0 ;
	return NULL ;
}

void nd_node_unlock_iterator(struct node_root *root, node_iterator *it) 
{
	nd_node_unlock(root, it->node_id) ;

	it->node_id = 0 ;
	it->numbers = 0 ;
	it->total = 0 ;
}

void* nd_node_lock_next(struct node_root *root,node_iterator *it)
{
	void *p;
	ndatomic_t v = 0 ;
	int i = (it->node_id) - root->base_id ;

	nd_assert(i>=0 && i<root->max_conn_num) ;

	nd_node_unlock(root, it->node_id) ;
	++i ;
	++(it->node_id) ;

	for ( ;i<root->max_conn_num && it->numbers < it->total; i++, (it->node_id)++) {
		struct node_info *node = root->connmgr_addr + i  ;
#ifdef USER_SYS_LOCK		
		if (node->node_addr){
			++(it->numbers) ; 
			p = nd_node_trylock(root,it->node_id) ;
			if (p){
				return  p ;
			}
		}
#else 
		if (nd_atomic_read(&node->__used)){
			++(it->numbers) ; 
			p = nd_node_trylock(root,it->node_id) ;
			if (p){
				return  p ;
			}
		}
#endif
	}
	it->node_id = 0 ;
	return NULL ;
}

void nd_node_set_allocator(struct node_root *root,nd_handle allocator,node_alloc alloc,node_dealloc dealloc)
{
	root->node_alloctor = allocator;
	root->alloc = alloc ;
	root->dealloc = dealloc ;
}
int nd_node_free_num(struct node_root *root)
{
	return nd_sa_freenum(root->node_alloctor) ;
}
int nd_node_capacity(struct node_root *root)
{
	return nd_sa_capacity(root->node_alloctor) ;

}

void _nd_node_set_owner(struct node_root *root,NDUINT16 node_id , ndthread_t owner) 
{

	int index = node_id - root->base_id ;
	
	if(index<0 || index >= root->max_conn_num)
		return ;

	root->connmgr_addr[index].owner = owner;
}

ndthread_t  _nd_node_get_owner(struct node_root *root,NDUINT16 node_id ) 
{
	int index = node_id - root->base_id ;
	
	if(index<0 || index >= root->max_conn_num)
		return 0;

	return root->connmgr_addr[index].owner ;
}

void nd_node_change_owner(struct node_root *root,ndthread_t owner)
{
	int i;
	int num = nd_atomic_read(&root->connect_num);
	NDUINT16 nodeid = root->base_id ;

	for (i=0; i<root->max_conn_num && num>0; i++,nodeid++){
		root->set_owner(root, nodeid, owner ) ;
	}
}

