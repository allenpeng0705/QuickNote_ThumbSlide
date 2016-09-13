#import "WordSelectAction.h"

@class QIInputView;

@interface NoteContentView : UIView <UITextViewDelegate,
                                     UIActionSheetDelegate,
                                        WordSelectAction>
{
    UILabel *iTimeField;
    UITextView  *iContentView;
    QIInputView *iInputView;
    UIToolbar   *iToolBar;
    BOOL iKeyboardShowing;
    NSCharacterSet *iValidCharSet;
    NSCharacterSet *iStopCharSet;
    
    BOOL iSelectionChangedByTouch;
}

@property (nonatomic,retain) UITextView  *iContentView;
@property (nonatomic,retain) UILabel *iTimeField;
@property (nonatomic,retain) QIInputView *iInputView;
@property (nonatomic,retain) UIToolbar   *iToolBar;
@property (nonatomic) BOOL iKeyboardShowing;

- (void)showInputView:(BOOL)show;
- (BOOL)showingInputView;

-(NSUInteger)lengthOfContent;
-(NSUInteger)indexOfCursor;
-(NSRange)selectingRange;
-(BOOL)isSelecting;
-(void)handleBackspace;
-(void)handleEnter;
-(void)handleTab;
-(void) setContent:(NSString*)aContent;
-(NSString*)content;
-(void)insertText:(NSString*)aText;
-(NSString*)selectedString;
-(NSRange)rangeAroundCursor;
-(NSString*)stringAroundCursor;
-(void)jumpToStart;
-(void)jumpToEnd;
-(void)hideKeyboard;
-(void)showKeyboard;
-(void)setSelectedRange:(NSRange)aRange;
-(void)handleOrientationChang:(BOOL)aToLandscape;
-(BOOL)shouldCapitalize;
-(BOOL)shouldSendBackspace;
-(void)replaceTextAroundCursor:(NSString*)aText;
-(void)sendText:(NSString*)aText WithSpace:(BOOL)aSpaceFlag;
-(NSRange)rangeAroundLocation:(NSUInteger)aLocation;
- (NSString*)stringAroundLocation:(NSUInteger)aLocation;

@end
