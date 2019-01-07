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

class NDBaseSession;
class NDListener ;
class NDInstanceBase ;

//extern NDBaseSession *NDGetSession(nd_handle session, NDListener * Listener);

class  ND_COMMON_CLASS NDSessionMgr : public NDObjectMgrBase
{
public:	
	NDBaseSession * GetObject(NDObjectMgrBase::iterator &it);
	NDBaseSession* Lock(OBJECTID_T oid);
	NDBaseSession* TryLock(OBJECTID_T oid);
	NDSessionMgr(NDListener *listener) ;
	NDSessionMgr(NDInstanceBase *inst) ;
};

class  ND_COMMON_CLASS NDThreadSessionIterator
{
public:
	NDThreadSessionIterator()  ;
	NDThreadSessionIterator(NDUINT16 *, NDBaseSession *, struct thread_pool_info*);
	~NDThreadSessionIterator() ;
	const NDThreadSessionIterator& operator = (const NDThreadSessionIterator &) ;
	bool operator == (const NDThreadSessionIterator &) ;
	bool operator != (const NDThreadSessionIterator &) ;
	NDThreadSessionIterator& operator ++ () ;
	NDThreadSessionIterator operator++ (int) ;

	NDUINT16 *first ;
	NDBaseSession *second;
private:
	struct thread_pool_info *m_tpi;
};
//
class  ND_COMMON_CLASS NDThreadSessionMgr
{
public:
	typedef NDThreadSessionIterator iterator;
	NDThreadSessionMgr(struct thread_pool_info *pi) ;
	virtual ~NDThreadSessionMgr() ;
	
	NDBaseSession *Search(OBJECTID_T sessionid);
	iterator begin() ;
	iterator end();

protected:
	struct thread_pool_info *m_tpi ;
};

#endif
