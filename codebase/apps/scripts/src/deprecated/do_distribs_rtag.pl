#!/usr/bin/perl
#
# Name: do_distribs_rtag.pl
#
# Function:
#	Perl script to tag a distribs file and all the 'cvs co'
#       commands in it with a CVS rtag
#
# Overview:
#       Checks out a distribs file from CVS and then creates a
#       copy of it replacing all 'cvs co' with 'cvs rtag' commands
#       in it. 
#
# Usage:
#       do_distribs_rtag.pl
#
# Input:
#       1) An ASCII distribs file with a set of 'cvs co' commands
#
# Output:
#
# Dependencies:
#	Perl must be available in the location at the top of this file.
# 	
# Author: Deirdre Garvey - NCAR/RAP/SDG
#
# 16-MAR-2009
#
#---------------------------------------------------------------------------------
#
#===================================================================
#
# --------------------------- Externals -------------------------
#
# The sys_wait_h is required to get the correct return codes from the system() calls.

require 5.002;
use POSIX 'sys_wait_h'; 
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

($prog = $0) =~ s|.*/||;

# ... Like pound-defines ...
$ExitSuccess=0;
$ExitFailure=1;

$StringDefault="NOTDEFINED";

#
# Set command line defaults
#

$Debug=0;                           # Flag to print debug messages to STDERR
$Debug_level=0;                     # Level of debugging
$Rtag=$StringDefault;               # Rtag to use
$PrevRtag=$StringDefault;           # Previous rtag to use
$DistribsFile=$StringDefault;       # Distribs file to use
$CheckoutDir="/tmp";                # Directory to checkout distribs file into
$DoExecuteRtag=1;                   # Flag to actually rtag the files
$DoTagDistribs=1;                   # Flag to tag the distribs file
$UseTaggedDistribs=0;               # Flag to use a specific tagged distribs
$DoCVSLatest=1;                     # Flag to tag latest-and-greatest in CVS
$SpecificDistribsRtag=$StringDefault;

#
# Save the usage to print to the user if there is a problem
#
$usage =                                                 
    "\n" .
    "Usage: $prog -r <tag> -i <input_distribs_file> [-dhn] <-c dir> <-p tag> <-v level>\n" .
    "Purpose: To checkout a distribs file, create a copy (with a new name) to be\n" .
    "         a cvs rtagging file and optionally execute the cvs rtagging file.\n" .
    "         By default will rtag the latest-and-greatest in CVS\n" .
    "   -c --checkoutdir <dir>   : Directory to checkout the distribs file into.\n" .
    "                              Default: $CheckoutDir\n" .
    "   -d --debug               : Print debugging messages\n" .
    "   -h --help                : Print this usage message\n" .
    "   -i --indistribsfile <file>: (Required.) The name of the distribs\n" .
    "                              file to checkout. Must be the full CVS\n" .
    "                              name of the distibs file, e.g., distribs/juneau_checkout\n" .
    "                              not juneau_checkout\n" .
    "   -n --notexecute          : Flag to not execute the rtagging file, just create it\n" .
    "                              in the --checkoutdir\n" .
    "   -p --prevtagname <tag>   : Instead of tagging the latest and greatest in CVS, apply the\n" .
    "                              new --rtagname to files already tagged with the --prevtagname tag\n" .
    "   -r --rtagname <tag>      : (Required) CVS rtag to use in (1) tagging the distribs file\n" .
    "                              and (2) generating the rtagging file\n" .
    "   -s --skiptaggingdistribs : Flag to skip tagging the distribs file with the\n" .
    "                              --rtagname\n" .
    "   -u --usetaggeddistribs <tag>: CVS rtag to use in checking out the distribs file\n" .
    "                                 The distribs file must already be tagged with this tag.\n" .
    "   -v --verbose <num>       : A debug level number (1..2)\n" .
    "                              Will also turn on --debug\n";

# ----------------------------- Parse command line --------------------------
#
# Get the arguments from the command line
#
$result = &GetOptions('checkoutdir=s',
		      'debug',
		      'help',
		      'indistribsfile=s',
		      'notexecute',
		      'prevtagname=s',
		      'rtagname=s',
		      'skiptaggingdistribs',
		      'usetaggeddistribs=s',
		      'verbose=i',
                       '<>', \&badArg );

$FoundErrors=0;
if ( $result == 0 || $opt_help ) {
   print $usage;
   exit $ExitFailure;
}

if (!$opt_indistribsfile) {
    print (STDERR "ERROR: You must specify a distribs file to use\n");
    $FoundErrors=1;
} else {
    $DistribsFile=$opt_indistribsfile;
}

if ($opt_prevtagname) {
    $PrevRtag=$opt_prevtagname;
    $DoCVSLatest=0;
}

if (!$opt_rtagname) {
    print (STDERR "ERROR: You must specify a CVS rtag to use\n");
    $FoundErrors=1;
} else {
    $Rtag=$opt_rtagname;
}

if ($opt_debug) {
  $Debug=1;
  $Debug_level=1;
}

if ($opt_checkoutdir) {
    $CheckoutDir=$opt_checkoutdir;
}

if ($opt_notexecute) {
    $DoExecuteRtag=0;
}

if ($opt_usetaggeddistribs) {
    $UseTaggedDistribs=1;
    $SpecificDistribsRtag=$opt_usetaggeddistribs;
}

if ($opt_skiptaggingdistribs) {
    $DoTagDistribs=0;
}

if ($opt_verbose) {
  if ($opt_verbose < 0) {
    print(STDERR "ERROR: $prog: Invalid debug level. Ignoring it\n");
    $Debug=$Debug_level= 1;
  } else {
    $Debug_level=$opt_verbose;
    $Debug=1;
  }
}

# --------------------------- Error checking ---------------------
#

if ($FoundErrors) {
    exit $ExitFailure;
}

# --------------------------- Debugging --------------------------
#
if ($Debug) {
    print(STDERR "Running $prog\n");
    print(STDERR "Options specified...\n");
    print(STDERR "\tdistribs file: $DistribsFile\n");
    print(STDERR "\trtag: $Rtag\n");
    print(STDERR "\tcheckout dir: $CheckoutDir\n");
    print(STDERR "\trun the rtagging file: $DoExecuteRtag\n");
    print(STDERR "\tdo tag the distribs file: $DoTagDistribs\n");
    print(STDERR "\tuse an already tagged distribs: $UseTaggedDistribs\n");
    print(STDERR "\ttag latest-and-greatest in CVS: $DoCVSLatest\n");
    print(STDERR "\tprevious rtag to use in retag (optional): $PrevRtag\n");
    print(STDERR "\tspecific distribs rtag: $SpecificDistribsRtag\n");
    print(STDERR "\tverbosedebuglevel: $opt_verbose\n");
}


# --------------------------- Initialization -----------------------
#
# Get today

$Today=`date`;
chop($Today);

# Determine a full pathname for the checkout dir so handle relative
# pathnames correctly. If checkout dir starts with a slash, it is
# absolute, not relative

if ($CheckoutDir !~ /^\//) {
    $cwd = Cwd::cwd();
    $CheckoutDir="${cwd}/${CheckoutDir}";
}

#
# Does the checkout dir exist?
#

if (!-d $CheckoutDir) {
    system("mkdir -p $CheckoutDir");
}

# ------------------------------ Main -------------------------------
#
# Checkout the distribs file, create a copy with rtag commands in it

$is_ok=&checkoutFiles($Debug_level);
if (!$is_ok) {
    print(STDERR "Exiting...\n");
    $ExitFailure;
}

# Done

exit $ExitSuccess;


# =============================== SUBROUTINES ===========================
#
# Subroutine checkoutFiles
#
# Usage: $return_val = checkoutFiles()
#
# Function: checkout distribs file, create a copy with rtags instead of 'cvs co'
#
# Input:    globals
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub checkoutFiles
{
  local ($debug) = @_;

  # Local variables

  local($subname, $return_val);
  local($dbg2, $dbg3);
  local($is_ok, $co_ok, $cmd, $dir, $file);

  # Set defaults

  $return_val=0;
  $subname="checkoutFiles";

  # Debug

  $dbg2=0;
  $dbg3=0;

  if ($debug == 2) {
      $dbg2=1;
  }

  if ($debug == 3) {
      $dbg3=1;
      $dbg2=1;
  }

  # Checkout the distribs file so know what else to checkout

  chdir($CheckoutDir);

  if ($debug) {
      print(STDERR "Checkout distribs file: $DistribsFile\n");
  }
  if ($UseTaggedDistribs) {
      if ($debug) {
	  print(STDERR "\twith specific rtag: $SpecificDistribsRtag\n");
      }
      $cmd="cvs co -r $SpecificDistribsRtag $DistribsFile";
  } else {
      $cmd="cvs co $DistribsFile";
  }

  $is_ok=system($cmd);
  $co_ok=WEXITSTATUS($?);
  if ($co_ok != 0) {
      print(STDERR "ERROR! Cannot checkout the distribs file: ${DistribsFile}\n");
      return ($return_val);
  }

  # Tag the distribs file itself unless execute turned off

  if ($DoTagDistribs) {
      if ($debug) {
	  print(STDERR "Do rtag the distribs file itself with: $Rtag\n");
	  print(STDERR "\tthe distribs file: $DistribsFile\n");
      }
      $cmd="cvs rtag $Rtag $DistribsFile";
      $is_ok=system($cmd);
  }

  # Create a copy of the distribs file with the rtag specified

  $file="${DistribsFile}.${Rtag}";
  if ($debug) {
      print(STDERR "Create a version of ${DistribsFile} as the rtagging file\n");
      print(STDERR "\tfile: $file\n");
  }

  # Open the input file

  $is_ok=open(INFILE, $DistribsFile);
  if (!$is_ok) {
      print(STDERR "ERROR: $subname: Cannot open file $CheckoutDir/$DistribsFile\n");
      return($return_val);
  }

  # Open the output file

  $is_ok=open(OUTFILE, "> $file");
  if (!$is_ok) {
      print(STDERR "ERROR: $subname: Cannot open file $CheckoutDir/$file\n");
      return($return_val);
  }

  # Parse the file

  while ($line = <INFILE>) {

      # Skip comment lines

      if ($line =~ /^#/) {
	  print(OUTFILE $line);
	  next;
      }

      # Skip blank lines

      if ($line !~ /\w/) {
	  print(OUTFILE $line);
	  next;
      }

      # Debug

      if ($dbg3) {
	  print (STDERR $line);
      }

      # Have we found a line with a cvs co in it, if not make it a
      # comment

      if ($line =~ /cvs co/) {
	  if ($DoCVSLatest > 0) {
	      $line =~ s/cvs co/cvs rtag ${Rtag}/;
	  } else {
	      $line =~ s/cvs co/cvs rtag -r ${PrevRtag} ${Rtag}/;
	  }
      } else {
	  $line="## $line";
      }
      print(OUTFILE $line);

  } #endwhile

  close(INFILE);
  close(OUTFILE);

  # Set execute permissions

  $cmd="chmod 777 ${file}";
  $is_ok=system($cmd);

  # Execute the rtagging file

  if ($DoExecuteRtag) {
      if ($debug) {
	  print(STDERR "Executing rtag file: $file for tag $Rtag\n");
      }
      $cmd="${CheckoutDir}/$file";
      $is_ok=system($cmd);
  } else {
      print(STDERR "The rtagging file is in: $CheckoutDir/$file\n");
      print(STDERR "It has not been executed to rtag the files\n");
  }

  # Done

  $return_val=1;
  return($return_val);

}

#========================================= EOF =====================================
