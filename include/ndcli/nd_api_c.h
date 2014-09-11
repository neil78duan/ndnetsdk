//
//  net_conn.h
//  FlashRoutes
//
//  Created by duanxiuyun on 14-6-25.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//
//  bridge ND module and others .
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


//typedef int (*ndNetFunc)(netObject netObj, char *msg_buf , netObject* listener );
typedef int (*ndNetFunc)(netObject netObj, char *data, int dataLen );

CPPAPI netObject ndOpenConnect(const char *host, int port) ;
CPPAPI void ndClostConnect(netObject netObj) ;

CPPAPI int ndSendMsg(netObject netObj,int maxid, int minid, int argc, ...) ;
CPPAPI int ndSendData(netObject netObj, char *data, int dataLen, int flag) ;
CPPAPI int ndSendMsgEx(netObject netObj,struct ndMsgData *data, int flag) ;

CPPAPI int ndNetFuncInstall(netObject netObj,ndNetFunc func, int maxID, int minID) ;
CPPAPI int ndSetDftMsgHandler(netObject netObj,ndNetFunc dftFunc) ;

CPPAPI int ndUpdateConnect(netObject netObj,unsigned int timeOutMS) ;
CPPAPI int ndWaitMsg(netObject netObj, char *buf, int timeOutMS) ;

// for test
CPPAPI int ndSentTest(netObject netObj) ;
CPPAPI void ndMsgfuncInit(netObject netObj) ;

CPPAPI int ndInitNet() ;
CPPAPI void ndDeinitNet() ;

/*
CPPAPI netObject ndGetConnector() ;
CPPAPI int SendFormat(int maxid, int minid, int argc, ...) ;
CPPAPI void init_messageHandler() ;

CPPAPI int TestNet();

CPPAPI int sendTest() ;
*/



#endif
