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

typedef void* netObject ;

#ifndef CPPAPI
#define CPPAPI extern "C" 
#endif 



enum eStreamType{
    ESTREAM_BYTE,
    ESTREAM_SHORT,
    ESTREAM_INT32,
    ESTREAM_INT64,
    ESTREAM_FLOAT,
    ESTREAM_TEXT,
    ESTREAM_BIN
    
};

struct ndMsgData
{
    unsigned short length;
    unsigned char version;
    unsigned char reserved;
    
    unsigned char maxID;
    unsigned char minID;
    unsigned char data[0xfffa] ;
};


typedef int (*ndNetFunc)(netObject netObj, unsigned char *data, int dataLen );

//connect server
CPPAPI netObject ndOpenConnect(const char *host, int port) ;
CPPAPI void ndClostConnect(netObject netObj) ;

//send message
/* send message with format*/
CPPAPI int ndSendMsg(netObject netObj,int maxid, int minid, int argc, ...) ;
/* send data ,before send the data need to convert to net byte-order*/
CPPAPI int ndSendData(netObject netObj, char *data, int dataLen, int flag) ;
/* send struct message, ndMsgData::data need to convert to net byteoreder */
CPPAPI int ndSendMsgEx(netObject netObj,struct ndMsgData *data, int flag) ;

//install message handle functions
CPPAPI int ndNetFuncInstall(netObject netObj,ndNetFunc func, int maxID, int minID) ;
CPPAPI int ndSetDftMsgHandler(netObject netObj,ndNetFunc dftFunc) ;

//tic net message , maybe you need a single thread to call this function
CPPAPI int ndUpdateConnect(netObject netObj, int timeOutMS) ;
//wait net message untill timeout, when you get the data ,you need handle it yourself
CPPAPI int ndWaitMsg(netObject netObj, char *buf, int timeOutMS) ;

//init/deinit net 
CPPAPI int ndInitNet() ;
CPPAPI void ndDeinitNet() ;

// for test
CPPAPI int ndSentTest(netObject netObj) ;
CPPAPI void ndMsgfuncInit(netObject netObj) ;


/*
CPPAPI netObject ndGetConnector() ;
CPPAPI int SendFormat(int maxid, int minid, int argc, ...) ;
CPPAPI void init_messageHandler() ;

CPPAPI int TestNet();

CPPAPI int sendTest() ;
*/



#endif
