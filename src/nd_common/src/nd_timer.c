/* file nd_timer.c
 * timer of nd engine 
 *
 * all right reserved 2009
 * create time 2009-2-24 14:38
 */

#include "nd_common/nd_common.h"
#include "nd_common/nd_timer.h"

#define DELBUF_SIZE 16
struct nd_timer_root
{
	ND_OBJ_BASE ;
	ndatomic_t	num ;
	ndatomic_t start_id ;
	nd_handle pallocator ;					//memory allocator 
	nd_mutex	list_lock ;
	struct list_head list ;					//node list
	ndatomic_t	del_buf[DELBUF_SIZE] ;
} ;


struct timer_node 
{	
	ndtimer_t timer_id ;
	int run_type ;
	void *param ;
	nd_timer_entry entry;
	ndtime_t last_time ;
	ndtime_t interval ;
	
	struct list_head list ;
};

static void tryto_del_timer(struct nd_timer_root *root) ;
/* */
ndtimer_t nd_timer_add(nd_handle handle,nd_timer_entry func,void *param,ndtime_t interval, int run_type )
{
	struct nd_timer_root * root ;
	struct timer_node *node ;
	nd_assert(handle) ;
	root = (struct nd_timer_root*)handle ;
	nd_assert(root->pallocator) ;
	if(!func)
		return 0 ;

	node = nd_pool_alloc(root->pallocator, sizeof(*node)) ;
	if(!node) {
		return 0 ;
	}
	node->timer_id = nd_atomic_inc(&root->start_id) ;
	node->run_type = run_type ;
	node->param = param ;
	node->entry = func ;
	node->interval = interval ;
	node->last_time = nd_time() ;
	
	INIT_LIST_HEAD(&node->list) ;
	

	nd_mutex_lock(&root->list_lock) ;
		list_add_tail(&node->list, &root->list) ;
	nd_mutex_unlock(&root->list_lock) ;
	nd_atomic_inc(&root->num) ;
	return node->timer_id;
}

int nd_timer_del(nd_handle handle, ndtimer_t id) 
{
	int i ,ret=-1;
	struct nd_timer_root *root ;

	if(!handle)
		return -1 ;
	if(0==id)
		return -1 ;
	root = (struct nd_timer_root *)handle ;
	
	for (i=0; i<DELBUF_SIZE; i++) {
		ret = nd_compare_swap(root->del_buf+i,0, id) ;
		if(0==ret)
			break ;
	}
	return (ret==0)? 0 : -1 ;

}

/* */
int  nd_timer_destroy(nd_handle timer_handle, int force) 
{
	struct timer_node *node ;
	struct nd_timer_root *root ;
	struct list_head *pos , *next,header;
	if(!timer_handle) {
		return -1 ;
	}
	root =(struct nd_timer_root *) timer_handle ;
	if(!root->pallocator|| root->type !=NDHANDLE_TIMER || root->size != sizeof(*root)) {
		return -1 ;
	}

	INIT_LIST_HEAD(&header) ;
	nd_mutex_lock(&root->list_lock) ;
		list_add(&header, &(root->list)) ;
		list_del_init(&root->list) ;
	nd_mutex_unlock(&root->list_lock) ;

	list_for_each_safe(pos,next,&header) {
		node = list_entry(pos, struct timer_node, list) ;
		if (node->run_type==ETT_DESTROY){
			node->entry(node->param) ;
		}
		list_del(pos) ;
		nd_pool_free(root->pallocator,node) ;
	}
	nd_mutex_destroy(&root->list_lock) ;
	//release root node 
	nd_pool_free(root->pallocator,root) ;
	return 0 ;
}

/* create timer root */
nd_handle nd_timer_create(nd_handle pallocator) 
{
	//static ndatomic_t _s_id = 0 ;
	struct nd_timer_root *root ;
	if(!pallocator) {
		pallocator = nd_global_mmpool() ;

	}
	nd_assert(pallocator) ;

	root = nd_pool_alloc(pallocator, sizeof(*root)) ;
	if(!root) {
		return NULL ;
	}

	root->size = sizeof(*root) ;					
	root->type = NDHANDLE_TIMER ;					
	root->myerrno = 0 ;
	root->close_entry =(nd_close_callback )nd_timer_destroy;
	root->pallocator = pallocator;					//memory allocator 
	root->start_id = 0 ;
	root->num = 0 ;
	nd_mutex_init(&root->list_lock) ;
	INIT_LIST_HEAD(&root->list) ;					//node list
	bzero((void*)root->del_buf, sizeof(root->del_buf)) ;
	return (nd_handle) root ;
}

/* run timer function*/
int  nd_timer_update(nd_handle handle) 
{
	int ret = 0 ,run_ret =0 ;
	ndtime_t now_tm ;
	struct list_head *pos,*next, header ;
	struct timer_node *node ;
	struct nd_timer_root *root = (struct nd_timer_root *)handle ;
	
	if(nd_atomic_read(&root->num) < 1 ) {
		return 0 ;
	}
	now_tm = nd_time() ;

	tryto_del_timer(root) ;

	INIT_LIST_HEAD(&header) ;

	nd_mutex_lock(&root->list_lock) ;
		list_add(&header, &(root->list)) ;
		list_del_init(&root->list) ;
	nd_mutex_unlock(&root->list_lock) ;

	list_for_each_safe(pos,next,&header) {
		node = list_entry(pos, struct timer_node, list) ;
		if (node->run_type==ETT_DESTROY){
			continue;
		}
		if((now_tm - node->last_time) >= node->interval || node->interval < 10) {
			++ret ;
			run_ret = node->entry(node->param) ;
			if(node->run_type==ETT_ONCE || -1==run_ret) {
				list_del(pos) ;
				nd_pool_free(root->pallocator,node) ;
			}
			else
				node->last_time = now_tm ;
		}
	}
	
	nd_mutex_lock(&root->list_lock) ;
		list_join(&header, &root->list) ;
	nd_mutex_unlock(&root->list_lock) ;
	
	return ret ;
}

static void _del_from_list(struct nd_timer_root *root, ndtimer_t id) 
{
	struct list_head *pos ;
	struct timer_node *node =0;
	nd_assert(root) ;

	nd_mutex_lock(&root->list_lock) ;
		list_for_each(pos, &root->list){
			node = list_entry(pos, struct timer_node, list) ;
			if(node->timer_id == id) {
				list_del(pos) ;
				break ;
			}
			else 
				node = 0 ;
		}
	nd_mutex_unlock(&root->list_lock) ;

	if(node) {
		nd_atomic_dec(&root->num) ;
		nd_pool_free(root->pallocator,node) ;
	}

}

void tryto_del_timer(struct nd_timer_root *root)
{
	int i ;
	ndtimer_t id ;

	for (i=0; i<DELBUF_SIZE; i++) {
		id = nd_atomic_swap(root->del_buf+i,0) ;
		if(id) {
			_del_from_list(root, id) ;
		}
	}
}
