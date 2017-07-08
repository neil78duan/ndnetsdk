//
//  msg_format.cpp
//  FlashRoutes
//
//  Created by duanxiuyun on 14-6-25.
//  Copyright (c) 2014 duanxiuyun. All rights reserved.
//

#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"

#include "ndapplib/nd_msgpacket.h"
#include "ndapplib/nd_datatransfer.h"
#include "ndcli/nd_api_c.h"

#include "nd_msg.h"

int ndSend(netObject netObj,int maxid, int minid, void *data, unsigned int size)
{
    NDOStreamMsg omsg(maxid, minid) ;
    if(-1==omsg.WriteBin(data, (size_t) size) ) {
        return -1 ;
    }
    
    return nd_connector_send((nd_handle)netObj,(nd_packhdr_t*) (omsg.GetMsgAddr()), ESF_URGENCY) ;
}


int ndSendFormat(netObject netObj,int maxid, int minid,int argc, ...)
{
    int i=0;
    NDOStreamMsg omsg(maxid, minid) ;
    va_list arg;
    
    //omsg.SetID(maxid, minid) ;
    
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

struct nd_message_wrapper
{
    ND_OBJ_BASE ;
    NDIStreamMsg *inStream ;
    nd_usermsgbuf_t *msgdata ;
    
};

//create message wrapper
netObject ndMsgInputWrapperCreate(unsigned char *data, int dataLen)
{
    nd_message_wrapper *pwrapper = new nd_message_wrapper ;
    if (!pwrapper) {
        return NULL;
    }
    nd_usermsgbuf_t *pMsg = (nd_usermsgbuf_t*) data ;
    size_t size = (size_t) ND_USERMSG_LEN(pMsg) ;
    nd_assert((size_t)dataLen==size) ;
    
    pwrapper->size = sizeof(nd_message_wrapper);
    pwrapper->type = NDHANDLE_USER1 + 1;
    pwrapper->myerrno = NDERR_SUCCESS;
    pwrapper->close_entry = ndMsgInputWrapperDestroy;
    
    pwrapper->msgdata = (nd_usermsgbuf_t*)malloc(size) ;
    
    memcpy(pwrapper->msgdata, data, size) ;
    
    pwrapper->inStream = new NDIStreamMsg(pwrapper->msgdata) ;
    //packet_hton(&crypt_buf) ;
    return (netObject) pwrapper ;
}

int ndMsgInputWrapperDestroy(netObject msgWrapper , int flag)
{
    nd_message_wrapper *pwapper = (nd_message_wrapper*)msgWrapper ;
    if (pwapper) {
        nd_assert(pwapper->size == sizeof(nd_message_wrapper));
        nd_assert(pwapper->type == NDHANDLE_USER1 + 1);
        
        if (pwapper->inStream) {
            delete pwapper->inStream ;
            pwapper->inStream = 0 ;
        }
        if (pwapper->msgdata) {
            free(pwapper->msgdata) ;
            pwapper->msgdata = 0;
        }
        pwapper->type = 0 ;
        delete pwapper ;
        return  0;
    }
    return -1;
}

#define READ_DATA_FROM_WRAPPER(_type , _objectWrapper)  \
    _type a=0;        \
    nd_message_wrapper *pwapper = (nd_message_wrapper*)_objectWrapper ; \
    nd_assert(pwapper->size == sizeof(nd_message_wrapper)); \
    nd_assert(pwapper->type == NDHANDLE_USER1 + 1);         \
    if (pwapper->myerrno) {         \
        return  -1 ;                \
    }                               \
    if (pwapper->inStream->Read(a)==0) {    \
        return a ;                  \
    }                               \
    pwapper->myerrno = NDERR_READ;  \
    return 0

unsigned char ndMsgWrapperReadInt8(netObject msgWrapper)
{
    READ_DATA_FROM_WRAPPER(NDUINT8, msgWrapper) ;
}

unsigned short ndMsgWrapperReadInt16(netObject msgWrapper)
{
    READ_DATA_FROM_WRAPPER(NDUINT16, msgWrapper) ;
}

unsigned int ndMsgWrapperReadInt32(netObject msgWrapper)
{
    READ_DATA_FROM_WRAPPER(NDUINT32, msgWrapper) ;
}
unsigned long long ndMsgWrapperReadInt64(netObject msgWrapper)
{
    READ_DATA_FROM_WRAPPER(NDUINT64, msgWrapper) ;
    
}
float ndMsgWrapperReadFloat(netObject msgWrapper)
{
    READ_DATA_FROM_WRAPPER(float, msgWrapper) ;
    
}
double ndMsgWrapperReadDouble(netObject msgWrapper)
{
    READ_DATA_FROM_WRAPPER(double, msgWrapper) ;
}

unsigned int ndMsgWrapperReadText(netObject msgWrapper, unsigned char *buf, int size)
{
    size_t readLen ;
    nd_message_wrapper *pwapper = (nd_message_wrapper*)msgWrapper ;
    nd_assert(pwapper->size == sizeof(nd_message_wrapper));
    nd_assert(pwapper->type == NDHANDLE_USER1 + 1);
    if (pwapper->myerrno) {
        return  -1 ;
    }
    readLen = pwapper->inStream->Read((NDUINT8*)buf, (size_t) size);
    if (readLen) {
        return (unsigned int)readLen ;
    }
    pwapper->myerrno = NDERR_READ;
    return 0;

}

unsigned int ndMsgWrapperReadBin (netObject msgWrapper, unsigned char *buf, int size_buf)
{
    size_t readLen ;
    nd_message_wrapper *pwapper = (nd_message_wrapper*)msgWrapper ;
    nd_assert(pwapper->size == sizeof(nd_message_wrapper));
    nd_assert(pwapper->type == NDHANDLE_USER1 + 1);
    if (pwapper->myerrno) {
        return  -1 ;
    }
    readLen = pwapper->inStream->ReadBin(buf, (size_t) size_buf);
    if (readLen) {
        return (unsigned int)readLen ;
    }
    pwapper->myerrno = NDERR_READ;
    return 0;
}

int ndMsgWrapperReadMaxID (netObject msgWrapper) 
{
	nd_message_wrapper *pwapper = (nd_message_wrapper*)msgWrapper ;	
	nd_assert(pwapper->size == sizeof(nd_message_wrapper));
	nd_assert(pwapper->type == NDHANDLE_USER1 + 1);
	return (int) pwapper->inStream->MsgMaxid() ;	
}

int ndMsgWrapperReadMinID (netObject msgWrapper) 
{
	nd_message_wrapper *pwapper = (nd_message_wrapper*)msgWrapper ;	
	nd_assert(pwapper->size == sizeof(nd_message_wrapper));
	nd_assert(pwapper->type == NDHANDLE_USER1 + 1);
	return (int) pwapper->inStream->MsgMinid() ;
	
}

// output message wrapper

struct nd_message_out_wrapper
{
    ND_OBJ_BASE ;
    NDOStreamMsg outStream ;
};

netObject ndMsgOutputWrapperCreate(int maxID, int minID)
{
    nd_message_out_wrapper *poutWrapper = new nd_message_out_wrapper ;
    if (!poutWrapper) {
        return NULL;
    }
    
    poutWrapper->size = sizeof(nd_message_out_wrapper);
    poutWrapper->type = NDHANDLE_USER1 + 2;
    poutWrapper->myerrno = NDERR_SUCCESS;
    poutWrapper->close_entry = ndMsgOuputWrapperDestroy;
    
    poutWrapper->outStream.SetID(maxID, minID) ;
    //packet_hton(&crypt_buf) ;
    return (netObject) poutWrapper ;
}


int ndMsgOuputWrapperDestroy(netObject msgWrapper , int flag)
{
    nd_message_out_wrapper *pwapper = (nd_message_out_wrapper*)msgWrapper ;
    if (pwapper) {
        nd_assert(pwapper->size == sizeof(nd_message_out_wrapper));
        nd_assert(pwapper->type == NDHANDLE_USER1 + 2);
        
        delete pwapper ;
        return  0;
    }
    return -1;
}

#define WRITE_DATA_TO_WRAPPER(_type, _wrapper, _val) \
    nd_message_out_wrapper *pwapper = (nd_message_out_wrapper*)_wrapper ; \
    nd_assert(pwapper) ;    \
    nd_assert(pwapper->size == sizeof(nd_message_out_wrapper)); \
    nd_assert(pwapper->type == NDHANDLE_USER1 + 2);\
    if(0==pwapper->outStream.Write((_type)_val) ) { \
        return 0;   \
    }               \
    pwapper->myerrno = NDERR_WRITE ;    \
    return -1


int ndMsgWrapperWriteInt8(netObject msgWrapper,unsigned char val)
{
    WRITE_DATA_TO_WRAPPER(NDUINT8, msgWrapper,val) ;
}

int ndMsgWrapperWriteInt16(netObject msgWrapper,unsigned short val)
{
    WRITE_DATA_TO_WRAPPER(NDUINT16, msgWrapper,val) ;
}

int ndMsgWrapperWriteInt32(netObject msgWrapper,unsigned int val)
{
   WRITE_DATA_TO_WRAPPER(NDUINT32, msgWrapper,val) ;
}
int ndMsgWrapperWriteInt64(netObject msgWrapper,unsigned long long val)
{
    WRITE_DATA_TO_WRAPPER(NDUINT64, msgWrapper,val) ;
    
}
int ndMsgWrapperWriteFloat(netObject msgWrapper,float val)
{
    WRITE_DATA_TO_WRAPPER(float, msgWrapper,val) ;
    
}
int ndMsgWrapperWriteDouble(netObject msgWrapper,double val)
{
    WRITE_DATA_TO_WRAPPER(double, msgWrapper,val) ;
}


int ndMsgWrapperWriteText(netObject msgWrapper, const char *text)
{
    WRITE_DATA_TO_WRAPPER(const NDUINT8*, msgWrapper,text) ;
    
}

int ndMsgWrapperWriteBin (netObject msgWrapper,  char *buf, int size_buf)
{
    nd_message_out_wrapper *pwapper = (nd_message_out_wrapper*)msgWrapper ;
    nd_assert(pwapper) ;
    nd_assert(pwapper->size == sizeof(nd_message_out_wrapper));
    nd_assert(pwapper->type == NDHANDLE_USER1 + 2);
    if(0==pwapper->outStream.WriteBin((void*)buf, (size_t) size_buf ) ) {
        return 0;
    }
    pwapper->myerrno = NDERR_WRITE ;
    return -1;
    
}

int ndSendWrapMsg(netObject netObj,netObject msgObj, int flag)
{
    nd_message_out_wrapper *pwapper = (nd_message_out_wrapper*)msgObj ;
    nd_assert(pwapper) ;
    nd_assert(pwapper->size == sizeof(nd_message_out_wrapper));
    nd_assert(pwapper->type == NDHANDLE_USER1 + 2);
    
    return nd_connector_send((nd_handle)netObj,(nd_packhdr_t*) (pwapper->outStream.GetMsgAddr()), flag) ;
    
    
}

int ndCryptMsg(netObject netObj,netObject msgObj, int bEncrypt) 
{
	nd_assert(msgObj) ;
	
	nd_packetbuf_t* packet = NULL ;
	nd_handle h = (nd_handle) msgObj ;
	
	if (h->type == (NDHANDLE_USER1 + 2))  {
		nd_message_out_wrapper *pwapper = (nd_message_out_wrapper*)msgObj ;
		nd_assert(pwapper) ;
		nd_assert(pwapper->size == sizeof(nd_message_out_wrapper));
		
		packet = (nd_packetbuf_t*) pwapper->outStream.GetMsgAddr() ;
	}
	else if(h->type == (NDHANDLE_USER1 + 1)) {
		nd_message_wrapper *pwapper = (nd_message_wrapper*)msgObj ;	
		nd_assert(pwapper->size == sizeof(nd_message_wrapper));		
		packet = (nd_packetbuf_t*) pwapper->inStream->GetMsgAddr() ;
	}
	if (!packet) {
		return -1 ;
	}
	
	
	if (bEncrypt) {
		return nd_packet_encrypt((nd_handle)netObj, packet) ;
	}
	else {
		return nd_packet_encrypt((nd_handle)netObj, packet) ;
	}
	
}

int ndGetMsglen(netObject msgObj) 
{
	nd_assert(msgObj) ;
	
	nd_handle h = (nd_handle) msgObj ;	
	if (h->type == (NDHANDLE_USER1 + 2))  {
		nd_message_out_wrapper *pwapper = (nd_message_out_wrapper*)msgObj ;
		nd_assert(pwapper) ;
		nd_assert(pwapper->size == sizeof(nd_message_out_wrapper));
		
		return (int)pwapper->outStream.MsgLength() ;
	}
	else if(h->type == (NDHANDLE_USER1 + 1)) {
		nd_message_wrapper *pwapper = (nd_message_wrapper*)msgObj ;	
		nd_assert(pwapper->size == sizeof(nd_message_wrapper));		
		return (int) pwapper->inStream->MsgLength() ;
	}
	
	return 0;

}

char* ndGetMsgAddr(netObject msgObj) 
{
	nd_assert(msgObj) ;
	
	nd_handle h = (nd_handle) msgObj ;
	
	if (h->type == (NDHANDLE_USER1 + 2))  {
		nd_message_out_wrapper *pwapper = (nd_message_out_wrapper*)msgObj ;
		nd_assert(pwapper) ;
		nd_assert(pwapper->size == sizeof(nd_message_out_wrapper));
		
		return  (char*) pwapper->outStream.GetMsgAddr() ;
	}
	else if(h->type == (NDHANDLE_USER1 + 1)) {
		nd_message_wrapper *pwapper = (nd_message_wrapper*)msgObj ;	
		nd_assert(pwapper->size == sizeof(nd_message_wrapper));		
		return  (char*) pwapper->inStream->GetMsgAddr() ;
	}
	
	return 0 ;

}

int nd_checkErrorMsg(netObject nethandle,ndMsgData *msg)
{
	if (msg->maxID==ND_MAIN_ID_SYS && msg->minID ==ND_MSG_SYS_ERROR) {
		NDIStreamMsg inmsg((nd_usermsgbuf_t*) msg) ;
		NDUINT32 errcode =NDERR_BADTHREAD;
		inmsg.Read(errcode) ;
		nd_object_seterror((nd_handle)nethandle, errcode) ;
		return  errcode ;
	}
	return  0 ;

}

/////////

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

int ndBigDataSend(netObject netObj,NDUINT64 param, void *data, size_t datalen) 
{
	return BigDataAsyncSend((nd_handle)netObj, data, datalen,  param, NULL) ;
}


static NDBigDataReceiver *__bigDataRecv ;
void ndSetBigDataHandler(netObject netObj,ndBigDataHandler entry) 
{
	if (!__bigDataRecv) {
		__bigDataRecv = new NDBigDataReceiver((data_recv_callback)entry, netObj);
	}
	else {
		__bigDataRecv->SetHandler((data_recv_callback)entry) ;
	}
	
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
	nd_log_screen("recv time ");
	return 0;
}



MSG_ENTRY_INSTANCE(__data_recv_handler)
{
	int len = ND_USERMSG_DATALEN(msg) ;
	
	NDIStreamMsg inmsg(msg) ;
	if (__bigDataRecv) {
		__bigDataRecv->OnRecv(inmsg) ;
		
		if (len != NDERR_SUCCESS && len != NDERR_WOULD_BLOCK) {
			nd_logerror("error onRecv big data message ret=%d\n", len) ;
		}
	}
	return 0 ;
}


#define MSG_HANDLER_INS(_f, _maxid, _minid) \
nd_msgentry_install((nd_handle)netObj, _f,_maxid, _minid,EPL_CONNECT,#_maxid "_" #_minid )

void ndMsgfuncInit(netObject netObj)
{
    
    //MSG_HANDLER_INS(netmsg_sys_echo,ND_MAIN_ID_SYS,ND_MSG_SYS_ECHO) ;
    
    MSG_HANDLER_INS(netmsg_sys_error,ND_MAIN_ID_SYS,ND_MSG_SYS_ERROR) ;
    
    //MSG_HANDLER_INS(netmsg_sys_time,ND_MAIN_ID_SYS,ND_MSG_SYS_TIME) ;
	
	MSG_HANDLER_INS(__data_recv_handler,ND_MAIN_ID_SYS,ND_MSG_BIG_DATA_TRANSFER) ;

}


