
#import <UIKit/UIKit.h>
#import "WordSelectAction.h"
#import "KeyboardOutputListener.h"
#import "q_keyboard.h"

@class KeyboardView;
@class CandidatesListView;

@interface QIInputView : UIInputView <KeyboardOutputListener>
{
    KeyboardView* iKeyboardView;
    CandidatesListView* iCandidatesList;
    
    NSUInteger iPosX;
    NSUInteger iPosY;
    NSUInteger iWidth;
    NSUInteger iHeight;
}
@property (nonatomic,retain) KeyboardView* iKeyboardView;
@property (nonatomic,retain) CandidatesListView *iCandidatesList;
@property (nonatomic) NSUInteger iPosX;
@property (nonatomic) NSUInteger iPosY;
@property (nonatomic) NSUInteger iWidth;
@property (nonatomic) NSUInteger iHeight;

-(void)attachKeyboard:(Keyboard*)aKeyboard;
-(void)clear;

@end
