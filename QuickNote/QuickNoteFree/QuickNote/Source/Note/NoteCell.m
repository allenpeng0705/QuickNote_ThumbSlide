#import "NoteCell.h"

#import "Util.h"

@implementation NoteCell

@synthesize iTitle;
@synthesize iDate;
@synthesize iIndexPath;

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier
{
	if ((self = [super initWithStyle:style reuseIdentifier:reuseIdentifier])) {			
        // layoutSubViews will decide the final frame
		self.iTitle = [[[UILabel alloc] initWithFrame:CGRectZero] autorelease];
		iTitle.backgroundColor = [UIColor clearColor];
		iTitle.textColor = [[Util sharedInstance] TextColorOfQITextView];
		iTitle.highlightedTextColor = [UIColor whiteColor];
		iTitle.font = [UIFont boldSystemFontOfSize:16];
		[self.contentView addSubview:iTitle];
		
		self.iDate = [[[UILabel alloc] initWithFrame:CGRectZero] autorelease];
		iDate.backgroundColor = [UIColor clearColor];
		iDate.textColor = [[Util sharedInstance] TextColorOfQITextView];
		iDate.highlightedTextColor = [UIColor whiteColor];
		iDate.font = [UIFont systemFontOfSize:10];
		iDate.textAlignment = UITextAlignmentLeft;
		[self.contentView addSubview:iDate];
	}
	return self;
}

- (void)layoutSubviews
{
	[super layoutSubviews];
	
	CGRect baseRect,rect;
	
	if(self.editing) {
		baseRect = CGRectInset(self.contentView.bounds,  5, 5);
    } else {
		baseRect = CGRectInset(self.contentView.bounds, 10, 5);
    }
	
	rect = baseRect;	
	rect.origin.x    = baseRect.origin.x;
	rect.origin.y    = baseRect.origin.y;
    rect.size.width  = baseRect.size.width;
    rect.size.height = baseRect.size.height*2.0f/3.0f;
    iTitle.frame = rect;
	
	rect.origin.x    = baseRect.origin.x;
	rect.origin.y    = baseRect.origin.y + baseRect.size.height*2.0f/3.0f;
    rect.size.width  = baseRect.size.width;
    rect.size.height = baseRect.size.height*1.0f/3.0f;
    iDate.frame = rect;
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
	[super setSelected:selected animated:animated];
	iTitle.highlighted = selected;
}

- (void)dealloc
{
	[iTitle release];
	[iDate release];
	[iIndexPath release];
    [super dealloc];
}

@end
