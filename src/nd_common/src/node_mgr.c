/* file nd_node_mgr.c
* 
* memory block node manager 
*
* all right reserved by neil duan 
*/

#include "nd_common/nd_common.h"

#include "nd_common/nd_static_alloc.h"
#include "nd_common/nd_node_mgr.h"

//////////////////////////////////////////////////////////////////////////
// define session nodes manager
static void _nd_node_init(void *socket_node, nd_handle h);
static NDUINT16 _nd_node_accept(struct node_root *root, void *socket_node);
static int _nd_node_deaccept(struct node_root *root, NDUINT16 node_id);
static void *_nd_node_lock(struct node_root *root, NDUINT16 node_id);
static void *_nd_node_trylock(struct node_root *root, NDUINT16 node_id);
static void _nd_node_unlock(struct node_root *root, NDUINT16 node_id);
static void _nd_node_walk_node(struct node_root *root, node_walk_callback cb_entry, void *param);
static void *_nd_node_search(struct node_root *root, NDUINT16 node_id);
static void* _nd_node_lock_first(struct node_root *root, node_iterator *it);
static void* _nd_node_lock_next(struct node_root *root, node_iterator *it);
static void _nd_node_unlock_iterator(struct node_root *root, node_iterator *it);
static int _nd_node_inc_ref(struct node_root *root, NDUINT16 node_id);
static void _nd_node_dec_ref(struct node_root *root, NDUINT16 node_id);
static void *_nd_node_safe_lock(struct node_root *root, NDUINT16 node_id);
static int _nd_node_free_num(struct node_root *root);
static int _nd_node_capacity(struct node_root *root);

static void _nd_node_set_owner(struct node_root *root, NDUINT16 node_id, ndthread_t owner);
static ndthread_t  _nd_node_get_owner(struct node_root *root, NDUINT16 node_id);


struct srvnode_info
{
	ndatomic_t used;			//used status 0 not used
	ndthread_t  owner;			//owner id
	void *node_addr;
};


static int nd_node_mgr_create(struct node_root *root, int max_num, size_t node_size, NDUINT16 start_id, nd_handle mmpool)
{
	if (!mmpool){
		mmpool = nd_global_mmpool();
		nd_assert(mmpool);
	}

	root->param = 0;
	root->max_conn_num = max_num;
	nd_atomic_set(&(root->connect_num), 0);
	root->base_id = start_id;
	root->node_size = node_size;

	if (node_size > 0) {
		root->node_alloctor = nd_sa_create(max_num, node_size, mmpool);
		if (!root->node_alloctor) {
			nd_logerror("create session allocator error!\n");
			return -1;
		}
	}

	root->mm_pool = mmpool;
	root->connmgr_addr = (struct node_info *) nd_pool_alloc(mmpool, max_num*sizeof(struct srvnode_info));

	if (!root->connmgr_addr) {
		nd_sa_destroy(root->node_alloctor, 1);
		root->node_alloctor = 0;
		nd_logerror("malloc error!\n");
		return -1;
	}
	memset(root->connmgr_addr, 0, max_num*sizeof(struct srvnode_info));


#define SET_FUNC(a,name) if((a)-> name==0)(a)-> name = _nd_node_##name
	//SET_FUNC(root,alloc) ;
	SET_FUNC(root, init);
	//SET_FUNC(root,dealloc) ;
	SET_FUNC(root, accept);
	SET_FUNC(root, deaccept);
	SET_FUNC(root, lock);
	SET_FUNC(root, trylock);
	SET_FUNC(root, unlock);
	SET_FUNC(root, walk_node);
	SET_FUNC(root, search);
	SET_FUNC(root, lock_first);
	SET_FUNC(root, lock_next);
	SET_FUNC(root, unlock_iterator);
	SET_FUNC(root, inc_ref);
	SET_FUNC(root, dec_ref);
	SET_FUNC(root, free_num);
	SET_FUNC(root, capacity);
	SET_FUNC(root, safe_lock);

	SET_FUNC(root, set_owner);
	SET_FUNC(root, get_owner);

#undef SET_FUNC

	root->alloc = nd_sa_alloc;
	root->dealloc = nd_sa_free;


	return 0;
}

static void nd_node_mgr_destroy(struct node_root *root)
{
	nd_assert(root->connmgr_addr);
	if (!root->connmgr_addr) {
		return;
	}

	nd_pool_free(root->mm_pool, root->connmgr_addr);
	root->connmgr_addr = 0;
	if (root->node_alloctor)	{
		nd_sa_destroy(root->node_alloctor, 1);
	} 
	root->node_alloctor = 0;
}


NDUINT16 _nd_node_accept(struct node_root *root, void *socket_node)
{
	int i, index;
	struct srvnode_info *node;

	if (!root)
		return 0;
	index = root->param;

	for (i = 0; i < root->max_conn_num; i++, index++) {
		if (index >= root->max_conn_num){
			index = 0;
		}
		node = &((struct srvnode_info*) root->connmgr_addr)[index];

		if (nd_compare_swap(&node->used, 0, 1)) {
			nd_assert(node->owner == 0);
			node->owner = nd_thread_self();
			node->node_addr = socket_node;
			nd_atomic_inc(&(root->connect_num));
			return index + root->base_id;
		}
	}
	return 0;
}


int _nd_node_deaccept(struct node_root *root, NDUINT16 node_id)
{
	int index = node_id - root->base_id;
	struct srvnode_info *node;

	if (index < 0 || index >= root->max_conn_num)
		return -1;

	node = (struct srvnode_info*) root->connmgr_addr;
	node += index;

	node->node_addr = 0;
	node->owner = 0;
	nd_atomic_set(&node->used, 0);
	nd_atomic_dec(&(root->connect_num));

	return 0;
}

//inc reference 
int _nd_node_inc_ref(struct node_root *root, NDUINT16 node_id)
{
	ndatomic_t tmp;
	int index = node_id - root->base_id;
	struct srvnode_info *node;

	if (index < 0 || index >= root->max_conn_num)
		return -1;

	node = (struct srvnode_info *) root->connmgr_addr;
	node += index;

	while ((tmp = nd_atomic_read(&(node->used))) > 0) {
		if (nd_compare_swap(&node->used, tmp, tmp + 1)) {
			return 0;
		}
	}
	return -1;
}
//dec reference count
void _nd_node_dec_ref(struct node_root *root, NDUINT16 node_id)
{
	int index = node_id - root->base_id;
	struct srvnode_info *node;

	if (index < 0 || index >= root->max_conn_num)
		return;

	node = (struct srvnode_info *)root->connmgr_addr;
	node += index;
	nd_atomic_dec(&node->used);
}

void *_nd_node_lock(struct node_root *root, NDUINT16 node_id)
{
	int index = node_id - root->base_id;
	struct srvnode_info *node;

	if (index < 0 || index >= root->max_conn_num)
		return NULL;

	node = (struct srvnode_info *)root->connmgr_addr;
	node += index;
	if (nd_atomic_read(&node->used) == 0) {
		return NULL;
	}
	if (node->owner != nd_thread_self()) {
		return NULL;
	}
	return node->node_addr;
}

void *_nd_node_safe_lock(struct node_root *root, NDUINT16 node_id)
{
	return _nd_node_lock(root, node_id);

}

void *_nd_node_trylock(struct node_root *root, NDUINT16 node_id)
{
	return _nd_node_lock(root, node_id);

}

void _nd_node_unlock(struct node_root *root, NDUINT16 node_id)
{
	return;
}

void _nd_node_walk_node(struct node_root *root, node_walk_callback cb_entry, void *param)
{
	int i;
	struct srvnode_info *node = (struct srvnode_info *) root->connmgr_addr;
	int num = nd_atomic_read(&root->connect_num);

	for (i = 0; i < root->max_conn_num && num>0; i++, node++){

		if (nd_atomic_read(&(node->used)) > 0) {
			--num;
			cb_entry(root,i+root->base_id, param); //unlock only read
		}
	}
}

void *_nd_node_search(struct node_root *root, NDUINT16 node_id)
{
	return _nd_node_trylock(root, node_id);
}


void _nd_node_init(void *socket_node, nd_handle h)
{
	nd_msgbox("please set initialization function of client manager", "error");
	nd_logerror("please set initialization function of client manager");
	return;
}

void* _nd_node_lock_first(struct node_root *root, node_iterator *it)
{
	//ndatomic_t v = 0 ;
	int i;
	struct srvnode_info *node;
	if (nd_atomic_read(&(root->connect_num)) < 1){
		return NULL;
	}

	node = (struct srvnode_info *) root->connmgr_addr;
	it->node_id = root->base_id;
	it->numbers = 0;
	it->total = (NDUINT16)root->connect_num;

	for (i = 0; i < root->max_conn_num; i++, node++, (it->node_id)++) {

		if (nd_atomic_read(&(node->used)) >0) {
			++(it->numbers);
			if (node->owner == nd_thread_self())	{
				return node->node_addr;
			}
		}
	}
	it->node_id = 0;
	return NULL;
}

void _nd_node_unlock_iterator(struct node_root *root, node_iterator *it)
{
	_nd_node_unlock(root, it->node_id);

	it->node_id = 0;
	it->numbers = 0;
	it->total = 0;
}

void* _nd_node_lock_next(struct node_root *root, node_iterator *it)
{
	//ndatomic_t v = 0 ;
	int i = (it->node_id) - root->base_id;

	nd_assert(i >= 0 && i < root->max_conn_num);

	_nd_node_unlock(root, it->node_id);
	++i;
	++(it->node_id);

	for (; i < root->max_conn_num && it->numbers < it->total; i++, (it->node_id)++) {
		struct srvnode_info *node = (struct srvnode_info *) root->connmgr_addr;
		node += i;

		if (nd_atomic_read(&(node->used)) > 0) {
			++(it->numbers);
			if (node->owner == nd_thread_self())	{
				return node->node_addr;
			}
		}
	}
	it->node_id = 0;
	return NULL;
}

int _nd_node_free_num(struct node_root *root)
{
	return nd_sa_freenum(root->node_alloctor);
}
int _nd_node_capacity(struct node_root *root)
{
	return nd_sa_capacity(root->node_alloctor);

}

void _nd_node_set_owner(struct node_root *root, NDUINT16 node_id, ndthread_t owner)
{
	struct srvnode_info *node = (struct srvnode_info *)root->connmgr_addr;
	int index = node_id - root->base_id;

	if (index < 0 || index >= root->max_conn_num)
		return;

	node[index].owner = owner;
}

ndthread_t  _nd_node_get_owner(struct node_root *root, NDUINT16 node_id)
{
	struct srvnode_info *node = (struct srvnode_info *)root->connmgr_addr;
	int index = node_id - root->base_id;

	if (index < 0 || index >= root->max_conn_num)
		return 0;

	return node[index].owner;
}


//////////////////////////////////////////////////////////////////////////

int nd_node_preinit(struct node_root *root, node_create_func creator, node_destroy_func destroy)
{
	memset(root, 0, sizeof(*root));
	root->root_creator = creator;
	root->root_destroy = destroy;
	return 0;
}


int nd_node_create_ex(struct node_root *root, int max_num, size_t node_size,NDUINT16 start_id,nd_handle mmpool) 
{
	if (root->root_creator){
		return root->root_creator(root, max_num, node_size, start_id, mmpool);
	}
	else {
		return nd_node_mgr_create(root, max_num, node_size, start_id, mmpool);
	}
}

void nd_node_destroy_ex(struct node_root *root)
{
	if(root->root_destroy) {
		root->root_destroy(root) ;
	}
	else {
		nd_node_mgr_destroy(root);
	}
}



void nd_node_set_allocator(struct node_root *root,nd_handle allocator,node_alloc alloc,node_dealloc dealloc)
{
	root->node_alloctor = allocator;
	root->alloc = alloc ;
	root->dealloc = dealloc ;
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

