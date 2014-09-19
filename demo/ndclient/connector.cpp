//
//  main.cpp
//  httpRobort
//
//  Created by duanxiuyun on 14-8-4.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//


#include "ndcli/nd_iconn.h"
#include "nd_msg.h"

int volatile __exit;

char *__host;
int __port ;

int ParseArg(int argc, char *argv[]) {
    if (argc < 3) {
    ERROR_EXIT:
        printf("USAGE: %s host port \n" , argv[0]) ;
                      
        return -1 ;
    }
    
    __host = argv[1] ;
    __port = atoi(argv[2]) ;
    
    return 0;
}


NDIConn *pconnector ;

int open_net()
{
    pconnector = CreateConnectorObj(NULL);
    if (pconnector) {
        return pconnector->Open(__host, __port, "tcp-connector", NULL) ;
    }
    return -1;
}

int sent_echo()
{
    NDOStreamMsg omsg(ND_MAIN_ID_SYS, ND_MSG_SYS_ECHO) ;
    omsg.Write((NDUINT8*)"hello world!") ;
    
    if(pconnector) {
        return pconnector->SendMsg(omsg);
    }
    return -1 ;
}

int updateConnect()
{
    if (pconnector) {
        sent_echo();
        pconnector->Update(100) ;
    }
    return 0;
}

int close_net()
{
    if (pconnector) {
        pconnector->Close(0) ;
        DestroyConnectorObj(pconnector) ;
        pconnector = 0 ;
    }
    return 0 ;
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
            nd_sleep(500) ;
        }
        index++ ;
    }
    return 0;
}


int main(int argc , char *argv[])
{	
    if(InitNet()){
        ndprintf(_NDT("connect error :%s!"), nd_last_error()) ;
        getch();
        exit(1);
    }
    
    if(-1==open_net()  ) {
        ndprintf(_NDT("open net error \n press any key to exit")) ;
        getch();
        exit(1);
    }
    
    wait_exit() ;
    
    close_net() ;
    
    DeinitNet();
    
    fprintf(stderr, "client test exit!\n") ;
    getch();
    return 0;
}
