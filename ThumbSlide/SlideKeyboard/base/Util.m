
#import "Util.h"

static Util* _instance = nil;

@implementation Util

+ (Util *)sharedInstance
{
    NSAutoreleasePool *thePool = [[NSAutoreleasePool alloc] init];
    
    @synchronized(@"Util.instance") {
        if (_instance == nil) {
            _instance = [[Util alloc] init];
            
            [_instance SetColorForCandidate:[UIColor colorWithRed:(CGFloat)0xff/0xff 
                                                                green:(CGFloat)0xff/0xff blue:(CGFloat)0xff/0xff alpha:0xff/0xff]];
            [_instance SetColorForSelectedCandidate:[UIColor colorWithRed:(CGFloat)0x00/0xff 
                                                                green:(CGFloat)0xCC/0xff blue:(CGFloat)0x00/0xff alpha:0xff/0xff]];
            [_instance SetColorForExactCandidate:[UIColor colorWithRed:(CGFloat)0x00/0xff 
                                                                green:(CGFloat)0xff/0xff blue:(CGFloat)0xff/0xff alpha:0xff/0xff]];
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
    [iFontOfCandidatesList release];
    [iDateFormatter release];
    
	[super dealloc];
}

-(UIFont*)FontOfCandidatesList
{
    if (iFontOfCandidatesList != nil) return iFontOfCandidatesList;
    iFontOfCandidatesList = [UIFont systemFontOfSize:24];
    [iFontOfCandidatesList retain];
    return iFontOfCandidatesList;    
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
        NSDictionary *attributes = @{NSFontAttributeName: aFont, NSForegroundColorAttributeName: aColor};
        [aText drawAtPoint:CGPointMake(x, y) withAttributes:attributes];
    } else {
        NSDictionary *attributes = @{NSFontAttributeName: [UIFont systemFontOfSize:[UIFont systemFontSize]], NSForegroundColorAttributeName: aColor};
        [aText drawAtPoint:CGPointMake(x, y) withAttributes:attributes];
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
    NSDictionary *fontattributes = @{NSFontAttributeName: aFont};
    CGSize txtSize = [aText sizeWithAttributes:fontattributes];
    int x, y, w, h;
    x = aRect.origin.x;
    y = aRect.origin.y;
    w = aRect.size.width;
    h = aRect.size.height;
    
    CGPoint startPoint =CGPointMake(x + (w - txtSize.width)/2, y + (h - txtSize.height)/2);
    CGRect rect;
    rect.origin.x = startPoint.x;
    rect.origin.y = startPoint.y;
    rect.size.width = w;
    rect.size.height = h;
    
    NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
    paragraphStyle.lineBreakMode = NSLineBreakByWordWrapping;
    NSDictionary *attributes = @{NSFontAttributeName: aFont, NSParagraphStyleAttributeName: paragraphStyle, NSForegroundColorAttributeName: aColor};
    [aText drawInRect:rect withAttributes:attributes];
    [paragraphStyle release];
    
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
        NSDictionary *fontattributes = @{NSFontAttributeName: aFont};
        size = [aText sizeWithAttributes:fontattributes];
	} else {
        NSDictionary *fontattributes = @{NSFontAttributeName: [UIFont systemFontOfSize:[UIFont systemFontSize]]};
        size = [aText sizeWithAttributes:fontattributes];
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

@end
