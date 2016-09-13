#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@protocol KeyboardOutputListener <NSObject>

@required
    -(void)onResultsReceived:(NSArray*)aCandidates;

@end
