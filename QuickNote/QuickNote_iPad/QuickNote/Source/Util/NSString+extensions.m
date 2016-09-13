#import "NSString+extensions.h"

@implementation NSString (Extensions)

- (NSRange)getWordRangeAtIndex:(NSUInteger)aIndex
{
	NSRange wordRange;
	unsigned short c;
	int first = 0;
	int last  = 0;
	int len = self.length;
	
	if (aIndex >= len) {
		wordRange.location = aIndex;
		wordRange.length   = 0;
		return wordRange;
	}
	
	c = [self characterAtIndex:aIndex];
	
	if (isalpha(c) == 0) {
		if (aIndex == 0) {
			first = aIndex;
			last  = aIndex;
		} else {
			c = [self characterAtIndex:aIndex - 1];
			if (isalpha(c) == 0) {
				first = aIndex;
				last  = aIndex;
			} else {
				last  = aIndex;
				first = aIndex -1;
				while ( first >= 0 ) {
					c = [self characterAtIndex:first];
					if (isalpha(c)) {
						first--;
					} else {
						break;
					}
				}
				
				if (first < 0) first = 0;
				
				if (isalpha([self characterAtIndex:first]) == 0) first++;
				
				if (first >= last) first = last = aIndex;
			}
		}
	} else {
		first = aIndex;
		while (first >= 0) {
			c = [self characterAtIndex:first];
			if (isalpha(c)) {
				first--;
			} else {
				break;
			}
		}
		if (first < 0) first = 0;

		if (isalpha([self characterAtIndex:first]) == 0) first++;
		last = aIndex;
		while (last < len) {
			c = [self characterAtIndex:last];
			if (isalpha(c)) {
				last++;
			} else {
				break;
			}			
		}
		if (last > len) last = len;
		if (first >= last) first = last = aIndex;
	}
	
	wordRange.location = first;
	wordRange.length = last-first;
	
	return wordRange;
}

@end
