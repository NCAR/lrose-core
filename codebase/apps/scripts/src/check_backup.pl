#!/usr/bin/perl
#
# check_archive.pl: Checks MSS backup files for date provided on command lin
#
# Usage:
#   daily_backup.pl <options>
#
#    Options:
#       -help                : Print usage
#       -date                : YYYYMMDD
#       -debug               : debugging on
#       -params              : archive parameter file
#
#===================================================================================
# Get the needed PERL supplied library modules.
#===================================================================================
use Getopt::Std qw( getopts );
use Getopt::Long;                                                                           
use Time::Local;#

#===================================================================================
# Set up the needed environment variables
#===================================================================================
use Env qw(RAP_DATA_DIR);
use Env qw(DATA_HOME);

#
# Get the program basename.
#
($prog = $0) =~ s|.*/||;

#
# Unbuffer standard output
#
$| = 1;

#
# Set file permissions
#
umask 002;

# Initialize command line arguments
$opt_date = "";
$opt_debug = "";
$opt_params = "";

$usage =
    "\nUsage: $prog <options>\n" .
    "Options:\n" .
    "  -help                 :Print usage\n" .
    "  -date                 :YYYYMMDD\n" .
    "  -params               :archive parameter file\n" .
    "  -debug                :debugging on\n" .
    "\n" .
    "Note:  Both the -date and -params arguments are required\n" .
    "       in order to check the contents on the MSS.\n";

# Get the arguments from the command line
$results = &GetOptions('help',
                       'date:i',
                       'params:s',
                       'debug');

if ($opt_help || ! $opt_date || ! $opt_params) {
    print $usage;
    exit 0;
}

# get date from command line argument
$year  = substr($opt_date,0,4);
$month = substr($opt_date,4,2);
$day   = substr($opt_date,6,2);

$date = "$year" . "$month" . "$day";

 
open(IF,"$opt_params") || die "\nCannot find file \"$opt_params\"\n\n";
$count_archive_files = 0;

if ($opt_debug) {
    print "cd $RAP_DATA_DIR\n";
}
chdir("$RAP_DATA_DIR");

while(<IF>) {
    @line = split;
    $comment = substr($line[0],0,1);
    if ($comment ne "#" && $line[0] ne "") {
	$archive_file_name = $line[0];
	$local_archive_dir = $line[1];
	$file_type = $line[2];
	$path_to_archive_files = $line[3];
	$path_on_mss = $line[4];
	&mss_path($archive_file_name, $path_on_mss,$local_archive_dir);

    }
}


for($i=0;$i<@archive_file_list;$i++) {
    # get listing
    if ($opt_debug) {
	print "msls -l ";
	print "${mss_path_list[$i]}/${year}/${month}${day}/${date}_${archive_file_list[$i]}.gz\n";
    }
    
     system("msls -l \\
            ${mss_path_list[$i]}/${year}/${month}${day}/${date}_${archive_file_list[$i]}.gz");
}

exit;

sub mss_path {
    $archive_file = $_[0];
    $mss_path = $_[1];
    $local_path = $_[2];
    if($archive_file ne "$previous_name") {
	$archive_file_list[$count_archive_files] = $archive_file;
	$mss_path_list[$count_archive_files] = $mss_path;
	$local_path_list[$count_archive_files] = $local_path;
	$previous_name = $archive_file;
	$count_archive_files++;
    }
}
