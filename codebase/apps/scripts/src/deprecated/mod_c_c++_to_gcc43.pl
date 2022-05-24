#! /usr/bin/perl
#
# mod_c_c++_to_gcc43.pl
#
# Function:
#	Perl script to change references in C and C++ source code files
#       to system include files as required to compile under gcc 4.3
#       (lenny)
#
# Usage:
#       mod_c_c++_to_gcc43.pl -h
#
# Input:
#       Source files (*.cc, *.hh, *.c, *.h)
#
# Output:
#       Copies the input source file into <file>.bak and puts the 
#       filtered file into <file>.
#
# Dependencies:
#	Perl must be available in the location at the top of this file.
# 	
# Author: Deirdre Garvey - NCAR/RAP/SDG		11-NOV-2008
#         from a similar Perl script mod_c++_to_gcc32.pl
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
$Do_incs=1;
$Do_exit_memcpy=1;
$Do_regexp=1;

# Get command line options
&getopts('hdeirtv'); 

if ($opt_h) {
   print(STDERR "Usage: $0 [-h] [-d] [-e] [-i] [-r] [-t] [-v] <files> ...\n");
   print(STDERR "Purpose: Modifies the input .cc, .hh, .c, .h files to modify system\n");
   print(STDERR "         include files so can be compiled with gcc 4.3\n");
   print(STDERR " -d : Print debug messages\n");
   print(STDERR " -e : Do NOT check for calls to exit(), memcpy() and add \#include directives in the input files\n");
   print(STDERR " -h : Help. Print this usage statement.\n");
   print(STDERR " -i : Do NOT replace the old \#includes with new in the input files\n");
   print(STDERR " -r : Do NOT modify the \#include <regexp.h> to add \#define __USE_GNU in the input files\n");
   print(STDERR " -t : Test mode, don't actually change the file\n");
   print(STDERR " -v : Print verbose debug messages\n");
   exit $Exit_failure;
}

# Check debug flag
if ($opt_d) {
   $Debug = 1;
}

# Check exit flag
if ($opt_e) {
    $Do_exit_memcpy=0;
}

# Check incs flag
if ($opt_i) {
    $Do_incs=0;
}

# Check regexp flag
if ($opt_r) {
    $Do_regexp=0;
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
    print(STDERR "Do_incs: $Do_incs\n");
    print(STDERR "Do_exit_memcpy: $Do_exit_memcpy\n");
    print(STDERR "Do_regexp: $Do_regexp\n");
    print(STDERR "Test_mode: $Test_mode\n");
    print(STDERR "VerboseDebug: $VerboseDebug\n");
}

# Go through each file on the command line

FILE: foreach $filename (@ARGV) {

    # ----- skip the file if it is not .cc or .hh --

    $is_cc=0;
    $is_hh=0;
    $is_h=0;
    $is_c=0;
    if ($filename =~ /\.cc$/) { 
	$is_cc=1;
    } elsif ($filename =~ /\.hh$/) {
	$is_hh=1;
    } elsif ($filename =~ /\.h$/) {
	$is_h=1;
    } elsif ($filename =~ /\.c$/) {
	$is_c=1;
    } else {
	print(STDERR "Skipping $filename, not .cc or .hh\n");
	next;
    }

    &printverbosedebug("filename: $filename, is_cc: $is_cc, is_hh: $is_hh, is_c: $is_c, is_h: $is_h\n");

    # ----- open each file -----

    print(STDERR "\n");
    print(STDERR "Modifying file: $filename\n");

    # open file for reading only

    if ($Test_mode) {
	$source_file=$filename;
    }

    # copy file to .bak file then read .bak file and write to original filename

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

    # Read the input file but do not make any changes yet, we are gathering info
    # grep returns zero on success

    # --------------- Does the file include cstring already? ------

    $file_has_cstring=0;
    $cmd="grep \"\^\#include \<cstring\>\" $source_file > /dev/null";
    &printverbosedebug("Running: $cmd\n");
    $is_ok=system($cmd);
    $found_cstring=WEXITSTATUS($?);
    if ($found_cstring == 0) {
	$file_has_cstring=1;
    }

    # --------------- Does the file include cstdlib already? ------

    $file_has_cstdlib=0;
    $cmd="grep \"\^\#include \<cstdlib\>\" $source_file > /dev/null";
    &printverbosedebug("Running: $cmd\n");
    $is_ok=system($cmd);
    $found_cstdlib=WEXITSTATUS($?);
    if ($found_cstdlib == 0) {
	$file_has_cstdlib=1;
    }

    # --------------- Does the file include regexp.h ? ------

    $file_has_regexp=0;
    $cmd="grep \"\^\#include \<regexp\" $source_file > /dev/null";
    &printverbosedebug("Running: $cmd\n");
    $is_ok=system($cmd);
    $found_regexp=WEXITSTATUS($?);
    if ($found_regexp == 0) {
	$file_has_regexp=1;
    }

    # --------------- Does the file define __USE_GNU ------

    $file_has_use_gnu=0;
    $cmd="grep \"\^\#define \_\_USE\_GNU\" $source_file > /dev/null";
    &printverbosedebug("Running: $cmd\n");
    $is_ok=system($cmd);
    $found_regexp=WEXITSTATUS($?);
    if ($found_use_gnu == 0) {
	$file_has_use_gnu=1;
    }

    # --------------- Does the file have exit() calls ------

    $file_has_exit=0;
    $cmd="grep \"exit\" $source_file > /dev/null";
    &printverbosedebug("Running: $cmd\n");
    $is_ok=system($cmd);
    $found_exit=WEXITSTATUS($?);
    if ($found_exit == 0) {
	$file_has_exit=1;
    }

    # --------------- Does the file have memcpy() calls ------

    $file_has_memcpy=0;
    $cmd="grep \"memcpy\" $source_file > /dev/null";
    &printverbosedebug("Running: $cmd\n");
    $is_ok=system($cmd);
    $found_memcpy=WEXITSTATUS($?);
    if ($found_memcpy == 0) {
	$file_has_memcpy=1;
    }

    # --------------- Does the file have strcmp() calls ------

    $file_has_strcmp=0;
    $cmd="grep \"strcmp\" $source_file > /dev/null";
    &printverbosedebug("Running: $cmd\n");
    $is_ok=system($cmd);
    $found_strcmp=WEXITSTATUS($?);
    if ($found_strcmp == 0) {
	$file_has_strcmp=1;
    }

    # --------------- Does the file have strcpy() calls ------

    $file_has_strcpy=0;
    $cmd="grep \"strcpy\" $source_file > /dev/null";
    &printverbosedebug("Running: $cmd\n");
    $is_ok=system($cmd);
    $found_strcpy=WEXITSTATUS($?);
    if ($found_strcpy == 0) {
	$file_has_strcpy=1;
    }

    # --------------- Does the file have strdup() calls ------

    $file_has_strdup=0;
    $cmd="grep \"strdup\" $source_file > /dev/null";
    &printverbosedebug("Running: $cmd\n");
    $is_ok=system($cmd);
    $found_strdup=WEXITSTATUS($?);
    if ($found_strdup == 0) {
	$file_has_strdup=1;
    }

    # --------------- Does the file have atoi() calls ------

    $file_has_atoi=0;
    $cmd="grep \"atoi\" $source_file> /dev/null";
    &printverbosedebug("Running: $cmd\n");
    $is_ok=system($cmd);
    $found_atoi=WEXITSTATUS($?);
    if ($found_atoi == 0) {
	$file_has_atoi=1;
    }

    # --------------- Does the file have free() calls ------

    $file_has_free=0;
    $cmd="grep \"free\" $source_file > /dev/null";
    &printverbosedebug("Running: $cmd\n");
    $is_ok=system($cmd);
    $found_free=WEXITSTATUS($?);
    if ($found_free == 0) {
	$file_has_free=1;
    }

    # --------------- Does the file have rand() calls ------

    $file_has_rand=0;
    $cmd="grep \"rand\" $source_file > /dev/null";
    &printverbosedebug("Running: $cmd\n");
    $is_ok=system($cmd);
    $found_rand=WEXITSTATUS($?);
    if ($found_rand == 0) {
	$file_has_rand=1;
    }

    # --------------- Summarize ------

    &printdebug("\tfile_has_cstring: $file_has_cstring, file_has_cstdlib: $file_has_cstdlib, file_has_regexp: $file_has_regexp, file_has_use_gnu: $file_has_use_gnu\n");
    &printdebug("\tfile_has_exit: $file_has_exit, file_has_memcpy: $file_has_memcpy\n");
    &printdebug("\tfile_has_strcmp: $file_has_strcmp, file_has_strcpy: $file_has_strcpy, file_has_strdup: $file_has_strdup\n");
    &printdebug("\tfile_has_atoi: $file_has_atoi, file_has_free: $file_has_free, file_has_rand: $file_has_rand\n");

    # --------------- Set flags ------

    $do_add_cstring=0;
    $do_add_cstdlib=0;

    if (($is_cc > 0) || ($is_hh > 0)) {
	if  (($file_has_cstring < 1) && 
	     (($file_has_strcmp > 0) || ($file_has_strdup > 0) || ($file_has_strcpy > 0) || ($file_has_memcpy > 0))) {
	    $do_add_cstring=1;
	}
	if (($file_has_cstdlib < 1) && 
	    (($file_has_atoi > 0) || ($file_has_free > 0) || ($file_has_rand > 0) || ($file_has_exit > 0))) {
	    $do_add_cstdlib=1;
	}
    }

    $do_add_use_gnu=0;
    if (($Do_regexp > 0) && ($file_has_regexp > 1) && ($file_has_use_gnu < 1)) {
	$do_add_use_gnu=1;
    }

    # --------------- Summarize ---------

    &printdebug("\tdo_add_cstring: $do_add_cstring, do_add_cstdlib: $do_add_cstdlib, do_add_use_gnu: $do_add_use_gnu\n");

    # --------------- loop through the lines in the input file ---------

    $found=0;
    $counter=0;
    $end_include=0;
    $inside_function=0;
    $inside_comments=0;
    $inside_include=0;
    $done_add_includes=0;

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

	# ---------------- Look for include directives ------

	if ($line =~ /^\#include/) {

	    $inside_include=1;

	    # Do substitutions for includes
	    # Skip includes that have subdirs, we are only replacing system
	    # level includes

	    if (($Do_incs) && ($line !~ /\//)) {

		if ($line =~ /\<string\.h/) {
		    print(STDERR "\tReplace string.h with cstring\n");
		    &dochange("string.h", cstring, $Test_mode, $Debug);
		    $do_add_cstring=0;
		}

		if ($line =~ /\<iostream\.h/) {
		    print(STDERR "\tReplace iostream.h with iostream\n");
		    &dochange("iostream.h", iostream, $Test_mode, $Debug);
		}

		if ($line =~ /\<algobase.h\.h/) {
		    print(STDERR "\tReplace algobase.h with algorithm\n");
		    &dochange("algobase.h", algorithm, $Test_mode, $Debug);
		}

		if ($line =~ /\<alloc\.h/) {
		    print(STDERR "\tReplace alloc.h with memory\n");
		    &dochange("alloc.h", memory, $Test_mode, $Debug);
		}

		if ($line =~ /\<bvector\.h/) {
		    print(STDERR "\tReplace bvector.h with vector\n");
		    &dochange("bvector.h", vector, $Test_mode, $Debug);
		}

		if ($line =~ /\<complex\.h/) {
		    print(STDERR "\tReplace complex.h with complex\n");
		    &dochange("complex.h", complex, $Test_mode, $Debug);
		}

		if ($line =~ /\<defalloc\.h/) {
		    print(STDERR "\tReplace defalloc.h with memory\n");
		    &dochange("defalloc.h", memory, $Test_mode, $Debug);
		}

		if ($line =~ /\<deque\.h/) {
		    print(STDERR "\tReplace deque.h with deque\n");
		    &dochange("deque.h", deque, $Test_mode, $Debug);
		}

		if ($line =~ /\<fstream\.h/) {
		    print(STDERR "\tReplace fstream.h with fstream\n");
		    &dochange("fstream.h", fstream, $Test_mode, $Debug);
		}

		if ($line =~ /\<iterator\.h/) {
		    print(STDERR "\tReplace iterator.h with iterator\n");
		    &dochange("iterator.h", iterator, $Test_mode, $Debug);
		}

		if ($line =~ /\<list\.h/) {
		    print(STDERR "\tReplace list.h with list\n");
		    &dochange("list.h", list, $Test_mode, $Debug);
		}

		if ($line =~ /\<function\.h/) {
		    print(STDERR "\tReplace function.h with functional\n");
		    &dochange("function.h", functional, $Test_mode, $Debug);
		}

		if ($line =~ /\<istream\.h/) {
		    print(STDERR "\tReplace istream.h with istream\n");
		    &dochange("istream.h", istream, $Test_mode, $Debug);
		}

		if ($line =~ /\<ostream\.h/) {
		    print(STDERR "\tReplace ostream.h with ostream\n");
		    &dochange("ostream.h", ostream, $Test_mode, $Debug);
		}

		if ($line =~ /\<stream\.h/) {
		    print(STDERR "\tReplace stream.h with iostream\n");
		    &dochange("stream.h", iostream, $Test_mode, $Debug);
		}

		if ($line =~ /\<vector\.h/) {
		    print(STDERR "\tReplace vector.h with vector\n");
		    &dochange("vector.h", vector, $Test_mode, $Debug);
		}

	    } #endif $Do_incs

	    # Do the regexp change

	    if (($Do_regexp) && ($do_add_use_gnu > 0) && ($line =~ /regexp\.h/)) {
		print(STDERR "\tWill add use_gnu to include of regexp.h\n");
		&doprint("\#if defined \(\_\_linux\)\n");
		&doprint("\#define \_\_USE\_GNU\n");
		&doprint("\#endif\n");
	    }

	} #endif $line has include

	if (($line !~ /^\#include/) && ($inside_include > 0)) {
	    $end_include = 1;
	}
	 	
	# Add additional includes after the existing includes

	if (($end_include > 0) && ($done_add_includes < 1)) {

	    if ($Do_exit_memcpy > 0) {

		if ($do_add_cstring > 0) {
		    print(STDERR "\tAdding include cstring\n");
		    &doprint("\#include \<cstring\>\n");
		}

		if ($do_add_cstdlib > 0) {
		    print(STDERR "\tAdding include cstdlib\n");
		    &doprint("\#include \<cstdlib\>\n");
		}
	    }

	    # Done adding includes

	    $done_add_includes=1;

	} #endif inside_include

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
