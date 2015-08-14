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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"
#include "ndapplib/nd_msgpacket.h"
//#include "ndcli/nd_api_c.h"
#endif 

class NDIConn ;
typedef int (*nd_iconn_func)(NDIConn* pconn, nd_usermsgbuf_t *msg );

#define CONNECT_INSTALL_MSG(connect, msgFunc, maxID, minID) \
	(connect)->InstallMsgFunc(msgFunc, maxID, minID, #maxID"-"#minID) 

#define CONNECT_INSTALL_MSG_INT16(connect, msgFunc, msgID) \
	(connect)->InstallMsgFunc(msgFunc, ND_HIBYTE(msgID),ND_LOBYTE(msgID), #msgID)


typedef int (*nd_bigdata_handler)(nd_handle nethandle,  NDUINT64 param , void *data, size_t datalen) ;

#define  WAITMSG_TIMEOUT 300000

//net connector 
class NDIConn
{
public :		
	virtual int Open(const char*host, int port,const char *protocol_name, nd_proxy_info *proxy=NULL) = 0;
	virtual int Open(ndip_t ip, int port,const char *protocol_name, nd_proxy_info *proxy=NULL) = 0;
	virtual int Close(int force=0) = 0;
	virtual int Send(int maxid, int minid, void *data, size_t size)  = 0;
	virtual int SendMsg(NDSendMsg &msg, int flag=0) = 0;
	virtual int SendMsg(nd_usermsgbuf_t *msghdr, int flag=0) = 0;
	virtual int SendRawData(void *data , size_t size)  = 0;
	virtual int RecvRawData(void *buf, size_t size, ndtime_t waittm) = 0 ;
	virtual int BigDataSend(NDUINT64 param, void *data, size_t datalen) =0;
	virtual int CryptMsg(nd_usermsgbuf_t *msghdr, bool bEncrypt=true) = 0 ;
	virtual int CheckValid() = 0;
	virtual int WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time=100) = 0;
	virtual int Update(ndtime_t wait_time) = 0;
    virtual void InstallMsgFunc(nd_iconn_func, ndmsgid_t maxid, ndmsgid_t minid,const char *name=NULL) = 0;
    virtual void SetDftMsgHandler(nd_iconn_func) = 0;
	virtual void SetMsgNum(int maxmsg_num , int maxid_start)  = 0;
	virtual int Reconnect(ndip_t IP, int port,nd_proxy_info *proxy=NULL) = 0 ;//connect to another host
	virtual nd_handle GetHandle()=0;
	virtual int ExchangeKey(void *output_key) =0;
	virtual int LastError()=0 ;
	virtual void SetLastError(NDUINT32 errcode) =0;
	virtual const char *ErrorDesc() =0;
	virtual const char *ConvertErrorDesc(NDUINT32 errcode) =0;
    virtual void *GetUserData() = 0;
    virtual void SetUserData(void *pData) = 0;

	virtual int ioctl(int cmd, void *val, int *size) = 0;
	virtual void SetBigDataHandler(nd_bigdata_handler entry) = 0 ;
	
protected:
	NDIConn() {} 
	virtual~NDIConn() {}
};

ND_COMMON_API int InitNet();
ND_COMMON_API void DeinitNet();
ND_COMMON_API NDIConn* CreateConnectorObj(const char *protocol_name);
ND_COMMON_API void DestroyConnectorObj(NDIConn *pconn);
ND_COMMON_API NDIConn * htoConnector(nd_handle h);

#endif
