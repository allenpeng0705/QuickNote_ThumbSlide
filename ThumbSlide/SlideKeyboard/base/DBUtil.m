#import "DBUtil.h"
#import "Util.h"
#import <assert.h>
#import "utf8_string.h"
#import "IMESingleton.h"
#include "q_malloc.h"

static DBUtil* _instance = nil;

@implementation DBUtil

+ (DBUtil *)sharedInstance
{
    NSAutoreleasePool *thePool = [[NSAutoreleasePool alloc] init];
    
    @synchronized(@"NoteDB.instance") {
        if (_instance == nil) {
            _instance = [[DBUtil alloc] init];            
        }
    }
    
    [thePool release];
    
    return(_instance);
}

+ (void)destroy
{
    if (_instance != nil) {
        [_instance release];
    }
}


- (id)init
{
    int success;
    self = [super init];
    if (self) {
        [[Util sharedInstance] makeFileWritable:@"words.db"];
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *docPath = [paths objectAtIndex:0];
        NSString *dbPath = [docPath stringByAppendingPathComponent:@"words.db"];
        
        success = sqlite3_open([dbPath UTF8String], &iDB);
        NSAssert1((success == SQLITE_OK), @"Error: failed to open database with message '%s'.", sqlite3_errmsg(iDB)); 
        
        iUserWords = [[NSMutableArray alloc] init];
        iActiveWords = [[NSMutableArray alloc] init];
        iUserWordsLoaded = NO;
        iActiveWordsLoaded = NO;
    }
    
    return self;
}

- (void)dealloc
{
    if (iDB) {
        int success = sqlite3_close(iDB);
        NSAssert1((success == SQLITE_OK), @"Error: failed to close database with message '%s'.", sqlite3_errmsg(iDB));
    }
    
    [iUserWords removeAllObjects];
    [iUserWords release];
    [iActiveWords removeAllObjects];
    [iActiveWords release];
    [super dealloc];
}

-(void)loadUserWords
{
    if (iUserWordsLoaded == YES) return;
    int success;
	sqlite3_stmt *statement;
	
    success = sqlite3_prepare_v2(iDB, "SELECT WORD FROM user_dict", -1, &statement, NULL);
    NSAssert1((success == SQLITE_OK), @"Error: failed to prepare sentence with message '%s'.", sqlite3_errmsg(iDB));
    char** words;
    words = (char**)q_malloc(1000*sizeof(char*));
    
    for (int count = 0; count < 100; count++) {
        words[count] = q_malloc(100);
    }
    
    int i = 0;
    
    while (sqlite3_step(statement) == SQLITE_ROW) {        
        char* tmp = (char *)sqlite3_column_text(statement, 0);
        utf8_strcpy(words[i], tmp);
        i++;
    }
    
    IME* ime = [[IMESingleton sharedInstance] instance];
    Filter* filter = currentFilter(ime);
    if (i > 1) {
        addWords(filter, (const char**)words, i);
    } else {
        addWord(filter, words[0]);
    }
    
    sqlite3_finalize(statement); 
    
    for (int count = 0; count < 100; count++) {
        q_free(words[count]);
    }
    q_free(words);
    iUserWordsLoaded = YES;
}

-(void)loadActiveWords
{
    if (iActiveWordsLoaded == YES) return;
    int success;
	sqlite3_stmt *statement;
	
    success = sqlite3_prepare_v2(iDB, "SELECT WORD FROM active_dict", -1, &statement, NULL);
    NSAssert1((success == SQLITE_OK), @"Error: failed to prepare sentence with message '%s'.", sqlite3_errmsg(iDB));
    IME* ime = [[IMESingleton sharedInstance] instance];
    Filter* filter = currentFilter(ime);
    
    while (sqlite3_step(statement) == SQLITE_ROW) {        
        const char* tmp = (char *)sqlite3_column_text(statement, 0);
        if (isActiveWord(filter, tmp) == FALSE) {
            setWordActive(filter, tmp, TRUE);
        }
    }
    
    sqlite3_finalize(statement);  
    iActiveWordsLoaded = YES;
} 

-(void)addUserWord:(NSString*)aWord
{
    if ((aWord == nil) || ([aWord length] == 0)) return;
    const char* word = [aWord UTF8String];
    IME* ime = [[IMESingleton sharedInstance] instance];
    Filter* filter = currentFilter(ime);
    if (wordExisted(filter, word) == FALSE) {
        addWord(filter, word);
        NSString* tmpWord = [NSString stringWithCString:word encoding:NSUTF8StringEncoding];
        [iUserWords addObject:tmpWord];
        return;
    }

}

-(void)addActiveWord:(NSString*)aWord
{
    if ((aWord == nil) || ([aWord length] == 0)) return;
    const char* word = [aWord UTF8String];
    IME* ime = [[IMESingleton sharedInstance] instance];
    Filter* filter = currentFilter(ime);
    if (isActiveWord(filter, word) == FALSE) {
        setWordActive(filter, word, TRUE);
        NSString* tmpWord = [NSString stringWithCString:word encoding:NSUTF8StringEncoding];
        [iActiveWords addObject:tmpWord];
    }    
}

-(void)storeUserWords
{
    int success;
    sqlite3_stmt *statement;
    NSDate* now;
    now = [NSDate date];
    
    int n = [iUserWords count];
    for (int i = 0; i < n; i++) {
        success = sqlite3_prepare_v2(iDB, "INSERT INTO user_dict (WORD,COUNT,ADD_DATE,LAST_SELECTED_DATE) VALUES(?,?,?,?)", -1, &statement, NULL);
        NSAssert1((success == SQLITE_OK), @"Error: failed to prepare statement for creating empty note with message '%s'.", sqlite3_errmsg(iDB));
        NSString* word = (NSString*)[iUserWords objectAtIndex:i];
        const char* newWord = [word cStringUsingEncoding:NSUTF8StringEncoding];
        sqlite3_bind_text(statement, 1, newWord, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(statement, 2, 1);
        sqlite3_bind_double(statement, 3, [now timeIntervalSince1970]);
        sqlite3_bind_double(statement, 4, [now timeIntervalSince1970]);
        success = sqlite3_step(statement);
        NSAssert1((success != SQLITE_ERROR), @"Error: failed to insert into the database with message '%s'.", sqlite3_errmsg(iDB));
        sqlite3_finalize(statement);
    }
    
    [iUserWords removeAllObjects];
    
}

-(void)storeActiveWords
{
    int success;
    sqlite3_stmt *statement;
    NSDate* now;
    now = [NSDate date];
    
    int n = [iUserWords count];
    for (int i = 0; i < n; i++) {
        success = sqlite3_prepare_v2(iDB, "INSERT INTO active_dict (WORD,COUNT,ADD_DATE,LAST_SELECTED_DATE) VALUES(?,?,?,?)", -1, &statement, NULL);
        NSAssert1((success == SQLITE_OK), @"Error: failed to prepare statement for creating empty note with message '%s'.", sqlite3_errmsg(iDB));
        NSString* word = [iActiveWords objectAtIndex:i];
        sqlite3_bind_text(statement,   1, [word UTF8String], -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(statement, 2, 1);
        sqlite3_bind_double(statement, 3, [now timeIntervalSince1970]);
        sqlite3_bind_double(statement, 4, [now timeIntervalSince1970]);
        success = sqlite3_step(statement);
        NSAssert1((success != SQLITE_ERROR), @"Error: failed to insert into the database with message '%s'.", sqlite3_errmsg(iDB));
        sqlite3_finalize(statement);
    }
    
    [iActiveWords removeAllObjects];    
}


@end
