/* file ndobject.h
 * header file of class NDObject 
 *
 * 2009-4-24 23:04
 */

#ifndef _NDOBJECT_H_
#define _NDOBJECT_H_

//#include <stdexcept>
#include "nd_common/nd_common.h"
#include "ndstl/nd_utility.h"
#include "ndstl/nd_new.h"
#include <sys/types.h>


using namespace std ;
//#pragma  warning(push)
#pragma  warning (disable : 4290 )
#pragma  warning (disable : 4291 )

class NDObject 
{
protected :
	NDObject();
	virtual ~NDObject() {}
public :
#if 0
	void *operator new(size_t size) throw (std::bad_alloc);
	void *operator new[](size_t size) throw (std::bad_alloc);
	
	void operator delete(void *p) throw();
	void operator delete[](void *p) throw();
#endif

	virtual nd_handle GetHandle() ;
	virtual NDObject* GetParent() ;
	virtual int LastError() ;
	virtual void SetLastError(NDUINT32 errcode) ;

	virtual int Create(char *name) ;
	virtual void Destroy(int flag=0) ;
	virtual void OnCreate() ;			//call on create
	virtual void OnDestroy() ;

	virtual int Close(int flag=0) ;
	virtual int Open(int param) ;
	virtual void OnClose() ;
	virtual void OnInitilize() ;		// call on open
	
    virtual int Update() ;
    
    virtual nd_handle GetMmpool() ;
    virtual int SetMmpool(nd_handle pool) ;
	
	void SetUserObj(NDObject *obj) {m_userData = obj ;}
	NDObject *GetUserObj() {return m_userData ;}

	const char *getName();
	void setName(const char *name);

protected:
    NDUINT8 m_bPoolOwner ;
    nd_handle m_pool ;
    
    nd_handle m_objhandle ;
	NDObject *m_userData ;
};

typedef void (NDObject::*NDObjectFunc)();


//#pragma warning (pop)

class nd_fectory_base : public NDObject
{
public:
	nd_fectory_base() {} ;
	virtual ~nd_fectory_base() {} 
	virtual NDObject *construct(void *p) = 0;
	//virtual NDObject *construct() = 0;
	virtual void destruct(void *p) = 0;
	virtual void selfdestroy() = 0 ;
};
template<class T>
class nd_fectory : public  nd_fectory_base
{
public:
	typedef T                 value_type;	
	typedef value_type*       pointer;	
	typedef const value_type* const_pointer;	
	typedef value_type&       reference;	
	typedef const value_type& const_reference;	
	typedef size_t       size_type;	
	typedef ptrdiff_t    difference_type;	

	nd_fectory() {}
	virtual ~nd_fectory() {}
	pointer construct(void *p){	return new(p) value_type;}
	//pointer construct() {return new value_type;}
	void destruct(void* obj) {
		pointer p = static_cast<pointer>(obj) ;
		_NDDestroy(p) ;
	}
	void selfdestroy(){delete this;}

	pointer address(void*p) {return (void*) (static_cast<pointer>(p));}
};

#endif 
