#import <UIKit/UIKit.h>
#import <AudioToolbox/AudioToolbox.h>


@interface PopupKeyView : UIView {
    NSString* iContent;
	CFURLRef		soundFileURLRef;
	SystemSoundID	soundFileObject;    
}
@property (nonatomic, retain) NSString* iContent;
@property (readwrite)	CFURLRef		soundFileURLRef;
@property (readonly)	SystemSoundID	soundFileObject;

-(void)showTextWithWidth:(CGFloat)aWidth Height:(CGFloat)aHeight Text:(NSString*)aText WithSound:(BOOL)aFlag;
-(void)hide;
-(void)playSound;
-(void) playFileInSys:(NSString*) filename vibration:(BOOL)vibration;

@end
