//
//  nd_msg.h
//  srvtest1
//
//  Created by duanxiuyun on 14-6-24.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//

#ifndef _ND_MSG_ID_H_
#define _ND_MSG_ID_H_


#include "nd_net/nd_netlib.h"

#define ND_MAIN_MSG_CAPACITY 16
#define ND_MSG_BASE_ID 0

enum eNDMsgMainID{
    ND_MAIN_ID_SYS,
    ND_MAIN_ID_LOGIN,
    ND_MAIN_ID_NUMBER
};

enum eNDMsgSys{
    ND_MSG_SYS_GETVERSION,
    ND_MSG_SYS_ERROR,
    ND_MSG_SYS_ECHO,
    ND_MSG_SYS_TIME,
    ND_MSG_SYS_BROADCAST,
    ND_MSG_SYS_NUMBER
};

enum eNDmsgLogin{
    EFRMSG_LOGIN,
    EFRMSG_LOGOUT,
    
    EFRMSG_NUMBERS
};

#define USER_NAME_SIZE 128
#define PASSWORD_SIZE 24


#define INIT_MSG_HEADER(_mainid, _subid, _length) \
    maxid = _mainid; \
    minid =	_subid;  \
    packet_hdr.length = (NDUINT16) ( _length )

#pragma pack(push, 1)
/*
//#include "nd_common/nd_common.h"
struct nd_msg_base : public nd_usermsghdr_t
{
    nd_msg_base()
    {
        nd_hdr_init((nd_packhdr_t *)&packet_hdr) ;
        maxid = 0 ;
        minid = 0;
    }
    nd_msg_base(ndmsgid_t xid, ndmsgid_t nid)
    {
        nd_hdr_init((nd_packhdr_t *)&packet_hdr) ;
        maxid = xid ;
        minid = nid;	
    }
};

struct sys_getversion_ack : public nd_msg_base
{
    NDUINT16 ver ;
    
    sys_getversion_ack() {
        INIT_MSG_HEADER(ND_MAIN_ID_SYS,ND_MSG_SYS_GETVERSION, sizeof(*this)) ;
        ver = 0;
    }
};
*/

#pragma pack(pop)

#endif 
