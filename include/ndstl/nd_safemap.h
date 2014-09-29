/* file nd_safemap.h
 *
 * simple map with thread safe
 *
 * create by duan 
 * 2012/4/16 11:25:37
 */

#ifndef _ND_SAFEMAP_H_
#define _ND_SAFEMAP_H_

#include "nd_common/nd_common.h"
#include "nd_allocator.h"
#include "ndstl/nd_rbtree.h"

#ifdef ND_MULTI_THREAD_GAME
typedef nd_rbnode_locker savemap_locker_t ;
#else 
typedef nd_rbnode_empty_locker savemap_locker_t ;
#endif


template<class _Tfirst, class _Ttype  >
class nd_safemap : public nd_refcount_rbtree<_Tfirst,_Ttype,nd_rbnode_empty_cleanup<_Ttype>,savemap_locker_t>
{
public:
	typedef nd_refcount_rbtree<_Tfirst,_Ttype,nd_rbnode_empty_cleanup<_Ttype>,savemap_locker_t> _MyBase ;
	typedef nd_safemap<_Tfirst,_Ttype> _Myt ;

	typedef nd_pair<typename _MyBase::iterator, bool> _Pairib;
	//typedef typename _MyBase::iterator iterator ;
	nd_safemap(nd_handle pool):_MyBase(pool) 
	{

	}
	nd_safemap():_MyBase(0) 
	{

	}
	virtual ~nd_safemap() 
	{
		_Myt::clear() ;
	}
	nd_safemap (const _Myt&r):_MyBase(r)
	{
	}
#ifdef _MSC_VER
	template<class _Valty>
	_Pairib insert(_Valty&& _Val)
	{
		ND_TRACE_FUNC() ;
		_Pairib ret ;
		ret.first = _MyBase::insert(_Val.first,_Val.second);
		if (ret.first != end() ){
			ret.second = true ;
		}
		else {
			ret.second = false ;
		}
		return ret ;
	}
#else 
	template<class _Valty>
	_Pairib insert(const _Valty& _Val)
	{
		ND_TRACE_FUNC() ;
		_Pairib ret ;
		ret.first = _MyBase::insert(_Val.first,_Val.second);
		if (ret.first != _MyBase::end() ){
			ret.second = true ;
		}
		else {
			ret.second = false ;
		}
		return ret ;
	}
#endif

	_Ttype& operator[](const _Tfirst& _Keyval)
	{	
		ND_TRACE_FUNC() ;
		typename _MyBase::iterator _Where = _MyBase::search(_Keyval) ;
		if (_Where == _MyBase::end())
			_Where = _MyBase::insert(_Keyval, _Ttype());
		return ((*_Where).second);
	}
};

#if 0
template<class _Tfirst, class _Ttype  >
class nd_safemap 
{
public:
	typedef _Tfirst				comp_type ;
	typedef _Tfirst				value_key ;
	typedef _Ttype				value_type;	
	typedef value_type*			pointer;
	typedef const value_type*	const_pointer;	
	typedef value_type&			reference;	
	typedef const value_type&	const_reference;	
	typedef size_t				size_type;	
	typedef ptrdiff_t			difference_type;
	typedef nd_safemap<comp_type, value_type> _MyT ;
	struct rbtree_node {
		rb_node hdr ;
		ndatomic_t is_deleted ;
		ndatomic_t ref_count ;
		struct rbtree_node*next ;//delete list
		comp_type first ;
		value_type second;
	};
	typedef rbtree_node			value_node ;
	typedef value_node			rbtree_value ;
	typedef nd_poolalloc<rbtree_node> _TAlloc ;
	class base_iterator
	{
	public:
		base_iterator() {treenode=0;parent=0;}
		virtual ~base_iterator() 
		{ 
			if (parent)	{
				if (treenode){
					parent->dec_ref(treenode)	;
				}
				parent->dec_self_ref() ;
			}
		}
		base_iterator(const base_iterator &r)
		{
			treenode = r.treenode;
			parent = r.parent ;

			if (parent &&treenode){
				parent->inc_ref(treenode);
			}
			if (parent)	{
				parent->inc_self_ref() ;
			}	
		}
#ifdef _MSC_VER
		base_iterator(base_iterator &&r)
		{
			treenode = r.treenode;
			parent = r.parent ;

			if (parent &&treenode){
				parent->inc_ref(treenode);
			}
			if (parent)	{
				parent->inc_self_ref() ;
			}	
		}
#else 
		base_iterator(base_iterator &r)
		{
			treenode = r.treenode;
			parent = r.parent ;

			if (parent &&treenode){
				parent->inc_ref(treenode);
			}
			if (parent)	{
				parent->inc_self_ref() ;
			}	
		}
#endif
		base_iterator(value_node *it,_MyT *pp):treenode(it),parent(pp)
		{
			if (parent)	{
				parent->inc_self_ref() ;
			}
			if (treenode&&parent) {
				parent->inc_ref(treenode);
			}
		}
		base_iterator& operator=(const base_iterator &r)
		{
			ND_TRACE_FUNC() ;
			if (parent)	{
				if (treenode){
					parent->dec_ref(treenode)	;
				}
				parent->dec_self_ref() ;
			}
			treenode = r.treenode;
			parent = r.parent ;

			if (parent &&treenode){
				parent->inc_ref(treenode);
			}
			if (parent)	{
				parent->inc_self_ref() ;
			}
			return *this;
		}		
		bool operator==(const base_iterator& _Right) const{ return treenode == _Right.treenode;	}
		bool operator!=(const base_iterator& _Right) const{	return (!(treenode == _Right.treenode));}
		value_node& operator*() const{return *treenode ;}
		value_node* operator->() const{return treenode ;}
		value_node *treenode ;
		_MyT *parent;
	};
	class rbtree_terator :public base_iterator
	{
	public:
		rbtree_terator():base_iterator() {}
		rbtree_terator(value_node *it,_MyT *pp):base_iterator(it,pp) {}
		rbtree_terator& operator++()
		{
			ND_TRACE_FUNC() ;
			if (!base_iterator::treenode){
				return *this ;
			}
			base_iterator::parent->dec_ref(base_iterator::treenode) ;
			base_iterator::parent->__lock() ;
			rb_node  *node = &base_iterator::treenode->hdr ;
			base_iterator::treenode = NULL ;
			while ((node=rb_next(node)) != NULL) 	{
				base_iterator::treenode = rb_entry(node,struct rbtree_node, hdr) ;
				if (base_iterator::parent->inc_ref(base_iterator::treenode))	{
					break ;
				}
				else {
					base_iterator::treenode = NULL;
				}
			} ;	
			base_iterator::parent->__unlock() ;
			return *this ;
		}
		rbtree_terator operator++(int)
		{
			rbtree_terator _Tmp = *this;
			++*this;
			return (_Tmp);
		}
	};
	class rbtree_reverse_iterator :public base_iterator
	{
	public:
		rbtree_reverse_iterator():base_iterator() {}
		rbtree_reverse_iterator(value_node *it,_MyT *pp):base_iterator(it,pp) {}
		rbtree_reverse_iterator& operator++()
		{
			ND_TRACE_FUNC() ;
			if (!base_iterator::treenode){
				return *this ;
			}
			base_iterator::parent->dec_ref(base_iterator::treenode) ;

			base_iterator::parent->__lock() ;
			rb_node  *node = &base_iterator::treenode->hdr ;
			base_iterator::treenode = NULL ;
			while ((node=rb_prev(node)) != NULL) 	{
				base_iterator::treenode = rb_entry(node,struct rbtree_node, hdr) ;
				if (base_iterator::parent->inc_ref(base_iterator::treenode))	{
					break ;
				}
				else {
					base_iterator::treenode = NULL;
				}
			} ;	
			base_iterator::parent->__unlock() ;
			return *this ;
		}
		rbtree_reverse_iterator operator++(int)
		{
			ND_TRACE_FUNC() ;
			rbtree_reverse_iterator _Tmp = *this;
			++*this;
			return (_Tmp);
		}
	};
	typedef rbtree_terator iterator ;
	typedef rbtree_reverse_iterator reverse_iterator;

	typedef rbtree_terator const_iterator ;
	typedef rbtree_reverse_iterator const_reverse_iterator;

	iterator search(comp_type &key)
	{
		ND_TRACE_FUNC() ;
		value_node* ptreenode = _search(key) ;
		if (!ptreenode)	{
			return iterator(NULL,NULL);
		}
		if (nd_atomic_read(&ptreenode->ref_count)==0 || nd_atomic_read(&ptreenode->is_deleted)){
			return iterator(0,NULL) ;
		}
		return iterator(ptreenode,this) ;
	}

	iterator find(comp_type &key){return search(key);}
	iterator find(const comp_type &key){return search((comp_type &)key);}

	iterator insert(const comp_type &ct,const value_type &vt)
	{
		ND_TRACE_FUNC() ;
		rbtree_node *ins_node = _create_obj() ;
		if (!ins_node){
			return iterator(NULL,NULL);
		}
		ins_node->first = ct; 
		ins_node->second = vt ;
		if(insert( ins_node ) ) {
			return iterator(ins_node,this) ;
		}
		_destroy_obj(ins_node) ;
		return iterator(NULL,NULL);
	}
	bool insert( value_node *data)
	{
		ND_TRACE_FUNC() ;
		LockHelper __tmplock(&m_mutex) ;

		struct rb_root *root = &m_header;
		struct rb_node **new_node = &(root->rb_node), *parent = NULL;

		//Figure out where to put new node 
		while (*new_node) {
			struct rbtree_node *cur_node = rb_entry(*new_node, rbtree_node, hdr);			
			parent = *new_node;
			if (data->first < cur_node->first)
				new_node = &((*new_node)->rb_left);
			else if (data->first > cur_node->first)
				new_node = &((*new_node)->rb_right);
			else
				return false;
		}
		++m_count;
		nd_atomic_set(&data->ref_count,1) ;
		nd_atomic_set(&data->is_deleted,0);

		//Add new node and rebalance tree. 
		rb_link_node(&data->hdr, parent, new_node);
		rb_insert_color(&data->hdr, root);
		return true;
	}

	bool erase(comp_type &key)
	{
		ND_TRACE_FUNC() ;
		value_node* ptreenode = _search(key) ;
		if (ptreenode) {
			return _set_erase_node(ptreenode) ;
		}
		return false;
	}

	bool erase(const comp_type &key)
	{
		return erase((comp_type &)key);
	}

	iterator erase(iterator &erase_it)
	{
		ND_TRACE_FUNC() ;
		__lock();
		rb_node *next_node = rb_next(&(erase_it->hdr)) ;
		__unlock();
		_set_erase_node(&*erase_it) ;

		if (next_node){
			return iterator(rb_entry(next_node, struct rbtree_node, hdr),this);
		}
		return iterator(NULL,NULL);
	}
	reverse_iterator erase(reverse_iterator &erase_it)
	{
		ND_TRACE_FUNC() ;
		__lock();
		rb_node *next_node = rb_next(&(erase_it->hdr)) ;
		__unlock();
		_set_erase_node(&*erase_it) ;

		if (next_node){
			return reverse_iterator(rb_entry(next_node, struct rbtree_node, hdr),this);
		}
		return reverse_iterator(NULL,NULL);
	}

	iterator erase(const iterator &erase_it)
	{
		ND_TRACE_FUNC() ;
		__lock();
		rb_node *next_node = rb_next(&(erase_it->hdr)) ;
		__unlock();
		_set_erase_node(&*erase_it) ;

		if (next_node){
			return iterator(rb_entry(next_node, struct rbtree_node, hdr),this);
		}
		return iterator(NULL,NULL);
	}
	reverse_iterator erase(const reverse_iterator &erase_it)
	{
		ND_TRACE_FUNC() ;
		__lock();
		rb_node *next_node = rb_next(&(erase_it->hdr)) ;
		__unlock();
		_set_erase_node(&*erase_it) ;

		if (next_node){
			return reverse_iterator(rb_entry(next_node, struct rbtree_node, hdr),this);
		}
		return reverse_iterator(NULL,NULL);
	}
	void clear() 
	{
		ND_TRACE_FUNC() ;
		LockHelper __tmplock(&m_mutex) ;

		rb_node *cur_node = rb_first(&m_header) ;

		while ( cur_node) {
			value_node *del_data = rb_entry(cur_node, struct rbtree_node, hdr) ;
			cur_node = rb_next(cur_node) ;
			nd_atomic_set(&del_data->ref_count,0) ;
			nd_atomic_set(&del_data->is_deleted,1);
			_erase(del_data) ;
		} 
		m_del_list = NULL;
	}

	iterator begin() 
	{
		ND_TRACE_FUNC() ;
		LockHelper __tmplock(&m_mutex) ;

		struct rbtree_node *treedata = NULL ;
		rb_node *node = rb_first(&m_header) ;		
		while (node) 	{
			treedata = rb_entry(node,struct rbtree_node, hdr) ;
			if (inc_ref(treedata))	{
				break ;
			}
			else {
				treedata = NULL;
				node=rb_next(node);
			}
		} 
		if (treedata && dec_ref(treedata))
			return iterator(treedata,this) ;
		else 
			return iterator(NULL,NULL) ;
	}

	iterator end() { return iterator(NULL,NULL);}
	reverse_iterator rbegin() 
	{
		ND_TRACE_FUNC() ;
		LockHelper __tmplock(&m_mutex) ;

		struct rbtree_node *treedata = NULL ;
		rb_node *node = rb_last(&m_header) ;
		while (node) 	{
			treedata = rb_entry(node,struct rbtree_node, hdr) ;
			if (inc_ref(treedata))	{
				break ;
			}
			else {
				treedata = NULL;
				node=rb_prev(node);
			}
		} 
		if (treedata && dec_ref(treedata))
			return reverse_iterator(treedata,this) ;
		else 
			return reverse_iterator(NULL,NULL) ;		
	}

	reverse_iterator rend() { return reverse_iterator(NULL,this);}
	int size(){		return m_count;	}

	void initilize_node(rbtree_value &data, comp_type *key=NULL,value_type *val=NULL )
	{
		ND_TRACE_FUNC() ;
		rb_init_node(&data.hdr);
		data.ref_count = 0 ;
		data.is_deleted = 0 ;
		data.next = 0 ;
		if (key){
			data.first = *key ;
		}
		if (val){
			data.second = *val;
		}
	}
	value_node *construct() {return _create_obj();}
	void destruct(value_node *addr) {_destroy_obj(addr);} 

	void _set_pool(nd_handle pool) {m_alloc._set_pool(pool);}
	bool check_erased(value_node *ptreenode)
	{
		if (nd_atomic_read(&ptreenode->ref_count)==0 || nd_atomic_read(&ptreenode->is_deleted)){
			return true ;
		}
		return false;
	}
	value_node *ptr2treenode(pointer ptr)
	{
		return rb_entry(ptr, struct rbtree_node, second) ;
	}
	bool _set_erase_node(value_node *del_data)
	{
		ND_TRACE_FUNC() ;
		if(0==nd_testandset(&(del_data->is_deleted)) ) {
			return dec_ref(del_data) ;
		}
		return false ;
	}
	void inc_self_ref() {nd_atomic_inc(&m_self_ref) ;}
	void dec_self_ref() 
	{
		ND_TRACE_FUNC() ;
		ndatomic_t tmp ;
		while((tmp = nd_atomic_read(&m_self_ref)) > 0) {
			if(nd_compare_swap(&m_self_ref,tmp,tmp-1)) {
				if (tmp==1) {
					break ;
				}
				return  ;
			}
		}
		tryto_erase() ;
	}

	_Ttype& operator[](const _Tfirst& _Keyval)
	{	
		ND_TRACE_FUNC() ;
		iterator _Where = _MyBase::search(_Keyval) ;
		if (_Where == _MyBase::end())
			_Where = _MyBase::insert(_Keyval, _Ttype());
		return ((*_Where).second);
	}

	nd_safemap(nd_handle pool):m_alloc(pool),m_self_ref(0) 
	{
		m_count = 0; m_header.rb_node = 0 ; m_del_list=0;
		nd_mutex_init(&m_mutex) ;
	}
	nd_safemap():m_self_ref(0) 
	{
		m_count = 0; m_header.rb_node = 0 ; m_del_list=0;
		nd_mutex_init(&m_mutex) ;
	}
	virtual ~nd_safemap() { nd_mutex_destroy(&m_mutex) ;}
private:
	nd_safemap(_MyT &_rt):m_alloc((nd_handle)-1),m_self_ref(0) {}

	_MyT& operator=(_MyT &r){return *this;}
	//_MyT& operator=(const _MyT &r){return *this;}

protected:

	value_node* _search(comp_type &key)
	{
		ND_TRACE_FUNC() ;
		LockHelper __tmplock(&m_mutex) ;
		struct rb_node *node = m_header.rb_node;
		while (node) {
			comp_type &data = ((rbtree_node*)node)->first ;
			if (key < data)
				node = node->rb_left;
			else if (key > data)
				node = node->rb_right;
			else
				return rb_entry(node, struct rbtree_node, hdr);
		}
		return (NULL);
	}

	void _erase( value_node *del_data)
	{
		ND_TRACE_FUNC() ;
		nd_assert(nd_atomic_read(&del_data->ref_count) == 0) ;
		rb_erase(&del_data->hdr,&m_header ) ;
		_destroy_obj(del_data) ;
		--m_count;
	}

	bool inc_ref(value_node* cur_node) 
	{
		ND_TRACE_FUNC() ;
		if (nd_atomic_read(&cur_node->ref_count)==0 || nd_atomic_read(&cur_node->is_deleted)){
			return false ;
		}
		nd_atomic_inc(&cur_node->ref_count) ;
		return true ;
	}
	bool dec_ref(value_node* cur_node) 
	{
		ND_TRACE_FUNC() ;
		bool ret = false ;
		ndatomic_t tmp ;

		while((tmp = nd_atomic_read(&cur_node->ref_count)) > 0) {
			if(nd_compare_swap(&cur_node->ref_count,tmp,tmp-1)) {				
				if (tmp==1) {
					ret = true ;
					addto_erase_list(cur_node) ;
					break ;
				}
				return true ;
			}
		}		
		return ret ;
	}
	rbtree_node* _create_obj() 
	{
		ND_TRACE_FUNC() ;
		//rbtree_node *ins_node =(rbtree_node *) nd_pool_alloc(m_mmpool,sizeof(rbtree_node)) ;
		rbtree_node *ins_node = m_alloc.allocate(1) ;
		if (!ins_node){
			return NULL;
		}
		ins_node = new(ins_node) rbtree_node ;
		rb_init_node(&ins_node->hdr);
		ins_node->ref_count = 0 ;
		ins_node->is_deleted = 0 ;
		ins_node->next = NULL;
		return ins_node;
	}
	void _destroy_obj(rbtree_node *node) 
	{
		ND_TRACE_FUNC() ;
		node->~rbtree_node() ;
		//nd_pool_free(m_mmpool,node) ;
		m_alloc.deallocate(node,1);
	}

	
	void tryto_erase() 
	{
		ND_TRACE_FUNC() ;
		LockHelper __tmplock(&m_mutex) ;

		struct rbtree_node *deletlist = m_del_list,*pnext ;
		while (deletlist){
			pnext = deletlist->next ;
			nd_atomic_set(&deletlist->ref_count,0) ;
			nd_atomic_set(&deletlist->is_deleted,1) ;
			_erase(deletlist) ;
			deletlist = pnext ;
		}
		m_del_list = NULL;
	}
	void addto_erase_list( value_node *del_data)
	{
		ND_TRACE_FUNC() ;
		__lock() ;
		del_data->next = m_del_list ;
		m_del_list = del_data ;
		__unlock();
		if (nd_atomic_read(&m_self_ref)==0) {
			tryto_erase() ;
		}
	}
	void __lock() { nd_mutex_lock(&m_mutex);}
	void __unlock() { nd_mutex_unlock(&m_mutex);}
	friend class rbtree_terator ;
	friend class rbtree_reverse_iterator ;
	ndatomic_t m_self_ref ; //被引用次数
	int m_count;
	rb_root m_header;
	//nd_handle m_mmpool;
	nd_mutex m_mutex;
	struct rbtree_node* m_del_list ;//delete list
	_TAlloc m_alloc ;
};
#endif 

#endif
