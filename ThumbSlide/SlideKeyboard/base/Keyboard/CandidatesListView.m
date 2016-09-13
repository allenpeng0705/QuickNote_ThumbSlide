
#import "CandidatesListView.h"
//#import "ImageFactory.h"
#import "Util.h"
#import "q_malloc.h"

struct WordInfo {
    CGRect iBoundingRect;
    NSUInteger iIndex;
    NSMutableString* iWord;
    NSUInteger iFontSize;
};

struct Page {
	NSUInteger iIndexOfFirstWord;
	NSUInteger iIndexOfLastWord;
};

@implementation CandidatesListView

@synthesize iWordSelectedListener;
@synthesize iPosX;
@synthesize iPosY;
@synthesize iWidth;
@synthesize iHeight;
@synthesize iShouldDrawExactWord;

-(void)setWordSelectedListener:(id<WordSelectAction>)aWordSelectedListener
{
    self.iWordSelectedListener = aWordSelectedListener;
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
        iWidth = frame.size.width;
        iHeight = frame.size.height;
        iPosX = frame.origin.x;
        iPosY = frame.origin.y;
        
        iWordSelectedListener = nil;
        iIndexOfSelectedCandidates = 0;
        iIndexOfCurrentPage = 0;
        iNumOfPages = 0;
        iNumOfCandidates = 0;

        iWords = (WordInfo*)q_malloc(sizeof(WordInfo)*MAX_COUNT_CANDIDATES);
        iPages = (Page*)q_malloc(sizeof(Page)*MAX_COUNT_CANDIDATES);
        for (NSUInteger i = 0; i < MAX_COUNT_CANDIDATES; i++) {
            iWords[i].iWord = [[NSMutableString alloc] initWithString:@""];
            iWords[i].iIndex = 0;
            iWords[i].iBoundingRect = CGRectZero;
            
            iPages[i].iIndexOfFirstWord = 0;
            iPages[i].iIndexOfLastWord = 0;
        }
        
        iColorForCandidate = [[Util sharedInstance] FillColorOfCandidate];
        [iColorForCandidate retain];
        iColorForSelectedCandidate = [[Util sharedInstance] FillColorOfSelectedCandidate];
        [iColorForSelectedCandidate retain];
        iColorForExactCandidate = [[Util sharedInstance] FillColorOfExactCandidate];
        [iColorForExactCandidate retain];
        
        self.backgroundColor = [UIColor grayColor];
        
        iShouldDrawExactWord = NO;
    }
    return self;
}

-(void)setFrame:(CGRect)frame
{
    [super setFrame:frame];
    iWidth = frame.size.width;
    iHeight = frame.size.height;
    iPosX = frame.origin.x;
    iPosY = frame.origin.y;
    
    [self setCandidates:nil WithSelectIndex:0];
}


-(void)setCandidates:(NSArray*)aCandidates WithSelectIndex:(NSUInteger)aIndex
{
    NSUInteger i, left, right, top, bottom;
    NSUInteger min_offset = 5;
    NSUInteger max_num_candidates_page = 5;

    for (i = 0; i < iNumOfCandidates; i++) {
        [iWords[i].iWord setString:@""];
        iWords[i].iIndex = 0;
        iWords[i].iBoundingRect = CGRectZero;
        
        iPages[i].iIndexOfFirstWord = 0;
        iPages[i].iIndexOfLastWord = 0;
    }

    left = self.frame.origin.x + min_offset;
    right = self.frame.origin.x + self.frame.size.width - min_offset;
    top = self.frame.origin.y;
    bottom = self.frame.origin.y + self.frame.size.height;

    iIndexOfCurrentPage = 0;
    iIndexOfSelectedCandidates = aIndex;
    iNumOfCandidates = 0;
    iNumOfPages = 0;
    
    if (aCandidates == nil) {
        [self setNeedsDisplay];
        return;
    }
    
    iNumOfCandidates = [aCandidates count];
    
    for (i = 0; i < iNumOfCandidates; i++) {
        iWords[i].iIndex = i;
        NSString* str = (NSString*)[aCandidates objectAtIndex:i];
        [iWords[i].iWord appendString:str];
    }
    
    Page* page = &(iPages[0]);
    UIFont* default_font = [[Util sharedInstance] FontOfCandidatesList];
    
    for (i = 0; i < iNumOfCandidates; i++) {
        CGRect rect;
        NSString* content = iWords[i].iWord;
        NSDictionary *fontattributes = @{NSFontAttributeName: default_font};
        CGSize size = [content sizeWithAttributes:fontattributes];
        rect.origin.x = left;
        rect.origin.y = (self.frame.size.height - size.height)/2;
        rect.size.width = size.width;
        rect.size.height = size.height;        
        iWords[i].iFontSize = [UIFont systemFontSize]; 
        
        while (rect.size.width > self.frame.size.width - 2*min_offset) {
            iWords[i].iFontSize--;
            UIFont* font = [UIFont systemFontOfSize:iWords[i].iFontSize];
            NSDictionary *attributes = @{NSFontAttributeName: font};
            size = [content sizeWithAttributes:attributes];
            rect.size.width = size.width;
        }
        
        iWords[i].iBoundingRect = rect;
        
        if (((rect.origin.x + rect.size.width + min_offset) < right) && 
            (page->iIndexOfLastWord - page->iIndexOfFirstWord + 1 < max_num_candidates_page)) {
            page->iIndexOfLastWord = i;
            left += rect.size.width;
            left += min_offset;
        } else {
            NSUInteger j;
            NSUInteger word_num = page->iIndexOfLastWord - page->iIndexOfFirstWord + 1;
            NSUInteger space_width = self.frame.size.width;
            
            for (j = 0; j < word_num; j++) {
                space_width -= iWords[page->iIndexOfFirstWord + j].iBoundingRect.size.width;
            }            
            NSUInteger avg_space_width = space_width/(word_num + 1);
            
            for (j = 0; j < word_num; j++) {
                if (j == 0) {
                    iWords[page->iIndexOfFirstWord].iBoundingRect.origin.x = avg_space_width;
                } else {
                    iWords[page->iIndexOfFirstWord + j].iBoundingRect.origin.x = avg_space_width
                    + iWords[page->iIndexOfFirstWord + j - 1].iBoundingRect.origin.x + 
                    iWords[page->iIndexOfFirstWord + j - 1].iBoundingRect.size.width;
                }
            }

            // Start the next page
            iIndexOfCurrentPage++;
            page = &(iPages[iIndexOfCurrentPage]);
            page->iIndexOfFirstWord = i;
            page->iIndexOfLastWord = i;
            left = self.frame.origin.x + min_offset;
            right = self.frame.origin.x + self.frame.size.width - min_offset;
            
            iWords[i].iBoundingRect.origin.x = left;
            iWords[i].iBoundingRect.origin.y = (self.frame.size.height - size.height)/2;
            iWords[i].iBoundingRect.size.width = size.width;
            iWords[i].iBoundingRect.size.height = size.height;
            
            left += rect.size.width;
            left += min_offset;
        }
        
        if (i == iNumOfCandidates - 1) {
            NSUInteger j;
            NSUInteger word_num = page->iIndexOfLastWord - page->iIndexOfFirstWord + 1;
            NSUInteger space_width = self.frame.size.width;
            
            for (j = 0; j< word_num; j++) {
                space_width -= iWords[page->iIndexOfFirstWord + j].iBoundingRect.size.width;
            }
            NSUInteger avg_space_width = space_width/(word_num + 1);
            
            for (j = 0; j < word_num; j++) {
                if (j == 0) {
                    iWords[page->iIndexOfFirstWord].iBoundingRect.origin.x = avg_space_width;
                } else {
                    iWords[page->iIndexOfFirstWord + j].iBoundingRect.origin.x = avg_space_width
                    + iWords[page->iIndexOfFirstWord + j - 1].iBoundingRect.origin.x + 
                    iWords[page->iIndexOfFirstWord + j - 1].iBoundingRect.size.width;
                }
            }
        }
    }
    
    iNumOfPages = iIndexOfCurrentPage + 1;
    iIndexOfCurrentPage = 0;
    [self setNeedsDisplay];
}

-(NSUInteger)indexOfTouchWord:(CGFloat)aPointX
{
    NSUInteger ret = 0;
    CGFloat touch_x = aPointX; 
    CGFloat min_distance = self.frame.size.width;
    NSUInteger start_index = iPages[iIndexOfCurrentPage].iIndexOfFirstWord;
    NSUInteger end_index = iPages[iIndexOfCurrentPage].iIndexOfLastWord;
    if (start_index == end_index) return start_index;
    
    
    for (NSUInteger i = start_index; i <= end_index; i++) {
        CGFloat distance = 0.0;
        CGFloat x = iWords[i].iBoundingRect.origin.x + iWords[i].iBoundingRect.size.width/2;
        if (x < touch_x) {
            distance = touch_x - x;
        } else {
            distance = x - touch_x;
        }
        if (distance < min_distance) {
            min_distance = distance;
            ret = i;
        }
    }
    
    return ret;
}

- (void) touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
    if ([touches count] > 1) return;
    if ((iNumOfPages == 0) || (iNumOfCandidates == 0)) return;
    
	UITouch* touch = [touches anyObject];
	CGPoint location = [touch locationInView:touch.view];    
    UITouchPhase phase = touch.phase;
    if (phase == UITouchPhaseBegan) {
        iStartScrollX = location.x;
    }           
}

- (void) touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
    if ([touches count] > 1) return;
    if ((iNumOfPages == 0) || (iNumOfCandidates == 0)) return;
    
    UITouch* touch = [touches anyObject];
	CGPoint	 location = [touch locationInView:touch.view];
    UITouchPhase phase = touch.phase;
    CGFloat touch_x = location.x;    
	
    if (phase == UITouchPhaseMoved) {
        iScrollX = touch_x - iStartScrollX;
        [self setNeedsDisplay];
    }
}

- (void) touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
    if ([touches count] > 1) return;
    if ((iNumOfPages == 0) || (iNumOfCandidates == 0)) return;
    
    UITouch* touch = [touches anyObject];
	CGPoint	 location = [touch locationInView:touch.view];
    UITouchPhase phase = touch.phase;
    CGFloat touch_x = location.x;    
	
    if (phase == UITouchPhaseEnded) {
        iScrollX = touch_x - iStartScrollX;
        if ((touch_x > iStartScrollX) && (touch_x -iStartScrollX > self.frame.size.width/5)) {
            if (iIndexOfCurrentPage > 0) iIndexOfCurrentPage--;
        } else if ((touch_x < iStartScrollX) && (iStartScrollX - touch_x > self.frame.size.width/5)) {
            if (iIndexOfCurrentPage < iNumOfPages -1) iIndexOfCurrentPage++;
        } else {
            iIndexOfSelectedCandidates = [self indexOfTouchWord:iStartScrollX];
            if (iWordSelectedListener != nil) {
                [iWordSelectedListener selectWord:iWords[iIndexOfSelectedCandidates].iWord 
                                      AtIndex:iIndexOfSelectedCandidates OnListView:self];
            }
        }
        iScrollX = 0;
        iStartScrollX = 0;
        [self setNeedsDisplay];
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    iScrollX = 0;
    iStartScrollX = 0;
    [self setNeedsDisplay];
}



- (void)drawRect:(CGRect)rect
{
    if (iMainGraphicCtx == nil) {
        iMainGraphicCtx = UIGraphicsGetCurrentContext();
    }
    
    if (iBackgroundLayerRef == nil) {
        CGSize size = self.frame.size;        
        iBackgroundLayerRef = CGLayerCreateWithContext (
                                                        iMainGraphicCtx,
                                                        size,
                                                        NULL);
        CGContextRef ctx = CGLayerGetContext(iBackgroundLayerRef);
        UIGraphicsPushContext(ctx);
        UIColor* fillcolor = [UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:1.0];
        [[Util sharedInstance] fillRoundRect:ctx leftX:(self.frame.origin.x + 3) topY:(self.frame.origin.y + 3) 
                                  width:(self.frame.size.width - 6) height:(self.frame.size.height - 6)
                                  Color:fillcolor];
        UIGraphicsPopContext(); 
    }
    CGContextDrawLayerInRect(iMainGraphicCtx, rect, iBackgroundLayerRef);
    
    if (iNumOfPages == 0 || iNumOfCandidates == 0) return;
    
    NSUInteger i = 0;
    NSUInteger start_index = iPages[iIndexOfCurrentPage].iIndexOfFirstWord;
    NSUInteger end_index = iPages[iIndexOfCurrentPage].iIndexOfLastWord;
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    UIColor* color = nil;
    for (i = start_index; i <= end_index; i++) {
        if ((i == 0) && (iShouldDrawExactWord == YES) && (i != iIndexOfSelectedCandidates)) {
            color = iColorForExactCandidate;
            CGContextSetFillColorWithColor(ctx, [iColorForExactCandidate CGColor]);
        } else if (i == iIndexOfSelectedCandidates) {
            color = iColorForSelectedCandidate;
            CGContextSetFillColorWithColor(ctx, [iColorForSelectedCandidate CGColor]);  
        } else {
            color = iColorForCandidate;
            CGContextSetFillColorWithColor(ctx, [iColorForCandidate CGColor]);            
        }
        

        
        NSString* word = iWords[i].iWord;
        CGRect rect = iWords[i].iBoundingRect;
        rect.origin.x += iScrollX;
        UIFont* font = [UIFont systemFontOfSize:22];
        NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
        paragraphStyle.lineBreakMode = NSLineBreakByWordWrapping;
        paragraphStyle.alignment = NSTextAlignmentCenter;
        NSDictionary *attributes = @{NSFontAttributeName: font, NSParagraphStyleAttributeName: paragraphStyle, NSForegroundColorAttributeName: color};
        [word drawInRect:rect withAttributes:attributes];
        [paragraphStyle release];
    }
    
    if (((iIndexOfCurrentPage == iNumOfPages -1) && (iScrollX < 0)) ||
        ((iIndexOfCurrentPage == 0) && (iScrollX > 0)) || (iScrollX == 0)) {
        return;
    } else {
        NSInteger index = iIndexOfCurrentPage;
        if (iScrollX < 0) {
            index = iIndexOfCurrentPage + 1;
        } else {
            index  = iIndexOfCurrentPage - 1;
        }
        
        start_index = iPages[index].iIndexOfFirstWord;
        end_index = iPages[index].iIndexOfLastWord; 
        UIColor* color = nil;
        for (i = start_index; i <= end_index; i++) {
            if (i != iIndexOfSelectedCandidates) {
                color = iColorForCandidate;
                CGContextSetFillColorWithColor(ctx, [iColorForCandidate CGColor]);   
            } else {
                color = iColorForSelectedCandidate;
                CGContextSetFillColorWithColor(ctx, [iColorForSelectedCandidate CGColor]);
            }
            NSString* word = iWords[i].iWord;
            CGRect rect = iWords[i].iBoundingRect;
            if (iScrollX < 0) {
                rect.origin.x = rect.origin.x + self.frame.size.width + iScrollX;
            } else {
                rect.origin.x = rect.origin.x - self.frame.size.width + iScrollX;
            }
            UIFont* font = [UIFont systemFontOfSize:iWords[i].iFontSize];
            NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
            paragraphStyle.lineBreakMode = NSLineBreakByWordWrapping;
            paragraphStyle.alignment = NSTextAlignmentCenter;
            NSDictionary *attributes = @{NSFontAttributeName: font, NSParagraphStyleAttributeName: paragraphStyle, NSForegroundColorAttributeName: color};
            [word drawInRect:rect withAttributes:attributes];
            [paragraphStyle release];
        }   
    }        
}

- (void)dealloc
{
    for (NSUInteger i = 0; i < MAX_COUNT_CANDIDATES; i++) {
        [iWords[i].iWord release];
    }
    q_free(iWords);
    q_free(iPages);
    [iColorForExactCandidate release];
    [iColorForCandidate release];
    [iColorForSelectedCandidate release];
    [super dealloc];
}

@end
