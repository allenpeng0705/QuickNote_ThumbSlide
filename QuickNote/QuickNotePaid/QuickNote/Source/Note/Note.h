#import <sqlite3.h>

@interface Note : NSObject 
{
    NSInteger   iID;
    NSInteger   iSequence;
    NSString    *iTitle;
    NSDate      *iDate;    
    NSString    *iContent;
    NSString    *iTag;

    BOOL        iLoaded; 
    BOOL        iNeedUpdate; 
	BOOL        iLocked;
}

@property (assign, nonatomic) NSInteger iID;
@property (assign, nonatomic) NSInteger iSequence;

@property (nonatomic, copy) NSString *iTitle;
@property (nonatomic, copy) NSString *iTag;
@property (nonatomic, copy) NSDate   *iDate;
@property (nonatomic, copy) NSString *iContent;
@property BOOL iNeedUpdate;
@property BOOL iLocked;
@property BOOL iLoaded;

- (NSComparisonResult)compare:(Note*)aNote;

@end
