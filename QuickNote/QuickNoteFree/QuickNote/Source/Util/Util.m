
#import "Util.h"
#import <sqlite3.h>

#define FONT_SIZE_QIVIEW        20.0
#define CURSOR_WIDTH_QIVIEW   2.0
#define CURSOR_BLINK_INTERVAL_QIVIEW  0.75 //unit seconds
#define MAX_LINENUM_QIVIEW 1000
#define MAX_CONTENT_LENGTH_QIVIEW 50000

NSString* const BeginEditingNotification = @"ScrollQITextViewBeginEditingNotification";
NSString* const TextWillChangeNotification	= @"ScrollQITextViewTextWillChangeNotification";
NSString* const TextChangedNotification = @"ScrollQITextViewTextChangedNotification";
NSString* const SelectionChangedNotification = @"ScrollQITextViewSelectionChangedNotification";
NSString* const EndEditingNotification	  = @"ScrollQITextViewEndEditingNotification";

static Util* _instance = nil;

@implementation Util

+ (Util *)sharedInstance
{
    NSAutoreleasePool *thePool = [[NSAutoreleasePool alloc] init];
    
    @synchronized(@"Util.instance") {
        if (_instance == nil) {
            _instance = [[Util alloc] init];
            
            [_instance SetFontSizeForQITextView:FONT_SIZE_QIVIEW];
            [_instance SetCursorBlinkIntervalOfQITextView:CURSOR_BLINK_INTERVAL_QIVIEW];
            [_instance SetCursorWidthOfQITextView:CURSOR_WIDTH_QIVIEW];
            [_instance SetMaxLineNumForQITextView:MAX_LINENUM_QIVIEW];
            [_instance SetMaxContentLengthForQITextView:MAX_CONTENT_LENGTH_QIVIEW];
            [_instance SetFontNameForQITextView:(([UIFont systemFontOfSize:FONT_SIZE_QIVIEW]).fontName)];
            [_instance SetTextColorOfQITextView:[UIColor colorWithRed:(CGFloat)0x00/0xff 
                        green:(CGFloat)0x00/0xff blue:(CGFloat)0x00/0xff alpha:1]];
            [_instance SetFillColorOfQITextView:[UIColor colorWithRed:(CGFloat)0x00/0xff 
                                                                green:(CGFloat)0x00/0xff blue:(CGFloat)0x00/0xff alpha:1]];
            
            [_instance SetColorForCandidate:[UIColor colorWithRed:(CGFloat)0xff/0xff 
                                                                green:(CGFloat)0xff/0xff blue:(CGFloat)0xff/0xff alpha:0xff/0xff]];
            [_instance SetColorForSelectedCandidate:[UIColor colorWithRed:(CGFloat)0x00/0xff 
                                                                green:(CGFloat)0xCC/0xff blue:(CGFloat)0x00/0xff alpha:0xff/0xff]];
            [_instance SetColorForExactCandidate:[UIColor colorWithRed:(CGFloat)0x00/0xff 
                                                                green:(CGFloat)0xff/0xff blue:(CGFloat)0xff/0xff alpha:0xff/0xff]];
            
            [_instance SetFontSizeForTimeFieldOfWritingPage:14.0];
            [_instance SetBackgroundColorForScrollQITextView:[UIColor whiteColor]];

        }
    }
    
    [thePool release];
    
    return(_instance);
}

-(id) init
{
    if ((self = [super init])) {
        iDateFormatter = [[NSDateFormatter alloc] init];
        [iDateFormatter setDateStyle:NSDateFormatterMediumStyle];
        [iDateFormatter setTimeStyle:NSDateFormatterShortStyle];
    }
    return self;    
}

- (void)dealloc 
{
    [iFontOfQITextView release];
    [iTextColorOfQITextView release];
    [iFillColorOfQITextView release];
    [iBackgroundColorOfScrollQITextView release];
    [iFontNameOfQITextView release];
    [iFontOfCandidatesList release];
    [iDateFormatter release];
    
	[super dealloc];
}

-(CGFloat)FontSizeOfQITextView
{
    return iFontSizeOfQITextView;
}

-(void)SetFontSizeForQITextView:(CGFloat)aFontSize
{
    iFontSizeOfQITextView = aFontSize;
}

-(NSString*)FontNameOfQITextView
{
    return iFontNameOfQITextView;
}

-(void)SetFontNameForQITextView:(NSString*)aFontName
{
    [iFontNameOfQITextView release];
    iFontNameOfQITextView = [[NSString alloc] initWithString:aFontName];
}

-(UIFont*)FontOfQITextView
{   
    if (iFontOfQITextView != nil) return iFontOfQITextView;
    if ((iFontNameOfQITextView == nil) || (0 == [iFontNameOfQITextView length])) return nil;
    iFontOfQITextView = [[UIFont fontWithName:iFontNameOfQITextView size:iFontSizeOfQITextView] retain];
    return iFontOfQITextView;
}

-(UIFont*)FontOfCandidatesList
{
    if (iFontOfCandidatesList != nil) return iFontOfCandidatesList;
    iFontOfCandidatesList = [UIFont systemFontOfSize:24];
    [iFontOfCandidatesList retain];
    return iFontOfCandidatesList;    
}

-(UILineBreakMode)LineBreakModeOfQITextView
{
    return iLineBreakMode;
}

-(NSUInteger) MaxContentLengthOfQITextView
{
    return iMaxContentlengthOfQITextView;
}

-(void)SetMaxContentLengthForQITextView:(NSUInteger)aMaxContentLen
{
    iMaxContentlengthOfQITextView = aMaxContentLen;
}

-(NSUInteger)FontHeightOfTextView
{
    UIFont* font = [self FontOfQITextView];
   return [@"The height of one line" sizeWithFont:font].height; 
}

-(NSUInteger) MaxLineNumOfQITextView
{
    return iMaxLineNumOfQITextView;
}

-(void)SetMaxLineNumForQITextView:(NSUInteger)aMaxLineNum
{
    iMaxLineNumOfQITextView = aMaxLineNum;
}

-(NSTimeInterval)CursorBlinkIntervalOfQITextView
{
    return iCursorBlinkIntervalOfQITextView;
}

-(void)SetCursorBlinkIntervalOfQITextView:(NSTimeInterval)aCursorBlinkInterval
{
    iCursorBlinkIntervalOfQITextView = aCursorBlinkInterval;
}

-(NSUInteger)CursorWidthOfQITextView
{
    return iCursorWidthOfQITextView;
}

-(void)SetCursorWidthOfQITextView:(NSUInteger)aWidthOfCuror
{
    iCursorWidthOfQITextView = aWidthOfCuror;
}

-(UIColor*)TextColorOfQITextView
{
    return iTextColorOfQITextView;
}

-(void)SetTextColorOfQITextView:(UIColor*)aTextColor
{
    [iTextColorOfQITextView release];
    iTextColorOfQITextView = aTextColor;
    [iTextColorOfQITextView retain];
}

-(UIColor*)FillColorOfQITextView
{
    return iFillColorOfQITextView;
}

-(void)SetColorForSelectedCandidate:(UIColor*)aColor
{
    [iColorForSelectedCandidate release];
    iColorForSelectedCandidate = aColor;
    [iColorForSelectedCandidate retain];
}

-(UIColor*)FillColorOfSelectedCandidate
{
    return iColorForSelectedCandidate;
}

-(void)SetColorForCandidate:(UIColor*)aColor
{
    [iColorForCandidate release];
    iColorForCandidate = aColor;
    [iColorForCandidate retain];
}

-(UIColor*)FillColorOfCandidate
{
    return iColorForCandidate;
}

-(void)SetColorForExactCandidate:(UIColor*)aColor
{
    [iColorForExactCandidate release];
    iColorForExactCandidate = aColor;
    [iColorForExactCandidate retain];
}

-(UIColor*)FillColorOfExactCandidate
{
    return iColorForExactCandidate;
}

-(void)SetFillColorOfQITextView:(UIColor*)aFillColor
{
    [iFillColorOfQITextView release];
    iFillColorOfQITextView = aFillColor;
    [iFillColorOfQITextView retain];
}

-(UIEdgeInsets)EdgeOfScrollQITextView
{
	return UIEdgeInsetsMake(0.0/*top*/, 0.0/*left*/, 0.0/*bottom*/, 10.0/*right*/);
//	return UIEdgeInsetsMake( 0.0/*top*/,  0.0/*left*/,	0.0/*bottom*/,	0.0/*right*/);

}

-(UIEdgeInsets)EdgeOfQITextView
{
    //return UIEdgeInsetsMake(10.0/*top*/,  0.0/*left*/, 10.0/*bottom*/,  0.0/*right*/);
    return UIEdgeInsetsMake( -30.0/*top*/, 10.0/*left*/,  20.0/*bottom*/, 10.0/*right*/);

}

-(void)SetBackgroundColorForScrollQITextView:(UIColor*)aColor
{
    [iBackgroundColorOfScrollQITextView release];
    iBackgroundColorOfScrollQITextView = aColor;
    [iBackgroundColorOfScrollQITextView retain];
}

-(UIColor*)BackgroundColorOfScrollQITextView
{
    return iBackgroundColorOfScrollQITextView;
}

-(NSString*)BeginEditingNotification
{
    return BeginEditingNotification;
}

-(NSString*)TextWillChangeNotification
{
    return TextWillChangeNotification;    
}

-(NSString*)TextChangedNotification
{
    return TextChangedNotification;
}

-(NSString*)SelectionChangedNotification
{
    return SelectionChangedNotification;    
}

-(NSString*)EndEditingNotification
{
    return EndEditingNotification;
}

-(CGFloat)FontSizeOfTimeFieldOfWritingPage
{
    return iFontSizeOfTimeFieldOfWritingPage;
}

-(void)SetFontSizeForTimeFieldOfWritingPage:(CGFloat)aFontSize
{
    iFontSizeOfTimeFieldOfWritingPage = aFontSize;
}

- (void)makeFileWritable:(NSString*)aFilename
{
    NSError *error = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docPath = [paths objectAtIndex:0];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *writableFullPath = [docPath stringByAppendingPathComponent:aFilename];
	
    if (![fileManager fileExistsAtPath:writableFullPath]) {
        NSString *defaultFullPath = [[[NSBundle mainBundle] resourcePath] 
                                     stringByAppendingPathComponent:aFilename];
        [fileManager copyItemAtPath:defaultFullPath toPath:writableFullPath error:&error];
        NSAssert2(!error, @"Failed to create writable file:%@ with message '%@'.",aFilename,[error localizedDescription]);
    }
}

- (NSString*)versionOfApp
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    
	NSString *currentVer = [defaults stringForKey:@"Current Version"];
	if (!currentVer) return nil;

    return currentVer;
}

- (BOOL) isSQLComment:(NSString *) aStatementFragment
{	
	NSString *str = [aStatementFragment stringByTrimmingCharactersInSet:
					 [NSCharacterSet whitespaceCharacterSet]];
	
	// First two characters should now be the SQL comment "--"
	if ([str rangeOfString:@"--"].location != 0) {
		return NO;
	}
	return YES;
}

-(BOOL) checkVersion
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *ver = [self versionOfApp];
    NSString *verOfBundle = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"];
    if (verOfBundle == nil) {
        NSAssert(verOfBundle, @"Cannot get the version of the Bundle");
    }
    
    if ((ver == nil) || ([ver compare:verOfBundle] == NO)) {
        [defaults setObject:verOfBundle forKey:@"Current Version"];
        return NO;
    }
    return YES;
}

- (BOOL)executeSQLScript:(NSString *)aScript OnDBNamed:(NSString *)aDBName 
{	
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *docPath = [paths objectAtIndex:0];
    NSString *dbPath = [docPath stringByAppendingPathComponent:aDBName];
    
	sqlite3 *noteDB;
    
	int result = sqlite3_open([dbPath UTF8String], &noteDB);
	if (result != SQLITE_OK) {
		return NO;
	}
    
	char *errorMessage = NULL;
	BOOL success = NO;
	result = sqlite3_exec(noteDB, [aScript UTF8String], NULL, NULL, &errorMessage);
	if (result != SQLITE_OK) {
		NSLog(@"Database reported error while executing upgrade script. Error is: %s", errorMessage);
		sqlite3_free(errorMessage);
	} else {
		success = YES;
	}
	
	sqlite3_close(noteDB);
	return success;
}

-(BOOL) isDevicePortrait
{
	return UIInterfaceOrientationIsPortrait(UIInterfaceOrientationPortrait);
}


-(CGRect)makeRectForFootBar
{
    return CGRectMake(0.0, 367.0,  320.0,  49.0);
}

-(NSDateFormatter*)dateFormatter
{
    return iDateFormatter;
}

-(void)drawText:(CGContextRef)aCtx AtX:(int)x AtY:(int)y WithContent:(NSString*)aText Font:(UIFont*)aFont Color:(const UIColor*)aColor
{
    UIGraphicsPushContext(aCtx);
    CGContextSaveGState(aCtx);
    
    if (nil != aColor) {
        const CGFloat *color = CGColorGetComponents(aColor.CGColor);  
        CGFloat red = color[0]; 
        CGFloat green = color[1];
        CGFloat blue = color[2];
        CGFloat alpha = color[CGColorGetNumberOfComponents(aColor.CGColor) - 1];
        CGContextSetRGBFillColor(aCtx,red, green, blue, alpha);
    }
    
    if (NULL != aFont) {
        [aText drawAtPoint:CGPointMake(x, y) withFont:aFont];
    } else {
        [aText drawAtPoint:CGPointMake(x,y) withFont:[UIFont systemFontOfSize:[UIFont systemFontSize]]];
    }
    
    CGContextRestoreGState(aCtx);    
    UIGraphicsPopContext();
}

-(void) drawText:(CGContextRef)aCtx InRect:(CGRect)aRect WithContent:(NSString*)aText Font:(UIFont*)aFont Color:(const UIColor*)aColor
{
    UIGraphicsPushContext(aCtx);
    CGContextSaveGState(aCtx);
    
    if (nil != aColor) {
        const CGFloat *color = CGColorGetComponents(aColor.CGColor);  
        CGFloat red = color[0]; 
        CGFloat green = color[1];
        CGFloat blue = color[2];
        CGFloat alpha = color[CGColorGetNumberOfComponents(aColor.CGColor) - 1];
        CGContextSetRGBFillColor(aCtx,red, green, blue, alpha);
    }
    
    CGSize txtSize = [aText sizeWithFont:aFont];
    int x, y, w, h;
    x = aRect.origin.x;
    y = aRect.origin.y;
    w = aRect.size.width;
    h = aRect.size.height;
    
    CGPoint startPoint =CGPointMake(x + (w - txtSize.width)/2, y + (h - txtSize.height)/2); 
    
    [aText drawAtPoint:startPoint forWidth:w withFont:aFont lineBreakMode:UILineBreakModeWordWrap];
    
    CGContextRestoreGState(aCtx);
    UIGraphicsPopContext();
}

-(void)lineTo:(CGContextRef)aCtx startX:(int)aX1 startY:(int)aY1 endX:(int)aX2 endY:(int)aY2 strokeWidth:(int)aStrokeWidth Color:(UIColor*)aColor
{
    
    /* Set the line property*/
    CGContextSetLineWidth(aCtx, aStrokeWidth);
	CGContextSetLineCap(aCtx, kCGLineCapRound);
    if (nil != aColor) {
        const CGFloat *color = CGColorGetComponents(aColor.CGColor);  
        CGFloat red = color[0]; 
        CGFloat green = color[1];
        CGFloat blue = color[2];
        CGFloat alpha = color[CGColorGetNumberOfComponents(aColor.CGColor) - 1];
        CGContextSetRGBFillColor(aCtx,red, green, blue, alpha);
    }
    
	/* Draw a line segment in the context */
	CGContextBeginPath(aCtx);
	CGContextMoveToPoint(aCtx, aX1, aY1);
	CGContextAddLineToPoint(aCtx, aX1, aY1);
	CGContextStrokePath(aCtx);
}

-(void)drawRect:(CGContextRef)aCtx leftX:(int)aLeft topY:(int)aTop width:(int)aWidth height:(int)aHeight strokeWidth:(int)aStrokeWidth Color:(UIColor*)aColor
{
    CGContextSetLineWidth(aCtx, aStrokeWidth);
	CGContextSetLineCap(aCtx, kCGLineCapRound);
    if (nil != aColor) {
        const CGFloat *color = CGColorGetComponents(aColor.CGColor);  
        CGFloat red = color[0]; 
        CGFloat green = color[1];
        CGFloat blue = color[2];
        CGFloat alpha = color[CGColorGetNumberOfComponents(aColor.CGColor) - 1];
        CGContextSetRGBFillColor(aCtx,red, green, blue, alpha);
    }
    
	/* Draw a line segment in the context */
	CGContextBeginPath(aCtx);
	CGContextMoveToPoint(aCtx, aLeft, aTop);
	CGContextAddLineToPoint(aCtx, aLeft + aWidth, aTop);
	CGContextAddLineToPoint(aCtx, aLeft + aWidth, aTop + aHeight);
	CGContextAddLineToPoint(aCtx, aLeft, aTop + aHeight);
	CGContextAddLineToPoint(aCtx, aLeft, aTop);
	CGContextStrokePath(aCtx);
}

-(void)drawRoundRect:(CGContextRef)aCtx leftX:(int)aLeft topY:(int)aTop width:(int)aWidth height:(int)aHeight strokeWidth:(int)aStrokeWidth Color:(UIColor*)aColor
{
	float radius = 10.0f;	
	CGRect rect = CGRectMake(aLeft, aTop, aWidth, aHeight);	
	CGContextSetLineWidth(aCtx, aStrokeWidth);
	CGContextSetLineCap(aCtx, kCGLineCapRound);
    if (nil != aColor) {
        const CGFloat *color = CGColorGetComponents(aColor.CGColor);  
        CGFloat red = color[0]; 
        CGFloat green = color[1];
        CGFloat blue = color[2];
        CGFloat alpha = color[CGColorGetNumberOfComponents(aColor.CGColor) - 1];
        CGContextSetRGBFillColor(aCtx,red, green, blue, alpha);
    }
    
	CGContextMoveToPoint(aCtx, rect.origin.x, rect.origin.y + radius);	
	CGContextAddLineToPoint(aCtx, rect.origin.x, rect.origin.y + rect.size.height - radius);	
	CGContextAddArc(aCtx, rect.origin.x + radius, rect.origin.y + rect.size.height - radius,					
					radius, M_PI, M_PI / 2, 1);
	CGContextAddLineToPoint(aCtx, rect.origin.x + rect.size.width - radius,							
							rect.origin.y + rect.size.height);
	CGContextAddArc(aCtx, rect.origin.x + rect.size.width - radius,					
					rect.origin.y + rect.size.height - radius, radius, M_PI / 2, 0.0f, 1);
	CGContextAddLineToPoint(aCtx, rect.origin.x + rect.size.width, rect.origin.y + radius);	
	CGContextAddArc(aCtx, rect.origin.x + rect.size.width - radius, rect.origin.y + radius,					
					radius, 0.0f, -M_PI / 2, 1);
	CGContextAddLineToPoint(aCtx, rect.origin.x + radius, rect.origin.y);	
	CGContextAddArc(aCtx, rect.origin.x + radius, rect.origin.y + radius, radius,					
					-M_PI / 2, M_PI, 1);
	CGContextStrokePath(aCtx);
}

-(void)drawEllipse:(CGContextRef)aCtx leftX:(int)aLeft topY:(int)aTop width:(int)aWidth height:(int)aHeight strokeWidth:(int)aStrokeWidth Color:(UIColor*)aColor
{
    CGContextSetLineWidth(aCtx, aStrokeWidth);
	CGContextSetLineCap(aCtx, kCGLineCapRound);
    if (nil != aColor) {
        const CGFloat *color = CGColorGetComponents(aColor.CGColor);  
        CGFloat red = color[0]; 
        CGFloat green = color[1];
        CGFloat blue = color[2];
        CGFloat alpha = color[CGColorGetNumberOfComponents(aColor.CGColor) - 1];
        CGContextSetRGBFillColor(aCtx,red, green, blue, alpha);
    }
    
    CGContextStrokeEllipseInRect (aCtx, CGRectMake(aLeft, aTop, aWidth, aHeight));
}

-(void)fillRect:(CGContextRef)aCtx leftX:(int)aLeft topY:(int)aTop width:(int)aWidth height:(int)aHeight Color:(UIColor*)aColor
{
    if (nil != aColor) {
        const CGFloat *color = CGColorGetComponents(aColor.CGColor);  
        CGFloat red = color[0]; 
        CGFloat green = color[1];
        CGFloat blue = color[2];
        CGFloat alpha = color[CGColorGetNumberOfComponents(aColor.CGColor) - 1];
        CGContextSetRGBFillColor(aCtx, red, green, blue, alpha);
    }
    CGContextFillRect(aCtx, CGRectMake(aLeft, aTop, aWidth, aHeight));
}

-(void)fillRoundRect:(CGContextRef)aCtx leftX:(int)aLeft topY:(int)aTop width:(int)aWidth height:(int)aHeight Color:(UIColor*)aColor
{
	float radius = 10.0f;
	CGRect rect = CGRectMake(aLeft, aTop, aWidth, aHeight);
    if (nil != aColor) {
        const CGFloat *color = CGColorGetComponents(aColor.CGColor);  
        CGFloat red = color[0]; 
        CGFloat green = color[1];
        CGFloat blue = color[2];
        CGFloat alpha = color[CGColorGetNumberOfComponents(aColor.CGColor) - 1];
        CGContextSetRGBFillColor(aCtx, red, green, blue, alpha);
    }	
    
	CGContextMoveToPoint(aCtx, rect.origin.x, rect.origin.y + radius);	
	CGContextAddLineToPoint(aCtx, rect.origin.x, rect.origin.y + rect.size.height - radius);
	CGContextAddArc(aCtx, rect.origin.x + radius, rect.origin.y + rect.size.height - radius,					
					radius, M_PI, M_PI / 2, 1);
	CGContextAddLineToPoint(aCtx, rect.origin.x + rect.size.width - radius,							
							rect.origin.y + rect.size.height);
	CGContextAddArc(aCtx, rect.origin.x + rect.size.width - radius,					
					rect.origin.y + rect.size.height - radius, radius, M_PI / 2, 0.0f, 1);
	CGContextAddLineToPoint(aCtx, rect.origin.x + rect.size.width, rect.origin.y + radius);
	CGContextAddArc(aCtx, rect.origin.x + rect.size.width - radius, rect.origin.y + radius,					
					radius, 0.0f, -M_PI / 2, 1);
	CGContextAddLineToPoint(aCtx, rect.origin.x + radius, rect.origin.y);	
	CGContextAddArc(aCtx, rect.origin.x + radius, rect.origin.y + radius, radius,					
					-M_PI / 2, M_PI, 1);
	CGContextFillPath(aCtx);
}

-(void)fillEllipse:(CGContextRef)aCtx leftX:(int)aLeft topY:(int)aTop width:(int)aWidth height:(int)aHeight Color:(UIColor*)aColor
{
    if (nil != aColor) {
        const CGFloat *color = CGColorGetComponents(aColor.CGColor);  
        CGFloat red = color[0]; 
        CGFloat green = color[1];
        CGFloat blue = color[2];
        CGFloat alpha = color[CGColorGetNumberOfComponents(aColor.CGColor) - 1];
        CGContextSetRGBFillColor(aCtx, red, green, blue, alpha);
    }
    CGContextFillEllipseInRect(aCtx, CGRectMake(aLeft, aTop, aWidth, aHeight));
}

-(CGRect)calcTextRect:(CGContextRef)aCtx Text:(NSString*)aText WithFont:(UIFont*)aFont
{
    UIGraphicsPushContext(aCtx);
    CGContextSaveGState(aCtx);
    
    CGSize size;
    
    if(NULL != aFont) {
        size = [aText sizeWithFont:aFont];
	} else {
        size = [aText sizeWithFont:[UIFont systemFontOfSize:[UIFont systemFontSize]]];
	}
    CGRect rect;
    rect.origin.x = 0;
    rect.origin.y = 0;
    rect.size.width  = size.width;
    rect.size.height = size.height;
    
    CGContextRestoreGState(aCtx);
    UIGraphicsPopContext();
    return rect;
}

-(QuickNoteDelegate*) appDelegate
{
    return ((QuickNoteDelegate*)[[UIApplication sharedApplication]  delegate]);
}

-(int)maxCountOfNotes
{
    return 30;
}

@end
