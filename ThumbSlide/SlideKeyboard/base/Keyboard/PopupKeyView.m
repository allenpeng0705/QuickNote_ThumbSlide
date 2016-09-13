
#import "PopupKeyView.h"
#import "Util.h"

@implementation PopupKeyView
@synthesize iContent;
@synthesize soundFileURLRef;
@synthesize soundFileObject;

- (id)init
{
    self = [super init];
    if (self) {
        iContent = nil;
        self.opaque = NO;
        
        // Create the URL for the source audio file. The URLForResource:withExtension: method is
        //    new in iOS 4.0.
        NSURL *tapSound   = [[NSBundle mainBundle] URLForResource: @"tap"
                                                    withExtension: @"aif"];
        
        // Store the URL as a CFURLRef instance
        self.soundFileURLRef = (CFURLRef) [tapSound retain];
        AudioServicesCreateSystemSoundID (                                          
                                          soundFileURLRef,
                                          &soundFileObject
                                          ); 
    }
    return self;
}


- (void)drawRect:(CGRect)rect
{
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    
    if (iContent != nil) {
        UIColor* fillcolor = [UIColor blueColor];
        [[Util sharedInstance] fillRoundRect:ctx leftX:0 topY:0 
                                  width:self.frame.size.width height:self.frame.size.height Color:fillcolor];
        
        UIColor* textcolor = [UIColor colorWithRed:(CGFloat)1.0 green:(CGFloat)1.0 blue:1.0 alpha:1.0];
        CGRect rect = CGRectMake(0, 0, self.frame.size.width, self.frame.size.height);
        
        int fontSize = 30;
        CGRect txtRect = [[Util sharedInstance] calcTextRect:ctx Text:iContent WithFont:[UIFont systemFontOfSize:fontSize]];
        
        while (txtRect.size.width > rect.size.width) {
            txtRect = [[Util sharedInstance] calcTextRect:ctx Text:iContent WithFont:[UIFont systemFontOfSize:--fontSize]];
        }        
        
        [[Util sharedInstance] drawText:ctx InRect:rect WithContent:iContent 
                                   Font:[UIFont systemFontOfSize:fontSize] Color:textcolor];
                [self playSound];
    }
}

- (void)dealloc
{
    [iContent release];
    AudioServicesDisposeSystemSoundID (soundFileObject);
    CFRelease (soundFileURLRef);
    [super dealloc];
}

-(void)showTextWithWidth:(CGFloat)aWidth Height:(CGFloat)aHeight Text:(NSString*)aText WithSound:(BOOL)aFlag
{
    self.iContent = aText;
    if (aFlag == YES) {
        
    }
    [self setNeedsDisplay];
}

-(void)playSound
{  
    // Create a system sound object representing the sound file.
    BOOL play = NO;
    NSNumber* result = [[NSUserDefaults standardUserDefaults] objectForKey:@"KeyboardAudio"];
	if (result)
	{
		play = [result boolValue];
	}
    if (play) {
        AudioServicesPlaySystemSound (soundFileObject);
    }
    //[self playFileInSys:@"tap.aif" vibration:NO];
}

-(void) playFileInSys:(NSString*) filename vibration:(BOOL)vibration
{
    /*
    NSString* f = [filename stringByDeletingPathExtension];
    NSString* ext = [filename pathExtension];
    
    CFBundleRef mainBundle = CFBundleGetMainBundle ();
    CFURLRef soundFileURLRef  =	CFBundleCopyResourceURL (mainBundle,(CFStringRef)f,(CFStringRef)ext,nil);
    SystemSoundID soundFileObject;
    AudioServicesCreateSystemSoundID (soundFileURLRef,&soundFileObject);
    AudioServicesPlaySystemSound(soundFileObject);
    CFRelease (soundFileURLRef);
//    AudioServicesDisposeSystemSoundID(soundFileObject);
     */
}


-(void)hide
{
    [self removeFromSuperview];
}

@end
