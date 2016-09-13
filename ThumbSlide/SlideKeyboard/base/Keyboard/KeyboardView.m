#import "KeyboardView.h"
#import "CandidatesListView.h"
//#import "ImageFactory.h"
#import "Util.h"
#import "IMESingleton.h"

#import "q_keyboard.h"
#import "DBUtil.h"
#import <stdarg.h>
#import "KeyboardViewController.h"
#import "CommonKeyboardBridge.h"

static void drawLine(CGContextRef aCtx, int32 aPx, int32 aPy, int32 aQx, int32 aQy, int aWidth, const CGColorRef aColor);
static void drawPolygon(CGContextRef aCtx, const CGColorRef aColor, 
                        int aWidth, int aCount, ...);
static void drawBackspaceKey(CGContextRef aCtx, int32 x, int32 y, int32 w, int32 h, CGColorRef aColor);
static void drawEnterKey(CGContextRef aCtx, int32 x, int32 y, int32 w, int32 h, CGColorRef aColor);
static void drawShiftKey(CGContextRef aCtx, int32 x, int32 y, int32 w, int32 h, CGColorRef aColor);
static void drawSpaceKey(CGContextRef aCtx, int32 x, int32 y, int32 w, int32 h, CGColorRef aColor);

void drawLine(CGContextRef aCtx, int aPx, int aPy, int aQx, int aQy, int aWidth, const CGColorRef aColor)
{
    /* Set the line property*/
    CGContextSetLineWidth(aCtx, aWidth);
	CGContextSetLineCap(aCtx, kCGLineCapRound);
    CGContextSetStrokeColorWithColor(aCtx, aColor);
    
	/* Draw a line segment in the context */
	CGContextBeginPath(aCtx);
	CGContextMoveToPoint(aCtx, aPx, aPy);
	CGContextAddLineToPoint(aCtx, aQx, aQy);
	CGContextStrokePath(aCtx);
}

void drawPolygon(CGContextRef aCtx, CGColorRef aColor, int aWidth, int aCount, ...)
{
    CGContextSetLineWidth(aCtx, aWidth);
	CGContextSetLineCap(aCtx, kCGLineCapRound);
	CGContextSetStrokeColorWithColor(aCtx, aColor);
    
    va_list params;
	va_start(params, aCount);
	int x,y;
	CGContextBeginPath(aCtx);
    x = va_arg(params, int);
    y = va_arg(params, int);
	CGContextMoveToPoint(aCtx, x, y);
    for(int i=1; i<aCount; i++){
        x = va_arg(params, int);
        y = va_arg(params, int);
        CGContextAddLineToPoint(aCtx, x, y);
    }
	CGContextStrokePath(aCtx);
    va_end(params);
}

void drawBackspaceKey(CGContextRef aCtx, int32 x, int32 y, int32 w, int32 h, CGColorRef aColor)
{
	drawPolygon(aCtx, aColor, 1, 8, x + w/4, y + h/2, 
                x + w/2, y + 4*h/12, x + w/2, y + 5*h/12, x + 3*w/4, y + 5*h/12, x + 3*w/4, y + 7*h/12,
                x + w/2, y + 7*h/12, x + w/2, y + 8*h/12, x + w/4, y + h/2);     	
	
}

void drawEnterKey(CGContextRef aCtx, int32 x, int32 y, int32 w, int32 h, CGColorRef aColor)
{
	drawLine(aCtx, x + 7*w/8, y + 3*h/8, x + 7*w/8, y + h/2, 1, aColor);
	drawLine(aCtx, x + 3*w/8, y + h/2, x + 7*w/8, y + h/2, 1, aColor);
	drawPolygon(aCtx, aColor, 1, 4, x + w/8, y + h/2, 
								  x + 3*w/8, y + 3*h/8, x + 3*w/8, y + 5*h/8, x + w/8, y + h/2);
}


void drawShiftKey(CGContextRef aCtx, int32 x, int32 y, int32 w, int32 h, CGColorRef aColor)
{
	drawPolygon(aCtx, aColor, 1, 8, x + w/2, y + h/4,
                x + 17*w/24, y + h/2, x+7*w/12, y + h/2, x + 7*w/12, y + 17*h/24, x + 5*w/12, y + 17*h/24,
                x + 5*w/12, y + h/2, x + 7*w/24, y + h/2, x + w/2, y + h/4);	
}

void drawSpaceKey(CGContextRef aCtx, int32 x, int32 y, int32 w, int32 h, CGColorRef aColor)
{
    drawLine(aCtx, w/4 + x, y + h/2, w/4 + x, y + h*3/4, 1, aColor);
	drawLine(aCtx, w/4 + x, y + h*3/4, x + w*3/4, y + h*3/4, 1, aColor);
	drawLine(aCtx, w*3/4 + x, y + h/2, w*3/4 + x, y + h*3/4, 1, aColor);
}


@implementation KeyboardView
@synthesize iWidthRatio;
@synthesize iHeightRatio;

-(TraceKeyboardHandler*) handler
{
    return iHandler;
}

-(void)dealloc
{
    if (iBackgroundLayerRef != nil) {
        CGLayerRelease(iBackgroundLayerRef);
        iBackgroundLayerRef = nil;
    }
    
    if ([[self subviews] containsObject:iPreview]) {
        [iPreview removeFromSuperview];
    }
    [iPreview release];
    
    [iHandler.iKeyboardBridge release];
    [iHandler release];
    [super dealloc];
}

- (void) changeKeyboard
{
    IME* ime = [[IMESingleton sharedInstance] instance];
    Keyboard* currentKB = CurrentKeyboard(ime);
    if (iKeyboard != currentKB) {
        iCurrTouch = nil;
        iKeyboard = currentKB;
        iHandler.iKeyboard = iKeyboard;
        [self clearBackground];
        [self setNeedsDisplay];
    }
}

-(void)attachKeyboard:(Keyboard*)aKeyboard AndKeyboardOutputListener:(id<KeyboardOutputListener>)aListener
{
    if (aKeyboard == nil) return;
    if (aKeyboard == iKeyboard) return;
    iKeyboard = aKeyboard;
    int32 kb_width = 0;
    int32 kb_height = 0;
    SizeOfKeyboard(iKeyboard, &kb_width, &kb_height);
    iWidthRatio = (self.frame.size.width*100)/kb_width;
    iHeightRatio = (self.frame.size.height*100)/kb_height;
    
    //Create different keyboard handler according to the different keyboard type
    iCurrTouch = nil;
    [iHandler release];
    iHandler = [[TraceKeyboardHandler alloc] init];
    iHandler.iKeyboard = iKeyboard;
    iHandler.iKeyboardInteractionListener = self;
    iHandler.iKeyboardOutputListener = aListener;
    [iHandler.iKeyboardBridge release];
    iHandler.iKeyboardBridge = [[CommonKeyboardBridge alloc] init];
    
    [[DBUtil sharedInstance] loadUserWords];
    [[DBUtil sharedInstance] loadActiveWords];
    
    [self setNeedsDisplay];
}

-(void)setFrame:(CGRect)frame
{
    [super setFrame:frame];
    [self clearBackground];
    [self setNeedsDisplay];
}

- (void)drawRect:(CGRect)clip
{
    if (iMainGraphicCtx == nil) {
        iMainGraphicCtx = UIGraphicsGetCurrentContext();
    }
    
    if (iBackgroundLayerRef == nil) {
        CGSize size;
        int32 width, height;
        SizeOfKeyboard(iKeyboard, &width, &height);
        size.width = (width*iWidthRatio)/100;
        size.height = (height*iHeightRatio)/100;
        
        iBackgroundLayerRef = CGLayerCreateWithContext (
                                                        iMainGraphicCtx,
                                                        size,
                                                        NULL);
        CGContextRef ctx = CGLayerGetContext(iBackgroundLayerRef);
        UIGraphicsPushContext(ctx);
        [self drawKeyboard:ctx];
        UIGraphicsPopContext(); 
    }
    
    CGContextDrawLayerInRect(iMainGraphicCtx, clip, iBackgroundLayerRef);
    if (iTracePointIndex >= 5) {
        int i = 0;
        if (iTracePointIndex < 30) {
            i = 0;
        } else {
            i = iTracePointIndex - 20;
        }
        CGContextSetLineWidth(iMainGraphicCtx, 4);
        [[UIColor greenColor] set];
        CGContextMoveToPoint(iMainGraphicCtx, iTraceArray[i].x, iTraceArray[i].y);
        for (++i; i < iTracePointIndex; i++) {
            CGContextAddLineToPoint(iMainGraphicCtx, iTraceArray[i].x, iTraceArray[i].y);
        }
        CGContextStrokePath(iMainGraphicCtx);
    }
}

-(void)drawKeyboard:(CGContextRef)aCtx
{
    if (iKeyboard == nil) return;    
    int key_num = NumOfKeys(iKeyboard);
    for (int i = 0; i < key_num; i++) {
        Key* key = KeyAtIndex(iKeyboard, i);
        [self drawKey:aCtx Key:key WithShiftState:iShiftModeOn];
    }
}

-(void)drawKey:(CGContextRef)aCtx Key:(Key*)aKey WithShiftState:(BOOL)aShiftState
{
    if (aKey == nil) return;
    int32 posX, posY, width, height;
    float32 adjustPosX, adjustPosY, adjustWidth, adjustHeight;
    PositionOfKey(aKey, &posX, &posY);
    SizeOfKey(aKey, &width, &height);
    
    adjustPosX = (posX*iWidthRatio)/100;
    adjustPosY = (posY*iHeightRatio)/100;
    adjustWidth = (width*iWidthRatio)/100;
    adjustHeight = (height*iHeightRatio)/100;
        
    CGRect rect;
    rect.origin.x = adjustPosX + 5;
    rect.origin.y = adjustPosY + 5;
    rect.size.width = adjustWidth - 8;
    rect.size.height = adjustHeight - 8;
    
    
    NSString* keyLabel = nil;
    
    char* label = NULL;
    int fontSize = 0;
    if (aShiftState == NO) {
        label = LabelOfKey(aKey);
        if (label != NULL) {
            keyLabel = [NSString stringWithCString:label encoding:NSUTF8StringEncoding];
        }
        fontSize = 28;
    } else {
        label = ShiftLabelOfKey(aKey);
        if (label != NULL) {
            keyLabel = [NSString stringWithCString:label encoding:NSUTF8StringEncoding];
            fontSize = 24;
        } else {
            label = LabelOfKey(aKey);
            keyLabel = [NSString stringWithCString:label encoding:NSUTF8StringEncoding]; 
            fontSize = 28;
        }
    }
 
    UIColor* color = [UIColor colorWithRed:(CGFloat)1.0 green:(CGFloat)1.0 blue:1.0 alpha:1.0];
    UIColor* fillcolor = [UIColor colorWithRed:(CGFloat)(0.0) green:(CGFloat)(0.0) blue:(CGFloat)(0.0) alpha:1.0];
    [[Util sharedInstance] fillRoundRect:aCtx leftX:(adjustPosX + 3) topY:(adjustPosY + 3) width:(adjustWidth - 5) height:(adjustHeight - 5) Color:fillcolor];
    
    if (TypeOfKey(aKey) == KEY_SYMBOL_BACKSPACE) {
        drawBackspaceKey(aCtx, adjustPosX + 3, adjustPosY + 3, adjustWidth - 5, adjustHeight - 5, [color CGColor]);
    } else if (TypeOfKey(aKey) == KEY_SYMBOL_SHIFT) {
        drawShiftKey(aCtx, adjustPosX + 3, adjustPosY + 3, adjustWidth - 5, adjustHeight - 5, [color CGColor]);
    } else if (TypeOfKey(aKey) == KEY_SYMBOL_ENTER) {
        drawEnterKey(aCtx, adjustPosX + 3, adjustPosY + 3, adjustWidth - 5, adjustHeight - 5, [color CGColor]);
    } else if (TypeOfKey(aKey) == KEY_SYMBOL_SPACE) {
        drawSpaceKey(aCtx, adjustPosX + 3, adjustPosY + 3, adjustWidth - 5, adjustHeight - 5, [color CGColor]);
    } else {    
        if (keyLabel != nil) {
            CGRect txtRect = [[Util sharedInstance] calcTextRect:aCtx Text:keyLabel WithFont:[UIFont systemFontOfSize:fontSize]];
            while (txtRect.size.width > rect.size.width) {
                txtRect = [[Util sharedInstance] calcTextRect:aCtx Text:keyLabel WithFont:[UIFont systemFontOfSize:--fontSize]];
            }
            [[Util sharedInstance] drawText:aCtx InRect:rect WithContent:keyLabel Font:[UIFont systemFontOfSize:fontSize] Color:color]; 
        }
    }

 }

-(void)showPreview:(Key*)aPressedKey WithSound:(BOOL)aFlag;
{
    if (aPressedKey == nil) return;
    if (StatusOfKey(aPressedKey) != KEY_PRESSED) return;
    int32 posX, posY, width, height;
    float32 adjustPosX, adjustPosY, adjustWidth, adjustHeight;
    int kbposX, kbposY, kbWidth, kbHeight;
    float32 adjustKBPosX, adjustKBPosY, adjustKBWidth, adjustKBHeight;
    PositionOfKey(aPressedKey, &posX, &posY);
    SizeOfKey(aPressedKey, &width, &height);
    PositionOfKeyboard(iKeyboard, &kbposX, &kbposY);
    SizeOfKeyboard(iKeyboard, &kbWidth, &kbHeight);
    
    adjustPosX = (posX*iWidthRatio)/100;
    adjustPosY = (posY*iHeightRatio)/100;
    adjustWidth = (width*iWidthRatio)/100;
    adjustHeight = (height*iHeightRatio)/100;
    
    adjustKBPosX = (kbposX*iWidthRatio)/100;
    adjustKBPosY = (kbposY*iHeightRatio)/100;
    adjustKBWidth = (kbWidth*iWidthRatio)/100;
    adjustKBHeight = (kbHeight*iHeightRatio)/100;
    
    CGRect rect;    
    rect.origin.x = adjustPosX + 10;
    rect.origin.y = adjustPosY - (adjustHeight)/2 - 20;
    rect.size.width = 3*adjustWidth/2;
    rect.size.height = adjustHeight;
    
    if (IDOfKey(aPressedKey) == KEY_ID_SPACE) {
        rect.origin.x = adjustKBPosX + 40;
        rect.size.width = adjustKBWidth - adjustKBPosX - 80;
    } else if ((rect.origin.x + rect.size.width) > (adjustKBPosX + adjustKBWidth)) {
        rect.origin.x -= ((rect.origin.x + rect.size.width) - (adjustKBPosX + adjustKBWidth) + 20);
    }    
    NSString* keyLabel = nil;
    
    char* label = NULL;
    BOOL shiftState =  Shifted(iKeyboard);
    
    if (shiftState == NO) {
        label = LabelOfKey(aPressedKey);
        if (label != NULL) {
            keyLabel = [[NSMutableString alloc] initWithUTF8String:label];
        }
    } else {
        label = ShiftLabelOfKey(aPressedKey);
        if (label == NULL) {
            label = LabelOfKey(aPressedKey);
        } 
        keyLabel = [[NSMutableString alloc] initWithUTF8String:label];
    }
    
    if (keyLabel != nil) {
        iPreview.frame = rect;  
        [iPreview showTextWithWidth:adjustWidth Height:adjustHeight Text:keyLabel WithSound:aFlag];
        [self addSubview:iPreview];

    }
    
    [keyLabel release];
}

- (id)initWithFrame:(CGRect)frame 
{	
    if ((self = [super initWithFrame:frame])) {
        self.multipleTouchEnabled = YES;
        iMainGraphicCtx = nil;
        iBackgroundLayerRef = nil;
        
        self.backgroundColor = [UIColor grayColor];
        iPreview = [[PopupKeyView alloc] init];
        
//        UIInterfaceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
//        if (UIInterfaceOrientationIsPortrait(orientation) == YES) {
//            iWidthRatio = 100.0;
//            iHeightRatio = 100.0;
//        } else {
//            iWidthRatio = 150;
//            iHeightRatio = 80;
//        }
        //iWidthRatio = 100.0;
        //iHeightRatio = 100.0;

        return self;
    }
	return nil;
}



-(Keyboard*)keyboard
{
    return iKeyboard;
}

-(void)onChangeToShiftMode:(BOOL)aFlag
{
    iShiftModeOn = aFlag;
    [self clearBackground];
    [self setNeedsDisplay];
}

-(void)onHide
{
    
}

-(void)onRepaint
{
    
}

-(void)clearBackground
{
    if (iBackgroundLayerRef != nil) {
        CGLayerRelease(iBackgroundLayerRef);
        iBackgroundLayerRef = nil;
        iMainGraphicCtx = nil;
    }
    
    [self setNeedsDisplay];
}

- (void) touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
    int32 x,y;
    float32 adjustX, adjustY;
    if (iCurrTouch) {		
        CGPoint location = [iCurrTouch locationInView:self];
        x = location.x;    
        y = location.y;
        iTracePointIndex = 0;
        adjustX = (x*100)/iWidthRatio;
        adjustY = (y*100)/iHeightRatio;
        
        [iHandler onFingerUpAtX:adjustX AndY:adjustY];		
        iCurrTouch = nil;
    }
    
    NSUInteger touchCount = [[event allTouches] count];
    NSArray *touchObjects = [[event allTouches] allObjects];

    int i;
    
    for (i = 0; i < touchCount; ++i) {
        
        UITouch *touch = [touchObjects objectAtIndex:i];
        
        if ([touch phase] != UITouchPhaseBegan) {
            continue;
        }

        iCurrTouch = touch;
        CGPoint	 location = [touch locationInView:touch.view];
        x = location.x;    
        y = location.y;	
        adjustX = (x*100)/iWidthRatio;
        adjustY = (y*100)/iHeightRatio;        
        [iHandler onFingerDownAtX:adjustX AndY:adjustY];
        
        iLastTouchKey = CurrentKey(iKeyboard);
    }
    
    Key* key = CurrentKey(iKeyboard);
    if ((key != NULL) && (StatusOfKey(key) == KEY_PRESSED)) {
        BOOL flag = YES;
        if (SupportRegionCorrection(iKeyboard)) {
//            flag = !([iHandler tracing]);
        }        
        [self showPreview:key WithSound:flag];
    }
    
    [self setNeedsDisplay];
}

- (void) touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
    if (iCurrTouch && [iCurrTouch phase] == UITouchPhaseMoved) {
        int32 x,y;
        float32 adjustX, adjustY;
        CGPoint location = [iCurrTouch locationInView:self];
        x = location.x;    
        y = location.y;	

        iTraceArray[iTracePointIndex++] = location;
        adjustX = (x*100)/iWidthRatio;
        adjustY = (y*100)/iHeightRatio; 
        
        [iHandler onFingerMoveAtX:adjustX AndY:adjustY]; 
        Key* key = CurrentKey(iKeyboard);
        if (key != iLastTouchKey) {        
            [iPreview hide];
            iLastTouchKey = CurrentKey(iKeyboard);
        }
        [self setNeedsDisplay];
    }
}

- (void) touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
    if (iCurrTouch && [iCurrTouch phase] == UITouchPhaseEnded) {
        int32 x,y;
        float32 adjustX, adjustY;
        CGPoint location = [iCurrTouch locationInView:self];
        x = location.x;    
        y = location.y;
        adjustX = (x*100)/iWidthRatio;
        adjustY = (y*100)/iHeightRatio; 
        iCurrTouch = nil;
        iTracePointIndex = 0;
        if ([[self subviews] containsObject:iPreview]) {
            [iPreview hide];
        } 
        
        [iHandler onFingerUpAtX:adjustX AndY:adjustY]; 
        [self setNeedsDisplay];
    }
}

-(void)onChangeKeyboard:(NSString*)aName
{
    IME* ime = [[IMESingleton sharedInstance] instance];
    iKeyboard = CurrentKeyboard(ime);
    iHandler.iKeyboard = iKeyboard;
    [self clearBackground];
    [self setNeedsDisplay];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    if ([[self subviews] containsObject:iPreview]) {
        [iPreview hide];
    }
    
    [iHandler cancelFingerOperation];
}

-(NSUInteger)heightOfKeyboard
{
    int32 width, height;
    SizeOfKeyboard(iKeyboard, &width, &height);
    return (NSUInteger)height;
}

-(NSUInteger)widthOfKeyboard
{
    int32 width, height;
    SizeOfKeyboard(iKeyboard, &width, &height);
    return (NSUInteger)width;    
}

-(NSUInteger)height
{
    return [self frame].size.height;
}

-(NSUInteger)width
{
    return [self frame].size.width;
}

@end
