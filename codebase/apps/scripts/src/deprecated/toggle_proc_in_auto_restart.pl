#! /usr/bin/perl
#
# toggle_proc_in_auto_restart
#
# Function:
#	Perl script to either remove a process from auto_restart
#       proc_list file and then kill the process OR to uncomment 
#       a process from an auto_restart proc_list file.
#
# Usage: Run:
#       % toggle_proc_in_auto_restart  -h
#
# Input:
#       proc_list file
#
# Output:
#       Copies the input source file into <file>.bak and puts the 
#       filtered file into <file>.
#
# Dependencies:
#	Perl must be available in the location at the top of this file.
#       snuff and snuff_inst must be available in the path.
# 	
# Author: Deirdre Garvey - NCAR/RAP/SDG		25-JUN-2004
#
##
#---------------------------------------------------------------------------------

# Perl modules

use Getopt::Long;
use Env;
use Cwd;
Env::import();

sub badArg;
sub translateDataDir;
sub verifyDataDir;

# Set return code defaults

$Exit_success=0;
$Exit_failure=-1;

# Get program name

($Prog = $0) =~ s%.*/%%;

# Command line defaults

$Print_debug=0;
$Test_mode=0;
$Do_comment=0;
$Do_kill=0;
$Do_uncomment=0;
$Do_instance=0;

# Save the usage to print to the user if there is a problem
$usage =                                                 
    "\nUsage: $Prog -k|-u -c <proc_list> -p <process> [-i <instance>] [-dht]\n" .
    "Purpose: To kill a process/instance and remove it from the auto_restart\n" .
    "         proc_list file OR to uncomment a process/instance from a\n" .
    "         proc_list file. Only finds a match with a process name at the\n" .
    "         start of a line or preceded by a single \#.\n" .
    " -c --config <file>:    (Required) auto_restart proc_list file to modify\n" .
    " -d --debug:            Print debug messages\n" .
    " -h --help:             Print this usage statement\n" .
    " -i --instance <name>:  Process instance. If not specified, will kill or\n" .
    "                           uncomment all processes with the process name\n" .
    "                           regardless of instance\n" .
    " -k --kill:             Kill the process/instance and comment out from\n" .
    "                           the proc_list file\n" .
    " -p --process <name>:   Process name\n" .
    " -t --test:             Test mode, only report on what would have been done\n" .
    " -u --uncomment:        Uncomment the process/instance from the proc_list file\n";

# Get the arguments from the command line
$result = &GetOptions('debug',
		      'help',
		      'config=s',
		      'instance=s',
		      'kill',
		      'process=s',
		      'test',
		      'uncomment',
                       '<>', \&badArg );

if ( $result == 0 || $opt_help ) {
   print $usage;
   exit($Exit_success);
}

if ((!$opt_kill) && (!$opt_uncomment)) {
    print(STDERR "ERROR: You must specify either --kill or --uncomment\n");
    exit($Exit_failure);
}

if ($opt_debug) {
   $Print_debug = 1;
}

if ($opt_config) {
    $Proc_list_file=$opt_config;
} else {
    print(STDERR "ERROR: You must specify a proc_list file\n");
    exit($Exit_failure);
}

if ($opt_instance) {
    $Instance=$opt_instance;
    $Do_instance=1;
}

if ($opt_kill) {
    $Do_uncomment=0;
    $Do_comment=1;
    $Do_kill=1;
}

if ($opt_process) {
    $Process=$opt_process;
} else {
    print(STDERR "ERROR: You must specify a process name\n");
    exit($Exit_failure);
}

if ($opt_test) {
    $Test_mode = 1;
}

if ($opt_uncomment) {
    $Do_uncomment=1;
    $Do_comment=0;
    $Do_kill=0;
}

# Print out debug info

if ($Print_debug == 1) {
    print(STDERR "Debug: $Prog: Command line options...\n");
    print(STDERR "\tPrint_debug: $Print_debug\n");
    print(STDERR "\tTest_mode: $Test_mode\n");
    print(STDERR "\tDo_kill: $Do_kill\n");
    print(STDERR "\tDo_uncomment: $Do_uncomment\n");
    print(STDERR "\tDo_instance: $Do_instance\n");
    print(STDERR "\tProcess: $Process\n");
    print(STDERR "\tInstance: $Instance\n");
    print(STDERR "\tProc_list (config) file: $Proc_list_file\n");
}

# ------------------------------ Error checking -----------------------
#
# Does the proc_list file exist?

if (!-e $Proc_list_file) {
    print(STDERR "ERROR: Proc_list file does not exist: $Proc_list_file\n");
    exit($Exit_failure);
}

# ------------------------------ Main --------------------------------
#
# Print a banner
$Date=`date -u`;
print(STDERR "Running $Prog at: $Date\n");

# Open the proc_list file
# Test mode, open file for reading only

if ($Test_mode) {
    if (!open(SRCFILE, $Proc_list_file)) {
	print(STDERR "Cannot open proc_list file: $Proc_list_file\n");
	exit($Exit_failure);
    }
}

# Modify mode, copy file to .bak file then read .bak file and write to file

else {

    $bakfile = $Proc_list_file . ".bak";
    system ('/bin/mv', $Proc_list_file, $bakfile);

     if (!open(SRCFILE, $bakfile)) {
         print(STDERR "Cannot open file $bakfile\n");
	 exit($Exit_failure);
     }

    if (!open(OUTFILE, ">$Proc_list_file")) {
         print(STDERR "Cannot open file $Proc_list_file\n");
	 exit($Exit_failure);
     }
} #end else

# --------------- loop through the lines in the file ---------

$found_match=0;
while ($line = <SRCFILE>) {

    # --------------------- BEGIN changes to file ------------------

    # Comment out the process/instance. Need to be careful to only
    # find the process at the beginning of the line because 
    # of possible use of the process name in comments
      
    $found_process=0;
    if (($line =~ /^$Process/) || ($line =~ /^\#$Process/)) {
	$found_process=1;
	if ($Do_instance) {
	    if ($line !~ /$Instance/) {
		$found_process=0;
	    }
	}
	if ($found_process) {
	    if ($Print_debug) {
		print(STDERR "Debug: $Prog: Found process: $Process");
		if ($Do_instance) {
		    print(STDERR ", instance: $Instance");
		}
		print(STDERR "\n");
	    }

	    if ($Do_comment == 1) {
		&dochange('^', '#', $Test_mode, $Print_debug);
	    } elsif ($Do_uncomment) {
		&dochange('^\#', '', $Test_mode, $Print_debug);
	    }
	}

	# Set the found flag for this file but do not reset
	# if already set to true once.

	if (!$found_match && $found_process) {
	    $found_match=$found_process;
	}

    } #endif $line

    # --------------------- END changes to file ------------------

    # print line to output file if NOT in test mode
      
    if (!$Test_mode) {
	print(OUTFILE $line);
    }
    
} # end while

close(SRCFILE);
close(OUTFILE);

# Error checking

if (!$found_match) {
    print(STDERR "ERROR: $Prog: Did not find process: $Process");
    if ($Do_instance) {
	print(STDERR ", instance: $Instance");
    }
    print(STDERR ", in proc_list file: $Proc_list_file\n");
    exit($Exit_failure);
}

# Kill the process, if specified

if ($Do_kill) {
    if ($Do_instance) {
	$cmd="snuff_inst $Process $Instance";
    } else {
	$cmd="snuff $Process";
    }
    if ($Test_mode) {
	print(STDERR "Test: $Prog: Would run: $cmd\n");
    } else {
	system($cmd);
    }
}

# Done
exit($Exit_success);

#
#--------------------------------------------------------------------------------
#

sub dochange {
    local($searchstr, $replacestr, $test_flag, $debug_flag) = @_;

    local($subname);
    $subname="dochange";
    
    if ($debug_flag == 1) {
	print STDERR "Debug: $subname: search for: $searchstr and replace with: $replacestr\n";
    }

    # search for string and change it if we are not in test mode

    if ($line =~ /$searchstr/) {

        if ($test_flag == 1) {
            print(STDERR "\tFound: $searchstr\n");
	}
        else {
	    if ($debug_flag == 1) {
		print(STDERR "\tChanging: $searchstr \tto: $replacestr\n");
	    }

  	    $line =~ s/$searchstr/$replacestr/;
	}
    } #endif $line
}

