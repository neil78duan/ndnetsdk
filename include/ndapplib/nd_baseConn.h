/* file NDBaseConnector.h
 *
 * define net object base class
 *
 * create by duan 
 * 2019.5.24
 */

#ifndef _ND_BASE_CONNECTOR_H_
#define _ND_BASE_CONNECTOR_H_


#include "ndapplib/nd_object.h"
#include "ndapplib/nd_msgpacket.h"
#include "nd_net/nd_netioctl.h"
#include "nd_common/nd_common.h"


class  ND_COMMON_CLASS NDBaseConnector : public NDObject
{
public :
	
	NDBaseConnector();
	virtual ~NDBaseConnector() ;
	virtual void *GetScriptEngine() ;
	
	int Send(NDOStreamMsg &omsg) ;		//send message in script
	int SendMsg(NDSendMsg &msg, int flag=ESF_NORMAL);
	int SendMsg(nd_usermsgbuf_t *msghdr, int flag=ESF_NORMAL) ;
	int SendMsg(nd_usermsghdr_t *msghdr, int flag=ESF_NORMAL);
	int SendMsg(NDIStreamMsg &resendmsg, int flag=ESF_NORMAL);
	
	int SendRawData(void *data , size_t size) ;
	int RecvRawData(void *buf, size_t size, ndtime_t waittm) ;
	
	int WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time);
	bool FlushSendBuf(bool bForce);
	
	int CryptPackage(nd_usermsgbuf_t *msgBuf);
	
	ndip_t Getip() ;
	ndport_t GetPort() ;
	ndip_t GetPeerip() ;
	ndport_t GetPeerPort() ;
	
	NDUINT16 GetSessionID();
	
	nd_handle GetListenerHandle() ;
	
	int CheckValid();
	
	void SetConnectTimeOut(int seconds) ;
	int Ioctl(int cmd, void *val, int *size) ;
	
	void SetPrivilege(int level);
	int GetPrivilege() ;
	
	void *GetUserData();
	void SetUserData(void *pData);
	
protected:
	virtual int sendPack(nd_packhdr_t *msghdr, int flag) ;
	
};


typedef int (*nd_conn_msg_entry)(NDBaseConnector* pconn, nd_usermsgbuf_t *msg);

#endif 
