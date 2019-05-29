/* file : nd_listener.h
 * server listener 
 * 
 * 2009-4-25 1:14
 */

#ifndef _NDLISTENER_H_
#define _NDLISTENER_H_

#include "ndapplib/nd_baseConn.h"
#include "ndapplib/applib.h"
#include "ndapplib/nd_session.h"
#include "ndapplib/nd_objmgr.h"
#include "ndapplib/nd_objfactory.h"

#pragma  warning (disable : 4786) 

//class NDNetMsg ;
class NDSession ;
class NDConnector ;
class NDOStreamMsg ;
class NDInstanceBase ;


#define LISTENER_INSTALL_MSG(listener, msgFunc, maxID, minID, level) \
	(listener)->InstallMsgFunc(msgFunc, maxID, minID, level, #maxID"-"#minID) 

#define LISTENER_INSTALL_MSG_INT16(listener, msgFunc, msgID, level) \
	(listener)->InstallMsgFunc(msgFunc, ND_HIBYTE(msgID),ND_LOBYTE(msgID),level, #msgID)

class  ND_COMMON_CLASS NDListener : public NDObject
{
public:
	int Close(int force=0);
	int Open(int port,int thNum =0, const char *bindIp=NULL, int isIpv6=0);
	virtual int Create(const char *listen_name,int session_num, size_t session_size);
	void Destroy(int flag) ;
	
	int GetAllocatorFreenum();
	int GetAllocatorCapacity();
	
	int CloseAllConnects() ;
	void InstallMsgFunc(nd_conn_msg_entry func, ndmsgid_t maxid, ndmsgid_t minid,int level=EPL_CONNECT, const char *msgname=NULL);
	void setScriptEngine(void *script_engine, nd_msg_script_entry entry);
	virtual int OnAccept(NDBaseSession *pSession, SOCKADDR_IN*addr);			//accept callback
	
	NDBaseSession *ConstructSession(void *addr);
	void DestructSession(NDBaseSession *psession);
	NDBaseSession *GetSession(NDUINT16 sessionId);
	

	int Attach(NDObject &conn, nd_thsrvid_t thid = 0);
	int Deattach(NDObject &conn,nd_thsrvid_t thid = 0);
	
	int AttachPort(int port, const char* bindIP=NULL) ;

	int GetMsgBase() {return m_msg_base;}
	void SetMsgBase(int n) {m_msg_base=n;}

	int GetMsgNum() {return m_msg_kinds;}
	void SetMsgNum(int n) {m_msg_kinds=n;}

	NDObjectMgrBase &GetSessionMgr() {return m_session_mgr;}
	ndthread_t GetListenThid() ;

	ndthread_t OpenListenThread(int session_num) ;
	int SwitchTothread(NDBaseSession *session, ndthread_t aimth);
	int GetClientsInThreads(ndthread_t *threadid_buf, int *count_buf, int size) ;

	NDBaseSession *htoSession(nd_handle h_session);
	void SetEmptyConnTimeout(int seconds) ;		//set time out value before reveived a first packet
	NDListener(nd_fectory_base *sf=NULL ) ;	
	//void SetFectory(nd_ifectory *sf) ;
	virtual ~NDListener() ;
	NDListener &operator = (nd_handle)  ;
	void SetAccept(int bClose=0) ;
	
	size_t m_total_send ;		
	size_t m_total_recv;		
	ndtime_t m_total_online ;	
	int m_max_onlines ;			
	NDInstanceBase *m_inst ;
protected :	
	int m_msg_kinds ;			//
	int m_msg_base ;			//
	nd_fectory_base *session_fectory ;
	NDObjectMgrBase m_session_mgr ;
} ;

class  ND_COMMON_CLASS NDSafeListener: public NDListener
{
public:
	NDSafeListener(nd_fectory_base *sf=NULL ) ;	
	void Destroy(int flag) ;
protected:
	virtual int OnAccept(NDBaseSession *pSession, SOCKADDR_IN*addr);			
};

#endif

