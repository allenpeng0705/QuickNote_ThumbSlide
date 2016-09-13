#import <Foundation/NSCharacterSet.h>

#import "QuickNoteDelegate.h"
#import "NoteViewController.h"
#import "NotesListViewController.h"
#import "Note.h"
#import "NoteContentView.h"
#import "DBUtil.h"
#import "Appirater.h"

#import <assert.h>

#define TAG_IMPORT_NOTES 1111

@interface QuickNoteDelegate (Private)
- (void)autoSaveTimerAction:(id)timer;
@end

@implementation QuickNoteDelegate

@synthesize window, navigationController, notesListViewCtrl;

#pragma mark -
#pragma mark Application start/end

- (void)dealloc 
{
    if ((autoSaveTimer != nil) && ([autoSaveTimer isValid])) {
		[autoSaveTimer invalidate];
		autoSaveTimer = nil;        
    }
	[adMobRefreshTimer release];

    [notesListViewCtrl release];
    [navigationController release];
	[window release];
	
//	[progressIndicatorWindow release];
//	[progressIndicatorView release];
    
    [DBUtil destroy];
	
	[super dealloc];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    [[DBUtil sharedInstance] storeUserWords];
    [[DBUtil sharedInstance] storeActiveWords];
}

- (void)applicationDidFinishLaunching:(UIApplication *)application 
{
/*
	progressIndicatorWindow = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	
	CGRect frame = CGRectMake(140,300,40,40);
	progressIndicatorView = [[UIActivityIndicatorView alloc] initWithFrame:frame];
	[progressIndicatorView startAnimating];
	progressIndicatorView.activityIndicatorViewStyle = UIActivityIndicatorViewStyleWhiteLarge;
	[progressIndicatorView sizeToFit];
	progressIndicatorView.autoresizingMask = (UIViewAutoresizingFlexibleLeftMargin |
											  UIViewAutoresizingFlexibleRightMargin |
											  UIViewAutoresizingFlexibleTopMargin |
											  UIViewAutoresizingFlexibleBottomMargin);
	
	UIImageView *splashImageView = [[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"Default.png"]] autorelease];
	
	[progressIndicatorWindow addSubview:splashImageView];
	[splashImageView addSubview:progressIndicatorView];
	[progressIndicatorWindow makeKeyAndVisible];
	
	[self performSelectorOnMainThread:@selector(loadApplication:) withObject:application waitUntilDone:NO];
	return;
 */
    autoSaveTimer = nil;
    [self loadApplication:application];

}

- (void)loadApplication:(UIApplication *)application
{	
	application.applicationSupportsShakeToEdit = YES;
	
	DBUtil* noteDB = [DBUtil sharedInstance];
    [noteDB loadNotes];
	noteDB.iSearchResult = nil;
    
	[[UIApplication sharedApplication] setStatusBarHidden:YES];
	UIWindow *mainwindow = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	self.window = mainwindow;
	[mainwindow release];
	
    //Create notesListViewCtrl and noteViewCtrl
    NotesListViewController *notelistViewCtl = [[NotesListViewController alloc] init];
	self.notesListViewCtrl = notelistViewCtl;
	[notelistViewCtl release];
	
	// create a navigation controller using the new controller
	UINavigationController *navCtl = [[UINavigationController alloc] initWithRootViewController:notesListViewCtrl];
	self.navigationController = navCtl;
	[navCtl release];
	
	navigationController.navigationBar.barStyle = UIBarStyleBlackOpaque;
	[window addSubview:[navigationController view]];

	/*
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:0.5];
	[UIView setAnimationCurve:UIViewAnimationCurveEaseInOut];
	[UIView setAnimationTransition:UIViewAnimationTransitionNone forView:progressIndicatorWindow cache:YES];
	
	[progressIndicatorView stopAnimating];
	[progressIndicatorView removeFromSuperview];
	progressIndicatorWindow.frame = CGRectZero;
	progressIndicatorWindow.hidden = YES;
	[UIView commitAnimations];
     */
    
    [window makeKeyAndVisible];
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application 
{
    [[DBUtil sharedInstance] saveAllNotes];
}

/*
 1. save all changes in memory to the database
 2. close the database
 */
- (void)applicationWillTerminate:(UIApplication *)application 
{
    [[DBUtil sharedInstance] saveAllNotes];
}

#pragma mark -
#pragma mark Note auto save

- (void)autoSaveTimerSuspend
{
	if (autoSaveTimer && [autoSaveTimer isValid]) {
		[autoSaveTimer invalidate];
		autoSaveTimer = nil;
	}
}

- (void)autoSaveTimerResume
{
	if(autoSaveTimer && [autoSaveTimer isValid]) return;
	
    autoSaveTimer = [NSTimer scheduledTimerWithTimeInterval: 20 
                                                     target: self
                                                   selector: @selector(autoSaveTimerAction:)
                                                   userInfo: nil
                                                    repeats: YES];
}

- (void)autoSaveTimerAction:(id)timer
{
    if (notesListViewCtrl.iNoteContentViewCtrl == navigationController.topViewController) {
        DBUtil* noteDB = [DBUtil sharedInstance];
        Note* note = noteDB.iCurrentNote;
        [noteDB saveNote:note];
    } else {
        [self autoSaveTimerSuspend];
    }
}

#pragma mark -
#pragma mark Application become/resign active

- (void)applicationDidBecomeActive:(UIApplication *)application
{	
    [self autoSaveTimerResume];
    [Appirater appEnteredForeground:YES];
}

- (void)applicationWillResignActive:(UIApplication *)application
{	
    [self autoSaveTimerSuspend];
}

#pragma mark Mail sending utility

// After iPhone OS 3.0
- (void)sendMailWithSubject:(NSString*)subject Body:(NSString*)body
{
	MFMailComposeViewController* mailCtl = [[[MFMailComposeViewController alloc] init] autorelease];
	[mailCtl setSubject:subject];
	[mailCtl setMessageBody:body isHTML:NO];
	mailCtl.mailComposeDelegate = self;
	
	[navigationController presentModalViewController:mailCtl animated:YES];
}

- (void)mailComposeController:(MFMailComposeViewController *)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError *)error
{
	switch(result)
	{
		case MFMailComposeResultCancelled: /*User cancelled...*/
			break;
		case MFMailComposeResultSaved: /*Saved to Mail Drafts...*/
			break;
		case MFMailComposeResultSent: /*Queued by system for sending (NOT necessarily sent)*/
			break;
		case MFMailComposeResultFailed: /*Failed to queue*/
			break;
	}
	[navigationController dismissModalViewControllerAnimated:YES];
}

- (void)sendSMS:(NSString*)aContent
{
    MFMessageComposeViewController *controller = [[[MFMessageComposeViewController alloc] init] autorelease];  
    if([MFMessageComposeViewController canSendText]) {  
        controller.body = aContent;  
        controller.recipients = nil;
        controller.messageComposeDelegate = self;  
        [navigationController presentModalViewController:controller animated:YES];  
    } 
}

- (void)messageComposeViewController:(MFMessageComposeViewController *)controller didFinishWithResult:(MessageComposeResult)result  
{  
    switch (result) {  
        case MessageComposeResultCancelled:  
            break;  
        case MessageComposeResultFailed: 
        {
            UIAlertView* alert = [[UIAlertView alloc] initWithTitle:@"QuickNote" message:@"Unknown Error"  
                                                           delegate:self cancelButtonTitle:@"OK" otherButtonTitles: nil];  
            [alert show];  
            [alert release];
        }
            break;  
        case MessageComposeResultSent:  
            
            break;  
        default:  
            break;  
    }  
    
    [navigationController dismissModalViewControllerAnimated:YES];  
}  

-(void)applicationWillEnterForeground:(UIApplication *)application
{
    
}

- (BOOL)application:(UIApplication *)application handleOpenURL:(NSURL *)url {
    return [[self.notesListViewCtrl.iNoteContentViewCtrl facebook] handleOpenURL:url];
}

@end
