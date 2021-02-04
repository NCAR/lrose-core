#!/usr/bin/perl
#
# monitor_disk.pl
# 
# Function: Perl script to monitor disk usage. This registers with
#           the procmap so it can be used to show disk usage under
#           SysView.
#
# Usage: run "monitor_disk.pl -h" for options
#
# Input: output of df
#
# Output:
#
# Dependencies: 
#
# Known bugs:
#
# Author: Deirdre Garvey NCAR/RAP 17-MAY-2001
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

$Disk_warn_threshold_pct_used=90;
$Disk_severe_threshold_pct_used=95;
$Timeout=10;

#
# Set command line defaults
#

$Debug=0;                           # Flag to print debug messages to STDERR
$Debug_level=0;
$Sleep_secs=60;                     # How long to sleep between checks (secs)
$Do_test=0;                         # Flag for test mode
$Disk_reg_interval=60;              # Procmap registration interval (secs)
                                    #    on behalf of the disk usage
$Disk_proc_name="disk";             # Process name to send to procmap
                                    #    on behalf of the disk usage
$Disk_proc_instance="disk";         # Process instance to sent to procmap
                                    #    on behalf of the disk usage
$Do_register=1;                     # Flag to register this process with the procmap
$Instance="disk";                   # Instance for this process for the procmap
$Tmp_dir="/tmp";                    # Temporary directory to use
$Df_cmd="df -l -k";                 # df command to execute
$Disk_list="/, /home, /usr";       # List of disks to check
$Use_host="localhost";              # Host to query

#
# Save the usage to print to the user if there is a problem
#
$usage =                                                 
    "\n" .
    "Usage: $prog [-dhnq] <-a host> <-b instance> <-c cmd> <-i instance> <-l list> <-p name> <-r secs> <-s secs> <-t dir> <-v level>\n" .
    "Purpose: To check whether the expected disks are less than (warn) $Disk_warn_threshold_pct_used\%\n" .
    "         or (severe) $Disk_severe_threshold_pct_used\% full. Uses --cmd to get the disk status.\n" .
    "   -a --anotherhost <host>  : Host to monitor proclist on. If not the localhost,\n" .
    "                              you must have password-less ssh access to this host for the current user.\n" .
    "                              Default is: $Use_host\n" .
    "   -b --behalfinst <instance>: Ignored if --noregister.\n" .
    "                              Instance to pass to procmap on behalf of the disk\n" .
    "                              Default is: $Disk_proc_instance\n" .
    "   -c --cmd <cmd>           : df command to use to search for processes\n" .
    "                              Default is: $Df_cmd\n" .
    "   -d --debug               : Print debugging messages\n" .
    "   -h --help                : Print this usage message\n" .
    "   -i --instance <instance> : Instance to pass to procmap for this process.\n". 
    "                              Default is: $Instance\n" .
    "   -l --listdisk <list>     : List of disks to check. This should be a comma-separated\n" .
    "                              quoted list.\n" .
    "                              Default is: $Disk_list\n" .
    "   -n --noregister          : Do not register with the procmap for this process and for\n" .
    "                              the disk process\n" .
    "   -p --procname <name>     : Ignored if --noregister\n" .
    "                              Process name to pass to procmap on behalf of the disk\n" .
    "                              Default is: $Disk_proc_name\n" .
    "   -q --quit                : Flag to run once and exit\n" .
    "   -r --reg <secs>          : Ignored if --noregister\n" .
    "                              Registration interval (secs) to pass to procmap on behalf of the disk\n" .
    "                              Default is: $Disk_reg_interval\n" .
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
		      'listdisk=s',
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
  print(STDERR "\tdf command: $opt_cmd\n");
  print(STDERR "\tinstance: $opt_instance\n");
  print(STDERR "\tdisk list: $opt_listdisk\n");
  print(STDERR "\tnoregister: $opt_noregister\n");
  print(STDERR "\tdisk proc name: $opt_procname\n");
  print(STDERR "\tquit: $opt_quit\n");
  print(STDERR "\tdisk reg secs: $opt_reg\n");
  print(STDERR "\tsleep secs: $opt_sleep\n");
  print(STDERR "\ttmp dir: $opt_tmpdir\n");
  print(STDERR "\tverbosedebuglevel: $opt_verbose\n");
}

if ($opt_anotherhost) {
    $Use_host=$opt_anotherhost;
}

if ($opt_behalfinst) {
    $Disk_proc_instance=$opt_behalfinst;
}

if ($opt_cmd) {
    $Df_cmd=$opt_cmd;
}

if ($opt_instance) {
    $Instance=$opt_instance;
}

if ($opt_listdisk) {
    $Disk_list=$opt_listdisk;
}

if ($opt_noregister) {
    $Do_register=0;
}

if ($opt_procname) {
    $Disk_proc_name=$opt_procname;
} 

if ($opt_quit) {
    $Do_test=1;
}

if ($opt_regs) {
    $Disk_reg_interval=$opt_reg;
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

# Parse the list of disks to check

@disk_list=split(',', $Disk_list);

# Register with the procmap

if ($Do_register) {

    # Set the registration interval to the sleep

    $Reg_interval=$Sleep_secs;

  Toolsa::PMU_auto_init($prog, $Instance, $Reg_interval);
  Toolsa::PMU_auto_register("Initializing");

    # Register on behalf of the procmap

  Toolsa::PMU_auto_init($Disk_proc_name, $Disk_proc_instance, $Disk_reg_interval);
  Toolsa::PMU_auto_register("$prog initialize on behalf of disk");
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

    # Loop through the list of disks to see how full they are

    $found_problems=0;
    $warn_threshold=0;
    $severe_threshold=0;
    $status_msg="";

    foreach $disk (@disk_list) {

	# Remove blanks from the disk name

	if ($disk =~ /^\s/) {
	    ($blanks, $disk) = split('\s+', $disk); 
	}
	if ($disk =~ /$\s/) {
	    ($disk, $blanks) = split('\s+', $disk); 
	}

	if ($Debug) {
	    print(STDERR "Checking if ..$disk.. is under threshold...\n");
	}

	# Get a timestring so can add it to tmp file so do not overwrite

	$now=time;
	$tmpfile="${Tmp_dir}/${prog}.${now}";

	# Run the df check and redirect it to an output file so we can parse it

	$cmd="$Df_cmd | grep $disk \> $tmpfile";

	if ($Use_host ne "localhost") {
	    $cmd="ssh $Use_host ${cmd}";
	}

	if ($Debug_level > 1) {
	    print(STDERR "\tRunning $cmd\n");
	}

	$result=Toolsa::safe_system($cmd, $Timeout);

	# Parse the results of the return from df

	$is_ok=open(TMPFILE, "< $tmpfile");
	if (!$is_ok) {
	    print(STDERR "$Today: ERROR: Cannot open tmpfile $tmpfile\n");
	    &DoExit;
	}

	# Did safe_system kill us?

	if (!-s $tmpfile) {
	    print(STDERR "$Today: ERROR: search for $disk killed by safe_system, $Use_host may not be reachable\n");
	    &DoExit;
	}

	# Search for the disk in the tmpfile

	$found_disk=0;

	while ($line = <TMPFILE>) {

	    # The disk is in the device table?

	    if ($line =~ /$disk/) {
		
		chop($line);

		# Parse the df line to get the % used
		# Expect line to be of form: 
		#     Filesystem 1k-blocks Used Available Use% Mounted-on

		($fs, $blocks, $blocks_used, $blocks_avail, $pct_used, $mounted_on)=split(' ', $line);

		if ($Debug_level > 1) {
		    print(STDERR "Return from df: fs: $fs, blocks: $blocks, blocks_used: $blocks_used, blocks_avail: $blocks_avail, pct_used: $pct_used, mounted_on: $mounted_on\n");
		}

		# Need to find a true match for the disk name

		if (($disk ne $mounted_on) && ($disk ne $fs)) {
		    next;
		}

		if ($Debug_level >= 1) {
		    print(STDERR "Found $disk in df list\n");
		}
		$found_disk=1;

		# Remove the percent sign

		if ($pct_used =~ /\%/) {
		    ($pct_used, $junk) = split(/\%/, $pct_used);
		}

		if ($Debug_level >=1) {
		    print(STDERR "\tpct used: $pct_used\n");
		}

		# Is the percent used above any of the thresholds?

		if ($pct_used > $Disk_warn_threshold_pct_used) {
		    $found_problems=1;
		    $warn_threshold=1;
		    $status_msg = "${status_msg} ${disk} $pct_used\% full,";
		}
		if ($pct_used > $Disk_severe_threshold_pct_used) {
		    $warn_severe=1;
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

	if (!$found_disk) {
	    print(STDERR "$Today: WARNING: $disk not in disk list\n");
	    $status_msg="${status_msg} ${disk} missing,";
	    $found_problems=1;
	}
    }

    # Register with the procmap
    # Do not register on behalf of the disk if found problems,
    # exit if found a warn_severe state

    if ($Do_register) {

	# Register for this process

	$status_str="Sleeping $Sleep_secs seconds";
	if ($found_problems) {
	    $status_str=$status_msg;
	}
	$cmd="procmap_register -name $prog -instance $Instance -status_str \"$status_str\" -reg_int $Reg_interval -start $start_utime";
	safe_system($cmd, $Timeout);

	  
	# Register on behalf of the disk process

	if (!$found_problems) {
	    $status_str="Disks are okay";
	    $cmd="procmap_register -name $Disk_proc_name -instance $Disk_proc_instance -status_str \"$status_str\" -reg_int $Disk_reg_interval -start $start_utime";
	    safe_system($cmd, $Timeout);
	}

	if ($warn_severe) {
	    $cmd="procmap_unregister -name $Disk_proc_name -instance $Disk_proc_instance";
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

  local($subname, $cmd);

  # Set defaults

  $subname="DoExit";

  if ($Debug) {
    print(STDERR "in $subname...\n");
  }

  # Unregister from the procmap

  if ($Do_register) {

      $cmd="procmap_unregister -name $prog -instance $Instance";
      safe_system($cmd, $Timeout);

      # Unregister on behalf of the disk

      $cmd="procmap_unregister -name $Disk_proc_name -instance $Disk_proc_instance";
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
