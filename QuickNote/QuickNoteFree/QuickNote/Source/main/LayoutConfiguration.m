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
        ret = CGRectMake(0.0,  0.0,  320.0,   20.0);
    } else {
        ret = CGRectMake(0.0,  0.0,  0.0,   0.0);
    }
    
	return ret;
}

CGRect rectOfReadOnlyTextView()
{
    CGRect ret;
	int id = layoutID();
    if (id == 0) {
        ret = CGRectMake(0.0,  20.0,  320.0, 368.0);
    } else {
        ret = CGRectMake(0.0,   0.0,  480.0, 271.0);
    }
    
	return ret;
}

CGRect rectOfEditableTextView()
{
    CGRect ret;
	int id = layoutID();
    if (id == 0) {
        ret = CGRectMake(0.0,  20.0,  320.0, 188.0);
    } else {
        ret = CGRectMake(0.0,   0.0,  480.0,  72.0);
    }
    
	return ret;
}

CGRect rectOfInputView()
{
    CGRect ret;
	int id = layoutID();
   if (id == 0) {
        ret = CGRectMake( 0.0, 178.0,  320.0, 258);
    } else {
        ret = CGRectMake(0.0,  62.0,  480.0, 258.0);
    }
    
	return ret;
}

CGRect rectOfFootBar()
{
    CGRect ret;
    int id = layoutID();
    if (id == 0) {
        ret = CGRectMake(0.0, 387.0,  320.0,  49.0);
    } else {
        ret = CGRectMake(0.0, 271.0,  480.0,  49.0 );
    }

    return ret;
}
