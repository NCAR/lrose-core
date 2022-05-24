#! /usr/bin/perl
#
# mod_rtfdda_for_openmpi.pl
#
# Function:
#	Perl script to change configure files as required to compile with
#       openmpi instead of mpich
#
# Usage:
#       mod_rtfdda_for_openmpi.pl -h
#
# Input:
#       Source files (e.g., <dir-path>/configure.defaults
#
# Output:
#       Copies the input source file into <file>.bak and puts the 
#       filtered file into <file>.
#
# Dependencies:
#	Perl must be available in the location at the top of this file.
# 	
# Author: Deirdre Garvey - NCAR/RAL/SDG		12-JUL-2010
#         from a similar Perl script mod_c++_to_gcc32.pl
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

# Command line defaults
$Debug=0;
$TestMode=0;
$VerboseDebug=0;
$DoFastSse=0;
$DoNoSwitchError=0;

# Get command line options
&getopts('hdfntv'); 

if ($opt_h) {
   print(STDERR "Usage: $0 [-h] [-d] [-f] [-n] [-t] [-v] <files> ...\n");
   print(STDERR "Purpose: Modifies the input build files to change from mpich to openmpi\n");
   print(STDERR "         Only for MM5, WRF+friends configure.defaults type files\n");
   print(STDERR " -d : Print debug messages\n");
   print(STDERR " -f : Also mod to add -fastsse\n");
   print(STDERR " -h : Help. Print this usage statement.\n");
   print(STDERR " -n : Also mod mpif90 to add -noswitcherror (needed on Debian?)\n");
   print(STDERR " -t : Test mode, don't actually change the file\n");
   print(STDERR " -v : Print verbose debug messages\n");
   exit $Exit_failure;
}

# Check debug flag
if ($opt_d) {
   $Debug = 1;
}

# Check fastsse flag
if ($opt_f) {
   $DoFastSse = 1;
}

# Check noswitcherror flag
if ($opt_n) {
   $DoNoSwitchError = 1;
}

# Check test flag
if ($opt_t) {
    $TestMode = 1;
}

# Check verbose debug flag
if ($opt_v) {
   $VerboseDebug = 1;
}

# Look for /opt/openmpi

$MpiTopDir="/opt/openmpi";
if (!-e $MpiTopDir) {
    print(STDERR "Warning: /opt/openmpi does not exist. Will check if env var MPI_TOP defined and exists\n");
    $mpi_top_dir=$ENV{MPI_TOP};
    if (!-e $mpi_top_dir) {
	print(STDERR "Warning: env var MPI_TOP also does not exist. May cause compile problems.\n");
    } else {
	$MpiTopDir=$mpi_top_dir;
    }
}   

# Print out debug info

if (($Debug) || ($VerboseDebug)) {
    print(STDERR "Debug: $Debug\n");
    print(STDERR "DoFastSse: $DoFastSse\n");
    print(STDERR "DoNoSwitchError: $DoNoSwitchError\n");
    print(STDERR "TestMode: $TestMode\n");
    print(STDERR "VerboseDebug: $VerboseDebug\n");
}

# Go through each file on the command line

FILE: foreach $filename (@ARGV) {

    # ----- skip the file if it is not configure-something--

    if ($filename !~ /configure/) { 
	print(STDERR "Skipping $filename, not a configure-type file\n");
	next;
    }

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

	# Look for lines with /opt/mpich in them

	if ($line =~ /\/opt\/mpich/) {
	    &printdebug("Found line with /opt/mpich: line: $line");
	    &dochange("/opt/mpich", $MpiTopDir, $TestMode, $Debug);
	}

	# Look for lines with -lfmpich -lmpich in them

	if (($line =~ /\-lfmpich/) && ($line =~ /\-lmpich/)) {
	    &printdebug("Found line with -fmpich -lmpich: line: $line");
	    &dochange('\-lfmpich \-lmpich', '\-lmpi_f77 \-lmpi', $TestMode, $Debug);
	}

	# Look for lines with -f90=...

	if ($line =~ /\-f90\=/) {
	    &printdebug("Found line with -f90=... : line: $line");
	    if ($line =~ /\-f90\=pgf90/) {
		&dochange('\-f90\=pgf90', '', $TestMode, $Debug);
	    } elsif ($line =~ /-f90=\$\(SFC\)/) {
		&dochange('\-f90\=\$\(SFC\)', '', $TestMode, $Debug);
	    } elsif ($line =~ /-f90=\$\(SF90\)/) {
		&dochange('\-f90\=\$\(SF90\)', '', $TestMode, $Debug);
	    }
	}

	# Look for lines with mpicc -cc=...

	if ($line =~ /mpicc \-cc\=/) {
	    &printdebug("Found line with mpicc -cc=... : line: $line");
	    if ($line =~ /\-cc\=gcc/) {
		&dochange('mpicc \-cc\=gcc', 'mpicc -DRSL_LOCAL', $TestMode, $Debug);
	    } elsif ($line =~ /-cc=\$\(SCC\)/) {
		&dochange('mpicc \-cc\=\$\(SCC\)', 'mpicc -DRSL_LOCAL', $TestMode, $Debug);
	    } elsif ($line =~ /-cc=pgcc/) {
		&dochange('mpicc \-cc\=pgcc', 'mpicc -DRSL_LOCAL', $TestMode, $Debug);
	    }
	}

	# Look for lines with FCOPTIM to add fastsse

	if (($DoFastSse) && (($line =~ /^FCOPTIM/) || ($line =~ /^\#FCOPTIM/)) && 
	    ($line !~ /fastsse/)) {
	    &printdebug("Found line with FCOPTIM and no fastsse: line: $line");
	    if (($line =~ /\-O3/) && ($line !~ /\-O2/)) {
		&dochange('\-O3', '\-O3 \-fastsse', $TestMode, $Debug);
	    } elsif (($line !~ /\-O3/) && ($line =~ /\-Mvect\=noaltcode/)) {
		&dochange('\-Mvect\=noaltcode', '\-fastsse \-Mvect\=noaltcode', $TestMode, $Debug);
	    }
	}

	if (($DoNoSwitchError) && ($line =~ /mpif90/) && ($line !~ /noswitcherror/)) {
	    &printdebug("Found line with mpif90: line: $line");
	    &dochange('mpif90', 'mpif90 \-noswitcherror', $TestMode, $Debug);
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
