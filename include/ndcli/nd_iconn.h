/* file nd_iconn.h
 *
 * connector base interface 
 *
 * create by duan
 * 2011/4/2 15:57:50
 */

#ifndef _ND_ICONN_H_
#define _ND_ICONN_H_


#ifndef BUILD_AS_THIRD_PARTY

#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"
#endif 
//#include "ndcli/nd_api_c.h"
//#include "ndapplib/nd_iBaseObj.h"
#include "ndapplib/nd_msgpacket.h"
#include "ndapplib/nd_connector.h"
#include "nd_msg.h"


//#define ND_CONNCLI_API NDNET_API 

typedef int (*ndNetFunc)(nd_handle handle, unsigned char *data, int dataLen );

#define  WAITMSG_TIMEOUT ndGetTimeoutVal()

class NDCliConnector : public NDConnector
{
	
public:
	NDCliConnector(int maxmsg_num = ND_MAIN_MSG_CAPACITY, int maxid_start = ND_MSG_BASE_ID);
	virtual ~NDCliConnector() ;
	int Create(const char *protocol_name) ;
	void Destroy(int flag=0) ;
	
	virtual int Open(const char*host, int port,const char *protocol_name, void *proxy=NULL) ;
	virtual int Open(ndip_t& ip, int port,const char *protocol_name, void *proxy=NULL) ;
	virtual int Close(int force=0) ;
	
	int CallMsgHandle(nd_usermsgbuf_t *msgbuf)  ;
	bool TestMsgIsHandle(ndmsgid_t maxid, ndmsgid_t minid) ;
	void SetMsgNum(int maxmsg_num , int maxid_start) ;
	int Reconnect(ndip_t& IP, int port,void *proxy=NULL)  ;//connect to another host
	int ExchangeKey(void *output_key) ;
	const char *ErrorDesc() ;
	const char *ConvertErrorDesc(NDUINT32 errcode) ;
	
};
//
//class NDObject: public NDIBaseObj
//{
//public:
//
//	virtual void *getScriptEngine() = 0;
//	virtual int LastError() = 0;
//	virtual void SetLastError(NDUINT32 errcode) = 0;
//	virtual nd_handle GetHandle() = 0;
//
//	virtual const char *getName() = 0;
//	virtual void setName(const char *name) = 0;
//
//	static NDObject * FromHandle(nd_handle h);
//protected:
//	NDObject() {}
//	virtual ~NDObject() {}
//};

//net connector
/*
class NDIConn : public NDObject
{
public:
	virtual int Open(const char*host, int port,const char *protocol_name, void *proxy=NULL) = 0;
	virtual int Open(ndip_t& ip, int port,const char *protocol_name, void *proxy=NULL) = 0;
	virtual int Close(int force=0) = 0;
	virtual int Send(int maxid, int minid, void *data, size_t size)  = 0;
	virtual int SendMsg(NDSendMsg &msg, int flag=0) = 0;
	virtual int SendMsg(nd_usermsgbuf_t *msghdr, int flag=0) = 0;
	virtual int SendRawData(void *data , size_t size)  = 0;
	virtual int RecvRawData(void *buf, size_t size, ndtime_t waittm) = 0 ;
	virtual int CryptMsg(nd_usermsgbuf_t *msghdr, bool bEncrypt=true) = 0 ;
	virtual int CheckValid() = 0;
	virtual int WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time=100) = 0;
	virtual int Update(ndtime_t wait_time) = 0;
    virtual void InstallMsgFunc(nd_iconn_func, ndmsgid_t maxid, ndmsgid_t minid,const char *name=NULL) = 0;
	virtual int CallMsgHandle(nd_usermsgbuf_t *msgbuf) = 0 ;
	virtual	bool TestMsgIsHandle(ndmsgid_t maxid, ndmsgid_t minid) = 0;
    virtual void SetDftMsgHandler(nd_iconn_func) = 0;
	virtual void SetMsgNum(int maxmsg_num , int maxid_start)  = 0;
	virtual int Reconnect(ndip_t& IP, int port,void *proxy=NULL) = 0 ;//connect to another host
	virtual int ExchangeKey(void *output_key) =0;
	virtual const char *ErrorDesc() =0;
	virtual const char *ConvertErrorDesc(NDUINT32 errcode) =0;
    virtual void *GetUserData() = 0;
    virtual void SetUserData(void *pData) = 0;

	virtual int ioctl(int cmd, void *val, int *size) = 0;
	virtual int GetStatus() = 0;

protected:
	NDIConn() {} 
	virtual~NDIConn() {}
};
 */

ND_CONNCLI_API int ndInitNet();
ND_CONNCLI_API void ndDeinitNet();
#define InitNet ndInitNet
#define DeinitNet ndDeinitNet
typedef NDCliConnector NDIConn;
ND_CONNCLI_API NDCliConnector* CreateConnectorObj(const char *protocol_name);
ND_CONNCLI_API void DestroyConnectorObj(NDIConn *pconn);
//ND_CONNCLI_API NDIConn * htoConnector(nd_handle h);

ND_CONNCLI_API void* ndSetLogoutFunc(void *func);
ND_CONNCLI_API void ndSetLogFile(const char *pathfile);
ND_CONNCLI_API int ndSetTimeoutVal(int val);

ND_CONNCLI_API int nd_exchange_key(nd_handle nethandle, void *output_key);
ND_CONNCLI_API int nd_checkErrorMsg(nd_handle nethandle,  nd_usermsghdr_t *msg);

ND_CONNCLI_API int ndSendAndWaitMessage(nd_handle nethandle, nd_usermsgbuf_t *sendBuf, nd_usermsgbuf_t* recvBuf, ndmsgid_t waitMaxid, ndmsgid_t waitMinid, int sendFlag, int timeout);

//set terminate callback function , return old function
ND_CONNCLI_API ndNetFunc ndSetTerminateFunc(ndNetFunc func);


ND_CONNCLI_API int ndGetTimeoutVal();


#endif
