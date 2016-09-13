
#import "NotesListViewController.h"
#import "NoteViewController.h"
#import "NoteContentView.h"

#import "QIInputView.h"
#import "Note.h"
#import "LayoutConfiguration.h"
#import "IMESingleton.h"
#import "KeyboardView.h"
#import "CandidatesListView.h"
#import "DBUtil.h"
#import "Util.h"

@implementation NoteContentView

@synthesize iTimeField, iContentView, iInputView, iToolBar, iKeyboardShowing;

- (void)dealloc 
{
    [iValidCharSet release];
    [iStopCharSet release];
    
    [iTimeField release];
    [iContentView release];
    [iInputView release];
    [iToolBar release];
	[super dealloc];
}

-(void)handleOrientationChang:(BOOL)aToLandscape
{
    CGRect frame = iInputView.frame;
    if (aToLandscape == YES) {
        iInputView.iKeyboardView.iWidthRatio = 150;
        iInputView.iKeyboardView.iHeightRatio = 80;
        frame.size.width = 480;
        frame.size.height = (frame.size.height*80)/100;
    } else {
        iInputView.iKeyboardView.iWidthRatio = 100;
        iInputView.iKeyboardView.iHeightRatio = 100; 
        frame.size.width = 320;
        frame.size.height = (frame.size.height*100)/80;
    }
    iInputView.frame = frame;
    
    [self setNeedsLayout];
}

- (id)initWithFrame:(CGRect)frame 
{    
	if ((self = [super initWithFrame:frame])) {
        NSString* set = @"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'";
        iValidCharSet = (NSCharacterSet*)[NSCharacterSet characterSetWithCharactersInString:set];
        [iValidCharSet retain];
        
        NSString* stop_set = @".?!;:";
        iStopCharSet = (NSCharacterSet*)[NSCharacterSet characterSetWithCharactersInString:stop_set];
        [iStopCharSet retain];
        
        CGRect textFrame = rectOfTimeField();
        UILabel* timelabel = [[UILabel alloc] initWithFrame:textFrame];
        timelabel.font = [UIFont systemFontOfSize:14];
        timelabel.textAlignment = UITextAlignmentCenter;
        timelabel.textColor = [UIColor blueColor];
        self.iTimeField = timelabel;
        [self addSubview:iTimeField];
        [timelabel release];

        
        //main text view
        textFrame = rectOfReadOnlyTextView();
        UITextView* contentview = [[UITextView alloc] initWithFrame:textFrame];
        contentview.scrollEnabled = YES;
        contentview.showsVerticalScrollIndicator = YES;
        contentview.delegate = self;
        contentview.font = [contentview.font fontWithSize:20.0];
        self.iContentView = contentview;
        [self addSubview:iContentView]; 
        [contentview release];
        
        textFrame = rectOfInputView();
		self.iInputView = [[QIInputView alloc] initWithFrame:textFrame];
        int32 kb_width = 0;
        int32 kb_height = 0;        
        IME* ime = [[IMESingleton sharedInstance] instance];
        initIME(ime, "ENGLISH");
        SizeOfKeyboard(CurrentKeyboard(ime), &kb_width, &kb_height);
        [iInputView attachKeyboard:CurrentKeyboard(ime)];
        [iInputView.iKeyboardView setHostEditor:self];
        [iInputView.iCandidatesList setWordSelectedListener:self];
        
        self.iContentView.inputView = (UIView*)iInputView;
		
        //footer toolbar view
        UIToolbar* toolbar = [[UIToolbar alloc] initWithFrame:rectOfFootBar()];
        toolbar.barStyle = UIBarStyleBlackOpaque;
        toolbar.hidden = NO;
        self.iToolBar = toolbar;
        [self addSubview:iToolBar];
        [toolbar release];
        
        iKeyboardShowing = NO;
        iSelectionChangedByTouch = YES;
	}
	return self;
}

- (void)showInputView:(BOOL)show
{
	if (show) {
        [self showKeyboard];
    } else {
        [self hideKeyboard]; 
    }
}

- (BOOL)showingInputView
{
    return iKeyboardShowing;
}

- (void)layoutSubviews
{
	if ((iContentView.dragging) || (iContentView.decelerating)) return;    
    [super layoutSubviews];
    
    iTimeField.frame = rectOfTimeField();
    CGRect appFrame = self.frame;

    
    if (iKeyboardShowing) {
        CGRect frame = rectOfEditableTextView();
        frame.size.height = appFrame.size.height - iTimeField.frame.size.height;
        if (iContentView.inputView != nil) {
            frame.size.height -= iContentView.inputView.frame.size.height;
        } else {
            if (appFrame.size.height > appFrame.size.width) {
                frame.size.height -= 216;
            } else {
                frame.size.height -= 140;
            }
        }
        iContentView.frame = frame;
        [iToolBar removeFromSuperview];
    } else {
        iToolBar.frame = rectOfFootBar();
        CGRect frame = rectOfReadOnlyTextView();
        frame.size.height = appFrame.size.height - iTimeField.frame.size.height;
        frame.size.height -= iToolBar.frame.size.height;
        iContentView.frame = frame;

        [self addSubview:iToolBar];
	}
}

#pragma mark Text Views Delegate

-(void) setContent:(NSString*)aContent
{
    [iContentView setText:aContent];
    NSRange range;
    range.location = [aContent length];
    range.length = 0;
    [iContentView setEditable:YES];
    [iContentView setSelectedRange:range];
}

-(NSString*)content
{
    return [iContentView text];
}

-(void)insertText:(NSString*)aText
{
    [aText retain];
    [iContentView setScrollEnabled:NO];
    NSRange range = [iContentView selectedRange];
    NSMutableString* newContent = [[NSMutableString alloc] initWithString:[iContentView text]];
    if (range.length == 0) {
        [newContent insertString:aText atIndex:range.location];       
    } else {
        [newContent replaceCharactersInRange:range withString:aText];
    }    
    [iContentView setText:newContent];
    range.location += [aText length];
    range.length = 0;
    
    iSelectionChangedByTouch = NO;
    [iContentView setSelectedRange:range];
    [iContentView setScrollEnabled:YES];
    [newContent release]; 
    [aText release];
}

-(void)handleBackspace
{
    NSRange range = [iContentView selectedRange];
    NSMutableString* newContent = [[NSMutableString alloc] initWithString:[iContentView text]];  
    if (([newContent length] == 0) || ((range.location == 0) && (range.length == 0))) return;
    [iContentView setScrollEnabled:NO];
    
    if (range.length > 0) {
        [newContent deleteCharactersInRange:range];
        range.length = 0;
    } else {
        range.location -= 1;
        range.length = 1;
        [newContent deleteCharactersInRange:range];
        range.length = 0;
        
    }
    
    [iContentView setText:newContent];
    iSelectionChangedByTouch = NO;
    [iContentView setSelectedRange:range];
    [iContentView setScrollEnabled:YES];
    [newContent release]; 
    
}

-(void)handleEnter
{
    [iContentView setScrollEnabled:NO];
    NSRange range = [iContentView selectedRange];
    NSMutableString* newContent = [[NSMutableString alloc] initWithString:[iContentView text]];  
    iSelectionChangedByTouch = NO;
    if (range.length == 0) {
        [newContent insertString:@"\n" atIndex:range.location];
    } else {
        [newContent replaceCharactersInRange:range withString:@"\n"];
    } 
    range.location += 1;
    range.length = 0;
    
    [iContentView setText:newContent];
    [iContentView setSelectedRange:range];
    [iContentView setScrollEnabled:YES];
    [newContent release];     
}

-(void)handleTab
{
    NSRange range = [iContentView selectedRange];
    [iContentView setScrollEnabled:NO];
    NSMutableString* newContent = [[NSMutableString alloc] initWithString:[iContentView text]];
    iSelectionChangedByTouch = NO;
    if (range.length == 0) {
        [newContent insertString:@"\t" atIndex:range.location];
    } else {
        [newContent replaceCharactersInRange:range withString:@"\t"];
    } 
    range.location += 1;
    range.length = 0;
    
    [iContentView setText:newContent];
    [iContentView setSelectedRange:range];
    [iContentView setScrollEnabled:YES];
    [newContent release]; 
    
}

-(NSUInteger)lengthOfContent
{
    if (iContentView == nil) return 0;
    return [[iContentView text] length];
}

-(NSUInteger)indexOfCursor
{
    if (iContentView == nil) return 0;
    NSRange range = [iContentView selectedRange];
    return range.location;
}

-(NSRange)selectingRange
{
    NSRange range;
    if (iContentView == nil) {
        range.location = 0;
        range.length = 0;
    } else {
        range = [iContentView selectedRange];
    }
    return range;
}

-(void)setSelectedRange:(NSRange)aRange
{
    if (iContentView == nil) return;
    iSelectionChangedByTouch = NO;
    [iContentView setSelectedRange:aRange];   
}

-(BOOL)shouldCapitalize
{
    NSRange range = [self rangeAroundCursor];
    NSUInteger index = range.location;
    if (index == 0) return YES;
    if ([[self stringAroundCursor] isEqualToString:@"i"] == YES) return YES;

    NSMutableString* newContent = [[NSMutableString alloc] initWithString:[iContentView text]];
    
    for (NSUInteger i = 1; i <= 3; i++) {
        if ((index - i) == 0) break; 
        unichar tmp = [newContent characterAtIndex:index - i];
        if ([iStopCharSet characterIsMember:tmp] == YES) {
            [newContent release];
            return YES;
        }
    }
    [newContent release];
    return NO;
}

-(void)sendText:(NSString*)aText WithSpace:(BOOL)aSpaceFlag
{   
    if ((aText == nil) || ([aText length] == 0)) return;
    iSelectionChangedByTouch = NO;
    [self insertText:aText];
    if (aSpaceFlag == YES) {
        [self insertText:@" "]; 
    }
}

-(BOOL)shouldSendBackspace
{
    BOOL ret = NO;
    NSRange range = [self rangeAroundCursor];
    NSInteger index = range.location;
    if (index == 0) return NO;
    NSMutableString* newContent = [[NSMutableString alloc] initWithString:[iContentView text]];
    unichar tmp = [newContent characterAtIndex:index - 1];
    if ((tmp == ' ') && (range.length == 0)) {
       ret = YES; 
    }
    [newContent release];
    return ret;
}

-(NSRange)rangeAroundCursor
{
    NSUInteger start_index = 0;
    NSUInteger end_index = 0;
    NSUInteger index = 0;
    NSRange range = [self selectingRange];    
    if (range.length > 0) return range;
    
    NSMutableString* newContent = [[NSMutableString alloc] initWithString:[iContentView text]];
    if (range.location == 0) {
        start_index = 0;
    } else {
        for (index = range.location; index > 0 ; index--) {
            unichar start_char = [newContent characterAtIndex:index - 1];
            if ([iValidCharSet characterIsMember:start_char] == YES) continue;
            break;
        }
        start_index = index;
    }
    
    if (range.location == [newContent length]) {
        end_index = [newContent length];
    } else {
        for (index = range.location; index < [newContent length]; index++) {
            unichar end_char = [newContent characterAtIndex:index];
            if ([iValidCharSet characterIsMember:end_char] == YES) continue;
            break;
        }
        end_index = index;
    }
    
    range.location = start_index;
    range.length = end_index -start_index;
    [newContent release];
    return range;
    
}

- (NSString*)stringAroundCursor
{
    NSRange range;
    NSString* ret;
    
    range = [self rangeAroundCursor];
    if (range.length == 0) return nil;
    
    NSMutableString* newContent = [[NSMutableString alloc] initWithString:[iContentView text]];
    ret = [newContent substringWithRange:range];
    [newContent release];
    return ret;
    
}

-(NSRange)rangeAroundLocation:(NSUInteger)aLocation
{
    NSUInteger start_index = 0;
    NSUInteger end_index = 0;
    NSUInteger index = 0;
    NSRange range;
    range.location = aLocation;
    range.length = 0;
    
    NSMutableString* newContent = [[NSMutableString alloc] initWithString:[iContentView text]];
    if (range.location == 0) {
        start_index = 0;
    } else {
        for (index = range.location; index > 0 ; index--) {
            unichar start_char = [newContent characterAtIndex:index - 1];
            if ([iValidCharSet characterIsMember:start_char] == YES) continue;
            break;
        }
        start_index = index;
    }
    
    if (range.location == [newContent length]) {
        end_index = [newContent length];
    } else {
        for (index = range.location; index < [newContent length]; index++) {
            unichar end_char = [newContent characterAtIndex:index];
            if ([iValidCharSet characterIsMember:end_char] == YES) continue;
            break;
        }
        end_index = index;
    }
    
    range.location = start_index;
    range.length = end_index -start_index;
    [newContent release];
    return range;
    
}


- (NSString*)stringAroundLocation:(NSUInteger)aLocation
{
    NSRange range;
    NSString* ret;
    
    range = [self rangeAroundLocation:aLocation];
    if (range.length == 0) return nil;
    
    NSMutableString* newContent = [[NSMutableString alloc] initWithString:[iContentView text]];
    ret = [newContent substringWithRange:range];
    [newContent release];
    return ret;
    
}


-(BOOL)isSelecting
{
    if (iContentView == nil) return NO;
    NSRange range = [iContentView selectedRange];
    if (range.length > 0) return YES;
    return NO;
}

-(NSString*)selectedString
{
    if ([self isSelecting] == NO) return nil;
    NSMutableString* newContent = [[NSMutableString alloc] initWithString:[iContentView text]];
    return [newContent substringWithRange:[self selectingRange]];
}

-(void)jumpToStart
{
    NSRange range;
    range.location = 0;
    range.length = 0;
    iSelectionChangedByTouch = NO;
    [iContentView setSelectedRange:range];
}

-(void)replaceTextAroundCursor:(NSString*)aText
{
    if (aText == nil) return;
    [aText retain];
    NSRange range;
    
    iSelectionChangedByTouch = NO;
    if ([self isSelecting] == NO) {
        range = [self rangeAroundCursor];
        if (range.length > 0) {
            [self setSelectedRange:range];
        }
    }
    if ([aText length] > 0) {
        [self insertText:aText]; 
    } else {
        [self handleBackspace];
    }
    [aText release];
}


-(void)jumpToEnd
{
    NSRange range;
    range.location = [self lengthOfContent];
    range.length = 0;
    iSelectionChangedByTouch = NO;
    [iContentView setSelectedRange:range];    
}

-(void)hideKeyboard
{
    QuickNoteDelegate* delegate = [[Util sharedInstance] appDelegate];
    [delegate autoSaveTimerSuspend];

    [iContentView resignFirstResponder];
    
}

-(void)showKeyboard
{
    QuickNoteDelegate* delegate = [[Util sharedInstance] appDelegate];
    [delegate autoSaveTimerResume];
    [iInputView clear];
    [iContentView becomeFirstResponder]; 
}


-(void) selectWord:(NSString*)aWord AtIndex:(NSUInteger)aIndex OnListView:(CandidatesListView*)aView
{
    if ((aWord == nil) || ([aWord length] == 0)) return;
    [aWord retain];
    BOOL without_space = [self isSelecting];
    BOOL bkSpace = [self shouldSendBackspace];
    if (bkSpace == YES) {
        [self handleBackspace];
    }
    
    BOOL capitalize = [self shouldCapitalize];
    NSString* tmp = nil;

    if (capitalize == YES) {
        NSString* tmp = [aWord capitalizedString];
        [tmp retain];
        [self replaceTextAroundCursor:tmp];
    } else {
        [self replaceTextAroundCursor:aWord];
    }
    
    if (without_space == NO) {
        [self sendText:@" " WithSpace:NO];
    }
    
    QIInputView* inputView = (QIInputView*)(iContentView.inputView);
    [[inputView.iKeyboardView handler] finishInput];
    
    if (aIndex == 0) {
        [[DBUtil sharedInstance] addUserWord:aWord];
    }
    
    if (aIndex != 1) {
        [[DBUtil sharedInstance] addActiveWord:aWord];
    }
    
    [aWord release];
    [tmp release];
}


- (BOOL)textViewShouldBeginEditing:(UITextView *)textView
{
    return YES;
}

- (BOOL)textViewShouldEndEditing:(UITextView *)textView
{
    return YES;
}

- (void)textViewDidEndEditing:(UITextView *)textView
{
    QIInputView* inputView = (QIInputView*)(iContentView.inputView);
    [[inputView.iKeyboardView handler] clearCandidates];    
}

- (void)textViewDidChangeSelection:(UITextView *)textView
{
    NSRange range = [self selectingRange];
    if (range.length > 0) {
        if (iSelectionChangedByTouch == YES) {
            QIInputView* inputView = (QIInputView*)(iContentView.inputView);
            [[inputView.iKeyboardView handler] clearCandidates];
            NSString* selectedStr = [self selectedString];
            NSString* word = [self stringAroundLocation:(range.location + range.length)];
            
            if ([word isEqualToString:selectedStr] == YES) {
                [[inputView.iKeyboardView handler] handleSelectedWord:word];
            }
            
        } else {
            iSelectionChangedByTouch = YES;
        }
    }

    DBUtil* noteDB = [DBUtil sharedInstance];
    Note* note = noteDB.iCurrentNote;
    NSString* date = [[[Util sharedInstance] dateFormatter] stringFromDate:note.iDate];
    NSUInteger count = [self lengthOfContent];
    NSString* text = [NSString stringWithFormat:@"%@, Count:%d", date, count]; 
    iTimeField.text = text;
    [iTimeField setNeedsDisplay];
    
}

- (void)textViewDidChange:(UITextView *)textView
{
    
}

- (void)textViewDidBeginEditing:(UITextView *)textView
{
    [self jumpToEnd];
}

- (BOOL)textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text
{
    return YES;
}

@end
