//
//  netRecvHdler.cpp
//  FlashRoutes
//
//  Created by duanxiuyun on 14-6-27.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//

// handler of net recv

#include "net_conn.h"
#include "nd_msgpacket.h"
#include "FRmsgdef.h"



int sendTest()
{
    NDOStreamMsg omsg(ND_MAIN_ID_SYS,ND_MSG_SYS_ECHO) ;
    omsg.Write((NDUINT32)35789) ;
    omsg.Write((float)3.14125f);
    omsg.Write((double)960000.345) ;
    omsg.Write((NDUINT8*)"hello world") ;
    
    
    nd_handle handle = FRGetConnector() ;
    return nd_connector_send(handle,(nd_packhdr_t*) (omsg.GetMsgAddr()), ESF_URGENCY) ;
    
}

#define MSG_ENTRY_INSTANCE(name) \
    int name (nd_handle nethandle,nd_usermsgbuf_t *msg, nd_handle h_listen)

MSG_ENTRY_INSTANCE(netmsg_sys_echo)
{
    NDUINT32 val32 ;
    float valf ;
    double vald ;
    NDIStreamMsg inmsg(msg) ;
    NDUINT8 echotext[256] ;
    char buf[512] ;
    echotext[0] = 0 ;
    
    inmsg.Read(val32);
    inmsg.Read(valf);
    inmsg.Read(vald);
    
    inmsg.Read(echotext,sizeof(echotext)) ;
    
    snprintf(buf, sizeof(buf), "echo: int=%d, float=%f, double=%f,\ntext= %s\n",val32,valf,vald, echotext);
    //netDataHandler(buf);
    //nd_log_screen("echo: int=%d, float=%f, double=%f,\ntext= %s\n",val32,valf,vald, echotext) ;
    return 0 ;
}


MSG_ENTRY_INSTANCE(netmsg_sys_error)
{
    nd_log_screen("recv error ") ;
    return 0 ;
}

MSG_ENTRY_INSTANCE(netmsg_sys_time)
{
    nd_log_screen("recv system time ") ;
    return 0 ;
}


MSG_ENTRY_INSTANCE(netmsg_login_ack)
{
    nd_log_screen("recv echo ") ;
    return 0 ;
}

#define MSG_HANDLER_INS(_f, _maxid, _minid) \
    nd_msgentry_install(FRGetConnector(), _f,_maxid, _minid,EPL_CONNECT)

void init_messageHandler()
{
    MSG_HANDLER_INS(netmsg_sys_echo,ND_MAIN_ID_SYS,ND_MSG_SYS_ECHO) ;
    
    MSG_HANDLER_INS(netmsg_sys_error,ND_MAIN_ID_SYS,ND_MSG_SYS_ERROR) ;
    
    MSG_HANDLER_INS(netmsg_sys_time,ND_MAIN_ID_SYS,ND_MSG_SYS_TIME) ;
    MSG_HANDLER_INS( netmsg_login_ack,ND_MAIN_ID_LOGIN,EFRMSG_LOGIN) ;
}
