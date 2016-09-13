#import "QIInputView.h"
#import "CandidatesListView.h"
#import "KeyboardView.h"


@implementation QIInputView

@synthesize iKeyboardView;
@synthesize iCandidatesList;
@synthesize iPosX;
@synthesize iPosY;
@synthesize iWidth;
@synthesize iHeight;

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
        CGRect rect = CGRectMake(0, 0, frame.size.width, 50);
        iCandidatesList = [[CandidatesListView alloc] initWithFrame:rect];
        [self addSubview:iCandidatesList];
        
        rect.origin.x = 0;
        rect.origin.y = 50;
        rect.size.width = frame.size.width;
        rect.size.height = frame.size.height - 50;
        iKeyboardView = [[KeyboardView alloc] initWithFrame:rect];
        [self addSubview:iKeyboardView];
        self.multipleTouchEnabled = YES;
    }
    return self;
}

-(void)setFrame:(CGRect)frame
{
    [super setFrame:frame];
    CGRect rect;
    if (frame.size.width == 320) {
        rect = CGRectMake(0, 0, frame.size.width, 50);
    } else {
        rect = CGRectMake(0, 0, frame.size.width, 40);
    }
    iCandidatesList.frame = rect;
    
    rect.origin.x = 0;
    rect.origin.y = rect.size.height;
    rect.size.width = frame.size.width;
    rect.size.height = frame.size.height - rect.size.height; 
    iKeyboardView.frame = rect;
    
    [self setNeedsLayout];
}

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

-(void)onResultsReceived:(NSArray*)aCandidates WithNum:(NSUInteger)aCount WithExactWord:(BOOL)aFlag
{
    if ((aCandidates == nil) || ([aCandidates count] == 0) || (aCount == 0)) {
        [iCandidatesList setCandidates:nil WithSelectIndex:0];
    } else {
        if (aFlag == YES) {
            if (aCount > 1) {
                iCandidatesList.iShouldDrawExactWord = YES;
                [iCandidatesList setCandidates:aCandidates WithSelectIndex:1];
                return;
            }
        }
        iCandidatesList.iShouldDrawExactWord = NO;
        [iCandidatesList setCandidates:aCandidates WithSelectIndex:0];
    }
}

-(void)clear
{
    [iKeyboardView clear];
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
