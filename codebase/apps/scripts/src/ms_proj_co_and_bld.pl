#!/usr/bin/perl -w
#
# Name: ms_proj_co_and_bld.pl
#
# Function:
#	Perl script to checkout the Modeling System project source
#       code from CVS and build it.
#
# Overview:
#       Checks out a distribs file from CVS and then executes it
#       to checkout a set of project files. Then does a series of
#       shell commands on the checked out code.
#
# Usage:
#       ms_proj_co_and_bld.pl
#
# Input:
#       1) An ASCII param file for this script
#       2) An ASCII distribs file with a set of 'cvs co' commands
#
# Output:
#       1) an ASCII param file for use with this script if use --print_params
#          option on the command line. This is printed to STDOUT.
#       2) header information (e.g., hostname, g++ version) to STDERR.
#
# Dependencies:
#	1) Perl must be available in the location at the top of this file.
#       2) The needed Perl modules must be installed and available
#       3) PERL_LIB_DIR env var must be defined and have needed local
#          Perl modules in it: ms_co_and_bld.pm
#
# 	
# Author: Deirdre Garvey - NCAR/RAP/SDG        04-JUN-2007 from a similar
#                                              script "ms_src_co_and_bld.pl"
#
#---------------------------------------------------------------------------------
#
# --------------------------- Externals -------------------------
#

use Getopt::Long;
use Env;
use Cwd;
Env::import();
use Time::Local;                   # make 'date' available
use strict;

# Get the local Perl .pm libs

use Env qw(PERL_LIB_DIR);
use lib "$PERL_LIB_DIR";
use ms_co_and_bld;

#
# --------------------------- Defaults and globals --------------
#
# Get the program basename.

our $prog;
($prog = $0) =~ s|.*/||;

# hostname

our $HostName=`hostname`;
chop($HostName);

# ... Like pound-defines ...
our $ExitSuccess=0;
our $ExitFailure=1;

our $StringDefault="UNKNOWN";
our $IntDefault=-1;
our $FloatDefault=-1.0;

our $BE_dir_idx=0;
our $BE_exec_idx=1;

# Get environment variables. Most are just to print to log for reference.

#our $env_make_macros_dir=$ENV{MS_MAKE_MACROS_DIR};
#our $env_os_type=$ENV{MS_OS_TYPE};
our $env_perl_lib_dir=$ENV{PERL_LIB_DIR};

# Check for missing local perl lib

if (!-e "${env_perl_lib_dir}/ms_co_and_bld.pm") {
    print(STDERR "ERROR: $prog: Cannot find local perl lib ms_co_and_bld.pm\n");
    print(STDERR "\tis PERL_LIB_DIR env var defined?\n");
    exit $ExitFailure;
}

#
# Set command line defaults
#

our $Debug=0;                           # Flag to print debug messages to STDERR
our $Debug_level=0;                     # Level of debugging
our $DoPrintParams=0;                   # Flag for print_params
our $Test=0;                            # Flag for test mode
our $ConfigFile=$StringDefault;         # Config file to read

#
# Save the usage to print to the user if there is a problem
#
our $usage =                                                 
    "\n" .
    "Usage: $prog -p|-c <file> [-dht] <-v level>\n" .
    "Purpose: To checkout and build Modeling System project code.\n" .
    "   -c --config <file>    : (Required.) The name of the config/param\n" .
    "                           file to read. NOT required if running\n" .
    "                           --print_params\n" .
    "   -d --debug            : Print debugging messages\n" .
    "   -h --help             : Print this usage message\n" .
    "   -p --print_params     : Generate an input config/param file for\n" .
    "                           this script. Writes to STDOUT.\n" .
    "   -t --test             : Test mode, only report on what would have been done\n" .
    "   -v --verbose <num>    : A debug level number (1..2)\n" .
    "                           Will also turn on --debug\n";

# ----------------------------- Parse command line --------------------------
#
# Get the arguments from the command line
#
our ($opt_help, $opt_config, $opt_debug, $opt_print_params, $opt_test, $opt_verbose);

our $result = &GetOptions('config=s',
			  'debug',
			  'help',
			  'print_params',
			  'test',
			  'verbose=i',
			  '<>', \&badArg );

if ( $result == 0 || $opt_help ) {
   print $usage;
   exit $ExitFailure;
}

if ((!$opt_config) && (!$opt_print_params)) {
    print(STDERR "ERROR: You must specify a config/param file!\n");
    print $usage;
    exit $ExitFailure;
}

if ($opt_debug) {
    $Debug=1;
    $Debug_level=1;
}

if ($opt_config) {
    $ConfigFile= $opt_config;
}

if ($opt_print_params) {
    $DoPrintParams=1;
}

if ($opt_test) {
    $Test=1;
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

if ($Debug) {
    print(STDERR "Will run with...\n");
    print(STDERR "\tDebug: $Debug\n");
    print(STDERR "\tConfigFile: $ConfigFile\n");
    print(STDERR "\tDoPrintParams: $DoPrintParams\n");
    print(STDERR "\tTest: $Test\n");
    print(STDERR "\tDebug_level: $Debug_level\n");
}

# --------------------------- Error checking ---------------------
#
my $FoundErrors=0;

# Does the config file exist?

if ((!-e $ConfigFile) && (!$DoPrintParams)) {
  print(STDERR "ERROR: $prog: The config file $ConfigFile does not exist\n");
  $FoundErrors=1;
}

if ($FoundErrors) {
    exit $ExitFailure;
}

# --------------------------- Initialization -----------------------
#
# Get today

my $Today=`date`;
chop($Today);

# Get the processing start time for statistics

my $start_utime=time;

# Set global parameter defaults

our($DoCheckout, $CheckoutDir, $DistribsFiles, @DistribsFilesArr);
our($TmpDir, $DeleteDirs, @DeleteDirsArr, $CreateDirs, @CreateDirsArr);
our($DoRtag, $Rtag, $DoBuildList);
our($MyPreBuildScript, $DoMyPreBuildScript);
our($MyPostBuildScript, $DoMyPostBuildScript);
our($MyBuildCheckScript, $DoMyBuildCheckScript);

&setAllParams2Defaults;

# Set local variables

my $is_ok;

# --------------------------- Print Params -------------------------
#
# Are we doing print_params? If so, generate the file and exit
#
if ($DoPrintParams) {
    $is_ok = &generateParamFile;
    exit ($is_ok);
}

# ------------------------------ Main -------------------------------
#
# Get the g++ version since this is a source of confusion

my $gcpp_version=`g++ --version`;

# Print banner

print(STDERR "*******************************************************************\n");
print(STDERR "Running $prog\n");
print(STDERR "\tHostname: ${HostName}\n");
print(STDERR "\tg++ version: $gcpp_version\n");
print(STDERR "\tStart time: ${Today}\n");
print(STDERR "\tEnvironment:\n");
#print(STDERR "\t\tMS_MAKE_MACROS_DIR: $env_make_macros_dir\n");
#print(STDERR "\t\tMS_OS_TYPE: $env_os_type\n");
print(STDERR "*******************************************************************\n");

# Read the config/param file and set globals

our $nbuildarr;
our @buildArr;

($is_ok, $nbuildarr) = &readParamFile($ConfigFile, \@buildArr, $Debug_level);
if (!$is_ok) {
    print(STDERR "Exiting...\n");
    exit $ExitFailure;
}

if ($Debug) {
    &printAllParams;
}

# Delete dirs before start

&doDeleteDirs(\@DeleteDirsArr, $Test, $Debug_level);

# Create dirs

$is_ok=&doCreateDirs(\@CreateDirsArr, $Test, $Debug_level);
if (!$is_ok) {
    print(STDERR "Exiting...\n");
    exit $ExitFailure;
}

# Checkout files

if ($DoCheckout) {
    $is_ok=&doCheckoutFiles(\@DistribsFilesArr, $CheckoutDir, $DoRtag, $Rtag, $Test, $Debug_level);
    if (!$is_ok) {
	print(STDERR "Exiting...\n");
	exit $ExitFailure;
    }
}

# Pre-Build

if ($DoMyPreBuildScript) {
    $is_ok=&runExternalScript($MyPreBuildScript, $Test, $Debug_level);
    if (!$is_ok) {
	print(STDERR "Exiting...\n");
	exit $ExitFailure;
    }
}

# Build

if ($DoBuildList) {
    $is_ok=&build($Test, $Debug_level);
    if (!$is_ok) {
	print(STDERR "Exiting...\n");
	exit $ExitFailure;
    }
}

# Post build

if ($DoMyPostBuildScript) {
    $is_ok=&runExternalScript($MyPostBuildScript, $Test, $Debug_level);
    if (!$is_ok) {
	print(STDERR "Exiting...\n");
	exit $ExitFailure;
    }
}

# Check script

if ($DoMyBuildCheckScript) {
    $is_ok=&runExternalScript($MyBuildCheckScript, $Test, $Debug_level);
    if (!$is_ok) {
	print(STDERR "Exiting...\n");
	exit $ExitFailure;
    }
}

# Done

exit $ExitSuccess;


# =============================== SUBROUTINES ===========================
#
# Subroutine build
#
# Usage: $return_val = build()
#
# Function: build dirs
#
# Input:    globals
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub build
{
  my ($test, $debug) = @_;

  # Local variables

  my($subname, $return_val);
  my($dbg2, $dbg3);
  my($is_ok, $cmd_ok, $i, $dir, $exec_cmds);

  # Set defaults

  $return_val=0;
  $subname="build";

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

  # Do the builds

  for ($i=0; $i<$nbuildarr; $i++) {
      $dir=$buildArr[$i][$BE_dir_idx];
      $exec_cmds=$buildArr[$i][$BE_exec_idx];

      if ($debug) {
	  print(STDERR "======= do src dir: $dir, with: $exec_cmds =======\n");
      }

      $exec_cmds="cd $dir; $exec_cmds";
      ($is_ok, $cmd_ok)=execSystemCall($exec_cmds, $test, $debug);

  } #endfor

  # Done
  
  $return_val=1;
  return($return_val);
}

#-------------------------------------------------------------------------
#
# Subroutine readParamFile
#
# Usage: ($return_val, $nbuild) = readParamFile($param_file, *outBuildArr, $debug)
#
# Function: Read the $param_file.
#
# Input:    $param_file          file to read
#           $debug               flag for debugging (1=on, 0=off)
#
# Output:   $ret_val             1 on success, 0 on error
#           $nbuild              size of *outBuildArr
#           *outBuildArr         array of build entries
#	         [n][$BE_dir_idx]  = build directory
#                [n][$BE_exec_idx] = build exec commmands
#
# Overview:
#

sub readParamFile
{
  my ($param_file, $outBuildArr, $debug) = @_;

  # Local variables

  my($return_val, $subname);
  my($dbg2, $dbg3);
  my($nbuild, $build_counter);
  my($is_ok, $keyword, $keyvalue, $type, $unstripped_keyvalue);
  my($save_delete_dirs);
  my($table_counter, $found_build_list, $found_be_list);
  my($line, $be_dir, $be_exec, $missing);
  my(@junk, $firstequals, $len);
  my($local_found, $local_type);

  # Set defaults

  $return_val=0;
  $subname="readParamFile";
  $nbuild=0;

  # Debug

  $dbg2=0;
  $dbg3=0;
  
  if ($debug == 2) {
      $dbg2=1;
  }

  if ($debug == 3) {
      $dbg3=1;
  }

  # Debugging

  if ($dbg2) {
      print(STDERR "$prog: $subname: Input paramfile: $param_file\n");
  }

  # Open the param file

  $is_ok=open(PARAM_FILE, $param_file);
  if (!$is_ok) {
      print(STDERR "ERROR: $prog: $subname: Cannot open file $param_file\n");
      return($return_val, $nbuild);
  }

  # Set loop defaults

  $found_build_list=0;
  $found_be_list=0;
  $build_counter=0;

  # Parse the file

  while ($line = <PARAM_FILE>) {

      # Skip comment lines

      if ($line =~ /^#/) {
	  next;
      }

      # Skip blank lines

      if ($line !~ /\w/) {
	  next;
      }

      # Debug

      if ($dbg3) {
	  print (STDERR $line);
      }

      # Have we found a line with an equals sign? Parse the keyword

      if ($line =~ /\=/) {

	  # Split the line at the equals sign

	  ($keyword, $keyvalue) = split('\=',$line);

	  # Are there >1 equals signs? If so, just split at the first one
	  
	  @junk=split('\=',$line);
	  if (@junk > 2) {
	      $firstequals=index($line, "=");
	      $keyword=substr($line, 0, $firstequals);
	      $keyvalue=substr($line, $firstequals+1);
	      
	      if ($dbg3) {
		  print(STDERR "more than 1 equals sign in the line\n");
		  print(STDERR "\tfirstequals: $firstequals, keyword: $keyword, keyvalue: $keyvalue\n");
	      }
	  }

	  # Chop off any leading and trailing blanks, save the unstripped
	  # keyvalue as we may need it in a special case

	  $unstripped_keyvalue=$keyvalue;
	  chop($unstripped_keyvalue);
	  ($is_ok, $keyword)=&removeSpaces($keyword, $debug);
	  ($is_ok, $keyvalue)=&removeSpaces($keyvalue, $debug);

	  # Skip blank keyvalues
	  
	  if ($unstripped_keyvalue !~ /\w/) {
	      next;
	  }

	  # Handle the keywords

	  if ($keyword eq "do_checkout") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoCheckout=1;
	      } else {
		  $DoCheckout=0;
	      }
	  }

	  elsif ($keyword eq "checkout_dir") {
	      $type="string";
	      $CheckoutDir= $keyvalue;
	      ($is_ok, $CheckoutDir) = &expandEnvVar($CheckoutDir, $debug);
	  }

	  elsif ($keyword eq "distribs_files") {
	      $type="comma_delimited_string";
	      ($is_ok, $DistribsFiles) = &checkKeywordValue($keyword, $keyvalue, $type, *DistribsFilesArr, $debug);
	  }

	  elsif ($keyword eq "tmp_dir") {
	      $type="string";
	      $TmpDir= $keyvalue;
	      ($is_ok, $TmpDir) = &expandEnvVar($TmpDir, $debug);
	  }

	  elsif ($keyword eq "delete_dirs") {
	      $type="comma_delimited_string";
	      ($is_ok, $DeleteDirs) = &checkKeywordValue($keyword, $keyvalue, $type, *DeleteDirsArr, $debug);
	      $save_delete_dirs=$keyvalue;
	  }

	  elsif ($keyword eq "create_dirs") {
	      $type="comma_delimited_string";
	      if ($keyvalue eq "delete_dirs") {
		  $keyvalue=$save_delete_dirs;
	      }
	      ($is_ok, $CreateDirs) = &checkKeywordValue($keyword, $keyvalue, $type, *CreateDirsArr, $debug);
	  }

	  elsif ($keyword eq "rtag") {
	      $type="string";
	      $Rtag = $keyvalue;
	      ($is_ok, $Rtag) = &expandEnvVar($Rtag, $debug);
	      if (($Rtag =~ /\w/) && ($Rtag ne $StringDefault)){
		  $DoRtag=1;
	      } else {
		  $DoRtag=0;
	      }
	  }

	  elsif ($keyword eq "do_build_list") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoBuildList=1;
	      } else {
		  $DoBuildList=0;
	      }
	  }

	  elsif ($keyword eq "my_prebuild_script") {
	      $type="string";
	      $MyPreBuildScript = $unstripped_keyvalue;
	      ($is_ok, $MyPreBuildScript) = &expandEnvVar($MyPreBuildScript, $debug);
	      if (($MyPreBuildScript =~ /\w/) && ($MyPreBuildScript ne $StringDefault)){
		  $DoMyPreBuildScript=1;
	      } else {
		  $DoMyPreBuildScript=0;
	      }
	  }

	  elsif ($keyword eq "my_postbuild_script") {
	      $type="string";
	      $MyPostBuildScript = $unstripped_keyvalue;
	      ($is_ok, $MyPostBuildScript) = &expandEnvVar($MyPostBuildScript, $debug);
	      if (($MyPostBuildScript =~ /\w/) && ($MyPostBuildScript ne $StringDefault)){
		  $DoMyPostBuildScript=1;
	      } else {
		  $DoMyPostBuildScript=0;
	      }
	  }

	  elsif ($keyword eq "my_build_check_script") {
	      $type="string";
	      $MyBuildCheckScript = $unstripped_keyvalue;
	      ($is_ok, $MyBuildCheckScript) = &expandEnvVar($MyBuildCheckScript, $debug);
	      if (($MyBuildCheckScript =~ /\w/) && ($MyBuildCheckScript ne $StringDefault)){
		  $DoMyBuildCheckScript=1;
	      } else {
		  $DoMyBuildCheckScript=0;
	      }
	  }

	  # Parse list entries

	  elsif (($found_be_list) && ($keyword eq "be_dir")) {
	      $type="string";
	      $be_dir = $keyvalue;
	      $be_dir = &expandEnvVar($be_dir, $debug);
	  }

	  elsif (($found_be_list) && ($keyword eq "be_exec")) {
	      $type="string";
	      $be_exec = $unstripped_keyvalue;
	      $be_exec = &expandEnvVar($be_exec, $debug);
	  }

	  else {
	      print(STDERR "ERROR: Cannot parse keyword line: $line");
	  }
	  
	  # End of keyword processing

	  next;
      }
	  
      # Are we inside a build list directive?

      if ($line =~ /\<BUILD_LIST\>/) {
	  $found_build_list=1;
	  next;
      }

      if ($line =~ /\<\\BUILD_LIST\>/) {
	  $found_build_list=0;
      }

      # Are we inside a build entry list?

      if ($line =~ /\<BUILD_ENTRY\>/) {
	  $found_be_list=1;
	  $be_dir=$StringDefault;
	  $be_exec=$StringDefault;
	  next;
      }

      # Update the build array if at the end of a table entry list

      if (($found_be_list) && ($line =~ /\<\\BUILD_ENTRY\>/)) {

	  # Fill the array if there is something in it

	  if ($be_dir !~ /$StringDefault/) {
	      $outBuildArr->[$build_counter][$BE_dir_idx]=$be_dir;
	      $outBuildArr->[$build_counter][$BE_exec_idx]=$be_exec;
	      $build_counter++;
	  }

	  $found_be_list=0;
	  next;
      }

  } #endwhile

  close(PARAM_FILE);

  # Okay, did everything get set???

  ($is_ok, $missing) = &checkAllParams;

  # Set return

  if ($missing > 0) {
      $return_val = 0;
  } else {
      $return_val = 1;
  }

  $nbuild=$build_counter;

  # Debug
  
  if ($dbg2) {
    print(STDERR "$subname: return: $return_val, missing: $missing, nbuild: $nbuild\n");
  }

  # Done
  return($return_val, $nbuild);
}

#------------------------------------------------------------------------
#
# Subroutine generateParamFile
#
# Usage: $return_val = generateParamFile
#
# Function: Generate a parameter file for this script.
#
# Input:    none
#
# Output:   $ret_val             1 on success, 0 on error
#           STDOUT               generates the param file text and writes it
#                                    to STDOUT
#
# Overview:
#

sub generateParamFile
{

  # Local variables

  my ($return_val, $subname);

  # Set defaults

  $return_val=0;
  $subname="generateParamFile";

  # Start writing param stuff to STDOUT

  # ... print header ...

  print(STDOUT "# Parameters for $prog\n");
  print(STDOUT "#\n");
  print(STDOUT "# This param file was generated on: $Today\n");
  print(STDOUT "# Note: environment variables must be delimited with \$\(\)\n");
  print(STDOUT "#=====================================================\n\n");
  
  print(STDOUT "#*************** General params **********************\n\n");

  # ... do checkout ...

  print(STDOUT "#------------ Do checkout ----------------\n");
  print(STDOUT "# do_checkout is a flag to do a CVS checkout\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_checkout=TRUE\n\n");

  # ... checkout dir ...

  print(STDOUT "#------------ Checkout dir ----------------\n");
  print(STDOUT "# checkout_dir is the directory to checkout the project code into.\n");
  print(STDOUT "# It is strongly suggested that you put this in a separate disk\n");
  print(STDOUT "# location than you normally work in so that you can delete the\n");
  print(STDOUT "# project and build files before checking out and rebuilding\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "checkout_dir = \n\n");

  # ... tmp dir ...

  print(STDOUT "#------------ Tmp dir ----------------\n");
  print(STDOUT "# tmp_dir is the directory to use for small temporary files\n");
  print(STDOUT "# used by this script during the build.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "tmp_dir = /tmp\n\n");

  # ... distribs files ...

  print(STDOUT "#------------ Distribs file ----------------\n");
  print(STDOUT "# distribs_files is the CVS paths to the distribs files to checkout\n");
  print(STDOUT "# from CVS and run and checkout the desired project code\n");
  print(STDOUT "# Type: string, must be a comma-separated list of CVS paths to\n");
  print(STDOUT "#       distribs files with no blanks or quotes\n");
  print(STDOUT "#\n");
  print(STDOUT "distribs_files = \n\n");

  # ... dirs to delete ...
  
  print(STDOUT "#------------ Dirs to delete ----------------\n");
  print(STDOUT "# delete_dirs is the directories to delete before starting the\n");
  print(STDOUT "# checkout and build. For clean builds it is suggested that you\n");
  print(STDOUT "# delete the checkout_dir and any target install dirs before starting\n");
  print(STDOUT "# the build. Leave blank to not delete any dirs.\n");
  print(STDOUT "# Type: string, must be a comma-separated list of directories\n");
  print(STDOUT "#       with no blanks or quotes\n");
  print(STDOUT "#\n");
  print(STDOUT "delete_dirs = \n\n");

  # ... dirs to create ...

  print(STDOUT "#------------ Dirs to create ----------------\n");
  print(STDOUT "# create_dirs is the directories to create before starting the\n");
  print(STDOUT "# checkout and build. For clean builds it is suggested that you\n");
  print(STDOUT "# create any needed directories before starting the build.\n");
  print(STDOUT "# Leave blank to not create any dirs\n");
  print(STDOUT "# Type: string, must be a comma-separated list of directories\n");
  print(STDOUT "#       with no blanks or quotes. Set to keyword: delete_dirs\n");
  print(STDOUT "#       to recreate all the delete_dirs after deleting them\n");
  print(STDOUT "#\n");
  print(STDOUT "create_dirs = \n\n");

  # ... rtag ...

  print(STDOUT "#------------ Rtag ----------------\n");
  print(STDOUT "# rtag is the name of a CVS rtag to use when checking out the\n");
  print(STDOUT "# distribs file and all the cvs co commands in the distribs file.\n");
  print(STDOUT "# You must have previously tagged all your desired project code\n");
  print(STDOUT "# with the specified CVS tag. Leave blank to checkout the latest\n");
  print(STDOUT "# code and not use a CVS tag.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "rtag = \n\n");

  print(STDOUT "#*************** Build params **********************\n\n");

  print(STDOUT "#------------ do build list ----------------\n");
  print(STDOUT "# do_build is a flag to build the code in the build list\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_build_list=FALSE\n\n");

  print(STDOUT "#------------- Build list --------------\n");
  print(STDOUT "# The build list contains the list of top-level build project code\n");
  print(STDOUT "# directories and the exec commands to use. Only one build list\n");
  print(STDOUT "# is allowed but may have multiple build entries. The build list\n");
  print(STDOUT "# must begin with a <BUILD_LIST> tag and end with a <\\BUILD_LIST> tag\n");
  print(STDOUT "# Within the build list, each build entry must begin with the <BUILD_ENTRY>\n");
  print(STDOUT "# tag and end with the <\\BUILD_ENTRY> tag. Each tag and each item in the build\n");
  print(STDOUT "# list must be on a separate line. Do not put quotes on the exec commands.\n");
  print(STDOUT "# <BUILD_LIST>\n");
  print(STDOUT "# <BUILD_ENTRY>\n");
  print(STDOUT "# be_dir=(Required) Top level directory to execute the be_exec commands in\n");
  print(STDOUT "# be_exec=(Required) Shell commands to execute in be_dir\n");
  print(STDOUT "# <\\BUILD_ENTRY>\n");
  print(STDOUT "# <\\BUILD_LIST>\n");
  print(STDOUT "#\n");
  print(STDOUT "<BUILD_LIST>\n");
  print(STDOUT "<BUILD_ENTRY>\n");
  print(STDOUT "<\\BUILD_ENTRY>\n");
  print(STDOUT "<\\BUILD_LIST>\n\n");

  print(STDOUT "#*************** External scripts **********************\n\n");

  # ... prebuild ...

  print(STDOUT "#------------ My prebuild script ----------------\n");
  print(STDOUT "# my_prebuild_script is a script to call after the checkout\n");
  print(STDOUT "# but before the build. Leave blank to not run an additional\n");
  print(STDOUT "# script. Do not enclose in quotes, either single or double\n");
  print(STDOUT "# since this causes problems with the system() call\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "my_prebuild_script=\n\n");

  # ... postbuild ...

  print(STDOUT "#------------ My postbuild script ----------------\n");
  print(STDOUT "# my_postbuild_script is a script to call after all the project\n");
  print(STDOUT "# has been built. Leave blank to not run an additional script.\n");
  print(STDOUT "# Do not enclose in quotes, either single or double since this causes\n");
  print(STDOUT "# problems with the system() call\n");
  print(STDOUT "# Type: string.\n");
  print(STDOUT "#\n");
  print(STDOUT "my_postbuild_script = \n\n");

  # ... build check script ...

  print(STDOUT "#------------ Build check script ----------------\n");
  print(STDOUT "# my_build_check_script is a check script to run to check the\n");
  print(STDOUT "# success/failure of the build. Leave blank to not run a check script.\n");
  print(STDOUT "# Do not enclose in quotes, either single or double since this causes\n");
  print(STDOUT "# problems with the system() call\n");
  print(STDOUT "# Type: string.\n");
  print(STDOUT "#\n");
  print(STDOUT "my_build_check_script =\n\n");

  print(STDOUT "#\n");
  print(STDOUT "#-------------------------- End of File ---------------------\n");

  # Done

  $return_val=1;
  return($return_val);
}

#------------------------------------------------------------------------------
#
# Subroutine setAllParams2Defaults
#
# Usage: $return_val = setAllParams2Defaults
#
# Function: Initialize all the params to defaults
#
# Input:    none, all is handled by globals
#
# Output:   none
#
# Overview:
#

sub setAllParams2Defaults
{
  # Local variables

  my($subname);

  # Set defaults

  $subname="setAllParams2Defaults";

  $DoCheckout=0;
  $CheckoutDir=$StringDefault;
  $DistribsFiles=$StringDefault;
  @DistribsFilesArr=("");
  $TmpDir="/tmp";
  $DeleteDirs="";
  @DeleteDirsArr=("");
  $CreateDirs="";
  @CreateDirsArr=("");
  $DoRtag=0;
  $Rtag=$StringDefault;
  $DoBuildList=0;

  $MyPreBuildScript=$StringDefault;
  $DoMyPreBuildScript=0;
  $MyPostBuildScript=$StringDefault;
  $DoMyPostBuildScript=0;
  $MyBuildCheckScript=$StringDefault;
  $DoMyBuildCheckScript=0;

  # Done

  return;
}

#------------------------------------------------------------------------------
#
# Subroutine checkAllParams
#
# Usage: ($return_val, $nmissing) = checkAllParams
#
# Function: Check that all the params are set. Return the number missing.
#
# Input:    none, all is handled by globals
#
# Output:   $ret_val             1 on success, 0 on error
#           $nmissing            number of params not set
#
# Overview:
#

sub checkAllParams
{
  # Local variables

  my ($return_val, $subname);
  my ($missing, $num);

  # Set defaults

  $return_val=0;
  $subname="checkAllParams";
  $missing=0;

  # Do it

  if ($CheckoutDir eq $StringDefault) {
      print(STDERR "WARNING: checkout_dir not set\n");
      $missing++;
  }
  if ($DistribsFiles eq $StringDefault) {
      print(STDERR "WARNING: distribs_file not set\n");
      $missing++;
  }
  # Done

  $return_val=1;
  return($return_val, $missing);
}


#------------------------------------------------------------------------------
#
# Subroutine printAllParams
#
# Usage: printAllParams
#
# Function: Print all the global params
#
# Input:    none, all is handled by globals
#
# Output:   none
#
# Overview:
#

sub printAllParams
{
  # Local variables

    my ($subname, $dir, $i);

  # Set defaults

    $subname="printAllParams";

  # Print all the params

    print(STDERR "Printing all global params...\n");
    print(STDERR "\tdo_checkout: $DoCheckout\n");
    print(STDERR "\tcheckout_dir: $CheckoutDir\n");
    print(STDERR "\ttmp_dir: $TmpDir\n");
    print(STDERR "\tdistribs_files: $DistribsFiles\n");
    if ($Debug_level > 1) {
	foreach $dir (@DistribsFilesArr) {
	    print(STDERR "\t\tdistribs_file_arr: $dir\n");
	}
    }
    print(STDERR "\tdelete_dirs: $DeleteDirs\n");
    if ($Debug_level > 1) {
	foreach $dir (@DeleteDirsArr) {
	    print(STDERR "\t\tdelete_dir: $dir\n");
	}
    }
    print(STDERR "\tcreate_dirs: $CreateDirs\n");
    if ($Debug_level > 1) {
	foreach $dir (@CreateDirsArr) {
	    print(STDERR "\t\tcreate_dir: $dir\n");
	}
    }
    print(STDERR "\tdo_rtag: $DoRtag\n");
    print(STDERR "\trtag: $Rtag\n");

    print(STDERR "\tdo_build_list: $DoBuildList\n");
    print(STDERR "\tBuild list, number of entries: $nbuildarr\n");
    for ($i=0; $i<$nbuildarr; $i++) {
	print(STDERR "\t\tbuild dir: $buildArr[$i][$BE_dir_idx], exec: $buildArr[$i][$BE_exec_idx]\n");
    }

    print(STDERR "\tDoMyPreBuildScript: $DoMyPreBuildScript\n");
    print(STDERR "\tmy_prebuild_script: $MyPreBuildScript\n");
    print(STDERR "\tDoMyPostBuildScript: $DoMyPostBuildScript\n");
    print(STDERR "\tmy_postbuild_script: $MyPostBuildScript\n");
    print(STDERR "\tDoMyBuildCheckScript: $DoMyBuildCheckScript\n");
    print(STDERR "\tmy_build_check_script: $MyBuildCheckScript\n");
  
  # Done

    return;
}



#========================================= EOF =====================================
