/* file nd_stllist.h
 *
 * stl-list
 *
 * create by duan 
 *
 * 2012/6/15 10:40:27
 */

#ifndef _ND_STLLIST_H_
#define _ND_STLLIST_H_

#include "nd_common/nd_common.h"
#include "nd_allocator.h"


//////////////////////////////////////////////////////////////////////////

template<class  _TContainor>
class _ndlist_base_iterator: public std::iterator<std::bidirectional_iterator_tag,typename _TContainor::value_type>
{
public:
	typedef typename _TContainor::pointer _Tptr;
	typedef std::bidirectional_iterator_tag iterator_category;

	typedef typename _TContainor::value_type value_type;
	typedef typename _TContainor::difference_type difference_type;	
	typedef typename _TContainor::size_type size_type;

	typedef typename _TContainor::value_node value_node;
	typedef _ndlist_base_iterator<_TContainor> _Myiter;

	_ndlist_base_iterator() {_node=0;_next = 0;_prev=0; _header=0;}
	_ndlist_base_iterator(list_head *cursor, list_head *hdr):_header(hdr) ,_node(0),_next(0),_prev(0)
	{
		if (cursor)	{
			_next = cursor->next ;
			_prev = cursor->prev ;
			if (cursor != hdr)	{
				_node = list_entry(cursor,value_node, hdr) ;
			}
			else {
				_node = NULL ;
			}
		}
	}
	
	_Myiter& operator=(const _Myiter &r)
	{
		_header = r._header;
		_node = r._node;
		if (_node) {
			_next = _node->hdr.next ;
			_prev = _node->hdr.prev;
		}
		else {
			_next = _header->next;
			_prev = _header->prev ;
		}
		return *this;
	}
	bool operator==(const _Myiter& _Right) const{ return _node == _Right._node;	}
	bool operator!=(const _Myiter& _Right) const{	return (!(_node == _Right._node));}
	struct list_head *next() {	return _next ;	}
	struct list_head *prev() {	return _prev ;	}
	struct list_head *getlist() {	return _node ? &_node->hdr : _header ;	}
	value_node *_getaddr() {return _node;}
	value_node *_node ;
	struct list_head *_next,*_prev ;//save pointer of to delete next druing iterate
	struct list_head *_header;
};

template<class  _TContainor>
class _ndlist_iterator :public _ndlist_base_iterator<_TContainor>
{
public:
	typedef typename _TContainor::pointer pointer;
	typedef typename _TContainor::reference reference;

	typedef _ndlist_iterator<_TContainor> _Myiter;
	typedef _ndlist_base_iterator<_TContainor> _Mybase;
	typedef typename _Mybase::value_node value_node;

	_ndlist_iterator():_Mybase() {}
	_ndlist_iterator(list_head *cursor,list_head *hdr):_Mybase(cursor,hdr) {}
	_Myiter& operator++()
	{
		struct list_head *pos = _Mybase::_next ;
		_Mybase::_next = pos->next ;
		_Mybase::_prev = pos->prev ;
		if (pos==_Mybase::_header){
			_Mybase::_node = 0 ;
			return *this;
		}
		_Mybase::_node = list_entry(pos,value_node, hdr);
		return *this ;
	}
	_Myiter& operator--()
	{
		struct list_head *pos = _Mybase::_prev ;
		_Mybase::_next = pos->next ;
		_Mybase::_prev = pos->prev ;
		if (pos==_Mybase::_header){
			_Mybase::_node = 0 ;
			return *this;
		}
		_Mybase::_node = list_entry(pos,value_node, hdr);
		return *this ;
	}
	_ndlist_iterator operator++(int)
	{
		_ndlist_iterator _Tmp = *this;
		++*this;
		return (_Tmp);
	}
	reference operator*() const{return _Mybase::_node->listval ;}
	pointer operator->() const{return &(_Mybase::_node->listval) ;}
};

template<class  _TContainor>
class _ndlist_reverse_iterator :public _ndlist_base_iterator<_TContainor>
{
public:
	typedef typename _TContainor::pointer pointer;
	typedef typename _TContainor::reference reference;

	typedef _ndlist_reverse_iterator<_TContainor> _Myiter;
	typedef _ndlist_base_iterator<_TContainor> _Mybase;
	typedef typename _Mybase::value_node value_node;

	_ndlist_reverse_iterator():_Mybase() {}
	_ndlist_reverse_iterator(list_head *cursor,list_head *hdr):_Mybase(cursor,hdr) {	}

	_Myiter& operator++()
	{
		struct list_head *pos = _Mybase::_prev ;
		_Mybase::_next = pos->next ;
		_Mybase::_prev = pos->prev ;
		if (pos==_Mybase::_header){
			_Mybase::_node = 0 ;
			return *this;
		}
		_Mybase::_node = list_entry(pos,value_node, hdr);
		return *this ;
	}
	_Myiter& operator--()
	{
		struct list_head *pos = _Mybase::_next ;
		_Mybase::_next = pos->next ;
		_Mybase::_prev = pos->prev ;
		if (pos==_Mybase::_header){
			_Mybase::_node = 0 ;
			return *this;
		}
		_Mybase::_node = list_entry(pos,value_node, hdr);
		return *this ;
	}
	_Myiter operator++(int)
	{
		_Myiter _Tmp = *this;
		++*this;
		return (_Tmp);
	}
	reference operator*() const{return _Mybase::_node->listval ;}
	pointer operator->() const{return &(_Mybase::_node->listval) ;}
};

//////////////////////////////////////////////////////////////////////////
/*

template<class  _TContainor>
class _const_ndlist_iterator :public _ndlist_base_iterator<_TContainor>
{
public:
	typedef typename _TContainor::const_pointer pointer;
	typedef typename _TContainor::const_reference reference;
	typedef _const_ndlist_iterator<_TContainor> _Myiter;
	typedef _ndlist_base_iterator<_TContainor> _Mybase;

	_const_ndlist_iterator():_ndlist_base_iterator() {}
	_const_ndlist_iterator(list_head *cursor,list_head *hdr):_ndlist_base_iterator(cursor,hdr) {}

	_Myiter& operator++()
	{
		struct list_head *pos = _Mybase::_next ;
		_Mybase::_next = pos->next ;
		_Mybase::_prev = pos->prev ;
		if (pos==_Mybase::_header){
			_Mybase::_node = 0 ;
			return *this;
		}
		_Mybase::_node = list_entry(pos,value_node, hdr);
		return *this ;
	}
	_Myiter& operator--()
	{
		struct list_head *pos = _Mybase::_prev ;
		_Mybase::_next = pos->next ;
		_Mybase::_prev = pos->prev ;
		if (pos==_Mybase::_header){
			_Mybase::_node = 0 ;
			return *this;
		}
		_Mybase::_node = list_entry(pos,value_node, hdr);
		return *this ;
	}
	_ndlist_iterator operator++(int)
	{
		_ndlist_iterator _Tmp = *this;
		++*this;
		return (_Tmp);
	}
	reference& operator*() const{return _node->listval ;}
	pointer* operator->() const{return &(_node->listval) ;}
};

template<class  _TContainor>
class _const_ndlist_reverse_iterator :public _ndlist_base_iterator<_TContainor>
{
public:
	typedef typename _TContainor::const_pointer pointer;
	typedef typename _TContainor::const_reference reference;
	typedef _const_ndlist_reverse_iterator<_TContainor> _Myiter;
	typedef _ndlist_base_iterator<_TContainor> _Mybase;
	_const_ndlist_reverse_iterator():_ndlist_base_iterator() {}
	_const_ndlist_reverse_iterator(list_head *cursor,list_head *hdr):_ndlist_base_iterator(cursor,hdr) {	}

	_Myiter& operator++()
	{
		struct list_head *pos = _Mybase::_prev ;
		_Mybase::_next = pos->next ;
		_Mybase::_prev = pos->prev ;
		if (pos==_Mybase::_header){
			_Mybase::_node = 0 ;
			return *this;
		}
		_Mybase::_node = list_entry(pos,value_node, hdr);
		return *this ;
	}
	_Myiter& operator--()
	{
		struct list_head *pos = _Mybase::_next ;
		_Mybase::_next = pos->next ;
		_Mybase::_prev = pos->prev ;
		if (pos==_Mybase::_header){
			_Mybase::_node = 0 ;
			return *this;
		}
		_Mybase::_node = list_entry(pos,value_node, hdr);
		return *this ;
	}
	_Myiter operator++(int)
	{
		_Myiter _Tmp = *this;
		++*this;
		return (_Tmp);
	}
	reference& operator*() const{return _node->listval ;}
	pointer* operator->() const{return &(_node->listval) ;}
};
*/

template<class _Ttype>
class nd_stllist
{
public:
	typedef _Ttype				value_type;	
	typedef value_type*			pointer;	
	typedef const value_type*	const_pointer;	
	typedef value_type&			reference;	
	typedef const value_type&	const_reference;	
	typedef size_t				size_type;	
	typedef ptrdiff_t			difference_type;
	typedef nd_stllist< value_type> _Myt ;
	struct list_nodeval {
		struct list_head  hdr ;
		value_type listval;
	};
	typedef list_nodeval value_node ;
	typedef nd_poolalloc<value_node> allocator_type;

	typedef _ndlist_iterator<_Myt> iterator ;
	typedef _ndlist_iterator<_Myt> const_iterator ;
	typedef _ndlist_reverse_iterator<_Myt> reverse_iterator;
	typedef _ndlist_reverse_iterator<_Myt> const_reverse_iterator;



	friend class _ndlist_iterator<_Myt>  ;
	friend class _ndlist_iterator<_Myt>  ;
	friend class _ndlist_reverse_iterator<_Myt> ;
	friend class _ndlist_reverse_iterator<_Myt> ;
	friend class _ndlist_base_iterator<_Myt>;

	//////////////////////////////////////////////////////////////////////////
	nd_stllist(nd_handle pool=NULL):_Allocator(pool),_cur_number(0)
	{
		ND_TRACE_FUNC() ;
		INIT_LIST_HEAD(&_Header) ;
	}
	nd_stllist (const _Myt&r):_Allocator(((_Myt&)r).get_allocator()),_cur_number(0)
	{
		ND_TRACE_FUNC() ;
		INIT_LIST_HEAD(&_Header) ;
		if (r.size() ){
			_Myt&tmp =(_Myt&) r ;
			for(iterator it = tmp.begin();it!=tmp.end(); ++it) {
				push_back(*it) ;
			}
		}
	}
	virtual ~nd_stllist()
	{
		clear();
	}

	_Myt& operator=(const  _Myt&r)
	{
		ND_TRACE_FUNC() ;
		if (size() ){
			erase(begin(), end()) ;
		}
		if (r.size() ){
			_Myt&tmp =(_Myt&) r ;
			for(iterator it = tmp.begin();it!=tmp.end(); ++it) {
				push_back(*it) ;
			}
		}
		return *this ;
	}
	//////////////////////////////////////////////////////////////////////////

	void _set_pool(nd_handle pool) {_Allocator._set_pool(pool);}

	allocator_type &get_allocator() 
	{
		return (this->_Allocator);
	}
	value_node *_Construct(const_reference _Val) 
	{
		ND_TRACE_FUNC() ;
		value_node *_addr = _Allocator.allocate(1) ;
		if (!_addr){
			return 0 ;
		}
		//_Allocator.construct(_addr,_Val) ;
		//_addr = new(_addr) value_node ;
		INIT_LIST_HEAD(&_addr->hdr) ;
		new(&_addr->listval) _Ttype(_Val);
		return _addr ;
	}
	void _Destruct(value_node*_Val) 
	{
		ND_TRACE_FUNC() ;
		list_del_init(&_Val->hdr) ;
		_Val->listval.~_Ttype() ;
		//_NDDestroy(&_Val->listval); 
		_Allocator.deallocate(_Val,1) ;
		--_cur_number;
	}
	value_node* _Insert_back(struct list_head *pos, const _Ttype& _Val )
	{
		ND_TRACE_FUNC() ;
		value_node *_addr = _Construct(_Val) ;
		if (_addr){
			if (pos){
				list_add(&_addr->hdr,pos) ;
			}
			else {
				list_add(&_addr->hdr,&_Header) ;
			}
			++_cur_number;
		}
		return _addr;
	}
	value_node* _Insert_front(struct list_head *pos, const _Ttype& _Val )
	{
		ND_TRACE_FUNC() ;
		value_node *_addr = _Construct(_Val) ;
		if (_addr){
			if (pos){
				list_add_tail(&_addr->hdr,pos) ;
			}
			else {
				list_add_tail(&_addr->hdr,&_Header) ;
			}
			++_cur_number;
		}
		return _addr;
	}

	void push_front(const _Ttype& _Val)
	{
		_Insert_back(&_Header, _Val) ;
	}
	void push_back(const _Ttype& _Val)
	{
		_Insert_front(&_Header, _Val) ;
	}
	void push_front(_Ttype& _Val)
	{
		_Insert_back(&_Header, (const_reference)_Val) ;
	}
	void push_back(_Ttype& _Val)
	{
		_Insert_front(&_Header,(const_reference) _Val) ;
	}

	void pop_front()
	{
		erase(begin());
	}

	void pop_back()
	{
		erase(--end());
	}
	iterator erase(iterator erit) 
	{
		ND_TRACE_FUNC() ;
		value_node *_addr = erit._getaddr() ;
		iterator ret = erit ;
		++ret;
		if (_addr){
			_Destruct(_addr) ;
		}
		else {
			return end();
		}
		return ret ;
	}
	//const_iterator erase(const_iterator &erit) 	{return(const_iterator) erase((const_iterator&)erit);}
	reverse_iterator erase(reverse_iterator erit) 
	{
		ND_TRACE_FUNC() ;
		value_node *_addr = erit._getaddr() ;
		reverse_iterator ret = erit ;
		++ret;
		if (_addr){
			_Destruct(_addr) ;
		}
		else {
			return rend();
		}
		return ret ;
	}
	//const_reverse_iterator erase(const_reverse_iterator &erit) {return (reverse_iterator)erase((reverse_iterator&)erit);}

	iterator erase(const_iterator _First, const_iterator _Last)
	{
		ND_TRACE_FUNC() ;
		if (_First == begin() && _Last == end()){	
			clear();
			return (end());
		}
		else{
			while (_First != _Last)
				_First = erase(_First);
			return (iterator)_Last;
		}
	}

	iterator begin()
	{
		return iterator(_Header.next, &_Header) ;
	}
// 	const_iterator begin() 
// 	{
// 		return (const_iterator(_Header.next, &_Header) );
// 	}
	iterator end()
	{
		return iterator(&_Header, &_Header) ;
	}
// 	const_iterator end() 
// 	{
// 		return (const_iterator(&_Header, &_Header));
// 	}
	reverse_iterator rbegin()
	{
		return reverse_iterator(_Header.prev, &_Header);
	}
// 	const_reverse_iterator rbegin() 
// 	{
// 		return const_reverse_iterator(_Header.prev, &_Header);
// 	}
	reverse_iterator rend()
	{
		return reverse_iterator(&_Header, &_Header);
	}
// 	const_reverse_iterator rend() 
// 	{
// 		return const_reverse_iterator(&_Header, &_Header);
// 	}
	size_type size() const
	{
		return _cur_number;
	}

	size_type max_size() const
	{
		return (this->_Allocator.max_size());
	}

	bool empty() const
	{
		return (_Header.next == &_Header);
	}

	reference front()
	{
		return (*begin());
	}

	const_reference front() const
	{
		return (*begin());
	}

	reference back()
	{
		return (*(--end()));
	}

	const_reference back() const
	{
		return (*(--end()));
	}

	iterator insert(const_iterator& _Where, const _Ttype& _Val)
	{	
		ND_TRACE_FUNC() ;

		value_node *ret = _Insert_front(_Where.getlist(), _Val ) ;
		if (!ret){
			return end() ;
		}
		return iterator(&ret->hdr, &_Header) ;
	}

	void clear()
	{
		ND_TRACE_FUNC() ;
		if (!empty()){
			struct list_head *pos,*next;
			list_for_each_safe(pos,next,&_Header) {
				value_node *node = list_entry(pos, struct list_nodeval , hdr) ;
				_Destruct(node) ;
			}
			INIT_LIST_HEAD(&_Header);
		}
	}

	void swap(_Myt& _Right)
	{
		ND_TRACE_FUNC() ;
		size_type tmum1 = _cur_number ;
		size_type tmum2 =_Right._cur_number ;

		struct list_head tmp1, tmp2 ;
		INIT_LIST_HEAD(&tmp1);
		INIT_LIST_HEAD(&tmp2);
		if (this->_Allocator == _Right._Allocator){
			list_add_tail(&tmp1, &_Header) ;
			list_del_init(&_Header) ;
			
			list_add_tail(&tmp2, &_Right._Header) ;
			list_del_init(&_Right._Header) ;

			list_add_tail(&_Header, &tmp2) ;
			list_del_init(&tmp2);

			list_add_tail(&_Right._Header, &tmp1) ;
			list_del_init(&tmp1);
		}
		else {
			list_add_tail(&tmp1, &_Header) ;
			list_del_init(&_Header) ;

			// right -> this
			struct list_head *pos,*next;
			list_for_each_safe(pos,next,&_Right._Header) {
				value_node *node = list_entry(pos, struct list_nodeval , hdr) ;
				_Insert_back(&_Header,node->listval) ;
				_Right._Destruct(node) ;
			}

			//this -> right
			list_for_each_safe(pos,next,&tmp1) {
				value_node *node = list_entry(pos, struct list_nodeval , hdr) ;
				_Right._Insert_back(&_Right._Header,node->listval) ;
				_Destruct(node) ;
			}
		}
		_cur_number = tmum2 ;
		_Right._cur_number = tmum1 ;
	}

	void splice(const_iterator& _Where, _Myt& _Right)
	{
		ND_TRACE_FUNC() ;
		nd_assert(NOT_SUPPORT_THIS_FUNCTION);
		struct list_head *pos,*next,*inspos;
		inspos = &(_Where._getaddr()->hdr) ;
		list_for_each_safe(pos,next,&_Right._Header) {
			value_node *node = list_entry(pos, struct list_nodeval , hdr) ;
			if (this->_Allocator == _Right._Allocator){
				list_del_init(&node->hdr) ;
				list_add_tail(&node->hdr,inspos) ;
			}
			else {
				_Insert_front(inspos,node->listval) ;
				_Right._Destruct(node) ;
			}
		}
	}
	void sort()
	{
		ND_TRACE_FUNC() ;
		struct list_head tmp1;
		INIT_LIST_HEAD(&tmp1);
		list_add_tail(&tmp1, &_Header) ;
		list_del_init(&_Header) ;
		
		struct list_head *pos,*next;
		list_for_each_safe(pos,next,&tmp1) {
			value_node *node = list_entry(pos, struct list_nodeval , hdr) ;
			list_del_init(pos) ;
			_insert_desc(node) ;
		}
	}

	void merge(_Myt& _Right)
	{
		ND_TRACE_FUNC() ;
		splice(const_iterator(_Header.prev, &_Header),  _Right);
	}
protected:
	void _insert_desc(value_node *insertnode)
	{
		ND_TRACE_FUNC() ;
		struct list_head *pos,*next;
		list_for_each_safe(pos,next,&_Header) {
			value_node *node = list_entry(pos, struct list_nodeval , hdr) ;
			if (!(node->listval < insertnode->listval) ){
				list_add_tail(&insertnode->hdr,&node->hdr) ;
				return ;
			}
		}
		list_add_tail(&insertnode->hdr,&_Header) ;
	}
	size_type _cur_number;
	list_head _Header ;
	allocator_type _Allocator ;
};


#endif
