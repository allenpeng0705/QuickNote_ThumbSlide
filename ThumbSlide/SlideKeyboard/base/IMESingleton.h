
#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

#include "q_ime.h"

@interface IMESingleton : NSObject {
    IME* iIME;
    UIInputViewController* iInputHandler;
    
    NSCharacterSet *iEnValidCharSet;
    NSCharacterSet *iEnStopCharSet;
    NSString* iLastInput;
}

@property (nonatomic,assign) IME* iIME;
@property (nonatomic, assign) UIInputViewController* iInputHandler;
@property (nonatomic, retain) NSCharacterSet *iEnValidCharSet;
@property (nonatomic, retain) NSCharacterSet *iEnStopCharSet;
@property (nonatomic, retain) NSString* iLastInput;

+(IMESingleton*)sharedInstance;
+(IMESingleton*)sharedInstanceWithInputHandler:(UIInputViewController*)aInputHandler;
+(void)destroyInstance;
-(IME*)instance;
-(UIInputViewController*)inputHandler;

@end
