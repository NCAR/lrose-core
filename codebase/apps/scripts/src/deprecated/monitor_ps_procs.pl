#!/usr/bin/perl
#
# monitor_ps_procs.pl
# 
# Function: Perl script to monitor processes under ps. This script registers
#           itself and on behalf of another process so it can be used
#           to show the status of the monitored process under SysView.
#
# Usage: run "monitor_ps_procs.pl -h" for options
#
# Input: output of ps
#
# Output:
#
# Dependencies: 
#
# Known bugs:
#
# Author: Deirdre Garvey NCAR/RAP 08-MAY-2001
#
#===================================================================
#
# --------------------------- Externals -------------------------
#
# For handling the long format of input options
#

use Getopt::Long;
use Env;
use Cwd;
Env::import();

sub badArg;
sub translateDataDir;
sub verifyDataDir;

# 
# External modules
#

use Time::Local;                   # make 'date' available

# External NCAR/RAP libraries
use Env qw(ADDSHOME);                    #...Get ADDS Home Dir
use lib "$ADDSHOME/perl_lib";

# External NCAR/RAP libraries
# Rel to PROJ_DIR
use Env qw(HOME);
use lib "$HOME/git/lrose-core/codebase/libs/perl5/src/";
use Env qw(PROJ_DIR);
use lib "$PROJ_DIR/lrose/lib/perl5/";

use Toolsa;

#
# --------------------------- Defaults and globals --------------
#
# Get the program basename.
#

($prog = $0) =~ s|.*/||;

#
# Set program defaults
#
$Timeout=10;

#
# Set command line defaults
#

$Debug=0;                           # Flag to print debug messages to STDERR
$Debug_level=0;
$Sleep_secs=60;                     # How long to sleep between checks (secs)
$Do_test=0;                         # Flag for test mode
$PROC_reg_interval=60;              # Procmap registration interval (secs)
                                    #    on behalf of the proclist
$PROC_proc_name="proc";             # Process name to send to procmap
                                    #    on behalf of the proclist
$PROC_proc_instance="proc";         # Process instance to sent to procmap
                                    #    on behalf of the proclist
$Do_register=1;                     # Flag to register this process with the procmap
$Instance="proc";                   # Instance for this process for the procmap
$Tmp_dir="/tmp";                    # Temporary directory to use
$Ps_cmd="ps axww";                  # ps command to execute
$PROC_process_list="rpc.ldmd, pqact"; 
                                    # List of PROC processes to check
$Use_host="localhost";              # Host to query

#
# Save the usage to print to the user if there is a problem
#
$usage =                                                 
    "\n" .
    "Usage: $prog [-dhnq] <-a host> <-b instance> <-c cmd> <-i instance> <-l list> <-p name> <-r secs> <-s secs> <-t dir> <-v level>\n" .
    "Purpose: To check whether the expected listed processes are running or not\n" .
    "         Uses --cmd to search for the list of processes.\n" .
    "   -a --anotherhost <host>  : Host to monitor proclist on. If not the localhost,\n" .
    "                              you must have password-less ssh access to this host for the current user.\n" .
    "                              Default is: $Use_host\n" .
    "   -b --behalfinst <instance>: Ignored if --noregister.\n" .
    "                              Instance to pass to procmap on behalf of the proclist\n" .
    "                              Default is: $PROC_proc_instance\n" .
    "   -c --cmd <cmd>           : ps command to use to search for processes\n" .
    "                              Default is: $Ps_cmd\n" .
    "   -d --debug               : Print debugging messages\n" .
    "   -h --help                : Print this usage message\n" .
    "   -i --instance <instance> : Instance to pass to procmap for this process.\n". 
    "                              Default is: $Instance\n" .
    "   -l --listprocs <list>    : List of processes to check (proclist). This should be a comma-separated\n" .
    "                              quoted list.\n" .
    "                              Default is: $PROC_process_list\n" .
    "   -n --noregister          : Do not register with the procmap for this process and for\n" .
    "                              the proclist process\n" .
    "   -p --procname <name>     : Ignored if --noregister\n" .
    "                              Process name to pass to procmap on behalf of the proclist\n" .
    "                              Default is: $PROC_proc_name\n" .
    "   -q --quit                : Flag to run once and exit\n" .
    "   -r --reg <secs>          : Ignored if --noregister\n" .
    "                              Registration interval (secs) to pass to procmap on behalf of the proclist\n" .
    "                              Default is: $PROC_reg_interval\n" .
    "   -s --sleep <secs>        : Seconds to sleep between checks\n" .
    "   -t --tmpdir <dir>        : Directory to use for temporary files\n" .
    "                              Default is: $Tmp_dir\n" .
    "   -v --verbose <num>       : A debug verbose level number (1..2)\n" ;

# ----------------------------- Parse command line --------------------------
#
# Get the arguments from the command line
#
$result = &GetOptions('debug',
		      'help',
		      'anotherhost=s',
		      'behalfinst=s',
		      'cmd=s',
		      'instance=s',
		      'listprocs=s',
		      'noregister',
		      'procname=s',
		      'quit',
		      'reg=i',
		      'sleep=i',
		      'tmpdir=s',
		      'verbose=i',
                       '<>', \&badArg );

if ( $result == 0 || $opt_help ) {
   print $usage;
   exit 0;
}

if (($opt_debug) || ($opt_verbose)) {
  $Debug=1;
  $Debug_level=1;
  print(STDERR "Input options specified...\n");
  print(STDERR "\tanotherhost: $opt_anotherhost\n");
  print(STDERR "\tbehalf instance: $opt_behalfinst\n");
  print(STDERR "\tps command: $opt_cmd\n");
  print(STDERR "\tinstance: $opt_instance\n");
  print(STDERR "\tproclist: $opt_listprocs\n");
  print(STDERR "\tnoregister: $opt_noregister\n");
  print(STDERR "\tbehalf proc name: $opt_procname\n");
  print(STDERR "\tquit: $opt_quit\n");
  print(STDERR "\tproc reg secs: $opt_reg\n");
  print(STDERR "\tsleep secs: $opt_sleep\n");
  print(STDERR "\ttmp dir: $opt_tmpdir\n");
  print(STDERR "\tverbosedebuglevel: $opt_verbose\n");
}

if ($opt_anotherhost) {
    $Use_host=$opt_anotherhost;
}

if ($opt_behalfinst) {
    $PROC_proc_instance=$opt_behalfinst;
}

if ($opt_cmd) {
    $Ps_cmd=$opt_cmd;
}

if ($opt_instance) {
    $Instance=$opt_instance;
}

if ($opt_listprocs) {
    $PROC_process_list=$opt_listprocs;
}

if ($opt_noregister) {
    $Do_register=0;
}

if ($opt_procname) {
    $PROC_proc_name=$opt_procname;
} 

if ($opt_quit) {
    $Do_test=1;
}

if ($opt_regs) {
    $PROC_reg_interval=$opt_reg;
}

if ($opt_sleep) {
    $Sleep_secs=$opt_sleep;
}

if ($opt_tmpdir) {
    $Tmp_dir=$opt_tmpdir;
}

if ($opt_verbose) {
  if ($opt_verbose < 0) {
    print(STDERR "ERROR: $prog: Invalid debug level. Ignoring it\n");
    $Debug=$Debug_level=1;
  } else {
    $Debug_level=$opt_verbose;
    $Debug=1;
  }
}

# --------------------------- Initialization -----------------------
#
# Does the temporary directory exist? Exit if it does not

if (!-d $Tmp_dir) {
    print(STDERR "The temporary directory $Tmp_dir does not exist\n");
    exit -1;
}

# Parse the list of processes to check

@proc_list=split(',', $PROC_process_list);

# Register with the procmap

if ($Do_register) {

    # Set the registration interval to the sleep

    $Reg_interval=$Sleep_secs;

  Toolsa::PMU_auto_init($prog, $Instance, $Reg_interval);
  Toolsa::PMU_auto_register("Initializing");

    # Register on behalf of the procmap

  Toolsa::PMU_auto_init($PROC_proc_name, $PROC_proc_instance, $PROC_reg_interval);
  Toolsa::PMU_auto_register("$prog initialize on behalf of PROC");
}	    

# Get the start time

$start_utime=time;

# Set up signal handlers now, just before the main loop.

$SIG{'INT'} = $SIG{'QUIT'} = 'DoExit';

# ----------------------------- Main -------------------------------

# Initialize the loop

$doLoop=1;

LOOP: while ($doLoop) {

    # Get the current date/time

    $Today=`date`;
    chop($Today);

    # Do not go through the main loop more than once if in test mode
	
    if ($Do_test) {
	$doLoop = 0;
    }

    # Loop through the list of PROC processes to see if they are running or not

    $found_problems=0;

    foreach $proc (@proc_list) {
	if ($Debug) {
	    print(STDERR "Checking if $proc is running...\n");
	}

	# Get a timestring so can add it to tmp file so do not overwrite

	$now=time;
	$tmpfile="${Tmp_dir}/${prog}.${now}";
		
	# Run the ps check and redirect it to an output file so we can parse it

	$cmd="$Ps_cmd | grep $proc \> $tmpfile";

	if ($Use_host ne "localhost") {
	    $cmd="ssh $Use_host ${cmd}";
	}

	if ($Debug_level > 1) {
	    print(STDERR "\tRunning $cmd\n");
	}
	$result=Toolsa::safe_system($cmd, $Timeout);
       
	# Did safe_system kill the search? The $tmpfile will not exist in this case.

	$successful_check=1;

	if (!-s $tmpfile) {
	    print(STDERR "$Today: ERROR: search for $proc killed by safe_system, $Use_host may not be reachable\n");
	    $successful_check=0;
	} 
	  
	# Parse the results of the return from ps

	$is_ok=open(TMPFILE, "< $tmpfile");
	if (!$is_ok) {
	    print(STDERR "$Today: ERROR: Cannot open tmpfile $tmpfile\n");
	    $successful_check=0;
	}

	# Search the tmpfile

	if ($successful_check) {

	    $found_process=0;

	    while ($line = <TMPFILE>) {

		# Skip lines that are not really the process

		if (($line =~ /grep/) || ($line =~ /$Ps_cmd/) || ($found_process)) {
		    next;
		}

		# The process is in the process table

		if ($line =~ /$proc/) {
		    if ($Debug_level >= 1) {
			print(STDERR "Found $proc in process list\n");
		    }
		    $found_process=1;
		}
	    } 
	}
	close(TMPFILE);
	
	# Remove the tmpfile

	$nfiles=unlink $tmpfile;
	if ($nfiles <=0) {
	    print(STDERR "$Today: WARNING: did not delete $tmpfile!\n");
	}

	# Print a warning and set the flag

	if (!$found_process) {
	    print(STDERR "$Today: WARNING: $proc not in process list\n");
	    $found_problems=1;
	}
    } #endforeach

    # Register with the procmap

    if ($Do_register) {
	
	# Register THIS script with the procmap

	$status_str="Sleeping $Sleep_secs seconds";
	if ($found_problems) {
	    $status_str="Found problem with $PROC_process_list";
	}
	$cmd="procmap_register -name $prog -instance $Instance -status_str \"$status_str\" -reg_int $Reg_interval -start $start_utime";
	safe_system($cmd, $Timeout);

	if (!$found_problems) {

	    # Register the process we are running on behalf of with the procmap

	    $status_str="$PROC_proc_name is alive";
	    $cmd="procmap_register -name $PROC_proc_name -instance $PROC_proc_instance -status_str \"$status_str\" -reg_int $PROC_reg_interval -start $start_utime";
	    safe_system($cmd, $Timeout);
	}
    }

    # Sleep

    if (!$Do_test) {
	sleep($Sleep_secs);
    }
}

&DoExit;


# =============================== SUBROUTINES ===========================
#
# Subroutine DoExit
#
# Usage: DoExit
#
# Function: Cleanup and exit
#
# Input:    none. All is handled by globals
#
# Output:   none
#
# Overview:
#

sub DoExit
{
 
  # Local variables

  local($subname, $cmd, $nfiles);

  # Set defaults

  $subname="DoExit";

  if ($Debug) {
    print(STDERR "in $subname...\n");
  }

  # Unregister from the procmap

  if ($Do_register) {

      $cmd="procmap_unregister -name $prog -instance $Instance";
      safe_system($cmd, $Timeout);

      # Unregister on behalf of the PROC

      $cmd="procmap_unregister -name $PROC_proc_name -instance $PROC_proc_instance";
      safe_system($cmd, $Timeout);
  }

  # Remove the tmpfile if it exists

  if (-e $tmpfile) {
      $nfiles=unlink $tmpfile;
      if ($nfiles <=0) {
	  print(STDERR "$Today: WARNING: did not delete $tmpfile!\n");
      }
  }

exit 0;

}


#========================================= EOF =====================================
