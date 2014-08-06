/* file nd_object.cpp
 * inplemention of class NDObject 
 *
 * 2009-4-24 23:04
 */
#include <stdexcept>
#include "nd_common/nd_common.h"
#include "nd_common/nd_alloc.h"
#include "ndapplib/nd_object.h"


#pragma  warning(push)
#pragma  warning (disable : 4290 )

using namespace std ;
#define __ND_THROW_ALLOC()  throw std::bad_alloc() 

#if 0
static nd_handle __object_pool  ;

static void *obj_alloc(size_t size) throw(std::bad_alloc)
{
	void *p ;
	if(!__object_pool) {
		__object_pool = nd_pool_create(EMEMPOOL_UNLIMIT,"nd_app_object_pool") ;
	}
	if(!__object_pool) {
		throw std::bad_alloc()   ;
	}
	p = nd_pool_alloc(__object_pool ,  size);
	if(!p) {
		throw std::bad_alloc()  ;
	}
	return p ;
}

static __INLINE__ void obj_free(void *p)
{
	nd_assert(p) ;
	nd_pool_free(__object_pool, p) ;
}


void *NDObject::operator new(size_t size)  throw (std::bad_alloc)
{
	return 	obj_alloc( size) ;
}

void* NDObject::operator new[](size_t size) throw (std::bad_alloc)
{
	return 	obj_alloc( size) ;
}
void NDObject::operator delete(void *p) throw()
{
	obj_free(p) ;
}

void NDObject::operator delete[](void *p) throw()
{
	obj_free(p) ;
}
#endif 

NDObject::NDObject() :m_objhandle(NULL)
{	
}

NDObject* NDObject::GetParent() 
{
	return NULL;
}

nd_handle NDObject::GetHandle() 
{
	return m_objhandle;
}

int NDObject::LastError() 
{
	nd_handle h = GetHandle() ;
	if(!h)
		return NDERR_INVALID_HANDLE ;
	else 
		return nd_object_lasterror(h) ;

}

void NDObject::SetLastError(NDUINT32 errcode) 
{

	nd_handle h = GetHandle() ;
	if(h)
		nd_object_seterror(h,errcode) ;
}

int NDObject::Create(char *name) 
{
	return 0 ;
}
void NDObject::Destroy(int flag) 
{

}
void NDObject::OnCreate() 
{

}
void NDObject::OnDestroy() 
{

}

int NDObject::Close(int flag) 
{
	return 0 ;
}
int NDObject::Open(int param) 
{
	return 0;
}
void NDObject::OnClose() 
{

}


int NDObject::Update()
{
	return 0 ;
}

void NDObject::OnInitilize()
{

}

#pragma  warning(pop)
