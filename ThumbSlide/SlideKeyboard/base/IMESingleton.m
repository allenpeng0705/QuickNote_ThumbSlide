#import "IMESingleton.h"

static IMESingleton* _instance = NULL;


@implementation IMESingleton

@synthesize iIME;
@synthesize iInputHandler;
@synthesize iEnStopCharSet;
@synthesize iEnValidCharSet;
@synthesize iLastInput;

+(IMESingleton*)sharedInstance
{
    NSAutoreleasePool *thePool = [[NSAutoreleasePool alloc] init];
    
    @synchronized(@"IME.instance") {
        if (_instance == nil) { 
            _instance = [[IMESingleton alloc] init];
            _instance.iIME = createIME();
            
            NSString* set = @"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'";
            _instance.iEnValidCharSet = (NSCharacterSet*)[NSCharacterSet characterSetWithCharactersInString:set];
            NSString* stop_set = @".?!;:";
            _instance.iEnStopCharSet = (NSCharacterSet*)[NSCharacterSet characterSetWithCharactersInString:stop_set];
            
            _instance.iInputHandler = nil;
            _instance.iLastInput = nil;
        }
    }
    
    [thePool release];
    
    return(_instance);
}

+(IMESingleton*)sharedInstanceWithInputHandler:(UIInputViewController*)aInputHandler
{
    _instance = [IMESingleton sharedInstance];
    _instance.iInputHandler = aInputHandler;
    return(_instance);
}

+(void)destroyInstance
{
    if (_instance != nil) {
        destroyIME(_instance.iIME);
        [_instance.iEnValidCharSet release];
        [_instance.iEnStopCharSet release];
        [_instance.iLastInput release];
        [_instance release];
        _instance = nil;
    }
}

-(IME*)instance
{
    return iIME;
}

-(UIInputViewController*)inputHandler
{
    return iInputHandler;
}

@end
