
#import <UIKit/UIKit.h>

@protocol KeyboardInteractionListener <NSObject>

@required
    -(void)onChangeToShiftMode:(BOOL)aFlag;
    -(void)onChangeKeyboard:(NSString*)aName;
    -(void)onHide;
    -(void)onRepaint;
@end