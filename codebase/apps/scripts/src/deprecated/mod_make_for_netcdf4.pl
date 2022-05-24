#! /usr/bin/perl
#
# mod_make_for_netcdf4.pl
#
# Function:
#	Perl script to change Makefiles and make macro files to change
#       link lines from netCDF3 to netCDF4. This is NOT intended for use
#       with apps that use the RAP Makefile System of make include files
#       but for RTFDDA and other stand-alone applications.
#
# Usage:
#       mod_make_for_netcdf4.pl -h
#
# Input:
#       Source files (e.g., <dir-path>/<file>)
#
# Output:
#       Copies the input source file into <file>.bak and puts the 
#       filtered file into <file>.
#
# Dependencies:
#	Perl must be available in the location at the top of this file.
# 	
# Author: Deirdre Garvey - NCAR/RAL/SDG		28-AUG-2012
#         from a similar Perl script mod_rtfdda_for_openmpi.pl
##
#---------------------------------------------------------------------------------

# The sys_wait_h is required to get the correct return codes from the system() calls.
require 5.002;
use POSIX 'sys_wait_h'; 
use Env;

# Need the Perl getopts library routine
use Getopt::Std qw( getopts );

# Program defaults

$Exit_failure=0;
$Exit_succcess=1;

$Netcdf_dir="/opt/netcdf/lib";
$Hdf_dir="/opt/hdf/lib";
$Szip_dir="/opt/szip/lib";

# Command line defaults

$Debug=0;
$TestMode=0;
$VerboseDebug=0;

# Get command line options
&getopts('hdtv'); 

if ($opt_h) {
   print(STDERR "Usage: $0 [-h] [-d] [-t] [-v] <files> ...\n");
   print(STDERR "Purpose: Modifies the input Makefile and make macro files link\n");
   print(STDERR "         lines to change netCDF3 to netCDF4. This is NOT for\n");
   print(STDERR "         use with the RAP Makefile System compiles but for standalone\n");
   print(STDERR "         Makefiles such as RTFDDA Makefiles. It also assumes that the\n");
   print(STDERR "         default $Netcdf_dir is pointing to netCDF4.\n");
   print(STDERR " -d : Print debug messages\n");
   print(STDERR " -h : Help. Print this usage statement.\n");
   print(STDERR " -t : Test mode, don't actually change the file\n");
   print(STDERR " -v : Print verbose debug messages\n");
   exit $Exit_failure;
}

# Check debug flag
if ($opt_d) {
   $Debug = 1;
}

# Check test flag
if ($opt_t) {
    $TestMode = 1;
}

# Check verbose debug flag
if ($opt_v) {
   $VerboseDebug = 1;
}

# Look for needed libs and dirs

$found_errors=0;
$use_hdf_dir=$Hdf_dir;
if (!-e $use_hdf_dir) {
    $use_hdf_dir="/usr/local/hdf";
    if (!-e $use_hdf_dir) {
	print(STDERR "ERROR: HDF dir does not exist $Hdf_dir or $use_hdf_dir\n");
	$found_errors=1;
    }
}

$use_szip_dir=$Szip_dir;
if (!-e $use_szip_dir) {
    $use_szip_dir="/usr/local/szip";
    if (!-e $use_szip_dir) {
	print(STDERR "ERROR: szip dir does not exist $Szip_dir or $use_szip_dir\n");
	$found_errors=1;
    }
}

if ($found_errors > 0) {
    exit($Exit_failure);
}

# Print out debug info

if (($Debug) || ($VerboseDebug)) {
    print(STDERR "$0...\n");
    print(STDERR "Debug: $Debug\n");
    print(STDERR "TestMode: $TestMode\n");
    print(STDERR "VerboseDebug: $VerboseDebug\n");
}

# Set link line

$link_string="-L${use_hdf_dir} -lhdf5_hl -lhdf5 -lz -lm -L${use_szip_dir} -lsz";

# Go through each file on the command line

FILE: foreach $filename (@ARGV) {

    # ----- open each file -----

    print(STDERR "\n");
    print(STDERR "Modifying file: $filename\n");

    # open file for reading only

    if ($TestMode) {
	$source_file=$filename;
    }

    # copy file to .bak file then read .bak file and write to original filename

    else {
	$source_file = $filename . ".bak";
	system ('/bin/mv', $filename, $source_file);

	if (!open(OUTFILE, ">$filename")) {
	    print(STDERR "WARNING: Cannot open output file $filename - continuing ... \n");
	    next FILE;
	}
    } #end else

    # Open the source file

    if (!open(SRCFILE, $source_file)) {
	print(STDERR "WARNING: Cannot open source file $source_file - continuing ... \n");
	next FILE;
    }

    # --------------- loop through the lines in the input file ---------

    while ($line = <SRCFILE>) {

	# Look for lines with -lnetcdf in them

	if ($line =~ /\-lnetcdff \-lnetcdf/) {
	    &printdebug("Found line with -lnetcdff -lnetcdf: line: $line");
	    &dochange("-lnetcdff -lnetcdf", "-lnetcdff -lnetcdf ${Link_string}", $TestMode, $Debug);
	} elsif (($line =~ /\-lnetcdf/) && ($line !~ /\-lnetcdff/)) {
	    &printdebug("Found line with -lnetcdf only: line: $line");
	    &dochange("-lnetcdf", "-lnetcdf ${Link_string}", $TestMode, $Debug);
	}

	# print line to output file

	&doprint($line);

    } # end while

    close(SRCFILE);
    close(OUTFILE);

} # end foreach

exit($Exit_success);

#
#------------------------------- SUBROUTINES -----------------------------
#
sub dochange {
    local($searchstr, $replacestr, $test_flag, $debug_flag) = @_;

    if ($debug_flag) {
	&printdebug("search for: $searchstr and replace with: $replacestr\n");
    }

    # search for string and change it if we are not in test mode

    if ($line =~ /$searchstr/) {

        if ($test_flag) {
            &printdebug("\tFound: $searchstr, would replace with $replacestr\n");
	}
        else {
	    if ($debug_flag) {
		&printdebug("\tChanging: $searchstr \tto: $replacestr\n");
	    }

  	    $line =~ s/$searchstr/$replacestr/;
	}
    } #endif $line
}

#--------------------------------------------- -----------------------------
sub doprint {
    local($printstr) = @_;

    if (!$TestMode) {
	print(OUTFILE $printstr);
    } 
}

#--------------------------------------------- -----------------------------
sub printdebug {
    local($printstr) = @_;

    if ($Debug) {
	print(STDERR $printstr);
    }
}

#--------------------------------------------- -----------------------------
sub printverbosedebug {
    local($printstr) = @_;

    if ($VerboseDebug) {
	print(STDERR $printstr);
    }
}
