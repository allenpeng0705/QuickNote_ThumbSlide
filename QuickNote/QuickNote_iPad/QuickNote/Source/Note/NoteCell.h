@class Note;

@interface NoteCell : UITableViewCell 
{
	UILabel			*iTitle;
	UILabel			*iDate;
	NSIndexPath     *iIndexPath;
}

@property (nonatomic, retain) UILabel     *iTitle;
@property (nonatomic, retain) UILabel     *iDate;
@property (nonatomic, retain) NSIndexPath *iIndexPath;

@end
