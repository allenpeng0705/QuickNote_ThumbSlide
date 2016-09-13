#import "IMESingleton.h"

static IMESingleton* _instance = NULL;


@implementation IMESingleton

@synthesize iIME;

+(IMESingleton*)sharedInstance
{
    NSAutoreleasePool *thePool = [[NSAutoreleasePool alloc] init];
    
    @synchronized(@"IME.instance") {
        if (_instance == nil) { 
            _instance = [[IMESingleton alloc] init];
            _instance.iIME = createIME();
        }
    }
    
    [thePool release];
    
    return(_instance);
}

+(void)destroyInstance
{
    if (_instance != nil) {
        destroyIME(_instance.iIME);
        [_instance release];
        _instance = nil;
    }
    
}

-(IME*)instance
{
    return iIME;
}


@end
