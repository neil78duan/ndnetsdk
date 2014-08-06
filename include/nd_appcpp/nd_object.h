/* file ndobject.h
 * header file of class NDObject 
 *
 * 2009-4-24 23:04
 */

#ifndef _NDOBJECT_H_
#define _NDOBJECT_H_

#include <stdexcept>
#include "nd_common/nd_common.h"
using namespace std ;
//#pragma  warning(push)
#pragma  warning (disable : 4290 )
#pragma  warning (disable : 4291 )
class NDObject 
{
protected :
	NDObject() {}
	virtual ~NDObject() {}
public :
	void *operator new(size_t size) throw (std::bad_alloc);
	void *operator new[](size_t size) throw (std::bad_alloc);
	void operator delete(void *p) throw();
	void operator delete[](void *p) throw();
	virtual nd_handle GetHandle() ;
	virtual int LastError() ;
	virtual void SetLastError(NDUINT32 errcode) ;
};

class NDCallTrace
{
public:
	NDCallTrace(const char *funcname) 
	{
		m_ret = push_func((char*)funcname) ;
	}
	~NDCallTrace() 
	{
		if(m_ret==0) {
			pop_func() ;
		}
	}
private:
	int m_ret ;
};

#ifdef ND_CALLSTACK_TRACE
#define ND_TRACE_FUNC() NDCallTrace _tmp_func_trace(__FUNC__)
#else 
#define ND_TRACE_FUNC() //
#endif 
//#pragma warning (pop)

#endif 
