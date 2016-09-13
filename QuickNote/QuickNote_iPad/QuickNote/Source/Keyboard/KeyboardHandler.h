#import <Foundation/Foundation.h>

#import "KeyboardInteractionListener.h"
#import "KeyboardOutputListener.h"
#import "KeyboardBridge.h"

#include "q_keyboard.h"

typedef struct Session Session;

@interface KeyboardHandler : NSObject {
    Keyboard* iKeyboard;
    Key* iPressedKey;
    InputSignal iInputSignal;
	SamplePoint* iSamplePoints;
    id<KeyboardInteractionListener> iKeyboardInteractionListener;
    id<KeyboardOutputListener> iKeyboardOutputListener;
    id<KeyboardBridge>iKeyboardBridge;
    NSMutableArray* iCandidates;
    NSMutableString* iExactInput;
    NSMutableString* iRecommendedWord;
    
    // For Backspace handling
    Session* iSessions;
    NSUInteger iSessionCount;
    
    BOOL iShiftModeOn;
    BOOL iFiltering;
    BOOL iTracing;
    BOOL iCurrentTracing;
    
    NSTimer *iLongPressTimer;
	NSTimer	*iRepeatPressTimer;
}

-(void)setKeyboardInteractionListener:(id<KeyboardInteractionListener>)aListener;
-(void)setKeyboardOutputListener:(id<KeyboardOutputListener>)aListener;
-(void)setKeyboardBridge:(id<KeyboardBridge>)aBridge;
-(void)setKeyboard:(Keyboard*)aKeyboard;
-(void)onFingerDownAtX:(int32)aPosX AndY:(int32)aPosY;
-(void)onFingerUpAtX:(int32)aPosX AndY:(int32)aPosY;
-(void)onFingerMoveAtX:(int32)aPosX AndY:(int32)aPosY;
-(BOOL)tracing;
-(void)clear;

-(void) sendToFilter;
-(void)finishInput;
-(void)cancelFingerOperation;
-(void)clearCandidates;
-(void)handleSelectedWord:(NSString*)tmp; 


@end
