
#import "CommonWordSelector.h"
#import "IMESingleton.h"
#import "CandidatesListView.h"
#import "CommonKeyboardBridge.h"

@implementation CommonWordSelector
@synthesize iInputHandler;
@synthesize iKeyboardBridge;

- (id) init
{
    self = [super init];
    if (self) {
        self.iInputHandler = [[IMESingleton sharedInstance] inputHandler];
        self.iKeyboardBridge = [[CommonKeyboardBridge alloc] init];
    }
    return self;
}

-(void) selectWord:(NSString*)aWord AtIndex:(NSUInteger)aIndex OnListView:(CandidatesListView*)aView
{
    if ((aWord == nil) || ([aWord length] == 0)) return;
    int len = [IMESingleton sharedInstance].iLastInput.length;
    if (len > 0) {
        for (int i = 0; i < len; i++) {
            [iInputHandler.textDocumentProxy deleteBackward];
        }
        [IMESingleton sharedInstance].iLastInput = @"";
    }
    [self.iKeyboardBridge sendText:aWord];
}

- (void) dealloc
{
    [iKeyboardBridge release];
    [super dealloc];
}

@end
