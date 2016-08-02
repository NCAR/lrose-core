#!/usr/bin/perl
#
# Name: watch_file_update.pl
#
# Function: Watch a file to see if it is updating
#
# Usage: see usage below
#
# Deirdre Garvey  NCAR/RAP/SDG      20-OCT-2010
#
#==================================================================
#
#
# --------------------------- Externals -------------------------
#
# For handling the long format of input options
#

use Getopt::Long;
use Env;
use Cwd;
Env::import();

# 
# External modules
#

use Time::Local;                   # make 'date' available

#
# --------------------------- Defaults and globals --------------
#
# Get the program basename.
#
($prog = $0) =~ s%.*/%%;

#
# Set global defaults
#
$Exit_success=1;
$Exit_failure=-1;
$This_host=`hostname`;
chop($This_host);

#
# Set command line defaults
#
$Debug=0;                                      # Flag for debug messages
$Debug_level=0;                                # Debug level
$Test=0;                                       # Test mode
$File_to_watch="";                             # Filename to watch
$File_late_secs=-1;                            # Number of seconds before warn that
                                               #   file update is late
$Check_secs=-1;                                # Number of seconds to wait between checks
$Do_send_email=0;                              # Flag to send email or not
$Mail_list="";                                 # List of email addresses

#
# Save the usage to print to the user if there is a problem
#
$usage=
    "\n" .
    "Usage: $prog -f <file> -l <latesecs> -c <checksecs> [-m <maillist>] [-dht] [-v level]\n" .
    "Purpose: To watch the input file and warn if it has not updated in latesecs secs\n" .
    "         Logs the information to STDERR, and optionally sends warning email\n" .
    "   -c --checksecs <secs>     : (Required) Number of seconds to wait between checks\n" .
    "   -d --debug                : Print debugging messages.\n" .
    "   -h --help                 : Print this usage message\n" .
    "   -f --file <file>          : (Required) File to watch\n" .
    "   -l --latesecs <secs>      : (Required) Number of seconds to wait before warn that file is late\n" .
    "   -m --maillist <list>      : List of email addresses. The list string must be inside\n" .
    "                               quotes and individual addresses separated by a comma\n" .
    "                               If no list specified, no email is sent\n" .
    "   -t --test                 : Test mode\n" .
    "   -v --verbose <n>          : Print verbose debugging messages.\n" .
    "                               Specify the level 1..3. Also sets --debug\n";


# ----------------------------- Parse command line --------------------------
#
# Get the arguments from the command line
#
$result = &GetOptions('debug',
		      'help',
		      'checksecs=i',
		      'file=s',
		      'latesecs=i',
		      'maillist=s',
		      'test',
		      'verbose=i',
                       '<>', \&badArg );

if ( $result == 0 || $opt_help ) {
   print $usage;
   exit $Exit_failure;
}

if ($opt_debug) {
  $Debug=1;
  $Debug_level=1;
  print(STDERR "Input options specified...\n");
  print(STDERR "\tchecksecs: $opt_checksecs\n");
  print(STDERR "\tfile: $opt_file\n");
  print(STDERR "\tlatesecs: $opt_latesecs\n");
  print(STDERR "\tmaillist: $opt_maillist\n");
  print(STDERR "\tverbose: $opt_verbose\n");
  print(STDERR "\ttest: $opt_test\n");
}

if ($opt_file) {
    $File_to_watch = $opt_file;
} else {
    print(STDERR "ERROR: $prog: You must specify a file to watch\n");
    exit $Exit_failure;
}

if ($opt_checksecs) {
    $Check_secs= $opt_checksecs;
} else {
    print(STDERR "ERROR: $prog: You must specify a number of seconds between checks\n");
    exit $Exit_failure;
}

if ($opt_latesecs) {
    $File_late_secs= $opt_latesecs;
} else {
    print(STDERR "ERROR: $prog: You must specify a number of late seconds\n");
    exit $Exit_failure;
}

if ($opt_maillist) {
    $Mail_list=$opt_maillist;
    $Do_send_email=1;
}

if ($opt_test) {
    $Test=1;
}

if ($opt_verbose) {
    $Debug=1;
    if ($opt_verbose < 1) {
	$Debug_level = 1;
    } else {
	$Debug_level=$opt_verbose;
    }
}

#--------------------------- Error checking -------------------------
#
# Does the file exist

if (!-f $File_to_watch) {
    print(STDERR "ERROR: $prog: File to watch does not exist: $File_to_watch\n");
    exit $Exit_failure;
}

# Get the maillist into an array

if ($Do_send_email > 0) {
    ($is_ok, $nlist)=getArray($Mail_list, *maillistarr, $Debug_level);
    if (!$is_ok) {
	exit $Exit_failure;
    }
}

#--------------------------- Initialization ---------------------------
#

# Set up signal handlers now, just before the main loop.

$SIG{'INT'} = $SIG{'QUIT'} = 'DoSignalExit';

#------------------------------- Main ---------------------------------

# Loop forever, checking on the file update times

$send_email=1;
$keep_looping=1;
while ($keep_looping == 1) {

    # Get the current time

    $now=time();
    $date=`date`;
    chop($date);

    # Get the file modify time

    ($is_ok, $file_mtime)=getFileModifyTime($File_to_watch, $Debug_level);

    # Is the file late?

    $update_secs=$now - $file_mtime;
    if ($update_secs >= $File_late_secs) {
	
	# Print warning

	print(STDERR "Warning: $date: $prog: file is late: $File_to_watch.\n");
	print(STDERR "\thas not updated in $update_secs seconds, expected update in $File_late_secs secs\n");
	
	# Send email? if so, only send once

	if (($Do_send_email > 0) && ($send_email > 0)) {
	    $is_ok=sendWarnEmail(*maillistarr, $update_secs, $Test, $Debug_level);
	    $send_email=0;
	}
	
    } else {
	if($Debug_level > 1) {
	    print(STDERR "Debug: $prog: file updated $update_secs secs ago, is not late\n");
	}
	$send_email=1;
    }

    if ($Test) {
	print(STDERR "Test: $prog: would sleep $Check_secs seconds\n");
	$keep_looping=0;
    } else {
	if ($Debug) {
	    print(STDERR "Debug: $prog: Sleeping $Check_secs seconds\n");
	}
	sleep($Check_secs);
    }
}

exit $Exit_success;

# ----------------------------- Subroutines -----------------------------
#
# Subroutine getFileModifyTime
#
# Usage: ($ret_val, $file_mod_time)=getFileModifyTime($file, $debug)
#
# Function: Get the file modify time
#
# Input:    $file            file to check
#           $debug           flag for debugging (1=on, 0=off)
#
# Output:   $ret_val         1 on success, 0 on error
#           $file_mod_time   Unix time for last file modify time 

#
# Overview:
#

sub getFileModifyTime
{
  local ($file, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($file_mod_time);
  local($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size);
  local($atime,$mtime,$ctime,$blksize,$blocks);

  # Set defaults

  $return_val=0;
  $subname="getFileModifyTime";
  $file_modify_time=-1;

  # Get the file modify time

  ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks)=stat($file);
  if ($mtime > 0) {
      $file_modify_time=$mtime;
      $return_val=1;
  }

  # Done
  return($return_val, $file_modify_time);
}

#----------------------------------------------------------------------
# Subroutine sendWarnEmail
#
# Usage: ($ret_val)=sendWarnEmail(@arr, $update_secs, $test, $debug)
#
# Function: Send a warning email to the list
#
# Input:    @arr             array of email addresses
#           $update_secs     update secs
#           $test            test mode (1=on, 0=off)
#           $debug           flag for debugging (1=on, 0=off)
#
# Output:   $ret_val         1 on success, 0 on error
#
# Overview:
#

sub sendWarnEmail
{
  local (*arr, $update_secs, $test, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($subject, $now, $tmp_file, $date, $mailname);

  # Set defaults

  $return_val=0;
  $subname="sendWarnEmail";
  $now=time();
  $tmp_file="/tmp/${prog}.${now}";
  $date=`date`;
  chop($date);

  # Debug

  if ($debug == 2) {
      print(STDERR "$subname: Inputs...\n");
      print(STDERR "\tupdate_secs: $update_secs, test: $test, debug: $debug\n");
      foreach $mailname (@arr) {
	  print(STDERR "\tmailname: $mailname\n");
      }
  }

  # Setup the email

  $subject="Warning: on host $This_host: File being watched has not updated";
  
  if (!open(OUTFILE, ">$tmp_file")) {
      print(STDERR "ERROR: $subname: Cannot open file $tmp_file\n");
      return($return_val);
  }

  print(OUTFILE "$date: Host: $This_host\n");
  print(OUTFILE "File being watched: $File_to_watch\n");
  print(OUTFILE "\tfile last updated $update_secs secs ago, that is more than expected $File_late_secs late secs\n");
  close(OUTFILE);

  if ($test > 0) {
      print(STDERR "$subname: Would send email with the following contents:\n");
      system("cat $tmp_file");
  }

  foreach $mailname (@arr) {
      if ($debug > 0) {
	  print(STDERR "$subname: Sending email to: $mailname\n");
      }
      if ($test > 0) {
	  print(STDERR "Test: $subname: would send email to $mailname, with subject $subject\n");
      } else {
	  system("mail -s \"$subject\" $mailname < $tmp_file");
      }
  }

  # Remove the tmp file 

  unlink $tmp_file;

  # Done

  $return_val=1;
  return($return_val);
}

#--------------------------------------------------------------------------------
# Subroutine: getArray
#
# Usage: ($return_val, $noutarr)=&getArray($inlist, *outarr, $debug)
#
# Function: Parse the input $inlist and return in *outarr
#
# Input:  $inlist           list to parse
#         $debug            flag (1=on, 0=off) for debug info
#
# Output: $return_val       1 on success, 0 otherwise
#         $noutarr          size of *outarr
#         *outarr           array items in the inlist
#

sub getArray {
    local($inlist, *outarr, $debug)  = @_;

    # Define local variables

    local($return_val, $subname);
    local($dbg2, $dbg3);
    local($noutarr, $counter, @split_list, $item, $junk);
    local($sub_counter, $j);

    # Set defaults

    $return_val = 0;
    $subname = "getArray";
    $noutarr=0;
    $dbg2=0;
    $dbg3=0;

    # Debug

    if ($debug == 2) {
	$dbg2=1;
    }
    if ($debug == 3) {
	$dbg3=1;
    }

    if ($debug) {
	print(STDERR "$subname: Input...\n");
	print(STDERR "\tinlist: $inlist\n");
    }

    @split_list=split('\,', $inlist);

    $counter=0;
    foreach $item (@split_list) {
	if ($dbg2) {
	    print(STDERR "$subname: list item: $item\n");
	}

	if (length($item) <= 1) {
	    next;
	}

	if ($item !~ /\w/) {
	    next;
	}
	$outarr[$counter]=$item;
	$counter++;
    }

    $noutarr=$counter;
    if ($counter > 0) {
	$return_val=1;
    }

    # Debug

    if ($dbg2) {
	print(STDERR "$subname: noutarr: $noutarr\n");
	for ($j=0; $j<$noutarr; $j++) {
	    print(STDERR "\tj: $j, dir: $outarr[$j]\n");
	}
    }

    return($return_val, $noutarr);
}

#---------------------------------------------------------------------------
# Subroutine DoSignalExit
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

sub DoSignalExit
{
 
  # Local variables

  local($subname);

  # Set defaults

  $subname="DoSignalExit";

  if ($Debug) {
    print(STDERR "in $subname...\n");
  }

  # Done

  exit $Exit_failure;

}

# -------------------------------- EOF -------------------------------
