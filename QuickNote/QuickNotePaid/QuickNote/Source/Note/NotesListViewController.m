#import "NotesListViewController.h"
#import "NoteViewController.h"
#import "NoteCell.h"
#import "Note.h"
#import "QuitState.h"
#import "LayoutConfiguration.h"

#import "Util.h"
#import "DBUtil.h"
#import "WebViewController.h"

@interface NotesListViewController (Private)
- (void)inspectNote:(Note*)note animated:(BOOL)animated;
- (void)createBarButtonItems;
- (void)sendAllNotes:(id)sender;
- (void)enterSearchMode;
- (void)exitSearchMode;
- (void)searchText:(NSString *)targetString;
- (BOOL)isKeyboardVisible;
- (void)setFooterBarWithEditing:(BOOL)editing;
@end

@implementation NotesListViewController

@synthesize iNoteContentViewCtrl, iNoteListView, iToolBar, curIndexPath, iSearchBar;

- (void)dealloc 
{
	[iSearchBar release];
    [iNoteContentViewCtrl release];
	[iNoteListView release];
	[iReplyBtn release];
	[iSortBtn release];
	[iAboutBtn release];
	[iSpaceBtn release];
	[iAddBtn release];
	[iSearchBtn release];
	[iTopSearchBtn release];
	[iToolBar release];
	[iSortActionSheet release];
	[iBackupActionSheet release];
	[super dealloc];
}

- (void)didReceiveMemoryWarning 
{
	[iSortActionSheet release]; iSortActionSheet = nil;
	[iBackupActionSheet release]; iBackupActionSheet = nil;
}

- (void)loadView 
{	
	self.title = @"ThumbSlide";		
	CGRect appFrame = [[UIScreen mainScreen] bounds];    
	UIView *contentView = [[UIView alloc] initWithFrame:appFrame];
	[contentView setAutoresizingMask:UIViewAutoresizingFlexibleTopMargin|UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleWidth];
	self.view = contentView;
	[contentView release];
	
	CGRect footerBarFrame = rectOfFootBar();

	appFrame.origin.y = self.navigationItem.accessibilityFrame.size.height;
	appFrame.size.height = appFrame.size.height - appFrame.origin.y - footerBarFrame.size.height;
    UITableView *tableview = [[UITableView alloc] initWithFrame:appFrame style:UITableViewStylePlain];
	tableview.delegate = self;
	tableview.dataSource = self;
    tableview.separatorStyle = UITableViewCellSeparatorStyleSingleLine;
    tableview.showsVerticalScrollIndicator = YES;
	tableview.backgroundColor = [[Util sharedInstance] BackgroundColorOfScrollQITextView];
	[tableview setAutoresizingMask:UIViewAutoresizingFlexibleTopMargin|UIViewAutoresizingFlexibleBottomMargin|UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleWidth];
	tableview.rowHeight = 50.0;
	self.iNoteListView = tableview;
	[self.view addSubview:tableview];
    [tableview release];
	
    // Create navigation bar buttons and toolbar butttons
	[self createBarButtonItems];
	
	// Setup navigation bar buttons
    self.navigationItem.rightBarButtonItem = iAddBtn;
	self.navigationItem.leftBarButtonItem = self.editButtonItem;
	
	// create a custom navigation bar button for Back
	UIBarButtonItem *notesBarButtonItem = [[UIBarButtonItem alloc] init];
	notesBarButtonItem.title = @"Notes";
	self.navigationItem.backBarButtonItem = notesBarButtonItem;
	[notesBarButtonItem release];

	iSearching = NO;
	// Setup toolbar buttons
	UIToolbar *toolbar = [[UIToolbar alloc] initWithFrame:footerBarFrame];
	toolbar.barStyle = UIBarStyleBlack;
	self.iToolBar = toolbar;
	[self.view addSubview:toolbar];
	[toolbar release];	
	[self setFooterBarWithEditing:NO];
}

- (void)viewWillAppear:(BOOL)animated
{
    [iNoteListView reloadData];
}

- (void)viewDidAppear:(BOOL)animated
{
	if(iSearching) {
		self.navigationItem.titleView = self.iSearchBar;
	} else {
		self.navigationItem.titleView = nil;
	}
}

- (void)viewWillDisappear:(BOOL)animated
{
}

#pragma mark View rotate
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{	
	if (UIInterfaceOrientationIsPortrait(interfaceOrientation)) {
		return YES;
	} else {
		return NO;
	}
}

#pragma mark Search text field
- (UISearchBar*)iSearchBar
{
	if (iSearchBar == nil) {
		iSearchBar = [[UISearchBar alloc] initWithFrame:CGRectMake(0.0,0.0,320.0,44.0)];
        iSearchBar.barStyle = UIBarStyleBlack;
		iSearchBar.delegate = self;
		iSearchBar.placeholder = @"Key words";
	}
	return iSearchBar;
}

- (void)searchText:(NSString *)targetString
{
	//construct matchedNotes array
	NSMutableArray * matchingNotes = [NSMutableArray arrayWithCapacity:10];
	NSRange notFoundRange = NSMakeRange(NSNotFound, 0);
	NSArray * words = [targetString componentsSeparatedByString:@" "];

	NSStringCompareOptions cmpOption = NSCaseInsensitiveSearch;
    DBUtil* noteDB = [DBUtil sharedInstance];
	for (Note *aNote in noteDB.iNotes) {
		if (aNote.iContent.length == 0) continue;
		for (NSString *word in words) {
			if (word.length && !NSEqualRanges(notFoundRange,[aNote.iContent rangeOfString:word options:cmpOption])) {
				[matchingNotes addObject:aNote];
				break;
			}
		}
	}
    
    [noteDB.iSearchResult removeAllObjects];
	noteDB.iSearchResult = matchingNotes;
	[iNoteListView reloadData];
}

- (void)doSearch:(id)sender
{
	NSString *targetString = iSearchBar.text;
	if (targetString.length <= 0) return;
    
	[self searchText:targetString];
	[iSearchBar resignFirstResponder];
}

- (void)enterSearchMode
{
    CGRect frame = iNoteListView.frame;
    frame.origin.y = 0;
    frame.size.height = frame.size.height + self.iToolBar.frame.size.height;
    iNoteListView.frame = frame;
    [self.iToolBar removeFromSuperview];
	
	iSearching = YES;
	
	self.navigationItem.titleView = self.iSearchBar;
    self.navigationItem.rightBarButtonItem = iTopSearchBtn;
	[super setEditing:YES animated:NO];
    [self.iSearchBar becomeFirstResponder];
	[iNoteListView setEditing:NO animated:NO];
	[iNoteListView reloadData];

}

- (void)exitSearchMode
{
	if (YES == iSearching) {
		CGRect frame = iNoteListView.frame;
		frame.origin.y = 0;
		frame.size.height = frame.size.height - self.iToolBar.frame.size.height;
		iNoteListView.frame = frame;        
        [self.view addSubview:self.iToolBar];
	}
	
	iSearching = NO;
	
    DBUtil* noteDB = [DBUtil sharedInstance];
	noteDB.iSearchResult = nil;
	iSearchBar.text = nil;
	self.navigationItem.rightBarButtonItem = iAddBtn;
	self.navigationItem.titleView = nil;
	[iNoteListView reloadData];
}

#pragma mark -  
#pragma mark UISearchBarDelegate Methods  

- (void)searchBar:(UISearchBar *)searchBar  
    textDidChange:(NSString *)searchText {  
    // We don't want to do anything until the user clicks   
    // the 'Search' button.  
    // If you wanted to display results as the user types   
    // you would do that here.  
} 

// We call this when we want to activate/deactivate the UISearchBar  
// Depending on active (YES/NO) we disable/enable selection and   
// scrolling on the UITableView  
// Show/Hide the UISearchBar Cancel button  
// Fade the screen In/Out with the disableViewOverlay and   
// simple Animations  
- (void)searchBar:(UISearchBar *)searchBar activate:(BOOL) active{     
    //    [searchBar setShowsCancelButton:active animated:YES];  
}

- (void)searchBarTextDidBeginEditing:(UISearchBar *)searchBar {  
    // searchBarTextDidBeginEditing is called whenever   
    // focus is given to the UISearchBar  
    // call our activate method so that we can do some   
    // additional things when the UISearchBar shows.  
    [self searchBar:searchBar activate:YES];  
}  

- (void)searchBarTextDidEndEditing:(UISearchBar *)searchBar {  
    // searchBarTextDidEndEditing is fired whenever the   
    // UISearchBar loses focus  
    // We don't need to do anything here.  
}  

- (void)searchBarCancelButtonClicked:(UISearchBar *)searchBar {  
    // Clear the search text  
    // Deactivate the UISearchBar  
    searchBar.text=@"";  
    [self searchBar:searchBar activate:NO];  
}  

- (void)searchBarSearchButtonClicked:(UISearchBar *)searchBar {  
    // Do the search and show the results in tableview  
    // Deactivate the UISearchBar  
    
    // You'll probably want to do this on another thread  
    // SomeService is just a dummy class representing some   
    // api that you are using to do the search   
    [self searchBar:searchBar activate:NO];  
    [self searchText:[searchBar text]];
    [iSearchBar resignFirstResponder];
}  

  

#pragma mark Table Content and Appearance

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
	return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    DBUtil* noteDB = [DBUtil sharedInstance];
    NSArray* array = iSearching?noteDB.iSearchResult:noteDB.iNotes;
	return array.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	static NSString *cellIdentifier = @"TxtFileListCellID";
	NoteCell *cell = (NoteCell*)[tableView dequeueReusableCellWithIdentifier:cellIdentifier];
	if (cell == nil) {
		cell = [[[NoteCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:cellIdentifier] autorelease];
        cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
	}

    DBUtil* noteDB = [DBUtil sharedInstance];
    Note *note = (Note *)[(iSearching?noteDB.iSearchResult:noteDB.iNotes) objectAtIndex:indexPath.row];
    if (note == nil) return nil;
    if ((note.iTitle == nil) || (note.iTitle.length == 0)) {
        cell.iTitle.text = @"";
    } else {
        cell.iTitle.text = note.iTitle;
    }
    if (note.iDate == nil) {
        cell.iDate.text = @"";
    } else {
        NSString* date = [[[Util sharedInstance] dateFormatter] stringFromDate:note.iDate];
        NSUInteger count = [note.iContent length];
        cell.iDate.text = [NSString stringWithFormat:@"%@, Count:%d", date, count]; 
	}
	cell.iIndexPath = indexPath;
   	return cell;
}

#pragma mark Table Selection 
- (NSIndexPath *)tableView:(UITableView *)tv willSelectRowAtIndexPath:(NSIndexPath *)indexPath 
{
    DBUtil* noteDB = [DBUtil sharedInstance];
	Note * note = [(iSearching?noteDB.iSearchResult:noteDB.iNotes) objectAtIndex:indexPath.row];
    self.curIndexPath = indexPath;
    noteDB.iCurrentNote = note;
    [self inspectNote:note animated:YES];
    return indexPath;
}

- (void)inspectNote:(Note *)note animated:(BOOL)animated
{
    if (iNoteContentViewCtrl == nil) {
        NoteViewController *viewController = [[NoteViewController alloc] init];
        self.iNoteContentViewCtrl = viewController;
        [viewController release];
    }
	
    if (iNoteContentViewCtrl == self.navigationController.topViewController) {
		[iNoteContentViewCtrl viewWillDisappear:animated];
		[iNoteContentViewCtrl viewDidDisappear:animated];
		[iNoteContentViewCtrl viewWillAppear:animated];
		[iNoteContentViewCtrl viewDidAppear:animated];
    } else {
		// install note from memory to NoteViewController
		[self.navigationController pushViewController:iNoteContentViewCtrl animated:animated];
	}
}

#pragma mark Table Editing

- (void)setEditing:(BOOL)editing animated:(BOOL)animated 
{
    [super setEditing:editing animated:animated];
	if (editing||iSearching) {
		[self exitSearchMode];
	}
	[iNoteListView setEditing:editing animated:animated];
	[self setFooterBarWithEditing:editing];
	[iNoteListView reloadData];
}

- (void)tableView:(UITableView *)tv commitEditingStyle:(UITableViewCellEditingStyle)editingStyle 
                                    forRowAtIndexPath:(NSIndexPath *)indexPath 
{
    if (editingStyle == UITableViewCellEditingStyleDelete) 
    {
        DBUtil* noteDB = [DBUtil sharedInstance];
        Note * note = [(iSearching?noteDB.iSearchResult:noteDB.iNotes) objectAtIndex:indexPath.row];
        [noteDB deleteNote:note];
        if (iSearching) { 
            [self searchText:iSearchBar.text];
        } else {
            [iNoteListView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
        }

    }
}

- (void)addNewNote
{
    DBUtil* noteDB = [DBUtil sharedInstance];
    [noteDB createNote];  
    Note* note = noteDB.iCurrentNote;
    [self inspectNote:note animated:YES];
}

#pragma mark Table row reordering

// Determine whether a given row is eligible for reordering or not.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath 
{
    return YES;
}

// Process the row move. This means updating the data model to correct the item indices.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath 
	  toIndexPath:(NSIndexPath *)toIndexPath 
{
    DBUtil* noteDB = [DBUtil sharedInstance];
	Note * fromNote = [[noteDB.iNotes objectAtIndex:fromIndexPath.row] retain];	
	[noteDB.iNotes removeObject:fromNote];	
	[noteDB.iNotes insertObject:fromNote atIndex:toIndexPath.row];
	
	//update notes' sequence
	int i = 0;
	for (Note *note in noteDB.iNotes) {
		note.iSequence = i++;
    }
	[fromNote release];
}

#pragma mark Buttons on navigation bar and toolbar

- (void)createBarButtonItems
{
    iTopSearchBtn  = [[UIBarButtonItem alloc] initWithTitle:@"Search"
                                                     style:UIBarButtonItemStyleDone
                                                    target:self
                                                    action:@selector(doSearch:)];
    
	iAddBtn = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemCompose
															target:self 
															action:@selector(addNewNote)];

    iSearchBtn  = [[UIBarButtonItem alloc] initWithTitle:@"Search"
                                                   style:UIBarButtonItemStyleBordered
                                                  target:self
                                                  action:@selector(searchAction:)];
	
	iAboutBtn  = [[UIBarButtonItem alloc] initWithTitle:@"Help"
												  style:UIBarButtonItemStyleBordered
												 target:self
												 action:@selector(aboutAction:)];
	
	iReplyBtn = [[UIBarButtonItem alloc] initWithTitle:@"Mail"
												  style:UIBarButtonItemStyleBordered
												 target:self
												 action:@selector(backupAction:)];
	
	iSpaceBtn = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace 
															  target:nil 
															  action:nil];
	
	iSortBtn  = [[UIBarButtonItem alloc] initWithTitle:@"Sort"
												 style:UIBarButtonItemStyleBordered
												target:self
												action:@selector(sortAction:)];
    
    
	iSortBtn.width   = 64;
	iAboutBtn.width  = 64;
	iReplyBtn.width = 64;
	iSearchBtn.width = 64;

}

- (void)setFooterBarWithEditing:(BOOL)editing
{
	if (editing) {
		[iToolBar setItems:[NSArray arrayWithObjects: iAboutBtn, iSpaceBtn, iSortBtn, iSpaceBtn, iReplyBtn, nil] animated:NO];
    } else {
		[iToolBar setItems:[NSArray arrayWithObjects: iAboutBtn, iSpaceBtn, iSearchBtn, iSpaceBtn, iSortBtn, iSpaceBtn, iReplyBtn, nil] animated:NO];
    }
}

- (void)searchAction:(id)sender
{
	[self enterSearchMode];
}

- (void)aboutAction:(id)sender
{
	WebViewController *webViewVC = [[WebViewController alloc] initWithNibName:@"WebViewController" bundle:nil];
	[self.navigationController pushViewController:webViewVC animated:YES];
	[webViewVC release];
}

- (void)jumpToFullVersion
{
}

- (void)sortAction:(id)sender
{
	if(iSortActionSheet == nil)
	{
		iSortActionSheet = [[UIActionSheet alloc] initWithTitle:@"Sort Notes by"
													  delegate:self 
											 cancelButtonTitle:@"Cancel" 
										destructiveButtonTitle:nil
											 otherButtonTitles:@"Title", @"Date",nil];
	}
    [iSortActionSheet showInView:self.view];
}

- (void)backupAction:(id)sender
{
    DBUtil* noteDB = [DBUtil sharedInstance];
	if (noteDB.iNotes.count < 1) {
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:nil 
														message:@"No notes!"
													   delegate:nil 
											  cancelButtonTitle:@"OK" 
											  otherButtonTitles:nil];
		[alert show];
		[alert release];			
		return;
	}
	
    // open a dialog with an OK and cancel button
	if(iBackupActionSheet == nil)
	{
#if 1
		iBackupActionSheet = [[UIActionSheet alloc] initWithTitle:@"Backup notes via email"
														delegate:self 
											   cancelButtonTitle:@"Cancel" 
										  destructiveButtonTitle:nil 
											   otherButtonTitles:@"OK",nil];
#else	
		backupActionSheet = [[UIActionSheet alloc] initWithTitle:@"Sync as Google Doc" 
														delegate:self 
											   cancelButtonTitle:@"Cancel" 
										  destructiveButtonTitle:nil 
											   otherButtonTitles:nil];
		[backupActionSheet addButtonWithTitle:@"Sync from Google Doc"];
		[backupActionSheet addButtonWithTitle:@"Sync to Google Doc"];
#endif
	}
    [iBackupActionSheet showInView:self.view];
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    DBUtil *noteDB = [DBUtil sharedInstance];
	if (actionSheet == iBackupActionSheet) {		
		
#if 1
		switch (buttonIndex) {
            case 0:  // Ok
			{
                [self sendAllNotes:nil];
				break;
			}
			default: // Cancel
				break;
		}
#else
		// Add sync code here
		if(buttonIndex != [actionSheet cancelButtonIndex]) {
			if([[actionSheet buttonTitleAtIndex:buttonIndex] rangeOfString:@"from"].location != NSNotFound) {	// Sync from Google Doc
				NSLog(@"Sync from Google Doc");
				//	Doing the real syncronizing action here
				
			} else {	// Sync to Google Doc
				NSLog(@"Sync to Google Doc");
				// Doing the real syncronizing action here
			}
		}
#endif
	}
	else if(actionSheet == iSortActionSheet)
	{
		int i = 0;
        NSArray* notes = nil;
		switch(buttonIndex)
		{
			case 0: // By Alphabetic
			{
				static BOOL orderA2Z = YES;
				if(orderA2Z) {
					notes = [noteDB.iNotes sortedArrayUsingSelector:@selector(compareByTitleA2Z:)];
                } else {
					notes = [noteDB.iNotes sortedArrayUsingSelector:@selector(compareByTitleZ2A:)];
                }
				orderA2Z = !orderA2Z;
				for (Note *note in notes) {
					note.iSequence = i++;
                }
                [noteDB.iNotes removeAllObjects];
                [noteDB.iNotes addObjectsFromArray:notes];
				[iNoteListView reloadData];
			}
				break;
			case 1: // By Date
			{
				static BOOL orderNewestFirst = YES;
				if(orderNewestFirst) {
					notes = [noteDB.iNotes sortedArrayUsingSelector:@selector(compareByDateNewestFirst:)];
                } else {
					notes = [noteDB.iNotes sortedArrayUsingSelector:@selector(compareByDateOldestFirst:)];
                }
				orderNewestFirst = !orderNewestFirst;
				for (Note *note in notes) {
					note.iSequence = i++;
                }
                [noteDB.iNotes removeAllObjects];
                [noteDB.iNotes addObjectsFromArray:notes];
				[iNoteListView reloadData];
                break;

			}

			default:
				break;
		}
	}
}

- (void)sendAllNotes:(id)sender
{
    DBUtil *noteDB = [DBUtil sharedInstance];

	NSArray * notes = noteDB.iNotes;
	NSMutableArray * mutableStringNotes = [NSMutableArray arrayWithCapacity:(2*notes.count)];
	for (Note *note in notes) {
        NSString *dividerString = @"Title: ";
		if (note.iTitle.length == 0) continue;
        dividerString = [dividerString stringByAppendingString:note.iTitle];
        dividerString = [dividerString stringByAppendingString:@"\n"];
        dividerString = [dividerString stringByAppendingString:@"Date: "];
        dividerString = [dividerString stringByAppendingString:[[[Util sharedInstance] dateFormatter] stringFromDate:note.iDate]];
        dividerString = [dividerString stringByAppendingString:@"\n"];
        dividerString = [dividerString stringByAppendingString:@"Detail: "];
        dividerString = [dividerString stringByAppendingString:@"\n"];
		[mutableStringNotes addObject:dividerString];
        NSString* content = [note.iContent stringByAppendingString:@"\n"];
        content = [content stringByAppendingString:@"\n"];
        content = [content stringByAppendingString:@"*************************************\n"];
        
		[mutableStringNotes addObject:content];
	}
	NSString *rawStr = [mutableStringNotes componentsJoinedByString:@""];
	QuickNoteDelegate* delegate = [[Util sharedInstance] appDelegate];
    [delegate sendMailWithSubject:@"Notes from ThumbSlide" Body:rawStr];
}

@end
