#import "QIInputView.h"
#import "CandidatesListView.h"
#import "KeyboardView.h"
#import "CommonWordSelector.h"

@implementation QIInputView

@synthesize iKeyboardView;
@synthesize iCandidatesList;
@synthesize iPosX;
@synthesize iPosY;
@synthesize iWidth;
@synthesize iHeight;

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame inputViewStyle:UIInputViewStyleKeyboard];
    if (self) {
        // Initialization code
        CGRect rect = CGRectMake(0, 0, frame.size.width, 40);
        iCandidatesList = [[CandidatesListView alloc] initWithFrame:rect];
        [self addSubview:iCandidatesList];
        
        rect.origin.x = 0;
        rect.origin.y = 40;
        rect.size.width = frame.size.width;
        rect.size.height = frame.size.height - 40;
        iKeyboardView = [[KeyboardView alloc] initWithFrame:rect];
        id<WordSelectAction> wordselector =  [[CommonWordSelector alloc] init];
        [iCandidatesList setWordSelectedListener:wordselector];
        [wordselector release];
        [self addSubview:iKeyboardView];
        self.multipleTouchEnabled = YES;
    }
    return self;
}

//-(void)setFrame:(CGRect)frame
//{
//    [super setFrame:frame];
//    CGRect rect;
//    //if (frame.size.width == 320) {
//        rect = CGRectMake(0, 0, frame.size.width, 50);
//    //} else {
//    //    rect = CGRectMake(0, 0, frame.size.width, 40);
//    //}
//    iCandidatesList.frame = rect;
//    
//    rect.origin.x = 0;
//    rect.origin.y = rect.size.height;
//    rect.size.width = frame.size.width;
//    rect.size.height = frame.size.height - rect.size.height; 
//    iKeyboardView.frame = rect;
//    
//    [self setNeedsLayout];
//}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
    [iCandidatesList drawRect:iCandidatesList.frame];
    [iKeyboardView drawRect:iKeyboardView.frame];
}
*/

-(void)onResultsReceived:(NSArray*)aCandidates
{
    if ((aCandidates == nil) || ([aCandidates count] == 0)) {
        [iCandidatesList setCandidates:nil WithSelectIndex:0];
    } else {
        iCandidatesList.iShouldDrawExactWord = NO;
        [iCandidatesList setCandidates:aCandidates WithSelectIndex:0];
    }
}

-(void)clear
{
    [iCandidatesList setNeedsDisplay];
}

- (void)dealloc
{
    [super dealloc];
}

-(void)attachKeyboard:(Keyboard*)aKeyboard
{
    if (iKeyboardView == nil) return;
    [iKeyboardView attachKeyboard:aKeyboard AndKeyboardOutputListener:self];
}



@end
