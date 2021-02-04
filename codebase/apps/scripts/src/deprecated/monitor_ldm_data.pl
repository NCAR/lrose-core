#!/usr/bin/perl
#
# monitor.pl
# 
# Function: Perl script to monitor an LDM directory for incoming data 
#           and register new data with the DataMapper
#
# Usage: run "monitor_ldm_data.pl -h" for options
#
# Input: data directory, subdirectory, data type, instance, interval
#
# Output:
#
# Dependencies: Requires the LdataInformer application to be in
#               the path. This is in CVS in: apps/didss/src/LdataInformer.
#
# Known bugs:
#
# Author: Deirdre Garvey NCAR/RAP 09-MAY-2001
#         Arnaud Dumont  NCAR/RAP 26-AUG-2003
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

# Unbuffer STDOUT
$| = 1;

#
# Set program defaults
#
$Timeout=900;

#
# Set command line defaults
#
my $Debug=0;                            # Flag to print debug messages to STDOUT
my $Debug_level=0;                      # Debug flag for verbose mode
my $Sleep_secs=60;                      # How long to sleep between checks (secs)
my $Do_test=0;                          # Flag for test mode
my $Do_register=1;                      # Flag to register with the DataMapper
my $LDM_data_dir="/data/ldm/data";      # LDM data directory
my $Instance="METAR";                   # Instance to pass to procmap for this script
my $LDM_dated_pattern=1;                # Search for file named "formattedDate.$Instance"
my $LDM_data_pattern='';                # Data file pattern to search for (will use script instance)
my $Min_secs=30;                        # Minimum number of seconds before update DataMapper
my $LDM_data_type="fos";                # Data type to register with the DataMapper

#
# Save the usage to print to the user if there is a problem
#
$usage =                                                 
    "\n" .
    "Usage: $prog [-dhnq] <-l dir> [-p pattern] <-t type> <-s secs> <-m secs> <-i instance> <-v level>\n" .
    "Purpose: To monitor a dataset in a directory to ensure that it is updating.\n" .
    "         Updates the DataMapper.\n" .
    "   -d --debug               : Print debugging messages\n" .
    "   -p --pattern <pattern>   : Regex pattern to match in dated directory.\n" .
    "                              Default is the current time, including hour, followed by\n" .
    "                              the script instance. For example: \"2003082619.METAR\"\n" .
    "   -h --help                : Print this usage message\n" .
    "   -i --instance <instance> : Instance to pass to procmap.\n" .
    "                              This may also be part of the search pattern.\n" .
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
		      'pattern=s',
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
  print "Input options specified...\n";
  print "\tpattern: $opt_pattern\n";
  print "\tinstance: $opt_instance\n";
  print "\tldm dir: $opt_ldmdir\n";
  print "\tmin secs: $opt_minsecs\n";
  print "\tnoregister: $opt_noregister\n";
  print "\tquit: $opt_quit\n";
  print "\tsleep secs: $opt_sleep\n";
  print "\tdata type: $opt_type\n";
  print "\tverbosedebuglevel: $opt_verbose\n";
}


if ($opt_pattern) {
    $LDM_data_pattern=$opt_pattern;
    $LDM_data_pattern=~ s/('|")//g; # Remove any quotes left by the shell
    $LDM_dated_pattern=0;
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
	print "ERROR: $prog: Invalid debug level. Ignoring it\n";
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

# Set up signal handlers now, just before the main loop.
$SIG{'INT'} = $SIG{'QUIT'} = 'DoExit';

# ----------------------------- Main -------------------------------

# Initialize the loop
$doLoop=1;

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
    # If we're using the default "dated pattern", build up the dated file pattern
    $LDM_data_pattern="${date_string}${hour_string}\.${Instance}" if ($LDM_dated_pattern);

    # Build the LDM directory to check
    $dataDir="${LDM_data_dir}/${date_string}";
    print "$prog: Checking directory $dataDir\n" if $Debug_level >= 3;

    # Get all of the files matching the pattern
    $openDir = opendir(DATADIR, "$dataDir");
    if( ! $openDir ) {
        print "Cannot open directory $dataDir\n" if $Debug_level >= 3;
    } else {
        print "$prog: Getting files which match pattern: ${LDM_data_pattern}\n" if $Debug_level >= 3;
        @allFiles = readdir(DATADIR);
        @files = grep /$LDM_data_pattern/, @allFiles;
        print "   Found ", scalar(@files), " matches. \n" if $Debug_level >= 3;
        closedir(DATADIR);
    }

    # Get the latest modification time of any of these files
    foreach $file (@files) {
        $modAge = (-M "$dataDir/$file")*24.0*3600.0;
        print "file:  $file age:  $modAge\n" if $Debug_level >= 3;
        if( ! defined $latestAge || $modAge < $latestAge ) {
            $latestAge = $modAge;
        }
    }

    # Make sure we got a valid mod time, which is long enough after the last mod time
    print "$prog: Latest mod time is $latestAge secs ago, previous mod time is $lastAge secs ago.\n" if ($Debug_level >= 2);
    if( defined $latestAge && (! defined $lastAge || ($lastAge - $latestAge) > $Min_secs )) {
	
        # Register with the DataMapper
	$cmd="LdataInformer -t $datamap_string -d $LDM_data_dir -D $LDM_data_type";
        print "Registering with cmd: $cmd\n" if ($Debug);
        Toolsa::safe_system($cmd, $Timeout);

        # Reset the last time we registered
        $lastAge = $latestAge;        
    }

    # Register this script with the procmap
    if ($Do_register) {
      Toolsa::PMU_auto_register("Sleeping $Sleep_secs");
    }

    # clear the latest age
    undef $latestAge;

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
    print "in DoExit...\n" if ($Debug);

    # Unregister from the procmap
    if ($Do_register) {
      Toolsa::PMU_auto_unregister();
    }

    exit 0;
}


#========================================= EOF =====================================
