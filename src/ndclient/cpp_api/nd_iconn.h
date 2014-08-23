/* file nd_iconn.h
 *
 * connector base interface 
 *
 * create by duan
 * 2011/4/2 15:57:50
 */

#ifndef _ND_ICONN_H_
#define _ND_ICONN_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef CPPAPI
#ifdef __cplusplus
#define CPPAPI extern "C" 
#else 
#define CPPAPI 
#endif 
#endif

#ifdef _MSC_VER

#ifdef CONN_CLI_EXPORTS
#define ND_CONNCLI_API 				CPPAPI  __declspec(dllexport)
#define ND_CONNCLI_CLASS 			__declspec(dllexport)
#else 
#define ND_CONNCLI_API 				CPPAPI __declspec(dllimport)
#define ND_CONNCLI_CLASS 			__declspec(dllimport)
#endif

#else

#define ND_CONNCLI_API 				CPPAPI
#define ND_CONNCLI_CLASS 			
#endif


#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"
#include "ndapplib/nd_msgpacket.h"


class NDIConn ;
typedef int (*nd_msg_func)(NDIConn* pconn, nd_usermsgbuf_t *msg , nd_handle listener);

struct pg_proxy_info
{
	int proxy_type ;
	short proxy_port;
	char proxy_host[128];
	char user[64];
	char password[64] ;
};

//net connector 
class NDIConn
{
public :		
	virtual int Open(char*host, int port,char *protocol_name, pg_proxy_info *proxy=NULL) = 0;
	virtual int Close(int force=0) = 0;
	virtual int Send(int maxid, int minid, void *data, size_t size)  = 0;
	virtual int SendMsg(NDSendMsg &msg, int flag=0) = 0;
	virtual int SendMsg(nd_usermsgbuf_t *msghdr, int flag=0) = 0;
	virtual int SendRawData(void *data , size_t size)  = 0;
	virtual int RecvRawData(void *buf, size_t size, ndtime_t waittm) = 0 ;
	virtual int CheckValid() = 0;
	virtual int WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time=100) = 0;
	virtual int Update(ndtime_t wait_time) = 0;
	virtual void InstallMsgFunc( nd_msg_func, ndmsgid_t maxid, ndmsgid_t minid) = 0;
	virtual void SetMsgNum(int maxmsg_num , int maxid_start)  = 0;
	virtual int Reconnect(ndip_t IP, int port,pg_proxy_info *proxy=NULL) = 0 ;//connect to another host
	virtual NDUINT32 GetID()  = 0;
	virtual void SetID(NDUINT32 id) = 0;
	virtual NDUINT32 GetType()  = 0;
	virtual void SetType(NDUINT32 type) = 0;
	virtual nd_handle GetHandle()=0;
	virtual int ExchangeKey() =0;
	virtual int LastError()=0 ;
	virtual void SetLastError(NDUINT32 errcode) =0;
	virtual const char *ErrorDesc() =0;
protected:
	NDIConn() {} 
	virtual~NDIConn() {}
};

ND_CONNCLI_API int InitNet() ;
ND_CONNCLI_API void DeinitNet() ;
ND_CONNCLI_API NDIConn* CreateConnectorObj(char *protocol_name) ;
ND_CONNCLI_API void DestroyConnectorObj(NDIConn *pconn) ;
ND_CONNCLI_API NDIConn * htoConnector(nd_handle h);

#endif
