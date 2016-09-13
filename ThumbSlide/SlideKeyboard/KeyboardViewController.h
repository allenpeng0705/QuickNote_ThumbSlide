
#import <UIKit/UIKit.h>
#import "q_ime.h"

@class QIInputView;
@interface KeyboardViewController : UIInputViewController
{
    QIInputView *iInputView;
    IME* iIME;
}

@end

