#!/usr/bin/perl
#
# Name: parse_makefile_subdirs.pl
#
# Function: Read the input Makefile to parse out the SUB_DIRS list and write
#           it to an output file. This output file can then be read by
#           the nightly build script.
#
# Input:    A Makefile with a list of SUB_DIRS
#
# Output:   A temporary file containing the list of SUB_DIRS on one line
#
# Dependencies:
#
# Author:   Deirdre Garvey         21-NOV-2001
#
#----------------------------------------------------------------------------

#       Externals

use Getopt::Long;

sub badArg;
sub translateDataDir;
sub verifyDataDir;

# Set program defaults

($prog = $0) =~ s|.*/||;               # Get the program basename

# Set default flags for command line args

$Debug=0;
$Debug_level=0;

# Save the usage to print to the user if there is a problem

$usage =                                                 
    "\nUsage: $prog [-dh] -i <file> -o <file>\n" .
    "Purpose: Read the input Makefile for the list of SUB_DIRS and write\n" .
    "         an output file containing only the list of SUB_DIRS on one line.\n" .
    "         This output file can then be used by the nightly build script\n" .
    "         to go through the list of SUB_DIRS.\n" .
    "   -d --debug          : Print debug messages\n" .
    "   -h --help           : Print this usage message\n" .
    "   -i --input <file>   : Makefile to read\n" .
    "   -o --output <file>  : Output file to write. Will be overwritten if it exists\n" ;

# Get the arguments from the command line

$result = &GetOptions('debug',
		      'help',
		      'input=s',
		      'output=s',
                       '<>', \&badArg );

if ( $result == 0 || $opt_help ) {
   print $usage;
   exit 0;
}

if ($opt_debug) {
    $Debug = 1;
    print(STDERR "Input options specified: \n");
    print(STDERR "\tinput file: $opt_input\n");
    print(STDERR "\toutput file: $opt_output\n");
}

if ($opt_input) {
    $InFile=$opt_input;
} else {
    print(STDERR "ERROR: You must specify an input file\n");
    exit -1;
}

if ($opt_output) {
    $OutFile=$opt_output;
} else {
    print(STDERR "ERROR: You must specify an output file\n");
    exit -1;
}

# Error checking

if (!-e $InFile) {
    print(STDERR "ERROR: The input file $InFile does not exist\n");
    exit -1;
}

# Open the input file and search for SUB_DIRS

if (!open(INFILE, $InFile)) {
    print(STDERR "Cannot open input file $InFile\n");
    exit -1;
}

$found=0;
$found_start=0;
$found_end=0;
$counter=0;
$output_list="";
while ($line = <INFILE>) {

    chop($line);

    # Skip the rest of the file

    if ($found_end) {
	next;
    }

    # Look for the SUB_DIRS

    if ($line =~ /^SUB_DIRS\s+\=/) {
	$found=1;
	$found_start=1;

	if ($Debug) {
	    print(STDERR "Found the start of SUB_DIRS, line: $line\n");
	}

	# Is there anything after the = on this line?

	@pieces=split(/\=/, $line);
	$npieces=@pieces;

	if ($Debug_level == 2) {
	    for ($i=0; $i<$npieces; $i++) {
		print(STDERR "i: $i, piece: $pieces[$i]\n");
	    }
	}

	if ($npieces > 1) {

	    if ($Debug) {
		print(STDERR "Found entries on the SUB_DIRS line, npieces: $npieces\n");
	    }

	    # Skip the case where the only entry is a line continuation char

	    if ($pieces[1] =~ /\s+\\/) {
		if ($Debug) {
		    print(STDERR "Skipping this line, only a line continuation\n");
		}
	    } else {
		# Add to the output list

		if ($Debug){
		    print(STDERR "\tadding to list: $pieces[1], counter: $counter\n");
		}

		$output_list="$output_list $pieces[1]";
		$counter++;
	    }
	}

	next;
    }

    # We are inside the SUB_DIRS list. Add to the array

    if (($found_start) && (!$found_end)) {
	if ($Debug) {
	    print(STDERR "Found SUB_DIR entry, line: $line\n");
	}

	# Need to remove leading whitespace, trailing continuation lines
	# and to split multiple entries if there are any

	@pieces=split(/\b/, $line);
	$npieces=@pieces;

	if ($Debug_level == 2) {
	    for ($i=0; $i<$npieces; $i++) {
		print(STDERR "i: $i, piece: $pieces[$i]\n");
	    }
	}
	
	for ($i=0; $i<$npieces; $i++) {

	    # Skip line continuation characters

	    if ($pieces[$i] =~ /\\/) {
		next;
	    }

	    # Skip blanks

	    if ($pieces[$i] =~ /\s+/) {
		next;
	    }

	    # Add to the output list

	    if ($Debug){
		print(STDERR "\tadding to list: $pieces[$i], counter: $counter\n");
	    }

	    $output_list="$output_list $pieces[$i]";

	    $counter++;
	}
    }

    # Look for the end of the SUB_DIRS list. Search for a line without
    # a line continuation character or a blank line

    if (($found_start) && (($line !~ /\\/) || ($line !~ /\w/))) {
	$found_end=1;
	if ($Debug) {
	    print(STDERR "Found the end of the SUB_DIR entries, line: $line\n");
	}
    }
}

# Close the input file

close(INFILE);

# Did we find any SUB_DIRS entries?

if (!$found) {
    print(STDERR "ERROR: Did not find any SUB_DIR entries in the input file, $InFile\n");
    print(STDERR "\tThe output file $OutFile will be empty.\n");
    if (-e $OutFile) {
	unlink($OutFile);
    }
    system("touch $OutFile");
    exit 0;
}

# Write the output file	

if ($Debug) {
    print(STDERR "Found $counter entries\n");
}

if (!open(OUTFILE, ">$OutFile")) {
    print(STDERR "Cannot open file $OutFile\n");
    exit -1;
}

if ($Debug) {
    print(STDERR "Printing to outfile...\n");
    print(STDERR "$output_list\n");
}

print(OUTFILE "$output_list\n");
close(OUTFILE);

# Done

exit 0;
