
#import <Foundation/Foundation.h>
#import "QuickNoteDelegate.h"


@interface Util : NSObject {
    CGFloat iFontSizeOfQITextView;
    NSString* iFontNameOfQITextView;
    UIFont* iFontOfQITextView;
    UIFont* iFontOfCandidatesList;
    UILineBreakMode iLineBreakMode;
    NSUInteger iMaxContentlengthOfQITextView;
    NSUInteger iMaxLineNumOfQITextView;
    NSTimeInterval iCursorBlinkIntervalOfQITextView;	
	NSUInteger iCursorWidthOfQITextView;
    UIColor* iTextColorOfQITextView;
    UIColor* iFillColorOfQITextView;
    UIColor* iBackgroundColorOfScrollQITextView;
    UIColor* iColorForSelectedCandidate;
    UIColor* iColorForCandidate;
    UIColor* iColorForExactCandidate;
    CGFloat iFontSizeOfTimeFieldOfWritingPage;
    NSDateFormatter* iDateFormatter;
}

+(Util*)sharedInstance;

-(CGFloat)FontSizeOfQITextView;
-(void)SetFontSizeForQITextView:(CGFloat)aFontSize;
-(NSString*)FontNameOfQITextView;
-(void)SetFontNameForQITextView:(NSString*)aFontName;
-(UIFont*)FontOfQITextView;
-(UIFont*)FontOfCandidatesList;
-(NSUInteger)FontHeightOfTextView;
-(UILineBreakMode)LineBreakModeOfQITextView;
-(NSUInteger) MaxContentLengthOfQITextView;
-(void)SetMaxContentLengthForQITextView:(NSUInteger)aMaxContentLen;
-(NSUInteger) MaxLineNumOfQITextView;
-(void)SetMaxLineNumForQITextView:(NSUInteger)aMaxLineNum;
-(NSTimeInterval)CursorBlinkIntervalOfQITextView;
-(void)SetCursorBlinkIntervalOfQITextView:(NSTimeInterval)aCursorBlinkInterval;
-(NSUInteger)CursorWidthOfQITextView;
-(void)SetCursorWidthOfQITextView:(NSUInteger)aWidthOfCuror;
-(UIColor*)TextColorOfQITextView;
-(void)SetTextColorOfQITextView:(UIColor*)aTextColor;
-(UIColor*)FillColorOfQITextView;
-(void)SetFillColorOfQITextView:(UIColor*)aFillColor;
-(UIEdgeInsets)EdgeOfScrollQITextView;
-(UIEdgeInsets)EdgeOfQITextView;
-(void)SetBackgroundColorForScrollQITextView:(UIColor*)aColor;
-(UIColor*)BackgroundColorOfScrollQITextView;
-(NSString*)BeginEditingNotification;
-(NSString*)TextWillChangeNotification;
-(NSString*)TextChangedNotification;
-(NSString*)SelectionChangedNotification;
-(NSString*)EndEditingNotification;
-(CGFloat)FontSizeOfTimeFieldOfWritingPage;
-(void)SetFontSizeForTimeFieldOfWritingPage:(CGFloat)aFontSize;
-(void)SetColorForSelectedCandidate:(UIColor*)aColor;
-(UIColor*)FillColorOfSelectedCandidate;
-(void)SetColorForCandidate:(UIColor*)aColor;
-(UIColor*)FillColorOfCandidate;
-(void)SetColorForExactCandidate:(UIColor*)aColor;
-(UIColor*)FillColorOfExactCandidate;

- (void)makeFileWritable:(NSString*)aFilename;
- (NSString*)versionOfApp;
-(BOOL) checkVersion;
- (BOOL)executeSQLScript:(NSString *)aScript OnDBNamed:(NSString *)aDBName;
- (BOOL) isSQLComment:(NSString *) aStatementFragment;
-(CGRect)makeRectForFootBar;
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

-(QuickNoteDelegate*) appDelegate;
-(int)maxCountOfNotes;

// Access the User Dict and Active Dict



@end
