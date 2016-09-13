

#import <Foundation/Foundation.h>

#include "q_ime.h"


@interface IMESingleton : NSObject {
    IME* iIME;
}

@property (nonatomic,assign) IME* iIME;

+(IMESingleton*)sharedInstance;
+(void)destroyInstance;
-(IME*)instance;

@end
