

@class NoteViewController,AboutViewController, UISearchBar, UISearchBarDelegate;

enum {
    None   = 0,
    Lock   = 1 << 0,
    Confirm= 1 << 1,
    Unlock = 1 << 2,
    Open   = 1 << 3,
    Delete = 1 << 4,
    Backup = 1 << 5,
    Export = 1 << 6
};
typedef NSUInteger Target;

@interface NotesListViewController : UIViewController <	
												UINavigationBarDelegate,
												UISearchBarDelegate,
												UITableViewDelegate, 
												UITableViewDataSource,
												UIActionSheetDelegate,
												UIAlertViewDelegate
                                                >
{
    NoteViewController  *iNoteContentViewCtrl;
    
	UITableView         *iNoteListView;
    UIToolbar           *iToolBar;
	
	BOOL                iSearching;
	UISearchBar	        *iSearchBar;	
	UIBarButtonItem     *iAddBtn;
	UIBarButtonItem     *iSearchBtn;
	UIBarButtonItem     *iReplyBtn;
	UIBarButtonItem     *iSortBtn;
	UIBarButtonItem     *iAboutBtn;
	UIBarButtonItem     *iSpaceBtn;
	UIBarButtonItem     *iTopSearchBtn;
	
	UIActionSheet		*iSortActionSheet;
	UIActionSheet		*iBackupActionSheet;
    NSIndexPath         *curIndexPath;
}

@property (nonatomic,retain) NoteViewController  *iNoteContentViewCtrl;
@property (nonatomic,retain) UITableView         *iNoteListView;
@property (nonatomic,retain) UIToolbar           *iToolBar;
@property (nonatomic,retain) UISearchBar        *iSearchBar;
@property (nonatomic,retain) NSIndexPath         *curIndexPath;

//- (void)restoreQuitState;

- (void)addNewNote;
-(void) showAlertForFullVersion;

@end
