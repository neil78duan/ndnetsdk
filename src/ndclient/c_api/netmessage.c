//
//  netmessage.c
//  FlashRoutes
//
//  Created by duanxiuyun on 14-6-25.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//

#include "net_conn.h"


//#include "FlashRoutes-Swift.h"

int UserLogin(const char *name, const char *pwd)
{
    
    return SendFormat(ND_MAIN_ID_LOGIN, EFRMSG_LOGIN,
                      4,
                      ESTREAM_TEXT, name,
                      ESTREAM_TEXT, pwd) ;
    
}

extern void netDataHandler(char *text);

int frTestNet()
{
    static int index = 0 ;
    char buf[128] ;
    snprintf(buf, sizeof(buf), "test net echo %d", index) ;
    ++index ;
    
    netDataHandler(buf);
    
    return SendFormat(ND_MAIN_ID_SYS, ND_MSG_SYS_ECHO,
               2,
               ESTREAM_TEXT, buf) ;
    

}