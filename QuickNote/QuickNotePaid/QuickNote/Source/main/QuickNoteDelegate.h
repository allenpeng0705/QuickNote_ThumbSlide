#include <sqlite3.h>
#import <MessageUI/MessageUI.h>
#import <UIKit/UIKit.h>

@class NoteViewController, NotesListViewController;

#define AppDelegate ((QuickNoteDelegate*)[[UIApplication sharedApplication]  delegate])
@interface QuickNoteDelegate : NSObject <UIApplicationDelegate, MFMailComposeViewControllerDelegate,UIAlertViewDelegate, MFMessageComposeViewControllerDelegate>
{
    UIWindow                *window;
	UINavigationController	*navigationController;
    NotesListViewController *notesListViewCtrl;	
	
	NSTimer                 *autoSaveTimer;
	NSTimeInterval           autoSaveInterval;	
	NSTimer                 *adMobRefreshTimer;
	
	//import
	NSURL                   *incomeURL;
    
    //  UIWindow                *progressIndicatorWindow;
    //	UIActivityIndicatorView *progressIndicatorView;
}

@property (nonatomic, retain) UIWindow                  *window;
@property (nonatomic, retain) UINavigationController	*navigationController;
@property (nonatomic, retain) NotesListViewController   *notesListViewCtrl;

- (void)sendMailWithSubject:(NSString*)subject Body:(NSString*)body;
- (void)sendSMS:(NSString*)aContent;
- (void)loadApplication:(UIApplication *)application;
- (void)autoSaveTimerSuspend;
- (void)autoSaveTimerResume;

@end
