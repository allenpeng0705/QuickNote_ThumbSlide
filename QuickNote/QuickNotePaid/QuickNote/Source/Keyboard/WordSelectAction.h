
#import <Foundation/Foundation.h>

@protocol WordSelectAction <NSObject>

@required
-(void) selectWord:(NSString*)aWord AtIndex:(NSUInteger)aIndex OnListView:(UIView*)aView;

@end
