#!/usr/bin/perl
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 2000 - 2005 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Program(RAP) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2005/11/22 18:27:5 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#
# check_Janitor_files.pl
# 
# Function: Perl script to check the _Janitor files in the data area
#
# Usage: run "check_Janitor_files.pl -h" for options
#
# Input: RAP_DATA_DIR directory
#
# Output: Prints out a couple of details about each _Janitor file found
#            in the input directory.
#         Returns $Exit_success if no errors found, returns $Exit_failure
#            otherwise.
#
# Overview: Searches the RAP_DATA_DIR for _Janitor files
#
# Dependencies:
#
# Known bugs:
#
# Author: Deirdre Garvey NCAR/RAP 11-MAR-2004
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

use Time::Local;                   # make 'date' available
use File::Find ();                 # make 'find' available

#
# --------------------------- Defaults and globals --------------
#
# Get the program basename.
#

($prog = $0) =~ s|.*/||;

#
# Set global defaults
#

$Exit_failure=0;                        # Return value on failure from $prog
$Exit_success=1;                        # Return value on success from $prog

#
# Set command line defaults.
#

$Debug=0;                               # Flag to print debug messages to STDERR
$InDir="NULL";
$Test=0;

#
# Save the usage to print to the user if there is a problem
#
$usage =                                                 
    "\n" .
    "Usage: $prog -i <dir> [-dh]\n" .
    "Purpose: To check the input directory recursively for _Janitor files\n" .
    "   -d --debug            : Print debugging messages\n" .
    "   -h --help             : Print this usage message\n".
    "   -i --indir <dir>      : (Required) Input directory to search\n";


# ----------------------------- Parse command line --------------------------
#
# Get the arguments from the command line
#
$result = &GetOptions('debug',
		      'help',
		      'indir=s',
                       '<>', \&badArg );

if ( $result == 0 || $opt_help ) {
   print $usage;
   exit ($Exit_failure);
}

if ($opt_debug) {
    $Debug=1;
}

if ($opt_indir) {
    $InDir=$opt_indir;
} else {
    print(STDERR "ERROR: $prog: You must specify a directory\n");
    exit ($Exit_failure);
}

#--------------------------- Error checking -------------------------
#
# Does the directory exist?

if (!-d $InDir) {
    print(STDERR "ERROR: $prog: The input directory does not exist: $InDir\n");
    exit($Exit_failure);
}

# --------------------------- Initialization -----------------------
#
# Set up signal handlers now, just before the main loop.

$SIG{'INT'} = $SIG{'QUIT'} = 'DoSignalExit';

# -------------------------------- Main -----------------------------
#
# Set a global for errors
$found_errors=0;

# Run find, old call was: &find($InDir);
# From find2perl...
#   Set the variable $File::Find::dont_use_nlink if you're using AFS,
#   since AFS cheats.
#   for the convenience of &wanted calls, including -eval statements:
#
use vars qw/*name *dir *prune/;
*name   = *File::Find::name;
*dir    = *File::Find::dir;
*prune  = *File::Find::prune;

File::Find::find( {wanted => \&wanted, follow => 1}, $InDir);

if ($found_errors) {
    exit($Exit_failure);
} else {
    exit($Exit_success);
}

#----------------------------- SUBROUTINES -----------------------------
#
# This is the subroutine called by find()
#

sub wanted {
    (($dev,$ino,$mode,$nlink,$uid,$gid) = lstat($_))
	|| warn "stat: $name: $!\n";

    # Local variables
    
    local($subname, $cmd);

    # Set defaults

    $subname="wanted";

    # Debugging

    if ($Debug) {
	print(STDERR "$subname: name: $name\n");
    }

    # Return if a directory

    if (-d $name) {
	return;
    }

    # We just want _Janitor files

    if ($name !~ /_Janitor$/) {
	return;
    }

    print(STDERR "----- $subname: Found a wanted file: $name\n");

    # Does the file have a size?

    if (-z $name) {
	print(STDERR "ERROR: zero length file: $name\n");
	$found_errors=1;
	return;
    }

    # Print important details
    
    $cmd="grep ^delete_files $name";
    system($cmd);
    $cmd="grep ^RemoveEmptyDirs $name";
    system($cmd);
    $cmd="grep ^MaxNoModDays $name";
    system($cmd);

}

#---------------------------------------------------------------------------
# Subroutine DoSignalExit
#
# Usage: DoExit
#
# Function: Exit
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

  exit ($Exit_failure);

}

#==================================== EOF ==================================

