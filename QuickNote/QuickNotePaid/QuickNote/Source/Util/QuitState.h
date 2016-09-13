#import <UIKit/UIKit.h>

@interface QuitState : NSObject 
{
	NSString     *m_version;
	NSDictionary *m_dict;
	BOOL          m_viewing;
	BOOL          m_editing;
	int           m_noteIndex;
	int           m_noteCursorIndex;
}

- (id)init;

@property (nonatomic, retain) NSDictionary *m_dict;
 
- (BOOL)isViewing; 
- (BOOL)isEditing;
- (int)noteIndex;
- (int)noteCursorIndex;
 
- (void)setViewing:(BOOL)viewing; 
- (void)setEditing:(BOOL)editing;
- (void)setNoteIndex:(int)noteIndex;
- (void)setNoteCursorIndex:(int)cursorIndex;

- (void)save;

- (void)traceDump;

@end
