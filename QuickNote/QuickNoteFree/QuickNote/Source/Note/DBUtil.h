#import <Foundation/Foundation.h>
#import <sqlite3.h>
#import "Note.h"

@interface DBUtil : NSObject {
    sqlite3     *iDB;
    NSMutableArray     *iNotes;
    NSMutableArray     *iSearchResult;
    NSMutableArray     *iUserWords;
    NSMutableArray     *iActiveWords;
    Note* iCurrentNote;
    BOOL iUserWordsLoaded;
    BOOL iActiveWordsLoaded;
}

@property (nonatomic, retain) Note *iCurrentNote;
@property (nonatomic, retain) NSMutableArray *iNotes;
@property (nonatomic, retain) NSMutableArray *iSearchResult;

+(DBUtil*)sharedInstance;
+ (void)destroy;

- (void)loadNotes;
- (void)createNote;
- (void)deleteNote:(Note*)aNote;
- (void)saveNote:(Note*)aNote;
- (void)saveAllNotes;

-(void)loadUserWords;
-(void)loadActiveWords;
-(void)addUserWord:(NSString*)aWord;
-(void)addActiveWord:(NSString*)aWord;
-(void)storeUserWords;
-(void)storeActiveWords;

@end
