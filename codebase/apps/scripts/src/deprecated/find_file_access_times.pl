#! /usr/bin/perl -w
    eval 'exec /usr/bin/perl -S $0 ${1+"$@"}'
        if 0; #$running_under_some_shell
#
# Purpose: Run recursively through thru files in the directory passed
#          in on the command line. Find their latest modify time and
#          latest access time. Then sort the list based on access time
#          and print it to STDOUT.
#
# Deirdre Garvey NCAR/RAL 2008
#

use strict;
use File::Find ();
use Time::Local;

# Set the variable $File::Find::dont_use_nlink if you're using AFS,
# since AFS cheats.

# for the convenience of &wanted calls, including -eval statements:
use vars qw/*name *dir *prune/;
*name   = *File::Find::name;
*dir    = *File::Find::dir;
*prune  = *File::Find::prune;

sub wanted;
sub printfile;

use Cwd ();
my $cwd = Cwd::cwd();

my $Debug=0;

my $TmpFile="/tmp/mytest.file";
my $SortFile="/tmp/mytest.sort";
unlink($TmpFile);
unlink($SortFile);

# Open tmp file

my $is_ok=open(TMPFILE, "> $TmpFile");

# Traverse desired filesystems
File::Find::find({wanted => \&wanted}, @ARGV);

close(TMPFILE);

# Sort the tmpfile in reverse numeric order.
# Will sort on the first field - access time

system("sort -rn $TmpFile > $SortFile");

# Print the output file

printfile;

# Done

exit;

#==========================================================================

sub wanted {
    my ($dev,$ino,$mode,$nlink,$uid,$gid);

    my($ldev, $lino, $lmode, $lnlink, $luid, $lgid);
    my($rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks);
    my($fname, $dir, $dirpos);

    if (!-f) {
	print(STDERR "Skipping, not a file: $name\n");
	return;
    }

    # if has a file path in the name, need to get to the file?

    if ($name =~ /\//) {
	$dirpos=rindex($name, "/");
	$dir=substr($name, 0, $dirpos);
	$fname=substr($name, $dirpos+1);
	chdir ($dir);
	($ldev,$lino,$lmode,$lnlink,$luid,$lgid,$rdev,$size,
	 $atime,$mtime,$ctime,$blksize,$blocks) = stat($fname);
    } else {
	($ldev,$lino,$lmode,$lnlink,$luid,$lgid,$rdev,$size,
	 $atime,$mtime,$ctime,$blksize,$blocks) = stat($name);
    }

    if ($size <= 0) {
	print(STDERR "Skipping, zero-length: $name\n");
	return;
    }

    if ($Debug) {
	print(STDOUT "access: $atime, modify: $mtime, name: $name\n");
    }

    print(TMPFILE "$atime, $mtime, $name\n");
}

sub printfile {

    my ($is_ok, $line, @tmparr, @arr, $narr, $i);
    my ($atime, $mtime, $name, $adate, $mdate, $age);
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst);
    my ($warn);

    my $nsecs_yr=60*60*24*365;
    my $now=time;
       
    # Read the sorted file into an array so can print it

    $is_ok=open(SORTFILE, "< $SortFile");

    $narr=0;
    while ($line = <SORTFILE>) {
	chop($line);
	($atime, $mtime, $name)=split(",", $line);

	# Convert to human readable time

	($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) =gmtime($atime);
	$year=$year+1900;
	$mon=$mon+1;
	$adate=sprintf("%02d/%02d/%04d", $mon, $mday, $year);

	($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) =gmtime($mtime);
	$year=$year+1900;
	$mon=$mon+1;
	$mdate=sprintf("%02d/%02d/%04d", $mon, $mday, $year);

	$arr[$narr][0]=$atime;
	$arr[$narr][1]=$mtime;
	$arr[$narr][2]=$name;
	$arr[$narr][3]=$adate;
	$arr[$narr][4]=$mdate;

	$narr++;
    }
    close(SORTFILE);

    # Print output

    my $Today=`date`;
    print(STDOUT "Date: $Today");
    print(STDOUT "Last access \tLast modify \tFilename\n");
    print(STDOUT "----------- \t----------- \t---------\n");
    for ($i=0; $i<$narr; $i++) {
	print(STDOUT "$arr[$i][3] \t$arr[$i][4] \t$arr[$i][2]\n");
    }
}
