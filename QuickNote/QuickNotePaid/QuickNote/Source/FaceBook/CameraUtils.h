
#import <Foundation/Foundation.h>

typedef enum
{
    EImage,
    EImageFromCamera
}EImagePickerType;

@protocol CameraUtilsDelegate<NSObject>

-(void)photoTaked:(UIImage*)image;
-(void)videoTaked:(NSString*)path;
-(void)previewImage:(NSDictionary*)dictionary;

@end
@interface CameraUtils : NSObject<UIImagePickerControllerDelegate,UINavigationControllerDelegate> {
	id<CameraUtilsDelegate>   _delegate;
    EImagePickerType        _pickerType;
    UIViewController*         _currentViewController;
    UIButton*                 _savedPhotoButton;
    UIViewController*         _currentImageViewController;
    UIStatusBarStyle          _fromStatusBarStyle;
}
@property(nonatomic,assign)id<CameraUtilsDelegate>   delegate;
@property(nonatomic,retain)UIViewController*         currentImageViewController;

-(void)showCamera:(UIViewController*)controller;
-(void)showPhotoLibrary:(UIViewController*)controller animated:(BOOL) animated;
+(void)savePhotoToCameraRoll:(UIImage*)photo;

-(UIView *)findView:(UIView *)aView withName:(NSString *)name;
-(void)addSomeElements:(UIViewController *)viewController;
@end

