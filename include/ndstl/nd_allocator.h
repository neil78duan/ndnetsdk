/* nd_allocator.h
 *
 * allocator of nd app
 * 2009-5-24 21:50
 */ 


#ifndef _ND_ALLOCATOR_H_
#define _ND_ALLOCATOR_H_

//implemention of nd_stdallocate
#define USE_ND_ALLOCATOR 


// #include <exception>
// #include <new>
// #include <xutility>
#include "ndstl/nd_utility.h"
//#include "ndstl/nd_object.h"
#include <exception>
#include "nd_common/nd_common.h"

void *_malloc_pool(size_t size, void *pool=NULL)  ;
void _free_pool(void *p,void *pool=NULL) ;

#define _ND_ROUNTD(size) 	(((size)+7) & (~7)) 

/* static allocator* 
 */
template<class T ,  int _numbers>
class static_alloc {
	struct _hdr_list{
		struct _hdr_list *next ;
	};

public :
	typedef T                 value_type;	
	typedef value_type*       pointer;	
	typedef const value_type* const_pointer;	
	typedef value_type&       reference;	
	typedef const value_type& const_reference;	
	typedef size_t       size_type;	
	typedef ptrdiff_t    difference_type;	

	static_alloc()  {
		ND_TRACE_FUNC() ;
		_hdr_list * list ;
		_M_free = NULL ;
		for(int i=0; i< _numbers; i++ ) {
			list =(struct _hdr_list *) _mm_pool[i] ; 
			list->next = _M_free ;
			_M_free = list ;
		}		
	}
	~static_alloc() {} 
	
	T *allocate(size_type n=1) {
		ND_TRACE_FUNC() ;
		if(_M_free) {
			_hdr_list * ret = (_hdr_list *)_M_free ;
			_M_free = _M_free->next ;
			return new((void*)ret) T ;
		}
		return NULL ;
	}
	T *allocate(const T& _val) {
		ND_TRACE_FUNC() ;
		if(_M_free) {
			_hdr_list * ret = (_hdr_list *)_M_free ;
			_M_free = _M_free->next ;
			return new((void*)ret) T(_val) ;
		}
		return NULL ;
	}
	
	void deallocate(T *p,size_type) {
		ND_TRACE_FUNC() ;
		char *start_addr = (char*) _mm_pool ;
		if((char*)p>=start_addr && (char*)p<(start_addr + sizeof(_mm_pool))) {			
			
			_hdr_list * addr = (_hdr_list *)p ;
			//p->~T() ;
			_NDDestroy(p) ;
			addr->next = _M_free ;
			_M_free = addr ;
		}
		else {
			nd_assert(0) ;
		}
	}

	pointer address(reference x) const { return &x; }

	const_pointer address(const_reference x) const {return x;}

	size_type max_size() const {
		return _numbers;
	}	

	void construct(pointer p, const value_type& x) {
		new(p) value_type(x);
	}

	void destroy(pointer p) { _NDDestroy(p) ; }
private:
	struct _hdr_list * _M_free ;
	char _mm_pool[_numbers][_ND_ROUNTD(sizeof(T))] ;

} ;

#ifdef USE_ND_ALLOCATOR


///////////////////////////////////////////////////////////////////////
/* use nd-mempool replace stl allocator*/
template <class T> 
class nd_stlalloc
{
public:
	typedef T                 value_type;	
	typedef value_type*       pointer;	
	typedef const value_type* const_pointer;	
	typedef value_type&       reference;	
	typedef const value_type& const_reference;	
	typedef size_t       size_type;	
	typedef ptrdiff_t    difference_type;	

	template <class U>	
	struct rebind { typedef nd_stlalloc<U> other; };	
	
	nd_stlalloc()
	{

	}
	nd_stlalloc(const nd_stlalloc<T>&) throw ()
	{	
	}

	template<class _Other>
	nd_stlalloc(const nd_stlalloc<_Other>&) throw ()
	{
	}

	template<class _Other>
	nd_stlalloc<T>& operator=(const nd_stlalloc<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}
	
	~nd_stlalloc() {}
		
	pointer address(reference x) const { return &x; }
	
	const_pointer address(const_reference x) const {return x;}

	pointer allocate(size_type n) 
	{
		ND_TRACE_FUNC() ;
		void* p = _malloc_pool(n * sizeof(T));		
// 		if (!p)
// 			throw std::bad_alloc();		
		return static_cast<pointer>(p);	
	}

	pointer allocate(size_type n, const void *)
	{	// allocate array of n elements, ignore hint
		return (allocate(n));
	}

	void deallocate(pointer p, size_type) { 
		ND_TRACE_FUNC() ;
		_free_pool(p); 
	}	
	size_type max_size() const {
		return static_cast<size_type>(-1) / sizeof(T);
	}	
	
	void construct(pointer p, const value_type& x) {
		ND_TRACE_FUNC() ;
		new((void*)p) value_type(x);
	}
	void destroy(pointer p) {
		ND_TRACE_FUNC() ;
		_NDDestroy(p); 
	}
private:
};

template<> class nd_stlalloc<void>	
{	
	typedef void        value_type;	
	typedef void*       pointer;	
	typedef const void* const_pointer;
	
	template <class U>
	
	struct rebind { typedef nd_stlalloc<U> other; };
	
};

template <class T>
inline bool operator==(const nd_stlalloc<T>&, const nd_stlalloc<T>&)
{
	return true;
}

template <class T>
inline bool operator!=(const nd_stlalloc<T>&, const nd_stlalloc<T>&) 
{
	return false;
}
// 
// template<class _Ty>
// struct nd_less	
// {	
// 	bool operator()(const _Ty& _Left, const _Ty& _Right) const
// 	{	
// 		return (_Left < _Right);
// 	}
// };

#endif	//USE_ND_ALLOCATOR

#include "nd_common/nd_common.h"
nd_handle nd_stlmmpool () ;

template <class T> 
class nd_poolalloc
{
public:
	typedef T                 value_type;	
	typedef value_type*       pointer;	
	typedef const value_type* const_pointer;	
	typedef value_type&       reference;	
	typedef const value_type& const_reference;	
	typedef size_t       size_type;	
	typedef ptrdiff_t    difference_type;	
	typedef nd_poolalloc<T> _MyType ;
	nd_poolalloc(nd_handle pool )
	{
		ND_TRACE_FUNC() ;
		if (pool){
			m_pool = pool;
		}
		else {
			m_pool = nd_stlmmpool();
		}
// 		if (pool){
// 			m_pool = pool ;
// 			m_bcreate_pool = 0 ;
// 		}
// 		else {
// 			m_pool = nd_pool_create(EMEMPOOL_UNLIMIT,__FUNC__) ;
// 			if(m_pool) {
// 				m_bcreate_pool = 1 ;
// 			}
// 		}
	}
	nd_poolalloc(const _MyType &r )
	{
		m_pool = r.m_pool;
// 		if (r.m_bcreate_pool==0)	{
// 			m_pool = r.m_pool ;
// 			m_bcreate_pool = 0 ;
// 		}
// 		else {
// 			m_pool = nd_pool_create(EMEMPOOL_UNLIMIT,__FUNC__) ;
// 			if(m_pool) {
// 				m_bcreate_pool = 1 ;
// 			}
// 		}
	}

	template <class U>	
	struct rebind { typedef nd_poolalloc<U> other; };	

	template<class _Other>
	nd_poolalloc(const nd_stlalloc<_Other>&r) throw ()
	{
		m_pool = r.m_pool;
// 		if (r.m_bcreate_pool==0)	{
// 			m_pool = r.m_pool ;
// 			m_bcreate_pool = 0 ;
// 		}
// 		else {
// 			m_pool = nd_pool_create(EMEMPOOL_UNLIMIT,__FUNC__) ;
// 			if(m_pool) {
// 				m_bcreate_pool = 1 ;
// 			}
// 		}
	}
	
	template<class _Other>
	nd_poolalloc<T>& operator=(const nd_poolalloc<_Other>&r)
	{
		m_pool = r.m_pool;
// 		if (r.m_bcreate_pool==0)	{
// 			_set_pool(NULL);
// 			m_pool = r.m_pool ;
// 			m_bcreate_pool = 0 ;
// 		}
// 		else {
// 			m_pool = nd_pool_create(EMEMPOOL_UNLIMIT,__FUNC__) ;
// 			if(m_pool) {
// 				m_bcreate_pool = 1 ;
// 			}
// 		}
		return (*this);
	}

	template<class _Other>
	nd_poolalloc<T>& operator=(nd_poolalloc<_Other>&r)
	{
		m_pool = r.m_pool;
// 		if (r.m_bcreate_pool==0)	{
// 			_set_pool(NULL);
// 			m_pool = r.m_pool ;
// 			m_bcreate_pool = 0 ;
// 		}else {
// 			m_pool = nd_pool_create(EMEMPOOL_UNLIMIT,__FUNC__) ;
// 			if(m_pool) {
// 				m_bcreate_pool = 1 ;
// 			}
// 		}
		return (*this);
	}
	_MyType& operator=(const _MyType&r)
	{
		m_pool = r.m_pool;
// 		if (r.m_bcreate_pool || m_pool)	{
// 			return (*this) ;
// 		}
// 
// 		if (r.m_bcreate_pool==0)	{
// 			_set_pool(NULL);
// 			m_pool = r.m_pool ;
// 			m_bcreate_pool = 0 ;
// 		}
// 		else {
// 			m_pool = nd_pool_create(EMEMPOOL_UNLIMIT,__FUNC__) ;
// 			if(m_pool) {
// 				m_bcreate_pool = 1 ;
// 			}
// 		}
		return (*this);
	}

	//nd_poolalloc():m_pool(0),m_bcreate_pool(0){ }
	nd_poolalloc()
	{
		m_pool = nd_stlmmpool() ;
	}
	virtual ~nd_poolalloc() 
	{
// 		if (m_bcreate_pool && m_pool){
// 			nd_pool_destroy(m_pool, 0) ;
// 			m_pool = 0 ;
// 			m_bcreate_pool = 0 ;
// 		}
	}

	void _set_pool(nd_handle p) 
	{
// 		if (m_bcreate_pool && m_pool){
// 			nd_pool_destroy(m_pool, 0) ;
// 			m_pool = 0 ;
// 			m_bcreate_pool = 0 ;
// 		}
		m_pool = p;
		//m_bcreate_pool = 0;
	}
	nd_handle _get_pool() const {return m_pool;}
	pointer address(reference x) const { return &x; }
	const_pointer address(const_reference x) const {return x;}

	pointer allocate(size_type n) {
		ND_TRACE_FUNC() ;
		void* p = _malloc_pool(n * sizeof(T), m_pool);		
		if (!p) {
			//throw std::bad_alloc();		
			return NULL;
		}
		return static_cast<pointer>(p);	
	}
	pointer allocate(size_type n, const void *)
	{	// allocate array of n elements, ignore hint
		return (allocate(n));
	}

	void deallocate(pointer p, size_type) { 
		ND_TRACE_FUNC() ;
		_free_pool(p, m_pool); 
	}	
	size_type max_size() const {
		return static_cast<size_type>(-1) / sizeof(T);
	}	

	void construct(pointer p, const value_type& x) {
		ND_TRACE_FUNC() ;
		new((void*)p) value_type(x);
	}
	void destroy(pointer p) {
		ND_TRACE_FUNC() ;
		_NDDestroy(p); 
	} 

protected:
	//int m_bcreate_pool;
	nd_handle m_pool;
};

template<> class nd_poolalloc<void>	
{	
	typedef void        value_type;	
	typedef void*       pointer;	
	typedef const void* const_pointer;

	template <class U>

	struct rebind { typedef nd_poolalloc<U> other; };

};

template <class T>
inline bool operator==(const nd_poolalloc<T>&left, const nd_poolalloc<T>&right)
{
	return (left._get_pool() == right._get_pool() );
}

template <class T>
inline bool operator!=(const nd_poolalloc<T>& left, const nd_poolalloc<T>& right) 
{
	return !(left._get_pool() == right._get_pool() );
}

#endif

