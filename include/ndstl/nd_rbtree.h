/* file nd_rbtree.h
 *
 * threaded tree 
 *
 * create by duan 
 *
 * 2012/1/4 10:20:18
 */
 
#ifndef _ND_RBTREE_H_
#define _ND_RBTREE_H_

#include "nd_common/nd_common.h"
#include "nd_allocator.h"
#include "nd_utility.h"

#include "nd_common/nd_common.h"
#include "nd_allocator.h"

template<class  _TContainor>
class rbtree_base_iterator
{
public:
	typedef typename _TContainor::pointer _Tptr;
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef typename _TContainor::value_type value_type;
	typedef typename _TContainor::difference_type difference_type;
	//typedef typename _TContainor::const_pointer pointer;
	//typedef typename _TContainor::const_reference reference;
	typedef typename _TContainor::size_type size_type;
	typedef typename _TContainor::value_node value_node;
	typedef rbtree_base_iterator<_TContainor> _Myiter;

	rbtree_base_iterator() {treenode=0;}
	rbtree_base_iterator(value_node *it):treenode(it) {}		
	_Myiter& operator=(const _Myiter &r)
	{
		treenode = r.treenode;
		return *this;
	}		
	bool operator==(const _Myiter& _Right) const{ return treenode == _Right.treenode;	}
	bool operator!=(const _Myiter& _Right) const{	return (!(treenode == _Right.treenode));}
	//value_type& operator*() const{return treenode->get_value() ;}
	//value_type* operator->() const{return &(treenode->get_value()) ;}
	value_node *get_addr() {return treenode;}
	value_node *treenode ;
};

template<class  _TContainor>
class rbtree_iterator :public rbtree_base_iterator<_TContainor>
{
public:
	typedef typename _TContainor::pointer pointer;
	typedef typename _TContainor::reference reference;

	typedef rbtree_base_iterator<_TContainor> _Mybase;
	typedef rbtree_iterator<_TContainor> _Myiter;

	rbtree_iterator():_Mybase() {}
	typedef typename _Mybase::value_node value_node;

	rbtree_iterator(value_node *it):_Mybase(it) {}
	rbtree_iterator(const _Mybase &it):_Mybase(it) {}
	_Myiter& operator++()
	{
		nd_rb_node  *node = rb_next(&_Mybase::treenode->hdr) ;
		if (node){
			_Mybase::treenode = _Mybase::treenode->get_entry(node);//rb_entry(node,struct rbtree_node, hdr) ;
		}else {
			_Mybase::treenode = NULL;
		}
		return *this ;
	}
	_Myiter operator++(int)
	{
		_Myiter _Tmp = *this;
		++*this;
		return (_Tmp);
	}
	reference operator*() const{return _Mybase::treenode->get_value() ;}
	pointer operator->() const{return &(_Mybase::treenode->get_value()) ;}
};

template<class  _TContainor>
class const_rbtree_iterator :public rbtree_base_iterator<_TContainor>
{
public:
	typedef typename _TContainor::const_pointer pointer;
	typedef typename _TContainor::const_reference reference;

	typedef rbtree_base_iterator<_TContainor> _Mybase;
	typedef const_rbtree_iterator<_TContainor> _Myiter;

	const_rbtree_iterator():_Mybase() {}
	typedef typename _Mybase::value_node value_node;

	const_rbtree_iterator(value_node *it):_Mybase(it) {}
	const_rbtree_iterator(const _Mybase &it):_Mybase(it) {}
	_Myiter& operator++() 
	{
		nd_rb_node  *node = rb_next(&_Mybase::treenode->hdr) ;
		if (node){
			_Mybase::treenode = _Mybase::treenode->get_entry(node);//rb_entry(node,struct rbtree_node, hdr) ;
		}else {
			_Mybase::treenode = NULL;
		}
		return *this ;
	}
	_Myiter operator++(int) 
	{
		_Myiter _Tmp = *this;
		++*this;
		return (_Tmp);
	}
	reference operator*() const{return _Mybase::treenode->get_value() ;}
	pointer operator->() const{return &(_Mybase::treenode->get_value()) ;}
};

template<class  _TContainor>
class rbtree_reverse_iterator :public rbtree_base_iterator<_TContainor>
{
public:
	typedef typename _TContainor::pointer pointer;
	typedef typename _TContainor::reference reference;
	typedef rbtree_base_iterator<_TContainor> _Mybase;
	typedef rbtree_reverse_iterator<_TContainor> _Myiter;

	typedef typename _Mybase::value_node value_node;

	rbtree_reverse_iterator():_Mybase() {}
	rbtree_reverse_iterator(const _Mybase &it):_Mybase(it) {}
	rbtree_reverse_iterator(value_node *it):_Mybase(it) {}
	_Myiter& operator++()
	{
		nd_rb_node  *node = rb_prev(&_Mybase::treenode->hdr) ;
		if (node){
			_Mybase::treenode = _Mybase::treenode->get_entry(node);//rb_entry(node,struct rbtree_node, hdr) ;
		}else {
			_Mybase::treenode = NULL;
		}
		return *this ;
	}
	_Myiter operator++(int)
	{
		_Myiter _Tmp = *this;
		++*this;
		return (_Tmp);
	}

	reference operator*() const{return _Mybase::treenode->get_value() ;}
	pointer operator->() const{return &(_Mybase::treenode->get_value()) ;}
};

template<class  _TContainor>
class const_rbtree_reverse_iterator :public rbtree_base_iterator<_TContainor>
{
public:
	typedef typename _TContainor::const_pointer pointer;
	typedef typename _TContainor::const_reference reference;
	typedef rbtree_base_iterator<_TContainor> _Mybase;
	typedef const_rbtree_reverse_iterator<_TContainor> _Myiter;

	typedef typename _Mybase::value_node value_node;

	const_rbtree_reverse_iterator():_Mybase() {}
	const_rbtree_reverse_iterator(const _Mybase &it):_Mybase(it) {}
	const_rbtree_reverse_iterator(value_node *it):_Mybase(it) {}
	_Myiter& operator++() 
	{
		nd_rb_node  *node = rb_prev(&_Mybase::treenode->hdr) ;
		if (node){
			_Mybase::treenode = _Mybase::treenode->get_entry(node);//rb_entry(node,struct rbtree_node, hdr) ;
		}else {
			_Mybase::treenode = NULL;
		}
		return *this ;
	}
	_Myiter operator++(int) const
	{
		_Myiter _Tmp = *this;
		++*this;
		return (_Tmp);
	}
	reference operator*() const{return _Mybase::treenode->get_value() ;}
	pointer operator->() const{return &(_Mybase::treenode->get_value()) ;}
};

//base rb tree without allocator
template<class _Ttype>
class _nd_rbtree
{
public:
	typedef _Ttype				value_type;	
	typedef value_type*			pointer;	
	typedef const value_type*	const_pointer;	
	typedef value_type&			reference;	
	typedef const value_type&	const_reference;	
	typedef size_t				size_type;	
	typedef ptrdiff_t			difference_type;
	typedef _nd_rbtree<_Ttype> _Myt ;
	struct rbtree_node {
		nd_rb_node hdr ;
		value_type _value;
		rbtree_node()
		{
			rb_init_node(&hdr);
		}
		rbtree_node(const value_type &_val) : _value(_val)
		{
			rb_init_node(&hdr);
		}
		struct rbtree_node *get_entry(nd_rb_node  *node)
		{
			return rb_entry(node,struct rbtree_node, hdr) ; 
		}
		value_type &get_value() {return _value;}
	};
	typedef rbtree_node			value_node ;
	typedef value_node			rbtree_value ;

	typedef rbtree_iterator<_Myt> iterator ;
	typedef rbtree_reverse_iterator<_Myt> reverse_iterator;

	typedef const_rbtree_iterator<_Myt> const_iterator ;
	typedef const_rbtree_reverse_iterator<_Myt> const_reverse_iterator;

	bool insert( value_node *data)
	{
		ND_TRACE_FUNC() ;
		struct rb_root *root = &m_header;
		struct rb_node **new_node = &(root->rb_node), *parent = NULL;

		//Figure out where to put new node 
		while (*new_node) {
			struct rbtree_node *cur_node = rb_entry(*new_node, rbtree_node, hdr);			
			parent = *new_node;
			if (data->_value < cur_node->_value)
				new_node = &((*new_node)->rb_left);
			else if (data->_value > cur_node->_value)
				new_node = &((*new_node)->rb_right);
			else
				return false;
		}
		++m_count;
		//Add new node and rebalance tree. 
		rb_link_node(&data->hdr, parent, new_node);
		rb_insert_color(&data->hdr, root);
		return true;
	}

	iterator search(const _Ttype& key)
	{
		ND_TRACE_FUNC() ;
		struct nd_rb_node *node = m_header.rb_node;
		while (node) {
			const_reference data = ((rbtree_node*)node)->_value ;
			if (key < data)
				node = node->rb_left;
			else if (key > data)
				node = node->rb_right;
			else
				return iterator(rb_entry(node, struct rbtree_node, hdr));
		}
		return iterator(NULL);
	}

	iterator _erase(iterator erase_it)
	{
		ND_TRACE_FUNC() ;
		value_node *_addr = erase_it.get_addr() ;
		nd_rb_node *node = rb_next(&_addr->hdr) ;
		rb_erase(&_addr->hdr,&m_header ) ;
		--m_count;
		if (node) {
			return iterator(rb_entry(node, struct rbtree_node, hdr));
		}
		else 
			return iterator(NULL);
	}
	reverse_iterator _erase(reverse_iterator erase_it)
	{
		ND_TRACE_FUNC() ;
		value_node *_addr = erase_it.get_addr() ;
		nd_rb_node *node = rb_prev(&_addr->hdr) ;
		rb_erase(&_addr->hdr,&m_header ) ;
		--m_count;
		if (node) {
			return reverse_iterator(rb_entry(node, struct rbtree_node, hdr));
		}
		else 
			return reverse_iterator(NULL);

	}

	iterator begin() 
	{
		ND_TRACE_FUNC() ;
		nd_rb_node *node = rb_first(&m_header) ;
		if (node)
			return iterator(rb_entry(node, struct rbtree_node, hdr)) ;
		else 
			return iterator(NULL) ;
	}
	iterator end() { return iterator(NULL);}
	reverse_iterator rbegin() 
	{
		ND_TRACE_FUNC() ;
		nd_rb_node *node = rb_last(&m_header) ;
		if (node)
			return reverse_iterator(rb_entry(node, struct rbtree_node, hdr)) ;
		else 
			return reverse_iterator(NULL) ;
	}
	reverse_iterator rend() { return reverse_iterator(NULL);}
	//////////////////////////////////////////////////////////////////////////
	const_iterator begin() const
	{
		ND_TRACE_FUNC() ;
		nd_rb_node *node = rb_first(&m_header) ;
		if (node)
			return const_iterator(rb_entry(node, struct rbtree_node, hdr)) ;
		else 
			return const_iterator(NULL) ;
	}
	const_iterator end() const{ return const_iterator(NULL);}
	const_reverse_iterator rbegin()const 
	{
		ND_TRACE_FUNC() ;
		nd_rb_node *node = rb_last(&m_header) ;
		if (node)
			return const_reverse_iterator(rb_entry(node, struct rbtree_node, hdr)) ;
		else 
			return const_reverse_iterator(NULL) ;
	}
	const_reverse_iterator rend()const { return const_reverse_iterator(NULL);}


	int size(){		return m_count;	}
	virtual size_t capacity() {return (size_t)-1;}
	bool empty() {return m_count == 0 ;}

	_nd_rbtree() {m_count = 0; m_header.rb_node = 0 ;}
	virtual ~_nd_rbtree(){}
protected:
	int m_count;
	nd_rb_root m_header;
};
/*
//base rb tree without allocator
template<class _Tfirst, class _Ttype>
class nd_base_rbtree
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
	typedef nd_base_rbtree<comp_type, value_type> _Myt ;
	struct rbtree_node {
		rb_node hdr ;
		ndatomic_t ref_count ;
		comp_type first ;
		value_type second;

		rbtree_node()
		{
			rb_init_node(&hdr);
			ref_count = 0 ;
		}
		rbtree_node(const comp_type &_first,const value_type &_val) : first(_first), second(_val)
		{
			rb_init_node(&hdr);
			ref_count = 0 ;
		}
	};
	typedef rbtree_node			value_node ;
	typedef value_node			rbtree_value ;
	class base_iterator
	{
	public:
		base_iterator() {treenode=0;}
		base_iterator(value_node *it):treenode(it) {}		
		base_iterator& operator=(const base_iterator &r)
		{
			treenode = r.treenode;
			return *this;
		}		
		bool operator==(const base_iterator& _Right) const{ return treenode == _Right.treenode;	}
		bool operator!=(const base_iterator& _Right) const{	return (!(treenode == _Right.treenode));}
		value_node& operator*() const{return *treenode ;}
		value_node* operator->() const{return treenode ;}
		value_node *treenode ;
	};
	class rbtree_terator :public base_iterator
	{
	public:
		rbtree_terator():base_iterator() {}
		rbtree_terator(value_node *it):base_iterator(it) {}
		rbtree_terator& operator++()
		{
			rb_node  *node = rb_next(&(base_iterator::treenode->hdr)) ;
			if (node){
				base_iterator::treenode = rb_entry(node,struct rbtree_node, hdr) ;
			}else {
				base_iterator::treenode = NULL;
			}
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
		rbtree_reverse_iterator(value_node *it):base_iterator(it) {}
		rbtree_reverse_iterator& operator++()
		{
			rb_node  *node = rb_prev(&(base_iterator::treenode->hdr)) ;
			if (node){
				base_iterator::treenode = rb_entry(node,struct rbtree_node, hdr) ;
			}else {
				base_iterator::treenode = NULL;
			}
			return *this ;
		}
		rbtree_reverse_iterator operator++(int)
		{
			rbtree_reverse_iterator _Tmp = *this;
			++*this;
			return (_Tmp);
		}
	};

	typedef rbtree_terator iterator ;
	typedef rbtree_reverse_iterator reverse_iterator;
	
	bool insert( value_node *data)
	{
		ND_TRACE_FUNC() ;
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
		//Add new node and rebalance tree. 
		rb_link_node(&data->hdr, parent, new_node);
		rb_insert_color(&data->hdr, root);
		return true;
	}

	iterator search(comp_type &key)
	{
		ND_TRACE_FUNC() ;
		struct rb_node *node = m_header.rb_node;
		while (node) {
			comp_type &data = ((rbtree_node*)node)->first ;
			if (key < data)
				node = node->rb_left;
			else if (key > data)
				node = node->rb_right;
			else
				return iterator(rb_entry(node, struct rbtree_node, hdr));
		}
		return iterator(NULL);
	}

	bool erase(comp_type &key, iterator &deleted_backup)
	{
		ND_TRACE_FUNC() ;
		deleted_backup = search(key) ;
		if (deleted_backup == end()) {
			return false ;
		}
		rb_erase(&deleted_backup->hdr,&m_header ) ;
		rb_init_node(&deleted_backup->hdr) ;
		--m_count;
		return true;
	}
	iterator erase(iterator erase_it)
	{
		ND_TRACE_FUNC() ;
		rb_node *node = rb_next(&(erase_it->hdr)) ;
		rb_erase(&erase_it->hdr,&m_header ) ;
		rb_init_node(&erase_it->hdr) ;
		--m_count;
		if (node) {
			return iterator(rb_entry(node, struct rbtree_node, hdr));
		}
		else 
			return iterator(NULL);
	}
	reverse_iterator erase(reverse_iterator erase_it)
	{
		ND_TRACE_FUNC() ;
		rb_node *node = rb_prev(&(erase_it->hdr)) ;
		rb_erase(&erase_it->hdr,&m_header ) ;
		rb_init_node(&erase_it->hdr) ;
		--m_count;
		if (node) {
			return reverse_iterator(rb_entry(node, struct rbtree_node, hdr));
		}
		else 
			return reverse_iterator(NULL);

	}

	iterator begin() 
	{
		ND_TRACE_FUNC() ;
		rb_node *node = rb_first(&m_header) ;
		if (node)
			return iterator(rb_entry(node, struct rbtree_node, hdr)) ;
		else 
			return iterator(NULL) ;
	}
	iterator end() { return iterator(NULL);}
	reverse_iterator rbegin() 
	{
		ND_TRACE_FUNC() ;
		rb_node *node = rb_last(&m_header) ;
		if (node)
			return reverse_iterator(rb_entry(node, struct rbtree_node, hdr)) ;
		else 
			return reverse_iterator(NULL) ;
	}

	reverse_iterator rend() { return reverse_iterator(NULL);}
	int size(){		return m_count;	}
	virtual size_t capacity() {return (size_t)-1;}

	void initilize_node(rbtree_value &data, comp_type *key=NULL,value_type *val=NULL )
	{
		ND_TRACE_FUNC() ;
		rb_init_node(&data.hdr);
		data.ref_count = 0 ;
		if (key){
			data.first = *key ;
		}
		if (val){
			data.second = *val;
		}
	}
	nd_base_rbtree() {m_count = 0; m_header.rb_node = 0 ;}
	virtual ~nd_base_rbtree(){}
protected:
	int m_count;
	rb_root m_header;
};
// end nd_base_rbtree

//”Îstlmap¿‡À∆µƒrbtree
#include "nd_allocator.h"
template<class _Tfirst, class _Ttype,class _Alloc = nd_stlalloc<typename nd_base_rbtree<_Tfirst, _Ttype>::value_node> >
class nd_std_rbtree : public nd_base_rbtree<_Tfirst, _Ttype>
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

	typedef nd_std_rbtree<_Tfirst, _Ttype> _Myt ;
	typedef nd_base_rbtree<_Tfirst, _Ttype> _MyBaseT ;
	typedef _Alloc allocator_type ;
	typedef typename _MyBaseT::iterator iterator ;
	typedef typename _MyBaseT::reverse_iterator reverse_iterator;
	typedef typename _MyBaseT::value_node value_node ;

	iterator insert(const comp_type &ct,const value_type &vt)
	{
		ND_TRACE_FUNC() ;
		value_node *ins_node = _create_obj(ct,vt) ;
		if (!ins_node){
			return iterator(NULL);
		}
// 		ins_node->first = ct; 
// 		ins_node->second = vt ;
		if(_MyBaseT::insert( ins_node) ) {
			++_MyBaseT::m_count;
			return iterator(ins_node) ;
		}
		_destroy_obj(ins_node) ;
		return iterator(NULL);
	}

	bool erase(comp_type &key)
	{
		ND_TRACE_FUNC() ;
		iterator del_it ;
		if(_MyBaseT::erase(key,del_it) ) {
			_destroy_obj(&(*del_it)) ;
			return true;
		}
		return false;
	}

	iterator erase(iterator &erase_it)
	{
		iterator next_it = _MyBaseT::erase(erase_it) ;
		_destroy_obj(&(*erase_it)) ;
		return next_it;
	}
	reverse_iterator erase(reverse_iterator &erase_it)
	{
		reverse_iterator next_it = _MyBaseT::erase(erase_it) ;
		_destroy_obj(&(*erase_it)) ;
		return next_it;
	}

	void clear() 
	{
		ND_TRACE_FUNC() ;
		for(iterator it =_MyBaseT::begin(); it!=_MyBaseT::end(); ) {
			it = erase(it);
			if (_MyBaseT::size()==0 || it == _MyBaseT::end()){
				break ;
			}
		}
	}
	value_type& operator[](const value_key& _Keyval)
	{	
		ND_TRACE_FUNC() ;
		iterator _Where = this->search(_Keyval) ;
		if (_Where == this->end())
			_Where = this->insert(_Keyval, value_type());
		return ((*_Where).second);
	}
	size_t capacity() {return (size_t)m_alInst.max_size();}
	nd_std_rbtree() {} 
	nd_std_rbtree(allocator_type &a):m_alInst(a)  {} 
	
	virtual ~nd_std_rbtree() {} 

	allocator_type &get_allocator() 
	{
		return (this->m_alInst);
	}
protected:
	
	value_node* _create_obj() 
	{
		ND_TRACE_FUNC() ;
		value_node *ins_node = m_alInst.allocate(1) ;
		if (!ins_node){
			return NULL;
		}
		ins_node = new(ins_node) value_node ;
// 		rb_init_node(&ins_node->hdr);
// 		ins_node->ref_count = 0 ;
		return ins_node;
	}
	value_node* _create_obj(const comp_type &_first,const value_type &_val) 
	{
		ND_TRACE_FUNC() ;
		value_node *ins_node =  m_alInst.allocate(1) ;
		if (!ins_node){
			return NULL;
		}
		return new(ins_node) value_node(_first, _val) ;
	}
	void _destroy_obj(value_node *node) 
	{
		ND_TRACE_FUNC() ;
		m_alInst.destroy(node) ;
		m_alInst.deallocate(node,1);
	}

	_Alloc m_alInst;	// allocator object for values
};
// end nd_std_rbtree
*/
#include "nd_reftree.h"
#endif
