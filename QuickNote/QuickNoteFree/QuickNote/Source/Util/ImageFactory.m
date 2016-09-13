#import "ImageFactory.h"

static ImageFactory* _instance = NULL;

@implementation ImageFactory

+ (ImageFactory *)sharedInstance
{
    NSAutoreleasePool *thePool = [[NSAutoreleasePool alloc] init];
    
    @synchronized(@"ImageFactory.instance") {
        if (_instance == NULL) {
            _instance = [[ImageFactory alloc] init];
        }
    }
    
    [thePool release];
    
    return(_instance);
}

-(id)init
{
    self = [super init];
    iImageDictionary = [[NSMutableDictionary alloc] init];
    return self;
}

- (void)dealloc 
{
    [iImageDictionary removeAllObjects];
    [iImageDictionary release];
	[super dealloc];
}

-(UIImage*)ImageWithName:(NSString*)aName
{
    UIImage* ret = [iImageDictionary objectForKey:aName];
    if (ret == nil) {
        ret = [UIImage imageNamed:aName];
        if (ret == nil) return nil;
        [iImageDictionary setObject:ret forKey:aName];
    }
    return ret;
}

-(UIImage*)BackgroundOfTextView
{
    UIImage* ret = [iImageDictionary objectForKey:@"writing-base.png"];
    if (ret == nil) {
        ret = [UIImage imageNamed:@"writing-base.png"];
        if (ret == nil) return nil;
        [iImageDictionary setObject:ret forKey:@"writing-base.png"];
    }
    return ret;
}

-(UIImage*)BgOfPortraitCandidatesList
{
    UIImage* ret = [iImageDictionary objectForKey:@"qwerty_320_X_240_loose_AUI.png"];
    if (ret == nil) {
        ret = [UIImage imageNamed:@"qwerty_320_X_240_loose_AUI.png"];
        if (ret == nil) return nil;
        [iImageDictionary setObject:ret forKey:@"qwerty_320_X_240_loose_AUI.png"];
    }
    return ret;
}

-(UIImage*)HighlightOfPortraitCandidatesList
{
    UIImage* ret = [iImageDictionary objectForKey:@"qwerty_320_X_240_loose_apple-AUI-hilite.png"];
    if (ret == nil) {
        ret = [UIImage imageNamed:@"qwerty_320_X_240_loose_apple-AUI-hilite.png"];
        if (ret == nil) return nil;
        [iImageDictionary setObject:ret forKey:@"qwerty_320_X_240_loose_apple-AUI-hilite.png"];
    }
    return ret;    
}

-(UIImage*)LineDividerOfTextView
{
    UIImage* ret = [iImageDictionary objectForKey:@"writing-line.png"];
    if (ret == nil) {
        ret = [UIImage imageNamed:@"writing-line.png"];
        if (ret == nil) return nil;
        [iImageDictionary setObject:ret forKey:@"writing-line.png"];
    }
    return ret;   
}

-(UIImage*)SelectionOfTextView
{
    UIImage* ret = [iImageDictionary objectForKey:@"writing-hilite.png"];
    if (ret == nil) {
        ret = [UIImage imageNamed:@"writing-hilite.png"];
        if (ret == nil) return nil;
        [iImageDictionary setObject:ret forKey:@"writing-hilite.png"];
    }
    return ret;   
}

-(UIImage*)BackgroundOfToolbar
{
    UIImage* ret = [iImageDictionary objectForKey:@"iphone-DOC-49x320.png"];
    if (ret == nil) {
        ret = [UIImage imageNamed:@"iphone-DOC-49x320.png"];
        if (ret == nil) return nil;
        [iImageDictionary setObject:ret forKey:@"iphone-DOC-49x320.png"];
    }
    return ret; 
}



@end
