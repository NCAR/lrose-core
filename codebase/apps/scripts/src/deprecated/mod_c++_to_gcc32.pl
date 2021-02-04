#! /usr/bin/perl
#
# mod_c++_to_gcc32.pl
#
# Function:
#	Perl script to change *.cc and *.hh files to more standard
#       syntax. This is needed in order to compile under gcc 3.2
#
# Usage:
#       mod_c++_to_gcc32.pl [-h] [-d] [-t] <list of source files>
#
#       -h : help, print usage
#       -d : print debug messages
#       -t : test mode, don't actually change the file
#
# Input:
#       Source files (*.cc, *.hh)
#
# Output:
#       Copies the input source file into <file>.bak and puts the 
#       filtered file into <file>.
#
# Dependencies:
#	Perl must be available in the location at the top of this file.
# 	
# Author: Deirdre Garvey - NCAR/RAP/SDG		20-JAN-2003
#         from a similar Perl script ch_awpg_inc.pl
#
##
#---------------------------------------------------------------------------------

# The sys_wait_h is required to get the correct return codes from the system() calls.
require 5.002;
use POSIX 'sys_wait_h'; 

# Need the Perl getopts library routine
use Getopt::Std qw( getopts );

# Program defaults

$Exit_failure=0;
$Exit_succcess=1;

# Command line defaults
$Debug=0;
$Test_mode=0;
$VerboseDebug=0;
$Do_namespace=1;
$Do_incs=1;
$Do_args=1;
$Do_elif=1;
$Do_namespace_move=0;

# Get command line options
&getopts('hadeimntv'); 

if ($opt_h) {
   print(STDERR "Usage: $0 -h [-a] [-d] [-e] [-i] [-m] [-n] [-t] [-v] <files> ...\n");
   print(STDERR "Purpose: Modifies the input .cc and .hh files to make the\n");
   print(STDERR "         syntax more C++ standard so can be compiled with gcc 3.2\n");
   print(STDERR " -a : Do NOT modify the default args in functions in .cc files\n");
   print(STDERR "      Default is to modify the default args\n");
   print(STDERR " -d : Print debug messages\n");
   print(STDERR " -e : Do NOT modify \#else if to \#elif in input files\n");
   print(STDERR "    : Default is to modify the \#else if\n");
   print(STDERR " -h : Help. Print this usage statement.\n");
   print(STDERR " -i : Do NOT modify the \#include directives in the input files\n");
   print(STDERR " -m : Move the using namespace std to be after the last \#include\n");
   print(STDERR "      Default is to only add it if it is missing, not to move it\n");
   print(STDERR " -n : Do NOT add: using namespace std to the input files\n");
   print(STDERR "    : Default is to add the namespace line\n");
   print(STDERR " -t : Test mode, don't actually change the file\n");
   print(STDERR " -v : Print verbose debug messages\n");
   exit $Exit_failure;
}

# Check args flag
if ($opt_a) {
    $Do_args=0;
}

# Check debug flag
if ($opt_d) {
   $Debug = 1;
}

# Check elsif flag
if ($opt_e) {
    $Do_elif=0;
}

# Check incs flag
if ($opt_i) {
    $Do_incs=0;
}

# Check move namespace flag
if ($opt_m) {
    $Do_namespace_move=1;
}

# Check namespace flag
if ($opt_n) {
    $Do_namespace=0;
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
    print(STDERR "Do_args: $Do_args\n");
    print(STDERR "Do_elif: $Do_elif\n");
    print(STDERR "Do_incs: $Do_incs\n");
    print(STDERR "Do_namespace_move: $Do_namespace_move\n");
    print(STDERR "Do_namespace: $Do_namespace\n");
    print(STDERR "Test_mode: $Test_mode\n");
    print(STDERR "VerboseDebug: $VerboseDebug\n");
}

# Go through each file on the command line

FILE: foreach $filename (@ARGV) {

    # ----- skip the file if it is not .cc or .hh --

    $is_cc=0;
    $is_hh=0;
    if ($filename =~ /$\.cc/) { 
	$is_cc=1;
    } elsif ($filename =~ /$\.hh/) {
	$is_hh=1;
    } else {
	print(STDERR "Skipping $filename, not .cc or .hh\n");
	next;
    }

    &printverbosedebug("filename: $filename, is_cc: $is_cc, is_hh: $is_hh\n");

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

    # --------------- Does the file have namespace already? ------

    $file_has_namespace=0;
    if (($Do_namespace) || ($Do_namespace_move)) {
	$cmd="grep \"\^using namespace std\" $source_file";
	&printverbosedebug("Running: $cmd\n");
	$is_ok=system($cmd);
	$found_namespace=WEXITSTATUS($?);
	if ($found_namespace == 0) {
	    if (($Do_namespace) && (!$Do_namespace_move)) {
		&printdebug("Found namespace so will not re-add: $source_file\n");
	    } elsif ($Do_namespace_move) {
		&printdebug("Found namespace so will move it: $source_file\n");
	    }
	    $file_has_namespace=1;
	} else {
	    &printdebug("File does not have using namespace std so will add\n");
	    $Do_namespace_move=0;
	    $Do_namespace=1;
	}
    }

    # --------------- Look for the last include line in the file ------

    $file_add_namespace=0;
    $nincludes=0;
    if (($Do_namespace) || ($Do_namespace_move)) {

	&printdebug("Search for last include line\n");

	$found_last_include=0;
	$last_include_line="NULL";

	while ($line = <SRCFILE>) {
	    if ($line =~ /^\#include/) {
		$found_last_include=1;
		$last_include_line=$line;
		$nincludes++;
	    }
	}
	close(SRCFILE);

	if (!$found_last_include) {
	    print(STDERR "ERROR: Cannot find the last include file!\n");
	}

	if (!open(SRCFILE, $source_file)) {
	    print(STDERR "Cannot open source file $source_file - continuing ... \n");
	    next FILE;
	}

	# Special case, if only include is a copyright.h, skip the namespace

	if (($nincludes == 1) && ($last_include_line =~ /copyright\.h/)) {
	    $file_add_namespace=0;
	    &printdebug("Skip namespace, only include is $last_include_line");
	} else {
	    $file_add_namespace=1;
	}
    }

    # --------------- loop through the lines in the file ---------

    $add_using_after_line=0;
    $found=0;
    $counter=0;
    $first_line=0;
    $inside_function=0;
    $inside_comments=0;

    while ($line = <SRCFILE>) {

	# Skip comments

	if ($line =~ /^\/\//) {
	    &doprint($line);
	    next;
	}

	if (($line =~ /^\/\*/) && ($line =~ /\*\//)) {
	    &doprint($line);
	    next;
	}

	if (($line =~ /^\/\*/) && ($line != /\*\//)) {
	    $inside_comments=1;
	    &doprint($line);
	    next;
	}

	if (($line =~ /\*\//) && ($inside_comments)) {
	    $inside_comments=0;
	    &doprint($line);
	    next;
	}
	
	if ($inside_comments) {
	    &doprint($line);
	    next;
	}

	# Skip lines with == and others in them, not likely to need changing

	if (($line =~ /\=\=/) || ($line =~ /\>\=/) || 
	    ($line =~ /\<\=/) || ($line =~ /\!\=/) ||
	    ($line =~ /\|\|/) || ($line =~ /\&\&/)) {
	    &doprint($line);
	    next;
	}

	# ---------------- Look for using directives ------
	# Save the line but do not print it

	if ($Do_namespace_move) {
	    if ($line =~ /^using\s*namespace\s*std/) {
		&printdebug("Found line to move, will skip: $line");
		next;
	    }
	}

	# --------------- Look for the last include --------
	# Need to look for the line before modify it

	if (($found_last_include) && ($file_add_namespace) && 
	    (($Do_namespace) || ($Do_namespace_move))) {
	    if ($line eq $last_include_line) {
		&printverbosedebug("Found the last include line: $line");
		$add_using_after_line=1;
	    }
	}

	# ---------------- Look for include directives ------

	if ($line =~ /^\#include/) {

	    if ($Do_incs) {
		
		# Skip includes that have subdirs, we are only replacing system
		# level includes

		if ($line =~ /\//) {
		    &printdebug("Skipping include line has a / in it: $line");
		} else {

		    # Substitutions for includes

		    if ($line =~ /stdio\.h/) {
			&dochange("stdio.h", cstdio, $Test_mode, $Debug);
		    }
		    if ($line =~ /time\.h/) {
			&dochange("time.h", ctime, $Test_mode, $Debug);
		    }
		    if ($line =~ /stdarg\.h/) {
			&dochange("stdarg.h", cstdarg, $Test_mode, $Debug);
		    }
		    if ($line =~ /iostream\.h/) {
			&dochange("iostream.h", iostream, $Test_mode, $Debug);
		    }
		    if ($line =~ /ostream\.h/) {
			&dochange("ostream.h", iostream, $Test_mode, $Debug);
		    }
		    if ($line =~ /stdlib\.h/) {
			&dochange("stdlib.h", cstdlib, $Test_mode, $Debug);
		    }
		    if ($line =~ /errno\.h/) {
			&dochange("errno.h", cerrno, $Test_mode, $Debug);
		    }
		    if ($line =~ /assert\.h/) {
			&dochange("assert.h", cassert, $Test_mode, $Debug);
		    }
		    if ($line =~ /fstream\.h/) {
			&dochange("fstream.h", fstream, $Test_mode, $Debug);
		    }
		    if ($line =~ /stream\.h/) {
			&dochange("stream.h", iostream, $Test_mode, $Debug);
		    }
		    if ($line =~ /function\.h/) {
			&dochange("function.h", functional, $Test_mode, $Debug);
		    }
		    if ($line =~ /iomanip\.h/) {
			&dochange("iomanip.h", iomanip, $Test_mode, $Debug);
		    }
		    if ($line =~ /algo\.h/) {
			&dochange("algo.h", algorithm, $Test_mode, $Debug);
		    }
		    if ($line =~ /pair\.h/) {
			&dochange("pair.h", utility, $Test_mode, $Debug);
		    }
		    if ($line =~ /new\.h/) {
			&dochange("new.h", new, $Test_mode, $Debug);
		    }
		    if ($line =~ /multimap\.h/) {
			&dochange("multimap.h", "map", $Test_mode, $Debug);
		    }
		} #endelse

		# Print the line
	    
		&doprint($line);
	    
		# Skip to the next line if not at the last include

		if (!$add_using_after_line) {
		    next;
		}
	    } #endif $Do_incs
	} #endif $line has include

	# ---------------------------- Add using ----------------------

	if (($add_using_after_line) && (($Do_namespace) || ($Do_namespace_move))) {

	    # Print the last include line

	    if (!$Do_incs) {
		&doprint($line);
	    }

	    # Print the using directive

	    if (($Do_namespace_move) || (($Do_namespace) && (!$file_has_namespace))) {
		&printdebug("Adding namespace std\n");
		print(OUTFILE "using namespace std;\n");
	    }
	    $add_using_after_line=0;
	    next;
	}

	# ---------------- Look for else if directives ------
	# Need to do the explicit substitution here because dochange() loses
	# the \s*

	if ($Do_elif) {
	    if ($line =~ /^#else\s*if/) {
		&printdebug("Change line from: $line");
		$line =~ s/\#else\s*if/\#elif/;
		&printdebug("\tto $line");
		&doprint();
		next;
	    }
	}

	# ---------------- Look for defaults in .cc function declarations ------

	# These defaults are okay in .hh files so only check .cc files.
	# The default cannot be the first variable in the declaration.
	# Can distinguish declarations because they have parens ()
	# and are followed by curly braces not semi-colons.

	if (($is_cc) && ($Do_args)) {

	    # If we are inside a function declaration, no modifying code

	    if ($line =~ /^\{/) {
		$inside_function = 1;
		&doprint($line);
		next;
	    }

	    if (($line =~ /^\}/) && ($inside_function)) {
		$inside_function = 0;
		&doprint($line);
		next;
	    }

	    if ($inside_function) {
		&doprint($line);
		next;
	    }

	    # Skip lines starting with if or for

	    if (($line =~ /^\s*if/) || ($line =~ /^\s*for/)) {
		&doprint($line);
		next;
	    }

	    # Skip lines starting with printf or fprintf

	    if ($line =~ /printf/) {
		&doprint($line);
		next;
	    }
		
	    # Look for lines with a left paren that do not end in a right paren and semi-colon
	    # we are looking for function declarations

	    if (($line =~ /\(/) && ($line !~ /\)\s*\;/)) {
		&printverbosedebug("Found a line with a left paren and no right paren+semi-colon: $line");
		if (!$found) {
		    $found=1;
		    $lines_arr[0]=$line;
		    $counter=1;
		    $first_line=1;
		}
	    }

	    if (($found) && ($line =~/\w*\(\s*\)/)) {
		&printverbosedebug("Never mind, is a null declaration\n");
		if (!$first_line) {
		    $lines_arr[$counter]=$line;
		    $counter=$counter+1;		
		}
		$do_mod=0;
		&printarray(*lines_arr, $counter, $do_mod);
		$counter=0;
		$found=0;
		$first_line=0;
		next;
	    }

	    if (($found) && ($line =~ /\)\s*\;/)) {
		&printverbosedebug("Never mind, found the right paren and it has a semi-colon\n");
		if (!$first_line) {
		    $lines_arr[$counter]=$line;
		    $counter=$counter+1;		
		}
		$do_mod=0;
		&printarray(*lines_arr, $counter, $do_mod);
		$counter=0;
		$found=0;
		$first_line=0;
		next;
	    }

	    if (($found) && ($line =~ /\)\s*\:/)) {
		&printverbosedebug("Never mind, found a colon to the right of the right paren\n");
		if (!$first_line) {
		    $lines_arr[$counter]=$line;
		    $counter=$counter+1;		
		}
		$do_mod=0;
		&printarray(*lines_arr, $counter, $do_mod);
		$counter=0;
		$found=0;
		$first_line=0;
		next;
	    }

	    if (($found) && ($line =~ /\=.*\(/)) {
		&printverbosedebug("Never mind, the equals is left of the left paren\n");
		if (!$first_line) {
		    $lines_arr[$counter]=$line;
		    $counter=$counter+1;		
		}
		$do_mod=0;
		&printarray(*lines_arr, $counter, $do_mod);
		$counter=0;
		$found=0;
		$first_line=0;
		next;
	    }

	    if (($found) && ($line =~ /\(.*\).*\{.*\}/)) {
		&printverbosedebug("Never mind, this is weird condensed syntax\n");
		&doprint($line);
		$counter=0;
		$found=0;
		$first_line=0;
		next;
	    }

	    if (($found) && ($line =~ /\(/) && (!$first_line)) {
		&printverbosedebug("Never mind, found an imbedded function call\n");
		if (!$first_line) {
		    $lines_arr[$counter]=$line;
		    $counter=$counter+1;		
		}
		$do_mod=0;
		&printarray(*lines_arr, $counter, $do_mod);
		$counter=0;
		$found=0;
		$first_line=0;
		next;
	    }

	    # Add to the array until we find the end paren, no semi-colon

	    if (($found) && ( $line !~ /\)/)) {
		
		# Skip if we are still on the first line

		if ($first_line) {
		    $first_line=0;
		    next;
		}

		&printverbosedebug("Found a line, counter; $counter, have not found the right paren yet: $line");
		$lines_arr[$counter]=$line;
		$counter=$counter+1;		
		next;
	    }

	    if (($found) && ($line =~ /\)/)) {
		&printverbosedebug("Found the end paren, counter: $counter, now print the lines: $line");
		
		# Do not increment if we we are still on the first line

		if (!$first_line) {
		    $lines_arr[$counter]=$line;
		    $counter=$counter+1;
		}
		$do_mod=1;
		&printarray(*lines_arr, $counter, $do_mod);
		$found=0;
		$counter=0;
		$first_line=0;
		next;
	    }

	} #endif $is_cc

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
sub printarray {
    local(*arr, $narr, $mod) = @_;

    local($i, $str, $newstr);

    &printverbosedebug("printing array, narr: $narr, mod: $mod\n");

    for ($i=0; $i<$narr; $i++) {
	$str=$arr[$i];
	$newstr=&modline($str, $mod);
	&printverbosedebug("Printing array at i: $i, $newstr");
	&doprint($newstr);
    }
}

#--------------------------------------------- -----------------------------
sub modline {
    local($inline, $mod) = @_;

    local($found, $left, $right, $value, $rest, $end_paren, $end_comma);
    local(*comma_arr, $ncomma_arr, $j, $comma_str, $outline);
    local($skip, $is_ok, $junk, $right_of_paren);

    # Set defaults

    $end_paren=0;
    $end_comma=0;
    $outline=$inline;

    # Skip if we are not modifying the line

    if (!$mod) {
	&printverbosedebug("Skipping mod: $outline");
	return($outline);
    }

    # Skip any change if we do not have an equals sign

    if ($inline !~ /\=/) {
	&printverbosedebug("No equals, not modify: $outline");
	return($outline);
    }

    # Skip any change if the equals is left of a left paren

    if ($inline =~ /\=.*\(/) {
	&printverbosedebug("Not modify: $outline");
	return($outline);
    }

    # Okay, we have an equals now need to do the commenting

    &printverbosedebug("=== Found equals: $inline");

    # Remove the <CR>

    chop($outline);

    # Does this have a final right paren?

    if ($outline =~ /\)/) {
	$end_paren=1;
    }

    # Does this have a final comma?

    if ($outline =~ /$\,/) {
	$end_comma=1;
    }

    # What is to the right of the right paren?

    ($junk, $right_of_paren)=split('\)', $outline);

    # First split on commas

    @comma_arr=split(',', $outline);
    $ncomma_arr=@comma_arr;
    &printverbosedebug("comma arr: ncomma_arr: $ncomma_arr\n");

    $outline="";

    for ($j=0; $j<$ncomma_arr; $j++) {

	$comma_str=$comma_arr[$j];
	$skip=0;

	# Is there an equal with a comment char already around it?

	if ($comma_str =~ /\/\*\s*\=/) {
	    &printverbosedebug("$comma_str already is commmented out, skip it\n");
	    $skip=1;
	}

	# Skip double equals

	if ($comma_str =~ /\=\=/) {
	    $skip=1;
	}

	# Is there an equal in this substring?

	if (($comma_str =~ /\=/) && (!$skip)) {

	    # Need to split out the equals and the value so can comment it
	    # Could have: int a = 1 )   or   int a=1)
		
	    ($left, $right)=split('=', $comma_str);
	    ($value, $rest)=split('\)', $right);

	    $outline=$outline . $left . '/*' . ' =' . $value . '*/';
	} else {
	    $outline=$outline . $comma_str;
	}
	
	# Add a comma

	if (($ncomma_arr > 1) && ($j < $ncomma_arr-1)) {
	    $outline = $outline . ",";
	}
    }

    # Add the ending paren and/or comma

    if (($end_paren) && ($outline !~ /\)/)) {
	$outline = $outline . ' )' ;
    } elsif (($end_comma) && ($outline !~ /$\,/)) {
	$outline = $outline . ',' ;
    }

    # Add anything to the right of the paren
    
    if ($right_of_paren =~ /\S+/) {
	$outline = $outline . $right_of_paren;
    }

    # Add <CR>

    $outline = $outline . "\n";

    # Does the output line differ from the input line?

    if ($outline eq $inline) {
	&printdebug("No change to: $inline");
    } else {
	&printdebug("Change line from: $inline");
	&printdebug("\tto: $outline");
    }
    return($outline);
}
