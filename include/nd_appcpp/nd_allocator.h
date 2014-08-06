/* nd_allocator.h
 *
 * allocator of nd app
 * 2009-5-24 21:50
 */ 


#ifndef _ND_ALLOCATOR_H_
#define _ND_ALLOCATOR_H_

//在stl中使用nd_stlalloc
//#define USE_ND_ALLOCATOR 


#include <exception>
#include <new>

#define _ND_ROUNTD(size) 	(((size)+7) & (~7)) 

#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE

#if !defined(ND_UNIX)  
inline void *__cdecl operator new(size_t, void *_P)
{
	return (_P); 
}

inline void __cdecl operator delete(void *, void *)
{
	return; 
}
#endif

#endif

/* 实现一个从静态数组中分配内存的分配器
 * 虽然长的很像stl::allocator但是不能在stl模板中使用
 */
template<class T , size_t _size, int _numbers>
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
		_hdr_list * list ;
		_M_free = NULL ;
		for(int i=0; i< _numbers; i++ ) {
			list =(struct _hdr_list *) _mm_pool[i] ; 
			list->next = _M_free ;
			_M_free = list ;
		}		
	}
	~static_alloc() {} 
	
	T *allocate() {
		if(_M_free) {
			_hdr_list * ret = (_hdr_list *)_M_free ;
			_M_free = _M_free->next ;
			return new((void*)ret) T ;
		}
		return NULL ;
	}
	T *allocate(const T& _val) {
		if(_M_free) {
			_hdr_list * ret = (_hdr_list *)_M_free ;
			_M_free = _M_free->next ;
			return new((void*)ret) T(_val) ;
		}
		return NULL ;
	}
	
	void deallocate(T *p) {
		if((char*)p>=_mm_pool && (char*)p<(_mm_pool + sizeof(_mm_pool))) {			
			
			_hdr_list * addr = (_hdr_list *)p ;
			p->~T() ;
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

	void destroy(pointer p) { p->~value_type(); }
private:
	struct _hdr_list * _M_free ;
	char _mm_pool[_numbers][_ND_ROUNTD(_size)] ;

} ;

#ifdef USE_ND_ALLOCATOR
extern void *_malloc_pool(size_t size) ;

extern  void _free_pool(void *p) ;
///////////////////////////////////////////////////////////////////////
/*用内存池代替标准的stl allocator*/
template <class T> class nd_stlalloc
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
	
	nd_stlalloc() {}

	nd_stlalloc(const nd_stlalloc<T>&) throw ()
	{	// construct by copying (do nothing)
	}

	template<class _Other>
	nd_stlalloc(const nd_stlalloc<_Other>&) throw ()
	{	// construct from a related allocator (do nothing)
	}

	template<class _Other>
	nd_stlalloc<T>& operator=(const nd_stlalloc<_Other>&)
	{	// assign from a related allocator (do nothing)
		return (*this);
	}
	
	~nd_stlalloc() {}
		
	pointer address(reference x) const { return &x; }
	
	const_pointer address(const_reference x) const {return x;}

	pointer allocate(size_type n, const_pointer = 0) {
		void* p = _malloc_pool(n * sizeof(T));		
		if (!p)
			throw std::bad_alloc();		
		return static_cast<pointer>(p);	
	}
		
	void deallocate(pointer p, size_type) { _free_pool(p); }
		
	size_type max_size() const {
		return static_cast<size_type>(-1) / sizeof(T);
	}	
	
	void construct(pointer p, const value_type& x) {
		new(p) value_type(x);
	}
	
	void destroy(pointer p) { p->~value_type(); }
	
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

#endif	//USE_ND_ALLOCATOR


#endif

