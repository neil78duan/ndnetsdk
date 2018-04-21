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

NDObject::NDObject() :m_objhandle(NULL),m_userData(NULL)
{
    m_bPoolOwner = NULL;
    m_pool = 0;
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

nd_handle NDObject::GetMmpool()
{
    if (m_pool) {
        return m_pool ;
    }
    return nd_global_mmpool() ;
}
int NDObject::SetMmpool(nd_handle pool)
{
    if (pool) {
        
        if (m_bPoolOwner && m_pool ) {
            nd_pool_destroy(m_pool, 0) ;
            m_bPoolOwner = 0 ;
            m_pool = 0 ;
        }
        
        m_pool = pool ;
        m_bPoolOwner = 0 ;
        return 0;
    }
    else {
        m_pool = nd_pool_create(EMEMPOOL_UNLIMIT, NULL);
        if (m_pool) {
            m_bPoolOwner = 1 ;
            return 0 ;
        }
        return -1;
    }
}

void *NDObject::getScriptEngine()
{
	return NULL;
}

int NDObject::Create(char *name)
{
	return 0 ;
}
void NDObject::Destroy(int flag) 
{
    if (m_bPoolOwner) {
        nd_pool_destroy(m_pool, flag) ;
        m_bPoolOwner = 0 ;
        m_pool = 0 ;
    }
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


const char *NDObject::getName()
{
	if (m_objhandle){
		return nd_object_get_instname(m_objhandle);
	}
	return NULL;
}
void NDObject::setName(const char *name)
{
	if (m_objhandle){
		nd_object_set_instname(m_objhandle,name);
	}

}

#pragma  warning(pop)
