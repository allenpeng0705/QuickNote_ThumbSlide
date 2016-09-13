
#import "NotesListViewController.h"
#import "NoteViewController.h"
#import "NoteContentView.h"
#import <assert.h>
#import "Note.h"

#import "LayoutConfiguration.h"
#import "DBUtil.h"
#import "Util.h"
#import "QuickNoteDelegate.h"
#import "IFNNotificationDisplay.h"
#import "NSDate+Helper.h"

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

@synthesize iNoteContentView;
@synthesize iDoneBtn;
@synthesize iSettingBtn;
@synthesize iOptionBtn;
@synthesize cameraUtils = _cameraUtils;

- (id)init
{
    self = [super init];  
    
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector (keyboardDidShow:)
												 name: UIKeyboardDidShowNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector (keyboardDidHide:)
												 name: UIKeyboardDidHideNotification object:nil];
    iOptionBtn = nil;
    _permissions =  [[NSArray arrayWithObjects:
                      @"read_stream", @"offline_access",nil] retain];
	return self;
}

- (void)dealloc 
{
    [iDoneBtn release];
    [iSettingBtn release];
    [iNoteContentView release];
    [iOptionBtn release];
    [_permissions release];
    [_cameraUtils release];
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
    CGRect appFrame  = [[UIScreen mainScreen] bounds];
//	appFrame.origin.y -= 20.0;
	
	UIView *contentView = [[UIView alloc] initWithFrame:appFrame];
	self.view = contentView;
	[contentView release];
	
	iNoteContentView = [[NoteContentView alloc] initWithFrame:appFrame];
	[self footerBarSetup:iNoteContentView.iToolBar];
    [iNoteContentView setAutoresizingMask:UIViewAutoresizingFlexibleTopMargin|UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleWidth];
	[self.view addSubview:iNoteContentView];
	
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
			
//			[SwiKeyboardView sharedInstanceWithOrientation:interfaceOrientation];
			break;
		case UIInterfaceOrientationLandscapeLeft:
		case UIInterfaceOrientationLandscapeRight:
//			[[UIApplication sharedApplication] setStatusBarHidden:YES];
			self.navigationController.navigationBarHidden = YES;

//			[SwiKeyboardView sharedInstanceWithOrientation:interfaceOrientation];
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
    
    DBUtil* noteDB = [DBUtil sharedInstance];
    Note* note = noteDB.iCurrentNote;
    [note setIContent:[iNoteContentView content]];
    [noteDB saveNote:note];
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
    
    UIActionSheet* optionActionSheet = [[UIActionSheet alloc] initWithTitle:nil
                                                     delegate:self 
                                                    cancelButtonTitle:@"Cancel"
                                            destructiveButtonTitle:nil 
                                            otherButtonTitles:quick_text, button_text, nil];
    optionActionSheet.actionSheetStyle = UIActionSheetStyleDefault;
    optionActionSheet.tag = 0;

    [optionActionSheet showInView:self.view]; 
    [optionActionSheet release];
    [button_text release];
    [quick_text release];
}

- (void)viewWillAppear:(BOOL)animated 
{
//	[noteContentView addSubview:noteContentView.imView];
//	noteContentView.imView.rotateDelegate = self;
	[super viewWillAppear:animated];
    
    _gotoCameraRoll = NO;

    if (iOptionBtn == nil) {
        CGRect rect = CGRectZero;
        rect.size.width = 100;
        rect.size.height = self.navigationController.navigationBar.frame.size.height - 20;
        rect.origin.x = 110;
        rect.origin.y = 10;
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
    
/*	
	if([gAppDelegate launching])
	{
		QuitState *quitState = gAppDelegate.quitState;
		
		int pos = [quitState noteCursorIndex];
		if( [quitState isEditing] )
		{
			[noteContentView.txtView setCursorPosition:pos length:0];
			[noteContentView setSwKeyboardVisible:YES];
			[self enableDoneButton];
//			[self disableInfoButton];
			[noteContentView.txtView ensureCursorVisible];
		}
		else if( [quitState isViewing] )
		{
			[noteContentView.txtView setCursorPosition:pos length:0];
			[noteContentView setSwKeyboardVisible:NO];
//			[self disableDoneButton];
			[self enableInfoButton];
			[noteContentView.txtView ensureCursorVisible];
		}
		
		gAppDelegate.launching = NO;
	}
	else
	{
 */

//	}

}

- (void)viewDidAppear:(BOOL)animated 
{
	self.navigationItem.titleView = nil;
}

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

- (void)saveNote
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
    
    emailItem.width   = 50;
	smsItem.width     = 50;
	copyItem.width    = 50;
	clearItem.width   = 50;
    sendItem.width    = 50;
	
	toolbar.items = [NSArray arrayWithObjects:spaceItem,emailItem,spaceItem,smsItem,spaceItem,copyItem, spaceItem, clearItem,spaceItem, sendItem, spaceItem, nil];
}

- (void)email:(id)sender
{	
	[self saveNote];
	[self doEmailInApp];
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
	[self saveNote];
	[self doSmsInApp];
}

- (void)copy:(id)sender
{	
	[self saveNote];
    DBUtil* noteDB = [DBUtil sharedInstance];
    Note* note = noteDB.iCurrentNote; 
	NSString *rawStr = [note iContent];
	
	[UIPasteboard generalPasteboard].string = rawStr;
    
    UIAlertView* alertView = [[UIAlertView alloc] initWithTitle:@"Copy All" message:@"Done, the content can be pasted in other applications now!" delegate:self cancelButtonTitle:@"OK" otherButtonTitles:nil];
    [alertView show];
    [alertView release];
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
            case 1: // Set the sound for key press
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



@end
