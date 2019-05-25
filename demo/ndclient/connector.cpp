//
//  main.cpp
//  httpRobort
//
//  Created by duanxiuyun on 14-8-4.
//  Copyright (c) 2014Äê duanxiuyun. All rights reserved.
//


#include "ndcli/nd_iconn.h"
#include "nd_msg.h"

#define  MAX_CONNECTOR_NUM 1024
int volatile __exit;

char *__host;
int __port ;

NDIConn *__conn_buf[MAX_CONNECTOR_NUM] ;
int __real_conn_num = 1;

int ParseArg(int argc, char *argv[])
{
    if (argc < 4) {
    ERROR_EXIT:
        printf("USAGE: %s host port connector_number \n" , argv[0]) ;
                      
        exit(1);
    }
    
    __host = argv[1] ;
    __port = atoi(argv[2]) ;
    __real_conn_num = atoi(argv[3]) ;
    
    return 0;
}


int msg_br_handler(NDIConn* pconn, nd_usermsgbuf_t *msg )
{
    NDIStreamMsg inmsg(msg) ;
    NDUINT8 buf[1024] ;
    buf[0] = 0 ;
    inmsg.Read(buf,sizeof(buf)) ;
    printf("recv (%d,%d) : %s \n",inmsg.MsgMaxid(), inmsg.MsgMinid(), buf ) ;
    return 0;
}


int open_net()
{
    for (int i=0; i<__real_conn_num; i++) {
        __conn_buf[i] = CreateConnectorObj(NULL);
        if (__conn_buf[i]) {
            //__conn_buf[i]->InstallMsgFunc(msg_br_handler, ND_MAIN_ID_SYS, ND_MSG_SYS_BROADCAST);
            
            __conn_buf[i]->SetUserData((void*) 0) ;
            __conn_buf[i]->Open(__host, __port, "tcp-connector", NULL) ;
            printf("create connect success %d \n", i);
            
        }
    }
    return 0;
    
}

int sent_echo()
{
    NDOStreamMsg omsg(ND_MAIN_ID_SYS, ND_MSG_SYS_BROADCAST) ;
    char buf[1024] ;
    
    for (int i=0; i<__real_conn_num; i++) {
        if (!__conn_buf[i]->CheckValid()) {
            continue ;
        }
        else {
            printf("connect invalid %d \n", i) ;
        }

        size_t udata = (size_t) __conn_buf[i]->GetUserData() ;
        
        snprintf(buf, sizeof(buf), "[%d] send %zu : hello world !", i, udata ) ;
        
        ++udata ;
        __conn_buf[i]->SetUserData((void*) udata);
        omsg.Write((NDUINT8*)buf) ;
        
        __conn_buf[i]->SendMsg(omsg) ;
    }
    return 0;

}

int updateConnect()
{
    sent_echo();
    
    for (int i=0; i<__real_conn_num; i++) {
        if (__conn_buf[i]->CheckValid()) {
            __conn_buf[i]->Update(30) ;
        }
        else {
            __conn_buf[i]->Open(__host, __port, "tcp-connector", NULL) ;
        }
    }
    return 0;
}

int close_net()
{
    for (int i=0; i<__real_conn_num; i++) {
        __conn_buf[i]->Close(0) ;
        DestroyConnectorObj(__conn_buf[i]) ;
        __conn_buf[i] = 0 ;

    }
    return 0;

}

int wait_exit()
{
    int ch;
    int index = 0 ;
    while( 0==__exit ){
        if(kbhit()) {
            ch = getch() ;
            if(ND_ESC==ch){
                printf_dbg("you are hit ESC, program eixt\n") ;
                __exit = 1 ;
                break ;
            }
        }
        else {
            updateConnect() ;
            //nd_sleep(500) ;
        }
        index++ ;
    }
    return 0;
}


int main(int argc , char *argv[])
{
    
    ParseArg( argc, argv) ;
    
    if(InitNet()){
        ndprintf(_NDT("connect error :%s!"), nd_last_error()) ;
        //getch();
        exit(1);
    }
    
    if(-1==open_net()  ) {
        ndprintf(_NDT("open net error \n press any key to exit")) ;
        //getch();
        exit(1);
    }
    
    wait_exit() ;
    
    close_net() ;
    
    DeinitNet();
    
    fprintf(stderr, "client test exit!\n") ;
    //getch();
    return 0;
}
