
#import "NotesListViewController.h"
#import "NoteViewController.h"
#import "NoteContentView.h"
#import <assert.h>
#import "Note.h"

#import "IMESingleton.h"
#import "LayoutConfiguration.h"
#import "DBUtil.h"
#import "Util.h"
#import "QuickNoteDelegate.h"
#import "IFNNotificationDisplay.h"
#import "NSDate+Helper.h"
#import "QIInputView.h"
#import "KeyboardView.h"
#import "IMESingleton.h"

#import <Foundation/Foundation.h>

static NSString* kAppId = @"155071991244885";

@interface NoteViewController (Private)
- (void)footerBarSetup:(UIToolbar*)footerBar;
- (void)doEmailInApp;
- (void)doSmsInApp;

- (void)login;
- (void)logout;

-(CGFloat) screenScale:(CGFloat)screenScale;
-(UIImage*) scale:(UIImage*) image targetSize:(CGSize) targetSize ignoreSceenScale:(BOOL)ingoreSceenScale;
@end

@implementation NoteViewController
//@synthesize navigationBar;
@synthesize popoverCtrl;
@synthesize iNoteContentView;
@synthesize iDoneBtn;
@synthesize iSettingBtn;
@synthesize iOptionBtn;
@synthesize cameraUtils = _cameraUtils;

- (id)init
{
    self = [super init];  
    iOptionBtn = nil;
    _permissions =  [[NSArray arrayWithObjects:
                      @"read_stream", @"offline_access",nil] retain];
    iDoneBtn = [[UIBarButtonItem alloc]
                initWithTitle:NSLocalizedString(@"Done", @"") style:UIBarButtonItemStyleBordered
                target:self action:@selector(doneAction:)];
    iSettingBtn = [[UIBarButtonItem alloc]
                   initWithTitle:NSLocalizedString(@"Setting", @"") style:UIBarButtonItemStyleBordered
                   target:self action:@selector(settingAction:)];
    
    NSNumber* result = [[NSUserDefaults standardUserDefaults] objectForKey:@"UsingQuickKeyboard"];
    if (result == nil) {
        [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithBool:YES] forKey:@"UsingQuickKeyboard"];
        [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithBool:YES] forKey:@"KeyboardAudio"];
        [[NSUserDefaults standardUserDefaults] synchronize]; 
    } 
    NSString* layout = [[NSUserDefaults standardUserDefaults] stringForKey:@"KeyboardLayout"];
    if (layout == nil) {
        [[NSUserDefaults standardUserDefaults] setObject:@"NormalMode" forKey:@"KeyboardLayout"];
        [[NSUserDefaults standardUserDefaults] synchronize];
    }
	return self;
}

- (void)dealloc 
{
    [iDoneBtn release];
    [iSettingBtn release];
    //[navigationBar release];
    [iNoteContentView release];
    [iOptionBtn release];
    [_permissions release];
    [_cameraUtils release];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

-(void)showCamera
{
    _gotoCameraRoll = YES;
    if (!_cameraUtils) {
        _cameraUtils = [[CameraUtils alloc] init];
        _cameraUtils.delegate = self;
    }
	[_cameraUtils showCamera:self];
}

-(void)showPhotos
{
    _gotoCameraRoll = YES;
    if (!_cameraUtils) {
        _cameraUtils = [[CameraUtils alloc] init];
        _cameraUtils.delegate = self;
    }
	[_cameraUtils showPhotoLibrary:self animated:YES];
}


-(void) keyboardDidShow: (NSNotification *)notif {
    [self enableDoneButton];
    self.iNoteContentView.iKeyboardShowing = YES;
    [self.iNoteContentView setNeedsLayout];
}

-(void) keyboardDidHide: (NSNotification *)notif {
	[self enableSettingButton];
    self.iNoteContentView.iKeyboardShowing = NO;
    [self.iNoteContentView setNeedsLayout];
}

- (void)loadView 
{
    [super loadView];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector (keyboardDidShow:)
												 name: UIKeyboardDidShowNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector (keyboardDidHide:)
												 name: UIKeyboardDidHideNotification object:nil];
	iNoteContentView = [[NoteContentView alloc] initWithFrame:self.view.frame];
	[self footerBarSetup:iNoteContentView.iToolBar];
    [iNoteContentView setAutoresizingMask:UIViewAutoresizingFlexibleTopMargin|UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleWidth];
	[self.view addSubview:iNoteContentView];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{	
//	if (!UIInterfaceOrientationIsPortrait(interfaceOrientation) && 
//	    !UIInterfaceOrientationIsLandscape(interfaceOrientation)){
//		return NO;
//	} 
	
	switch (interfaceOrientation) {
		case UIInterfaceOrientationPortrait:
		case UIInterfaceOrientationPortraitUpsideDown:
//			[[UIApplication sharedApplication] setStatusBarHidden:NO];
			self.navigationController.navigationBarHidden = NO;
			break;
		case UIInterfaceOrientationLandscapeLeft:
		case UIInterfaceOrientationLandscapeRight:
//			[[UIApplication sharedApplication] setStatusBarHidden:YES];
			self.navigationController.navigationBarHidden = NO;
			break;
	}
	return YES;
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
	iNoteContentView.userInteractionEnabled = NO;
    iToOrientation = toInterfaceOrientation;
}

/*
- (void)willAnimateFirstHalfOfRotationToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
}

// Called between the first and second halves of the rotation, outside any animation transactions. The header and footer views are offscreen.
- (void)didAnimateFirstHalfOfRotationToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
}

// A this point, our view orientation is set to the new orientation.
- (void)willAnimateSecondHalfOfRotationFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation duration:(NSTimeInterval)duration
{
}
*/

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
	iNoteContentView.userInteractionEnabled = YES;
    iFromOrientation = fromInterfaceOrientation;
    
    if ((UIInterfaceOrientationIsPortrait(iFromOrientation) == YES) && 
        (UIInterfaceOrientationIsLandscape(iToOrientation) == YES)) {
        [iNoteContentView handleOrientationChang:YES];
    } else if ((UIInterfaceOrientationIsPortrait(iToOrientation) == YES) && 
               (UIInterfaceOrientationIsLandscape(iFromOrientation) == YES)) {
        [iNoteContentView handleOrientationChang:NO];
    }
    
}

- (void)enableDoneButton
{
    self.navigationItem.rightBarButtonItem = iDoneBtn;
}

- (void)disableDoneButton
{
    self.navigationItem.rightBarButtonItem = nil;
}

- (void)enableSettingButton
{
	self.navigationItem.rightBarButtonItem = iSettingBtn;
}

- (void)disableSettingButton
{
	self.navigationItem.rightBarButtonItem = nil;
}

#pragma mark SwiKeyboardViewRotateDelegate

- (void)doneAction:(id)sender
{
    [iNoteContentView showInputView:NO];
    if (![self saveNoteWithChecking:NO]) {
        DBUtil* noteDB = [DBUtil sharedInstance];
        [noteDB createNote]; 
        [self saveNote];
        [self setupNote:NO];
        [[NSNotificationCenter defaultCenter] postNotificationName:NotesNeedRefresh object:self];
    }

}

- (void)settingAction:(id)sender
{
}

- (void)optionAction:(id)sender
{
    BOOL play = YES;
    NSString* button_text = nil;
    NSNumber* result = nil;
    result = [[NSUserDefaults standardUserDefaults] objectForKey:@"KeyboardAudio"];
	if (result) {
		play = [result boolValue];
	}
    
    if (play == YES) {
        button_text = [[NSString alloc] initWithString:@"Press Sounds Off"];
    } else {
        button_text = [[NSString alloc] initWithString:@"Press Sounds On"];
    }
    
    result = nil;
    BOOL quick = YES;
    NSString* quick_text = nil;
    result = [[NSUserDefaults standardUserDefaults] objectForKey:@"UsingQuickKeyboard"];
	if (result) {
		quick = [result boolValue];
	} 
    
    if (quick == YES) {
        quick_text = [[NSString alloc] initWithString:@"Apple Keyboard"];
    } else {
        quick_text = [[NSString alloc] initWithString:@"Quick Keyboard"];
    }
    
    NSString* thumb = @"Thumb Mode";
    NSString* trace = @"Trace Mode";
    NSString* traceleft = @"Trace Mode-Left Hand";
    NSString* type = @"Normal Mode";
    
    UIActionSheet* optionActionSheet = [[UIActionSheet alloc] initWithTitle:nil
                                                     delegate:self 
                                                    cancelButtonTitle:@"Cancel"
                                            destructiveButtonTitle:nil 
                                            otherButtonTitles:quick_text, thumb, trace, traceleft, type, button_text, nil];
    optionActionSheet.actionSheetStyle = UIActionSheetStyleDefault;
    optionActionSheet.tag = 0;

    [optionActionSheet showInView:self.view]; 
    [optionActionSheet release];
    [button_text release];
    [quick_text release];
}

- (void)viewWillAppear:(BOOL)animated 
{
	[super viewWillAppear:animated];
    
    _gotoCameraRoll = NO;

    if (iOptionBtn == nil) {
        CGRect rect = CGRectZero;
        rect.size.width = 100;
        rect.size.height = self.navigationController.navigationBar.frame.size.height - 10;
        rect.origin.x = (self.navigationController.navigationBar.frame.size.width - rect.size.width)/2;
        rect.origin.y = 5;
        UIButton* optionBtn = [UIButton buttonWithType:UIButtonTypeRoundedRect];
        optionBtn.frame = rect;
        optionBtn.tag = 1000;
        [optionBtn setTitle:@"Option" forState:UIControlStateNormal];
        [optionBtn addTarget:self action:@selector(optionAction:) forControlEvents:UIControlEventTouchUpInside];
        self.iOptionBtn = optionBtn;
    }
    [self.navigationController.navigationBar addSubview: iOptionBtn];

    
    DBUtil* noteDB = [DBUtil sharedInstance];
    Note* note = noteDB.iCurrentNote;
    
    [iNoteContentView setContent:note.iContent];
    NSString* date = [[[Util sharedInstance] dateFormatter] stringFromDate:note.iDate];
    NSUInteger count = [note.iContent length];
    NSString* text = [NSString stringWithFormat:@"%@, Count:%d", date, count]; 
    iNoteContentView.iTimeField.text = text;
    
    if ([note.iContent length] > 0) {
        if (iNoteContentView.iKeyboardShowing == YES) {;
            [iNoteContentView showInputView:NO];
        } else {
            [self enableSettingButton];
            [iNoteContentView setNeedsLayout];
        }
    } else {
        [iNoteContentView showInputView:YES];
    }
}

- (void)setupNote:(BOOL)showKeyboard
{
    DBUtil* noteDB = [DBUtil sharedInstance];
    Note* note = noteDB.iCurrentNote;
    
    [iNoteContentView setContent:note.iContent];
    NSString* date = [[[Util sharedInstance] dateFormatter] stringFromDate:note.iDate];
    NSUInteger count = [note.iContent length];
    NSString* text = [NSString stringWithFormat:@"%@, Count:%d", date, count]; 
    iNoteContentView.iTimeField.text = text;
	
	if (popoverCtrl != nil) {
		[popoverCtrl dismissPopoverAnimated:YES];
	}
    
    if ([note.iContent length] > 0) {
        [iNoteContentView showInputView:NO];
        [self enableSettingButton];
        [iNoteContentView setNeedsLayout];
    } else {
        if (showKeyboard) {
            [iNoteContentView showInputView:YES];
        }
    }
}

//- (void)viewDidAppear:(BOOL)animated 
//{
//	self.navigationItem.titleView = nil;
//}

- (void)viewWillDisappear:(BOOL)animated 
{
    DBUtil* noteDB = [DBUtil sharedInstance];
    Note* note = noteDB.iCurrentNote;
	if ([iNoteContentView lengthOfContent] != 0) {   
        [self saveNote];
    } else {
		[noteDB deleteNote:note];
	}

    if (iOptionBtn != nil) {
        [iOptionBtn removeFromSuperview];
    }

    QuickNoteDelegate* delegate = [[Util sharedInstance] appDelegate];
    [delegate autoSaveTimerSuspend];
    
    UIView *dv = [self.view viewWithTag:NOTIFICATION_DISPLAY_TAG];
	if (dv != nil) [dv removeFromSuperview];
    
    [super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated 
{
//	noteContentView.imView.rotateDelegate = nil;	
    [iNoteContentView hideKeyboard];
}

- (void)updateTitle
{
    NSString *tmpText = [iNoteContentView content];
    NSInteger minLen = (tmpText.length < 40) ? tmpText.length : 40;
    self.title = [tmpText substringToIndex:minLen];    
}

- (BOOL)saveNoteWithChecking:(BOOL)refresh
{
    DBUtil* noteDB = [DBUtil sharedInstance];
    Note* note = noteDB.iCurrentNote;
    if (note) {
        if ([iNoteContentView lengthOfContent] != 0) {   
            [self saveNote];
            return YES;
        } else {
            [noteDB deleteNote:note];
            if (refresh) {
                [[NSNotificationCenter defaultCenter] postNotificationName:NotesNeedRefresh object:self];
            }
            return NO;
        }
    }
}

- (BOOL)saveNote
{
    DBUtil* noteDB = [DBUtil sharedInstance];
    Note* note = noteDB.iCurrentNote;
    NSString *tmpText = [iNoteContentView content];
        
    NSRange firstLineBreakRange = [tmpText rangeOfString:@"\n"];
    NSInteger minLen = 40 > tmpText.length ? tmpText.length : 40;
    if ((firstLineBreakRange.location > minLen) || (firstLineBreakRange.location == 0)) {
        [note setITitle:[tmpText substringToIndex:minLen]]; 
    } else {
        [note setITitle:[tmpText substringToIndex:firstLineBreakRange.location]];  
    }
   
    [note setIContent:tmpText];
    [note setIDate:[NSDate date]];
    [note setITag:nil];
    [noteDB saveNote:note]; 
    [[NSNotificationCenter defaultCenter] postNotificationName:NotesNeedRefresh object:self];
    return YES;
}

#pragma mark Footer bar
- (void)footerBarSetup:(UIToolbar*)toolbar
{    
    UIBarButtonItem *emailItem = [[[UIBarButtonItem alloc] initWithTitle:@"Mail" style:UIBarButtonItemStyleBordered
                                                                  target:self action:@selector(email:)]autorelease];
    
    UIBarButtonItem *smsItem = [[[UIBarButtonItem alloc] initWithTitle:@"SMS" style:UIBarButtonItemStyleBordered
                                                                target:self action:@selector(sms:)] autorelease];
    
    UIBarButtonItem *copyItem = [[[UIBarButtonItem alloc] initWithTitle:@"Copy" style:UIBarButtonItemStyleBordered
                                                                target:self action:@selector(copy:)] autorelease];
    
    UIBarButtonItem *clearItem = [[[UIBarButtonItem alloc] initWithTitle:@"Clear" style:UIBarButtonItemStyleBordered target:self action:@selector(delete:)] autorelease];
    
    UIBarButtonItem *sendItem = [[[UIBarButtonItem alloc] initWithTitle:@"Post" style:UIBarButtonItemStyleBordered target:self action:@selector(post:)] autorelease];
	
	UIBarButtonItem *spaceItem = [[[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:nil] autorelease];
    
    emailItem.width   = 100;
	smsItem.width     = 100;
	copyItem.width    = 100;
	clearItem.width   = 100;
    sendItem.width    = 100;
	
	toolbar.items = [NSArray arrayWithObjects:spaceItem,emailItem,spaceItem,smsItem,spaceItem,copyItem, spaceItem, clearItem,spaceItem, sendItem, spaceItem, nil];
}

- (void)email:(id)sender
{	
	if ([self saveNote]) {
        [self doEmailInApp];
    }
}

- (void)post:(id)sender
{	
    
    // Invoke Twitter, Facebook, Gtalk etc
    UIActionSheet* snsActionSheet = [[UIActionSheet alloc] initWithTitle:@"POST Message to Facebook"
                                                                delegate:self 
                                                       cancelButtonTitle:@"Cancel" 
                                                  destructiveButtonTitle:nil 
                                                       otherButtonTitles:@"Note", @"Photo", @"Image", nil];
    snsActionSheet.actionSheetStyle = UIActionSheetStyleDefault;
    snsActionSheet.tag = 3;
    [snsActionSheet showInView:self.view];  
    [snsActionSheet release];

}

- (void)doEmailInApp
{
	NSString *rawStr;
    DBUtil* noteDB = [DBUtil sharedInstance];
    Note* note = noteDB.iCurrentNote;        
    rawStr = [note iContent];	
	[UIPasteboard generalPasteboard].string = rawStr;

	[AppDelegate sendMailWithSubject:[note iTitle] Body:rawStr];
}

- (void)sms:(id)sender
{	
	if ([self saveNote]) {
        [self doSmsInApp];
    }
}

- (void)copy:(id)sender
{	
	if ([self saveNote]) {
        DBUtil* noteDB = [DBUtil sharedInstance];
        Note* note = noteDB.iCurrentNote; 
        NSString *rawStr = [note iContent];
        
        [UIPasteboard generalPasteboard].string = rawStr;
        
        UIAlertView* alertView = [[UIAlertView alloc] initWithTitle:@"Copy All" message:@"Done, the content can be pasted in other applications now!" delegate:self cancelButtonTitle:@"OK" otherButtonTitles:nil];
        [alertView show];
        [alertView release];
    }
}

- (void)doSmsInApp
{
    DBUtil* noteDB = [DBUtil sharedInstance];
    Note* note = noteDB.iCurrentNote; 
	NSString *rawStr = [note iContent];
	
	[UIPasteboard generalPasteboard].string = rawStr;
	
	[AppDelegate sendSMS:rawStr];
}

- (void)new:(id)sender
{
/*
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:kTransitionDuration];
	[UIView setAnimationTransition:UIViewAnimationTransitionFlipFromLeft
						   forView:self.noteContentView cache:YES];

	if([noteContentView.txtView lengthOfContent]){	
		[self saveNote];
		self.note = nil;
	}
	
	[gAppDelegate.notesListViewCtrl addNewNote];
	noteContentView.imView.hidden		= NO;
	noteContentView.footerBar.hidden	= !noteContentView.imView.hidden;
	[noteContentView layoutSubviews];
	
	[UIView commitAnimations];
 */
}

- (void)delete:(id)sender
{
    UIActionSheet* clearActionSheet = [[UIActionSheet alloc] initWithTitle:nil
                                                         delegate:self 
                                                cancelButtonTitle:@"Cancel" 
                                           destructiveButtonTitle:@"Clear content" 
                                                otherButtonTitles:nil];
    clearActionSheet.actionSheetStyle = UIActionSheetStyleDefault;
    clearActionSheet.tag = 1;
    [clearActionSheet showInView:self.view];  
    [clearActionSheet release];
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{    
    if (actionSheet.tag == 1) {
		switch (buttonIndex) {
            case 0:
            {
                [iNoteContentView setContent:@""];
                NSString* date = [[[Util sharedInstance] dateFormatter] stringFromDate:[NSData data]];
                NSString* text = [NSString stringWithFormat:@"%@, Count:%d", date, 0]; 
                iNoteContentView.iTimeField.text = text;                
                [self.iNoteContentView showInputView:YES];
                
                break;
            }
			default: // Cancel
				break;
		}
        
    } else if (actionSheet.tag == 0) {
		switch (buttonIndex) {
            case 0: // Switch Keyboard
            {
                BOOL quick = YES;
                NSNumber* result = [[NSUserDefaults standardUserDefaults] objectForKey:@"UsingQuickKeyboard"];
                if (result) {
                    quick = [result boolValue];
                }
                [self.iNoteContentView showInputView:NO];
                
                if (quick == YES) {
                    self.iNoteContentView.iContentView.inputView = nil;
                    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithBool:NO] forKey:@"UsingQuickKeyboard"];
                } else {
                    self.iNoteContentView.iContentView.inputView = (UIView*)(self.iNoteContentView.iInputView);
                    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithBool:YES] forKey:@"UsingQuickKeyboard"];
                }
                [[NSUserDefaults standardUserDefaults] synchronize];
                
                [self.iNoteContentView showInputView:YES];
                break;
            }
            case 1:
            {
                [[NSUserDefaults standardUserDefaults] setObject:@"ThumbMode" forKey:@"KeyboardLayout"];
                [[NSUserDefaults standardUserDefaults] synchronize];
                [self.iNoteContentView showInputView:NO];
                IME* ime = [[IMESingleton sharedInstance] instance]; 
                changeKeyboard(ime, "ThumbMode");
                [self.iNoteContentView.iInputView.iKeyboardView changeKeyboard];
                [self.iNoteContentView showInputView:YES];
				break;
                
            }
            case 2:
            {
                [[NSUserDefaults standardUserDefaults] setObject:@"TraceMode" forKey:@"KeyboardLayout"];
                [[NSUserDefaults standardUserDefaults] synchronize];
                [self.iNoteContentView showInputView:NO];
                IME* ime = [[IMESingleton sharedInstance] instance]; 
                changeKeyboard(ime, "TraceMode");
                [self.iNoteContentView.iInputView.iKeyboardView changeKeyboard];
                [self.iNoteContentView showInputView:YES];
				break;
                
            }
            case 3:
            {
                [[NSUserDefaults standardUserDefaults] setObject:@"TraceLeftMode" forKey:@"KeyboardLayout"];
                [[NSUserDefaults standardUserDefaults] synchronize];
                [self.iNoteContentView showInputView:NO];
                IME* ime = [[IMESingleton sharedInstance] instance]; 
                changeKeyboard(ime, "TraceLeftMode");
                [self.iNoteContentView.iInputView.iKeyboardView changeKeyboard];
                [self.iNoteContentView showInputView:YES];
				break;
                
            }
            case 4:
            {
                [[NSUserDefaults standardUserDefaults] setObject:@"NormalMode" forKey:@"KeyboardLayout"];
                [[NSUserDefaults standardUserDefaults] synchronize];
                [self.iNoteContentView showInputView:NO];
                IME* ime = [[IMESingleton sharedInstance] instance]; 
                changeKeyboard(ime, "NormalMode");
                [self.iNoteContentView.iInputView.iKeyboardView changeKeyboard];
                [self.iNoteContentView showInputView:YES];
				break;
                
            }
            case 5:
            {
                BOOL play = YES;
                NSNumber* result = [[NSUserDefaults standardUserDefaults] objectForKey:@"KeyboardAudio"];
                if (result) {
                    play = [result boolValue];
                }
                
                if (play == YES) {
                    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithBool:NO] forKey:@"KeyboardAudio"];
                } else {
                    [[NSUserDefaults standardUserDefaults] setObject:[NSNumber numberWithBool:YES] forKey:@"KeyboardAudio"];
                }
                [[NSUserDefaults standardUserDefaults] synchronize];
				break;
                
            }
			default: //Cancel
                break;

		}        
    } else if (actionSheet.tag == 3) {
		switch (buttonIndex) {
            case 0: // Publish note to Facebook
            {
                [self publishStream];
            }
                break;
            case 1: // Publish note to Facebook
            {
                [self showCamera];
            }
                break;
            case 2: // Publish note to Facebook
            {
                [self showPhotos];
            }
                break;
			default: //Cancel
                break;
        }
                
        
    }
}

- (void)publishStream 
{
	//we will release this object when it is finished posting
    NSString* content = [self.iNoteContentView content];
    if ((content == nil) || ([content length] == 0)) return;
	FBFeedPost *post = [[FBFeedPost alloc] initWithPostMessage:content];
	[post publishPostWithDelegate:self];
	
	IFNNotificationDisplay *display = [[IFNNotificationDisplay alloc] init];
	display.type = NotificationDisplayTypeLoading;
	display.tag = NOTIFICATION_DISPLAY_TAG;
	[display setNotificationText:@"Posting"];
	[display displayInView:self.view atCenter:CGPointMake(self.view.center.x, self.view.center.y-100.0) withInterval:0.0];
	[display release];        
}

#pragma mark -
#pragma mark FBFeedPostDelegate

- (void) failedToPublishPost:(FBFeedPost*) _post {
    
	UIView *dv = [self.view viewWithTag:NOTIFICATION_DISPLAY_TAG];
	[dv removeFromSuperview];
	
	IFNNotificationDisplay *display = [[IFNNotificationDisplay alloc] init];
	display.type = NotificationDisplayTypeText;
	[display setNotificationText:@"Failed To Post"];
	[display displayInView:self.view atCenter:CGPointMake(self.view.center.x, self.view.center.y-100.0) withInterval:1.5];
	[display release];
	
	//release the alloc'd post
	[_post release];
}

- (void) finishedPublishingPost:(FBFeedPost*) _post {
    
	UIView *dv = [self.view viewWithTag:NOTIFICATION_DISPLAY_TAG];
	[dv removeFromSuperview];
	
	IFNNotificationDisplay *display = [[IFNNotificationDisplay alloc] init];
	display.type = NotificationDisplayTypeText;
	[display setNotificationText:@"Finished Posting"];
	[display displayInView:self.view atCenter:CGPointMake(self.view.center.x, self.view.center.y-100.0) withInterval:1.5];
	[display release];
	
	//release the alloc'd post
	[_post release];
}

#pragma mark -
#pragma mark CameraUtilsDelegate
-(void) imagePickerDidCancel
{
    [_cameraUtils release];
    _cameraUtils = nil;    
}

-(void)videoTaked:(NSString *)path
{
	
}

-(void)photoTakedTask:(UIImage*)image
{  
    NSDate* now = [NSDate date];
	FBFeedPost *post = [[FBFeedPost alloc] initWithPhoto:image name:[NSDate stringFromDate:now]];
	[post publishPostWithDelegate:self];
	
	IFNNotificationDisplay *display = [[IFNNotificationDisplay alloc] init];
	display.type = NotificationDisplayTypeLoading;
	display.tag = NOTIFICATION_DISPLAY_TAG;
	[display setNotificationText:@"Posting Photo"];
	[display displayInView:self.view atCenter:CGPointMake(self.view.center.x, self.view.center.y-100.0) withInterval:0.0];
	[display release];
    [_cameraUtils release];
    _cameraUtils = nil;
}

-(void)photoTaked:(UIImage*)image
{
    UIImage* result = [self scale:image targetSize:CGSizeMake(160.0, 240.0) ignoreSceenScale:NO];
	[self photoTakedTask:result];
}

-(void)previewImage:(NSDictionary*)dictionary
{
    UIImage* image = [dictionary objectForKey:@"0"];
    [self photoTaked:image];
}

-(CGFloat) screenScale:(CGFloat)screenScale
{
	if (screenScale < 0) 
	{
		screenScale = 1.0;
		UIScreen* screen = [UIScreen mainScreen];
		if ([screen respondsToSelector:@selector(scale)])
		{
			screenScale = screen.scale;
		} 
	}
	return screenScale;
}

-(UIImage*) scale:(UIImage*) image targetSize:(CGSize) targetSize ignoreSceenScale:(BOOL)ingoreSceenScale
{
    
	CGFloat screenScale = ingoreSceenScale ? 1.0 : [self screenScale:1.0];
	CGFloat width = targetSize.width * screenScale;
	CGFloat height = targetSize.height * screenScale;
    
	CGContextRef mainViewContentContext; 
	CGColorSpaceRef colorSpace; 
	colorSpace = CGColorSpaceCreateDeviceRGB(); 
	// create a bitmap graphics context the size of the image 
	mainViewContentContext = CGBitmapContextCreate (NULL, width, height, 8, 0, colorSpace, kCGImageAlphaPremultipliedLast); 
	// free the rgb colorspace 
	CGColorSpaceRelease(colorSpace);     
	if (mainViewContentContext==NULL) 
		return NULL;
    
	CGContextDrawImage(mainViewContentContext, CGRectMake(0, 0, width,  height), image.CGImage); 
	CGImageRef mainViewContentBitmapContext = CGBitmapContextCreateImage(mainViewContentContext); 
	CGContextRelease(mainViewContentContext); 
	UIImage *theImage = nil;
//	if ([UIImage respondsToSelector:@selector(imageWithCGImage:scale:orientation:)]) 
//	{
//		theImage = [UIImage imageWithCGImage:mainViewContentBitmapContext scale:screenScale orientation:UIImageOrientationUp]; 
//	}
//	else
//	{
		theImage = [UIImage imageWithCGImage:mainViewContentBitmapContext]; 		
//	}
	
	CGImageRelease(mainViewContentBitmapContext); 
    
	// return the image 
	return theImage; 
}

#pragma mark -
#pragma mark Popover view support

- (void)popoverControllerDidDismissPopover:(UIPopoverController *)popoverController {
}

- (BOOL)popoverControllerShouldDismissPopover:(UIPopoverController *)popoverController {
	return YES;
}

#pragma mark -
#pragma mark Split view support

- (void)splitViewController: (UISplitViewController*)svc willHideViewController:(UIViewController *)aViewController withBarButtonItem:(UIBarButtonItem*)barButtonItem forPopoverController: (UIPopoverController*)pc {
    
    barButtonItem.title = @"Notes";
    [self.navigationItem setLeftBarButtonItem:barButtonItem animated:YES];
    self.popoverCtrl = pc;
}


// Called when the view is shown again in the split view, invalidating the button and popover controller.
- (void)splitViewController: (UISplitViewController*)svc willShowViewController:(UIViewController *)aViewController invalidatingBarButtonItem:(UIBarButtonItem *)barButtonItem {
    [self.navigationItem setLeftBarButtonItem:nil animated:YES];
    self.popoverCtrl = nil;
}

@end
