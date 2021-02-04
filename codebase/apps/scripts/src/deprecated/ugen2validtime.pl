#! /usr/bin/perl
#
# ugen2validtime.pl
#
# Function:
#	Perl script to read an input generation time and forecast secs
#       and return the forecast time. This is useful with _latest_data_info
#       files.
#
# Usage:
#       ugen2validtime.pl [-h] <time>
#
#       -h : help, print usage
#
# Input:
#       UNIX generation time and forecast seconds
#
# Output:
#       valid time
#
# Dependencies:
#	Perl must be available in the location at the top of this file.
# 	
# Author: Deirdre Garvey - NCAR/RAP/ESG		14-JAN-2002
#
##
#---------------------------------------------------------------------------------
#
# Get the program name

($prog = $0) =~ s%.*/%%;

# Usage statement

$Usage=
    "\n".
    "$prog forecast-gen-utime forecast-secs\n" .
    "Purpose: To convert an input forecast generation time and\n" .
    "         forecast seconds into a forecast valid time.\n" .
    "         Returns the forecast valid time as:\n" .
    "            valid_time: utime YYYY MMM DD HH MM SS\n" .
    "         in UTC time.\n" .
    "         Expects the forecast-gen-time to be a UNIX time\n" .
    "         and the forecast-secs to be the forecast lead time.\n";

# Print usage if insufficient command line input

if (@ARGV != 2) {
    print $Usage;
    exit -1;
}

# Parse the command line

$gen_utime = shift(@ARGV);
$fcast_secs = shift(@ARGV);

# Print help message if that was what really was wanted

if ($gen_utime =~ /h/) {
    print $Usage;
    exit -1;
}

# Add the forecast seconds to the gen time to get the forecast
# UNIX time

$fcast_utime = $gen_utime + $fcast_secs;

# Convert the time to human-readable
# get gmttime

($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) =
  gmtime($fcast_utime);

printf ("valid_time: %s %4d %.2d %.2d %.2d %.2d %.2d\n",
	$fcast_utime, $year + 1900, $mon + 1, $mday, $hour, $min, $sec);

exit(0);



