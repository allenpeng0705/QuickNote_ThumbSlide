#import <Foundation/Foundation.h>


@protocol KeyboardBridge <NSObject>

-(void)sendText:(NSString*)aText WithSpace:(BOOL)aSpaceFlag;
-(void)replaceTextAroundCursor:(NSString*)aText;
-(void)sendBackspace;
-(void)sendEnter;
-(BOOL)shouldCapitalize;
-(BOOL)shouldSendBackspace;
-(NSString*)stringAroundCursor;
-(void)selectWordAroundCursor;
@end
