#!/usr/bin/perl
#
# Hiq2DsrLogDump.pl: Reads a Hiq2Dsr log file and dumps out the input or output
# beam information to files that can be used by a plotting program.
#
# Usage:
#   Hiq2DsrLogDump <options>
#
#    Options:
#       -help                : Print usage
#       -debug               : Debug flag
#       -input_file          : Input Hiq2Dsr log file.
#       -output_dir          : Output directory
#       -output              : Dump the output beam information rather than the input
#                              beam information
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
use Env qw(HOST);

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
$opt_output_dir = ".";
$opt_output = "";
$opt_input_file = "./Hiq2Dsr.test.log";
$opt_debug = "";

$usage =
    "\nUsage: $prog <options> <log file name>\n" .
    "Options:\n" .
    "  -help                 :Print usage\n" .
    "  -debug                :Debug flag\n" .
    "  -input_file           :Input Hiq2Dsr log file.\n" .
    "                         Note that log must have been created with debug on\n" .
    "                         for output beam information and both debug and\n" .
    "                         debug_hiq_summary on for input beam information.\n" .
    "  -output_dir           :Output directory (default = \"$opt_output_dir\")\n" .
    "  -output               :Dump the output beam information rather than the input" .
    "                         beam information.\n\n";

# Get the arguments from the command line
$results = &GetOptions('help',
		       'debug',
		       'input_file:s',
                       'output_dir:s',
                       'output');

if ($opt_help)
{
    print $usage;
    exit 0;
}

open(INPUT_FILE, "<$opt_input_file") || die "\nCannot find file \"$opt_input_file\"\n\n";

$volume_num = 0;
$output_file = sprintf("%s/%d.txt", $opt_output_dir, $volume_num);
open(OUTPUT_FILE, ">$output_file");

if ($opt_debug)
{
    print "Output file: $output_file\n";
}

while(<INPUT_FILE>)
{
    # Check for end of volume

    if (m/end of volume (\d+)/)
    {
	if ($opt_debug)
	{
	    print "Found end of volume\n";
	}

	$volume_num = $1;

	if ($out_debug)
	{
	    print "Input line: $_";
	    print "    volume num = $volume_num\n";
	}

	close OUTPUT_FILE;
	$output_file = sprintf("%s/%d.txt", $opt_output_dir, $volume_num);
	open(OUTPUT_FILE, ">$output_file");

	if ($opt_debug)
	{
	    print "Output file: $output_file\n";
	}
    }

    # Check for input beam

    elsif (m/beam: elev = (\d+\.?\d?), az = (\d+\.?\d?), time = (.*)/)
    {
	if ($opt_debug)
	{
	    print "Found input beam\n";
	}

	$elev = $1;
	$az = $2;
	$time = $3;

	if ($opt_debug)
	{
	    print "Input line: $_";
	    print "    elev = $elev\n";
	    print "    az = $az\n";
	    print "    time = $time\n";
	}

	if (!$opt_output)
	{
	    print OUTPUT_FILE "$elev, $az\n";
	}
    }

    # Check for output beam

    elsif (m/Writing beam: az = (\d+\.?\d?), el = (\d+\.?\d?)/)
    {
	if ($opt_debug)
	{
	    print "Found output beam\n";
	}

	$elev = $2;
	$az = $1;

	if ($opt_debug)
	{
	    print "Input line: $_";
	    print "    elev = $elev\n";
	    print "    az = $az\n";
	}

	if ($opt_output)
	{
	    print OUTPUT_FILE "$elev, $az\n";
	}
    }
}

close OUTPUT_FILE;
close INPUT_FILE;
