/* file : nd_listener.h
 * server listener 
 * 
 * 2009-4-25 1:14
 */

#ifndef _NDLISTENER_H_
#define _NDLISTENER_H_

#include "nd_appcpp/nd_object.h"
#include "nd_app/nd_app.h"
#include "nd_appcpp/nd_session.h"

#ifdef SINGLE_THREAD_MOD
#include "nd_appcpp/nd_allocator.h"
#include <map>
using namespace std ;
#endif 

//#pragma  warning (push)
#pragma  warning (disable : 4786) 

class NDNetMsg ;
class NDSession ;
//(nd_handle  handle, nd_usermsgbuf_t *msg , nd_handle listener);
//typedef int (*NDRecvMsgEntry)(NDSession* conn_session ,NDNetMsg *msg, nd_handle h_listen) ;

class NDListener : public NDObject
{
public:
	void InstallMsgFunc(USER_NETMSG_FUNC func, ndmsgid_t maxid, ndmsgid_t minid,int level=EPL_CONNECT);
	virtual void InstallMsgInstance();								//��װ��Ϣ����ʵ��
	virtual void OnClose(NDSession *pSession);						//�ر����ӻص�����
	virtual int OnAccept(NDSession *pSession, SOCKADDR_IN*addr);	//���ӽ���ص�����
	int Close(int force=0);
	int Open(int port);
	int Initilize(int session_num, size_t session_size, char *listen_name);
	
#ifdef SINGLE_THREAD_MOD
	//typedef map<NDUINT16, NDSession,less<NDUINT16>,nd_stlalloc<std::pair<const NDUINT16, NDSession>  >  > SESSION_MGR_T ;
	typedef map<NDUINT16, NDSession > SESSION_MGR_T ;

	NDSession* AddNewSession(ND_SESSION_T h_session) ;		//���»滰���������
	bool DeleteSession(ND_SESSION_T h_session) ;
	NDSession *FindSession(ND_SESSION_T h_session) ;
	SESSION_MGR_T *GetSessionMgr() {return &m_map_session;}
#else 
	int GetAllocatorFreenum();
	int GetAllocatorCapacity();
	virtual int OnPreclose(NDSession *pSession);					//�ڵ��߳�ģʽ�²���Ҫ����
	virtual NDSession *ConstructSession(void *addr) { return new(addr) NDSession ;}
	virtual void DestructSession(NDSession *psession) { delete psession ;}
	//inline void _InitSession(nd_handle s) { if(m_old_init)  m_old_init(s, m_hListen); 	}
#endif 
	
	NDListener(int maxmsg_num =MSG_CLASS_NUM, int maxid_start=MAXID_BASE) ;
	virtual ~NDListener() ;
	NDListener &operator = (nd_handle)  ;
	
	nd_handle GetHandle() {return m_hListen;	}
protected :	

	nd_handle m_hListen ;		//listen handle 
	int m_msg_kinds ;			//����Ϣ����
	int m_msg_base ;			//����Ϣ��ʼ��
#ifdef SINGLE_THREAD_MOD
	SESSION_MGR_T m_map_session; 
#else 
//	cm_init m_old_init ;		//����֮ǰ���õĳ�ʼ������,��OnAccept����hListen �ĳ�ʼ������
#endif 
} ;

NDListener *NDGetListener(nd_handle h_listen) ;
NDSession *NDGetSession(ND_SESSION_T session, NDListener * Listener= NULL) ;
//#pragma  warning (pop)
#endif

