
#import "FBFeedPost.h"
#import "CameraUtils.h"

@class Note,NoteContentView;

@interface NoteViewController : UIViewController <	UIActionSheetDelegate,UIPopoverControllerDelegate, 
    UISplitViewControllerDelegate,
    FBFeedPostDelegate, CameraUtilsDelegate>
{
    UIPopoverController *popoverCtrl;
    NoteContentView *iNoteContentView;	
    UIBarButtonItem *iDoneBtn;
	UIBarButtonItem *iSettingBtn;        
    UIInterfaceOrientation iToOrientation;
    UIInterfaceOrientation iFromOrientation;
    
    UIButton* iOptionBtn;
    NSArray* _permissions;
    
    CameraUtils* _cameraUtils;
    BOOL _gotoCameraRoll;
}
@property (nonatomic, retain)UIPopoverController *popoverCtrl;
@property (nonatomic,retain) NoteContentView    *iNoteContentView;
@property (nonatomic,retain) UIBarButtonItem    *iDoneBtn;
@property (nonatomic,retain) UIBarButtonItem	*iSettingBtn;
@property (nonatomic,retain) UIButton           *iOptionBtn;
@property (nonatomic,retain) CameraUtils        *cameraUtils;


- (BOOL)saveNote;
- (BOOL)saveNoteWithChecking:(BOOL)refresh;
- (void)updateTitle;
- (void)setupNote:(BOOL)showKeyboard;

- (void)enableDoneButton;
- (void)disableDoneButton;
- (void)enableSettingButton;
- (void)disableSettingButton; 
- (void)doneAction:(id)sender;
- (void)post:(id)sender;

- (void)publishStream;
@end
