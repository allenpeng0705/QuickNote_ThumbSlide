#import "TraceKeyboardHandler.h"
#import "IMESingleton.h"

#include "q_malloc.h"
#include "utf8_string.h"

@interface TraceKeyboardHandler (private)

//- (void)repeatPressTimerFireMethod:(NSTimer*)theTimer;

-(void) sendToFilter;
-(void) finishInput;
//-(void)handleSelectedWord:(NSString*)tmp;

@end


@implementation TraceKeyboardHandler
@synthesize iKeyboard;
@synthesize iKeyboardBridge;
@synthesize iKeyboardInteractionListener;
@synthesize iKeyboardOutputListener;

-(id)init
{
    if ((self = [super init])) {
        iSamplePoints = (SamplePoint*)q_malloc(sizeof(SamplePoint)*MAX_WORD_LENGTH*2);
        iInputSignal.iSamplePoints = iSamplePoints;
        iInputSignal.iNumOfSamplePoints = 0;
        
        iCandidates = [[NSMutableArray alloc] initWithCapacity:MAX_COUNT_CANDIDATES + 1];
        iRecommendedWord = [[NSMutableString alloc] init];
        
        iFiltering = NO;
        iShiftModeOn = NO;        
        iPressedKey = NULL;
        
//        iLongPressTimer = nil;
//        iRepeatPressTimer = nil;
    }
    return self;
}

-(void)dealloc
{
    if (iSamplePoints != NULL) {
        q_free(iSamplePoints);
    }
    [iCandidates removeAllObjects];
    [iCandidates release];
    [iKeyboardBridge release];
//    if ([iLongPressTimer isValid]) [iLongPressTimer invalidate];
//    if ([iRepeatPressTimer isValid]) [iRepeatPressTimer invalidate];
    [super dealloc];
}

//- (void)longPressTimerFireMethod:(NSTimer*)theTimer 
//{
//    if ([iLongPressTimer isValid]) [iLongPressTimer invalidate];
//    iLongPressTimer = nil;	
//    if (iPressedKey == NULL) return;
//        
//    if (IDOfKey(iPressedKey) == KEY_ID_BACKSPACE) {
//        [iKeyboardBridge sendBackspace];
//        iRepeatPressTimer = [NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(repeatPressTimerFireMethod:) userInfo:nil repeats:YES];
//    }
//}
//
//
//- (void)repeatPressTimerFireMethod:(NSTimer*)theTimer 
//{
//    if (iPressedKey == NULL) return;
//    
//    if (IDOfKey(iPressedKey) == KEY_ID_BACKSPACE) {
//        [self clear];
//        [iKeyboardBridge sendBackspace];
//    } else {
//        if ([iRepeatPressTimer isValid]) [iRepeatPressTimer invalidate];
//        iRepeatPressTimer = nil;
//    }
//}

-(void)onFingerDownAtX:(int32)aPosX AndY:(int32)aPosY
{
    [self finishInput];
    if (iKeyboard == NULL) return;
    Key* key = KeyAtPosition(iKeyboard, aPosX, aPosY);
    if (key == NULL) return;
    if (iPressedKey != NULL) {
        SetStatusForKey(iPressedKey, KEY_NORMAL);
    }
    iPressedKey = key;
    SetIndexForCurrentKey(iKeyboard, IndexOfKey(iKeyboard, key));
    SetStatusForKey(iPressedKey, KEY_PRESSED);
    
    if (SupportRegionCorrection(iKeyboard)) {
        iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosX = aPosX;
        iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosY = aPosY;
        iSamplePoints[iInputSignal.iNumOfSamplePoints].iTimestamp = 0;
        iInputSignal.iNumOfSamplePoints++;
    }
    
//    if ([iLongPressTimer isValid]) [iLongPressTimer invalidate];
//    if ([iRepeatPressTimer isValid]) [iRepeatPressTimer invalidate];
//    iLongPressTimer = nil;
//    iRepeatPressTimer = nil;
//    iLongPressTimer = [NSTimer scheduledTimerWithTimeInterval:0.6 target:self selector:@selector(longPressTimerFireMethod:) userInfo:nil repeats:NO];    
}

-(void)onFingerMoveAtX:(int32)aPosX AndY:(int32)aPosY
{
    if (iKeyboard == NULL) return;
    if (SupportRegionCorrection(iKeyboard)) {
        iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosX = aPosX;
        iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosY = aPosY;
        iSamplePoints[iInputSignal.iNumOfSamplePoints].iTimestamp = 0;
        iInputSignal.iNumOfSamplePoints++;
        
        Key* key = KeyAtPosition(iKeyboard, aPosX, aPosY);
        if (key == NULL) return;
        if (iPressedKey != key) {
            iFiltering = YES;
            SetIndexForCurrentKey(iKeyboard, IndexOfKey(iKeyboard, key));
        }
     }
    
}

-(void)onFingerUpAtX:(int32)aPosX AndY:(int32)aPosY
{
    if (iKeyboard == NULL) return;
    Key* key = KeyAtPosition(iKeyboard, aPosX, aPosY);
    SetStatusForKey(iPressedKey, KEY_NORMAL);
    
    int32 key_id = IDOfKey(key);
    if (key_id == -1) return;
    KeyType type = TypeOfKey(key);
    
    if (iFiltering) {
        if (SupportRegionCorrection(iKeyboard)) {
            iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosX = aPosX;
            iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosY = aPosY;
            iSamplePoints[iInputSignal.iNumOfSamplePoints].iTimestamp = 0;
            iInputSignal.iNumOfSamplePoints++;
            [self sendToFilter];
            
            if ([iRecommendedWord length] > 1) {
                [iKeyboardBridge sendText:iRecommendedWord];
            }
        }
    } else {
        if (key != iPressedKey) return;
        
        if (type == KEY_SYMBOL_PUNCTUATION) {
            NSString* tmp = [NSString stringWithCString:ValueOfKey(key) encoding:NSUTF8StringEncoding];
            [iKeyboardBridge sendText:tmp];
            [iKeyboardBridge sendText:@" "];
        } else if (type == KEY_SYMBOL_ALPHA) {
            NSString* tmp = [NSString stringWithCString:ValueOfKey(key) encoding:NSUTF8StringEncoding];
            [iKeyboardBridge sendText:tmp];
        } else if (key_id == KEY_ID_SHIFT) {
            if (iShiftModeOn == NO) {
                iShiftModeOn = YES;
            } else {
                iShiftModeOn = NO;
            }
            [iKeyboardInteractionListener onChangeToShiftMode:iShiftModeOn];
        } else if (key_id == KEY_ID_ABC) {
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
        } else if (key_id == KEY_ID_SPACE) {
            [iKeyboardBridge sendText:@" "];
        } else if (key_id == KEY_ID_ENTER) {
            //[iKeyboardBridge sendEnter];
            [iKeyboardBridge changeToNextSysKeyboard];
        } else if ((key_id == KEY_ID_COMMA) || (key_id == KEY_ID_FULL_STOP) ||
                   (key_id == KEY_ID_QUESTION_MARK) || (key_id == KEY_ID_GANTANHAO) ||
                   (key_id == KEY_ID_FENHAO) || (key_id == KEY_ID_MAOHAO)) {
            
            NSString* value = [NSString stringWithCString:ValueOfKey(key) encoding:NSUTF8StringEncoding];
            [iKeyboardBridge sendText:value];
        } else {
            NSString* value = [NSString stringWithCString:ValueOfKey(key) encoding:NSUTF8StringEncoding];
            [iKeyboardBridge sendText:value];
        }
        
    }
    
//    if ([iLongPressTimer isValid]) [iLongPressTimer invalidate];
//    if ([iRepeatPressTimer isValid]) [iRepeatPressTimer invalidate];
//    iLongPressTimer = nil;
//    iRepeatPressTimer = nil;
}

-(void)cancelFingerOperation
{
    [self finishInput];
//    if ([iLongPressTimer isValid]) [iLongPressTimer invalidate];
//    if ([iRepeatPressTimer isValid]) [iRepeatPressTimer invalidate];
//    iLongPressTimer = nil;
//    iRepeatPressTimer = nil;
}

-(void) sendToFilter
{
    const char** candidates = NULL;
    int32 i = 0;
    IME* ime = [[IMESingleton sharedInstance] instance];
    if (initialized(ime) == FALSE) return;
    if (iKeyboardOutputListener == nil) return;
    if ([iCandidates count] > 0) [iCandidates removeAllObjects];
    [iRecommendedWord setString:@""];
    
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

            [iCandidates addObject:candidate];
        }   
    }
    
    [iKeyboardOutputListener onResultsReceived:iCandidates];
}

-(void)finishInput
{
    iFiltering = NO;
    iInputSignal.iNumOfSamplePoints = 0;
    [iRecommendedWord setString:@""];
    [iCandidates removeAllObjects];
    [iKeyboardOutputListener onResultsReceived:nil];
}

//-(void)handleSelectedWord:(NSString*)aWord
//{
//    if (SupportRegionCorrection(iKeyboard) == FALSE) return;
//    [aWord retain];
//    NSUInteger len = [aWord length];
//    const char* word = [aWord UTF8String];
//    iInputSignal.iNumOfSamplePoints = 0;
//    for (NSUInteger i = 0; i < len; i++) {
//        char tmp[2] = {word[i], 0x00}; 
//        Key* key = KeyWithLabel(iKeyboard, tmp);
//        float64 posX, posY;
//        CentralPositionOfKey(key, &posX, &posY);         
//        iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosX = posX;
//        iSamplePoints[iInputSignal.iNumOfSamplePoints].iPosY = posY;
//        iSamplePoints[iInputSignal.iNumOfSamplePoints].iTimestamp = 0;
//        iInputSignal.iNumOfSamplePoints++;
//    }
//    
//    [self sendToFilter];
//    [aWord release];
//}

@end
