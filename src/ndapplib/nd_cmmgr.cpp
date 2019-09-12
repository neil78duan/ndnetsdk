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

static inline NDBaseSession* GetSessionFromHandle(nd_netui_handle handle)
{
	if(!handle)
		return NULL;
	return static_cast<NDBaseSession*>(nd_session_getdata((nd_handle)handle));
}

NDBaseSession* NDSessionMgr::GetObject(NDObjectMgrBase::iterator &it)
{
	return dynamic_cast<NDBaseSession*>( NDObject::FromHandle((nd_handle)it.second));
}

NDBaseSession* NDSessionMgr::Lock(OBJECTID_T oid)
{
	void *addr = NDObjectMgrBase::Lock(oid) ;
	if(!addr){return NULL;}
	return  GetSessionFromHandle((nd_netui_handle )addr) ;
}

NDBaseSession* NDSessionMgr::TryLock(OBJECTID_T oid)
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
			SetMgr(nd_listener_get_session_mgr(hl) ) ;
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
				SetMgr(nd_listener_get_session_mgr(hl)) ;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////

NDThreadSessionIterator::NDThreadSessionIterator()  
{

}

NDThreadSessionIterator::NDThreadSessionIterator(NDUINT16 *f, NDBaseSession *s, struct thread_pool_info* tpi) :
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
	return second == r.second ;
}

bool NDThreadSessionIterator::operator != (const NDThreadSessionIterator &r) 
{
	return second != r.second;
}

NDThreadSessionIterator& NDThreadSessionIterator::operator ++ () 
{
#if !defined (USE_NEW_MODE_LISTEN_THREAD)
	struct cm_manager * pcmmgr = nd_listener_get_session_mgr((nd_listen_handle)m_tpi->lh);
	nd_assert(pcmmgr) ;
	pcmmgr->unlock(pcmmgr,*first) ;
	++first ;
	if (first < &m_tpi->sid_buf[m_tpi->session_num]){
		void *p = pcmmgr->lock(pcmmgr,*first) ;
		nd_assert(p) ;
		second = GetSessionFromHandle((nd_netui_handle )p) ;
	}
#else 
	nd_handle h = second->GetHandle();
	if (!h)	{
		return *this;
	}
	struct nd_session_tcp *client = (struct nd_session_tcp *) h;
	struct list_head *pos = client->map_list.next;
	if (pos == &m_tpi->sessions_list)	{
		first = NULL;
		second = NULL;
	}
	else {
		client = list_entry(pos, struct nd_session_tcp, map_list);
		second = GetSessionFromHandle((nd_netui_handle)client);
		first = &client->connect_node.session_id;
	}
#endif

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

NDBaseSession *NDThreadSessionMgr::Search(OBJECTID_T sessionid)
{
	struct cm_manager * pcmmgr = nd_listener_get_session_mgr((nd_listen_handle)m_tpi->lh);
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

#if !defined (USE_NEW_MODE_LISTEN_THREAD)
	NDBaseSession *ps = NULL;
	NDUINT16 *p = m_tpi->sid_buf ;

	if (m_tpi->session_num > 0) {
		struct cm_manager * pcmmgr = nd_listener_get_session_mgr((nd_listen_handle)m_tpi->lh);
		nd_assert(pcmmgr) ;
		void *addr = pcmmgr->lock(pcmmgr,*p) ;
		nd_assert(addr) ;
		ps = GetSessionFromHandle((nd_netui_handle )addr) ;
	}
	
	return iterator(p,ps, m_tpi);
#else 
	struct list_head *pos = m_tpi->sessions_list.next;
	if (pos == &m_tpi->sessions_list)	{
		return iterator(NULL, NULL, m_tpi);
	}
	struct nd_session_tcp *client = list_entry(pos, struct nd_session_tcp, map_list);
	
	NDBaseSession *ps = (NDBaseSession*) GetSessionFromHandle((nd_netui_handle)client);
	NDUINT16 *p = &client->connect_node.session_id ;
	return iterator(p,ps,m_tpi);

#endif
}
NDThreadSessionMgr::iterator NDThreadSessionMgr::end()
{
	return iterator(NULL, NULL, m_tpi);
}
