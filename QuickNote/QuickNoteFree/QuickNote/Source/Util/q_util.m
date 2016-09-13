
#include "q_util.h"

char* nativePath(char* aFileName, BOOL aReadonly)
{
	NSString* name = [NSString stringWithUTF8String:aFileName];
	NSString* path;
    
    if (aReadonly == TRUE) {
		path = [[[NSBundle mainBundle] bundlePath] 
                stringByAppendingPathComponent:name];        
    } else {
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString* foldPath = [paths objectAtIndex:0];
		path = [foldPath stringByAppendingPathComponent:name];        
    }
    
	return (char*)[path cStringUsingEncoding:NSUTF8StringEncoding];
}

