//
//  net_conn.h
//  FlashRoutes
//
//  Created by duanxiuyun on 14-6-25.
//  Copyright (c) 2014 duanxiuyun. All rights reserved.
//
//  bridge ND module and others .
// Can be used in unity3d or unreal and etc..
//

#ifndef _NET_CONN_H_
#define _NET_CONN_H_


#include <stdlib.h>


typedef void* netObject ;


enum eNDSendFlag {
    ND_ESF_NORMAL = 0 ,		// normal
    ND_ESF_WRITEBUF =1,		// write buf
    ND_ESF_URGENCY = 2,		// send right now
    ND_ESF_POST	= 4,		// normal send, if buffer full the data will lost
    ND_ESF_ENCRYPT = 8			// encrypt
};


enum eStreamType{
    ESTREAM_BYTE,
    ESTREAM_SHORT,
    ESTREAM_INT32,
    ESTREAM_INT64,
    ESTREAM_FLOAT,
    ESTREAM_TEXT,
    ESTREAM_BIN
    
};


#pragma pack(push, 1)

struct ndMsgData
{
    unsigned short length;
    unsigned char version;
    unsigned char reserved;
    
    unsigned char maxID;
    unsigned char minID;
    unsigned char data[0xfffa] ;
};

#pragma pack(pop)

typedef int (*ndNetFunc)(netObject netObj, unsigned char *data, int dataLen );

typedef int (*ndBigDataHandler)(netObject nethandle,unsigned long long param , void *data, size_t datalen) ;

#define NDNET_INSTALL_MSG(netObj, msgFunc, maxID, minID) \
	ndNetFuncInstall(netObj,msgFunc, maxID, minID, #maxID"-"#minID) 

//connect server
ND_CONNCLI_API netObject ndOpenConnect(const char *host, int port);
ND_CONNCLI_API void ndClostConnect(netObject netObj);

//send message
/* send message and data */
ND_CONNCLI_API int ndSend(netObject netObj, int maxid, int minid, void *data, unsigned int size);
/* send message with format*/
ND_CONNCLI_API int ndSendFormat(netObject netObj, int maxid, int minid, int argc, ...);
/* send data ,before send the data need to convert to net byte-order*/
ND_CONNCLI_API int ndSendData(netObject netObj, char *data, int dataLen, int flag);
/* send struct message, ndMsgData::data need to convert to net byteoreder */
ND_CONNCLI_API int ndSendMsg(netObject netObj, struct ndMsgData *data, int flag);
/* send raw data , this function do not convert or format data */
ND_CONNCLI_API int ndSendRaw(netObject netObj, char *data, int size);

/* send data that the data len more than 64k */
ND_CONNCLI_API int ndBigDataSend(netObject netObj, unsigned long long param, void *data, size_t datalen);
ND_CONNCLI_API void ndSetBigDataHandler(netObject netObj, ndBigDataHandler entry);

/* send a wrapped message */
ND_CONNCLI_API int ndSendWrapMsg(netObject netObj, netObject msgObj, int flag);


//install message handle functions
ND_CONNCLI_API int ndNetFuncInstall(netObject netObj, ndNetFunc func, int maxID, int minID, const char *name);
ND_CONNCLI_API int ndSetDftMsgHandler(netObject netObj, ndNetFunc dftFunc);

//tic net message , maybe you need a single thread to call this function
ND_CONNCLI_API int ndUpdateConnect(netObject netObj, int timeOutMS);
//wait net message untill timeout, when you get the data ,you need handle it yourself
ND_CONNCLI_API int ndWaitMsg(netObject netObj, char *buf, int timeOutMS);

//set terminate callback function , return old function
ND_CONNCLI_API ndNetFunc ndSetTerminateFunc(ndNetFunc func);

ND_CONNCLI_API int ndGetLastError(netObject netObj);

ND_CONNCLI_API const char *ndGetLastErrorDesc(netObject netObj);

//init/deinit net 
ND_CONNCLI_API int ndInitNet();
ND_CONNCLI_API void ndDeinitNet();

//create message wrapper
ND_CONNCLI_API netObject ndMsgInputWrapperCreate(unsigned char *data, int dataLen);
ND_CONNCLI_API int ndMsgInputWrapperDestroy(netObject msgWrapper, int flag);

//crypt message 
ND_CONNCLI_API int ndCryptMsg(netObject netObj, netObject msgObj, int bEncrypt);
ND_CONNCLI_API int ndGetMsglen(netObject msgObj);
ND_CONNCLI_API char* ndGetMsgAddr(netObject msgObj);

// message read wrapper
ND_CONNCLI_API unsigned char ndMsgWrapperReadInt8(netObject msgWrapper);
ND_CONNCLI_API unsigned short ndMsgWrapperReadInt16(netObject msgWrapper);
ND_CONNCLI_API unsigned int ndMsgWrapperReadInt32(netObject msgWrapper);
ND_CONNCLI_API unsigned long long ndMsgWrapperReadInt64(netObject msgWrapper);
ND_CONNCLI_API float ndMsgWrapperReadFloat(netObject msgWrapper);
ND_CONNCLI_API double ndMsgWrapperReadDouble(netObject msgWrapper);
ND_CONNCLI_API unsigned int ndMsgWrapperReadText(netObject msgWrapper, unsigned char *buf, int size);
ND_CONNCLI_API unsigned int ndMsgWrapperReadBin(netObject msgWrapper, unsigned char *buf, int size_buf);

ND_CONNCLI_API int ndMsgWrapperReadMaxID(netObject msgWrapper);
ND_CONNCLI_API int ndMsgWrapperReadMinID(netObject msgWrapper);

//output Message wrapper

ND_CONNCLI_API netObject ndMsgOutputWrapperCreate(int maxID, int minID);
ND_CONNCLI_API int ndMsgOuputWrapperDestroy(netObject msgWrapper, int flag);

ND_CONNCLI_API int ndMsgWrapperWriteInt8(netObject msgWrapper, unsigned char val);
ND_CONNCLI_API int ndMsgWrapperWriteInt16(netObject msgWrapper, unsigned short val);
ND_CONNCLI_API int ndMsgWrapperWriteInt32(netObject msgWrapper, unsigned int val);
ND_CONNCLI_API int ndMsgWrapperWriteInt64(netObject msgWrapper, unsigned long long val);
ND_CONNCLI_API int ndMsgWrapperWriteFloat(netObject msgWrapper, float val);
ND_CONNCLI_API int ndMsgWrapperWriteDouble(netObject msgWrapper, double val);
ND_CONNCLI_API int ndMsgWrapperWriteText(netObject msgWrapper, const char *text);
ND_CONNCLI_API int ndMsgWrapperWriteBin(netObject msgWrapper, char *buf, int size_buf);

// for test
ND_CONNCLI_API int ndSentTest(netObject netObj);
ND_CONNCLI_API void ndMsgfuncInit(netObject netObj);


ND_CONNCLI_API int nd_exchange_key(netObject nethandle, void *output_key);
ND_CONNCLI_API int nd_checkErrorMsg(netObject nethandle, struct ndMsgData *msg);

ND_CONNCLI_API int ndSendAndWaitMessage(nd_handle nethandle, nd_usermsgbuf_t *sendBuf, nd_usermsgbuf_t* recvBuf, ndmsgid_t waitMaxid, ndmsgid_t waitMinid, int sendFlag, int timeout);
//add function call when object destroyed
typedef void (*nd_conn_close_entry)(netObject handle, void *param) ;
ND_CONNCLI_API int ndAddOnCloseCallback(netObject handle, nd_conn_close_entry callback, void *param);
ND_CONNCLI_API int ndAddOnDestroyCallback(netObject handle, nd_conn_close_entry callback, void *param);
ND_CONNCLI_API int ndDelDestroyCallback(netObject handle, nd_conn_close_entry callback, void *param);

/*
CPPAPI netObject ndGetConnector() ;
CPPAPI int SendFormat(int maxid, int minid, int argc, ...) ;
CPPAPI void init_messageHandler() ;

CPPAPI int TestNet();

CPPAPI int sendTest() ;
*/



#endif
