
#import <Foundation/Foundation.h>
#import "KeyboardBridge.h"

@protocol WordSelectAction <NSObject>

@required
-(void) selectWord:(NSString*)aWord AtIndex:(NSUInteger)aIndex OnListView:(UIView*)aView;

@property(nonatomic, assign) UIInputViewController* iInputHandler;
@property (nonatomic, assign) id<KeyboardBridge>iKeyboardBridge;
@end
