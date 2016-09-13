
#import "Note.h"

@implementation Note

@synthesize iID, iSequence, iNeedUpdate, iLocked, iLoaded;

-(id) init
{
    self = [super init];
    if (self) {
        iID = -1;
        iTitle = nil;
        iTag = nil;
        iDate = nil; 
        iContent = nil;
        iLocked = NO;
        iNeedUpdate = NO;
        iLoaded = NO; 
        iSequence = -1;
    }
    return self;
}

- (NSComparisonResult)compare:(Note*)aNote
{
	NSComparisonResult ret;
	ret = [aNote.iDate compare:self.iDate];
	if(NSOrderedSame == ret)
		ret = [self.iTitle caseInsensitiveCompare:aNote.iTitle];
	return ret;
}

- (NSComparisonResult)compareByTitleA2Z:(Note*)aNote
{
	NSComparisonResult ret;
	ret = [self.iTitle caseInsensitiveCompare:aNote.iTitle];
	return ret;
}

- (NSComparisonResult)compareByTitleZ2A:(Note*)aNote
{
	NSComparisonResult ret;
	ret = [aNote.iTitle caseInsensitiveCompare:self.iTitle];
	return ret;
}

- (NSComparisonResult)compareByDateNewestFirst:(Note*)aNote
{
	NSComparisonResult ret;
	ret = [aNote.iDate compare:self.iDate];
	return ret;
}

- (NSComparisonResult)compareByDateOldestFirst:(Note*)aNote
{
	NSComparisonResult ret;
	ret = [self.iDate compare:aNote.iDate];
	return ret;
}

- (NSComparisonResult)compareByLockedFirst:(Note*)aNote
{
	NSComparisonResult ret;
	ret = (NSComparisonResult)(aNote.iLocked - self.iLocked);
	return ret;
}

- (NSComparisonResult)compareByUnlockedFirst:(Note*)aNote
{
	NSComparisonResult ret;
	ret = (NSComparisonResult)(self.iLocked - aNote.iLocked);
	return ret;
}

#pragma mark Properties
//set and get method,
//don't change any code if you don't know what you are doing exactly.
- (NSInteger)iID 
{
    return iID;
}

- (NSString *)iTitle 
{
    return iTitle;
}

- (void)setITitle:(NSString *)aString 
{
    if ((aString == nil) || ([aString length] == 0)) {
        iTitle = @"";
        return;
    }
    
    if ((iTitle && aString) && ([iTitle isEqualToString:aString])) return;
    iNeedUpdate = YES;
    [iTitle release];
    iTitle = [aString copy];
}

- (NSString *)iContent 
{
    return iContent;
}

- (void)setIContent:(NSString *)aString 
{
    if ((!iContent && !aString) || (iContent && aString && [iContent isEqualToString:aString])) return;
    iNeedUpdate = YES;
    [iContent release];
    iContent = [aString copy];
}

- (NSString *)iTag 
{
    return iTag;
}

- (void)setITag:(NSString *)aString 
{
    if ((!aString) || (iTag && aString && [iTag isEqualToString:aString])) return;
    iNeedUpdate = YES;
    [iTag release];
    iTag = [aString copy];
}

- (NSDate *)iDate 
{
    return iDate;
}

- (void)setIDate:(NSDate *)aDate 
{
    if (aDate == nil) return;
    iNeedUpdate = YES;
    [iDate release];
    iDate = [aDate copy];
}

- (void)dealloc
{
    [iTitle release];
    [iContent release];
    [iDate release];
    [iTag release];
    
    [super dealloc];
}

@end
