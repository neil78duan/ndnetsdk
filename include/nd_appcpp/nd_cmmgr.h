/* file : nd_cmmgr.h
 * header file of client map manager 
 *
 * 2009-4-25 10:25
 */

#ifndef _NDCMMGR_H_
#define _NDCMMGR_H_

#include "nd_appcpp/nd_object.h"
#include "nd_app/nd_app.h"
#include "nd_appcpp/nd_listener.h"

class NDSession;
//class NDListener ;
class NDInstanceSrv ;
class NDCmIterator : public NDObject
{
public :
	//锁函数
	void Unlock(NDUINT16 sessionID);
	NDSession * Lock(NDUINT16 sessionID);
	NDSession * TryLock(NDUINT16 sessionID);
	//迭代函数
	void BreakIterator();
	NDSession * Next();
	NDSession* First();
#ifdef SINGLE_THREAD_MOD
	
	int CheckValid() {return 0!=m_clistener; }
#else 
	struct cm_manager *GetCmMgr() {return pManger ;}
	int GetCapacity();			//容量
	int GetActiveNum();			//当前活动连接
	int CheckValid() {return 0!=pManger; }
#endif
	NDCmIterator(NDListener &listener) ;
	NDCmIterator(NDInstanceSrv *inst) ;
	
	virtual ~NDCmIterator() ;
private:
	
#ifdef SINGLE_THREAD_MOD	
	NDListener::SESSION_MGR_T * m_pSession_mgr;
	NDListener::SESSION_MGR_T::iterator m_iter;
#else 
	struct cm_manager *pManger  ;
	cmlist_iterator_t cm_iterator ;
#endif 
	NDListener *m_clistener ;
};

#endif
