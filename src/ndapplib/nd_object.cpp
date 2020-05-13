/* file nd_object.cpp
 * inplemention of class NDObject 
 *
 * 2009-4-24 23:04
 */
//#include <stdexcept>
#include "nd_common/nd_common.h"
#include "nd_common/nd_alloc.h"
#include "nd_net/nd_netlib.h"
#include "ndapplib/nd_object.h"


#pragma  warning(push)
#pragma  warning (disable : 4290 )

using namespace std ;
#define __ND_THROW_ALLOC()  throw std::bad_alloc() 



NDObject *NDObject::FromHandle(nd_handle h)
{
	if (!h) {
		return NULL;
	}
	void *pData = nd_net_object_caller(h) ;
	if(!pData) {
		return NULL;
	}
	return static_cast<NDObject*>(pData);
}

NDObject::NDObject() :m_objhandle(NULL)//,m_userData(NULL)
{
    m_bPoolOwner = 0;
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

int NDObject::Create(const char *name)
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
#ifndef DND_CLIENT_ONLY
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

void NDObject::OnInitilize()
{

}
#endif

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
