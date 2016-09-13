#import <Foundation/Foundation.h>
#import "KeyboardHandler.h"

@interface TraceKeyboardHandler : NSObject <KeyboardHandler>
{
    Key* iPressedKey;
    InputSignal iInputSignal;
    SamplePoint* iSamplePoints;
    
    NSMutableArray* iCandidates;
    NSMutableString* iRecommendedWord;
    
    BOOL iShiftModeOn;
    BOOL iFiltering;
    
//    NSTimer *iLongPressTimer;
//    NSTimer	*iRepeatPressTimer;
}

@end
