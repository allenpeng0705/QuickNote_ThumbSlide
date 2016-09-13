#!/usr/bin/env perl
#
# Copyright 2003 Aplix Corporation. All rights reserved.
#
# $Id: romfile.pl,v 1.2 2004/06/18 06:57:29 matsui Exp $ 


$opt_old = 1;


$romfile_magic = 0x55ff55ff;
$romfile = '-';


$h_size = 8;

&usage if $#ARGV < 0;

$_ = shift;
while ($_ ne '') {
	m/^c/ && do { $opt_c = 1; s/^.//; next; };
	m/^t/ && do { $opt_t = 1; s/^.//; next; };
	m/^x/ && do { $opt_x = 1; s/^.//; next; };
	m/^v/ && do { $opt_v = 1; s/^.//; next; };
	m/^f/ && do { $opt_f = shift; s/^.//; next; };
	&usage("unknown option.  $_");
}
while ($_ = $ARGV[0], m/^-/) {
	shift;
	m/^-C$/		&& do { $opt_C = shift; next; };
	m/^-C(.+)/	&& do { $opt_C = $1; next; };
	m/^-dump$/	&& do { $opt_dump = 1; next; };
	m/^-old$/	&& do { $opt_old = 1; next; };
	m/^-el$/	&& do { $opt_el = 1; next; };
	m/^-eb$/	&& do { $opt_el = 0; next; };
	m/^--exclude=(.*)$/	&& do { $opt_exc = $1; next; };
	m/^--$/		&& last;
	&usage;
}

&usage('files ... please')	if $#ARGV == -1;
&usage('c? t? x?')		if ($opt_c + $opt_t + $opt_x != 1);
&usage("invalid romfile name")	if (defined $opt_f && $opt_f eq '');
sub usage()
{
	my ($msg) = @_;
	die ((defined $msg ? "$msg\n" : '')
		. "$0: {ctx}[vf] [romfile] [-C dir] [(-el|-eb)] files ...\n"
		. "eg.\n"
		. "\tromfile cf romfile.dat files\n"
		. "\tromfile tvf romfile.dat\n"
		. "\tromfile xf romfile.dat\n"
		. "\tetc...\n");
	# cvf, tvf, xvf ,,,,  [-C dir]
}

if ($opt_el) {		# big-endian is the default
	$ppat2 = 'VV';
	$ppat3 = 'VVV';
} else {
	$ppat2 = 'NN';
	$ppat3 = 'NNN';
}


$romfile = $opt_f if $opt_f ne '';

if ($opt_c) {
	&cvf(@ARGV);
} elsif ($opt_t || $opt_x) {
	&xvf(@ARGV);
}
exit 0;


sub cvf()
{
	open(FF, ">$romfile") or die "$0: cannot create $romfile\n";
	if (defined $opt_C) {
		chdir $opt_C or die "$0: cannot chdir to '$opt_C'\n";
	}

	@files = ();
	foreach ( @ARGV ) {
		if ( -f $_ ) {
			s:^\./::;
			push @files, $_;
		} elsif ( -d $_ ) {
			my @a = `find $_ -follow -type f -print`;
			chop @a;
			map s:^\./::o, @a;
			if (defined $opt_exc) {
				@a = grep ! m#(^|/)$opt_exc(/|$)#o, @a;
			}
			push @files, @a;
		}
	}
	{ # uniq
		my %h;
		foreach ( @files ) {
			$h{$_} = 1;
		}
		@files = sort keys %h;
	}

	$i = 0;
	my $ino = 1;
	my %ino;
	for ( @files ) {
		$stat{$_} = $r = [stat];
		die unless defined $r;

		$k = join(',', @$r[0..1]);
		if (exists $devino{$k}) {
			$shared{$_} = $devino{$k};	# body data shared
		} else {
			$devino{$k} = $_;
			$ino{$k} = $ino++;
		}
		$i++;
	}

	#for ( @files ) {
	#	$r = $stat{$_};
	#	$k = join(',', @$r[0..1]);
	#	print "nlink=@$r[3], devino=$devino{$k},  $_\n";
	#}


	# stat
	# dev,inum,att,nlink,uid,gid,spec,size,atim,utim,iutime,blksz,nblks


	$h_data = pack($ppat2, $romfile_magic, time);
	$h_data_size = 8;

	$r_data_size = ($#files + 2) * 12;


	# $p_data : path 名テーブルの作成とオフセット計算
	$p_data = '';
	for ( @files ) {
		$p_off{$_} = $r_data_size + length($p_data);
		$p_data .= '/' . $_ . "\0";
	}


	# $r_data : romfile 構造体の作成
	$d_off = $r_data_size + length($p_data);
	$r_data = '';
	if ( $opt_old || $ino > 255 ) {
		# hard link された file が data 部を共有するようになったが、
		# 条件が許せば extract もできるようにする。
		undef %ino;
		$ino = 0;
	}
	for ( @files ) {
		$r = $stat{$_};
		$size = @$r[7];
		my $d;
		if (! exists $shared{$_}) {
			$d = $d_off{$_} = $d_off;
			$d_off += $size;
		} else {
			$k = join(',', @$r[0..1]);
			$d = $d_off{$devino{$k}};
			if ($ino) {
				$size |= ($ino{$devino{$k}} << 24);
			}
		}
		$r_data .= pack($ppat3, $p_off{$_}, $size, $d);  # path, length, body
	}
	$r_data .= pack($ppat3, 0,0,0);


	die "die at 1:\n" if $r_data_size != length $r_data;

	print FF $h_data;
	print FF $r_data;
	print FF $p_data;
	for ( @files ) {
		if (! exists $shared{$_}) {
			$r = $stat{$_};
			$size = @$r[7];

			print STDERR "$_\n" if $opt_v;
			open BB, "$_" or die;
			$r = sysread BB, $buf, $size;
				die if $r != $size;
			print FF $buf;
			close BB;
		}
	}
	close FF;
}
sub xvf()
{
	open(FF, $romfile) or die "$0: cannot open $romfile\n";
	if (defined $opt_C) {
		chdir $opt_C or die "$0: cannot chdir to '$opt_C'\n";
	}

	my $r = read FF, $buf, 8;
	my ($ma, $tim) = unpack $ppat2, $buf;
	die "$0: corrupt magic number ?\n" if $ma != $romfile_magic;
	if ($opt_v) {
		print "'$romfile' time stamp : ", scalar localtime($tim), "\n";
	}


	# romfile table
	$d_total = 0;
	while (1) {
		$r = read FF, $buf, 12;
		if (! defined $r) {
			die "$0: romfile table too short.\n";
		}
		($p_off, $d_sz, $d_off) = unpack $ppat3, $buf;	# path, length, body
	#	print "$p_off, $d_sz, $d_off\n";
		last if ($p_off == 0 && $d_sz == 0 && $d_off == 0);
		push @p_off, $p_off;
		push @d_sz,  $d_sz;
		push @d_off, $d_off;
		$d_total += $d_sz;
	}


	# pathname table
	my ($p_min, $p_max) = &minmax(-s FF, 0, @p_off);
	my ($d_min, $d_max) = &minmax(-s FF, 0, @d_off);
	my $p_top = (tell FF) - $h_size;
	#print "tell=", $p_top, " min=", $min, "\n";
	if ($p_max < $d_min) {
		if ($p_min != $p_top) {
			die; # seek FF, $p_min + $h_size;
		}
		$r = read FF, $ptable, $d_min - $p_min;
		die "$0: pathname table read error.\n$!\n"
				if ! defined $r;
		die "$0: pathname table too short.\n"
				if ($r != $d_min - $p_min);
	} else {
		die "assert(p_max < d_min) : $p_max < $d_min\n";
	}
	foreach ( @p_off ) {
		$_ -= $p_min;
		($_) = substr($ptable, $_) =~ m/^([^\x00]+)\x00/;
		print "$_\n";
		push @paths, $_;
	}


	# file body
	if ($opt_x) {
	#	die "$0: 'x' not supported yet.\n";
		for ($i = 0; $i <= $#paths; $i++) {
			&extract($i, $paths[$i]);
		}
	}
	close FF;
}
sub extract($$)
{
	my ($i, $nam) = @_;

	$nam =~ s:^/::;
	print "xx $nam\n";
	my $dir = $nam;  ($dir =~ s:/[^/]+$::) or $dir = '.';

	my $buf;
	seek FF, $d_off[$i] + $h_size, 0 or die;
	read FF, $buf, $d_sz[$i];

	if (! -d $dir) {
		&mkdirs($dir);
	}
	open FO, ">$nam" or die "$nam: $!\n";
	print FO $buf;
	close FO;
}
sub minmax($$@)
{
	my ($min, $max, @a) = @_;
	foreach ( @a ) {
		$min = $_ if $min > $_;
		$max = $_ if $max < $_;
	}
	($min, $max);
}
sub mkdirs($)
{
	my ($dir) = @_;

	if ($dir =~ m:/:) {
		$dir =~ m:(.*)/:;
		mkdirs($1) if ! -d $1;
	}
	mkdir $dir or die "$dir: $!\n";
}

# $Log: romfile.pl,v $
# Revision 1.2  2004/06/18 06:57:29  matsui
# bugfix:
# 	wrong error check
#
# Revision 1.1.1.1  2004/02/24 07:56:31  matsui
# cdc1
#
# Revision 1.4  2003/10/10 09:53:04  matsui
# add copyright
#
# Revision 1.3  2003/07/03 02:13:54  matsui
# a bug
#
# Revision 1.2  2003/03/06 05:23:36  matsui
#
# implements strerror();
#
# Revision 1.1  2003/02/27 08:41:11  matsui
# romfs
#
# Revision 1.1  2002/10/01 05:54:57  matsui
# Initial revision
#
