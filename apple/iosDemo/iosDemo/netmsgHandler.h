//
//  netmsgHandler.h
//  iosDemo
//
//  Created by duanxiuyun on 14-8-24.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//

#ifndef __iosDemo__netmsgHandler__
#define __iosDemo__netmsgHandler__

#include <stdio.h>
#include "ndcli/nd_iconn.h"

void init_messageHandler(NDIConn *pconn);
int timeTick(NDIConn *pconn);
int sendTest(NDIConn *pconn) ;

#endif /* defined(__iosDemo__netmsgHandler__) */
