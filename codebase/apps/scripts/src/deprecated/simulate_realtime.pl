#!/usr/bin/perl
#
# simulate_realtime.pl 
# copies archive files into realtime directories
# 
# 
#
# Usage:
#   simulate_realtime.pl <options>
#
#    Options:
#       -help                : Print usage
#       -debug               : debugging on
#       -test                : print debug information but do nothing
#       -print_params        : print basic param file
#       -params              : parameter file name
#       -simulation_type     : current_time or data_time (default: data_time)
#                              current_time - data gets time stamp as if realtime
#                              data_time - data copied over with original time stamp
#       -simulation_speed    : 1 - 10 (default: 1)
#                              i.e. 1-realtime, 2-twice_realtime
#                              Used only if simulation_type is set to data_time
#       -start               : YYYYMMDD
#       -end                 : YYYYMMDD
#
#===================================================================================
# Get the needed PERL supplied library modules.
#===================================================================================
use Getopt::Std qw( getopts );
use Getopt::Long;                                                                           
use Time::Local;
use File::Path;

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
$opt_simulation_type = "";
$opt_simulation_speed = "";

$usage =
    "\nUsage: $prog <options>\n" .
    "Options:\n" .
    "  -help                 : Print usage\n" .
    "  -debug                : debugging on\n" .
    "  -test                 : print debug information but do nothing\n" .
    "  -print_params         : print basic parameter file\n" .
    "  -params               : parameter file name\n" .
    "  -simulation_type      : current_time or data_time (default: data_time)\n" .
    "                           current_time - data gets time stamp as if realtime\n" .
    "                           data_time - data copied over with original time stamp\n" .
    "  -simulation_speed     : 1 - 10 (default: 1)\n" .
    "                           i.e. 1-realtime, 2-twice_realtime\n" .
    "                           used only if simulation_type is set to data_time\n" .
    "  -start                : YYYYMMDDHHMM\n" .
    "  -end                  : YYYYMMDDHHMM\n\n";

# Get the arguments from the command line
$results = &GetOptions('help',
                       'debug',
                       'test',
                       'print_params',
                       'params:s',
                       'simulation_type:s',
                       'simulation_speed:i',
                       'start:i',
                       'end:i');

if($opt_print_params) {

    print "\########################################################################################################################\n";
    print "\# mdv input_directory  - Were to find data to be copied (relative to \$RAP_DATA_DIR)\n";
    print "\# output_directory     - Were to put input_data (relative to \$RAP_DATA_DIR)\n";
    print "\# data_latency         - For data that typically comes in late. i.e. Model data\n";
    print "\########################################################################################################################\n";
    print "\#\n";
    print "\#";
    for($i=0;$i<100;$i++) {
	print "-";
    }
    print "\n";

    print "\#";
    print "input_directory\t\t";
    print "output_directory\t\t";
    print "data_latency\t\t\n";

    print "\#";
    for($i=0;$i<100;$i++) {
	print "-";
    }
    print "\n";

    print "\#\n";
    
    exit;
}


if ($opt_help || ! $opt_params) {
    print $usage;
    exit 0;
}


# check command line arguments and set defaults
if ($opt_simulation_type eq "" || $opt_simulation_type eq "data_time") {
    $simulation_type = "data_time";
} elsif($opt_simulation_type eq "current_time") {
    $simulation_type = "current_time";
}
else {
    print "Bad simulation_type \"$opt_simulation_type\"\n";
    exit;
}

if ($opt_simulation_speed eq  "" || $opt_simulation_type eq "current_time") {
    $simulation_speed = 1;
}
elsif($opt_simulation_speed > 10 || $opt_simulation_speed < 1) {
    print "Bad simulation_speed \"$opt_simulation_speed\"\n";
    exit;
}else {
    $simulation_speed = $opt_simulation_speed;
}

if(!$opt_start || !$opt_end) {
    print "start and/or end time not provided\n";
    exit;
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
$s_time = timegm(0, $s_min, $s_hour, $s_day, $month , $s_year);

$month = $e_month - 1;
$e_time = timegm(0, $e_min, $e_hour, $e_day, $month , $e_year);

$sys_start_time = time;

open(IF,"$opt_params") ||
    die "\ncannot open param file $opt_params\n\n";

$array_counter = 0;
$unix_time_now = time;

# loop through parameter file and make list ($main_list) of data available and times
# column 0 - original data time (unix time)
# column 1 - input data directory
# column 2 - output data directory
# column 3 - new file time if simulate_type is "current_time" (unix time)
# column 4 - time data available in data_time mode (accounting for data latency) (unix time)
# column 5 - time data available in current_time mode (accounting for data latency) (unix time)
while(<IF>) {
    @line = split;
    $comment = substr($line[0],0,1);
    if ($comment ne "#" && $line[0] ne "") {
        $input_dir = $line[0];
        $output_dir = $line[1];
	$data_latency = $line[2];

	# file type is for adding symbolic products in the future
	$file_type = date;

        # Get list of times available for this data within our time limits
	@file_list = 0;
	@file_list = &create_file_list($s_time, $e_time, $input_dir, $file_type);
	
	$i=0;
	$number_in_array = @file_list;
	if($file_list[$i] > 0) {
	    for($j=0;$j<@file_list;$j++) {
		$time_offset = $unix_time_now - $file_list[$i];
		$main_list[$j + $array_counter][0] = $file_list[$i];
		$main_list[$j + $array_counter][1] = $input_dir;
		$main_list[$j + $array_counter][2] = $output_dir;
		$main_list[$j + $array_counter][3] = $file_list[$i] + $time_offset;
		$main_list[$j + $array_counter][4] = $file_list[$i] + $data_latency;

		($sec, $min, $hr, $day, $month, $year, $wday, $yday,$isdst) =  gmtime($file_list[$i]);
		if($hr < 10) { $hr = "0" . "$hr";}
		if($min < 10) { $min = "0" . "$min";}
		if($sec < 10) { $sec = "0" . "$sec";}
		$p_time = "$hr" . "$min" . "$sec";;

		$main_list[$j + $array_counter][6] = $p_time;
		$i++;
	    }
	    $array_counter = $j + $array_counter;
	}
    }
}

# sort in numerical order $main_list using column 4 (file time plus data latency)
# this puts the data in the order it will be coming in.
@sorted = sort {$a->[4] <=> $b->[4]} @main_list;

# calculating the times to give the data if using simulation_type current_time
print "current time = $unix_time_now\n";
for($j=0;$j<@sorted;$j++) {
    if($j > 0) {
	$new_time = $sorted[$j - 1][3] + ($sorted[$j][0] - $sorted[$j-1][0]);
	$sorted[$j][3] = $new_time; 
	$sorted[$j][5] = $new_time + $sorted[$j][4] - $sorted[$j][0];
    }
# uncomment next 4 lines to print the sorted list
#    for($i=0;$i<7;$i++) {
#	print "$sorted[$j][$i] ";
#    }
#    print "\n";
}

if($simulation_type eq "data_time") {
    $realtime_offset = time - $sorted[0][0];
    $simulate_start_time = $sorted[0][0];
    $simulate_end_time = $e_time + 10;
    $simulate_time = $simulate_start_time;
} elsif($simulation_type eq "current_time") {
    $realtime_offset = time - $sorted[0][3];
    $simulate_start_time = $sorted[0][3];
    $simulate_end_time = $sorted[@sorted - 1][3] + 10;
    $simulate_time = $simulate_start_time;
}
print "simulation start time  = $simulate_start_time\n";
print "simulation end time    = $simulate_end_time\n";
print "realtime offset        = $realtime_offset sec\n";
print "simulation type        = $simulation_type\n";
print "simulation speed       = $simulation_speed\n";
sleep 10;


# starting simulation
while($simulate_time <= $simulate_end_time) {

    # choose which data_available_time to use based on simulation_type
    if ($simulation_type eq "data_time") {
	$data_available_time = $sorted[0][4];
    }else {
	$data_available_time = $sorted[0][5];
    }

    if($data_available_time <= $simulate_time) {
        # $current_time used for information purposes only
	$current_time = time;

	($sec, $min, $hr, $day, $month, $year, $wday, $yday,$isdst) =  gmtime($sorted[0][0]);
	$month = $month + 1;
	$year = $year + 1900;
	if($month < 10) { $month = "0" . "$month"};
	if($day < 10) { $day = "0" . "$day"};
	if($hr < 10) { $hr = "0" . "$hr"};
	if($min < 10) { $min = "0" . "$min"};
	if($sec < 10) { $sec = "0" . "$sec"};
	$data_date = "$year" . "$month" . "$day";
	$data_time = "$hr" . "$min" . "$sec";

	if($simulation_type eq "data_time") {
	    if($opt_debug) {
		print "\nsimulation_time = $simulate_time\n";
		print "current_time = $current_time\n";
		print "PROCESSING: \"$sorted[0][0] $sorted[0][1]\"\n";
		print "cp $RAP_DATA_DIR/$sorted[0][1]/$data_date/${data_time}* $RAP_DATA_DIR/$sorted[0][2]/$data_date/.\n\n";
	    }
	    if(!$opt_test) {
		if(! -e "$RAP_DATA_DIR/$sorted[0][2]/$data_date") {
		    mkpath("$RAP_DATA_DIR/$sorted[0][2]/$data_date",1,0777);
		}
		system("cp $RAP_DATA_DIR/$sorted[0][1]/$data_date/${data_time}* $RAP_DATA_DIR/$sorted[0][2]/$data_date/.");
	    }
	}else {
	    ($sec, $min, $hr, $day, $month, $year, $wday, $yday,$isdst) =  gmtime($sorted[0][3]);
	    $month = $month + 1;
	    $year = $year + 1900;
	    if($month < 10) { $month = "0" . "$month"};
	    if($day < 10) { $day = "0" . "$day"};
	    if($hr < 10) { $hr = "0" . "$hr"};
	    if($min < 10) { $min = "0" . "$min"};
	    if($sec < 10) { $sec = "0" . "$sec"};
	    $RTSim_date = "$year" . "$month" . "$day";
	    $RTSim_time = "$hr" . "$min" . "$sec";

	    if($opt_debug) {
		print "\nsimulation_time = $simulate_time\n";
		print "current_time = $current_time\n";
		print "PROCESSING: \"$sorted[0][3] $sorted[0][1]\"\n";
		print "update_mdv_times -path $RAP_DATA_DIR/$sorted[0][1]/$data_date/${data_time}.mdv ";
		print "-modtime \"$year\/$month\/${day}_$hr\:$min\:$sec\" >& /dev/null\n\n";
	    }
	    if(!$opt_test) {
		if(! -e "$RAP_DATA_DIR/$sorted[0][2]/$RTSim_date") {
		    mkdir("$RAP_DATA_DIR/$sorted[0][2]/$RTSim_date",0777);
		}
		chdir("$RAP_DATA_DIR/$sorted[0][2]");
		system("update_mdv_times -path $RAP_DATA_DIR/$sorted[0][1]/$data_date/${data_time}.mdv \\
                        -modtime \"$year\/$month\/${day}_$hr\:$min\:$sec\" >& /dev/null");
	    }
	}

	shift(@sorted);
	if(@sorted == 0 ){
	    print "Simulation complete...exiting\n";
	    exit;
	}
	if($opt_debug) {
	    if($simulation_type eq "data_time") {
		print "WAITING FOR:  \"$sorted[0][0] $sorted[0][1]\"\n";
                print "AVAILABLE AT: \"$sorted[0][4]\"\n";
	    }else {
		print "WAITING FOR:  \"$sorted[0][3] $sorted[0][1]\"\n";
                print "AVAILABLE AT: \"$sorted[0][5]\"\n";
	    }
	}
    }
    sleep 1;
    $simulate_time = $simulate_time + $simulation_speed;
#    if($opt_debug) {
#	print "$simulate_time\n";
#    }
}
exit;

sub create_file_list {
    $start_time = $_[0];
    $end_time = $_[1];
    $directory = $_[2];
    $type = $_[3];

    ($sec, $min, $hr, $day, $month, $year, $wday, $yday,$isdst) =  gmtime($start_time);
    $month = $month + 1;
    $year = $year + 1900;
    if($month < 10) { $month = "0" . "$month"};
    if($day < 10) { $day = "0" . "$day"};
    $start_date =  "$year" . "$month" . "$day";

    ($sec, $min, $hr, $day, $month, $year, $wday, $yday,$isdst) =  gmtime($end_time);
    $month = $month + 1;
    $year = $year + 1900;
    if($month < 10) { $month = "0" . "$month"};
    if($day < 10) { $day = "0" . "$day"};
    $end_date =  "$year" . "$month" . "$day";

    $current_date = $start_date;
    $current_time = $start_time;
    $t=0;
    @return_files = 0;
    do {
	($sec, $min, $hr, $day, $month, $year, $wday, $yday,$isdst) =  gmtime($current_time);
	$month = $month + 1;
	$year = $year + 1900;
	if($month < 10) { $month = "0" . "$month"};
	if($day < 10) { $day = "0" . "$day"};
	$current_date = "$year" . "$month" . "$day";
	chdir("$RAP_DATA_DIR/$directory/$current_date");
	system("gunzip *.gz >& /dev/null");
	@files = 0;
	@files = `ls *`;
	for($z=0;$z<@files;$z++) {
	    $hour = substr($files[$z],0,2);
	    $min  = substr($files[$z],2,2);
	    $sec  = substr($files[$z],4,2);
	    $data_time = timegm($sec, $min, $hour, $day, $month-1 , $year);
	    if($data_time >= $start_time && $data_time <= $end_time) {
		$return_files[$t] = $data_time;
		$t++;
	    }
	}
	$current_time = $current_time + 86400;
	$counter++;
    }	while($current_date < $end_date );

    return @return_files;
}


