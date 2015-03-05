//
//  net_conn.h
//  FlashRoutes
//
//  Created by duanxiuyun on 14-6-25.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//
//  bridge ND module and others .
// Can be used in unity3d or unreal and etc..
//

#ifndef _NET_CONN_H_
#define _NET_CONN_H_

#include <stdlib.h>

typedef void* netObject ;

#ifndef CPPAPI
#define CPPAPI extern "C" 
#endif 

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
	ndNetFuncInstall(netObj,msgFunc, maxID, minID, "msgName_" #maxID"-"#minID) 

//connect server
CPPAPI netObject ndOpenConnect(const char *host, int port) ;
CPPAPI void ndClostConnect(netObject netObj) ;

//send message
/* send message and data */
CPPAPI int ndSend(netObject netObj,int maxid, int minid, void *data, unsigned int size) ;
/* send message with format*/
CPPAPI int ndSendFormat(netObject netObj,int maxid, int minid, int argc, ...) ;
/* send data ,before send the data need to convert to net byte-order*/
CPPAPI int ndSendData(netObject netObj, char *data, int dataLen, int flag) ;
/* send struct message, ndMsgData::data need to convert to net byteoreder */
CPPAPI int ndSendMsg(netObject netObj,struct ndMsgData *data, int flag) ;
/* send raw data , this function do not convert or format data */
CPPAPI int ndSendRaw(netObject netObj,char *data, int size) ;

/* send data that the data len more than 64k */
CPPAPI int ndBigDataSend(netObject netObj,unsigned long long param, void *data, size_t datalen) ;
CPPAPI void ndSetBigDataHandler(netObject netObj,ndBigDataHandler entry) ;

/* send a wrapped message */
CPPAPI int ndSendWrapMsg(netObject netObj,netObject msgObj, int flag) ;


//install message handle functions
CPPAPI int ndNetFuncInstall(netObject netObj,ndNetFunc func, int maxID, int minID,const char *name) ;
CPPAPI int ndSetDftMsgHandler(netObject netObj,ndNetFunc dftFunc) ;

//tic net message , maybe you need a single thread to call this function
CPPAPI int ndUpdateConnect(netObject netObj, int timeOutMS) ;
//wait net message untill timeout, when you get the data ,you need handle it yourself
CPPAPI int ndWaitMsg(netObject netObj, char *buf, int timeOutMS) ;

//set terminate callback function , return old function
CPPAPI ndNetFunc ndSetTerminateFunc(ndNetFunc func) ;

CPPAPI int ndGetLastError(netObject netObj) ;

CPPAPI const char *ndGetLastErrorDesc(netObject netObj) ;

//init/deinit net 
CPPAPI int ndInitNet() ;
CPPAPI void ndDeinitNet() ;

//create message wrapper
CPPAPI netObject ndMsgInputWrapperCreate(unsigned char *data, int dataLen);
CPPAPI int ndMsgInputWrapperDestroy(netObject msgWrapper , int flag);

//crypt message 
CPPAPI int ndCryptMsg(netObject netObj,netObject msgObj, int bEncrypt) ;
CPPAPI int ndGetMsglen(netObject msgObj) ;
CPPAPI char* ndGetMsgAddr(netObject msgObj) ;

// message read wrapper
CPPAPI unsigned char ndMsgWrapperReadInt8(netObject msgWrapper) ;
CPPAPI unsigned short ndMsgWrapperReadInt16(netObject msgWrapper) ;
CPPAPI unsigned int ndMsgWrapperReadInt32(netObject msgWrapper) ;
CPPAPI unsigned long long ndMsgWrapperReadInt64(netObject msgWrapper) ;
CPPAPI float ndMsgWrapperReadFloat(netObject msgWrapper) ;
CPPAPI double ndMsgWrapperReadDouble(netObject msgWrapper) ;
CPPAPI unsigned int ndMsgWrapperReadText(netObject msgWrapper, unsigned char *buf, int size) ;
CPPAPI unsigned int ndMsgWrapperReadBin (netObject msgWrapper, unsigned char *buf, int size_buf) ;

CPPAPI int ndMsgWrapperReadMaxID (netObject msgWrapper) ;
CPPAPI int ndMsgWrapperReadMinID (netObject msgWrapper) ;

//output Message wrapper

CPPAPI netObject ndMsgOutputWrapperCreate(int maxID, int minID);
CPPAPI int ndMsgOuputWrapperDestroy(netObject msgWrapper , int flag);

CPPAPI int ndMsgWrapperWriteInt8(netObject msgWrapper,unsigned char val);
CPPAPI int ndMsgWrapperWriteInt16(netObject msgWrapper,unsigned short val);
CPPAPI int ndMsgWrapperWriteInt32(netObject msgWrapper,unsigned int val);
CPPAPI int ndMsgWrapperWriteInt64(netObject msgWrapper,unsigned long long val);
CPPAPI int ndMsgWrapperWriteFloat(netObject msgWrapper,float val);
CPPAPI int ndMsgWrapperWriteDouble(netObject msgWrapper,double val);
CPPAPI int ndMsgWrapperWriteText(netObject msgWrapper, const char *text);
CPPAPI int ndMsgWrapperWriteBin (netObject msgWrapper,  char *buf, int size_buf);

// for test
CPPAPI int ndSentTest(netObject netObj) ;
CPPAPI void ndMsgfuncInit(netObject netObj) ;


CPPAPI int nd_exchange_key(netObject nethandle,void *output_key) ;
CPPAPI int nd_checkErrorMsg(netObject nethandle,struct ndMsgData *msg) ;

//add function call when object destroyed
typedef void (*nd_conn_close_entry)(netObject handle, void *param) ;
CPPAPI int ndAddOnCloseCallback(netObject handle,nd_conn_close_entry callback, void *param) ;
CPPAPI int ndAddOnDestroyCallback(netObject handle,nd_conn_close_entry callback, void *param) ;
CPPAPI int ndDelDestroyCallback(netObject handle,nd_conn_close_entry callback, void *param) ;

/*
CPPAPI netObject ndGetConnector() ;
CPPAPI int SendFormat(int maxid, int minid, int argc, ...) ;
CPPAPI void init_messageHandler() ;

CPPAPI int TestNet();

CPPAPI int sendTest() ;
*/



#endif
