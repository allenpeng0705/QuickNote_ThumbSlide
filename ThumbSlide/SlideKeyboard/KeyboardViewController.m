
#import "KeyboardViewController.h"
#import "QIInputView.h"
#import "IMESingleton.h"

@interface KeyboardViewController ()
@property (nonatomic, strong) UIButton *nextKeyboardButton;
@end

@implementation KeyboardViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        iIME = NULL;
    }
    return self;
}

- (void)updateViewConstraints {
    [super updateViewConstraints];
    
    // Add custom view sizing constraints here
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    NSLog(@"Width:%0.2f, Height:%0.2f", self.view.frame.size.width, self.view.frame.size.height);
    int32 kb_width = 0;
    int32 kb_height = 0;
    if (iIME == NULL) {
        iIME = [[IMESingleton sharedInstanceWithInputHandler:self] instance];
    }
    initIME(iIME, "ENGLISH");
    SizeOfKeyboard(CurrentKeyboard(iIME), &kb_width, &kb_height);
    iInputView = [[QIInputView alloc] initWithFrame:CGRectMake(0.0, 0.0, kb_width, kb_height+10)];
    [iInputView attachKeyboard:CurrentKeyboard(iIME)];
    self.inputView = iInputView;
}

- (void) dealloc
{
    if (iIME) {
        destroyIME(iIME);
        iIME = NULL;
    }
}


- (void) viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];

}

- (void) viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
    if (iIME) {
        destroyIME(iIME);
        iIME = NULL;
        self.inputView = nil;
    }
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated
}

- (void)textWillChange:(id<UITextInput>)textInput {
    // The app is about to change the document's contents. Perform any preparation here.
    NSLog(@"%s", "textWillChange");
    NSUserDefaults* ds = [[NSUserDefaults alloc] initWithSuiteName:@"group.com.ezinput.thumbslide"];
    NSString* value = (NSString*)[ds objectForKey:@"DataSharing"];
    NSLog(@"DataSharing Test:%s", [value cStringUsingEncoding:NSUTF8StringEncoding]);
    
    if (textInput) {
         NSLog(@"%s", "textInput is not empty");
    }
}

- (void)textDidChange:(id<UITextInput>)textInput {
    // The app has just changed the document's contents, the document context has been updated.
    NSLog(@"%s", "textDidChange");
    if (textInput) {
        NSLog(@"%s", "textInput is not empty");
    }
}

@end
