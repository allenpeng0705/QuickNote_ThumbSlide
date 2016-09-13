
#import <Foundation/Foundation.h>


@interface ImageFactory : NSObject {
    NSMutableDictionary* iImageDictionary;
}

+(ImageFactory*)sharedInstance;
-(id)init;
-(UIImage*)BackgroundOfTextView;
-(UIImage*)LineDividerOfTextView;
-(UIImage*)SelectionOfTextView;
-(UIImage*)BackgroundOfToolbar;
-(UIImage*)ImageWithName:(NSString*)aName;
-(UIImage*)BgOfPortraitCandidatesList;
-(UIImage*)HighlightOfPortraitCandidatesList;


@end
