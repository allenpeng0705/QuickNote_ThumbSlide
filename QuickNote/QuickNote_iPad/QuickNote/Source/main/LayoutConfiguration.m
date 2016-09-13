/*
 *  LayoutConfiguration.c
 */
#import  <UIKit/UIKit.h>
#include "LayoutConfiguration.h"

BOOL isDevicePortrait()
{
	BOOL bPortrait = NO;
    
    UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
    if (UIInterfaceOrientationIsPortrait(orientation) == YES) {
        bPortrait = YES;
    }
	return bPortrait;
}

int layoutID()
{
	int id = 0;
	BOOL portrait = isDevicePortrait();
    if (portrait == YES) {
        id = 0;
    } else {
        id = 1;
    }
	return id;
}

CGRect rectOfTimeField()
{
    CGRect ret;
	int id = layoutID();
    if (id == 0) {
        ret = CGRectMake(0.0,  0.0,  768.0,   44.0);
    } else {
        ret = CGRectMake(0.0,  0.0,  703.0,   44.0);
    }
    
	return ret;
}

CGRect rectOfReadOnlyTextView()
{
    CGRect ret;
	int id = layoutID();
    if (id == 0) {
        ret = CGRectMake(0.0,  44.0,  768.0, 916.0);
    } else {
        ret = CGRectMake(0.0,   44.0,  703.0, 704.0);
    }
    
	return ret;
}

CGRect rectOfEditableTextView()
{
    CGRect ret;
	int id = layoutID();
    if (id == 0) {
        ret = CGRectMake(0.0, 44.0, 768.0, 596.0);
    } else {
        ret = CGRectMake(0.0, 44.0, 703.0, 384.0);
    }
    
	return ret;
}

CGRect rectOfInputView()
{
    CGRect ret;
	int id = layoutID();
   if (id == 0) {
        ret = CGRectMake(0.0, 596.0,  768.0, 384.0);
    } else {
        ret = CGRectMake(0.0, 0.0,  1024.0, 384.0);
    }
    
	return ret;
}

CGRect rectOfFootBarOfPopList()
{
    CGRect ret;
    int id = layoutID();
    if (id == 0) {
        ret = CGRectMake(0.0, 916.0,  320.0,  49.0);
    } else {
        ret = CGRectMake(0.0, 660.0,  320.0,  49.0 );
    }
    
    return ret;
}

CGRect rectOfFootBar()
{
    CGRect ret;
    int id = layoutID();
    if (id == 0) {
        ret = CGRectMake(0.0, 916.0,  768.0,  49.0);
    } else {
        ret = CGRectMake(0.0, 660.0,  703.0,  49.0 );
    }

    return ret;
}
