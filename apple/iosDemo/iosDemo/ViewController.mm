//
//  ViewController.m
//  iosDemo
//
//  Created by duanxiuyun on 14-8-23.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//

#import "ViewController.h"
//#include "ndcli/nd_iconn.h"
#include "ndcli/nd_api_c.h"
#include "netmsgHandler.h"

@interface ViewController (){
    bool inConnect;
    //NDIConn *conn ;
    netObject hConn ;
    NSTimer *myTimer ;
}

@end


@implementation ViewController

@synthesize connectButton;
@synthesize tipText;

- (void)viewDidLoad {
    [super viewDidLoad];
    InitNet() ;
    
    //conn = CreateConnectorObj("tcp-connector") ;
    
    myTimer =  [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(timerTick:) userInfo:nil repeats:YES];
    // Do any additional setup after loading the view, typically from a nib.
}
- (void)viewDidUnload {
    [myTimer invalidate];
    [super viewDidUnload];
    DeinitNet() ;
    // Do any additional setup after loading the view, typically from a nib.
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


- (IBAction)timerTick:(id)sender {
    /*if (inConnect && hConn) {
        timeTick(conn) ;
        
        sendTest(conn) ;
    }*/
    if (hConn) {
        ndUpdateConnect(hConn, 0);
        ndSentTest(hConn);
    }
}


- (IBAction)connectButtonHit:(id)sender {
    if (hConn) {
        ndClostConnect(hConn) ;
        //conn->Close() ;
        //inConnect = false ;
        hConn = NULL ;
    }
    else {
        hConn = ndOpenConnect("192.168.0.107", 6000) ;
        /*if(!conn) {
            conn =  CreateConnectorObj("tcp-connector") ;
            if (!conn) {
                return ;
            }
        }
        if(0==conn->Open("192.168.199.175", 7828,"tcp-connector",NULL) ){
            init_messageHandler(conn) ;
            inConnect = true ;
        }*/
    }
}

@end
