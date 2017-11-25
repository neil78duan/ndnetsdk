/* file : nd_session.h
 * define net connection session of nd 
 * 
 * 2009-4-25 0:57
 */

#ifndef _NDSESSION_H_
#define _NDSESSION_H_

#include "ndapplib/nd_msgpacket.h"
#include "ndapplib/nd_object.h"
#include "nd_net/nd_netlib.h"
#include "ndapplib/nd_alarms.h"

#pragma  warning(push)
#pragma  warning (disable : 4290 )
//net session 

class NDBaseSession : public NDAlarm 
{
public:
	NDBaseSession  ();
	virtual~NDBaseSession ();

	const char* GetInetAddr();
	virtual void Initilize(nd_handle hsession, nd_handle listen = NULL);
	NDUINT16 GetSessionID();

	int BigDataSend(int maxID, int minID, void *data, size_t datalen);
	int Close(int flag = 0);
	int SendRawData(void *data, size_t size);
	NDObject* GetParent();

	void DecRefCount();
	int IncRefCount();

	void SetPrivilege(int level);
	int GetPrivilege();

	ndip_t Getip();
	ndport_t GetPort();
	ndip_t GetPeerip();
	ndport_t GetPeerPort();

	int Ioctl(int cmd, void *val, int *size);
	int SetDelayClose();
	int LoadBalance();

	void baseUpdate();

	int CryptPackage(nd_usermsgbuf_t *msgBuf);
private:

	net_update_entry m_handle_update;
};


class NDSession : public NDBaseSession
{
public :

	int SendMsg(NDSendMsg &smsg, int flag=ESF_NORMAL);//{return ::nd_connector_send(GetHandle() , (nd_packhdr_t *)(smsg.GetMsgAddr()), flag) ;	}	
	int SendMsg(nd_usermsghdr_t *msghdr, int flag=ESF_NORMAL);//{return ::nd_connector_send(GetHandle() , &msghdr->packet_hdr, flag) ;	}
	int ResendMsg(NDIStreamMsg &resendmsg, int flag=ESF_NORMAL);//{return ::nd_connector_send(GetHandle() , (nd_packhdr_t *)(resendmsg.GetMsgAddr()), flag) ;	}	
	
	
	int Send(NDOStreamMsg &omsg) ;		//send message in script
	int Send(int maxId, int minId, void *data, size_t len);
	int Send(NDUINT16 messageId, void *data, size_t len);

	NDUINT32 GetID() {return m_id;}
	void SetID(NDUINT32 id) {m_id = id;}
	NDUINT32 GetType() {return m_type;}
	void SetType(NDUINT32 type) {m_type = type;}

	bool RedirectLogToMe();

#if 0
	void *  operator new(size_t size,void *addr) throw (std::bad_alloc) ;
	void operator delete(void *p) ;
#endif 

	NDSession () ; 
	virtual ~NDSession() ;
protected:
	//void *  operator new(size_t size) ;//throw (std::bad_alloc) ;
	NDUINT32 m_id ;				//id
	NDUINT32 m_type ;			//¿‡–Õ
	bool m_bRedirctLog;
public:
	static NDSession *g_redirect_log_object;
	static nd_log_entry g_redirectOrgFunc;
};

#pragma  warning(pop)

#endif
