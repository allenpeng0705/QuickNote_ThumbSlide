#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface Util : NSObject {
    UIFont* iFontOfCandidatesList;
    UIColor* iColorForSelectedCandidate;
    UIColor* iColorForCandidate;
    UIColor* iColorForExactCandidate;
    NSDateFormatter* iDateFormatter;
}

+(Util*)sharedInstance;

-(UIFont*)FontOfCandidatesList;
-(void)SetColorForSelectedCandidate:(UIColor*)aColor;
-(UIColor*)FillColorOfSelectedCandidate;
-(void)SetColorForCandidate:(UIColor*)aColor;
-(UIColor*)FillColorOfCandidate;
-(void)SetColorForExactCandidate:(UIColor*)aColor;
-(UIColor*)FillColorOfExactCandidate;

- (void)makeFileWritable:(NSString*)aFilename;
-(NSDateFormatter*)dateFormatter;

-(void)drawText:(CGContextRef)aCtx AtX:(int)x AtY:(int)y WithContent:(NSString*)aText Font:(UIFont*)aFont Color:(const UIColor*)aColor;
-(void) drawText:(CGContextRef)aCtx InRect:(CGRect)aRect WithContent:(NSString*)aText Font:(UIFont*)aFont Color:(const UIColor*)aColor;
-(void)lineTo:(CGContextRef)aCtx startX:(int)aX1 startY:(int)aY1 endX:(int)aX2 endY:(int)aY2 strokeWidth:(int)aStrokeWidth Color:(UIColor*)aColor;
-(void)drawRect:(CGContextRef)aCtx leftX:(int)aLeft topY:(int)aTop width:(int)aWidth height:(int)aHeight strokeWidth:(int)aStrokeWidth Color:(UIColor*)aColor;
-(void)drawRoundRect:(CGContextRef)aCtx leftX:(int)aLeft topY:(int)aTop width:(int)aWidth height:(int)aHeight strokeWidth:(int)aStrokeWidth Color:(UIColor*)aColor;
-(void)drawEllipse:(CGContextRef)aCtx leftX:(int)aLeft topY:(int)aTop width:(int)aWidth height:(int)aHeight strokeWidth:(int)aStrokeWidth Color:(UIColor*)aColor;
-(void)fillRect:(CGContextRef)aCtx leftX:(int)aLeft topY:(int)aTop width:(int)aWidth height:(int)aHeight Color:(UIColor*)aColor;
-(void)fillRoundRect:(CGContextRef)aCtx leftX:(int)aLeft topY:(int)aTop width:(int)aWidth height:(int)aHeight Color:(UIColor*)aColor;
-(void)fillEllipse:(CGContextRef)aCtx leftX:(int)aLeft topY:(int)aTop width:(int)aWidth height:(int)aHeight Color:(UIColor*)aColor;
-(CGRect)calcTextRect:(CGContextRef)aCtx Text:(NSString*)aText WithFont:(UIFont*)aFont;

// Access the User Dict and Active Dict



@end
