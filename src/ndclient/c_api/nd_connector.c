//
//  nd_connector.c
//  FlashRoutes
//
//  Created by duanxiuyun on 14-6-25.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//


#include "net_conn.h"

static nd_handle s_connObj ;

#define FR_HOST "192.168.199.175"
#define FR_PORT 7828

#define ND_MAINMSG_NUMBRE 8
#define ND_SUBMSG_NUMBER 16


nd_handle ndGetConnector()
{
    return s_connObj ;
}

int ndGetConnectorStat()
{
    return nd_connector_valid((nd_netui_handle)s_connObj) ;
}

int ndInitNet()
{
    char *argv[] = {"FlashRoutes"} ;
    
    printf("test byte order is : %s \n" , nd_byte_order()?"little":"big") ;
    
    nd_arg(1, argv);
    
    nd_common_init() ;
    nd_net_init() ;
    //RSAinit_random(&__rsa_contex.randomStruct);
    nd_net_set_crypt((nd_netcrypt)nd_TEAencrypt, (nd_netcrypt)nd_TEAdecrypt, sizeof(tea_v)) ;
    return 0 ;
}

void ndDeinitNet()
{
    if (s_connObj) {
        nd_object_destroy(s_connObj,1) ;
        s_connObj = NULL ;
    }
    
    nd_net_destroy() ;
    nd_common_release() ;
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
