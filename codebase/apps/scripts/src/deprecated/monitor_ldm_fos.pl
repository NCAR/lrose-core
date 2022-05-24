#!/usr/bin/perl
#
# monitor_ldm_fos.pl
# 
# Function: Perl script to monitor LDM FOS data and register with
#           the DataMapper
#
# Usage: run "monitor_ldm_fos.pl -h" for options
#
# Input: LDM data directory
#
# Output:
#
# Dependencies: Requires the LdataInformer application to be in
#               the path. This is in CVS in: apps/didss/src/LdataInformer.
#
# Known bugs:
#
# Author: Deirdre Garvey NCAR/RAP 09-MAY-2001
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
$Timeout=900;

#
# Set command line defaults
#

$Debug=0;                           # Flag to print debug messages to STDERR
$Debug_level=0;                     # Debug flag for verbose mode
$Sleep_secs=60;                     # How long to sleep between checks (secs)
$Do_test=0;                         # Flag for test mode
$Do_register=1;                     # Flag to register with the DataMapper
$LDM_data_dir="/data/ldm/data";     # LDM data directory
$LDM_data_file_ext="METAR";         # LDM data type (extension) to watch
$Instance="ldm";                    # Instance to pass to procmap
$Min_secs=30;                       # Minimum number of seconds before update DataMapper
$LDM_data_type="fos";               # Data type to register with the DataMapper

#
# Save the usage to print to the user if there is a problem
#
$usage =                                                 
    "\n" .
    "Usage: $prog [-dhnq] <-e ext> <-i instance> <-l dir> <-m secs> <-s secs> <-t type> <-v level>\n" .
    "Purpose: To monitor a dataset in the FOS data directory to ensure that it is updating.\n" .
    "         Updates the DataMapper.\n" .
    "   -d --debug               : Print debugging messages\n" .
    "   -e --ext <extension>     : Type of file to monitor.\n" .
    "                              Default is: METAR\n" .
    "   -h --help                : Print this usage message\n" .
    "   -i --instance <instance> : Instance to pass to procmap\n" .
    "                              Default is: $Instance\n" .
    "   -l --ldmdir <dir>        : Input LDM directory to monitor\n" .
    "                              Default is: $LDM_data_dir\n" .
    "   -m --minsecs <secs>      : Minimum number of seconds since last update to\n" .
    "                              data file before notify the DataMapper. This is to\n" .
    "                              prevent flooding the DataMapper\n" .
    "                              Default is: $Min_secs\n" .
    "   -n --noregister          : Do not register this process with the procmap\n" .
    "   -q --quit                : Flag to run once and exit\n" .
    "   -s --sleep <secs>        : Seconds to sleep between checks\n" .
    "   -t --type <type>         : Data type to register with the DataMapper\n" .
    "                              Default is: $LDM_data_type\n" .
    "   -v --verbose <num>       : A debug verbose level number (1..2)\n" ;

# ----------------------------- Parse command line --------------------------
#
# Get the arguments from the command line
#
$result = &GetOptions('debug',
		      'help',
		      'ext=s',
		      'instance=s',
		      'ldmdir=s',
		      'minsecs=i',
		      'noregister',
		      'quit',
		      'sleep=i',
		      'type=s',
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
  print(STDERR "\text: $opt_ext\n");
  print(STDERR "\tinstance: $opt_instance\n");
  print(STDERR "\tldm dir: $opt_ldmdir\n");
  print(STDERR "\tmin secs: $opt_minsecs\n");
  print(STDERR "\tnoregister: $opt_noregister\n");
  print(STDERR "\tquit: $opt_quit\n");
  print(STDERR "\tsleep secs: $opt_sleep\n");
  print(STDERR "\tdata type: $opt_type\n");
  print(STDERR "\tverbosedebuglevel: $opt_verbose\n");
}


if ($opt_ext) {
    $LDM_data_file_ext=$opt_ext;
}

if ($opt_instance) {
    $Instance=$opt_instance;
}

if ($opt_ldmdir) {
    $LDM_data_dir=$opt_ldmdir;
}

if ($opt_minsecs) {
    $Min_secs=$opt_minsecs;
}

if ($opt_noregister) {
    $Do_register=0;
}

if ($opt_quit) {
    $Do_test=1;
}

if ($opt_sleep) {
    $Sleep_secs=$opt_sleep;
}

if ($opt_type) {
    $LDM_data_type=$opt_type;
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
# Register with the procmap

if ($Do_register) {

    # Set the registration interval to the sleep

    $Reg_interval=$Sleep_secs;

  Toolsa::PMU_auto_init($prog, $Instance, $Reg_interval);
  Toolsa::PMU_auto_register("Initializing");
}	    

# Get the start time

$start_utime=time;

# Set up signal handlers now, just before the main loop.

$SIG{'INT'} = $SIG{'QUIT'} = 'DoExit';

# ----------------------------- Main -------------------------------

# Initialize the loop

$doLoop=1;
$last_update_utime=0;

LOOP: while ($doLoop) {

    # Do not go through the main loop more than once if in test mode
	
    if ($Do_test) {
	$doLoop = 0;
    }

    # Get the current date/time so we can get the correct LDM data subdir 
    # which is the UTC date YYYYMMDD. Also need the current UTC hour.

    $now=time;
    ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=gmtime($now);
    $date_string=sprintf("%4d%02d%02d", $year+1900, $mon+1, $mday);
    $hour_string=sprintf("%02d", $hour);
    $datamap_string=$date_string . sprintf("%02d%02d%02d", $hour, $min, $sec);
    $current_time_string=$date_string . sprintf(" %02d:%02d:%02d UTC", $hour, $min, $sec);

    # Build the LDM filename to check

    $file_to_check="${LDM_data_dir}/${date_string}/${date_string}${hour_string}.${LDM_data_file_ext}";
    if ($Debug) {
	print(STDERR "$prog: Checking $file_to_check\n");
    }

    # When did this file last update?

    ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks)=stat($file_to_check);  
    # Is this the same as the last time we checked?

    $secs_since_last_update=$mtime - $last_update_time;

    if ($secs_since_last_update > $Min_secs) {
	if ($Debug_level > 1) {
	    print(STDERR "$current_time_string: Last update time $mtime is > $Min_secs,\n");
	}
	
	# Register with the DataMapper

	$cmd="LdataInformer -t $datamap_string -d $LDM_data_dir -D $LDM_data_type";
	
	if ($Debug) {
	    print(STDERR "Running cmd: $cmd\n");
	}

      Toolsa::safe_system($cmd, $Timeout);

	# Update the last modify time

	$last_update_time=$mtime;

    }

    # Register with the procmap

    if ($Do_register) {
      Toolsa::PMU_auto_register("Sleeping $Sleep_secs");
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
    Toolsa::PMU_auto_unregister();
  }

  # Remove the tmpfile if it exists

  if (-e $tmpfile) {
      $nfiles=unlink $tmpfile;
      if ($nfiles <= 0) {
	  print(STDERR "WARNING: Could not delete tmpfile $tmpfile\n");
      }
  }

exit 0;

}


#========================================= EOF =====================================
