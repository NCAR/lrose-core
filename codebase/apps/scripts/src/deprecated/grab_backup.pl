#!/usr/bin/perl
#
# Usage:
#   grab_backup.pl <options>
#
#    Options:
#       -help                : Print usage
#       -start               : YYYYMMDD
#       -end                 : YYYYMMDD
#       -params              : archive parameter file
#       -debug               : debugging on
#       -test                : do not execute system calls
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
$opt_test = "";

$usage =
    "\nUsage: $prog <options>\n" .
    "Options:\n" .
    "  -help                 :Print usage\n" .
    "  -start                :YYYYMMDD\n" .
    "  -end                  :YYYYMMDD\n" .
    "  -params               :archive parameter file\n" .
    "  -debug                :debugging on\n" .
    "  -test                 :do not execute system calls\n\n";

# Get the arguments from the command line
$results = &GetOptions('help',
                       'start:i',
                       'end:i',
                       'params:s',
		       'test',
                       'debug');

if ($opt_help || ! $opt_start || ! $opt_end || ! $opt_params) {
    print $usage;
    exit 0;
}

$s_year  = substr($opt_start,0,4);
$s_month = substr($opt_start,4,2);
$s_day   = substr($opt_start,6,2);
    
$e_year  = substr($opt_end,0,4);
$e_month = substr($opt_end,4,2);
$e_day   = substr($opt_end,6,2);

$month = $s_month - 1;
$s_time = timegm(0, 00, 00, $s_day, $month , $s_year);

$month = $e_month - 1;
$e_time = timegm(0, 00, 00, $e_day, $month , $e_year);

$current_time = $s_time;

open(IF,"$opt_params") || die "\nCannot find file \"$opt_params\"\n\n";

my(@lines) = <IF>; # read file into list
@lines = sort(@lines); # sort the list
my($line);
close(IF);

if ($opt_debug) {
    print "\ncd $RAP_DATA_DIR\n";
}
chdir("$RAP_DATA_DIR");
$previous_tar_name = "";

while($current_time <= $e_time) {
    ($sec, $min, $hr, $day, $month, $year, $wday, $yday,$isdst) =  gmtime($current_time);
    $month = $month + 1;
    $year = $year + 1900;
    if($month < 10) { $month = "0" . "$month"};
    if($day < 10) { $day = "0" . "$day"};
    $days_list[$counter] = "$year" . "$month" . "$day";
    $current_time = $current_time + 86400;

    $date = "$year" . "$month" . "$day";
    print "Getting files for $date\n";
 
    $count_archive_files = 0;

    foreach $line (@lines) {
	@line = split(/\s+/,$line);
	$comment = substr($line[0],0,1);
	if ($comment ne "#" && $line[0] ne "") {
	    $tar_file_name = $line[0];
	    $local_archive_dir = $line[1];
	    $file_type = $line[2];
	    $path_to_archive_files = $line[3];
	    $path_on_mss = $line[4];

	    #make sure not to get duplicate tar files
	    if($tar_file_name ne $previous_tar_name ||
	       $previous_tar_name ne ""){
		&mss_path($tar_file_name, $path_on_mss,$local_archive_dir);
	    }
	}
    }
    

    for($i=0;$i<@archive_file_list;$i++) {
	# get listing
	if ($opt_debug) {
	    print "msrcp ";
	    print "\"mss\:${mss_path_list[$i]}/${year}/${month}${day}/${date}_${archive_file_list[$i]}*\" ";
	    print ".\n";
	}
	
	if(! $opt_test) {
	    system("msrcp \\
            \"mss:${mss_path_list[$i]}/${year}/${month}${day}/${date}_${archive_file_list[$i]}*\" .");
	}

	# if compressed tar file
	if(-e "${date}_${archive_file_list[$i]}.gz"){

	    if ($opt_debug) {
		print "tar -xzf ${date}_${archive_file_list[$i]}.gz\n";
	    }

	    if(! $opt_test) {
		system("tar -xzf ${date}_${archive_file_list[$i]}.gz");
	    }

	    if ($opt_debug) {
		print "removing file ${date}_${archive_file_list[$i]}.gz\n\n";
	    }

	    if(! $opt_test) {
		unlink("${date}_${archive_file_list[$i]}.gz");
	    }
	}

	# if uncompressed tar file
	if(-e "${date}_${archive_file_list[$i]}"){

	    if ($opt_debug) {
		print "tar -xf ${date}_${archive_file_list[$i]}\n";
	    }

	    if(! $opt_test) {
		system("tar -xf ${date}_${archive_file_list[$i]}");
	    }

	    if ($opt_debug) {
		print "removing file ${date}_${archive_file_list[$i]}\n\n";
	    }

	    if(! $opt_test) {
		unlink("${date}_${archive_file_list[$i]}");
	    }
	}
    }
}
exit;

sub mss_path {
    $archive_file = $_[0];
    $mss_path = $_[1];
    $local_path = $_[2];
    if($archive_file ne "$previous_tar_name") {
	$archive_file_list[$count_archive_files] = $archive_file;
	$mss_path_list[$count_archive_files] = $mss_path;
	$local_path_list[$count_archive_files] = $local_path;
	$previous_tar_name = $archive_file;
	$count_archive_files++;
    }
}
