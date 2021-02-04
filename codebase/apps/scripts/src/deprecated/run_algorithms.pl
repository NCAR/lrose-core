#!/usr/bin/perl
#
# run_algs.pl: Uses parameter file to run multiple algorithms. 
# 
# 
#
# Usage:
#   daily_backup.pl <options>
#
#    Options:
#       -help                : Print usage
#       -debug               : debugging on
#       -test                : no system calls executed
#       -snuff_servers       : snuff all data servers
#       -print_params        : print basic param file
#       -params              : parameter file name
#       -proj                : project name (RAP_DATA_DIR/PROJECT)
#       -start               : YYYYMMDDHHMM
#       -end                 : YYYYMMDDHHMM
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
use Env qw(PROJECT);
use Env qw(DATA_HOST);
use Env qw(REMOTE_DATA_HOST);
use Env qw(PROJECT_HOME);

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
$opt_start = "";
$opt_end = "";
$opt_print_params = "";
$opt_params = "";
$opt_debug = "";
$opt_test = "";
$opt_proj = "";
$opt_snuff_servers = "";

$usage =
    "\nUsage: $prog <options>\n" .
    "Options:\n" .
    "  -help                 : Print usage\n" .
    "  -debug                : debugging on\n" .
    "  -test                 : no system calls executed\n" .
    "  -snuff_servers        : snuff all data servers\n" .
    "  -print_params         : print basic parameter file\n" .
    "  -params               : parameter file name\n" .
    "  -start                : YYYYMMDDHHMM\n" .
    "  -end                  : YYYYMMDDHHMM\n\n";

# Get the arguments from the command line
$results = &GetOptions('help',
                       'debug',
                       'test',
                       'snuff_servers',
                       'print_params',
                       'params:s',
                       'start:i',
                       'end:i');

if($opt_print_params) {

    print "\#";
    for($i=0;$i<204;$i++) {
	print "-";
    }
    print "\n";

    print "\#";
    print "Algorithm\t\t";
    print "Path-to-params\t\t";
    print "Parameter_filename\t\t";
    print "File_type\t\t";
    print "Remove_files?\t\t";
    print "Output_dir\t\t";
    print "Time_id\t\t";
    print "Mode\t\t";
    print "Input_url/input_dir\n";

    print "\#";
    for($i=0;$i<204;$i++) {
	print "-";
    }
    print "\n";

    print "\#\n";
    
    print "\########################################################################################################################\n";
    print "\# Algorithm          - Name of algorithm to run\n";                                                            
    print "\# Path_to_params     - Location of parameter file\n";
    print "\# Parameter_filename - Name of parameter file (assumes -params command line option)\n";
    print "\#                    - options:\n";
    print "\#                       none - for running algs without parameter file\n";
    print "\# File_type          - Needed for removing or unziping existing data files.\n";
    print "\#                    - options:\n";
    print "\#                        time - data files stored as YYYYMMDD/HHMMSS (i.e. mdv files)\n";
    print "\#                        date - data files stored as YYYYMMDD.* (i.e. spdb files)\n";
    print "\#                        colide - speacial clean up just for colide run\n";
    print "\# Remove_files?      - If true all files under YYYYMMDD directory are removed (File_type = time).\n";
    print "\#                    - If false all files under YYYYMMDD directory are unzipped (File_type = time).\n";
    print "\#                    - If true all files with *YYYYMMDD* are removed (File_type = time).\n";
    print "\#                    - If false all files with *YYYYMMDD* are unzipped (File_type = time).\n";
    print "\#                    Helps to prevent duplicate data. Spdb data is typically appended so delete files to rerun case\n";
    print "\#                    Mdv files may be zipped and multiple files end up in data directory.\n";
    print "\# Output_dir         - Tells script where (relative to \$RAP_DATA_DIR) to go to unzip or remove files\n";
    print "\#                        Works only if data files are on a disk mounted localy\n";
    print "\# Time_id            - What command line time format does algorithm use?\n";
    print "\#                    - options:\n";
    print "\#                        dixon    - assumes format -start \"YYYY MM DD HH MM SS\" -end \"YYYY MM DD HH MM SS\"\n";
    print "\#                        rehak    - assumes format -starttime YYYY/MM/DD_HH:MM:SS -endtime YYYY/MM/DD_HH:MM:SS\n";
    print "\#                        struct   - assumes format -url mdvp:://hostname::path/to/data\n";
    print "\#                                                   -start \"YYYY MM DD HH MM SS\"\n";
    print "\#                                                   -end   \"YYYY MM DD HH MM SS\"\n";
    print "\#                        interval - assumes format -interval YYYYMMDDHHMMSS YYYYMMDDHHMMSS\n";
    print "\#                        colide   - Special just for colide\n";
    print "\#                        f        - assumes format -f (list of all days within time range set on command line)\n";
    print "\#                                 - Only full directories can be processed with this option\n";
    print "\#                        if       - assumes format -if (list of all days within time range set on command line)\n";
    print "\#                                 - Only full directories can be processed with this option\n";
    print "\#                        none     - Use when there is no command line time options\n";
    print "\# mode                -  input for algorithms command line option \"-mode\"\n";
    print "\#                     -  Use \"none\" here if algorithms does not have the option to set the mode\n";
    print "\#                     -  Common post processing modes are \"Archive\", \"TIME_LIST\" and \"FILELIST\"\n";
    print "\# Input_url/input_dir -  Some programs need an input url or directory on the command line.\n";
    print "\#                        For Input_url use the following format:\n";
    print "\#                          mdvp:://hostname::path/to/data (where path/to/data is relative to \$RAP_DATA_DIR)\n";
    print "\#                        For Input_dir use the following format:\n";
    print "\#                          path/to/data (where path/to/data is relative to \$RAP_DATA_DIR)\n";
    print "\########################################################################################################################\n";
    print "\#\n";
    print "\# Use \"start\" or \"begin\" to select were in the parameter file exectuion is to begin.\n";
    print "\# Use \"stop\" or \"end\" if you want to stop execution but want to start again further down in the parameter file.\n";
    print "\# Use \"exit\" to stop processing the parameter file and exit script.\n";
    print "\#\n";
    print "\########################################################################################################################\n";
    print "\#\n";
    print "\#";
    for($i=0;$i<204;$i++) {
	print "-";
    }
    print "\n";

    print "\#";
    print "Algorithm\t\t";
    print "Path-to-params\t\t";
    print "Parameter_filename\t\t";
    print "File_type\t\t";
    print "Remove_files?\t\t";
    print "Output_dir\t\t";
    print "Time_id\t\t";
    print "Mode\t\t";
    print "Input_url/input_dir\n";

    print "\#";
    for($i=0;$i<204;$i++) {
	print "-";
    }
    print "\n";

    print "\#\n";
    
    exit;
}

if($opt_proj) {
    $PROJECT = $opt_proj;
}

# set option to run to false
$run_algs = 0;

if ($opt_help || ! $opt_params) {
    print $usage;
    exit 0;
}


if(! $opt_test && $opt_snuff_servers) {
    if($opt_debug) {
	print "\nsnuff_servers\n\n";
    }
    system("snuff_servers");
    if($opt_debug) {
	print "start_DsServerMgr\n\n";
    }
    system("start_DsServerMgr");
}

if(!$opt_start || !$opt_end) {
    # if start and/or end is not provied on command line
    # use date based on system time minus 24 hours and
    # run from 0z - 2359z on that day
    print "start and/or end time not provided\n";
    print "Running on previous day\n\n";
    system("sleep 5");
    $sys_date = `date -u +%D`;
    $year  = substr($sys_date,6,2);
    $year = "20" . "$year";
    $month = substr($sys_date,0,2);
    $day   = substr($sys_date,3,2);

    # take date and subract 24 hrs to get previous days date
    $month = $month - 1;
    $time = timegm(0, 00, 00, $day, $month , $year);
    $time = $time - 86400;
    ($sec, $min, $hr, $b_day, $b_month, $b_year, $wday, $yday,$isdst) =  gmtime($time); 

    $month = $month + 1;
    $b_month = $b_month + 1;
    $b_year = $b_year + 1900;

    if($month < 10) { $month = "0" . "$month";}
    if($b_month < 10) { $b_month = "0" . "$b_month";}
    if($b_day < 10) { $b_day = "0" . "$b_day";}

    $s_year =  $b_year ;
    $s_month = $b_month;
    $s_day =   $b_day;
    $s_hour = "00";
    $s_min = "00";

    $e_year =  $b_year ;
    $e_month = $b_month;
    $e_day =   $b_day;
    $e_hour = "23";
    $e_min = "59";

    $date = "$year" . "$month" . "$day";

} else {
# get date from command line argument
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

}
$month = $s_month - 1;
$s_time = timegm(0, 00, 00, $s_day, $month , $s_year);

$month = $e_month - 1;
$e_time = timegm(0, 00, 00, $e_day, $month , $e_year);

$current_time = $s_time;

open(IF,"$opt_params") ||
    die "\ncannot open param file $opt_params\n\n";

&calc_days($s_time, $e_time);

while(<IF>) {
    @line = split;
    $comment = substr($line[0],0,1);
    if($line[0] eq "start" || $line[0] eq "begin") {
	print "\nHit start command, starting to run block\n";
	$run_algs = 1;
	next;
    }
    if($line[0] eq "stop" || $line[0] eq "end") {
	print "\nHit stop, finished running block\n";
	print "Looking for more blocks to run....\n";
	$run_algs = 0;
	next;
    }
    if($line[0] eq "exit") {
	print "\nHit exit, exiting script.......\n";
	exit;
    }
    if ($comment ne "#" && $line[0] ne "") {
        $algname = $line[0];
        $path_to_params = $line[1];
        $params = $line[2];
        $file_type = $line[3];
        $remove_flag = $line[4];
	$output_dir = $line[5];
        $time_id = $line[6];
        $mode = $line[7];
	$input = $line[8];

	$rm_command = "rm -f";
	$gunzip_command = "gunzip";

	if($run_algs) {
	    print "\n###\n";
	}
	if($file_type eq "date") {
	    for($i=0;$i<@days_list;$i++) {
		if($remove_flag eq "true") {
		    $files = "$RAP_DATA_DIR/${output_dir}/*${days_list[$i]}* ";
		    if($opt_debug && $run_algs) {
			print "$rm_command $files\n";
		    }
		    if(! $opt_test && $run_algs) {
			system("$rm_command $files");
		    }
		}elsif ($remove_flag eq "false") {
		    $files = "$RAP_DATA_DIR/${output_dir}/*${days_list[$i]}*.gz ";
		    if($opt_debug && $run_algs) {
			print "$gunzip_command ${files}\n";
		    }
		    if(! $opt_test && $run_algs) {
			system("$gunzip_command $files");
		    }
		}else {
		    print "\nRemove_files \"$remove_flag\" not valid, use \"ture\" or \"false\"\n";
		    print "Files will not be removed or unziped\n\n";
		    next;
		}
	    }
	}elsif($file_type eq "time") {
	    for($i=0;$i<@days_list;$i++) {
		if($remove_flag eq "true") {
		    $files = "$RAP_DATA_DIR/${output_dir}/${days_list[$i]}/* ";
		    if($opt_debug && $run_algs) {
			print "$rm_command $files\n";
		    }
		    if(! $opt_test && $run_algs) {
			system("$rm_command $files");
		    }
		}elsif ($remove_flag eq "false") {
		    $files = "$RAP_DATA_DIR/${output_dir}/${days_list[$i]}/*.gz ";
		    if($opt_debug && $run_algs) {
			print "$gunzip_command ${files}\n";
		    }
		    if(! $opt_test && $run_algs) {
			system("$gunzip_command $files");
		    }
		}else {
		    print "\nRemove_files \"$remove_flag\" not valid, use \"ture\" or \"false\"\n";
		    print "Files will not be removed or unziped\n\n";
		    next;
		}
	    }
	}elsif($file_type eq "colide") {
	    for($i=0;$i<@days_list;$i++) {
		# Always remove the colide spdb files
		if($remove_flag eq "true") {
		    @files = ("$RAP_DATA_DIR/$PROJECT/spdb/colide/regions/*${days_list[$i]}* ",
		              "$RAP_DATA_DIR/$PROJECT/spdb/colide/line_smooth/*${days_list[$i]}* ",
		              "$RAP_DATA_DIR/$PROJECT/spdb/colide/tracked/*${days_list[$i]}* ",
			      "$RAP_DATA_DIR/$PROJECT/spdb/colide/tracked_internal/*${days_list[$i]}* ");
		    
		    if($opt_debug && $run_algs) {
			for($file=0;$file<@files;$file++) {
			    print "\n###\n";
			    print "$rm_command $files[$file]\n";
			}
		    }
		    if(! $opt_test && $run_algs) {
			for($file=0;$file<@files;$file++) {
			    system("$rm_command $files[$file]");
			}
		    }
		}elsif ($remove_flag eq "false") {
		    @files = ("$RAP_DATA_DIR/$PROJECT/spdb/colide/regions/*${days_list[$i]}*.gz ",
		              "$RAP_DATA_DIR/$PROJECT/spdb/colide/line_smooth/*${days_list[$i]}*.gz ",
		              "$RAP_DATA_DIR/$PROJECT/spdb/colide/tracked/*${days_list[$i]}*.gz ",
			      "$RAP_DATA_DIR/$PROJECT/spdb/colide/tracked_internal/*${days_list[$i]}*.gz ");
		    
		    if($opt_debug && $run_algs) {
			for($file=0;$file<@files;$file++) {
			    print "\n###\n";
			    print "$gunzip_command $files[$file]\n";
			}
		    }
		    if(! $opt_test && $run_algs) {
			for($file=0;$file<@files;$file++) {
			    system("$gunzip_command $files[$file]");
			}
		    }
		}else {
		    if ($run_algs) {
			print "\nOn command line:  $_";
			print "\nRemove_files \"$remove_flag\" not valid, use \"ture\" or \"false\"\n";
			print "Files will not be removed or unziped\n\n";
		    }
		    next;
		}
	    }
	}else {
	    if ($run_algs) {
		print "\nOn command line:  $_";
		print "File_type \"$file_type\" not valid, use \"date\" ,\"time\" or \"colide\"\n";
		print "Files will not be removed or unziped\n\n";
	    }
	    next;
	}
	    
	
	# Building command line
	if($params ne "none") {
	    $command = "$algname -params ${path_to_params}/$params ";
	}else {
	    $command = "$algname ";
	}

	if($mode ne "none" && $time_id ne "colide") {
	    $command = "$command " . "-mode $mode ";
	}

	if($time_id eq "dixon") {
	    $command = "$command" . "-start \"$s_year $s_month $s_day $s_hour $s_min 00\" ";
	    $command = "$command" . "-end \"$e_year $e_month $e_day $e_hour $e_min 00\"";
	}elsif($time_id eq "rehak") {
	    $command = "$command" . "-starttime ${s_year}/${s_month}/${s_day}_${s_hour}:${s_min}:00 ";
	    $command = "$command" . "-endtime ${e_year}/${e_month}/${e_day}_${e_hour}:${e_min}:00";
	}elsif($time_id eq "struct") {
	    $command = "$command" . "-url ${input} ";
	    $command = "$command" . "-starttime ${s_year}/${s_month}/${s_day}_${s_hour}:${s_min}:00 ";
	    $command = "$command" . "-endtime ${e_year}/${e_month}/${e_day}_${e_hour}:${e_min}:00";
	}elsif($time_id eq "interval") {
	    $command = "$command" . "-interval ${s_year}${s_month}${s_day}${s_hour}${s_min}00 ${e_year}${e_month}${e_day}${e_hour}${e_min}00";
	}elsif($time_id eq "colide") {
	    $ENV{PROCESSING_MODE} = $mode;
	    $ENV{MIN_DATE} = "${s_year}${s_month}${s_day}";
	    $ENV{MIN_TIME} = "${s_hour}${s_min}00";
	    $ENV{MAX_DATE} = "${e_year}${e_month}${e_day}";
	    $ENV{MAX_TIME} = "${e_hour}${e_min}00";
	    chdir("$COLIDE_HOME/bin");
	}elsif($time_id eq "if") {
	    $command_ft = "$command" . "-if ";
	    $command    = "$command" . "-if ";
	    for($i=0;$i<@days_list - 1;$i++) {
		$command = "$command" . "$RAP_DATA_DIR/${input}/${days_list[$i]}/*\; " . "$command_ft";
	    }
	    $command = "$command" . "$RAP_DATA_DIR/${input}/${days_list[$i]}/*\; ";
	}elsif($time_id eq "f") {
	    $command_ft = "$command" . "-f ";
	    $command    = "$command" . "-f ";
	    for($i=0;$i<@days_list - 1;$i++) {
		$command = "$command" . "$RAP_DATA_DIR/${input}/${days_list[$i]}/*\; " . "$command_ft";
	    }
	    $command = "$command" . "$RAP_DATA_DIR/${input}/${days_list[$i]}/*\; ";
	}elsif($time_id eq "none") {
	    $command = "$command"
	}else {
	    print "$time_id is not a valid \"Time format\" use \"dixon\", \"rehak\", \"colide\", \"interval\",\"struct\", \"f\", \"if\" or \"none\" .\n\n";
	    next;
	}
	# run the algorthm
	if($opt_debug && $run_algs) {
	    print "$command\n";
	    print "###\n";
	}
	if(! $opt_test && $run_algs) {
	    system("$command");
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
    }
}


