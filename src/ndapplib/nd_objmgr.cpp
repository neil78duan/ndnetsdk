/* file nd_objmgr.cpp
 *
 * object manager of ndproject
 *
 * create by duan
 * 2011/2/13 11:48:47
 */

#include <stdexcept>
#include "nd_common/nd_common.h"
#include "nd_common/nd_alloc.h"
#include "ndapplib/nd_objmgr.h"

//////////////////////////////////////////////////////////////////////////
//manager session in listenor

void NDObjectMgrBase::Unlock(OBJECTID_T oid )
{

	if (!handle)
		return  ;
	handle->unlock(handle,oid) ;
}

void * NDObjectMgrBase::Lock(OBJECTID_T oid )
{
	if (!handle)
		return NULL ;
	return handle->lock(handle,oid) ;
}

void * NDObjectMgrBase::TryLock(OBJECTID_T oid )
{

	if (!handle)
		return NULL ;
	return handle->trylock(handle,oid) ;
}

int NDObjectMgrBase::GetCapacity()
{
	if (!handle)
		return 0 ;
	return (handle)->max_conn_num ;

}

int NDObjectMgrBase::GetActiveNum()
{
	if (!handle)
		return 0 ;
	return (handle)->connect_num ;
}

bool NDObjectMgrBase::IsValid()
{
	if(handle &&  nd_atomic_read(&(handle)->connect_num ))
		return true;
	return false; 
}
NDObjectMgrBase::NDObjectMgrBase() 
{
	handle = NULL ;
}
NDObjectMgrBase::NDObjectMgrBase(nd_manager_handle  cmmgr) 
{
	handle =  cmmgr ;
}

NDObjectMgrBase::~NDObjectMgrBase() 
{

}

NDObjectMgrBase::iterator NDObjectMgrBase::begin() 
{
	nd_object_iterator tmp_it = {0} ;
	if(!IsValid())
		return iterator(tmp_it,NULL, handle);

	void *paddr = (handle)->lock_first (handle,&tmp_it) ;
	return iterator(tmp_it,paddr, handle) ;
}

NDObjectMgrBase::iterator NDObjectMgrBase::end() 
{
	nd_object_iterator tmp_it ;
	tmp_it.node_id = 0 ;tmp_it.numbers = 0;
	return iterator(tmp_it,NULL, handle);
}

void NDObjectMgrBase::SetMgr(nd_manager_handle cmmgr)
{
	handle = cmmgr;
}


void NDObjectMgrBase::CheckUnrelease(OBJECTID_T exceptid ) 
{
	nd_node_checkerror(handle,exceptid) ; 
}

ndthread_t NDObjectMgrBase::GetOwner(OBJECTID_T oid) 
{
	return handle->get_owner(handle,oid) ;
}
void NDObjectMgrBase::SetOwner(OBJECTID_T oid,ndthread_t ownerid) 
{
	handle->set_owner(handle,oid, ownerid) ;
}
//////////////////////////////////////////////////////////////////////////
//

NDObjectIterator::NDObjectIterator()  
{
	memset(&first,0, sizeof(first)) ;
	second = 0 ;
	m_needrelease = 1 ;
}

NDObjectIterator::NDObjectIterator(nd_object_iterator &f, void *s, nd_manager_handle h) 
{
	first = f ;
	second = s ;
	handle = h ;
	m_needrelease = 0;
}
NDObjectIterator::~NDObjectIterator() 
{
	if(first.node_id != 0 && m_needrelease) {
		handle->unlock_iterator(handle, &first) ;
	}
}
const NDObjectIterator& NDObjectIterator::operator=(const NDObjectIterator &r) 
{
	first = r.first ;
	second = r.second ;
	handle = r.handle ;
	return *this ;
}
bool NDObjectIterator::operator == (const NDObjectIterator & r) 
{
	return (first.node_id == r.first.node_id);
}

bool NDObjectIterator::operator != (const NDObjectIterator &r) 
{
		return !(first.node_id == r.first.node_id);
}
NDObjectIterator& NDObjectIterator::operator ++ () 
{
	nd_assert(handle);
	nd_assert(first.node_id);
	second = handle->lock_next (handle,&first) ;
	if(!second) {
		first.node_id = 0 ;
		first.numbers = 0 ;
	}
	
	return (*this);
}

NDObjectIterator NDObjectIterator::operator++ (int) 
{
	NDObjectIterator tmp = *this ;

	nd_assert(handle);
	nd_assert(first.node_id);
	tmp.m_needrelease = 0 ;

	second = handle->lock_next (handle,&first) ;
	if(!second) {
		first.node_id = 0 ;
		first.numbers = 0 ;
	}
	return tmp;//NDObjectIterator(tmp.first,tmp.second, handle);
}