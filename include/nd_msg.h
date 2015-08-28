//
//  nd_msg.h
//  srvtest1
//
//  Created by duanxiuyun on 14-6-24.
//  Copyright (c) 2014 duanxiuyun. All rights reserved.
//

#ifndef _ND_MSG_ID_H_
#define _ND_MSG_ID_H_


#ifndef BUILD_AS_THIRD_PARTY
#include "nd_net/nd_netlib.h"
#endif

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

	ND_MSG_SYS_CHECK_CRYPT_VERSION ,	//get RSA key version info
	ND_MSG_SYS_GET_PUBLIC_KEY,		//get RSA public md5
	ND_MSG_SYS_GET_PUBLIC_KEY_MD5,	//get RSA public-KEY md5
	ND_MSG_SYS_SEND_SYM_KEY ,		//send SYM encrypt key
	ND_MSG_SYS_SYM_KEY_ACK ,		// acknowledgement of send-sym-key
	
	ND_MSG_SYS_DIRECTLY_TO_CLIENT_WRAPPER,	// send message to other client 
	ND_MSG_SYS_CALL_SESSION_MSGPROC_WRAPPER,	// Send message to other session's message-handler 
	
	ND_MSG_SYS_GET_MESSAGE_NAME ,// GET message name 
	
	ND_MSG_BIG_DATA_TRANSFER ,
	
	ND_MSG_SYS_GET_RLIMIT ,
    
    ND_MSG_SYS_SET_MSGLOG ,
	ND_MSG_SYS_GAME_TIME ,
	ND_MSG_SYS_RELOAD_DATA,
	ND_MSG_SYS_SHUTDOW_NTF, //close server ntf
	ND_MSG_SYS_BEGIN_MEM_STATICS,
	ND_MSG_SYS_END_MEM_STATICS,

    ND_MSG_SYS_NUMBER
};

enum eNDmsgLogin{
    EFRMSG_LOGIN,
    EFRMSG_LOGOUT,
    
    EFRMSG_NUMBERS
};

//#define USER_NAME_SIZE 128
//#define PASSWORD_SIZE 24


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
