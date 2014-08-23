/* file nd_vector.h
 *
 * stl-vector 
 *
 * create by duan 
 *
 * 2012/6/15 10:38:00
 */

#ifndef _ND_VECTOR_H_
#define _ND_VECTOR_H_

#include "nd_common/nd_common.h"
#include "nd_allocator.h"
#include "ndapplib/nd_object.h"
#include "ndapplib/nd_utility.h"

template<class  _TContainor>
class nd_vector_iterator_base
{
public:
 	typedef typename _TContainor::pointer _Tptr;
	typedef std::bidirectional_iterator_tag iterator_category;
	typedef typename _TContainor::value_type value_type;
	typedef typename _TContainor::difference_type difference_type;
	typedef typename _TContainor::const_pointer pointer;
	typedef typename _TContainor::const_reference reference;
	typedef typename _TContainor::size_type size_type;

	typedef nd_vector_iterator_base<_TContainor> _Myiter;
	nd_vector_iterator_base() : _ptr(0) {}
	nd_vector_iterator_base(const _Tptr p) : _ptr(p) {}
	
	bool operator==(const _Myiter& _Right) const{ return _ptr == _Right._ptr;	}
	bool operator!=(const _Myiter& _Right) const{	return (!(_ptr == _Right._ptr));}
	value_type& operator*() const{return *_ptr;}
	value_type* operator->() const{return _ptr;}
	_Tptr _ptr;
};

template<class  _TContainor>
class nd_vector_iterator : public nd_vector_iterator_base<_TContainor>
{
public:
	typedef nd_vector_iterator_base<_TContainor> _Mybase ;
	typedef nd_vector_iterator<_TContainor> _Myiter;
	typedef typename _Mybase::_Tptr _Tptr ;
	nd_vector_iterator() : _Mybase() {}
	nd_vector_iterator(const _Tptr p) : _Mybase(p) {}
	nd_vector_iterator(const _Myiter &r) : _Mybase(r._ptr) {}

	_Myiter& operator++()
	{
		++_Mybase::_ptr;
		return *this ;
	}
	_Myiter& operator--()
	{
		--_Mybase::_ptr ;
		return *this ;
	}
	_Myiter operator++(int)
	{
		_Myiter _Tmp = *this;
		++*this;
		return (_Tmp);
	}
	_Myiter operator--(int)
	{
		_Myiter _Tmp = *this;
		--*this;
		return (_Tmp);
	}
	friend const size_t operator-(const _Myiter& left,const _Myiter& _Right)
	{
		return left._Mybase::_ptr - _Right._Mybase::_ptr ;
	}

	friend _Myiter operator-(const _Myiter& left,int n)
	{
		_Myiter tmp = left ;
		tmp._Mybase::_ptr -= n ;
		return tmp;
	}
	friend _Myiter operator+(const _Myiter& left,int n)
	{
		_Myiter tmp = left ;
		tmp._Mybase::_ptr += n ;
		return tmp;
	}
};
template<class  _TContainor>
class nd_reserve_vector_iterator:public nd_vector_iterator_base<_TContainor>
{
public:
	typedef nd_vector_iterator_base<_TContainor> _Mybase ;
	typedef nd_reserve_vector_iterator<_TContainor> _Myiter;
	typedef typename _Mybase::_Tptr _Tptr ;

	nd_reserve_vector_iterator() : _Mybase() {}
	nd_reserve_vector_iterator(const _Tptr p) : _Mybase(p) {}
	nd_reserve_vector_iterator(const _Myiter &r) : _Mybase(r._ptr) {}
	_Myiter& operator++()
	{
		--_Mybase::_ptr;
		return *this ;
	}
	_Myiter& operator--()
	{
		++_Mybase::_ptr ;
		return *this ;
	}
	_Myiter operator++(int)
	{
		_Myiter _Tmp = *this;
		++*this;
		return (_Tmp);
	}
	_Myiter operator--(int)
	{
		_Myiter _Tmp = *this;
		--*this;
		return (_Tmp);
	}

	friend size_t operator-(_Myiter& left, _Myiter& _Right)
	{
		return _Right._Mybase::_ptr-left._Mybase::_ptr  ;
	}

	friend _Myiter operator-(_Myiter& left,int n)
	{
		_Myiter tmp = left ;
		tmp._Mybase::_ptr += n ;
		return tmp;
	}
	friend _Myiter operator+(_Myiter& left,int n)
	{
		_Myiter tmp = left ;
		tmp._Mybase::_ptr -= n ;
		return tmp;
	}
};

template<class  _Ttype>
class nd_stlvector
{
public:
	typedef _Ttype				value_type;	
	typedef value_type*			pointer;
	typedef const value_type*	const_pointer;	
	typedef value_type&			reference;	
	typedef const value_type&	const_reference;	
	typedef size_t				size_type;	
	typedef ptrdiff_t			difference_type;
	typedef nd_stlvector< value_type> _Myt ;
	typedef nd_poolalloc<value_type> allocator_type;

	typedef nd_vector_iterator<_Myt> iterator ;
	typedef iterator const_iterator ;
	typedef nd_reserve_vector_iterator<_Myt> reverse_iterator;
	typedef reverse_iterator const_reverse_iterator;
	enum {E_DEFAULT_ALLOC_NUM = 8};
	nd_stlvector(nd_handle pool=NULL):_Allocator(pool),_capacity(0), _start_ptr(0),_cur_num(0)
	{
	}
	void _set_pool(nd_handle pool) {_Allocator._set_pool(pool);}
	virtual ~nd_stlvector()
	{
		ND_TRACE_FUNC() ;
		clear();
	}
	nd_stlvector (const _Myt&r):_Allocator(((_Myt&)r).get_allocator()),_capacity(0), _start_ptr(0),_cur_num(0)
	{
		ND_TRACE_FUNC() ;
		if (r.size() ){
			_Insert_n(end(),(iterator)r.begin(),(iterator)r.end()) ;
		}else {clear() ;}
	}
#ifdef _MSC_VER
	nd_stlvector (_Myt&&r):_Allocator(r.get_allocator()),_capacity(0), _start_ptr(0),_cur_num(0)
	{
		ND_TRACE_FUNC() ;
		if (r.size() ){
			_Insert_n(end(),(iterator)r.begin(),(iterator)r.end()) ;
		}else {clear() ;}
	}
	_Myt& operator=(_Myt&& _Right)
	{
		ND_TRACE_FUNC() ;
		if (size() ){
			erase(begin(), end()) ;
		}
		_Insert_n(end(),(iterator)_Right.begin(),(iterator)_Right.end()) ;
		return *this ;
	}
#endif
	//////////////////////////////////////////////////////////////////////////

	_Myt& operator=(const  _Myt& _Right)
	{
		ND_TRACE_FUNC() ;
		if (size() ){
			erase(begin(), end()) ;
		}
		_Insert_n(end(),(iterator)_Right.begin(),(iterator)_Right.end()) ;
		return *this ;
	}


	void resize(size_type _Newsize)
	{	
		ND_TRACE_FUNC() ;
		if (_Newsize < size())
			erase(begin() + (int)_Newsize, end());	
		else if (_Newsize > size() )	{
			_Insert_n(end(), _Newsize - size(), _Ttype());
		}
	}

	void resize(size_type _Newsize, _Ttype _Val)
	{
		ND_TRACE_FUNC() ;
		if (_Newsize < size())
			erase(begin() + _Newsize, end());	
		else if (_Newsize > size() )	{
			_Insert_n(end(), _Newsize - size(), _Val);
		}
	}

	size_type capacity() const
	{
		return _capacity ;
	}

	iterator begin()
	{
		return (iterator(this->_start_ptr));
	}

	const_iterator begin() const
	{
		return (const_iterator(this->_start_ptr));
	}

	iterator end()
	{
		return (iterator(this->_start_ptr + _cur_num));
	}

	const_iterator end() const
	{
		return (const_iterator(this->_start_ptr+_cur_num));
	}


	reverse_iterator rbegin()
	{	
		return reverse_iterator(this->_start_ptr +_cur_num-1);
	}

	const_reverse_iterator rbegin() const
	{
		return const_reverse_iterator(this->_start_ptr +_cur_num-1);
	}

	reverse_iterator rend()
	{
		return reverse_iterator(this->_start_ptr -1);
	}

	const_reverse_iterator rend() const
	{
		return const_reverse_iterator(this->_start_ptr -1);
	}


	size_type size() const
	{
		return _cur_num ;
	}

	size_type max_size() const
	{	
		return (this->_Allocator.max_size());
	}

	bool empty() const
	{
		return (_cur_num == 0 );
	}

	allocator_type &get_allocator() 
	{
		return (this->_Allocator);
	}

	const_reference at(size_type _Pos) const
	{
		return (*(this->_start_ptr + _Pos));
	}

	reference at(size_type _Pos)
	{
		return (*(this->_start_ptr + _Pos));
	}

	const_reference operator[](size_type _Pos) const
	{
		return (*(this->_start_ptr + _Pos));
	}

	reference operator[](size_type _Pos)
	{	
		return (*(this->_start_ptr + _Pos));
	}


	value_type& front()
	{
		return (*begin());
	}

	const value_type& front() const
	{
		return (*begin());
	}

	reference back()
	{
		return (*(end() - 1));
	}

	const value_type& back() const
	{
		return (*(end() - 1));
	}

	void push_back(const_reference _Val)
	{
		_Insert_n(end(),(size_type)1, _Val) ;
	}

	void pop_back()
	{
		if (!empty())	{
			_Allocator.destroy(&_start_ptr[--_cur_num]) ;
		}
	}


	iterator insert(const_iterator _Where, const_reference _Val)
	{
		ND_TRACE_FUNC() ;
		size_type _Off = size() == 0 ? 0 : _Where - begin();
		_Insert_n(_Where, (size_type)1, _Val);
		return (begin() + (int)_Off);
	}

	void insert(const_iterator _Where, size_type _Count, const_reference _Val)
	{
		ND_TRACE_FUNC() ;
		_Insert_n(_Where, _Count, _Val);
	}

	iterator erase(iterator _Where)
	{	
		ND_TRACE_FUNC() ;
		erase(_Where,	_Where +1) ;
		return _Where;
	}

	iterator erase(iterator _First,	iterator _Last)
	{	
		ND_TRACE_FUNC() ;
		if (_First != _Last)
		{
			size_type distance = _Last - _First;
			size_type start_pos = _First - begin() ;
			
			for(size_type i=0; i<distance; ++i) {
				_Allocator.destroy(&_start_ptr[start_pos + i] );
				
			}
			int tail_num = (int) (_cur_num -start_pos - distance );
			for(int i=0; i<tail_num; i++) {
				_Allocator.construct(&_start_ptr[start_pos + i] , _start_ptr[start_pos + i + distance] );
				_Allocator.destroy(&_start_ptr[start_pos + distance +i] );
			}
			_cur_num -= distance;
			/*
			size_type erasenum = _Last - _First ;
			iterator start_it = _First ;
			for(size_type i=0; i<erasenum; i++) {
				_Allocator.destroy(&*(_First++)) ;					
			}
			for(iterator it= _Last; it!=end(); ++it, ++start_it) {
				*start_it = *it ;
			}
			_cur_num -= erasenum ;
			*/
		}
		return (_Make_iter(_First));
	}

	void clear()
	{	// erase all
		ND_TRACE_FUNC() ;
		//erase(begin(), end());
		if(_start_ptr) {
			_Destruct(_start_ptr,(int)_cur_num) ;
			//_Allocator.deallocate(_start_ptr,_capacity) ;
			_start_ptr = 0 ;
			_capacity = 0 ;
			_cur_num = 0;
		}
	}

	void swap(_Myt& _Right)
	{	// exchange contents with _Right
		ND_TRACE_FUNC() ;
		if (this == &_Right)
			;	// same object, do nothing
		else if (this->_Allocator == _Right._Allocator)	{
			__swap(_start_ptr, _Right._start_ptr) ;
			__swap(_capacity, _Right._capacity) ;
			__swap(_cur_num, _Right._cur_num) ;
		}
		else {
			size_type cap = _capacity ,num = _cur_num ;
			pointer _ptr = _start_ptr ;

			_start_ptr = 0 ;
			_capacity = 0;
			_cur_num = 0 ;
			_Insert_n(end(),_Right.begin(), _Right.end() );

			_Right.clear() ;
			_Right._Insert_n(end(), iterator(_ptr), iterator(_ptr + num)) ;
			_Destruct(_ptr,(int)num) ;
		}
	}

	void reserve(size_type _Count)
	{
		ND_TRACE_FUNC() ;
		if (max_size() < _Count)
			return ;
		else if (capacity() < _Count)	{	
			//remalloc
			pointer newaddr = _Construct(_Count) ;
			
			if (_start_ptr) {
				for(int i=0; i<_cur_num; i++) {
					_Allocator.construct(&newaddr[i] , _start_ptr[i] );
				}
				_Destruct(_start_ptr,_cur_num) ;
			}
			_start_ptr = newaddr ;
			_capacity = _Count ;
		}
	}
	//////////////////////////////////////////////////////////////////////////
protected:
	pointer _Construct(size_type n) 
	{
		ND_TRACE_FUNC() ;
		return _Allocator.allocate(n) ;
// 		pointer _addr = _Allocator.allocate(n) ;
// 		if (!_addr){
// 			return 0 ;
// 		}
// 		for(size_type i=0; i<n; ++i) {
// 			pointer tmp = new(_addr+ i) value_type ;
// 		}
// 		return _addr ;
	}
	void _Destruct(pointer _addr, size_type number = 0) 
	{
		ND_TRACE_FUNC() ;
		for(size_type i=0; i<number; i++)
			_Allocator.destroy(&_addr[i]) ;
		_Allocator.deallocate(_addr,1) ;
	}

	void _Insert_n(const_iterator _Where,size_type _Count, const value_type& _Val)
	{
		ND_TRACE_FUNC() ;
		int need_realloc = 0;
		size_type empty_num =0 ,alloc_num = _capacity ;
		if (alloc_num ==0)	{
			alloc_num = E_DEFAULT_ALLOC_NUM ;
			need_realloc = 1;
		}
		do {
			empty_num = alloc_num - _cur_num ;
			if (alloc_num > max_size())	{
				return ;//error
			}
			else if (empty_num < _Count)	{
				need_realloc = 1;
				alloc_num = alloc_num << 1 ;
			}
			else {
				break;
			}
		} while (alloc_num < max_size());

		if (_start_ptr == 0 ){
			//初始化
			_start_ptr = _Construct(alloc_num) ;
			for(size_type i =0; i<_Count; i++) {
				_Allocator.construct(_start_ptr+i,_Val) ;
				++_cur_num ;
			}
			_capacity = alloc_num ;
			return ;
		}
		else if (!need_realloc){
			//剩余空间够用
			size_type distance = end() - _Where;
			size_type insert_pos = _Where - begin() ;
			for(size_type i=0; i<distance; ++i) {

				_Allocator.construct(&_start_ptr[_cur_num + _Count - i -1], _start_ptr[_cur_num -1 - i] ) ;
				_Allocator.destroy(&_start_ptr[_cur_num -1 - i] ) ;
			}
			for(size_type i=0; i<_Count; i++) {
				_Allocator.construct(&_start_ptr[insert_pos +i] ,_Val );
			}
			_cur_num += _Count;
		}
		else {
			//remalloc
			pointer newaddr = _Construct(alloc_num) ;
			iterator it ;
			int number = 0 ;
			for(it = begin(); it != _Where; ++it) {
				_Allocator.construct(&newaddr[number++] , *it );
			}
			for(size_type i=0; i<_Count; i++) {
				_Allocator.construct(&newaddr[number++] , _Val );
			}
			for(; it != end(); ++it) {
				_Allocator.construct(&newaddr[number++] , *it );
			}
			_Destruct(_start_ptr,(int)_cur_num) ;
			_start_ptr = newaddr ;
			_capacity = alloc_num ;
			_cur_num = number;
		}
	}

	void _Insert_n(const_iterator _Where, iterator _First, iterator _Last)
	{
		ND_TRACE_FUNC() ;
		int need_realloc = 0;
		size_type empty_num =0 ,alloc_num = _capacity ;
		size_type _Count = _Last - _First;

		if (alloc_num ==0)	{
			alloc_num = E_DEFAULT_ALLOC_NUM ;
			need_realloc = 1;
		}
		do {
			empty_num = alloc_num - _cur_num ;
			if (alloc_num > max_size())	{
				return ;//error
			}
			else if (empty_num < _Count)	{
				need_realloc = 1;
				alloc_num = alloc_num << 1 ;
			}
			else {
				break;
			}
		} while (alloc_num < max_size());

		if (_start_ptr == 0 ){
			//初始化
			_start_ptr = _Construct(alloc_num) ;
			for(int i =0; i<_Count; i++) {
				_Allocator.construct(&_start_ptr[_cur_num++], *(_First + (int)i) );
			}
			_capacity = alloc_num ;
			return ;
		}
		else if (!need_realloc){
			//剩余空间够用
			size_type distance = end() - _Where;
			size_type insert_pos = _Where - begin() ;
			//iterator pos = _Where + distance;
			for(int i=0; i<distance; ++i) {
				_Allocator.construct(&_start_ptr[_cur_num + _Count - i -1], _start_ptr[_cur_num -1 - i] ) ;
				_Allocator.destroy(&_start_ptr[_cur_num -1 - i] ) ;

			}
			for(int i=0; i<_Count; i++) {
				_Allocator.construct(&_start_ptr[insert_pos +i] , *(_First + i) );
			}
			_cur_num += _Count;
		}
		else {
			//remalloc
			pointer newaddr = _Construct(alloc_num) ;
			iterator it ;
			int number = 0 ;
			for(it = begin(); it != _Where; ++it) {
				_Allocator.construct(&newaddr[number++] , *it );
			}
			for(int i=0; i<_Count; i++) {
				_Allocator.construct(&newaddr[number++] , *(_First + i) ); 
			}
			for(; it != end(); ++it) {
				_Allocator.construct(&newaddr[number++] , *it );
			}
			_Destruct(_start_ptr,_cur_num) ;
			_start_ptr = newaddr ;
			_capacity = alloc_num ;
			_cur_num = number;
		}
	}
	iterator _Make_iter(const_iterator _Where) const
	{	
		return (iterator(_Where._ptr));
	}
	allocator_type _Allocator ;
	size_type _capacity, _cur_num;
	pointer _start_ptr ;
};
#endif
