//
//  ViewController.m
//  iosDemo
//
//  Created by duanxiuyun on 14-8-23.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//

#import "ViewController.h"

@interface ViewController (){
    bool inConnect;
}

@end


@implementation ViewController

@synthesize connectButton;
@synthesize tipText;

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}



- (IBAction)connectButtonHit:(id)sender {
    if(padPopover && padPopover.popoverVisible)
        [padPopover dismissPopoverAnimated:YES];
    [self showColorPicker];
}

@end
