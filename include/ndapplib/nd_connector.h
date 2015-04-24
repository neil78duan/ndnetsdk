/* file : nd_connector.h
 * header file of connector class of nd engine 
 *
 * all right reserved by neil duan 
 * 2009-5-8 23:25 
 */

#ifndef _ND_CONNECTOR_H_
#define _ND_CONNECTOR_H_

#include "ndapplib/nd_object.h"
#include "ndapplib/nd_msgpacket.h"
//#include "nd_net/nd_netlib.h"
#include "nd_net/nd_netioctl.h"

#define CONNECT_INSTALL_MSG(connect, msgFunc, maxID, minID) \
	(connect)->InstallMsgFunc(msgFunc, maxID, minID, "msgName_" #maxID"-"#minID) 

#define CONNECT_INSTALL_MSG_INT16(connect, msgFunc, msgID) \
	(connect)->InstallMsgFunc(msgFunc, ND_HIBYTE(msgID),ND_LOBYTE(msgID), "msgName_"#msgID)
//net connector 
class NDConnector : public NDObject 
{
public :		
	void Destroy(int flag = 0);
	int Create(const char *protocol_name=NULL) ;
	int Open(const char*host, int port,const char *protocol_name, struct nd_proxy_info *proxy=NULL);
	int Close(int force=0);

	int SendMsg(NDSendMsg &msg, int flag=ESF_URGENCY);
	int SendMsg(nd_usermsghdr_t *msghdr, int flag=ESF_URGENCY);
	int ResendMsg(NDIStreamMsg &resendmsg, int flag=ESF_URGENCY);
	int SendRawData(void *data , size_t size) ;
	int RecvRawData(void *buf, size_t size, ndtime_t waittm) ;
	
	int BigDataSend(NDUINT64 param, void *data, size_t datalen) ;

	int CheckValid();
	int WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time=100);
	int Update(ndtime_t wait_time);
	void InstallMsgFunc( nd_usermsg_func, ndmsgid_t maxid, ndmsgid_t minid,const char *msgname=NULL);

	ndip_t Getip() ;
	ndport_t GetPort() ;
	ndip_t GetPeerip() ;
	ndport_t GetPeerPort() ;

	void SetConnectTimeOut(int seconds) ;
	int Ioctl(int cmd, void *val, int *size) ;

	NDConnector(int maxmsg_num =ND_DFT_MAXMSG_NUM, int maxid_start=0) ;
	void SetMsgNum(int maxmsg_num , int maxid_start=0) ;
	virtual~NDConnector() ;

	int _data_func(void *data, size_t size) ;
private:
	int m_open ;
	//nd_handle m_objhandle ;
	int msg_kinds ;
	int msg_base ;
	data_in_entry m_old_in_func;
};

NDConnector * htoConnector(nd_handle h);
#endif
