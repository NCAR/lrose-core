#!/usr/bin/perl
#
# daily_command.pl: Runs a given command once for each date in the specified
#   time list.  The command is run for each date with the date string appended
#   to the command as specified.
#
#===================================================================================
# Get the needed PERL supplied library modules.
#===================================================================================
use Getopt::Std qw( getopts );
use Getopt::Long;                                                                           
use Time::Local;

#
# Get the program basename.
#
($prog = $0) =~ s|.*/||;

#
# Unbuffer standard output
#
$| = 1;

# Initialize command line arguments
$opt_start = "";
$opt_end = "";
$opt_debug = "";
$opt_cmd = "";

$usage =
    "\nUsage: $prog <options>\n" .
    "Options:\n" .
    "  -help                 :Print usage\n" .
    "  -start                :YYYYMMDD\n" .
    "  -end                  :YYYYMMDD\n" .
    "  -cmd                  :Command string\n" .
    "  -debug                :Debugging on\n" .

# Get the arguments from the command line
$results = &GetOptions('help',
                       'start:i',
                       'end:i',
                       'cmd:s',
                       'debug');

if ($opt_help)
{
    print $usage;
    exit 0;
}

if (!$opt_start || !$opt_end || !$opt_cmd)
{
    print "Must specify -start, -end and -cmd\n";
    print $usage;
    exit 0;
}

# Extract the start and end times from the command line

$start_year = substr($opt_start, 0, 4) - 1900;
$start_month = substr($opt_start, 4, 2) - 1;
$start_day = substr($opt_start, 6, 2);

$end_year = substr($opt_end, 0, 4) - 1900;
$end_month = substr($opt_end, 4, 2) - 1;
$end_day = substr($opt_end, 6, 2);

$current_date = timegm(0, 0, 0, $start_day, $start_month, $start_year);
$end_date = timegm(0, 0, 0, $end_day, $end_month, $end_year);

while ($current_date <= $end_date)
{
    # Construct the date string

    ($curr_sec, $curr_min, $curr_hour, $curr_day, $curr_month, $curr_year) =
	gmtime($current_date);

    $curr_year += 1900;
    $curr_month += 1;

    $date_string = sprintf("%04d%02d%02d", $curr_year, $curr_month, $curr_day);

    # Construct the command

    $command = $opt_cmd . " " . $date_string;

    if ($opt_debug)
    {
	print "Command: $command \n";
    }

    # Execute the command

    system($command);

    # Increment the date

    $current_date += 86400;
}
