#import <UIKit/UIKit.h>

#import "KeyboardInteractionListener.h"
#import "TraceKeyboardHandler.h"
#import "KeyboardOutputListener.h"
#import "KeyboardBridge.h"
#import "PopupKeyView.h"
#import "WordSelectAction.h"
#include "q_keyboard.h"

@interface KeyboardView : UIView <KeyboardInteractionListener>
{
@private
    Keyboard* iKeyboard;
    UIImage* iImgOfBg;
    TraceKeyboardHandler* iHandler;
    UITouch* iCurrTouch;
    
    CGContextRef iMainGraphicCtx;
    CGLayerRef iBackgroundLayerRef;
    PopupKeyView* iPreview;
    
    Key* iLastTouchKey;
    
    float32 iWidthRatio;
    float32 iHeightRatio; 
    CGPoint iTraceArray[1000];
    int32 iTracePointIndex;
    BOOL iShiftModeOn;
}

@property (nonatomic) float32 iWidthRatio;
@property (nonatomic) float32 iHeightRatio;


-(Keyboard*)keyboard;
- (void) changeKeyboard;
-(TraceKeyboardHandler*)handler;
-(void)attachKeyboard:(Keyboard*)aKeyboard AndKeyboardOutputListener:(id<KeyboardOutputListener>)aListener;
-(NSUInteger)heightOfKeyboard;
-(NSUInteger)widthOfKeyboard;

-(void)drawKeyboard:(CGContextRef)aCtx;
-(void)drawKey:(CGContextRef)aCtx Key:(Key*)aKey WithShiftState:(BOOL)aShiftState;
-(void)showPreview:(Key*)aPressedKey WithSound:(BOOL)aFlag;
-(void)clearBackground;

@end
