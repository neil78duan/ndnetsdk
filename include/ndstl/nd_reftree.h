/* file : nd_reftree.h
 *
 * RB tree with reference count
 *
 * create by duan 
 *
 * 2013/2/7 11:21:46 
 *
 */

#ifndef _ND_REFTREE_H_
#define _ND_REFTREE_H_

#include "nd_common/nd_common.h"
#include "nd_allocator.h"
#include "nd_utility.h"

//////////////////////////////////////////////////////////////////////////
//rb-tree with reference count

template<class _Ty>
struct nd_rbnode_empty_cleanup
{	
	void operator()( _Ty& node) 
	{	
		
	}
};

struct nd_rbnode_empty_locker
{
	void lock() {}
	void unlock(){}
};
struct nd_rbnode_locker
{
	void lock() {	nd_mutex_lock(&m_mutex);}
	void unlock() {	nd_mutex_unlock(&m_mutex);	}
	nd_rbnode_locker() { nd_mutex_init(&m_mutex);}
	~nd_rbnode_locker() { nd_mutex_destroy(&m_mutex);}
private:
	nd_mutex m_mutex;
};

template<class _TLocker>
class _rbnode_lock_helper
{
public:
	_rbnode_lock_helper(_TLocker &locker) : m_locker(locker)
	{
		m_locker.lock() ;
	}
	~_rbnode_lock_helper() 
	{
		m_locker.unlock() ;
	}
private:
	_TLocker &m_locker ; 
};



template<class _Tfirst, class _Tsecond>
struct ref_tree_val : public nd_pair<_Tfirst,  _Tsecond>
{
	typedef nd_pair<_Tfirst,  _Tsecond> _MyBase ;
	typedef ref_tree_val<_Tfirst,  _Tsecond> _MyT ;
	ref_tree_val() :_MyBase() {}
	ref_tree_val(const _Tfirst& f, const _Tsecond& s) : _MyBase(f,s) {} 
	ref_tree_val(const _MyBase &_Right) : _MyBase(_Right) {} 	
	bool operator > (const _MyT& _Right) const 
	{
		return this->first > _Right.first ;
	}
	bool operator < (const _MyT& _Right)const 
	{
		return this->first < _Right.first ;
	}
	bool operator == (const _MyT& _Right)const 
	{
		return this->first == _Right.first ;
	}
};

//////////////////////////////////////////////////////////////////////////

template<class  _TContainor>
class _ref_base_iterator
{
public:
	typedef typename _TContainor::pointer _Tptr;
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef typename _TContainor::value_type value_type;
	typedef typename _TContainor::difference_type difference_type;
	typedef typename _TContainor::pointer pointer;
	typedef typename _TContainor::reference reference;
	typedef typename _TContainor::size_type size_type;

	typedef typename _TContainor::value_node value_node;

	typedef _ref_base_iterator<_TContainor> _Myiter;

// 
// 	//typedef typename std::forward_iterator_tag iterator_category;
// 	typedef typename _Myt::value_type value_type;
// 	typedef typename _Myt::difference_type difference_type;
// 	typedef typename _Myt::const_pointer pointer;
// 	typedef typename _Myt::const_reference reference;
// 	typedef typename _Myt::size_type size_type;

	_ref_base_iterator() {treenode=0;parent=0;}
	_ref_base_iterator(const _Myiter &r)
	{
		treenode = r.treenode;
		parent = r.parent ;
		_inc_ref_rbtree() ;
	}
#ifdef _MSC_VER
	_ref_base_iterator(_Myiter &&r)
	{
		treenode = r.treenode;
		parent = r.parent ;
		_inc_ref_rbtree() ;
	}
#endif
	virtual ~_ref_base_iterator() 
	{ 
		_dec_ref_rbtree() ;
		treenode = 0;
		parent = 0;
	}

	_ref_base_iterator(value_node *it,_TContainor *pp):treenode(it),parent(pp)
	{
		_inc_ref_rbtree() ;
	}

	_Myiter& operator=(const _Myiter &r)
	{
		__inc_ref(r.treenode , r.parent) ;
		_dec_ref_rbtree() ;
		treenode = r.treenode;
		parent = r.parent ;
		return *this;
	}	
	bool operator==(const _Myiter& _Right) const{ return treenode == _Right.treenode;	}
	bool operator!=(const _Myiter& _Right) const{	return (!(treenode == _Right.treenode));}
	reference operator*() const{return (treenode->_data) ;}
	pointer operator->() const{return &(treenode->_data) ;}
	value_node* _getaddr(){return treenode;}
	void _inc_ref_rbtree() 
	{
		if (parent)	{
			parent->inc_self_ref() ;
			if (treenode) {
				if(!parent->inc_ref(treenode)) {
					treenode = NULL ;
				}
			}
		}			
	}
	void __inc_ref(value_node *_treenode , _TContainor *_parent) 
	{
		if (_parent)	{
			_parent->inc_self_ref() ;
			if (_treenode) {
				_parent->inc_ref(_treenode);
			}
		}			
	}
	void _dec_ref_rbtree() 
	{
		if (parent)	{
			if (treenode){
				parent->dec_ref(treenode)	;
			}
			parent->dec_self_ref() ;
		}
	}
	value_node *treenode ;
	_TContainor *parent;
};

template<class  _TContainor>
class ref_tree_iterator :public _ref_base_iterator<_TContainor>
{

public:
	typedef ref_tree_iterator<_TContainor> _Myiter;
	typedef _ref_base_iterator<_TContainor> _Mybase;
	typedef typename _Mybase::value_node value_node ;

	ref_tree_iterator():_Mybase() {}
	ref_tree_iterator(value_node *it,_TContainor *pp):_Mybase(it,pp) {}
	_Myiter& operator++()
	{
		if (!_Mybase::treenode){
			return *this ;
		}
		_Mybase::parent->_lock();
		_Mybase::parent->dec_ref(_Mybase::treenode) ;
		nd_rb_node  *node = &_Mybase::treenode->hdr ;
		_Mybase::treenode = NULL ;
		while ((node=rb_next(node)) != NULL) 	{
			_Mybase::treenode = rb_entry(node,value_node, hdr) ;
			if (_Mybase::parent->inc_ref(_Mybase::treenode))	{
				break ;
			}
			else {
				_Mybase::treenode = NULL;
			}
		} 
		_Mybase::parent->_unlock();
		return *this ;
	}
	_Myiter operator++(int)
	{
		ref_tree_iterator _Tmp = *this;
		++*this;
		return (_Tmp);
	}
private:
	ref_tree_iterator(const _Mybase &rbase):_Mybase(rbase) {}
	ref_tree_iterator(_Mybase &rbase):_Mybase(rbase) {}
};

template<class  _TContainor>
class ref_tree_reverse_iterator :public _ref_base_iterator<_TContainor>
{
public:
	typedef ref_tree_reverse_iterator<_TContainor> _Myiter;
	typedef _ref_base_iterator<_TContainor> _Mybase;
	typedef typename _Mybase::value_node value_node ;

	ref_tree_reverse_iterator():_Mybase() {}
	ref_tree_reverse_iterator(value_node *it,_TContainor *pp):_Mybase(it,pp) {}
	_Myiter& operator++()
	{
		if (!_Mybase::treenode){
			return *this ;
		}
		_Mybase::parent->_lock();
		_Mybase::parent->dec_ref(_Mybase::treenode) ;
		nd_rb_node  *node = &_Mybase::treenode->hdr ;
		_Mybase::treenode = NULL ;
		while ((node=rb_prev(node)) != NULL) 	{
			_Mybase::treenode = rb_entry(node,value_node, hdr) ;
			if (_Mybase::parent->inc_ref(_Mybase::treenode))	{
				break ;
			}
			else {
				_Mybase::treenode = NULL;
			}
		} 
		_Mybase::parent->_unlock();
		return *this ;
	}
	_Myiter operator++(int)
	{
		_Myiter _Tmp = *this;
		++*this;
		return (_Tmp);
	}
};
//////////////////////////////////////////////////////////////////////////

template<class _Tfirst, class _Ttype ,class clr_func = nd_rbnode_empty_cleanup<_Ttype> , class _TLock = nd_rbnode_empty_locker >
class nd_refcount_rbtree 
{
#define TREE_LOCK() _rbnode_lock_helper<_TLock>  __TmpLcok(m_lcok_hpl) 
public:
	typedef _Tfirst				comp_type ;
	typedef _Tfirst				value_key ;
	typedef ref_tree_val<_Tfirst, _Ttype> value_type ;

	typedef value_type*			pointer;
	typedef const value_type*	const_pointer;	
	typedef value_type&			reference;	
	typedef const value_type&	const_reference;	
	typedef size_t				size_type;	
	typedef ptrdiff_t			difference_type;
	typedef nd_refcount_rbtree<_Tfirst, _Ttype,clr_func,_TLock> _Myt ;

	struct rbtree_node {
		nd_rb_node hdr ;
		ndatomic_t ref_count ;
		size_t next_del ;//delete list
		value_type _data;
// 		comp_type first ;
// 		value_type second;

		rbtree_node* get_next_delnode() { return (rbtree_node* )(next_del & ~1);}
		void set_next_delnode(rbtree_node*p) {next_del = (next_del & 1) | (size_t)p;}
		int check_cleanup() {return (next_del & 1) ; }
		void set_cleanup(int clean) { next_del = (next_del & ~1) | (clean?1:0);}
		rbtree_node() 
		{
			rb_init_node(&hdr);
			ref_count = 0 ;
			next_del = 0;
		}
		rbtree_node(const _Tfirst &_first,const _Ttype &_val) :_data(_first,_val) //first(_first), second(_val)
		{
			rb_init_node(&hdr);
			ref_count = 0 ;
			next_del = 0;
		}
	};
	typedef rbtree_node				 value_node ;
	typedef nd_poolalloc<value_node> allocator_type ;
	
	typedef ref_tree_iterator<_Myt> iterator ;
	typedef ref_tree_reverse_iterator<_Myt> reverse_iterator;	
	typedef ref_tree_iterator<_Myt> const_iterator ;
	typedef ref_tree_reverse_iterator<_Myt> const_reverse_iterator;

// 	friend iterator;
// 	friend reverse_iterator ;
// 	friend const_iterator ;
// 	friend const_reverse_iterator;
	friend class ref_tree_iterator<_Myt>  ;
	friend class ref_tree_reverse_iterator<_Myt> ;
	friend class ref_tree_iterator<_Myt>  ;
	friend class ref_tree_reverse_iterator<_Myt> ;


	friend class _ref_base_iterator<_Myt>;

	iterator search(const comp_type &key)
	{
		ND_TRACE_FUNC() ;
		value_node* ptreenode = _search(key) ;
		if (!ptreenode)	{
			return iterator(NULL,NULL);
		}
		return iterator(ptreenode,this) ;
	}

	iterator find(comp_type &key){return search(key);}
	iterator find(const comp_type &key){return search((comp_type &)key);}

	iterator insert(const _Tfirst &ct,const _Ttype &vt)
	{
		ND_TRACE_FUNC() ;
		rbtree_node *ins_node = _create_obj(ct, vt) ;
		if (!ins_node){
			return iterator(NULL,NULL);
		}
		if(_insert( ins_node ) ) {
			return iterator(ins_node,this) ;
		}
		_destroy_obj(ins_node) ;
		return iterator(NULL,NULL);
	}
	bool _insert( value_node *data)
	{
		ND_TRACE_FUNC() ;
		TREE_LOCK() ;
		struct nd_rb_root *root = &m_header;
		struct nd_rb_node **new_node = &(root->rb_node), *parent = NULL;

		inc_self_ref() ;
		//Figure out where to put new node 
		while (*new_node) {
			struct rbtree_node *cur_node = rb_entry(*new_node, rbtree_node, hdr);			
			parent = *new_node;
			if (data->_data.first < cur_node->_data.first)
				new_node = &((*new_node)->rb_left);
			else if (data->_data.first > cur_node->_data.first)
				new_node = &((*new_node)->rb_right);
			else {
				dec_self_ref() ;
				return false;
			}
		}
		++m_count;
		nd_atomic_set(&data->ref_count,1) ;

		//Add new node and rebalance tree. 
		rb_link_node(&data->hdr, parent, new_node);
		rb_insert_color(&data->hdr, root);

		dec_self_ref() ;
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
		_lock() ;
		nd_rb_node *next_node = rb_next(&(erase_it._getaddr()->hdr)) ;
		_unlock() ;
		_set_erase_node(erase_it._getaddr()) ;
		
		if (next_node){
			return iterator(rb_entry(next_node, struct rbtree_node, hdr),this);
		}
		return iterator(NULL,NULL);
	}
	reverse_iterator erase(reverse_iterator &erase_it)
	{
		ND_TRACE_FUNC() ;
		_lock() ;
		nd_rb_node *next_node = rb_prev(&(erase_it._getaddr()->hdr)) ;
		_unlock() ;

		_set_erase_node(erase_it._getaddr()) ;

		if (next_node){
			return reverse_iterator(rb_entry(next_node, struct rbtree_node, hdr),this);
		}
		return reverse_iterator(NULL,NULL);
	}

	iterator erase(const iterator &erase_it)
	{
		ND_TRACE_FUNC() ;
		_lock() ;
		nd_rb_node *next_node = rb_next(&(erase_it._getaddr()->hdr)) ;
		_unlock() ;
		_set_erase_node(&*erase_it) ;

		if (next_node){
			return iterator(rb_entry(next_node, struct rbtree_node, hdr),this);
		}
		return iterator(NULL,NULL);
	}
	reverse_iterator erase(const reverse_iterator &erase_it)
	{
		ND_TRACE_FUNC() ;
		_lock() ;
		nd_rb_node *next_node = rb_prev(&(erase_it._getaddr()->hdr)) ;
		_unlock() ;
		_set_erase_node(&*erase_it) ;

		if (next_node){
			return reverse_iterator(rb_entry(next_node, struct rbtree_node, hdr),this);
		}
		return reverse_iterator(NULL,NULL);
	}
	void clear() 
	{
		ND_TRACE_FUNC() ;
		TREE_LOCK() ;
		nd_rb_node *cur_node = rb_first(&m_header) ;

		while ( cur_node) {
			value_node *del_data = rb_entry(cur_node, struct rbtree_node, hdr) ;
			cur_node = rb_next(cur_node) ;
			nd_atomic_set(&del_data->ref_count,0) ;
			_erase(del_data) ;
		} 
		m_del_list = NULL;
	}

	iterator begin() 
	{
		ND_TRACE_FUNC() ;
		inc_self_ref();
		struct rbtree_node *treedata = NULL ;
		TREE_LOCK() ;
		nd_rb_node *node = rb_first(&m_header) ;
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
		dec_self_ref();
		if (treedata && dec_ref(treedata))
			return iterator(treedata,this) ;
		else 
			return iterator(NULL,NULL) ;
	}

	iterator end() { return iterator(NULL,NULL);}
	reverse_iterator rbegin() 
	{
		ND_TRACE_FUNC() ;
		inc_self_ref();
		TREE_LOCK() ;
		struct rbtree_node *treedata = NULL ;
		nd_rb_node *node = rb_last(&m_header) ;		
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
		dec_self_ref();
		if (treedata && dec_ref(treedata))
			return reverse_iterator(treedata,this) ;
		else 
			return reverse_iterator(NULL,NULL) ;		
	}

	reverse_iterator rend() { return reverse_iterator(NULL,this);}
	int size()const{		return m_count;	}

	bool empty() const
	{
		return (m_count == 0 );
	}
	void initilize_node(value_node &data, _Tfirst *key=NULL,_Ttype *val=NULL )
	{
		ND_TRACE_FUNC() ;
		rb_init_node(&data.hdr);
		data.ref_count = 0 ;
		data.next_del = 0 ;
		//data.need_cleanup = 0 ;
		if (key){
			data._data.first = *key ;
		}
		if (val){
			data._data.second = *val;
		}
	}
	value_node *construct() {return _create_obj();}
	void destruct(value_node *addr) {_destroy_obj(addr);} 

	nd_refcount_rbtree(nd_handle pool):m_alloc(pool),m_self_ref(0) {m_count = 0; m_header.rb_node = 0 ; m_del_list=0;}
	nd_refcount_rbtree():m_self_ref(0) {m_count = 0; m_header.rb_node = 0 ; m_del_list=0;}
	nd_refcount_rbtree (const _Myt&r):m_alloc(((_Myt&)r).get_allocator()),m_self_ref(0) 
	{
		ND_TRACE_FUNC() ;
		m_count = 0; 
		m_header.rb_node = 0 ; 
		m_del_list=0;
		if (r.size() ){
			_Myt&tmp =(_Myt&) r ;
			for(iterator it = tmp.begin();it!=tmp.end(); ++it) {
				insert(it->first, it->second) ;
			}
		}
	}

	_Myt& operator=(const  _Myt&r)
	{
		ND_TRACE_FUNC() ;
		TREE_LOCK() ;
		if (size() ){
			clear();
		}
		if (r.size() ){
			_Myt&tmp =(_Myt&) r ;
			for(iterator it = tmp.begin();it!=tmp.end(); ++it) {
				insert(it->first, it->second) ;
			}
		}
		return *this ;
	}

	virtual ~nd_refcount_rbtree() {}
	void _set_pool(nd_handle pool) {m_alloc._set_pool(pool);}

	allocator_type &get_allocator() 
	{
		return (this->m_alloc);
	}
	bool check_erased(value_node *ptreenode)
	{
		if (nd_atomic_read(&ptreenode->ref_count)==0 ){
			return true ;
		}
		return false;
	}
	value_node *ptr2treenode(_Ttype* ptr)
	{
		value_type *pv = rb_entry(ptr, value_type, second) ;
		return rb_entry(pv, struct rbtree_node, _data) ;
	}
	bool _set_erase_node(value_node *del_data)
	{
		return dec_ref(del_data) ;
	}
	value_node* _search(const comp_type &key)
	{
		ND_TRACE_FUNC() ;
		TREE_LOCK() ;
		struct nd_rb_node *node = m_header.rb_node;
		while (node) {
			comp_type &data = ((rbtree_node*)node)->_data.first ;
			if (key < data)
				node = node->rb_left;
			else if (key > data)
				node = node->rb_right;
			else {
				struct rbtree_node *retnode = rb_entry(node, struct rbtree_node, hdr);
				if (0==nd_atomic_read(&retnode->ref_count)){
					return 0;
				}
				return retnode ;
			}
		}
		return (NULL);
	}

	value_node* _create_obj(const _Tfirst &_first,const _Ttype &_val) 
	{
		ND_TRACE_FUNC() ;
		rbtree_node *ins_node = m_alloc.allocate( 1 ) ;
		if (!ins_node){
			return NULL;
		}
		return new(ins_node) value_node(_first, _val) ;
	}
	void _destroy_obj(value_node *node) 
	{
		ND_TRACE_FUNC() ;
		if (node->check_cleanup())	{
			m_clean_func(node->_data.second);
		}
		node->~rbtree_node() ;
		m_alloc.deallocate(node,1) ; 
	}
	void _erase( value_node *del_data)
	{
		ND_TRACE_FUNC() ;
		nd_assert(nd_atomic_read(&del_data->ref_count) == 0) ;
		
		if (!RB_EMPTY_NODE(&del_data->hdr)) {
			--m_count;
			rb_erase(&del_data->hdr,&m_header ) ;
			rb_init_node(&del_data->hdr) ;
			_destroy_obj(del_data) ;
		}		
	}
	void _lock() 
	{
		m_lcok_hpl.lock() ;
	}
	void _unlock() 
	{
		m_lcok_hpl.unlock() ;
	}
protected:
	bool inc_ref(value_node* cur_node) 
	{
		ND_TRACE_FUNC() ;

		ndatomic_t v;
		
		if (nd_atomic_read(&cur_node->ref_count)==0 ){
			return false ;
		}

		while ( (v=nd_atomic_read(&cur_node->ref_count)) > 0) 	{
			if(nd_compare_swap(&cur_node->ref_count, v,v+1) ){
				return true ;
			}
		}
		return false ;

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
		rbtree_node *ins_node = m_alloc.allocate( 1 ) ;
		if (!ins_node){
			return NULL;
		}
		ins_node = new(ins_node) rbtree_node ;
		return ins_node;
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
	void tryto_erase() 
	{
		ND_TRACE_FUNC() ;
		m_del_lock.lock() ;
		struct rbtree_node *deletlist = m_del_list,*pnext ;
		m_del_list = NULL;
		m_del_lock.unlock() ;

		TREE_LOCK() ;
		while (deletlist){
			pnext = deletlist->get_next_delnode() ;
			//nd_atomic_set(&deletlist->ref_count,0) ;
			nd_assert(nd_atomic_read(&deletlist->ref_count)==0);
			_erase(deletlist) ;
			deletlist = pnext ;
		}
	}
	void addto_erase_list( value_node *del_data)
	{
		ND_TRACE_FUNC() ;
		m_del_lock.lock() ;
		del_data->set_next_delnode(m_del_list) ;
		m_del_list = del_data ;
		m_del_lock.unlock() ;

		if (nd_atomic_read(&m_self_ref)==0)	{
			tryto_erase() ;
		}
	}

	ndatomic_t m_self_ref ; //被引用次数
	int m_count;
	nd_rb_root m_header;
	struct rbtree_node* m_del_list ;//delete list
	clr_func m_clean_func;
	_TLock m_lcok_hpl;
	_TLock m_del_lock;
	allocator_type m_alloc ;
};
//end nd_refcount_rbtree

template<class _Tfirst, class _Ttype  >
class nd_tiny_map : public nd_refcount_rbtree<_Tfirst,_Ttype>
{
public:
	typedef nd_refcount_rbtree<_Tfirst,_Ttype> _MyBaseT ;
	nd_tiny_map():_MyBaseT(nd_global_mmpool()) 
	{

	}
	virtual ~nd_tiny_map() 
	{

	}
};
//end nd_tiny_map
#endif
