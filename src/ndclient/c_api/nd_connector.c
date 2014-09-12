//
//  nd_connector.c
//  FlashRoutes
//
//  Created by duanxiuyun on 14-6-25.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//


#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"
#include "nd_msg.h"
#include "ndcli/nd_api_c.h"

static int __bInit = 0 ;

int _translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle) ;

int ndInitNet()
{
    if (__bInit) {
        return 0;
    }
    const char *argv[] = {"FlashRoutes"} ;
    
    printf("test byte order is : %s \n" , nd_byte_order()?"little":"big") ;
    
    nd_arg(1, argv);
    
    nd_common_init() ;
    nd_net_init() ;
    //RSAinit_random(&__rsa_contex.randomStruct);
    nd_net_set_crypt((nd_netcrypt)nd_TEAencrypt, (nd_netcrypt)nd_TEAdecrypt, sizeof(tea_v)) ;
    __bInit =1 ;
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


int ndSendData(netObject netObj, char *data, int dataLen, int flag)
{
    int ret = 0 ;
    nd_packhdr_t *msghdr = (nd_packhdr_t *) data ;
    packet_ntoh(&msghdr) ;
    ret = nd_connector_send((nd_handle) netObj, (nd_packhdr_t *)data, flag) ;
    packet_hton(&msghdr) ;
    return ret ;
}

int ndSendRaw(netObject netObj,char *data, int size)
{
    return  nd_connector_raw_write((nd_handle) netObj , data, (size_t) size) ;
}


int ndSendMsg(netObject netObj,struct ndMsgData *data, int flag)
{
    data->reserved = 0 ;
    return nd_connector_send((nd_handle) netObj, (nd_packhdr_t *)data, flag) ;

}


int ndUpdateConnect(netObject netObj, int timeOutMS)
{
    if (!nd_connector_valid((nd_netui_handle)netObj)) {
        return -1;
    }
    
    nd_connector_update((nd_handle)netObj,timeOutMS);
    return 0;

    
}
int ndNetFuncInstall(netObject netObj,ndNetFunc func, int maxID, int minID)
{
    if(netObj)
        return nd_msgentry_install((nd_handle)netObj, (nd_usermsg_func)func,  maxID,  minID,EPL_CONNECT) ;
    return -1;
    
}


int ndSetDftMsgHandler(netObject netObj,ndNetFunc dftFunc)
{
    if(netObj)
        return nd_msgentry_def_handler((nd_netui_handle)netObj, (nd_usermsg_func)dftFunc) ;
    return -1;
}

int ndWaitMsg(netObject netObj, char *buf, int timeOutMS)
{
    return nd_connector_waitmsg(netObj, (nd_packetbuf_t *)buf,timeOutMS);
}



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
    int	main_num ;
    int msgid_base ;
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