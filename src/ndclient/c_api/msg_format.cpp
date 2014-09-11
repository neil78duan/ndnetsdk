//
//  msg_format.cpp
//  FlashRoutes
//
//  Created by duanxiuyun on 14-6-25.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//

#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"

#include "nd_msgpacket.h"
#include "ndcli/nd_api_c.h"

#include "nd_msg.h"


int ndSendMsg(netObject netObj,int maxid, int minid,int argc, ...)
{
    int i=0;
    NDOStreamMsg omsg ;
    va_list arg;
    
    omsg.SetID(maxid, minid) ;
    
#define ARG_2_STREAM(_arg, _stream, _type) \
{\
    _type  a= va_arg(_arg, _type ) ; \
    _stream.Write(a) ; \
    i+=2 ;  \
}
    
    va_start (arg, argc);
    while (i<argc) {
        eStreamType type = (eStreamType)  va_arg(arg, int ) ;
        switch (type) {
            case ESTREAM_BYTE:
                ARG_2_STREAM(arg, omsg, NDUINT8);
                break;
            case  ESTREAM_SHORT:
                
                ARG_2_STREAM(arg, omsg, NDUINT16);
                break;
            case  ESTREAM_INT32:
                ARG_2_STREAM(arg, omsg, NDUINT32);
                break;
            case  ESTREAM_INT64:
                ARG_2_STREAM(arg, omsg, NDUINT64);
                break;
            case  ESTREAM_FLOAT:
                ARG_2_STREAM(arg, omsg, float);
                break;
                
            case  ESTREAM_TEXT:
                ARG_2_STREAM(arg, omsg, NDUINT8*);
                break;
            case  ESTREAM_BIN:
            {
                size_t len = va_arg(arg, size_t ) ;
                void *p = va_arg(arg, void* ) ;
                omsg.WriteBin(p, len) ;
            }
                break;
                
            default:
                break;
        }
    }
    va_end (arg);
    
    
    return nd_connector_send((nd_handle)netObj,(nd_packhdr_t*) (omsg.GetMsgAddr()), ESF_URGENCY) ;
    
}

// for test

int ndSentTest(netObject netObj)
{
    NDOStreamMsg omsg(ND_MAIN_ID_SYS,ND_MSG_SYS_ECHO) ;
    omsg.Write((NDUINT32)35789) ;
    omsg.Write((float)3.14125f);
    omsg.Write((double)960000.345) ;
    omsg.Write((NDUINT8*)"hello world") ;
    
    return nd_connector_send((nd_handle)netObj,(nd_packhdr_t*) (omsg.GetMsgAddr()), ESF_URGENCY) ;
    
}

#define MSG_ENTRY_INSTANCE(name) \
int name (nd_handle handle,nd_usermsgbuf_t *msg, nd_handle h_listen)

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

#define MSG_HANDLER_INS(_f, _maxid, _minid) \
nd_msgentry_install((nd_handle)netObj, _f,_maxid, _minid,EPL_CONNECT)

void ndMsgfuncInit(netObject netObj)
{
    
    //MSG_HANDLER_INS(netmsg_sys_echo,ND_MAIN_ID_SYS,ND_MSG_SYS_ECHO) ;
    
    MSG_HANDLER_INS(netmsg_sys_error,ND_MAIN_ID_SYS,ND_MSG_SYS_ERROR) ;
    
    MSG_HANDLER_INS(netmsg_sys_time,ND_MAIN_ID_SYS,ND_MSG_SYS_TIME) ;
    
}


