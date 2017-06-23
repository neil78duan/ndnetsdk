/* file nd_utility.h
 *
 * utility of nd-stl
 *
 * create by duan 
 *
 * 2012/6/22 10:18:41
 *
 */

#ifndef _ND_UTILITY_H_ 
#define _ND_UTILITY_H_

#include <cstddef>
#include <new>
#include <iterator>

#include "nd_common/nd_comcfg.h"

#ifdef ND_CALLSTACK_TRACE

class NDCallTrace
{
public:
    NDCallTrace(const char *funcname)
    {
        m_ret = push_func(funcname) ;
		pName = funcname;
    }
    ~NDCallTrace()
    {
        if(m_ret==0) {
            pop_func(pName) ;
        }
    }
private:
    int m_ret ;
	const char *pName;
};

#define ND_TRACE_FUNC() NDCallTrace _tmp_func_trace(__FUNC__)
#define ND_TRACE_FUNC_EX(name) NDCallTrace name(__FUNC__)

#else

struct NDCallTrace
{
	NDCallTrace(const char *funcname)
	{
	}
};
#define ND_TRACE_FUNC() //
#define ND_TRACE_FUNC_EX(name) //
#endif

template<class T> inline
	void _NDDestroy(T *p)
{	
	p->~T();
}


template<class T> inline
	void _NDDestroy(char *p)
{
}

template<> inline
	void _NDDestroy(wchar_t *)
{	
}
// 
// #ifndef __PLACEMENT_NEW_INLINE
// #define __PLACEMENT_NEW_INLINE
// 
// #if !defined(ND_UNIX)  
// inline void *__cdecl operator new(size_t, void *_P)
// {
// 	return (_P); 
// }
// 
// inline void __cdecl operator delete(void *, void *)
// {
// 	return; 
// }
// #endif
// #endif

template <class T>
void __swap(T &left, T &right)
{
	T tmp = left; 
	left = right ;
	right = tmp ;
}

template<class _Tfirst, class _Tsecond>
struct nd_pair{
	typedef nd_pair<_Tfirst, _Tsecond> _Myt;
	//typedef typename _Tfirst first_type;
	//typedef typename _Tsecond second_type;

	nd_pair(): first(_Tfirst()), second(_Tsecond())
	{
	}

	nd_pair(const _Myt& _Right)
		: first(_Right.first), second(_Right.second)
	{
	}
#ifdef _MSC_VER
	nd_pair( _Myt&& _Right)
		: first(_Right.first), second(_Right.second)
	{
	}
#endif
	nd_pair(const _Tfirst& _Val1, const _Tsecond& _Val2)
		: first(_Val1), second(_Val2)
	{
	}

	_Tfirst first ;
	_Tsecond second;
};

template<class _Type>
struct nd_unrefwrap
{
	typedef _Type type;
};

template<class _Ty>
struct nd_identity
{
	typedef _Ty type;
	const _Ty& operator()(const _Ty& _Left) const
	{
		return (_Left);
	}
};

// TEMPLATE FUNCTION forward
#ifdef _MSC_VER
template<class _Ty> inline
	_Ty&& nd_forward(typename nd_identity<_Ty>::type& _Arg)
{
	return ((_Ty&&)_Arg);
};

template<class _Ty1,class _Ty2> inline
	nd_pair<typename nd_unrefwrap<_Ty1>::type,	typename nd_unrefwrap<_Ty2>::type>
	nd_make_pair(_Ty1&& _Val1, _Ty2&& _Val2)
{	// return pair composed from arguments
	typedef nd_pair<typename nd_unrefwrap<_Ty1>::type,
		typename nd_unrefwrap<_Ty2>::type> _Mypair;
	return (_Mypair( nd_forward<_Ty1>(_Val1),	 nd_forward<_Ty2>(_Val2)));
};

template<class _Ty1,
class _Ty2> inline
	nd_pair<typename nd_unrefwrap<_Ty1>::type,
	typename nd_unrefwrap<_Ty2>::type>
	nd_make_pair(_Ty1&& _Val1, const _Ty2& _Val2)
{	// return pair composed from arguments
	typedef nd_pair<typename nd_unrefwrap<_Ty1>::type,
		typename nd_unrefwrap<_Ty2>::type> _Mypair;
	return (_Mypair(nd_forward<_Ty1>(_Val1),
		nd_forward<_Ty2>(_Val2)));
};

template<class _Ty1,
class _Ty2> inline
	nd_pair<typename nd_unrefwrap<_Ty1>::type,
	typename nd_unrefwrap<_Ty2>::type>
	nd_make_pair(const _Ty1& _Val1, _Ty2&& _Val2)
{	// return pair composed from arguments
	typedef nd_pair<typename nd_unrefwrap<_Ty1>::type,
		typename nd_unrefwrap<_Ty2>::type> _Mypair;
	return (_Mypair( nd_forward<_Ty1>(_Val1),
		nd_forward<_Ty2>(_Val2)));
};

#else 
template<class _Ty> inline
	const _Ty& nd_forward(const typename nd_identity<_Ty>::type& _Arg)
{
	return ((const _Ty&)_Arg);
};
#endif 

template<class _Ty1,
class _Ty2> inline
	nd_pair<typename nd_unrefwrap<const _Ty1>::type,
	typename nd_unrefwrap<const _Ty2>::type>
	nd_make_pair(const _Ty1& _Val1, const _Ty2& _Val2)
{	// return pair composed from arguments
	typedef nd_pair<typename nd_unrefwrap<const _Ty1>::type,
		typename nd_unrefwrap<const _Ty2>::type> _Mypair;
	return (_Mypair( nd_forward<const _Ty1>(_Val1),
		 nd_forward<const _Ty2>(_Val2)));
};


// 
// #define ND_STD_MAP(_first, _second) std::map<_first, _second,std::less<_first>,nd_stlalloc<std::pair<const _first, _second> > >
// #define ND_STD_VECTOR(_type)		std::vector<_type,nd_stlalloc<_type> >
// #define ND_STD_SET(_type)			std::set<_type, std::less<_type> ,nd_stlalloc<_type 
// #define ND_STD_LIST(_type)			std::list<_type,nd_stlalloc<_type> >
// #define ND_STD_DEQUE(_type)			std::deque<_type,nd_stlalloc<_type> >
// #ifdef _MSC_VER
// #include <xstring>
// #endif 
// #define ndstring					std::basic_string<char, char_traits<char>, nd_stlalloc<char> >
// #else 

#define ND_STD_MAP(_first, _second)			std::map<_first, _second>
#define ND_STD_VECTOR(_type)				std::vector<_type>
#define ND_STD_SET(_type)					std::set<_type>
#define ND_STD_LIST(_type)					std::list<_type>
#define ND_STD_DEQUE(_type)					std::deque<_type>
#define ndstring							std::string

#endif
