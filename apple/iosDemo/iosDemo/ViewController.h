//
//  ViewController.h
//  iosDemo
//
//  Created by duanxiuyun on 14-8-23.
//  Copyright (c) 2014�� duanxiuyun. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ViewController : UIViewController


@property (strong, nonatomic) IBOutlet UIButton *connectButton;
@property (strong, nonatomic) IBOutlet UITextField *tipText;


- (IBAction)connectButtonHit:(id)sender;

@end

