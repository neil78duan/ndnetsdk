//
//  msg_format.cpp
//  FlashRoutes
//
//  Created by duanxiuyun on 14-6-25.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//

#include "ndcli/nd_api_c.h"
#include "nd_msgpacket.h"


int SendFormat(int maxid, int minid,int argc, ...)
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
    
    
    nd_handle handle = ndGetConnector() ;
    return nd_connector_send(handle,(nd_packhdr_t*) (omsg.GetMsgAddr()), ESF_URGENCY) ;
    
}

