#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include "base.h"
#import "KeyboardOutputListener.h"
#import "KeyboardInteractionListener.h"
#import "KeyboardBridge.h"
#include "q_keyboard.h"

@protocol KeyboardHandler <NSObject>

@required
    -(void)onFingerDownAtX:(int32)aPosX AndY:(int32)aPosY;
    -(void)onFingerUpAtX:(int32)aPosX AndY:(int32)aPosY;
    -(void)onFingerMoveAtX:(int32)aPosX AndY:(int32)aPosY;
    -(void)cancelFingerOperation;

@property (nonatomic, assign) id<KeyboardInteractionListener> iKeyboardInteractionListener;
@property (nonatomic, assign) id<KeyboardOutputListener> iKeyboardOutputListener;
@property (nonatomic, assign) id<KeyboardBridge>iKeyboardBridge;
@property (nonatomic, assign) Keyboard* iKeyboard;

@end

