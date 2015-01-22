/* file : nd_listener.h
 * server listener 
 * 
 * 2009-4-25 1:14
 */

#ifndef _NDLISTENER_H_
#define _NDLISTENER_H_

#include "ndapplib/nd_object.h"
#include "ndapplib/applib.h"
#include "ndapplib/nd_session.h"
#include "ndapplib/nd_objmgr.h"

//#pragma  warning (push)
#pragma  warning (disable : 4786) 

//class NDNetMsg ;
class NDSession ;
class NDConnector ;
class NDOStreamMsg ;
class NDInstanceBase ;


#define LISTENER_INSTALL_MSG(listener, msgFunc, maxID, minID, level) \
	(listener)->InstallMsgFunc(msgFunc, maxID, minID, level, "msgName_" #maxID"-"#minID) 


class NDListener : public NDObject
{
public:
	int Close(int force=0);
	int Open(int port,int thread_num=0);
	int Create(const char *listen_name,int session_num, size_t session_size);
	void Destroy(int flag) ;
	
	int GetAllocatorFreenum();
	int GetAllocatorCapacity();
	
	int CloseAllConnects() ;
	void InstallMsgFunc(nd_usermsg_func func, ndmsgid_t maxid, ndmsgid_t minid,int level=EPL_CONNECT, const char *msgname=NULL);
	virtual int OnAccept(NDSession *pSession, SOCKADDR_IN*addr);			//连接进入回调函数
	
	NDSession *ConstructSession(void *addr);
	void DestructSession(NDSession *psession);

	int Attach(NDConnector &conn, nd_thsrvid_t thid = 0);
	int Deattach(NDConnector &conn,nd_thsrvid_t thid = 0);

	int GetMsgBase() {return m_msg_base;}
	void SetMsgBase(int n) {m_msg_base=n;}

	int GetMsgNum() {return m_msg_kinds;}
	void SetMsgNum(int n) {m_msg_kinds=n;}

	NDObjectMgrBase &GetSessionMgr() {return m_session_mgr;}
	ndthread_t GetListenThid() ;

	ndthread_t OpenListenThread(int session_num) ;
	int SwitchTothread(NDSession *session, ndthread_t aimth) ;
	int GetClientsInThreads(ndthread_t *threadid_buf, int *count_buf, int size) ;

	NDSession *htoSession(nd_handle h_session) ;
	void SetEmptyConnTimeout(int seconds) ;		//设置空连接超时时间
	NDListener(nd_fectory_base *sf=NULL ) ;	
	//void SetFectory(nd_ifectory *sf) ;
	virtual ~NDListener() ;
	NDListener &operator = (nd_handle)  ;
	void SetAccept(int bClose=0) ;
	
	size_t m_total_send ;		//总发送长度
	size_t m_total_recv;		//总接收长度
	ndtime_t m_total_online ;	//总在线时间
	int m_max_onlines ;			//最大在线人数
	NDInstanceBase *m_inst ;
protected :	
	int m_msg_kinds ;			//主消息数量
	int m_msg_base ;			//主消息起始号
	nd_fectory_base *session_fectory ;
	NDObjectMgrBase m_session_mgr ;
} ;

class NDSafeListener: public NDListener
{
public:
	NDSafeListener(nd_fectory_base *sf=NULL ) ;	
	void Destroy(int flag) ;
protected:
	int OnAccept(NDSession *pSession, SOCKADDR_IN*addr);			//连接进入回调函数
};
NDListener *NDGetListener(nd_handle h_listen) ;
NDSession *NDGetSession(nd_handle session, NDListener * Listener= NULL) ;
//#pragma  warning (pop)
#endif

