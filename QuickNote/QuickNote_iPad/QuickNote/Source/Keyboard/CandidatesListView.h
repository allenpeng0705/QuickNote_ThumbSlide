#import <UIKit/UIKit.h>

#import "KeyboardOutputListener.h"
#import "WordSelectAction.h"

typedef struct Page Page;
typedef struct WordInfo WordInfo;

@interface CandidatesListView : UIView {
    NSUInteger iPosX;
    NSUInteger iPosY;
    NSUInteger iWidth;
    NSUInteger iHeight;
    CGFloat iStartScrollX;
    CGFloat iScrollX;
    
    id<WordSelectAction> iWordSelectedListener;
    NSUInteger iIndexOfSelectedCandidates;
    NSUInteger iIndexOfCurrentPage;
    NSUInteger iNumOfPages;
    Page* iPages;
    WordInfo* iWords;
    NSUInteger iNumOfCandidates;
    UIColor* iColorForCandidate;
    UIColor* iColorForSelectedCandidate;
    UIColor* iColorForExactCandidate;
    
    CGContextRef iMainGraphicCtx;
    CGLayerRef iBackgroundLayerRef;
    BOOL iShouldDrawExactWord;
}

@property (nonatomic,retain) id<WordSelectAction> iWordSelectedListener;
@property (nonatomic)NSUInteger iPosX;
@property (nonatomic)NSUInteger iPosY;
@property (nonatomic)NSUInteger iWidth;
@property (nonatomic)NSUInteger iHeight;
@property (nonatomic)BOOL iShouldDrawExactWord;

-(void)setCandidates:(NSArray*)aCandidates WithSelectIndex:(NSUInteger)aIndex;
-(NSUInteger)indexOfTouchWord:(CGFloat)aPointX;
-(void)setWordSelectedListener:(id<WordSelectAction>)aWordSelectedListener;

@end

