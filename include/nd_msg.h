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

#define ND_MAIN_MSG_CAPACITY 32
#define ND_MSG_BASE_ID 0

enum eNDMsgMainID{
    ND_MAIN_ID_SYS,
    ND_MAIN_ID_LOGIN,
    ND_MAIN_ID_NUMBER,
};

enum eNDMsgSys{
	ND_MSG_SYS_ECHO,
    ND_MSG_SYS_ERROR,
	ND_MSG_SYS_GETVERSION,
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
    
    ND_MSG_SYS_SET_MSGLOG , // set message(maxid, minid) logged when recv
	ND_MSG_SYS_GAME_TIME ,
	ND_MSG_SYS_RELOAD_DATA,
	ND_MSG_SYS_SHUTDOW_NTF, //close server ntf
	ND_MSG_SYS_BEGIN_MEM_STATICS,
	ND_MSG_SYS_END_MEM_STATICS,
	ND_MSG_SYS_GET_USER_DEFINE_DATA,	//get user defined data format,from datatype.xml

	ND_MSG_SYS_REQUEST_CLIENT_LOG_FILE, //get the log file from client (32bit:user_id, text:file_name) 
	ND_MSG_SYS_BACK_CLIENT_LOG_FILE, //get the log file from client (32bit:user_id,text:file_name, bin:file_data) 

	ND_MSG_SYS_GET_MESSAGE_FORMAT_LIST , // get message id, name ,and data stream-format : 16bits:number [16bits:id, string:name, string:format]

	ND_MSG_SYS_GET_MESSAGE_BUILD_TIME, //get message build time or version 
	ND_MSG_SYS_GET_ERROR_DESC, // SEND, int32 errorid, recv { int32:errorId, string:errordesc}
	ND_MSG_SYS_REDIRECT_SRV_LOG_OUTPUT, //server redirect send log to client
	ND_MSG_SYS_OPEN_LOG, // let server open/close log format: {uint8:logtype, unit8:isOpen}
	ND_MSG_SYS_SET_MSG_PRINT,// set message(maxid, minid) print print format and data
	ND_MSG_SYS_CLOSE_MESSAGE_HANDLER,//  message(maxid, minid) 
	ND_MSG_SYS_GET_APPLICATION_NET_PROCOCOL_VERSION , // get application net prococol communation version
	ND_MSG_SYS_INSTALL_HANDLER, // FORMAT 16bits:maxId,16bit:minId, 8bits:privilege, 8bits:byteOrder, string:funcName, bindata:funcBody

	ND_MSG_SYS_REQ_OTHER_STORE_FILE,		//GM request other player handl in client, format 32bit:roleId,string:fileName, bindata:filedata
	ND_MSG_SYS_REQ_OTHER_INSTALL_HANDLER, //GM request other player handl in client, FORMAT 32bit:roleId,16bits:maxId,16bit:minId,  8bits:byteOrder, string:funcName, bindata:funcBody
	ND_MSG_SYS_REQ_OTHER_RELOAD_SCRIPT,		// format :string:filename
	ND_MSG_SYS_CALL_CLIENT_MSG_PROC,		// resend msg to client format:aimRoleId, simulateMsgId(16), simulateMessage

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
