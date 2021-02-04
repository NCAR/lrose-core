#! /usr/bin/perl
#
# mod_hdr_for_doxygen.pl
#
# Function:
#	Perl script to change *.hh and *.h files for use with
#       Doxygen. This uses the simplest form of the Doxygen tags for
#       the header file code comments.
#
# Usage:
#       mod_hdr_for_doxygen.pl [-h] [-d] [-t] <list of source files>
#
#       -h : help, print usage
#       -d : print debug messages
#       -t : test mode, don't actually change the file
#
# Input:
#       Source files (*.hh)
#
# Output:
#       Copies the input source file into <file>.bak and puts the 
#       filtered file into <file>.
#
# Dependencies:
#	Perl must be available in the location at the top of this file.
# 	
# Author: Deirdre Garvey - NCAR/RAP/SDG		14-NOV-2003
#         from a similar Perl script mod_c++_to_gcc32.pl
#
##
#---------------------------------------------------------------------------------

# Need the Perl getopts library routine
use Getopt::Std qw( getopts );

# Program defaults

$Exit_failure=0;
$Exit_succcess=1;

# Command line defaults
$Debug=0;
$Test_mode=0;
$VerboseDebug=0;

# Get command line options
&getopts('hdtv'); 

if ($opt_h) {
   print(STDERR "Usage: $0 [-h] [-d] [-t] [-v] <files> ...\n");
   print(STDERR "Purpose: Modifies input .h and .hh files to add simple Doxygen tags.\n");
   print(STDERR "         For .hh files will change // to ///.\n");
   print(STDERR "         For .h files will change /* to /**.\n");
   print(STDERR "         This script does not attempt to be \"smart\" about finding\n");
   print(STDERR "         true function descriptions as opposed to general comments.\n");
   print(STDERR "files: Required. Must be *.hh and/or *.h files\n");
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
    $Test_mode = 1;
}

# Check verbose debug flag
if ($opt_v) {
   $VerboseDebug = 1;
}

# Print out debug info

if ($Debug) {
    print(STDERR "Debug: $Debug\n");
    print(STDERR "Test_mode: $Test_mode\n");
    print(STDERR "VerboseDebug: $VerboseDebug\n");
}

# Go through each file on the command line

FILE: foreach $filename (@ARGV) {

    # ----- skip the file if it is not .cc or .hh --

    $is_h=0;
    $is_hh=0;
    if ($filename =~ /$\.hh/) { 
	$is_hh=1;
    } elsif ($filename =~ /$\.h/) {
	$is_h=1;
    } else {
	print(STDERR "Skipping $filename, not .h or .hh\n");
	next;
    }

    &printverbosedebug("filename: $filename, is_h: $is_h, is_hh: $is_hh\n");

    # ----- open each file -----

    print(STDERR "\n");
    print(STDERR "Modifying file: $filename\n");

    # open file for reading only

    if ($Test_mode) {
	$source_file=$filename;
    }

    # copy file to .bak file then read .bak file and write to file

    else {
	$source_file = $filename . ".bak";
	system ('/bin/mv', $filename, $source_file);

	if (!open(OUTFILE, ">$filename")) {
	    print(STDERR "Cannot open output file $filename - continuing ... \n");
	    next FILE;
	}
    } #end else

    # Open the source file

    if (!open(SRCFILE, $source_file)) {
	print(STDERR "Cannot open source file $source_file - continuing ... \n");
	next FILE;
    }

    # --------------- loop through the lines in the file ---------

    $found=0;
    $inside_copyright=0;
    $counter=0;

    while ($line = <SRCFILE>) {

	# ---------------- Skip certain lines ------------------------

	# Skip the copyright

	if ($line =~ /\*\=\*\=\*\=/) {
	    if ($inside_copyright) {
		$inside_copyright=0;
	    } else {
		$inside_copyright=1;
	    } 
	    &doprint($line);
	    next;
	}
	if ($inside_copyright) {
	    &doprint($line);
	    next;
	}

	# Look for doxygen specific lines that we do not want in the
	# general case. Do not print the line.

	if ((($is_h) || ($is_hh)) && ($line =~ /\\mainpage/)) {
	    print(STDERR "Skipping line: $line\n");
	    next;
	}

	# Skip C++ comments that have already been changed.

	if ($is_hh) {

	    # 3 or more slashes

	    if ($line =~ /\s*\/\/\//) {
		&doprint($line);
		next;
	    }
	}

	# Skip C comments that have already been changed.

	if ($is_h) {

	    # /** and a space

	    if ($line =~ /\s*\/\*\*\s+/) {
		&doprint($line);
		next;
	    }

	    # /** and a newline

	    if ($line =~ /\s*\/\*\*\$/) {
		&doprint($line);
		next;
	    }
	}

	# ----------------- Lines to change ------------------------

	# Look for C++ comments

	if ($is_hh) {

	    # C++ comments with 2 slashes in them, need to change to 3

	    if ($line =~ /\s*\/\/\s+/) {
		&dochange("//", "///", $Test_mode, $Debug);
	    }
	}

	# Look for C comments

	if ($is_h) {

	    # C comments with a /*******, need to change to /** ****

	    if ($line =~ /\s*\/\*\*\*/) {
		&dochange('/\*\*\*', '/** *', $Test_mode, $Debug);
	    }

	    # C comments with a /*, need to change to /**
	    # The strange syntax below works, not sure why have to
	    # use a /* for the replacement string to get /**...

	    elsif ($line =~ /\s*\/\*/) {
		&dochange("/\*", "/*", $Test_mode, $Debug);
	    }
	}

	# --------------------- END changes to file ------------------

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

	&printverbosedebug("\tline: $line");

        if ($test_flag) {
            &printdebug("\tFound: $searchstr\n");
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

    if (!$Test_mode) {
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

#--------------------------------------------- -----------------------------
