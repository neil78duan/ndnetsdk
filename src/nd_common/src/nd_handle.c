/* file nd_handle.c
 * define object handle of nd engine 
 * 
 * all right reserved by neil duan 
 * 2008-12-25 
 * version 1.0
 */

#include "nd_common/nd_common.h"
#include "nd_common/nd_handle.h"
#include "nd_common/nd_alloc.h"

#define ALLOC_MARK 0x7f3e 

static ND_LIST_HEAD(__reg_list) ;

static ND_LIST_HEAD(__alloced_obj_list) ;

struct register_object_info
{
	struct list_head list ;
	struct nd_handle_reginfo obj ;

};

struct alloced_objects 
{
	struct list_head list ;
	struct tag_nd_handle alloced_obj ;
};
static nd_mutex __alloced_lock ;
static int __inited ;


nd_handle _object_create(const char *name)
{
	struct register_object_info *objlist ;
	struct nd_handle_reginfo *reginfo = 0  ;
	struct list_head *pos = __reg_list.next ;
	
	nd_logmsg(" start create %s object \n", name) ;
	
	if(0==__inited) {
		if(-1==nd_mutex_init(&__alloced_lock) ) {
			
			nd_logmsg(" nd_mutex_init error :%s \n", nd_last_error()) ;
		}
		__inited = 1 ;
	}
	while (pos != &__reg_list) {
		objlist = list_entry(pos, struct register_object_info, list) ;		
		if(0==ndstrcmp(objlist->obj.name,name)) {
			reginfo = &(objlist->obj) ;
			break ;
		}
		pos = pos->next ;
	}

	if(reginfo) {
		size_t alloc_size = reginfo->object_size + sizeof(struct list_head) ;
		struct alloced_objects *ao =(struct alloced_objects *) malloc(alloc_size) ;
		if(ao) {
			
			INIT_LIST_HEAD(&ao->list) ;				
			
			nd_mutex_lock(&__alloced_lock) ;
			list_add_tail(&ao->list, &__alloced_obj_list) ;
			nd_mutex_unlock(&__alloced_lock) ;

			memset(&ao->alloced_obj, 0, reginfo->object_size) ;
			if(reginfo->init_entry) {
				reginfo->init_entry(&ao->alloced_obj) ;
			}
			ao->alloced_obj.size = reginfo->object_size ;
			ao->alloced_obj.close_entry = reginfo->close_entry ;
			
			INIT_LIST_HEAD(&ao->alloced_obj.__release_cb_hdr) ;
			
			nd_reg_handle((nd_handle)&ao->alloced_obj) ;
			nd_object_set_instname((nd_handle)&(ao->alloced_obj), name);
			
			nd_logmsg(" create %s object success ! \n", name) ;
			return &(ao->alloced_obj) ;
		}
		else {
			nd_logerror("malloc error: %s\n", nd_last_error()) ;;
		}
	}
	
	else {
		nd_logerror("not found object %s \n", name) ;
	}
	return NULL ;
}


int _object_destroy(nd_handle handle, int flag) 
{
	int ret = -1;
	int is_free = 0 ;
	struct list_head  *pos ;
	struct list_head  *free_addr =(struct list_head  *) handle ;
	
	if (!handle){
		return -1;
	}
	--free_addr ;
	//try to free !
	nd_mutex_lock(&__alloced_lock) ;
	pos = __alloced_obj_list.next ;
	while(pos != &__alloced_obj_list) {
		if(pos == free_addr) {
			is_free = 1 ;
			list_del(pos) ;
			break ;
		}
		pos = pos->next ; 

	}
	nd_mutex_unlock(&__alloced_lock) ;

	nd_unreg_handle(handle) ;
	if(handle->close_entry) {		
		ret = handle->close_entry(handle, flag) ;		
	}
	
	_nd_object_on_destroy(handle,-1) ;
	if(is_free) {
		free(free_addr) ;
	}
	return ret ;
}


int nd_object_register(struct nd_handle_reginfo *reginfo) 
{
	struct register_object_info *pobj;
	nd_assert(reginfo) ;
	
	pobj = (struct register_object_info *)malloc(sizeof(*pobj)) ;
	if(!pobj) {
		nd_logerror("nd_object_register: malloc error %s\n", nd_last_error()) ;
		return -1 ;
	}

	INIT_LIST_HEAD(&pobj->list) ;
	memcpy(&pobj->obj, reginfo, sizeof(*reginfo)) ;
	pobj->obj.name[OBJECT_NAME_SIZE-1] = 0 ;

	list_add_tail(&pobj->list, &__reg_list) ;
	
	//nd_logmsg(" register object success : %s\n", pobj->obj.name) ;
	return 0 ;
}

int destroy_object_register_manager(void) 
{
	struct register_object_info *objlist ;
	//struct nd_handle_reginfo *reginfo = 0  ;
	struct list_head *pos = __reg_list.next ;

	while (pos != &__reg_list) {
		objlist = list_entry(pos, struct register_object_info, list) ;
		pos = pos->next ;
		list_del_init(&objlist->list);
		free(objlist) ;
	}
	return 0 ;
}

//////////////

int nd_object_add_destroy_cb(nd_handle handle,nd_object_destroy_callback callback, void *param,int type) 
{
	struct release_callback_source_node *node = malloc(sizeof(struct release_callback_source_node)) ;
	if(!node ) {
		return -1;
	}
	INIT_LIST_HEAD(&node->list) ;
	node->func = callback ;
	node->param = param ;
	node->type = type ;
	
	list_add(&node->list, &handle->__release_cb_hdr) ;
	return 0;
}
int nd_object_del_destroy_cb(nd_handle handle,nd_object_destroy_callback callback, void *param) 
{
	struct list_head *source_hdr = &handle->__release_cb_hdr ;
	struct release_callback_source_node *node,*next ;
	if (list_empty(source_hdr)) {
		return -1;
	}
	list_for_each_entry_safe(node, next, source_hdr, struct release_callback_source_node, list) {
		if (node->func == callback && node->param ==param) {
			
			node->func(handle, node->param) ;
			list_del_init(&node->list) ;
			free(node) ;
			break ;
		}
	}
	
	return  0;
	
}
//call when destory or something 
int _nd_object_on_destroy(nd_handle handle,int type) 
{
	ENTER_FUNC() ;
	struct list_head *source_hdr = &handle->__release_cb_hdr ;
	struct release_callback_source_node *node,*next ;
	if (list_empty(source_hdr)) {
		LEAVE_FUNC();
		return 0;
	}
	list_for_each_entry_safe(node, next, source_hdr, struct release_callback_source_node, list) {
		if (-1==type || node->type== type) {
			node->func(handle, node->param) ;
			list_del_init(&node->list) ;
			free(node) ;
		}
	}
	
	INIT_LIST_HEAD(&handle->__release_cb_hdr) ;
	LEAVE_FUNC() ;
	return 0;
}

///////////////////////////


const char *nd_object_errordesc(nd_handle h)
{
	return nd_error_desc(h->myerrno);
}

int nd_object_get_type(nd_handle h)
{
	return (h->type);
}

int nd_object_check_error(nd_handle h) 
{
	if (h->myerrno ==NDERR_SUCCESS ||
		h->myerrno == NDERR_WOULD_BLOCK ||
		h->myerrno==NDERR_NO_PRIVILAGE ||
		h->myerrno >= NDERR_USER_BREAK)	{
		return 0 ;
	}
	return h->myerrno ;
}

int nd_tryto_clear_err(nd_handle h)
{
//	int ret = h->myerrno ;

	if (h->myerrno ==NDERR_INVALID_HANDLE || 
		h->myerrno==NDERR_TIMEOUT || 
		h->myerrno==NDERR_OPENFILE||
		h->myerrno==NDERR_USER||
		h->myerrno==NDERR_IO||
		h->myerrno==NDERR_RESET||
		h->myerrno==NDERR_WRITE||
		h->myerrno==NDERR_CLOSED||
		h->myerrno==NDERR_READ)	{
			return h->myerrno ;
	}
	else {
		h->myerrno = 0 ;
		return 0 ;
	}
	
}

#ifdef ND_SOURCE_TRACE

static struct nd_rb_root _g_rb_handler_root  ;
static ndatomic_t _g_is_init = 0;
static ndatomic_t _g_objindex = 1 ;
static nd_mutex _g_handle_lock ;
static ndatomic_t _g_handle_num ;

int _tryto_init_handle_mgr()
{
	if (nd_compare_swap(&_g_is_init, 0, 1)){
		nd_mutex_init(&_g_handle_lock) ;
		_g_rb_handler_root.rb_node = 0 ;
		_g_objindex = 1;
		_g_handle_num = 0 ;
	}
	return 0 ;
}

void _tryto_deinit_handle_mgr()
{
// 	if(nd_atomic_read(&_g_handle_num)== 0) {
// 		nd_mutex_destroy(&_g_handle_lock) ;
// 		_g_rb_handler_root.rb_node = 0 ;
// 		nd_atomic_set(&_g_objindex ,0) ;
// 	}
}

struct tag_nd_handle * _search_handle(NDUINT32 objid) 
{
	struct nd_rb_node *node ;

	if(nd_atomic_read(&_g_handle_num) < 1)
		return NULL ;

	nd_mutex_lock(&_g_handle_lock) ;
	node = _g_rb_handler_root.rb_node;
	while (node) {
		struct tag_nd_handle *data = rb_entry(node, struct tag_nd_handle, __self_rbnode);
		int result = (int) (objid - data->__objid) ;

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else {
			nd_mutex_unlock(&_g_handle_lock) ;
			return data;
		}
	}
	nd_mutex_unlock(&_g_handle_lock) ;
	return NULL;
}

int nd_reg_handle(nd_handle hobj) 
{
	struct tag_nd_handle *handle = (struct tag_nd_handle *) hobj ;
	nd_assert(hobj) ;
	if (!hobj){
		return -1;
	}
	_tryto_init_handle_mgr() ;

	nd_atomic_set(&handle->__created,1) ;
	rb_init_node(&handle->__self_rbnode);
	
	//insert 
	nd_mutex_lock(&_g_handle_lock) ;
	handle->__objid = nd_atomic_inc(&_g_objindex) ;
	do 	{
		struct nd_rb_root *root = &_g_rb_handler_root;
		struct nd_rb_node **new_node = &(root->rb_node), *parent = NULL;

		while (*new_node) {
			struct tag_nd_handle *cur_node = rb_entry(*new_node, struct tag_nd_handle, __self_rbnode);			
			parent = *new_node;
			if (handle->__objid < cur_node->__objid)
				new_node = &((*new_node)->rb_left);
			else if (handle->__objid > cur_node->__objid)
				new_node = &((*new_node)->rb_right);
			else {
				nd_mutex_unlock(&_g_handle_lock) ;
				return -1;
			}
		}
		//Add new node and rebalance tree. 
		rb_link_node(&handle->__self_rbnode, parent, new_node);
		rb_insert_color(&handle->__self_rbnode, root);
	}while(0) ;
	nd_atomic_inc(&_g_handle_num) ;
	nd_mutex_unlock(&_g_handle_lock) ;
	return 0;
}
int nd_unreg_handle(nd_handle hobj) 
{
	struct tag_nd_handle * p ;
	struct tag_nd_handle *handle = (struct tag_nd_handle *) hobj ;
	nd_assert(hobj) ;
	if (!hobj){
		return -1;
	}
	p = _search_handle(handle->__objid) ;
	if (!p) {
		return -1;
	}
	
	nd_atomic_set(&handle->__created,0) ;

	nd_mutex_lock(&_g_handle_lock) ;
	rb_erase(&p->__self_rbnode, &_g_rb_handler_root);
	nd_atomic_dec(&_g_handle_num);
	nd_mutex_unlock(&_g_handle_lock) ;

	rb_init_node(&handle->__self_rbnode);
	handle->__objid = 0 ;
	
	_tryto_deinit_handle_mgr() ;
	return 0;
}

int nd_handle_checkvalid(nd_handle hobj, NDUINT16 objtype)
{
	struct tag_nd_handle * p ;
	struct tag_nd_handle *handle = (struct tag_nd_handle *) hobj ;
	nd_assert(hobj) ;
	if (!hobj){
		return 0;
	}
	p = _search_handle(handle->__objid) ;
	if (!p) {
		return 0;
	}
	if(!nd_atomic_read(&p->__created) || 
		!p->__objid || 
		p->__objid> _g_objindex ||
		p->type != objtype) {
		return 0 ;
	}
	return 1;
}
#else 


int nd_reg_handle(nd_handle hobj) 
{
	nd_atomic_set(&((struct tag_nd_handle *)hobj)->__created,1) ;
	return 0 ;
}
int nd_unreg_handle(nd_handle hobj) 
{
	nd_atomic_set(&((struct tag_nd_handle *)hobj)->__created,0) ;
	return 0 ;
}

int nd_handle_checkvalid(nd_handle hobj, NDUINT16 objtype)
{
	if (nd_atomic_read(&((struct tag_nd_handle *)hobj)->__created) ==0 ){
		return 0 ;
	}
	else if (((struct tag_nd_handle *)hobj)->type ==0 || ((struct tag_nd_handle *)hobj)->type!=objtype ){
		return 0 ;
	}
	else if (((struct tag_nd_handle *)hobj)->size ==0 ){
		return 0 ;
	}
	return 1 ;
}
#endif
