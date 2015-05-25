/* file : nd_cmmgr.h
 * header file of client map manager 
 *
 * 2009-4-25 10:25
 */

#ifndef _NDCMMGR_H_
#define _NDCMMGR_H_

#include "ndapplib/nd_object.h"
#include "ndapplib/nd_listener.h"
#include "ndapplib/nd_session.h"
#include "ndapplib/nd_objmgr.h"

class NDSession;
class NDListener ;
class NDInstanceBase ;

extern NDSession *NDGetSession(nd_handle session, NDListener * Listener) ;

class NDSessionMgr : public NDObjectMgrBase
{
public:	
	NDSession* GetObject(NDObjectMgrBase::iterator &it) 
	{
		return NDGetSession((nd_handle)it.second,NULL) ;
	}
	NDSession* Lock(OBJECTID_T oid);
	NDSession* TryLock(OBJECTID_T oid);
	NDSessionMgr(NDListener *listener) ;
	NDSessionMgr(NDInstanceBase *inst) ;
};

class NDThreadSessionIterator 
{
public:
	NDThreadSessionIterator()  ;
	NDThreadSessionIterator(NDUINT16 *,NDSession *,struct thread_pool_info*)  ;
	~NDThreadSessionIterator() ;
	const NDThreadSessionIterator& operator = (const NDThreadSessionIterator &) ;
	bool operator == (const NDThreadSessionIterator &) ;
	bool operator != (const NDThreadSessionIterator &) ;
	NDThreadSessionIterator& operator ++ () ;
	NDThreadSessionIterator operator++ (int) ;

	NDUINT16 *first ;
	NDSession *second ;
private:
	struct thread_pool_info *m_tpi;
};
//每个线程池的会话管理器
class NDThreadSessionMgr
{
public:
	typedef NDThreadSessionIterator iterator;
	NDThreadSessionMgr(struct thread_pool_info *pi) ;
	virtual ~NDThreadSessionMgr() ;
	
	NDSession *Search(OBJECTID_T sessionid) ;
	iterator begin() ;
	iterator end();

protected:
	struct thread_pool_info *m_tpi ;
};

#endif
