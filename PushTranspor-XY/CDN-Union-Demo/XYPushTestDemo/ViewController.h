//
//  ViewController.h
//  XYPushTestDemo
//
//  Created by hongduoxing on 16/12/1.
//  Copyright © 2016年 hongduoxing. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ViewController : UIViewController<UIPickerViewDelegate, UIPickerViewDataSource>

@property(nonatomic, assign) IBOutlet UITextField *textField;

@property(nonatomic, assign) IBOutlet UIPickerView *pickView;

@end

