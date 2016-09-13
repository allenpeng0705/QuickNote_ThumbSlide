#import "KeyboardHandler.h"
#import "IMESingleton.h"

#include "q_malloc.h"
#include "utf8_string.h"

struct Session {
    NSUInteger iStartIndex;
    NSUInteger iEndIndex;
};

@interface KeyboardHandler (private)

-(void)handleFunctionKeyDown:(Key*) aKey;
-(void)handleFunctionKeyUp:(Key*) aKey;
-(void) sendToFilter;

@end


@implementation KeyboardHandler

-(id)init
{
    if ((self = [super init])) {
        iSamplePoints = (SamplePoint*)q_malloc(sizeof(SamplePoint)*MAX_WORD_LENGTH*2);
        iInputSignal.iSamplePoints = iSamplePoints;
        iInputSignal.iNumOfSamplePoints = 0;
        iFiltering = NO;
        iTracing = NO;
        iCurrentTracing = NO;
        iShiftModeOn = NO;
        iCandidates = [[NSMutableArray alloc] initWithCapacity:MAX_COUNT_CANDIDATES + 1];
        iExactInput = [[NSMutableString alloc] init];
        iRecommendedWord = [[NSMutableString alloc] init];
        
        iPressedKey = NULL;
        
        iSessions = (Session*)q_malloc(sizeof(Session)*MAX_WORD_LENGTH*2);
        iSessionCount = 0;
        
        iLongPressTimer = nil;
        iRepeatPressTimer = nil;
    }
    return self;
}

-(void)clear
{
    iFiltering = NO;
    iInputSignal.iNumOfSamplePoints = 0;
    [iExactInput setString:@""];
    iSessionCount = 0;    
}

-(void)dealloc
{
    if (iSamplePoints != NULL) {
        q_free(iSamplePoints);
    }
    [iCandidates removeAllObjects];
    [iCandidates release];
    [iExactInput release];
    
    if ([iLongPressTimer isValid]) [iLongPressTimer invalidate];
    if ([iRepeatPressTimer isValid]) [iRepeatPressTimer invalidate];
    [super dealloc];
}

- (void)longPressTimerFireMethod:(NSTimer*)theTimer 
{
    if ([iLongPressTimer isValid]) [iLongPressTimer invalidate];
    iLongPressTimer = nil;	
    if (iPressedKey == NULL) return;
        
    if (IDOfKey(iPressedKey) == KEY_ID_BACKSPACE) {
        [iKeyboardBridge sendBackspace];
        iRepeatPressTimer = [NSTimer scheduledTimerWithTimeInterval:0.3 target:self selector:@selector(repeatPressTimerFireMethod:) userInfo:nil repeats:YES];
    }
}


- (void)repeatPressTimerFireMethod:(NSTimer*)theTimer 
{
    if (iPressedKey == NULL) return;
    
    if (IDOfKey(iPressedKey) == KEY_ID_BACKSPACE) {
        [self clear];
        [iKeyboardBridge selectWordAroundCursor];
        [iKeyboardBridge sendBackspace];
    } else {
        if ([iRepeatPressTimer isValid]) [iRepeatPressTimer invalidate];
        iRepeatPressTimer = nil;
    }
}

-(void)setKeyboardInteractionListener:(id<KeyboardInteractionListener>)aListener
{
    iKeyboardInteractionListener = aListener;
}

-(void)setKeyboardBridge:(id<KeyboardBridge>)aBridge
{
    iKeyboardBridge = aBridge;
}

-(void)setKeyboardOutputListener:(id<KeyboardOutputListener>)aListener
{
    iKeyboardOutputListener = aListener;
}

-(void)setKeyboard:(Keyboard*)aKeyboard
{
    iKeyboard = aKeyboard;
}

-(void)onFingerDownAtX:(int32)aPosX AndY:(int32)aPosY
{
    if (iKeyboard == NULL) return;
    Key* key = KeyAtPosition(iKeyboard, aPosX, aPosY);
    if (key == NULL) return;
    if (iPressedKey != NULL) {
        SetStatusForKey(iPressedKey, KEY_NORMAL);
    }
    iPressedKey = key;
    SetIndexForCurrentKey(iKeyboard, IndexOfKey(iKeyboard, key));
    SetStatusForKey(key, KEY_PRESSED);
    
    int32 key_id = IDOfKey(key);
    if (key_id == -1) return;
    

    if (SupportRegionCorrection(iKeyboard)) {
        KeyType type = TypeOfKey(key);
        NSString* tmp = [iKeyboardBridge stringAroundCursor];
        if ((type == KEY_SYMBOL_ALPHA) && (iFiltering == NO) && ([iExactInput length] == 0) 
            && ((tmp == nil) || ([tmp length] == 0))) {
            iFiltering = YES;            
        }
        if (iFiltering ) {
            if (type == KEY_SYMBOL_ALPHA) {
                NSString* value = nil;
                if (iShiftModeOn) {
                    value = [NSString stringWithCString:ShiftValueOfKey(key) encoding:NSUTF8StringEncoding]; 
                } else {
                    value = [NSString stringWithCString:ValueOfKey(key) encoding:NSUTF8StringEncoding];
                }
                [iExactInput appendString:value];            
            }
            
            if (iInputSignal.iNumOfSamplePoints == 0) {
                iTracing = NO;
                iCurrentTracing = NO;
            }
            
            if (iTracing == YES) {
                iCurrentTracing = NO;
            }
            
            if ((type == KEY_SYMBOL_ALPHA) && (iFiltering == YES)) {
                iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosX = aPosX;
                iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosY = aPosY;
                iSamplePoints[iInputSignal.iNumOfSamplePoints].iTimestamp = 0;
                
                NSUInteger index = [iExactInput length] - 1;
                iSessionCount = index + 1;
                iSessions[index].iStartIndex = iInputSignal.iNumOfSamplePoints;
                        
                iInputSignal.iNumOfSamplePoints++;
            }
        }
        
        if (key_id == KEY_ID_SPACE) {
            if (iFiltering) {
                [self sendToFilter];
                
                BOOL shouldCapitalize = [iKeyboardBridge shouldCapitalize];
                if (shouldCapitalize == YES) {
                    NSString* tmp = [iKeyboardBridge stringAroundCursor];
                    tmp = [tmp capitalizedString];
                    [iKeyboardBridge replaceTextAroundCursor:tmp];
                }
                
                [iKeyboardBridge sendText:@" " WithSpace:NO];
                iFiltering = NO;
                iInputSignal.iNumOfSamplePoints = 0;
                [iExactInput setString:@""];
                iSessionCount = 0;
            } else {
                [iKeyboardBridge sendText:@" " WithSpace:NO];
            }
            return;
        } else if (key_id == KEY_ID_ENTER) {
            if (iFiltering) {
                [self sendToFilter];
                [iKeyboardBridge sendEnter];
                iFiltering = NO;
                iInputSignal.iNumOfSamplePoints = 0;
                [iExactInput setString:@""];
                iSessionCount = 0;
            } else {
                [iKeyboardBridge sendEnter];
            }
            return;            
        }
        
        [self handleFunctionKeyDown:key];
    }
    
    if ([iLongPressTimer isValid]) [iLongPressTimer invalidate];
    if ([iRepeatPressTimer isValid]) [iRepeatPressTimer invalidate];
    iLongPressTimer = nil;
    iRepeatPressTimer = nil;
    iLongPressTimer = [NSTimer scheduledTimerWithTimeInterval:0.6 target:self selector:@selector(longPressTimerFireMethod:) userInfo:nil repeats:NO];    
}

-(void)onFingerMoveAtX:(int32)aPosX AndY:(int32)aPosY
{
    if (iKeyboard == NULL) return;
    if (SupportRegionCorrection(iKeyboard)) {
        Key* key = KeyAtPosition(iKeyboard, aPosX, aPosY);
        if (key == NULL) {
            return;
        }
        if (iPressedKey != key) {
            SetStatusForKey(iPressedKey, KEY_NORMAL);
            iPressedKey = key;
            SetIndexForCurrentKey(iKeyboard, IndexOfKey(iKeyboard, key));
            SetStatusForKey(key, KEY_PRESSED);
            
            iTracing = YES;
            iCurrentTracing = YES;
        }
        
        int32 key_id = IDOfKey(key);
        if (key_id == -1) return;
        
        if (iFiltering) {
            iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosX = aPosX;
            iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosY = aPosY;
            iSamplePoints[iInputSignal.iNumOfSamplePoints].iTimestamp = 0;
            iInputSignal.iNumOfSamplePoints++;
        }
     }
    
}

-(void)onFingerUpAtX:(int32)aPosX AndY:(int32)aPosY
{
    if (iKeyboard == NULL) return;
    Key* key = KeyAtPosition(iKeyboard, aPosX, aPosY);
    
    if (key != iPressedKey) {
        SetStatusForKey(iPressedKey, KEY_NORMAL);
        SetStatusForKey(key, KEY_NORMAL);
    } else {
        SetStatusForKey(key, KEY_NORMAL);
    }
    
    int32 key_id = IDOfKey(key);
    if (key_id == -1) return;
    KeyType type = TypeOfKey(key);
    
    if (SupportRegionCorrection(iKeyboard)) { 
        
        if (iFiltering) {            
            if ((iCurrentTracing) || (type == KEY_SYMBOL_ALPHA)) {
                
                NSUInteger index = [iExactInput length] - 1;
                iSessionCount = index + 1;
                iSessions[index].iEndIndex = iInputSignal.iNumOfSamplePoints;
                
                //iInputSignal.iNumOfSamplePoints++;
                [self sendToFilter];
                if ([iRecommendedWord length] > 1) {
                    [iKeyboardBridge replaceTextAroundCursor:iRecommendedWord];    
                } else {
                    [iKeyboardBridge replaceTextAroundCursor:iExactInput];
                }            
            } else if ((!iCurrentTracing) && ((type == KEY_SYMBOL_PUNCTUATION))) {
                [self sendToFilter];
                NSString* tmp = [NSString stringWithCString:ValueOfKey(key) encoding:NSUTF8StringEncoding];
                [iKeyboardBridge sendText:tmp WithSpace:NO];
                [iKeyboardBridge sendText:@" " WithSpace:NO];
                [iKeyboardOutputListener onResultsReceived:nil WithNum:0 WithExactWord:NO]; 
                iFiltering = NO;
                iInputSignal.iNumOfSamplePoints = 0;
                [iExactInput setString:@""];
                iSessionCount = 0;            
            } else if (key_id == KEY_ID_BACKSPACE) {
                NSUInteger len = [iExactInput length];
                
                if (len > 0) {
                    NSRange range;
                    range.location = len - 1;
                    range.length = 1;
                    [iExactInput deleteCharactersInRange:range];                    
                }
                
                if (len > 2) {
                    NSUInteger index = [iExactInput length] - 1;
                    iInputSignal.iNumOfSamplePoints = iSessions[index].iEndIndex; 
                    iSessionCount--;
                    [self sendToFilter];
                    [iKeyboardBridge replaceTextAroundCursor:iRecommendedWord]; 

                } else if (len == 2) {
                    [iKeyboardBridge replaceTextAroundCursor:iExactInput];
                    [iCandidates removeAllObjects];
                    [iCandidates addObject:iExactInput];
                    [iKeyboardOutputListener onResultsReceived:iCandidates WithNum:1 WithExactWord:NO]; 
                    iSessionCount--;
                } else if (len == 1) {                
                    iFiltering = NO;
                    [iKeyboardBridge sendBackspace];
                    [iKeyboardOutputListener onResultsReceived:nil WithNum:0 WithExactWord:NO];
                    iSessionCount = 0;
                    iInputSignal.iNumOfSamplePoints = 0;
                    [iExactInput setString:@""];
                }
            }
            
        } else {
            if (type == KEY_SYMBOL_ALPHA) {
                NSString* tmp = [NSString stringWithCString:ValueOfKey(key) encoding:NSUTF8StringEncoding];
                [iKeyboardBridge sendText:tmp WithSpace:NO];                
            } else if (type == KEY_SYMBOL_PUNCTUATION) {
                BOOL bkspace = [iKeyboardBridge shouldSendBackspace];
                if (bkspace == YES) {
                    [iKeyboardBridge sendBackspace];
                }
                NSString* tmp = [NSString stringWithCString:ValueOfKey(key) encoding:NSUTF8StringEncoding];
                [iKeyboardBridge sendText:tmp WithSpace:YES];
                [iKeyboardOutputListener onResultsReceived:nil WithNum:0 WithExactWord:NO];
            } else if (key_id == KEY_ID_BACKSPACE) {
                [iKeyboardBridge sendBackspace];
                [iKeyboardOutputListener onResultsReceived:nil WithNum:0 WithExactWord:NO];
            }
        }
        
        if (key_id == KEY_ID_123) {
            iFiltering = NO;
            [iKeyboardOutputListener onResultsReceived:nil WithNum:0 WithExactWord:NO];
            iSessionCount = 0;
            iInputSignal.iNumOfSamplePoints = 0;
            [iExactInput setString:@""]; 
            
            IME* ime = [[IMESingleton sharedInstance] instance]; 
            changeKeyboard(ime, "NUMBER");
            [iKeyboardInteractionListener onChangeKeyboard:@"NUMBER"];            
        } else if (key_id == KEY_ID_ALT) {
            iFiltering = NO;
            [iKeyboardOutputListener onResultsReceived:nil WithNum:0 WithExactWord:NO];
            iSessionCount = 0;
            iInputSignal.iNumOfSamplePoints = 0;
            [iExactInput setString:@""];   
            IME* ime = [[IMESingleton sharedInstance] instance]; 
            changeKeyboard(ime, "PUNC");
            [iKeyboardInteractionListener onChangeKeyboard:@"PUNC"];           
        } else if (key_id == KEY_ID_SHIFT) {
            if (iShiftModeOn == NO) {
                iShiftModeOn = YES;
            } else {
                iShiftModeOn = NO;

            }
            [iKeyboardInteractionListener onChangeToShiftMode:iShiftModeOn];
        }
        
    } else {
        if (key_id == KEY_ID_ABC) {
            IME* ime = [[IMESingleton sharedInstance] instance]; 
            changeKeyboard(ime, "QWERTY_ALPHA");
            [iKeyboardInteractionListener onChangeKeyboard:@"QWERTY_ALPHA"];            
        } else if (key_id == KEY_ID_123) {
            IME* ime = [[IMESingleton sharedInstance] instance]; 
            changeKeyboard(ime, "NUMBER");
            [iKeyboardInteractionListener onChangeKeyboard:@"NUMBER"];            
        } else if (key_id == KEY_ID_ALT) {
            IME* ime = [[IMESingleton sharedInstance] instance]; 
            changeKeyboard(ime, "PUNC");
            [iKeyboardInteractionListener onChangeKeyboard:@"PUNC"];            
        } else if (key_id == KEY_ID_BACKSPACE) {
            [iKeyboardBridge sendBackspace];           
        } else if ((key_id == KEY_ID_COMMA) || (key_id == KEY_ID_FULL_STOP) || 
                   (key_id == KEY_ID_QUESTION_MARK) || (key_id == KEY_ID_GANTANHAO) ||
                   (key_id == KEY_ID_FENHAO) || (key_id == KEY_ID_MAOHAO)) {
            BOOL bkspace = [iKeyboardBridge shouldSendBackspace];
            if (bkspace == YES) {
                [iKeyboardBridge sendBackspace];
            }
            NSString* value = [NSString stringWithCString:ValueOfKey(key) encoding:NSUTF8StringEncoding];
            [iKeyboardBridge sendText:value WithSpace:YES];
        } else {
            NSString* value = [NSString stringWithCString:ValueOfKey(key) encoding:NSUTF8StringEncoding];
            [iKeyboardBridge sendText:value WithSpace:NO];
        }
    }
    
    if ([iLongPressTimer isValid]) [iLongPressTimer invalidate];
    if ([iRepeatPressTimer isValid]) [iRepeatPressTimer invalidate];
    iLongPressTimer = nil;
    iRepeatPressTimer = nil;
}

-(void)cancelFingerOperation
{
    if ([iLongPressTimer isValid]) [iLongPressTimer invalidate];
    if ([iRepeatPressTimer isValid]) [iRepeatPressTimer invalidate];
    iLongPressTimer = nil;
    iRepeatPressTimer = nil;
}

-(void)handleFunctionKeyUp:(Key*) aKey
{
    
}

-(BOOL)tracing
{
    return iTracing;
}

-(void) sendToFilter
{
    const char** candidates = NULL;
    int32 i = 0;
    BOOL with_exact_word = YES;
    IME* ime = [[IMESingleton sharedInstance] instance];
    if (initialized(ime) == FALSE) return;
    if (iKeyboardOutputListener == nil) return;
    if ([iCandidates count] > 0) [iCandidates removeAllObjects];
    [iRecommendedWord setString:@""];
    
    if (([iExactInput length]  == 1) && (iTracing == NO)) {
        [iCandidates insertObject:iExactInput atIndex:0];
        [iKeyboardOutputListener onResultsReceived:iCandidates WithNum:1 WithExactWord:YES];
    } else {
        int32 n = 0;
        candidates = filterInputSignal(ime, &iInputSignal, &n); 
        
        if (n > 0) {
            NSString* candidate = nil;
            for (i = 0; i < n; i++) {
                candidate = [NSString stringWithUTF8String:(const char*)(candidates[i])];
                if (iShiftModeOn) {
                    candidate = [candidate uppercaseString]; 
                }
                
                if (i == 0) {
                    [iRecommendedWord setString:candidate];
                } 

                if ([iExactInput isEqualToString:candidate] == YES) {
                    [iCandidates insertObject:iExactInput atIndex:0];
                    with_exact_word = NO;
                    [iRecommendedWord setString:iExactInput];
                } else {
                    [iCandidates addObject:candidate];
                } 
            }   
        }
        
        BOOL flag = NO;
        if ((n <= 0) || (iTracing == NO)) {
            if ((with_exact_word == YES) && ([iExactInput length] > 0)) {
                [iCandidates insertObject:iExactInput atIndex:0];
                flag = YES;
            }
                
        }
        
        [iKeyboardOutputListener onResultsReceived:iCandidates WithNum:[iCandidates count] WithExactWord:flag];
    }
}

-(void)handleFunctionKeyDown:(Key*) aKey
{
    
}

-(void)finishInput
{
    iFiltering = NO;
    iInputSignal.iNumOfSamplePoints = 0;
    [iExactInput setString:@""];  
    [iRecommendedWord setString:@""];
    [iCandidates removeAllObjects];
    iSessionCount = 0;
}

-(void)clearCandidates
{
    [self finishInput];
    [iKeyboardOutputListener onResultsReceived:nil WithNum:0 WithExactWord:NO];
}

-(void)handleSelectedWord:(NSString*)aWord
{
    if (SupportRegionCorrection(iKeyboard) == FALSE) return;
    [aWord retain];
    NSUInteger len = [aWord length];
    const char* word = [aWord UTF8String];
    iInputSignal.iNumOfSamplePoints = 0;
    for (NSUInteger i = 0; i < len; i++) {
        char tmp[2] = {word[i], 0x00}; 
        Key* key = KeyWithLabel(iKeyboard, tmp);
        float64 posX, posY;
        CentralPositionOfKey(key, &posX, &posY);         
        iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosX = posX;
        iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosY = posY;
        iSamplePoints[iInputSignal.iNumOfSamplePoints].iTimestamp = 0;
        iInputSignal.iNumOfSamplePoints++;
    }
    
    [self sendToFilter];
    [aWord release];
}

@end
