/* file nd_stdset.h
 *
 * stl-set
 *
 * create by duan 
 *
 * 2012/7/2 18:03:39
 */

#ifndef _ND_STDSET_H_
#define _ND_STDSET_H_


template< class _Ttype  >
class nd_stlset : public _nd_rbtree<_Ttype>
{
public:
	typedef _Ttype				value_type;	
	typedef value_type*			pointer;	
	typedef const value_type*	const_pointer;	
	typedef value_type&			reference;	
	typedef const value_type&	const_reference;	
	typedef size_t				size_type;	
	typedef ptrdiff_t			difference_type;

	typedef _nd_rbtree<_Ttype> _MyBase ;
	typedef nd_stlset<_Ttype> _Myt ;

	typedef typename _MyBase::value_node value_node;

	typedef typename _MyBase::iterator iterator ;
	typedef typename _MyBase::reverse_iterator reverse_iterator;

	typedef typename _MyBase::const_iterator const_iterator;
	typedef typename _MyBase::const_reverse_iterator const_reverse_iterator;

	nd_stlset(nd_handle pool):m_alInst(pool)
	{

	}
	nd_stlset() 
	{

	}
	~nd_stlset() 
	{
	}
	nd_stlset (const _Myt&r):_MyBase(r)
	{
	}
	typedef nd_poolalloc<value_node> allocator_type ;
	void _set_pool(nd_handle pool) {m_alInst._set_pool(pool);}
	//////////////////////////////////////////////////////////////////////////

	iterator find(const value_type& keyval) 
	{
		return _MyBase::search(keyval );
	}

	const_iterator find(const value_type& keyval) const
	{
		iterator it =  _MyBase::search(keyval );
		return const_iterator(it.get_addr()) ;
	}

	iterator insert(const value_type &vt)
	{
		ND_TRACE_FUNC() ;
		value_node *ins_node = _create_obj(vt) ;
		if (!ins_node){
			return iterator(NULL);
		}
		if(_MyBase::insert( ins_node) ) {
			return iterator(ins_node) ;
		}
		_destroy_obj(ins_node) ;
		return iterator(NULL);
	}

	bool erase(const value_type &val)
	{
		ND_TRACE_FUNC() ;
		iterator it = _MyBase::search( val );
		if (it != _MyBase::end() ){
			erase( it );
			return true ;
		}
		return false;
	}

	iterator erase(iterator &erase_it)
	{
		ND_TRACE_FUNC() ;
		iterator next_it = _MyBase::_erase(erase_it) ;
		_destroy_obj(erase_it.get_addr()) ;
		return next_it;
	}
	reverse_iterator erase(reverse_iterator &erase_it)
	{
		ND_TRACE_FUNC() ;
		reverse_iterator next_it = _MyBase::_erase(erase_it) ;
		_destroy_obj(erase_it.get_addr()) ;
		return next_it;
	}

	void clear() 
	{
		ND_TRACE_FUNC() ;
		for(iterator it =_MyBase::begin(); it!=_MyBase::end(); ) {
			it = erase(it);
			if (_MyBase::size()==0 || it == _MyBase::end()){
				break ;
			}
		}
	}
// 	value_type& operator[](const value_key& _Keyval)
// 	{	
// 		iterator _Where = this->search(_Keyval) ;
// 		if (_Where == this->end())
// 			_Where = this->insert(_Keyval, value_type());
// 		return ((*_Where).second);
// 	}
	size_t capacity() {return (size_t)m_alInst.max_size();}
	
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
		return ins_node;
	}
	value_node* _create_obj(const value_type &_val) 
	{
		ND_TRACE_FUNC() ;
		value_node *ins_node =  m_alInst.allocate(1) ;
		if (!ins_node){
			return NULL;
		}
		return new(ins_node) value_node( _val) ;
	}
	void _destroy_obj(value_node *node) 
	{
		ND_TRACE_FUNC() ;
		m_alInst.destroy(node) ;
		m_alInst.deallocate(node,1);
	}

	allocator_type m_alInst;	// allocator object for values

};

#endif
