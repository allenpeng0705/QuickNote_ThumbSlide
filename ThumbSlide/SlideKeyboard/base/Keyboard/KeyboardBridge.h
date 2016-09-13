#import <Foundation/Foundation.h>


@protocol KeyboardBridge <NSObject>

@required
    -(void)sendText:(NSString*)aText;
    -(void)sendBackspace;
    -(void)sendEnter;
    -(void)sendTab;
    -(void)changeToNextSysKeyboard;

@property(nonatomic, assign) UIInputViewController* iInputHandler;
@end
