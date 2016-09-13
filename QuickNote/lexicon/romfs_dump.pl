#!/usr/bin/env perl
#
# Copyright 2003 Aplix Corporation. All rights reserved.
#
# $Id: romfs_dump.pl,v 1.1.1.1 2004/02/24 07:56:31 matsui Exp $


(my $c0 = $0) =~ s!.*/!!;


die "usage: $c0 filename\n" if $#ARGV != 0;

my $infile = $ARGV[0];
open(IN, $infile) or die "$c0: $infile: $!\n";

my $size = -s $infile;

print <<"EOD";
static const union t {
    unsigned char c[$size];
    long l;
} romfs_data = {{
EOD

my $cnt = 0;
while ($_ = getc(IN), defined $_) {
	print ord($_), ",";
	print "\n" if (++$cnt % 20) == 0;
}
print "\n}};\n\n";

print <<"EOD";
struct romfs_head;
const struct romfs_head *const jb_romfs 
	= (const struct romfs_head *) &romfs_data;

EOD

close(IN);
exit 0;
