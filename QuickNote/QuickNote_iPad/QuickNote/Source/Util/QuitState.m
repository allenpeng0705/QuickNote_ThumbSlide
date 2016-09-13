#import "QuitState.h"

NSString *kDictVersion     = @"QuitStateVersion";
NSString *kQuitState       = @"QuitState";
NSString *kGaming          = @"Gaming";
NSString *kViewing         = @"Viewing";
NSString *kEditing         = @"Eidting";
NSString *kNoteIndex       = @"NoteIndex";
NSString *kNoteCursorIndex = @"NoteCursorIndex";
NSString *kPasteBoardText  = @"PasteBoardText";

@implementation QuitState

@synthesize m_dict;

- (id)init
{
	// load the stored preference of the user's last location from a previous launch
	m_version = [[NSUserDefaults standardUserDefaults] objectForKey:kDictVersion];

	if (m_version == nil)
	{
		m_version         = @"1.0";
		m_viewing         = NO;
		m_editing         = NO;
		m_noteIndex       = 0;
		m_noteCursorIndex = 0;
	}
	else
	{
		self.m_dict = [[NSUserDefaults standardUserDefaults] objectForKey:kQuitState];
		m_viewing         = [[m_dict objectForKey:kViewing] boolValue];
		m_editing         = [[m_dict objectForKey:kEditing] boolValue];
		m_noteIndex       = [[m_dict objectForKey:kNoteIndex] intValue];
		m_noteCursorIndex = [[m_dict objectForKey:kNoteCursorIndex] intValue];
	}
	#if TRACE_ENABLE
	[self traceDump];
	#endif
	return self;
}

- (void)save
{
	self.m_dict =  [NSDictionary dictionaryWithObjectsAndKeys:
						   [NSNumber numberWithBool:m_viewing],        kViewing,
						   [NSNumber numberWithBool:m_editing],        kEditing,
						   [NSNumber numberWithInt:m_noteIndex],       kNoteIndex,
						   [NSNumber numberWithInt:m_noteCursorIndex], kNoteCursorIndex,
						   nil];
	[[NSUserDefaults standardUserDefaults] setObject:m_version forKey:kDictVersion];
	[[NSUserDefaults standardUserDefaults] setObject:m_dict forKey:kQuitState];
	[[NSUserDefaults standardUserDefaults] synchronize];	
}

- (void)traceDump
{
	NSLog(@"quitState.m_version: %@",m_version);
	NSLog(@"quitState.m_viewing: %d",m_viewing);
	NSLog(@"quitState.m_editing: %d",m_editing);
	NSLog(@"quitState.m_noteIndex: %d",m_noteIndex);
	NSLog(@"quitState.m_noteCursorIndex: %d",m_noteCursorIndex);	
}

- (BOOL)isViewing
{
	return m_viewing;
}

- (BOOL)isEditing
{
	return m_editing;
}

- (int)noteIndex
{
	return m_noteIndex;
}

- (int)noteCursorIndex
{
	return m_noteCursorIndex;
}

- (void)setViewing:(BOOL)viewing
{
	m_viewing = viewing;
}

- (void)setEditing:(BOOL)editing
{
	m_editing = editing;
}

- (void)setNoteIndex:(int)noteIndex
{
	m_noteIndex = noteIndex;
}

- (void)setNoteCursorIndex:(int)cursorIndex
{
	m_noteCursorIndex = cursorIndex;
}

- (void)dealloc
{
	[m_dict release];
    [super dealloc];
}

@end
