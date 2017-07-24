//
//  nd_connector.c
//  FlashRoutes
//
//  Created by duanxiuyun on 14-6-25.
//  Copyright (c) 2014 duanxiuyun. All rights reserved.
//


#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"
#include "nd_msg.h"
#include "ndcli/nd_api_c.h"

static int __bInit = 0 ;

int _translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle) ;

static ndNetFunc __terminate_entry ;

void tryto_terminate(netObject netObj) ;

int ndInitNet()
{
    if (__bInit) {
        return 0;
    }
    const char *argv[] = {"ndNetCAPI"} ;
    
    printf("test byte order is : %s \n" , nd_byte_order()?"little":"big") ;
    
    nd_arg(1, argv);
    
    nd_common_init() ;
    nd_net_init() ;
    //RSAinit_random(&__rsa_contex.randomStruct);
    nd_net_set_crypt((nd_netcrypt)nd_TEAencrypt, (nd_netcrypt)nd_TEAdecrypt, sizeof(tea_v)) ;
    __bInit =1 ;
	__terminate_entry = 0 ;
    return 0 ;
}

void ndDeinitNet()
{
    if (__bInit==0) {
        return ;
    }
    nd_net_destroy() ;
    nd_common_release() ;
    __bInit = 0;
}


netObject ndOpenConnect(const char *host, int port)
{
    nd_handle handle = 0 ;
    
    ndInitNet() ;
    
    handle = nd_object_create("tcp-connector"  ) ;
    
    if(!handle){
        nd_logerror((char*)"connect error :%s!" AND nd_last_error()) ;
        return NULL;
    }
    if(-1==nd_msgtable_open(handle, 16) ) {
        nd_object_destroy(handle, 0) ;
        return NULL ;
    }
	
    //open
    if(-1==nd_connector_open( handle, (char*)host,  port,NULL ) ){
        nd_logerror("connect error :%s!" AND nd_last_error()) ;
        nd_object_destroy(handle,1) ;
        return NULL;
    }
    
    nd_hook_packet((netObject) handle,(net_msg_entry )_translate_message);
    ndMsgfuncInit( (netObject) handle ) ;

    return (netObject) handle;
}

void ndClostConnect(netObject netObj)
{
    if(netObj) {
        nd_connector_close((nd_handle)netObj, 0) ;
    }
    
}
ndNetFunc ndSetTerminateFunc(ndNetFunc func) 
{
	ndNetFunc ret =	__terminate_entry ;
	__terminate_entry = func;
	return ret ;
}

int ndAddOnCloseCallback(netObject handle,nd_conn_close_entry callback, void *param) 
{
	return nd_object_add_destroy_cb((nd_handle)handle,(nd_object_destroy_callback)callback, param, 1) ; 
	
}
int ndAddOnDestroyCallback(netObject handle,nd_conn_close_entry callback, void *param) 
{
	return nd_object_add_destroy_cb((nd_handle)handle,(nd_object_destroy_callback)callback, param, 0) ;	
}
int ndDelDestroyCallback(netObject handle,nd_conn_close_entry callback, void *param) 
{	
	return nd_object_del_destroy_cb((nd_handle) handle,(nd_object_destroy_callback)callback, param) ;
}


int ndGetLastError(netObject netObj) 
{
	return nd_object_lasterror((nd_handle)netObj) ;
}

const char *ndGetLastErrorDesc(netObject netObj) 
{
	return nd_object_errordesc((nd_handle)netObj) ;
}


int ndSendData(netObject netObj, char *data, int dataLen, int flag)
{
    int ret = 0 ;
    nd_packhdr_t *msghdr = (nd_packhdr_t *) data ;
	
	ND_USERMSG_SYS_RESERVED(msghdr) = 0 ;
    packet_ntoh(&msghdr) ;
    ret = nd_connector_send((nd_handle) netObj, (nd_packhdr_t *)data, flag) ;
    packet_hton(&msghdr) ;
	if (-1 == ret && NDERR_WOULD_BLOCK != nd_object_lasterror((nd_handle)netObj)) {
		tryto_terminate(netObj) ;
	}
    return ret ;
}

int ndSendRaw(netObject netObj,char *data, int size)
{
    int ret = nd_connector_raw_write((nd_handle) netObj , data, (size_t) size) ;
	if (-1 == ret && NDERR_WOULD_BLOCK != nd_object_lasterror((nd_handle)netObj)) {
		tryto_terminate(netObj) ;
	}
	return ret ;
}


int ndSendMsg(netObject netObj,struct ndMsgData *data, int flag)
{
	int ret ;
    data->reserved = 0 ;
    ret = nd_connector_send((nd_handle) netObj, (nd_packhdr_t *)data, flag) ;
	if (-1 == ret && NDERR_WOULD_BLOCK != nd_object_lasterror((nd_handle)netObj)) {
		tryto_terminate(netObj) ;
	}
	return ret ;
}

int ndUpdateConnect(netObject netObj, int timeOutMS)
{
	int ret ;
    if (!nd_connector_valid((nd_netui_handle)netObj)) {
        return -1;
    }
    
    ret = nd_connector_update((nd_handle)netObj,timeOutMS);
	
	if (-1==ret ) {
		if (NDERR_WOULD_BLOCK != nd_object_lasterror((nd_handle)netObj) &&
			NDERR_TIMEOUT != nd_object_lasterror((nd_handle) netObj) ) {
			tryto_terminate(netObj) ;
		}		
	}
	return ret ;

    
}
int ndNetFuncInstall(netObject netObj,ndNetFunc func, int maxID, int minID,const char *name)
{
    if(netObj)
        return nd_msgentry_install((nd_handle)netObj, (nd_usermsg_func)func,  maxID,  minID,EPL_CONNECT,name) ;
    return -1;
    
}


int ndSetDftMsgHandler(netObject netObj,ndNetFunc dftFunc)
{
    if(netObj)
        return nd_msgentry_def_handler((nd_handle)netObj, (nd_usermsg_func)dftFunc) ;
    return -1;
}

int ndWaitMsg(netObject netObj, char *buf, int bufsize, int timeOutMS)
{
    int ret = nd_connector_waitmsg(netObj, (nd_packetbuf_t *)buf,timeOutMS);
	
	if (-1==ret ) {
		if (NDERR_WOULD_BLOCK != nd_object_lasterror((nd_handle)netObj) &&
			NDERR_TIMEOUT != nd_object_lasterror((nd_handle) netObj) &&
			NDERR_INVALID_INPUT != nd_object_lasterror((nd_handle) netObj) ) {
			tryto_terminate(netObj) ;
		}		
	}
	else if (ret > bufsize) {
		nd_object_seterror(netObj,NDERR_LIMITED);
		return -1;
	}

	return ret ;
}


int ndSendAndWaitMessage(nd_handle nethandle, nd_usermsgbuf_t *sendBuf, nd_usermsgbuf_t* recvBuf, ndmsgid_t waitMaxid, ndmsgid_t waitMinid, int sendFlag, int timeout)
{
	if (nd_connector_send(nethandle, (nd_packhdr_t*)sendBuf, sendFlag) <= 0) {
		nd_object_seterror(nethandle, NDERR_WRITE);
		nd_logerror("send data error: NDERR_WRITE\n");
		return -1;
	}
	ndtime_t start_tm = nd_time();
RE_RECV:

	if (-1 == nd_connector_waitmsg(nethandle, (nd_packetbuf_t *)recvBuf, timeout)) {
		//nd_object_seterror(nethandle, NDERR_TIMEOUT);
		nd_logerror("wait message timeout\n");
		return -1;
	}
	else if (recvBuf->msg_hdr.packet_hdr.ndsys_msg){
		if (-1 == nd_net_sysmsg_hander((nd_netui_handle)nethandle, (nd_sysresv_pack_t *)recvBuf)){
			nd_logerror("receive system mesaage and handler error \n");
			return -1;
		}
	}
	else if (nd_checkErrorMsg(nethandle, (struct ndMsgData*)recvBuf)) {
		nd_logerror("receive error message \n");
		return -1;
	}
	else if (ND_USERMSG_MAXID(recvBuf) != waitMaxid || ND_USERMSG_MINID(recvBuf) != waitMinid) {
		if ((nd_time() - start_tm) >= timeout) {
			nd_object_seterror(nethandle, NDERR_TIMEOUT);
			nd_logerror("wait message(%d,%d) timeout\n", waitMaxid, waitMinid);
			return -1;
		}
		if (((nd_netui_handle)nethandle)->msg_handle) {
			//int ret = nd_translate_message(nethandle, (nd_packhdr_t*)recvBuf, NULL);
			int ret = _packet_handler((nd_netui_handle)nethandle, &recvBuf->msg_hdr.packet_hdr, NULL);
			if (ret == -1){
				nd_logerror("wait message(%d,%d) error ,recvd(%d,%d)\n", waitMaxid, waitMinid, ND_USERMSG_MAXID(recvBuf), ND_USERMSG_MINID(recvBuf));
				return ret;
			}
		}
		goto RE_RECV;
	}
	return 0;
}

void tryto_terminate(netObject netObj) 
{
	if (__terminate_entry) {		
		nd_netui_handle handle = (nd_netui_handle) netObj ;
		if (handle->status != ETS_DEAD && handle->status != ETS_CLOSED) {
			__terminate_entry(netObj, NULL, 0) ;
		}		
	}
}


int _translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle)
{
	ENTER_FUNC()	
	
	int ret = 0 ;
	int data_len = nd_pack_len(msg);
	
	nd_usermsghdr_t *usermsg = (nd_usermsghdr_t *) (msg) ;	
	nd_usermsg_func func ;
	
	nd_netmsg_ntoh(usermsg) ; 
	func = nd_msgentry_get_func((nd_handle)connect_handle, usermsg->maxid,  usermsg->minid);
	
	func = func ? func : nd_msgentry_get_def_func((nd_handle)connect_handle);
	
	if (func){
		ret = func((nd_handle)connect_handle, (nd_usermsgbuf_t *) msg, (nd_handle)ND_USERMSG_LEN(msg));
	}
	else {
		nd_logmsg("received message (%d,%d) UNHANDLED\n" AND usermsg->maxid AND usermsg->minid) ;		
	}
	
	LEAVE_FUNC();
	return  ret==-1? -1:data_len ;
}

/*
struct msg_entry_node
{
    NDUINT32			level;
    nd_usermsg_func		entry ;
};

struct sub_msgentry
{
    struct msg_entry_node   msg_buf[SUB_MSG_NUM] ;
};

struct msgentry_root
{
	ND_OBJ_BASE;
	NDUINT16	main_num ;			//∞¸∫¨∂‡…Ÿ∏ˆœ˚œ¢¿‡±
	NDUINT16	msgid_base ;		//÷˜œ˚œ¢∫≈∆ ºµÿ÷∑
	NDUINT32	msg_node_size ;
    nd_usermsg_func		def_entry ;
    struct sub_msgentry sub_buf[ND_MAIN_MSG_CAPACITY] ;
};



int _translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle)
{
    ENTER_FUNC()
    int ret = 0 ;
    int data_len = nd_pack_len(msg);
    struct msgentry_root *root_entry= NULL;
    ndNetFunc  func = NULL ;
    nd_usermsghdr_t *usermsg =  (nd_usermsghdr_t *) msg ;
    
    nd_assert(msg) ;
    nd_assert(connect_handle) ;
    
    root_entry =(struct msgentry_root *)nd_get_msg_hadle(connect_handle);
    
    if(root_entry) {
        ndmsgid_t main_index , minid;
        nd_netmsg_ntoh(usermsg) ;
        main_index = usermsg->maxid - root_entry->msgid_base;
        minid = usermsg->minid ;
        if(main_index >= root_entry->main_num || minid>=SUB_MSG_NUM){
            nd_object_seterror((nd_handle)connect_handle, NDERR_INVALID_INPUT) ;
            LEAVE_FUNC();
            return -1 ;
        }
        
        func = (ndNetFunc) root_entry->sub_buf[main_index].msg_buf[minid].entry ;
        if(!func && root_entry->def_entry){
            func =(ndNetFunc) root_entry->def_entry ;
        }
        
        if(func) {
            ret = func((netObject) connect_handle, (char*)msg, (int)ND_USERMSG_LEN(msg) );
        }
    }
    
    if(-1==ret) {
        nd_object_seterror((nd_handle)connect_handle, NDERR_USER) ;
        LEAVE_FUNC();
        return -1 ;
    }
    LEAVE_FUNC();
    return  data_len ;
}
*/
/*
static nd_handle s_connObj ;

#define FR_HOST "192.168.199.175"
#define FR_PORT 7828

#define ND_MAINMSG_NUMBRE 8
#define ND_SUBMSG_NUMBER 16


netObject ndGetConnector()
{
    return (netObject)s_connObj ;
}

int ndGetConnectorStat()
{
    return nd_connector_valid((nd_netui_handle)s_connObj) ;
}

int ndOpenNet()
{
    //create
    if (!s_connObj) {
        s_connObj = nd_object_create("tcp-connector"  ) ;
        
        if(!s_connObj){
            nd_logerror((char*)"connect error :%s!" AND nd_last_error()) ;
            return -1;
        }
        if(-1==nd_msgtable_open(s_connObj, ND_MAINMSG_NUMBRE) ) {
            nd_object_destroy(s_connObj, 0) ;
            return -1 ;
        }
        
        //open
        if(-1==nd_connector_open( s_connObj, FR_HOST,  FR_PORT,NULL ) ){
            nd_logerror("connect error :%s!" AND nd_last_error()) ;
            nd_object_destroy(s_connObj,1) ;
            s_connObj = NULL ;
            return -1;
        }
        init_messageHandler() ;

    }
    else if(!nd_connector_valid((nd_netui_handle)s_connObj)) {
        if(-1==nd_connector_open( s_connObj, FR_HOST,  FR_PORT,NULL ) ){
            nd_logerror("connect error :%s!" AND nd_last_error()) ;
            nd_object_destroy(s_connObj,1) ;
            s_connObj = NULL ;
            return -1;
        }
        init_messageHandler();
    }
    
    return 0 ;

}
void ndCloseNet()
{
    if(s_connObj) {
        nd_connector_close(s_connObj, 0) ;
    }
}

int ndUpdateNet()
{
    if (!nd_connector_valid((nd_netui_handle)s_connObj)) {
        return -1;
    }
    sendTest() ;
    //frTestNet();    //for test

    nd_connector_update(s_connObj,0);
    return 0;
}
*/