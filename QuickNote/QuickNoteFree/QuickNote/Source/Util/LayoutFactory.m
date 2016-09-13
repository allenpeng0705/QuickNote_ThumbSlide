#import  <UIKit/UIKit.h>

#define IOS_FOR_IPHONE
//#define IOS_FOR_IPAD

BOOL isPortrait()
{
	BOOL ret = NO;
    NSUInteger orientation = [UIDevice currentDevice].orientation;
	switch (orientation) {
        case UIDeviceOrientationUnknown:
		case UIDeviceOrientationPortrait: 
		case UIDeviceOrientationPortraitUpsideDown:
			ret = YES;
			break;
        case UIDeviceOrientationFaceUp:
        case UIDeviceOrientationFaceDown:
		case UIDeviceOrientationLandscapeLeft:
		case UIDeviceOrientationLandscapeRight:
			ret = NO;
			break;
        default:
            break;
	}
    
	return ret;
}

CGFloat heightOfTimeFieldOfWritingPage()
{
#ifdef IOS_FOR_IPHONE
    return 40.0;
#endif
}

CGFloat heightOfToolbarOfWritingPage() 
{
#ifdef IOS_FOR_IPHONE    
    return 49.0;
#endif
}

