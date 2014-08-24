//
//  net_conn.h
//  FlashRoutes
//
//  Created by duanxiuyun on 14-6-25.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//

#ifndef _NET_CONN_H_
#define _NET_CONN_H_

#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"


enum eStreamType{
    ESTREAM_BYTE,
    ESTREAM_SHORT,
    ESTREAM_INT32,
    ESTREAM_INT64,
    ESTREAM_FLOAT,
    ESTREAM_TEXT,
    ESTREAM_BIN
    
};

nd_handle ndGetConnector() ;
int SendFormat(int maxid, int minid, int argc, ...) ;
void init_messageHandler() ;

int TestNet();

int sendTest() ;

#endif
