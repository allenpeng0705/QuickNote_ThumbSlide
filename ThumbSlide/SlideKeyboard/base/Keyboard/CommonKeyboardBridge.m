
#import "CommonKeyboardBridge.h"
#import "IMESingleton.h"

@implementation CommonKeyboardBridge
@synthesize iInputHandler;

- (id) init
{
    self = [super init];
    if (self) {
        self.iInputHandler = [[IMESingleton sharedInstance] inputHandler];
    }
    return self;
}

-(void)sendText:(NSString*)aText
{
    if (aText.length == 0) return;
    NSString* sendingText = @"";
    NSString* textBeforeCaret = [iInputHandler.textDocumentProxy documentContextBeforeInput];
    NSString* trimTextBeforeCaret = [textBeforeCaret stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
//    NSString* textAfterCaret = [iInputHandler.textDocumentProxy documentContextAfterInput];
    NSString* firstCharater = [aText substringToIndex:1];
    NSString* left = [aText substringFromIndex:1];
    NSString* upper = [firstCharater uppercaseString];
    if (aText.length > 1) {
        if (trimTextBeforeCaret.length == 0) {
            sendingText = [upper stringByAppendingString:left];
        } else {
            unichar last_char = [trimTextBeforeCaret characterAtIndex:trimTextBeforeCaret.length - 1];
            if ([[IMESingleton sharedInstance].iEnStopCharSet characterIsMember:last_char] == YES) {
                sendingText = [NSString stringWithFormat:@" %@%@", upper, left];
            } else {
                sendingText = [NSString stringWithFormat:@" %@", aText];
            }
        }
        [IMESingleton sharedInstance].iLastInput = sendingText;
    } else {
        if (trimTextBeforeCaret.length == 0) {
            sendingText = upper;
        } else {
            unichar last_char = [trimTextBeforeCaret characterAtIndex:trimTextBeforeCaret.length - 1];
            if ([[IMESingleton sharedInstance].iEnStopCharSet characterIsMember:last_char] == YES) {
                sendingText = [NSString stringWithFormat:@" %@", upper];
            } else {
                if ([IMESingleton sharedInstance].iLastInput.length > 1) {
                    unichar character = [aText characterAtIndex:0];
                    if ([[IMESingleton sharedInstance].iEnValidCharSet characterIsMember:character] == YES) {
                         sendingText = [NSString stringWithFormat:@" %@", aText];
                    } else {
                         sendingText = aText;
                    }
                } else {
                    sendingText = aText;
                }
            }
            
            if ((([sendingText isEqualToString:@"a"]) || ([sendingText isEqualToString:@"i"])) && ([IMESingleton sharedInstance].iLastInput.length > 1)) {
                sendingText = [NSString stringWithFormat:@" %@", upper];
            }
            [IMESingleton sharedInstance].iLastInput = @"";
        }

    }
    
    [iInputHandler.textDocumentProxy insertText:sendingText];
}

-(void)sendBackspace
{
    int len = [IMESingleton sharedInstance].iLastInput.length;
    if (len > 0) {
        for (int i = 0; i < len; i++) {
           [iInputHandler.textDocumentProxy deleteBackward];
        }
        [IMESingleton sharedInstance].iLastInput = @"";
    } else {
        [iInputHandler.textDocumentProxy deleteBackward];
    }
}

-(void)sendEnter
{
    [iInputHandler.textDocumentProxy insertText:@"\n"];
}

-(void)sendTab
{
    [iInputHandler.textDocumentProxy insertText:@"\t"];
}

-(void)changeToNextSysKeyboard
{
    [iInputHandler advanceToNextInputMode];
}


@end
