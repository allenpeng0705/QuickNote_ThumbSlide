#import "DBUtil.h"
#import "Util.h"
#import <assert.h>
#import "utf8_string.h"
#import "IMESingleton.h"
#include "q_malloc.h"

static DBUtil* _instance = nil;

@implementation DBUtil

@synthesize iNotes, iSearchResult, iCurrentNote;

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
        [[Util sharedInstance] makeFileWritable:@"notes.db"];
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *docPath = [paths objectAtIndex:0];
        NSString *dbPath = [docPath stringByAppendingPathComponent:@"notes.db"]; 
        
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
    [iCurrentNote release];
    [iNotes removeAllObjects];
    [iNotes release];
    
    [iUserWords removeAllObjects];
    [iUserWords release];
    [iActiveWords removeAllObjects];
    [iActiveWords release];
    [super dealloc];
}

- (void)loadNotes
{
    int success;
	sqlite3_stmt *statement;
    
    NSMutableArray *notes = [[NSMutableArray alloc] init];
	
    success = sqlite3_prepare_v2(iDB, "SELECT ID, TITLE, DATE, CONTENT, TAG, LOCKED FROM note_table", -1, &statement, NULL);
    NSAssert1((success == SQLITE_OK), @"Error: failed to prepare sentence with message '%s'.", sqlite3_errmsg(iDB));

    while (sqlite3_step(statement) == SQLITE_ROW)
    {
        Note *note = [[Note alloc] init];
		int ID = sqlite3_column_int(statement, 0);
        char *title = (char *)sqlite3_column_text(statement, 1);
        NSTimeInterval seconds = sqlite3_column_double(statement, 2);
        char *content = (char *)sqlite3_column_text(statement, 3);
        char *tag = (char *)sqlite3_column_text(statement, 4);
        int locked = sqlite3_column_int(statement, 5);
        
        note.iID = ID;
        note.iTitle = (title) ? [NSString stringWithUTF8String:title] : @"";
        note.iDate = [NSDate dateWithTimeIntervalSince1970:seconds];
        note.iContent = (content) ? [NSString stringWithUTF8String:content] : @"";
        note.iTag = (tag) ? [NSString stringWithUTF8String:tag] : @"";
        if (locked == 0) {
            note.iLocked = NO;   
        } else {
            note.iLocked = YES;
        }
        
        note.iLoaded = YES;

		[notes addObject:note];
		[note release];
    }
	self.iNotes = notes;
    [notes release];
    sqlite3_finalize(statement);
    iCurrentNote = nil;

}

//insert a empty row in the database and return the primary key
- (void)createNote
{
    int success;
    sqlite3_stmt *statement;
    success = sqlite3_prepare_v2(iDB, "INSERT INTO note_table (TITLE,DATE) VALUES(?,?)", -1, &statement, NULL);
    NSAssert1((success == SQLITE_OK), @"Error: failed to prepare statement for creating empty note with message '%s'.", sqlite3_errmsg(iDB));
    
    Note* note = [[Note alloc] init];
    
    const char *title = "";
    note.iTitle = @"";
    note.iDate = [NSDate date];
	sqlite3_bind_text(statement,   1, title, -1, SQLITE_TRANSIENT);
	sqlite3_bind_double(statement, 2, [note.iDate timeIntervalSince1970]);
    
    success = sqlite3_step(statement);
    NSAssert1((success != SQLITE_ERROR), @"Error: failed to insert into the database with message '%s'.", sqlite3_errmsg(iDB));
	
    note.iID = sqlite3_last_insert_rowid(iDB);
    note.iLoaded = YES;
    sqlite3_finalize(statement);
    [iNotes addObject:note];
    //if (iCurrentNote != nil) [iCurrentNote release];
    self.iCurrentNote = note;
    [note release];
}

- (void)deleteNote:(Note*)aNote
{
    int success;
    sqlite3_stmt *statement;
    if (aNote == nil) return;
    
    success = sqlite3_prepare_v2(iDB, "DELETE FROM note_table WHERE ID=?", -1, &statement, NULL);
    NSAssert1((success == SQLITE_OK), @"Error: failed to prepare statement for delete note with message '%s'.", sqlite3_errmsg(iDB));    
    sqlite3_bind_int(statement, 1, aNote.iID);    
    success = sqlite3_step(statement);    
    NSAssert1((success == SQLITE_DONE), @"Error: failed to delete note from database with message '%s'.", sqlite3_errmsg(iDB));
    sqlite3_finalize(statement);
    [iNotes removeObject:aNote]; 
    if (iCurrentNote.iID == aNote.iID) {
        [iCurrentNote release];
        iCurrentNote = nil;        
    }
}

- (void)saveNote:(Note*)aNote
{
    int success;
    sqlite3_stmt *statement;
    if (aNote == nil) return;
    
    if (aNote.iNeedUpdate) {

        success = sqlite3_prepare_v2(iDB, "UPDATE note_table SET TITLE=?, CONTENT=?, DATE=?, TAG=?, LOCKED=? WHERE ID=?", -1, &statement, NULL);
        NSAssert1((success == SQLITE_OK), @"Error: failed to save note with message '%s'.", sqlite3_errmsg(iDB));
        
        const char* title;
        if ((aNote.iTitle == nil) || ([aNote.iTitle length] == 0)) {
            if ((aNote.iContent == nil) || ([aNote.iContent length] == 0)) {
                title = "";
            } else {
                NSInteger minLen = ([aNote.iContent length] < 40) ? [aNote.iContent length] : 40;
                title = [[aNote.iContent substringToIndex:minLen] UTF8String];                  
            }
        } else {
            title = [aNote.iTitle UTF8String];
        }
        sqlite3_bind_text(statement,   1, title, -1, SQLITE_TRANSIENT);
 
        const char* cotent;
        if ((aNote.iContent == nil) || ([aNote.iContent length] == 0)) {
            cotent = "";
        } else {
            cotent = [aNote.iContent UTF8String];
        }
        sqlite3_bind_text(statement,   2, cotent, -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(statement, 3, [aNote.iDate timeIntervalSince1970]);
        
        const char* tag;
        if ((aNote.iTag == nil) || ([aNote.iTag length] == 0)) {
            tag = "";
        } else {
            tag = [aNote.iTag UTF8String];
        }        
        sqlite3_bind_text(statement,   4, tag, -1, SQLITE_TRANSIENT);
        if (aNote.iLocked == YES) {
            sqlite3_bind_int(statement, 5, 1);
        } else {
            sqlite3_bind_int(statement, 5, 0);
        }
        sqlite3_bind_int(statement, 6, aNote.iID);
        success = sqlite3_step(statement);
        NSAssert1((success == SQLITE_DONE), @"Error: failed to save note with message '%s'.", sqlite3_errmsg(iDB));
        sqlite3_finalize(statement);    
        aNote.iNeedUpdate = NO;
        if ((iCurrentNote == nil) || (iCurrentNote.iID != aNote.iID)) {
            self.iCurrentNote = aNote;
        }

    }
}

- (void)saveAllNotes
{
    for (NSInteger i = 0; i < [iNotes count]; i++) {
        Note* note = [iNotes objectAtIndex:i];
        if (note.iNeedUpdate == YES) {
            [self saveNote:note];
        }
    }
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
