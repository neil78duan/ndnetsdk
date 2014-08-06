/* file : nd_session.h
 * define net connection session of nd 
 * 
 * 2009-4-25 0:57
 */

#ifndef _NDSESSION_H_
#define _NDSESSION_H_

#include "nd_appcpp/nd_msgpacket.h"
#include "nd_appcpp/nd_object.h"
//#include "nd_net/nd_netlib.h"
#include "nd_app/nd_app.h"


#pragma  warning(push)
#pragma  warning (disable : 4290 )
//net session 
class NDSession 
{
public :
	char * GetInetAddr();

	virtual void InitSession(ND_SESSION_T hsession, nd_handle listen= NULL) ;

	inline ND_SESSION_T GetSessionHandle() {return m_net_handle ;} 

#ifdef SINGLE_THREAD_MOD
	inline int SendMsg(NDSendMsg &smsg, int flag=ESF_NORMAL){return ::nd_st_send(m_net_handle ,  (nd_usermsghdr_t  *)(smsg.GetMsgAddr()),  flag, m_hlisten) ;}
	NDUINT16 GetSessionID() {return m_net_handle ; }
	int Close(int flag = 0) {	nd_st_close(m_net_handle, flag,m_hlisten) ;}
#else 
	NDUINT16 GetSessionID() {return ((nd_netui_handle)m_net_handle)->session_id ; }
	void DecRefCount();
	int IncRefCount();
	void *  operator new(size_t size,void *addr) throw (std::bad_alloc) ;
	void operator delete(void *p) ;
	inline int SendMsg(NDSendMsg &smsg, int flag=ESF_NORMAL){return ::nd_connector_send(m_net_handle , (nd_packhdr_t *)(smsg.GetMsgAddr()), flag) ;	}
	
	int SendRawData(void *data, size_t size) ;
	int Close(int flag = 0) {	return nd_session_close(m_net_handle, flag) ;}
#endif 
	NDSession () ; 
	//NDSession (NDSession &r) ; 
	
	//NDSession & operator = (NDSession &r);
	virtual ~NDSession() ;

private:
#ifdef SINGLE_THREAD_MOD
	nd_handle m_hlisten ;
#endif 
	ND_SESSION_T m_net_handle ;
};

#pragma  warning(pop)
#endif
