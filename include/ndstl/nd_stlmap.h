/* file nd_stlmap.h
 *
 * ɽկstl map
 *
 * create by duan
 * 2012/6/21 18:28:31
 */

#ifndef _ND_STLMAP_H_
#define _ND_STLMAP_H_


#include "nd_common/nd_common.h"
#include "nd_allocator.h"
#include "ndapplib/nd_rbtree.h"


template<class _Tfirst, class _Ttype  >
class nd_stlmap : public nd_refcount_rbtree<_Tfirst,_Ttype>
{
public:
	typedef nd_refcount_rbtree<_Tfirst,_Ttype> _MyBase ;
	typedef nd_stlmap<_Tfirst,_Ttype> _Myt ;

	typedef nd_pair<typename _MyBase::iterator, bool> _Pairib;
	typedef typename _MyBase::iterator iterator ;
	nd_stlmap(nd_handle pool):_MyBase(pool) 
	{

	}
	nd_stlmap():_MyBase(0) 
	{

	}
	~nd_stlmap() 
	{
		_MyBase::clear() ;
	}
	nd_stlmap (const _Myt&r):_MyBase(r)
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
		iterator _Where = _MyBase::search(_Keyval) ;
		if (_Where == _MyBase::end())
			_Where = _MyBase::insert(_Keyval, _Ttype());
		return ((*_Where).second);
	}
};

#endif
