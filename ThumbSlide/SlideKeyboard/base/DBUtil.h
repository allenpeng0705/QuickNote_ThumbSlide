#import <Foundation/Foundation.h>
#import <sqlite3.h>

@interface DBUtil : NSObject {
    sqlite3     *iDB;
    NSMutableArray     *iUserWords;
    NSMutableArray     *iActiveWords;
    BOOL iUserWordsLoaded;
    BOOL iActiveWordsLoaded;
}

+(DBUtil*)sharedInstance;
+ (void)destroy;

-(void)loadUserWords;
-(void)loadActiveWords;
-(void)addUserWord:(NSString*)aWord;
-(void)addActiveWord:(NSString*)aWord;
-(void)storeUserWords;
-(void)storeActiveWords;

@end
