#!/usr/bin/perl
#
# tarDataSet.pl: Creates tar file of date for dates & times provided on command line
#
# Usage:
#   tarDataSet.pl <options>
#
#    Options:
#       -help                : Print usage
#       -start               : YYYYMMDDHHMM
#       -end                 : YYYYMMDDHHMM
#       -print_params        : prints a default parameter file
#       -params              : data parameter file name
#       -test                : do not execute system calls
#       -debug               : debugging on
#
#===================================================================================
# Get the needed PERL supplied library modules.
#===================================================================================
use Getopt::Std qw( getopts );
use Getopt::Long;                                                                           
use Time::Local;

#===================================================================================
# Set up the needed environment variables
#===================================================================================
use Env qw(RAP_DATA_DIR);
use Env qw(PROJ_DIR);

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

$mailprog = 'sendmail';

# Initialize command line arguments
$opt_start = "";
$opt_end   = "";
$opt_debug = "";
$opt_test  = "";
$opt_params = "";
$opt_print_params = "";

$usage =
    "\nUsage: $prog <options>\n" .
    "Options:\n" .
    "  -help                 :Print usage\n" .
    "  -start                :YYYYMMDDHHMM\n" .
    "  -end                  :YYYYMMDDHHMM\n" .
    "  -print_params         :prints default parameter file\n" .
    "  -params               :archive parameter file name\n" .
    "  -test                 :do not execute system calls\n" .
    "  -debug                :debugging on\n\n";

# Get the arguments from the command line
$results = &GetOptions('help',
                       'start:i',
                       'end:i',
                       'print_params',
                       'params:s',
                       'test',
                       'debug');

if($opt_print_params) {

    print "\########################################################################################################################\n";
    print "\# TAR FILE NAME           - Name that will be giving to tar file. YYYYMMDD is prepended and .tar is appended to file.\n";
    print "\#                         - final file will look like: YYYYMMDD_TARFILENAME.tar\n";
    print "\# LOCAL ARCHIVE DIRECTORY - Were to write tar files on local disk.\n";
    print "\# FILE_TYPE               - options:\n";
    print "\#                            time - data files stored as YYYYMMDD/HHMMSS (i.e. mdv files)\n";
    print "\#                            date - data files stored as YYYYMMDD.* (i.e. spdb files)\n";
    print "\# DATA PATH               - Were to find data to archive (relative to \$RAP_DATA_DIR).\n";
    print "\########################################################################################################################\n";
    print "#\n#\n";

    print "\# ";
    for($i=0;$i<150;$i++) {
	print "=";
    }
    print "\n";

    print "\# ";
    print "TAR FILE NAME\t\t";
    print "LOCAL ARCHIVE DIRECTORY\t\t\t";
    print "FILE_TYPE\t\t";
    print "DATA_PATH (relative to \$RAP_DATA_DIR)\n";

    print "\# ";
    for($i=0;$i<150;$i++) {
	print "=";
    }
    print "\n";

    print "\#\n";
    print "my_data_files\t\t";
    print "/temporary/tarfile/location\t\t";
    print "time\t\t\t";
    print "mdv/my_data_files_path\n";
    exit;
} 

if ($opt_help || ! $opt_params || ! $opt_start || ! $opt_end) {
    print $usage;
    exit 0;
}

$s_year  = substr($opt_start,0,4);
$s_month = substr($opt_start,4,2);
$s_day   = substr($opt_start,6,2);
$s_hour  = substr($opt_start,8,2);
$s_min   = substr($opt_start,10,2);
    
$e_year  = substr($opt_end,0,4);
$e_month = substr($opt_end,4,2);
$e_day   = substr($opt_end,6,2);
$e_hour  = substr($opt_end,8,2);
$e_min   = substr($opt_end,10,2);

$s_date = "$s_year"."$s_month"."$s_day";

$month = $s_month - 1;
$s_time = timegm(0, $s_min, $s_hour, $s_day, $month , $s_year);

$month = $e_month - 1;
$e_time = timegm(0, $e_min, $e_hour, $e_day, $month , $e_year);

open(IF,"$opt_params") || die "\nCannot find file \"$opt_params\"\n\n";
$count_archive_files = 0;

&calc_days($s_time, $e_time);
$number_of_days = $counter;

if ($opt_debug) {
    print "cd $RAP_DATA_DIR\n";
}
chdir("$RAP_DATA_DIR");


while(<IF>) {
    # creating tar files
    @line = split;
    $comment = substr($line[0],0,1);
    if ($comment ne "#" && $line[0] ne "") {
	$archive_file_name = $line[0];
	$local_archive_dir = $line[1];
	$file_type = $line[2];
	$path_to_archive_files = $line[3];

	if($path_to_archive_files ne "-") {
	    $c_date = $s_time;

	    if($file_type eq "date") {

		for($day_counter=0;$day_counter < $number_of_days;$day_counter++) {
		    ($sec, $min, $hr, $day, $month, $year, $wday, $yday,$isdst) =  gmtime($c_date); 

		    $month = $month + 1;
		    $year = $year + 1900;

		    if($month < 10) { $month = "0" . "$month";}
		    if($day < 10) { $day = "0" . "$day";}
		    $current_date = "$year" . "$month" . "$day";

		    if ($opt_debug) {
			print "\ngzip $RAP_DATA_DIR/${path_to_archive_files}/*${current_date}*\n";
			print "tar -C $RAP_DATA_DIR";
			print "-r ${path_to_archive_files}/*${current_date}* ";
			print "-f ${local_archive_dir}/${current_date}_${archive_file_name}\n\n";
		    }
		    if(! $opt_test) {
			system("gzip $RAP_DATA_DIR/${path_to_archive_files}/*${current_date}*");
			system("tar \\
                             -r ${path_to_archive_files}/*${current_date}* \\
                             -f ${local_archive_dir}/${current_date}_${archive_file_name}");
		    }
		    $c_date = $c_date + 86400;
		}

	    } elsif($file_type eq "time") {
		for($day_counter=0;$day_counter < $number_of_days;$day_counter++) {
		    ($sec, $min, $hr, $day, $month, $year, $wday, $yday,$isdst) =  gmtime($c_date); 

		    $month = $month + 1;
		    $year = $year + 1900;

		    if($month < 10) { $month = "0" . "$month";}
		    if($day < 10) { $day = "0" . "$day";}
		    $current_date = "$year" . "$month" . "$day";
		    
		    if($opt_debug) {
			print "gzip $RAP_DATA_DIR/$path_to_archive_files/$current_date/* > \&\! /dev/null\n";
		    }
		    if(! $opt_test) {
			system("gzip $RAP_DATA_DIR/$path_to_archive_files/$current_date/* >& /dev/null");
		    }

		    @files = `ls $path_to_archive_files/$current_date/*`;
		    $month = $month-1;
		    for($file_number=0;$file_number<@files;$file_number++) {
			chop($files[$file_number]);
			($files[$file_number] = $files[$file_number]) =~ s/$path_to_archive_files\/$current_date\///;
			$c_hour  = substr($files[$file_number],0,2);
			$c_min   = substr($files[$file_number],2,2);

			$c_time = timegm(0, $c_min, $c_hour, $day, $month , $year);
		
			if ($opt_debug) {
			    if($c_time >= $s_time && $c_time <= $e_time) {
				print "tar -C $RAP_DATA_DIR ";
				print "-r ${path_to_archive_files}/$current_date/$files[$file_number] ";
				print "-f ${local_archive_dir}/${current_date}_${archive_file_name}\n";
			    }
			}

			if(! $opt_test) {
			    if($c_time >= $s_time && $c_time <= $e_time) {
				system("tar -C $RAP_DATA_DIR \\
			                -r ${path_to_archive_files}/$current_date/$files[$file_number] \\
			                -f ${local_archive_dir}/${current_date}_${archive_file_name}");
			    }
			}
		    }
		    $c_date = $c_date + 86400;
		}
	    }else {
		warn "\nFile type \"$file_type\" not valid. Use \"time\" or \"date\". \"${path_to_archive_files}\" will not be tared to \"${date}_${archive_file_name}\"\n\n";
	    }
	}
    }
}
exit;

sub calc_days {
    $start_time = $_[0];
    $end_time = $_[1];
    $current_time = $start_time;
    $counter = 0;
    while($current_time <= $end_time) {
	($sec, $min, $hr, $day, $month, $year, $wday, $yday,$isdst) =  gmtime($current_time);
	$month = $month + 1;
	$year = $year + 1900;
	if($month < 10) { $month = "0" . "$month"};
	if($day < 10) { $day = "0" . "$day"};
	$days_list[$counter] = "$year" . "$month" . "$day";
	$current_time = $current_time + 86400;
	$counter++;
    }
    ($sec, $min, $hr, $day, $month, $year, $wday, $yday,$isdst) =  gmtime($end_time);
    $month = $month + 1;
    $year = $year + 1900;
    if($month < 10) { $month = "0" . "$month"};
    if($day < 10) { $day = "0" . "$day"};
    $end_date = "$year" . "$month" . "$day";
    if($end_date ne "$days_list[$counter - 1]") {
	$days_list[$counter] = $end_date;
	$counter++;
    }
}
