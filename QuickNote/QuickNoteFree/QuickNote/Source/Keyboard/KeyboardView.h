#import <UIKit/UIKit.h>

#import "KeyboardInteractionListener.h"
#import "KeyboardHandler.h"
#import "NoteContentView.h"
#import "KeyboardOutputListener.h"
#import "KeyboardBridge.h"
#import "PopupKeyView.h"

#include "q_keyboard.h"

@interface KeyboardView : UIView <KeyboardInteractionListener, KeyboardBridge>
{
@private
    Keyboard* iKeyboard;
    UIImage* iImgOfBg;
    KeyboardHandler* iHandler;
    NoteContentView* iHostEditor;
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
-(KeyboardHandler*)handler;
-(void)attachKeyboard:(Keyboard*)aKeyboard AndKeyboardOutputListener:(id<KeyboardOutputListener>)aListener;
-(NSUInteger)heightOfKeyboard;
-(NSUInteger)widthOfKeyboard;
-(void)setHostEditor:(NoteContentView*)aEditor;
-(NoteContentView*)hostEditor;

-(void)drawKeyboard:(CGContextRef)aCtx;
-(void)drawKey:(CGContextRef)aCtx Key:(Key*)aKey WithShiftState:(BOOL)aShiftState;
-(void)showPreview:(Key*)aPressedKey WithSound:(BOOL)aFlag;
-(void)clear;
-(void)clearBackground;

@end
