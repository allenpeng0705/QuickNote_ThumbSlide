
#import <MobileCoreServices/UTCoreTypes.h>
#import "CameraUtils.h"


@implementation CameraUtils
@synthesize delegate = _delegate;
@synthesize currentImageViewController = _currentImageViewController;

-(id)init
{
    self = [super init];
    if (self)
    {
        UIImage*	image = [UIImage imageNamed:@""];
        UIImage*	image_p = [UIImage imageNamed:@""];
        
        _savedPhotoButton = [UIButton buttonWithType:UIButtonTypeCustom];
        [_savedPhotoButton setImage:image forState:UIControlStateNormal];
        [_savedPhotoButton setImage:image_p forState:UIControlStateHighlighted];
        _savedPhotoButton.frame = CGRectMake(0, 0, image.size.width*1.5, image.size.height);
        [_savedPhotoButton retain];
        [_savedPhotoButton addTarget:self action:@selector(showSavedImage) forControlEvents:UIControlEventTouchUpInside];
    }
    return  self;
}


-(void)dealloc
{
    [_savedPhotoButton release];
    [_currentImageViewController release];
    [super dealloc];
}

-(void)showCamera:(UIViewController*)controller
{
	if ([UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera]) {
        _fromStatusBarStyle = [UIApplication sharedApplication].statusBarStyle;
        _pickerType = EImageFromCamera;
        _currentViewController = controller;
        UIImagePickerController *imagePickerController=[[UIImagePickerController alloc] init];
		imagePickerController.sourceType= UIImagePickerControllerSourceTypeCamera;
		imagePickerController.delegate = self;
        imagePickerController.mediaTypes = [NSArray arrayWithObject:(NSString*)kUTTypeImage];
		imagePickerController.modalTransitionStyle=UIModalTransitionStyleCoverVertical;
        self.currentImageViewController = imagePickerController;
		[controller presentModalViewController:imagePickerController animated:YES];
		[imagePickerController release];
	}
}

-(void)showPhotoLibrary:(UIViewController*)controller animated:(BOOL) animated
{
	if ([UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypePhotoLibrary])
	{
        _fromStatusBarStyle = [UIApplication sharedApplication].statusBarStyle;
        _pickerType = EImage;
        _currentViewController = controller;
        
        
		UIImagePickerController *imagePickerController=[[UIImagePickerController alloc] init];
		imagePickerController.delegate = self;
		imagePickerController.mediaTypes = [NSArray arrayWithObjects:(NSString*)kUTTypeImage,nil];
		imagePickerController.sourceType=UIImagePickerControllerSourceTypePhotoLibrary;
		imagePickerController.modalTransitionStyle=UIModalTransitionStyleCoverVertical;
        self.currentImageViewController = imagePickerController;
		[controller presentModalViewController:imagePickerController animated:animated];
		[imagePickerController release];
	}	
}

- (void) didPhotoTaked:(NSDictionary*)dictionary
{
    if (self.delegate && [self.delegate respondsToSelector:@selector(previewImage:)]) 
    {
        [self.delegate performSelector:@selector(previewImage:) withObject:dictionary];
    }
    UIImagePickerController* picker = [dictionary objectForKey:@"1"];
    [picker dismissModalViewControllerAnimated:NO];
}

#pragma mark UIImagePickerControllerDelegate
- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary *)info
{
	[UIApplication sharedApplication].statusBarStyle = _fromStatusBarStyle;
	NSString *mediaType = [info objectForKey:UIImagePickerControllerMediaType];
	if ([mediaType isEqualToString:@"public.image"])
	{
		UIImage *originalImage;
		
		if (picker.title.length > 0)//photo library
		{
			originalImage = [info objectForKey:UIImagePickerControllerOriginalImage];
            [self didPhotoTaked:[NSDictionary dictionaryWithObjectsAndKeys:originalImage,@"0",picker,@"1",nil]];
            return;
		}
		else //camera
		{
			UIImage *image = [info objectForKey:UIImagePickerControllerOriginalImage];
			if (self.delegate && [self.delegate respondsToSelector:@selector(photoTaked:)]) 
			{
				[self.delegate performSelector:@selector(photoTaked:) withObject:image];
				[CameraUtils savePhotoToCameraRoll:image];
			}
		}
	}
	else if([mediaType isEqualToString:@"public.movie"])
	{
		if (self.delegate && [self.delegate respondsToSelector:@selector(videoTaked:)]) 
		{
			[self.delegate performSelector:@selector(videoTaked:) withObject:nil];
		}
	}
	[picker dismissModalViewControllerAnimated:YES];
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker
{
    [UIApplication sharedApplication].statusBarStyle = _fromStatusBarStyle;
    if (_pickerType == EImageFromCamera) 
    {
        [picker dismissModalViewControllerAnimated:NO];
        //[self showCamera:_currentViewController];
    }
    else
    {
        [picker dismissModalViewControllerAnimated:YES];
        if (self.delegate && [self.delegate respondsToSelector:@selector(imagePickerDidCancel)])
        {
            [self.delegate performSelector:@selector(imagePickerDidCancel)];
        }
    }
}

#pragma mark UINavigationControllerDelegate
- (void)navigationController:(UINavigationController *)navigationController willShowViewController:(UIViewController *)viewController animated:(BOOL)animated{
	[self addSomeElements:viewController];
}



#pragma mark addSomeElements
-(void)addSomeElements:(UIViewController *)viewController{
    if (_pickerType == EImageFromCamera)
    {
     	UIView *PLCameraView=[self findView:viewController.view withName:@"PLCameraView"];

        _savedPhotoButton.hidden = NO;
        UIView *bottomBar=[self findView:PLCameraView withName:@"PLCropOverlayBottomBar"];
        CGRect rect = _savedPhotoButton.frame;
        rect.origin = CGPointMake(viewController.view.frame.size.width*0.8, (bottomBar.frame.size.height - _savedPhotoButton.frame.size.height)/2);
        _savedPhotoButton.frame = rect;
        [bottomBar addSubview:_savedPhotoButton];   
        
        UIImageView *bottomBarImageForCamera = [bottomBar.subviews objectAtIndex:1];
        UIImageView *bottomBarImageForSave = [bottomBar.subviews objectAtIndex:0];

        UIButton *cameraButton=[bottomBarImageForCamera.subviews objectAtIndex:0];
        UIButton *retakeButton=[bottomBarImageForSave.subviews objectAtIndex:0];

        [cameraButton addTarget:self action:@selector(captureAction) forControlEvents:UIControlEventTouchUpInside];
        [retakeButton addTarget:self action:@selector(reTake) forControlEvents:UIControlEventTouchUpInside];
        
    }


}


#pragma mark get/show the UIView we want
-(UIView *)findView:(UIView *)aView withName:(NSString *)name{
	Class cl = [aView class];
	NSString *desc = [cl description];
	
	if ([name isEqualToString:desc])
		return aView;
	
	for (NSUInteger i = 0; i < [aView.subviews count]; i++)
	{
		UIView *subView = [aView.subviews objectAtIndex:i];
		subView = [self findView:subView withName:name];
		if (subView)
			return subView;
	}
	return nil;	
}


-(void)showSavedImage
{
	if ([UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeSavedPhotosAlbum])
	{
        
        [_currentViewController dismissModalViewControllerAnimated:NO];
        _pickerType = EImageFromCamera;
		UIImagePickerController *imagePickerController=[[UIImagePickerController alloc] init];
		imagePickerController.delegate = self;
		imagePickerController.mediaTypes = [NSArray arrayWithObjects:(NSString*)kUTTypeImage,nil];
		imagePickerController.sourceType=UIImagePickerControllerSourceTypePhotoLibrary;
		imagePickerController.modalTransitionStyle=UIModalTransitionStyleCoverVertical;
		[_currentViewController presentModalViewController:imagePickerController animated:YES];
		[imagePickerController release];
	}	
}

-(void)captureAction
{
    _savedPhotoButton.hidden = YES;
}

-(void)reTake
{
    _savedPhotoButton.hidden = NO;
    [_savedPhotoButton removeFromSuperview];
    [self addSomeElements:self.currentImageViewController];
}

+(void)savePhotoToCameraRoll:(UIImage*)photo
{
	UIImageWriteToSavedPhotosAlbum(photo,nil,nil,nil);
}


@end


