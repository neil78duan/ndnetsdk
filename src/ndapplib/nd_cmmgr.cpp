/* file : nd_cmmgr.cpp
 * implemention client map manager 
 *
 * 2009-4-25 10:25
 */
// 
// #include "ndapplib/nd_cmmgr.h"
// #include "ndapplib/nd_listener.h"
// #include "ndapplib/nd_session.h"
// #include "ndapplib/nd_instance.h"

#include "ndapplib/applib.h"

static inline NDSession* GetSessionFromHandle(nd_netui_handle handle)
{
	if(!handle)
		return NULL;
	return static_cast<NDSession*>(nd_session_getdata(handle))  ;
}
NDSession* NDSessionMgr::Lock(OBJECTID_T oid)
{
	void *addr = NDObjectMgrBase::Lock(oid) ;
	if(!addr){return NULL;}
	return  GetSessionFromHandle((nd_netui_handle )addr) ;
}

NDSession* NDSessionMgr::TryLock(OBJECTID_T oid)
{
	void *addr = NDObjectMgrBase::TryLock(oid) ;
	if(!addr){return NULL;}
	return  GetSessionFromHandle((nd_netui_handle )addr) ;
}
NDSessionMgr::NDSessionMgr(NDListener *listener) 
{
	if(listener){
		nd_handle hl = listener->GetHandle() ;
		if(hl) {
			SetMgr(nd_listensrv_get_cmmamager(hl) ) ;
		}
	}
}
NDSessionMgr::NDSessionMgr(NDInstanceBase *inst) 
{
	if(inst){
		NDListener *plis = inst->GetDeftListener() ;
		if(plis) {
			nd_handle hl = plis->GetHandle() ;
			if(hl) {
				SetMgr(nd_listensrv_get_cmmamager(hl)) ;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////

NDThreadSessionIterator::NDThreadSessionIterator()  
{

}

NDThreadSessionIterator::NDThreadSessionIterator(NDUINT16 *f,NDSession *s,struct thread_pool_info* tpi) :
	first(f),second(s),m_tpi(tpi)
{

}
NDThreadSessionIterator::~NDThreadSessionIterator() 
{

}
const NDThreadSessionIterator& NDThreadSessionIterator::operator = (const NDThreadSessionIterator &r) 
{
	first = r.first ;
	second = r.second ;
	m_tpi = r.m_tpi;
	return *this ;

}
bool NDThreadSessionIterator::operator == (const NDThreadSessionIterator &r) 
{
	return first == r.first ;
}

bool NDThreadSessionIterator::operator != (const NDThreadSessionIterator &r) 
{
	return first != r.first ;
}

NDThreadSessionIterator& NDThreadSessionIterator::operator ++ () 
{
	struct cm_manager * pcmmgr = nd_listensrv_get_cmmamager((nd_listen_handle)m_tpi->lh);
	nd_assert(pcmmgr) ;
	pcmmgr->unlock(pcmmgr,*first) ;
	++first ;
	if (first < &m_tpi->sid_buf[m_tpi->session_num]){
		void *p = pcmmgr->lock(pcmmgr,*first) ;
		nd_assert(p) ;
		second = GetSessionFromHandle((nd_netui_handle )p) ;
	}
	return *this;
}

NDThreadSessionIterator NDThreadSessionIterator::operator++ (int) 
{
	NDThreadSessionIterator tmp = *this ;
	++(*this);	
	return tmp;
}
//////////////////////////////////////////////////////////////////////////
NDThreadSessionMgr::NDThreadSessionMgr(struct thread_pool_info *pi) : m_tpi(pi)
{

}
NDThreadSessionMgr::~NDThreadSessionMgr() 
{

}

NDSession *NDThreadSessionMgr::Search(OBJECTID_T sessionid) 
{
	struct cm_manager * pcmmgr = nd_listensrv_get_cmmamager((nd_listen_handle)m_tpi->lh);
	nd_assert(pcmmgr) ;
	void *p = pcmmgr->lock(pcmmgr,sessionid) ;
	if (p){
		pcmmgr->unlock(pcmmgr,sessionid) ;
		return GetSessionFromHandle((nd_netui_handle )p) ;
	}
	return NULL;
}

NDThreadSessionMgr::iterator NDThreadSessionMgr::begin() 
{
	NDSession *ps = NULL;
	NDUINT16 *p = m_tpi->sid_buf ;

	if (m_tpi->session_num > 0) {
		struct cm_manager * pcmmgr = nd_listensrv_get_cmmamager((nd_listen_handle)m_tpi->lh);
		nd_assert(pcmmgr) ;
		void *addr = pcmmgr->lock(pcmmgr,*p) ;
		nd_assert(addr) ;
		ps = GetSessionFromHandle((nd_netui_handle )addr) ;
	}
	
	return iterator(p,ps, m_tpi);
}
NDThreadSessionMgr::iterator NDThreadSessionMgr::end()
{
	NDUINT16 *p = &m_tpi->sid_buf[m_tpi->session_num] ;
	return iterator(p,NULL, m_tpi);
}