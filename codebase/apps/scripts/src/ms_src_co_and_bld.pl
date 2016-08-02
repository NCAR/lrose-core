#!/usr/bin/perl -w
#
# Name: ms_src_co_and_bld.pl
#
# Function:
#	Perl script to checkout the Modeling System source code
#       from CVS and build it.
#
# Overview:
#       Checks out a distribs file from CVS and then executes it
#       to checkout a set of source code. Then does a series of
#       make commands on the checked out code. Modifies the MM5
#       configure.user file if specified before doing MM5 make
#       commands.
#
# Usage:
#       ms_src_co_and_bld.pl
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
# Author: Deirdre Garvey - NCAR/RAP/SDG        29-MAR-2007 from a similar
#                                              script "checkout_and_build.pl"
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
our $BE_make_idx=1;
our $BE_flag_check_for_pgi_lm_problem_idx=2;
our $BE_flag_start_str_idx=3;
our $BE_flag_end_str_idx=4;
our $NP_ns_idx=0;
our $NP_ew_idx=1;

# Set enums
#   MM5ConfigureUser_MemOptionArr is used in the param file and in
#       selecting which section of the configure.user file to comment/uncomment

our @MM5ConfigureUser_MemOptionArr=("MPP", "NOTMPP", "NONE");

# Get environment variables. Most are just to print to log for reference.
# Comment this out as it adds unneccesary dependencies

##our $env_make_macros_dir=$ENV{MS_MAKE_MACROS_DIR};
##our $env_os_type=$ENV{MS_OS_TYPE};
##our $env_pgi=$ENV{PGI};
##our $env_ncarg_root=$ENV{NCARG_ROOT};
##our $env_mm5_home=$ENV{MM5HOME};
##our $env_executable_archive=$ENV{EXECUTABLE_ARCHIVE};

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
    "Usage: $prog -p|-c <file> [-dht] [-v <level>]\n" .
    "Purpose: To checkout and build Modeling System source code.\n" .
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
our ($opt_help, $opt_config, $opt_debug, $opt_print_params);
our ($opt_test, $opt_verbose);

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
our($DoRtag, $Rtag, $DoBuildList, $MaxFlexLmRetries);
our($DoWrfBuild, $WrfBuildCmd);
our($DoMM5Build, $DoMM5PreBuildMakeCommands, $MM5PreBuildMakeCommands);
our($MM5MakeCommands, $MM5DestDir);
our($DoMM5CheckForPgiLmProblems);
our($MM5SrcDir, $MM5ConfigureUserFile, $MM5ConfigureUserFilenameOnly);
our($SaveTmpMM5ConfigureUserFile, $DoSaveTmpMM5ConfigureUserFile);
our($DoModifyMM5ConfigureUser, $MM5ConfigureUser_MemOption);
our($MM5ConfigureUser_RuntimeSystem, $MM5ConfigureUser_UseRuntimeSystem);
our($MM5ConfigureUser_Mkx, $DoSetMM5ConfigureUser_Mkx);
our($MM5ConfigureUser_Mix, $DoSetMM5ConfigureUser_Mix);
our($MM5ConfigureUser_Mjx, $DoSetMM5ConfigureUser_Mjx);
our($MM5ConfigureUser_Maxnes, $DoSetMM5ConfigureUser_Maxnes);
our($DoMM5ConfigureUser_ModSuffix);
our($DoMM5PolarBuild, $MM5PolarSrcDir);
our($MM5PolarConfigureUser_Ipolar, $DoSetMM5PolarConfigureUser_Ipolar);
our($MM5PolarConfigureUser_Isoil, $DoSetMM5PolarConfigureUser_Isoil);
our($MM5PolarConfigureUser_Ibltyp, $DoSetMM5PolarConfigureUser_Ibltyp);

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

# Print banner.
# Comment out the Environment section because this added unnecessary
# dependencies in this script

print(STDERR "*******************************************************************\n");
print(STDERR "Running $prog\n");
print(STDERR "\tHostname: ${HostName}\n");
print(STDERR "\tg++ version: $gcpp_version\n");
print(STDERR "\tStart time: ${Today}\n");
##print(STDERR "\tEnvironment:\n");
##print(STDERR "\t\tMS_MAKE_MACROS_DIR: $env_make_macros_dir\n");
##print(STDERR "\t\tMS_OS_TYPE: $env_os_type\n");
##print(STDERR "\t\tPGI: $env_pgi\n");
##print(STDERR "\t\tNCARG_ROOT: $env_ncarg_root\n");
##print(STDERR "\t\tMM5HOME: $env_mm5_home\n");
##print(STDERR "\t\tEXECUTABLE_ARCHIVE: $env_executable_archive\n");
print(STDERR "*******************************************************************\n");

# Read the config/param file and set globals

our $nbuildarr;
our $nnprocarr;
our $polar_nnprocarr;
our @buildArr;
our @nprocArr;
our @polar_nprocArr;
our $is_polar;

($is_ok, $nbuildarr, $nnprocarr, $polar_nnprocarr) = &readParamFile($ConfigFile, \@buildArr, 
								    \@nprocArr, \@polar_nprocArr,
								    $Debug_level);


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

# Additional pre-build stuff

&preBuildSetup($Test, $Debug_level);

# WRF

if ($DoWrfBuild) {
    $is_ok=&runExternalScript($WrfBuildCmd, $Test, $Debug_level);
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

# MM5

if ($DoMM5Build) {
    $is_polar=0;
    $is_ok=&buildMM5($is_polar, $Test, $Debug_level);
    if (!$is_ok) {
	print(STDERR "Exiting...\n");
	exit $ExitFailure;
    }
}

if ($DoMM5PolarBuild) {
    $is_polar=1;
    $is_ok=&buildMM5($is_polar, $Test, $Debug_level);
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

# Post build

if ($DoMyPostBuildScript) {
    $is_ok=&runExternalScript($MyPostBuildScript, $Test, $Debug_level);
    if (!$is_ok) {
	print(STDERR "Exiting...\n");
	exit $ExitFailure;
    }
}

# Get the processing end time for statistics

my $end_utime=time;
my $elapsed_utime=$end_utime - $start_utime;
$Today=`date`;

print(STDERR "*******************************************************************\n");
print(STDERR "Done running $prog\n");
print(STDERR "End time: $Today");
print(STDERR "Elapsed time: $elapsed_utime secs\n");

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
  my($is_ok, $cmd_ok, $i, $dir, $make_cmds, $flag, $flag_start_str, $flag_end_str);
  my($tmp_file, $use_cmd);

  # Set defaults

  $return_val=0;
  $subname="build";
  $tmp_file="${TmpDir}/build.log";

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
      $make_cmds=$buildArr[$i][$BE_make_idx];
      $flag=$buildArr[$i][$BE_flag_check_for_pgi_lm_problem_idx];
      $flag_start_str=$buildArr[$i][$BE_flag_start_str_idx];
      $flag_end_str=$buildArr[$i][$BE_flag_end_str_idx];

      if ($debug) {
	  print(STDERR "======= making src dir: $dir, with: $make_cmds =======\n");
      }

      $use_cmd=$make_cmds;
      if ($flag) {
	  $use_cmd="${make_cmds} | tee $tmp_file";
      }

      $use_cmd="cd $dir; $use_cmd";
      ($is_ok, $cmd_ok)=execSystemCall($use_cmd, $test, $debug);

      # Deal with PGI compiler problems

      if ($flag) {
	  $is_ok=doPgiLmProblem($tmp_file, $flag_start_str, $flag_end_str, 
				$dir, $make_cmds, $test, $debug);
      }

  } #endfor

  unlink($tmp_file);

  # Done
  
  $return_val=1;
  return($return_val);
}

#-------------------------------------------------------------------------
#
# Subroutine buildMM5
#
# Usage: $return_val = buildMM5($polar_flag, $test, $debug)
#
# Function: build and install MM5
#
# Input:    globals
#           $polar_flag     0 not polar, 1 is polar
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub buildMM5
{
  my ($polar_flag, $test, $debug) = @_;

  # Local variables

  my($subname, $return_val);
  my($dbg2, $dbg3);
  my($is_ok, $cmd_ok, $cmd, $i, $np_ns, $np_ew, $num_proc, $line, $tmp_file);
  my($tmp_log_file, $use_cmd, $save_file, $target_file, $mod_file, $now);
  my($moved_file, $use_src_dir, $use_nnprocarr, @use_nproc_arr, $use_mod_file);
  my($use_ext, $use_save_file, $saved_orig_file, $use_saved_orig_file, $use_target_file);
  my($output_exe, $use_moved_file, $use_orig_file);

  # Set defaults

  $return_val=0;
  $subname="buildMM5";

  $now=time();
  $tmp_log_file="${TmpDir}/mm5build.log";
  $save_file="configure.user.SAVE.${now}";
  $mod_file="configure.user.modified.${now}";
  $target_file="configure.user";
  $moved_file="${MM5ConfigureUserFilenameOnly}.moved_out_of_the_way.${now}";
  $saved_orig_file="${MM5ConfigureUserFilenameOnly}.ORIG.${now}";

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

  # Set src dir and other things for polar vs non-polar

  if ($polar_flag) {
      $use_src_dir=$MM5PolarSrcDir;
      $use_ext=".polar";
      $use_nnprocarr=$polar_nnprocarr;
      for ($i=0; $i<$use_nnprocarr; $i++) {
	  $use_nproc_arr[$i][$NP_ns_idx]=$polar_nprocArr[$i][$NP_ns_idx];
	  $use_nproc_arr[$i][$NP_ew_idx]=$polar_nprocArr[$i][$NP_ew_idx];
      }
  } else {
      $use_src_dir=$MM5SrcDir;
      $use_ext="";
      $use_nnprocarr=$nnprocarr;
      for ($i=0; $i<$use_nnprocarr; $i++) {
	  $use_nproc_arr[$i][$NP_ns_idx]=$nprocArr[$i][$NP_ns_idx];
	  $use_nproc_arr[$i][$NP_ew_idx]=$nprocArr[$i][$NP_ew_idx];
      }
  }
  
  $output_exe="${use_src_dir}/Run/mm5.mpp";
  $use_save_file="${use_src_dir}/${save_file}";
  $use_mod_file="${use_src_dir}/${mod_file}";
  $use_target_file="${use_src_dir}/${target_file}";
  $use_moved_file="${use_src_dir}/${moved_file}";
  $use_orig_file="${use_src_dir}/${MM5ConfigureUserFilenameOnly}";
  $use_saved_orig_file="${use_src_dir}/${saved_orig_file}";

  # Return if no src dir or configure.user file defined
  
  if ((!$test) && (!-d $use_src_dir)) {
      print(STDERR "ERROR: $subname: source dir does not exist: $use_src_dir\n");
      return($return_val);
  }

  if ((!$test) && (!-e "$MM5ConfigureUserFile")) {
      print(STDERR "ERROR: $subname: MM5 configure.user file does not exist: $MM5ConfigureUserFile\n");
      return($return_val);
  }

  if ($debug) {
      print (STDERR "========== compiling MM5 in $use_src_dir with $MM5ConfigureUserFile =======\n");
  }
  
  # Save a copy of the input configure.user file (may not be its name)
  # so can restore it at the end

  if (-f $use_orig_file) {
      $cmd="cp -p $use_orig_file $use_saved_orig_file";
      ($is_ok, $cmd_ok)=execSystemCall($cmd, 0, $debug);
  }
  
  # If a file actually named configure.user exists, save it so can restore at the end

  if (-f $use_target_file) {
      if ($dbg2) {
	  print(STDERR "$subname: A file named configure.user exists\n");
	  print(STDERR "\tWill move configure.user to $moved_file temporarily\n");
      }
      $cmd="cp -p $use_target_file $use_moved_file";
      ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);
  }
  
  # Save the MM5 configure.user file into a saved name in the src dir
  
  $cmd="cp -p $MM5ConfigureUserFile $use_save_file";
  ($is_ok, $cmd_ok)=execSystemCall($cmd, 0, $debug);

  # cd to the src dir

  chdir ($use_src_dir);

  # Create the configure.user file to use, make needed modifications.
  # The file to use from here on out is the use_mod_file

  $is_ok=modifyMM5ConfigureUser($use_save_file, $use_mod_file, $polar_flag, $test, $debug);

  # Run pre build make commands if set

  if ($DoMM5PreBuildMakeCommands) {
      if ($debug) {
	  print(STDERR "======= running $MM5PreBuildMakeCommands\n");
      }

      $cmd="cd $use_src_dir; $MM5PreBuildMakeCommands";
      ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);
  }

  # Do the builds for the various number of processors

  chdir ($use_src_dir);
  for ($i=0; $i<$use_nnprocarr; $i++) {
      $np_ns=$use_nproc_arr[$i][$NP_ns_idx];
      $np_ew=$use_nproc_arr[$i][$NP_ew_idx];
      $num_proc=$np_ns * $np_ew;

      $tmp_file="${use_mod_file}.${num_proc}";

      if ($debug) {
	  print(STDERR "======= creating configure.user for num_proc: $num_proc, NS: $np_ns, EW: $np_ew\n");
      }
      
      # Edit the configure.user file to change the number of processors
      
      $is_ok=open(IN_CONFIGURE_USER_FILE, "${use_mod_file}");
      $is_ok=open(OUT_CONFIGURE_USER_FILE, "> $tmp_file");

      while ($line = <IN_CONFIGURE_USER_FILE>) {

	  # Look for lines with PROCMIN_NS or EW in them, skip comments

	  if (($line !~ /^\#/) && ($line =~ /PROCMIN_NS/)) {
	      $line="PROCMIN_NS = $np_ns\n";
	  }

	  elsif (($line !~ /^\#/) && ($line =~ /PROCMIN_EW/)) {
	      $line="PROCMIN_EW = $np_ew\n";
	  }

	  print (OUT_CONFIGURE_USER_FILE $line);
      }

      close(IN_CONFIGURE_USER_FILE);
      close(OUT_CONFIGURE_USER_FILE);

      # Need the modified tmp file to be named correctly

      $cmd="cp -p $tmp_file $use_target_file";
      ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);

      # Run the make command

      if ($debug) {
	  print(STDERR "======= running $MM5MakeCommands for num_proc: $num_proc =======\n");
      }

      $cmd="cd $use_src_dir; $MM5MakeCommands";
      ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);
      
# -------------- note, can skip this tee if have the suffix rules modified ---------------
#
#      if ($DoMM5CheckForPgiLmProblems) {
#	  $use_cmd="${cmd} | tee $tmp_log_file";
#      } else {
#	  $use_cmd=$cmd;
#      }
#
#      ($is_ok, $cmd_ok)=execSystemCall($use_cmd, $test, $debug);
#
#      if ($DoMM5CheckForPgiLmProblems) {
#	  $is_ok=doPgiLmProblem($tmp_log_file, $StringDefault, $StringDefault,
#				$MM5SrcDir, $cmd, $test, $debug);
#      }
#
# ----------------------------------------------------------------------------------------

      # Copy the output to the destination directory with the number of processors
      # in the output name

      if (-e $output_exe) {
	  if ($debug) {
	      print(STDERR "======= Copy ${output_exe} to ${MM5DestDir}/mm5.mpich${num_proc}${use_ext} =======\n");
	  }
	  $cmd="cp -p ${use_src_dir}/Run/mm5.mpp $MM5DestDir/mm5.mpich${num_proc}${use_ext}";
	  ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);
      } else {
	  print(STDERR "ERROR: Failed to create $output_exe for num_proc: $num_proc\n");
      }
  } #endfor

  # Copy the modified temp file to the save name if specified

  if ($DoSaveTmpMM5ConfigureUserFile > 0) {
      $cmd="cp -p $use_mod_file ${SaveTmpMM5ConfigureUserFile}${use_ext}";
      ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);
  }

  # Restore files moved at the start

  if ($debug) {
      print(STDERR "======= restore the $MM5ConfigureUserFile =======\n");
  }

  if (-f $moved_file) {
      if ($dbg2) {
	  print(STDERR "======= restore the $use_target_file =======\n");
      }
      $cmd="cp -p ${moved_file} ${use_target_file}";
      ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);
  }

  if (-f $use_saved_orig_file) {
      if ($dbg2) {
	  print(STDERR "======= restore the $use_orig_file =======\n");
      }
      $cmd="cp -p ${use_saved_orig_file} ${use_orig_file}";
      ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);
  }

  # Cleanup

  unlink($tmp_file);
  unlink($tmp_log_file);
  unlink($use_moved_file);
  unlink($use_save_file);
  unlink($use_saved_orig_file);
  $cmd="rm -f ${use_mod_file}*";
  ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);

  # Done
  
  $return_val=1;
  return($return_val);

}

#-------------------------------------------------------------------------
#
# Subroutine modifyMM5ConfigureUser
#
# Usage: $return_val = modifyMM5ConfigureUser($infile, $outfile, $polar_flag, $test, $debug)
#
# Function: Modify the MM5 configure.user file for various param file
#           options
#
# Input:    globals
#           $infile         input configure.user file to read
#           $outfile        modified configure.user file
#           $polar_flag     1 for polar, 0 for not
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub modifyMM5ConfigureUser
{
  my ($infile, $outfile, $polar_flag, $test, $debug) = @_;

  # Local variables

  my($subname, $return_val);
  my($dbg2, $dbg3);
  my($cmd, $is_ok, $cmd_ok, $line, $save_line);
  my($found_sect_5, $found_break, $found_suffix, $found_sect_6);
  my($found_sect_3, $found_sect_4, $found_sect_7, $found_wanted_section, $match);

  # Set defaults

  $return_val=0;
  $subname="modifyMM5ConfigureUser";

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

  if ($dbg2) {
      print(STDERR "$subname: Inputs...\n");
      print(STDERR "\tinfile: $infile, outfile: $outfile, polar_flag: $polar_flag\n");
  }
  
  # Determine if doing ANY modifications to the file. If no mods, just copy the file
  # and return
  
  if ($DoModifyMM5ConfigureUser < 1) {
      if ($debug) {
	  print(STDERR "$subname: No mods needed to configure.user\n");
      }
      $cmd="cp -p $infile $outfile";
      ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);
      return($is_ok);
  }

  # Open the input and output files

  $is_ok=open(IN_CONFIGURE_USER_FILE, "$infile");
  if (!$is_ok) {
      print(STDERR "ERROR: $subname: Cannot open file: $infile\n");
      return($return_val);
  }

  $is_ok=open(OUT_CONFIGURE_USER_FILE, "> $outfile");
  if (!$is_ok) {
      print(STDERR "ERROR: $subname: Cannot open file: $outfile\n");
      return($return_val);
  }

  # Set some defaults before begin loop

  $found_sect_3=0;
  $found_sect_4=0;
  $found_sect_5=0;
  $found_sect_6=0;
  $found_sect_7=0;
  $found_suffix=0;
  $found_wanted_section=0;
  $found_break=0;

  # Go through the file, parsing for needed items

  while ($line = <IN_CONFIGURE_USER_FILE>) {

      $save_line=$line;

      if ($dbg3) {
	  print(STDERR "$subname: input line: $line");
      }

      # Look for string of dashes, these indicate breaks between sections

      if (($line =~ /^\#/) && ($line =~ /--------------------/)) {
	  if ($dbg3) {
	      print(STDERR "$subname: found break: line: $line");
	  }
	  $found_break=1;
      } else {
	  $found_break=0;
      }

      # Found Section 3

      if (($found_sect_3 == 0) && ($found_sect_5 == 0) && ($found_sect_6 == 0) && 
	  ($found_sect_7 == 0) && ($line =~ /^\#*\s*RUNTIME_SYSTEM/)) {
	  $found_sect_3=1;
      }

      # Found Section 4

      if (($found_sect_4 == 0) && (($line =~ /^AR/) || ($line =~ /^RM/) || ($line =~ /^GREP/))) {
	  $found_sect_4=1;
	  $found_sect_3=2;
      }

      # Found Section 5

      if (($found_sect_5 == 0) && (($line =~ /MKX/) || ($line =~ /MIX/) || ($line =~ /MJX/) || ($line =~ /MAXNES/))) {
	  $found_sect_5=1;
	  $found_sect_4=2;
      }

      # Found Section 6

      if (($found_sect_6 == 0) && ($line =~ /IMPHYS/)) {
	  $found_sect_6=1;
	  $found_sect_5=2;
      }

      # Found Section 7

      if (($found_sect_7 == 0) && ($found_sect_3 == 2) && ($found_sect_5 == 2) &&
	  ($found_sect_6 > 0) && ($line =~ /RUNTIME_SYSTEM/)) {
	  $found_sect_7=1;
	  $found_sect_6=2;
      }

      # Found suffix rules

      if (($found_suffix == 0) && ($found_sect_7 > 0) && ($found_sect_5 > 0) && ($line =~ /^\./)) {
	  $found_suffix=1;
	  $found_sect_7=2;
      }

      # Do Section 5. Comment out existing lines.

      if ($found_sect_5 == 1) {
	  if ($dbg3) {
	      print(STDERR "$subname: in section 5, line: $line");
	  }

	  if (($DoSetMM5ConfigureUser_Mkx) && ($line =~ /MKX/) && ($line !~ /^\#/)) {
	      $line="#-- $line";
	  }
	  
	  if (($DoSetMM5ConfigureUser_Mix) && ($line =~ /MIX/) && ($line !~ /^\#/)) {
	      $line="#-- $line";
	  }

	  if (($DoSetMM5ConfigureUser_Mjx) && ($line =~ /MJX/) && ($line !~ /^\#/)) {
	      $line="#-- $line";
	  }

	  if (($DoSetMM5ConfigureUser_Maxnes) && ($line =~ /MAXNES/) && ($line !~ /^\#/)) {
	      $line="#-- $line";
	  }
      }

      # Print all the section 5 settings before the break line to section 6

      if (($found_sect_5 == 1) && ($found_break)) {
	  if ($dbg3) {
	      print(STDERR "$subname: Print section 5 lines...\n");
	  }
	  $line="#-- Next lines are from param file: $ConfigFile\n";
	  print(OUT_CONFIGURE_USER_FILE $line);
	  if ($DoSetMM5ConfigureUser_Mkx) {
	      $line="MKX=$MM5ConfigureUser_Mkx\n";
	      print(OUT_CONFIGURE_USER_FILE $line);
	  }
	  if ($DoSetMM5ConfigureUser_Mix) {
	      $line="MIX=$MM5ConfigureUser_Mix\n";
	      print(OUT_CONFIGURE_USER_FILE $line);
	  }
	  if ($DoSetMM5ConfigureUser_Mjx) {
	      $line="MJX=$MM5ConfigureUser_Mjx\n";
	      print(OUT_CONFIGURE_USER_FILE $line);
	  }
	  if ($DoSetMM5ConfigureUser_Maxnes) {
	      $line="MAXNES=$MM5ConfigureUser_Maxnes\n";
	      print(OUT_CONFIGURE_USER_FILE $line);
	  }
	  $line=$save_line;
	  $found_sect_5=2;
      }

      # Do Section 6. Comment out existing lines.

      if (($found_sect_6 == 1) && ($polar_flag)) {
	  if ($dbg3) {
	      print(STDERR "$subname: in section 6, line: $line");
	  }

	  if (($DoSetMM5PolarConfigureUser_Ipolar) && ($line =~ /IPOLAR/) && ($line !~ /^\#/)) {
	      $line="#-- $line";
	  }
	  
	  if (($DoSetMM5PolarConfigureUser_Isoil) && ($line =~ /ISOIL/) && ($line !~ /^\#/)) {
	      $line="#-- $line";
	  }

	  if (($DoSetMM5PolarConfigureUser_Ibltyp) && ($line =~ /IBLTYP/) && ($line !~ /^\#/)) {
	      $line="#-- $line";
	  }
      }

      # Print all the section 6 settings before the break line to section 6

      if (($found_sect_6 == 1) && ($polar_flag) && ($found_break)) {
	  if ($dbg3) {
	      print(STDERR "$subname: Print section 6 lines...\n");
	  }
	  $line="#-- Next lines are from param file: $ConfigFile\n";
	  print(OUT_CONFIGURE_USER_FILE $line);
	  if ($DoSetMM5PolarConfigureUser_Ipolar) {
	      $line="IPOLAR=$MM5PolarConfigureUser_Ipolar\n";
	      print(OUT_CONFIGURE_USER_FILE $line);
	  }
	  if ($DoSetMM5PolarConfigureUser_Isoil) {
	      $line="ISOIL=$MM5PolarConfigureUser_Isoil\n";
	      print(OUT_CONFIGURE_USER_FILE $line);
	  }
	  if ($DoSetMM5PolarConfigureUser_Ibltyp) {
	      $line="IBLTYP=$MM5PolarConfigureUser_Ibltyp\n";
	      print(OUT_CONFIGURE_USER_FILE $line);
	  }
	  $line=$save_line;
	  $found_sect_6=2;
      }

      # Do Section 3. Non-MPP. Need to uncomment needed RUNTIME SYSTEM and
      # comment out all others

      if ($found_sect_3 == 1) {
	  if ($dbg3) {
	      print(STDERR "$subname: in section 3, line: $line");
	  }
	  if ($MM5ConfigureUser_MemOption =~ /NOTMPP/) {
	      ($is_ok, $match)=matchMM5RuntimeSystem($line, $debug);
	      if ($match) {
		  $found_wanted_section=1;
		  ($is_ok, $line)=replaceMM5RuntimeSystem($line, $debug);
	      }

	      if (($found_wanted_section) && ($found_break)) {
		  $found_wanted_section=0;
	      }

	      if ($dbg3) {
		  print(STDERR "$subname: in section 3, found_wanted_section: $found_wanted_section\n");
	      }

	      if (($found_wanted_section) && ($line =~ /^\#/)) {
		  $line=substr($line, 1);
	      }

	      if ((!$found_wanted_section) && ($line !~ /^\#/)) {
		  $line="#-- $line";
	      }
	  } elsif (($MM5ConfigureUser_MemOption =~ /^MPP/) && ($line !~ /^\#/)) {
	      $line="#-- $line";
	      if ($dbg3) {
		  print(STDERR "$subname: in section 3, comment out all if MPP: line: $line\n");
	      }
	  }
      }

      # Do Section 7. MPP. Need to uncomment needed RUNTIME SYSTEM and
      # comment out all others.

      if (($MM5ConfigureUser_MemOption =~ /^MPP/) && ($found_sect_7 == 1)) {
	  if ($dbg3) {
	      print(STDERR "$subname: in section 7, line: $line");
	  }

	  ($is_ok, $match)=matchMM5RuntimeSystem($line, $debug);
	  if ($match) {
	      $found_wanted_section=1;
	      ($is_ok, $line)=replaceMM5RuntimeSystem($line, $debug);
	  }

	  if (($found_wanted_section) && ($found_break)) {
	      $found_wanted_section=0;
	  }
	  if ($dbg3) {
	      print(STDERR "$subname: in section 7, found_wanted_section: $found_wanted_section\n");
	  }
	  if (($found_wanted_section) && ($line =~ /^\#/) && ($line =~ /\=/)) {
	      $line=substr($line, 1);
	  }

	  if ((!$found_wanted_section) && ($line !~ /^\#/)) {
	      $line="#-- $line";
	  }
      }

      # Modify the suffix rules to add the loop retries after the RM line

      if (($DoMM5ConfigureUser_ModSuffix) && ($found_suffix > 0)) {
	  if ($found_suffix == 1) {
	      if ($dbg3) {
		  print(STDERR "$subname: suffix rules, add ntries and sleep secs, line: $line");
	      }
	      print(OUT_CONFIGURE_USER_FILE "COMPILE_MAX_NTRIES=$MaxFlexLmRetries\n");
	      print(OUT_CONFIGURE_USER_FILE "COMPILE_SLEEP_SECS=10\n");
	      $found_suffix=2;
	  }

	  if (($line =~ /^\t\$\(/) && ($line !~ /RM/)) {
	      if ($dbg3) {
		  print(STDERR "$subname: in suffix rules, add retries section: line $line");
	      }
	      print(OUT_CONFIGURE_USER_FILE "\tcount=1 \;\\\n");
	      print(OUT_CONFIGURE_USER_FILE "\twhile \[ \$\$\{count\} \-le \$\{COMPILE_MAX_NTRIES\} \] \;\\\n");
	      print(OUT_CONFIGURE_USER_FILE "\tdo \\\n");
	      chop($line);
	      print(OUT_CONFIGURE_USER_FILE "\t$line \;\\\n");
	      print(OUT_CONFIGURE_USER_FILE "\t\tif \[ \-e \$\@ \] \;\\\n");
	      print(OUT_CONFIGURE_USER_FILE "\t\tthen \\\n");
	      print(OUT_CONFIGURE_USER_FILE "\t\t\tbreak \;\\\n");
	      print(OUT_CONFIGURE_USER_FILE "\t\telse \\\n");
	      print(OUT_CONFIGURE_USER_FILE "\t\t\tsleep \$\{COMPILE_SLEEP_SECS\} \;\\\n");
	      print(OUT_CONFIGURE_USER_FILE "\t\t\tcount\=\`expr \$\$\{count\} \+ 1\` \;\\\n");
	      print(OUT_CONFIGURE_USER_FILE "\t\tfi \;\\\n");
	      print(OUT_CONFIGURE_USER_FILE "\tdone\n");
	      next;
	  }
      }

      # Print the line to the output file

      if ($dbg3) {
	  print(STDERR "$subname: output line: $line");
      }

      print (OUT_CONFIGURE_USER_FILE $line);
  }

  # Close the files

  close(IN_CONFIGURE_USER_FILE);
  close(OUT_CONFIGURE_USER_FILE);

  # Done

  $return_val=1;
  return($return_val);
}

#-------------------------------------------------------------------------
#
# Subroutine readParamFile
#
# Usage: ($return_val, $nbuild, $nnproc, $polar_nnproc) =
#       readParamFile($param_file, *outBuildArr, *outNprocArr, *outPolarNprocArr, $debug)
#
# Function: Read the $param_file.
#
# Input:    $param_file          file to read
#           $debug               flag for debugging (1=on, 0=off)
#
# Output:   $ret_val             1 on success, 0 on error
#           $nbuild              size of *outBuildArr
#           $nnproc              size of *outNprocArr
#           $polcar_nnproc       size of *outPolarNprocArr
#           *outBuildArr         array of build entries
#	         [n][$BE_dir_idx]  = build directory
#                [n][$BE_make_idx] = build make commmands
#                [n][$BE_flag_check_for_pgi_lm_problem_idx] = flag to check compiles
#                                                             for PGI LM problems
#                [n][$BE_flag_start_str_idx]   = string to bound start of block
#                [n][$BE_flag_end_str_idx]     = string to bound end of block 
#           *outNprocArr         array of MM5 num proc entries
#	         [n][$NP_ns_idx]   = nproc NS
#	         [n][$NP_ew_idx]   = nproc EW
#           *outPolarNprocArr    array of MM5 polar num proc entries
#	         [n][$NP_ns_idx]   = nproc NS
#	         [n][$NP_ew_idx]   = nproc EW
#
# Overview:
#

sub readParamFile
{
  my ($param_file, $outBuildArr, $outNprocArr, $outPolarNprocArr, $debug) = @_;

  # Local variables

  my($return_val, $subname);
  my($dbg2, $dbg3);
  my($nbuild, $nnproc, $polar_nnproc);
  my($nproc_counter, $build_counter, $nproc_polar_counter);
  my($is_ok, $keyword, $keyvalue, $type, $unstripped_keyvalue);
  my($save_delete_dirs, $found_nproc_list, $found_np_list, $found_nproc_polar_list);
  my($table_counter, $found_build_list, $found_be_list);
  my($line, $be_dir, $be_make, $np_ns, $np_ew, $missing);
  my(@junk, $firstequals, $len, $be_flag, $be_flag_start_str, $be_flag_end_str);
  my($local_found, $local_type, $pos);

  # Set defaults

  $return_val=0;
  $subname="readParamFile";
  $nbuild=0;
  $nnproc=0;

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
      return($return_val, $nbuild, $nnproc);
  }

  # Set loop defaults

  $found_build_list=0;
  $found_be_list=0;
  $found_nproc_list=0;
  $found_np_list=0;
  $nproc_counter=0;
  $nproc_polar_counter=0;
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

	  elsif ($keyword eq "max_flexlm_retries") {
	      $type="int";
	      ($is_ok, $MaxFlexLmRetries) = &checkKeywordValue($keyword, $keyvalue, $type, @junk, $debug);
	  }

	  elsif ($keyword eq "do_build_list") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoBuildList=1;
	      } else {
		  $DoBuildList=0;
	      }
	  }

	  elsif ($keyword eq "do_wrf_build") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoWrfBuild=1;
	      } else {
		  $DoWrfBuild=0;
	      }
	  }

	  elsif ($keyword eq "wrf_build_cmd") {
	      $type="string";
	      $WrfBuildCmd = $unstripped_keyvalue;
	      ($is_ok, $WrfBuildCmd) = &expandEnvVar($WrfBuildCmd, $debug);
	  }

	  elsif ($keyword eq "do_mm5_build") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoMM5Build=1;
	      } else {
		  $DoMM5Build=0;
	      }
	  }

	  elsif ($keyword eq "do_mm5_check_for_pgi_lm_problems") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoMM5CheckForPgiLmProblems=1;
	      } else {
		  $DoMM5CheckForPgiLmProblems=0;
	      }
	  }

	  elsif ($keyword eq "mm5_src_dir") {
	      $type="string";
	      $MM5SrcDir= $keyvalue;
	      ($is_ok, $MM5SrcDir) = &expandEnvVar($MM5SrcDir, $debug);
	  }

	  elsif ($keyword eq "mm5_configure_user_file") {
	      $type="string";
	      $MM5ConfigureUserFile= $keyvalue;
	      ($is_ok, $MM5ConfigureUserFile) = &expandEnvVar($MM5ConfigureUserFile, $debug);
	  }

	  elsif ($keyword eq "save_tmp_mm5_configure_user_file") {
	      $type="string";
	      $SaveTmpMM5ConfigureUserFile= $keyvalue;
	      ($is_ok, $SaveTmpMM5ConfigureUserFile) = &expandEnvVar($SaveTmpMM5ConfigureUserFile, $debug);
	      if (($SaveTmpMM5ConfigureUserFile =~ /\w/) && (length($SaveTmpMM5ConfigureUserFile) > 0)) {
		  $DoSaveTmpMM5ConfigureUserFile=1;
	      }
	  }

	  elsif ($keyword eq "do_modify_mm5_configure_user") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoModifyMM5ConfigureUser=1;
	      } else {
		  $DoModifyMM5ConfigureUser=0;
	      }
	  }

	  elsif ($keyword eq "mm5_configure_user_mem_option") {
	      $type="string";
	      ($is_ok, $keyvalue)= &expandEnvVar($keyvalue, $debug);
	      $local_found=0;
	      foreach $local_type (@MM5ConfigureUser_MemOptionArr) {
		  if ($keyvalue =~ /$local_type/) {
		      $local_found=1;
		  }
	      }
	      if (!$local_found) {
		  print(STDERR "ERROR: $subname: Invalid option for mm5_configure_user_mem_option: $keyvalue\n");
	      } else {
		  $MM5ConfigureUser_MemOption=$keyvalue;
	      }
	  }

	  elsif ($keyword eq "mm5_configure_user_runtime_system") {
	      $type="string";
	      $MM5ConfigureUser_RuntimeSystem= $keyvalue;
	      ($is_ok, $MM5ConfigureUser_RuntimeSystem) = &expandEnvVar($MM5ConfigureUser_RuntimeSystem, $debug);
	  }

	  elsif ($keyword eq "mm5_configure_user_use_runtime_system") {
	      $type="string";
	      $MM5ConfigureUser_UseRuntimeSystem= $keyvalue;
	      ($is_ok, $MM5ConfigureUser_UseRuntimeSystem) = &expandEnvVar($MM5ConfigureUser_UseRuntimeSystem, $debug);
	  }

	  elsif ($keyword eq "mm5_configure_user_maxnes") {
	      $type="int";
	      ($is_ok, $MM5ConfigureUser_Maxnes) = &checkKeywordValue($keyword, $keyvalue, $type, @junk, $debug);
	      $DoSetMM5ConfigureUser_Maxnes=1;
	  }

	  elsif ($keyword eq "mm5_configure_user_mkx") {
	      $type="int";
	      ($is_ok, $MM5ConfigureUser_Mkx) = &checkKeywordValue($keyword, $keyvalue, $type, @junk, $debug);
	      $DoSetMM5ConfigureUser_Mkx=1;
	  }

	  elsif ($keyword eq "mm5_configure_user_mix") {
	      $type="int";
	      ($is_ok, $MM5ConfigureUser_Mix) = &checkKeywordValue($keyword, $keyvalue, $type, @junk, $debug);
	      $DoSetMM5ConfigureUser_Mix=1;
	  }

	  elsif ($keyword eq "mm5_configure_user_mjx") {
	      $type="int";
	      ($is_ok, $MM5ConfigureUser_Mjx) = &checkKeywordValue($keyword, $keyvalue, $type, @junk, $debug);
	      $DoSetMM5ConfigureUser_Mjx=1;
	  }

	  elsif ($keyword eq "mm5_configure_user_mod_suffix") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoMM5ConfigureUser_ModSuffix=1;
	      } else {
		  $DoMM5ConfigureUser_ModSuffix=0;
	      }
	  }

	  elsif ($keyword eq "mm5_pre_build_make_commands") {
	      $type="string";
	      $MM5PreBuildMakeCommands= $unstripped_keyvalue;
	      ($is_ok, $MM5PreBuildMakeCommands) = &expandEnvVar($MM5PreBuildMakeCommands, $debug);
	      $DoMM5PreBuildMakeCommands=1;
	  }

	  elsif ($keyword eq "mm5_make_commands") {
	      $type="string";
	      $MM5MakeCommands= $unstripped_keyvalue;
	      ($is_ok, $MM5MakeCommands) = &expandEnvVar($MM5MakeCommands, $debug);
	  }

	  elsif ($keyword eq "mm5_dest_dir") {
	      $type="string";
	      $MM5DestDir= $keyvalue;
	      ($is_ok, $MM5DestDir) = &expandEnvVar($MM5DestDir, $debug);
	  }

	  elsif ($keyword eq "do_mm5_polar_build") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoMM5PolarBuild=1;
	      } else {
		  $DoMM5PolarBuild=0;
	      }
	  }

	  elsif ($keyword eq "mm5_polar_src_dir") {
	      $type="string";
	      $MM5PolarSrcDir= $keyvalue;
	      ($is_ok, $MM5PolarSrcDir) = &expandEnvVar($MM5PolarSrcDir, $debug);
	  }

	  elsif ($keyword eq "mm5_configure_user_polar_ipolar_option") {
	      $type="int";
	      ($is_ok, $MM5PolarConfigureUser_Ipolar) = &checkKeywordValue($keyword, $keyvalue, $type, @junk, $debug);
	      $DoSetMM5PolarConfigureUser_Ipolar=1;
	  }

	  elsif ($keyword eq "mm5_configure_user_polar_isoil_option") {
	      $type="int";
	      ($is_ok, $MM5PolarConfigureUser_Isoil) = &checkKeywordValue($keyword, $keyvalue, $type, @junk, $debug);
	      $DoSetMM5PolarConfigureUser_Isoil=1;
	  }

	  elsif ($keyword eq "mm5_configure_user_polar_ibltyp_option") {
	      $type="string";
	      $MM5PolarConfigureUser_Ibltyp=$keyvalue;
	      $DoSetMM5PolarConfigureUser_Ibltyp=1;
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

	  elsif (($found_be_list) && ($keyword eq "be_make")) {
	      $type="string";
	      $be_make = $unstripped_keyvalue;
	      $be_make = &expandEnvVar($be_make, $debug);
	  }

	  elsif (($found_be_list) && ($keyword eq "be_check_for_pgi_lm_problems")) {
	      $type="string";
	      if ($keyvalue eq "TRUE") {
		  $be_flag=1;
	      } else {
		  $be_flag=0;
	      }
	  }

	  elsif (($found_be_list) && ($keyword eq "be_flag_start_str")) {
	      $type="string";
	      $be_flag_start_str = $unstripped_keyvalue;
	  }

	  elsif (($found_be_list) && ($keyword eq "be_flag_end_str")) {
	      $type="string";
	      $be_flag_end_str = $unstripped_keyvalue;
	  }

	  elsif (($found_np_list) && ($keyword eq "np_ns")) {
	      $type="int";
	      ($is_ok, $np_ns) = &checkKeywordValue($keyword, $keyvalue, $type, @junk, $debug);
	  }

	  elsif (($found_np_list) && ($keyword eq "np_ew")) {
	      $type="int";
	      ($is_ok, $np_ew) = &checkKeywordValue($keyword, $keyvalue, $type, @junk, $debug);
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

      # Are we inside a nproc list directive?

      if ($line =~ /\<NPROC_LIST\>/) {
	  $found_nproc_list=1;
	  next;
      }

      if ($line =~ /\<\\NPROC_LIST\>/) {
	  $found_nproc_list=0;
	  next;
      }

      # Are we inside a polar nproc list directive?

      if ($line =~ /\<NPROC_POLAR_LIST\>/) {
	  $found_nproc_polar_list=1;
	  next;
      }

      if ($line =~ /\<\\NPROC_POLAR_LIST\>/) {
	  $found_nproc_polar_list=0;
	  next;
      }

      # Are we inside a build entry list?

      if ($line =~ /\<BUILD_ENTRY\>/) {
	  $found_be_list=1;
	  $be_dir=$StringDefault;
	  $be_make=$StringDefault;
	  $be_flag=0;
	  $be_flag_start_str=$StringDefault;
	  $be_flag_end_str=$StringDefault;
	  next;
      }

      # Update the build array if at the end of a table entry list

      if (($found_be_list) && ($line =~ /\<\\BUILD_ENTRY\>/)) {

	  # Fill the array

	  $outBuildArr->[$build_counter][$BE_dir_idx]=$be_dir;
	  $outBuildArr->[$build_counter][$BE_make_idx]=$be_make;
	  $outBuildArr->[$build_counter][$BE_flag_check_for_pgi_lm_problem_idx]=$be_flag;
	  $outBuildArr->[$build_counter][$BE_flag_start_str_idx]=$be_flag_start_str;
	  $outBuildArr->[$build_counter][$BE_flag_end_str_idx]=$be_flag_end_str;

	  $found_be_list=0;
	  $build_counter++;

	  next;
      }

      # Are we inside an nproc entry list?

      if ($line =~ /\<NPROC_ENTRY\>/) {
	  $found_np_list=1;
	  $np_ns=$IntDefault;
	  $np_ew=$IntDefault;
	  next;
      }

      # Are we inside an nproc polar entry list?

      if ($line =~ /\<NPROC_POLAR_ENTRY\>/) {
	  $found_np_list=1;
	  $np_ns=$IntDefault;
	  $np_ew=$IntDefault;
	  next;
      }

      # Update the nproc array if at the end of a nproc entry list

      if (($found_np_list) && ($line =~ /\<\\NPROC_ENTRY\>/)) {

	  # Fill the array

	  $outNprocArr->[$nproc_counter][$NP_ns_idx]=$np_ns;
	  $outNprocArr->[$nproc_counter][$NP_ew_idx]=$np_ew;

	  $found_np_list=0;
	  $nproc_counter++;

	  next;
      }

      # Update the nproc polar array if at the end of a nproc polar entry list

      if (($found_np_list) && ($line =~ /\<\\NPROC_POLAR_ENTRY\>/)) {

	  # Fill the array

	  $outPolarNprocArr->[$nproc_polar_counter][$NP_ns_idx]=$np_ns;
	  $outPolarNprocArr->[$nproc_polar_counter][$NP_ew_idx]=$np_ew;

	  $found_np_list=0;
	  $nproc_polar_counter++;

	  next;
      }

  } #endwhile

  close(PARAM_FILE);

  # Okay, did everything get set???

  ($is_ok, $missing) = &checkAllParams;

  # ...Handle special cases...

  if (($DoModifyMM5ConfigureUser) && ($MM5ConfigureUser_MemOption !~ /NONE/) &&
      ($MM5ConfigureUser_RuntimeSystem !~ /\w/)) {
      print(STDERR "ERROR: You must specify a mm5_configure_user_runtime_system if the mm5_configure_user_mem_option is other than NONE\n");
      $missing++;
  }

  if ($MM5ConfigureUser_UseRuntimeSystem =~ /$StringDefault/) {
      $MM5ConfigureUser_UseRuntimeSystem = $MM5ConfigureUser_RuntimeSystem;
  }

  if (($DoMM5PolarBuild) && ($MM5ConfigureUserFile =~ /$StringDefault/)) {
      print(STDERR "ERROR: You must specify an mm5_configure_user_file for use with polar builds\n");
      $missing++;
  }

  if (($DoModifyMM5ConfigureUser) && ($MM5ConfigureUserFile !~ /$StringDefault/)) {
      $pos=rindex($MM5ConfigureUserFile, "/");
      $MM5ConfigureUserFilenameOnly=substr($MM5ConfigureUserFile, $pos+1);
  }

  # Set return

  if ($missing > 0) {
      $return_val = 0;
  } else {
      $return_val = 1;
  }

  $nbuild=$build_counter;
  $nnproc=$nproc_counter;
  $polar_nnproc=$nproc_polar_counter;

  # Debug
  
  if ($dbg2) {
    print(STDERR "$subname: return: $return_val, missing: $missing, nbuild: $nbuild, nnproc: $nnproc, $polar_nnproc\n");
  }

  # Done
  return($return_val, $nbuild, $nnproc, $polar_nnproc);
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
  print(STDOUT "# checkout_dir is the directory to checkout the source code into.\n");
  print(STDOUT "# It is strongly suggested that you put this in a separate disk\n");
  print(STDOUT "# location than you normally work in so that you can delete the\n");
  print(STDOUT "# source and build files before checking out and rebuilding\n");
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
  print(STDOUT "# from CVS and run and checkout the desired source code\n");
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
  print(STDOUT "# You must have previously tagged all your desired source code\n");
  print(STDOUT "# with the specified CVS tag. Leave blank to checkout the latest\n");
  print(STDOUT "# code and not use a CVS tag.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "rtag = \n\n");

  print(STDOUT "#------------ Max retries for PGI ----------------\n");
  print(STDOUT "# max_retries is the max number of times to try to recompile code\n");
  print(STDOUT "# that has failed due to the PGI FLEX License manager problem.\n");
  print(STDOUT "# this is ONLY used for code that does not use the MS_MAKE_MACROS_DIR/make_defns\n");
  print(STDOUT "# suffix rules.\n");
  print(STDOUT "# Type: integer\n");
  print(STDOUT "#\n");
  print(STDOUT "max_flexlm_retries=3\n\n");

  print(STDOUT "#*************** Build params **********************\n\n");

  print(STDOUT "#------------ do build list ----------------\n");
  print(STDOUT "# do_build is a flag to build the code in the build list\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_build_list=TRUE\n\n");

  print(STDOUT "#------------- Build list --------------\n");
  print(STDOUT "# The build list contains the list of top-level build source code\n");
  print(STDOUT "# directories and the make commands to use. Only one build list\n");
  print(STDOUT "# is allowed but may have multiple build entries. The build list\n");
  print(STDOUT "# must begin with a <BUILD_LIST> tag and end with a <\\BUILD_LIST> tag\n");
  print(STDOUT "# Within the build list, each build entry must begin with the <BUILD_ENTRY>\n");
  print(STDOUT "# tag and end with the <\\BUILD_ENTRY> tag. Each tag and each item in the build\n");
  print(STDOUT "# list must be on a separate line. Do not put quotes on the make commands.\n");
  print(STDOUT "# Do not include MM5 in the build entries, it is handled separately below.\n");
  print(STDOUT "# Use the following format:\n");
  print(STDOUT "# <BUILD_LIST>\n");
  print(STDOUT "# <BUILD_ENTRY>\n");
  print(STDOUT "# be_dir=(Required) Top level directory to execute the be_make commands in\n");
  print(STDOUT "# be_make=(Required) make commands to execute in be_dir\n");
  print(STDOUT "# be_check_for_pgi_lm_problems=(Optional, default is FALSE)\n");
  print(STDOUT "#                            TRUE or FALSE. Set this to TRUE if any code in the\n");
  print(STDOUT "#                            be_make Makefile is for code that does not use the\n");
  print(STDOUT "#                            MS_MAKE_MACROS_DIR/make_defns suffix rules and you\n");
  print(STDOUT "#                            have problems with the PGI license manager\n");
  print(STDOUT "# be_flag_start_str=(Optional) unique string that bounds start of make output for\n");
  print(STDOUT "#                   code with PGI license manager problems\n");
  print(STDOUT "# be_flag_end_str=(Optional) unique string that bounds end of make output for\n");
  print(STDOUT "#                 code with PGI license manager problems\n");
  print(STDOUT "# <\\BUILD_ENTRY>\n");
  print(STDOUT "# <\\BUILD_LIST>\n");
  print(STDOUT "#\n");
  print(STDOUT "<BUILD_LIST>\n");
  print(STDOUT "<BUILD_ENTRY>\n");
  print(STDOUT "<\\BUILD_ENTRY>\n");
  print(STDOUT "<\\BUILD_LIST>\n\n");

  print(STDOUT "#*************** WRF params **********************\n\n");

  # ... do WRF build? ...

  print(STDOUT "#------------ WRF do build ----------------\n");
  print(STDOUT "# do_wrf_build is a flag to do the WRF build\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_wrf_build=FALSE\n\n");

  # ... wrf build command ...

  print(STDOUT "#------------ WRF build command ----------------\n");
  print(STDOUT "# wrf_build_cmd is the command to run to build WRF.\n");
  print(STDOUT "# This will be executed only if do_wrf_build is TRUE.\n");
  print(STDOUT "# Do not enclose in quotes, either single or double\n");
  print(STDOUT "# since this causes problems with the system() call\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "wrf_build_cmd=\n\n");

  print(STDOUT "#*************** MM5 params **********************\n\n");

  # ... do MM5 build? ...

  print(STDOUT "#------------ MM5 do build ----------------\n");
  print(STDOUT "# do_mm5_build is a flag to do the MM5 build\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_mm5_build=FALSE\n\n");

  # ... MM5 do check for PGI license manager problems ...

#  print(STDOUT "#------------ MM5 do check for PGI problems ----------------\n");
#  print(STDOUT "# do_mm5_check_for_pgi_problems is a flag to check the MM5 build for problems\n");
#  print(STDOUT "# with the PGI license manager (FLEXlm dropping out). This is not\n");
#  print(STDOUT "# necessary if the MM5 configure.user suffix rules have the mod\n");
#  print(STDOUT "# to delete the .o and retry if problems encountered.\n");
#  print(STDOUT "# Type: enum, with the following options:\n");
#  print(STDOUT "#         TRUE\n");
#  print(STDOUT "#         FALSE\n");
#  print(STDOUT "#\n");
#  print(STDOUT "do_mm5_check_for_pgi_lm_problems=FALSE\n\n");

  # ... MM5 src dir ...

  print(STDOUT "#------------ MM5 src dir ----------------\n");
  print(STDOUT "# mm5_src_dir is the directory path to the top-level MM5 source\n");
  print(STDOUT "# directory to compile.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_src_dir =\n\n");

  # ... MM5 configure.user file ...

  print(STDOUT "#------------ MM5 configure.user file ----------------\n");
  print(STDOUT "# mm5_configure_user_file is the full pathname of the MM5\n");
  print(STDOUT "# configure.user file to use in the MM5 compiles. This file\n");
  print(STDOUT "# will be copied to configure.user in the mm5_src_dir\n");
  print(STDOUT "# and will have the mods below applied before compiling\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_configure_user_file=\n\n");

  # ... Save the temp MM5 configure.user file ...

  print(STDOUT "#------------ Save temp MM5 configure.user file ----------------\n");
  print(STDOUT "# save_tmp_mm5_configure_user_file is the full pathname to save\n");
  print(STDOUT "# a copy of the temporary MM5 configure.user file into. The copy\n");
  print(STDOUT "# is for reference only. The default is to delete the temporary\n");
  print(STDOUT "# file. Leave blank to not save a copy of the temporary file.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "save_tmp_mm5_configure_user_file=\n\n");

  # ... Settings for configure.user file ...

  print(STDOUT "#------------ MM5 configure.user settings ----------------\n");

  print(STDOUT "# do_modify mm5_configure_user determines whether to modify\n");
  print(STDOUT "# the mm5_configure_user_file with any of the settings in the\n");
  print(STDOUT "# this ---MM5 configure.user settings--- section. Set to FALSE\n");
  print(STDOUT "# to not modify the mm5_configure_user_file except for the processor\n");
  print(STDOUT "# settings below.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_modify_mm5_configure_user=FALSE\n\n");

  print(STDOUT "# mm5_configure_user_mem_option determines which compiler\n");
  print(STDOUT "# settings to (un)comment in the mm5_configure_user_file\n");
  print(STDOUT "# Set to NONE to not modify the mm5_configure_user_file compiler\n");
  print(STDOUT "# settings.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         MPP\n");
  print(STDOUT "#         NOTMPP\n");
  print(STDOUT "#         NONE\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_configure_user_mem_option=MPP\n\n");
  
  print(STDOUT "# mm5_configure_user_runtime_system determines which runtime\n");
  print(STDOUT "# system section to uncomment in the mm5_configure_user_file.\n");
  print(STDOUT "# The string MUST match an entry in the mm5_configure_user_file\n");
  print(STDOUT "# for RUNTIME_SYSTEM\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_configure_user_runtime_system=linux\n\n");

  print(STDOUT "# mm5_configure_user_use_runtime_system determines what to set\n");
  print(STDOUT "# the RUNTIME_SYSTEM entry in the mm5_configure_user_file to for compiles.\n");
  print(STDOUT "# Leave blank to use the mm5_configure_user_runtime_system above.\n");
  print(STDOUT "# E.g., for 64 bit compiles, set mm5_configure_user_runtime_system\n");
  print(STDOUT "# to linux64 and this parameter to linux. This is because linux64\n");
  print(STDOUT "# is not understood outside the context of the comment/uncomment action\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_configure_user_use_runtime_system=\n\n");

  print(STDOUT "# mm5_configure_user_maxnes is the MAXNES max number of domains setting\n");
  print(STDOUT "# in the mm5_configure_user_file.\n");
  print(STDOUT "# Leave blank to not change the setting in the mm5_configure_user_file\n");
  print(STDOUT "# Type: integer\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_configure_user_maxnes=\n\n");

  print(STDOUT "# mm5_configure_user_mix and mm5_configure_user_mjx are the MIX and\n");
  print(STDOUT "# MJX maximum domain dimensions in the mm5_configure_user_file.\n");
  print(STDOUT "# Leave blank to not change the settings in the mm5_configure_user_file\n");
  print(STDOUT "# Type: integer\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_configure_user_mix=\n\n");
  print(STDOUT "mm5_configure_user_mjx=\n\n");

  print(STDOUT "# mm5_configure_user_mkx is the MKX number of half sigma levels\n");
  print(STDOUT "# in the model in the mm5_configure_user_file.\n");
  print(STDOUT "# Leave blank to not change the settings in the mm5_configure_user_file\n");
  print(STDOUT "# Type: integer\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_configure_user_mkx=\n\n");

  print(STDOUT "# mm5_configure_user_mod_suffix modifies the suffix rules in the\n");
  print(STDOUT "# mm5_configure_user_file to add max_flexlm_retries to the suffix rules\n");
  print(STDOUT "# to try and get around the PGI license manager issue at NCAR.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_configure_user_mod_suffix=FALSE\n\n");

  # ... MM5 pre build make commands ...

  print(STDOUT "#------------ MM5 pre-build make commands ----------------\n");
  print(STDOUT "# mm5_pre_build_make_commands is the make commands to execute\n");
  print(STDOUT "# in the mm5_src_dir before looping through the processor pairs.\n");
  print(STDOUT "# For example: make uninstall\n");
  print(STDOUT "# Do not enclose in quotes. Leave blank to not execute any\n");
  print(STDOUT "# make commands prior to looping through the processor pairs.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_pre_build_make_commands=make uninstall\n\n");
  print(STDOUT "#\n");

  # ... MM5 make commands ...

  print(STDOUT "#------------ MM5 make commands ----------------\n");
  print(STDOUT "# mm5_make_commands is the make commands to execute in the \n");
  print(STDOUT "# checkout_dir for each loop through the processor pairs. \n");
  print(STDOUT "# Do not enclose in quotes\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_make_commands=make mpclean; make mpp\n\n");

  # ... MM5 list of number-of-nodes ...

  print(STDOUT "#------------ MM5 number of processors ----------------\n");
  print(STDOUT "# The processor list contains the list of processor pairs to compile MM5\n");
  print(STDOUT "# for. Only one processor list is allowed but may have multiple processor\n");
  print(STDOUT "# entries. The processor list must begin with a <NPROC_LIST> tag and end with\n");
  print(STDOUT "# a <\\NPROC_LIST> tag\n");
  print(STDOUT "# Within the processor list, each processor entry must begin with the <NPROC_ENTRY>\n");
  print(STDOUT "# tag and end with the <\\NPROC_ENTRY> tag. Each tag and each item in the processor\n");
  print(STDOUT "# list must be on a separate line.\n");
  print(STDOUT "# Use the following format:\n");
  print(STDOUT "# <NPROC_LIST>\n");
  print(STDOUT "# <NPROC_ENTRY>\n");
  print(STDOUT "# np_ns=North-South number of processors\n");
  print(STDOUT "# np_ew=East-West number of processors\n");
  print(STDOUT "# <\\NPROC_ENTRY>\n");
  print(STDOUT "# <\\NPROC_LIST>\n");
  print(STDOUT "#\n");
  print(STDOUT "<NPROC_LIST>\n");
  print(STDOUT "<NPROC_ENTRY>\n");
  print(STDOUT "<\\NPROC_ENTRY>\n");
  print(STDOUT "<\\NPROC_LIST>\n\n");

  # ... MM5 destination directory ...

  print(STDOUT "#------------ MM5 destination dir ----------------\n");
  print(STDOUT "# mm5_dest_dir is the directory location to copy the mm5.mpichNN* to\n");
  print(STDOUT "# after compiled for the NN number of processors\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_dest_dir =\n\n");

  print(STDOUT "#*************** MM5 Polar params **********************\n\n");

  # ... do MM5 Polar build? ...

  print(STDOUT "#------------ MM5 do polar build ----------------\n");
  print(STDOUT "# do_mm5_polar_build is a flag to do the MM5 polar build. This will use many\n");
  print(STDOUT "# of the settings for the MM5 build above with a few additional changes.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_mm5_polar_build=FALSE\n\n");

  # ... MM5 Polar src dir ...

  print(STDOUT "#------------ MM5 polar src dir ----------------\n");
  print(STDOUT "# mm5_polar_src_dir is the directory path to the top-level MM5\n");
  print(STDOUT "# polar source directory to compile.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_polar_src_dir =\n\n");

  # ... Settings for Polar configure.user file ...

  print(STDOUT "#------------ MM5 Polar configure.user settings ----------------\n");
  
  print(STDOUT "# MM5 polar compiles will use the same settings as above in the\n");
  print(STDOUT "# section --- MM5 configure.user settings ---, with the following\n");
  print(STDOUT "# additions. \n\n");

  print(STDOUT "# mm5_configure_user_polar_ipolar_option determines which compiler\n");
  print(STDOUT "# settings to set IPOLAR to in the mm5_configure_user_file\n");
  print(STDOUT "# Type: boolean, with the following options: 0 or 1\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_configure_user_polar_ipolar_option=\n\n");

  print(STDOUT "# mm5_configure_user_polar_isoil_option determines which compiler\n");
  print(STDOUT "# settings to set ISOIL to in the mm5_configure_user_file\n");
  print(STDOUT "# Type: int, with the following options: 1 or 2\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_configure_user_polar_isoil_option=\n\n");

  print(STDOUT "# mm5_configure_user_polar_ibltyp_option determines which compiler\n");
  print(STDOUT "# settings to set IBLTYP in the mm5_configure_user_file\n");
  print(STDOUT "# Type: string of ints, comma-delimited, must be enclosed in quotes.\n");
  print(STDOUT "# For example: \"4,4,4,4,4,0,0,0,0,5\"\n");
  print(STDOUT "#\n");
  print(STDOUT "mm5_configure_user_polar_ibltyp_option=\n\n");

  # ... MM5 Polar list of number-of-nodes ...

  print(STDOUT "#------------ MM5 Polar number of processors ----------------\n");
  print(STDOUT "# The processor list contains the list of processor pairs to compile MM5 Polar\n");
  print(STDOUT "# for. Only one processor list is allowed but may have multiple processor\n");
  print(STDOUT "# entries. The processor list must begin with a <NPROC_POLAR_LIST> tag and end with\n");
  print(STDOUT "# a <\\NPROC_POLAR_LIST> tag\n");
  print(STDOUT "# Within the processor list, each processor entry must begin with the <NPROC_POLAR_ENTRY>\n");
  print(STDOUT "# tag and end with the <\\NPROC_POLAR_ENTRY> tag. Each tag and each item in the processor\n");
  print(STDOUT "# list must be on a separate line.\n");
  print(STDOUT "# Use the following format:\n");
  print(STDOUT "# <NPROC_POLAR_LIST>\n");
  print(STDOUT "# <NPROC_POLAR_ENTRY>\n");
  print(STDOUT "# np_ns=North-South number of processors\n");
  print(STDOUT "# np_ew=East-West number of processors\n");
  print(STDOUT "# <\\NPROC_POLAR_ENTRY>\n");
  print(STDOUT "# <\\NPROC_POLAR_LIST>\n");
  print(STDOUT "#\n");
  print(STDOUT "<NPROC_POLAR_LIST>\n");
  print(STDOUT "<NPROC_POLAR_ENTRY>\n");
  print(STDOUT "<\\NPROC_POLAR_ENTRY>\n");
  print(STDOUT "<\\NPROC_POLAR_LIST>\n\n");

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
  print(STDOUT "# my_postbuild_script is a script to call after all the source\n");
  print(STDOUT "# has been compiled. Leave blank to not run an additional script.\n");
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
  $MaxFlexLmRetries=3;
  $DoBuildList=1;

  $DoWrfBuild=0;
  $WrfBuildCmd=$StringDefault;

  $DoMM5Build=0;
  $DoMM5CheckForPgiLmProblems=0;
  $MM5SrcDir=$StringDefault;
  $DoMM5PreBuildMakeCommands=0;
  $MM5PreBuildMakeCommands="make uninstall";
  $MM5MakeCommands="make mpclean; make mpp";
  $MM5DestDir=$StringDefault;

  $MM5ConfigureUserFile=$StringDefault;

  $SaveTmpMM5ConfigureUserFile=$StringDefault;
  $DoSaveTmpMM5ConfigureUserFile=0;

  $DoModifyMM5ConfigureUser=0;
  $MM5ConfigureUser_MemOption=$MM5ConfigureUser_MemOptionArr[0];
  $MM5ConfigureUser_RuntimeSystem="linux";
  $MM5ConfigureUser_UseRuntimeSystem=$StringDefault;
  $MM5ConfigureUser_Mkx=$IntDefault;
  $DoSetMM5ConfigureUser_Mkx=0;
  $MM5ConfigureUser_Mix=$IntDefault;
  $DoSetMM5ConfigureUser_Mix=0;
  $MM5ConfigureUser_Mjx=$IntDefault;
  $DoSetMM5ConfigureUser_Mjx=0;
  $MM5ConfigureUser_Maxnes=$IntDefault;
  $DoSetMM5ConfigureUser_Maxnes=0;
  $DoMM5ConfigureUser_ModSuffix=0;

  $DoMM5PolarBuild=0;
  $MM5PolarSrcDir=$StringDefault;
  $MM5PolarConfigureUser_Ipolar=$IntDefault;
  $DoSetMM5PolarConfigureUser_Ipolar=0;
  $MM5PolarConfigureUser_Isoil=$IntDefault;
  $DoSetMM5PolarConfigureUser_Isoil=0;
  $MM5PolarConfigureUser_Ibltyp=$StringDefault;
  $DoSetMM5PolarConfigureUser_Ibltyp=0;

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

    print(STDERR "\tmax_flexlm_retries: $MaxFlexLmRetries\n");

    print(STDERR "\tdo_build_list: $DoBuildList\n");
    print(STDERR "\tBuild list, number of entries: $nbuildarr\n");
    for ($i=0; $i<$nbuildarr; $i++) {
	print(STDERR "\t\tbuild dir: $buildArr[$i][$BE_dir_idx], make: $buildArr[$i][$BE_make_idx]\n");
	print(STDERR "\t\tflag: $buildArr[$i][$BE_flag_check_for_pgi_lm_problem_idx], start_str: $buildArr[$i][$BE_flag_start_str_idx], end_str: $buildArr[$i][$BE_flag_end_str_idx]\n");
    }

    print(STDERR "\tdo_wrf_build: $DoWrfBuild\n");
    print(STDERR "\twrf_build_cmd: $WrfBuildCmd\n");

    print(STDERR "\tdo_mm5_build: $DoMM5Build\n");
#    print(STDERR "\tdo_mm5_check_for_pgi_lm_problems: $DoMM5CheckForPgiLmProblems\n");
    print(STDERR "\tmm5_src_dir: $MM5SrcDir\n");
    print(STDERR "\tmm5_configure_user_file: $MM5ConfigureUserFile\n");
    print(STDERR "\tsave_tmp_mm5_configure_user_file: $SaveTmpMM5ConfigureUserFile\n");
    print(STDERR "\tdo_save_tmp_mm5_configure_user_file: $DoSaveTmpMM5ConfigureUserFile\n");

    print(STDERR "\tdo_modify_mm5_configure_user: $DoModifyMM5ConfigureUser\n");
    print(STDERR "\tmm5_configure_user_mem_option: $MM5ConfigureUser_MemOption\n");
    print(STDERR "\tmm5_configure_user_runtime_system: $MM5ConfigureUser_RuntimeSystem\n");
    print(STDERR "\tmm5_configure_user_use_runtime_system: $MM5ConfigureUser_UseRuntimeSystem\n");
    print(STDERR "\tmm5_configure_user_mkx: $MM5ConfigureUser_Mkx\n");
    print(STDERR "\tDoSetMM5ConfigureUser_Mxk: $DoSetMM5ConfigureUser_Mkx\n");
    print(STDERR "\tmm5_configure_user_mix: $MM5ConfigureUser_Mix\n");
    print(STDERR "\tDoSetMM5ConfigureUser_Mix: $DoSetMM5ConfigureUser_Mix\n");
    print(STDERR "\tmm5_configure_user_mjx: $MM5ConfigureUser_Mjx\n");
    print(STDERR "\tDoSetMM5ConfigureUser_Mjx: $DoSetMM5ConfigureUser_Mjx\n");
    print(STDERR "\tmm5_configure_user_maxnes: $MM5ConfigureUser_Maxnes\n");
    print(STDERR "\tDoSetMM5ConfigureUser_Maxnes: $DoSetMM5ConfigureUser_Maxnes\n");
    print(STDERR "\tmm5_configure_user_mod_suffix: $DoMM5ConfigureUser_ModSuffix\n");

    print(STDERR "\tmm5_pre_build_make_commands: $MM5PreBuildMakeCommands\n");
    print(STDERR "\tmm5_make_commands: $MM5MakeCommands\n");
    print(STDERR "\tmm5_dest_dir: $MM5DestDir\n");

    print(STDERR "\tMM5 Num processor list, number of entries: $nnprocarr\n");
    for ($i=0; $i<$nnprocarr; $i++) {
	print(STDERR "\t\tNproc NS: $nprocArr[$i][$NP_ns_idx], EW: $nprocArr[$i][$NP_ew_idx]\n");
    }

    print(STDERR "\tdo_mm5_polar_build: $DoMM5PolarBuild\n");
    print(STDERR "\tmm5_polar_src_dir: $MM5PolarSrcDir\n");
    print(STDERR "\tMM5PolarConfigureUser_Ipolar: $MM5PolarConfigureUser_Ipolar\n");
    print(STDERR "\tDoSetMM5PolarConfigureUser_Ipolar: $DoSetMM5PolarConfigureUser_Ipolar\n");
    print(STDERR "\tMM5PolarConfigureUser_Isoil: $MM5PolarConfigureUser_Isoil\n");
    print(STDERR "\tDoSetMM5PolarConfigureUser_Isoil: $DoSetMM5PolarConfigureUser_Isoil\n");
    print(STDERR "\tMM5PolarConfigureUser_Ibltyp: $MM5PolarConfigureUser_Ibltyp\n");
    print(STDERR "\tDoSetMM5PolarConfigureUser_Ibltyp: $DoSetMM5PolarConfigureUser_Ibltyp\n");

    print(STDERR "\tMM5 Polar Num processor list, number of entries: $polar_nnprocarr\n");
    for ($i=0; $i<$polar_nnprocarr; $i++) {
	print(STDERR "\t\tNproc NS: $polar_nprocArr[$i][$NP_ns_idx], EW: $polar_nprocArr[$i][$NP_ew_idx]\n");
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

#---------------------------------------------------------------------
# Subroutine: doPgiLmProblem
#
# Usage: $return_val=doPgiLmProblem($infile, $flag_start_str $flag_end_str, 
# 				    $dir, $make_cmd, $debug);
#
# Function:   Look through $infile for PGI FLEX License Manager problems.
#             Recompile using $make_cmd if problems found.
#
# Input:      $infile          make output log to search
#             $flag_start_str  string to search for to bound start of problem
#                                search
#             $flag_end_str    string to search for to bound end of problem
#                                search
#             $topdir          top level dir used in make
#             $make_cmd        make commands used
#             $test            test mode
#             $debug           debug flag 
#
# Output:     $return_val      0 if errors in function, 1 on error
# 

sub doPgiLmProblem {
    my ($infile, $flag_start_str, $flag_end_str, $topdir, $make_cmd, $test, $debug) = @_;

    # Local variables

    my ($is_ok, @dir_arr, $i, $j, $cmd, $cmd_ok, $found_errors, $arrsize, $dir);
    
    my $return_val=0;
    my $subname="doPgiLmProblem";
    my $dbg2=0;
    my $dbg3=0;
    my $tmp_file="${TmpDir}/make.tmp.file";

    # Debugging

    if ($debug == 2) {
	$dbg2=1;
    }
    if ($debug == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    if ($dbg2) {
	print(STDERR "$subname: Input...\n");
	print(STDERR "\tinfile: $infile\n");
	print(STDERR "\tflag_start_str: $flag_start_str, flag_end_str: $flag_end_str\n");
	print(STDERR "\ttopdir: $topdir, make_cmd: $make_cmd, test: $test\n");
    }

    # Check the infile

    ($is_ok, $found_errors, $arrsize)=lookForPgiLmProblem($infile, $flag_start_str, $flag_end_str, 
							  \@dir_arr, $debug);

    if (!$is_ok) {
	return($return_val);
    }
    
    if (!$found_errors && $arrsize <= 0) {
	$return_val=1;
	return($return_val);
    }

    # Okay, found some errors now have to loop through recompile, recheck

    for ($j=0; $j<$MaxFlexLmRetries; $j++) {

	# Cleanup tmp files

	unlink<${tmp_file}*>;

	# Recompile

	print(STDERR "========== PGI LM problems found, recompile $dir ===========\n");
	chdir($topdir);
	$cmd="cd $topdir; ${make_cmd} | tee ${tmp_file}";
	($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);

#------------------------------------------------------------------------------------------	
# This block is commented out. Would be the preferred method but do not have
# needed make targets at all levels in sub_dirs in the MMM-type Makefiles
# so the make commands fail if the dir_arr[n] entry occurs at the wrong level.
#
##	# Loop through the array of problem directories and recompile
##
##	for ($i=0; $i<$arrsize; $i++) {
##	    $dir=$dir_arr[$i];
##
##	    if ($debug) {
##		print(STDERR "========== PGI LM problems found, recompile $dir ===========\n");
##	    }
##	    chdir($dir);
##	    $cmd="${make_cmd} | tee ${tmp_file}.${i}";
##	    ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);
##	}
##
##	# Concatenate all the problem logs together and search again
##
##	for ($i=0; $i<$arrsize; $i++) {
##	    $cmd="cat ${tmp_file}.${i} >> $tmp_file";
##	    ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);
##	}
#------------------------------------------------------------------------------------------	

	($is_ok, $found_errors, $arrsize)=lookForPgiLmProblem($tmp_file, $StringDefault, $StringDefault,
							      \@dir_arr, $debug);
	
	if (!$found_errors && $arrsize <= 0) {
	    $j=$MaxFlexLmRetries;
	}
    }

    # Cleanup tmp files

    $cmd="/bin/rm -rf ${tmp_file}*";
    ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $debug);

    # Done

    $return_val=1;
    return($return_val);
}
	
#---------------------------------------------------------------------
# Subroutine: lookForPgiLmProblem
#
# Usage: ($return_val, $found_error, $narr)
#             =lookForPgiLmProblem($infile, $flag_start_str $flag_end_str, 
# 				    $out_arr, $debug);
#
# Function:   Look through $infile for PGI FLEX License Manager problems.
#             Recompile using $make_cmd if problems found.
#
# Input:      $infile          make output log to search
#             $flag_start_str  string to search for to bound start of problem
#                                search
#             $flag_end_str    string to search for to bound end of problem
#                                search
#             $debug           debug flag 
#
# Output:     $return_val      0 if errors in function, 1 on error
#             $found_error     did find the PGI LM problem (1=yes, 0=no)
#             $narr            size of out_arr
#             $out_arr[n]      sub_dir with error
# 

sub lookForPgiLmProblem {
    my ($infile, $flag_start_str, $flag_end_str, $out_arr, $debug) = @_;

    # Local variables

    my ($is_ok, $wanted_section, $line, $save_line, $counter, $sub_dir);
    my ($junk, $i, $use_start_end_str, $orig_line, $search_str, $slash, $quote);
    
    my $return_val=0;
    my $subname="lookForPgiLmProblem";
    my $dbg2=0;
    my $dbg3=0;
    my $narr=0;
    my $found_error=0;

    # Debugging

    if ($debug == 2) {
	$dbg2=1;
    }
    if ($debug == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    if ($dbg2) {
	print(STDERR "$subname: Input...\n");
	print(STDERR "\tinfile: $infile\n");
	print(STDERR "\tflag_start_str: $flag_start_str, flag_end_str: $flag_end_str\n");
    }

    # Open the input file

    $is_ok=open(IN_FILE, $infile);
    if (!$is_ok) {
	print(STDERR "ERROR: $prog: $subname: Cannot open file $infile\n");
	return($return_val, $found_error, $narr);
    }

    # Should we look for the start/end strings

    if (($flag_start_str ne $StringDefault) && ($flag_end_str ne $StringDefault)) {
	$use_start_end_str=1;
    } else {
	$use_start_end_str=0;
    }

    if ($dbg2) {
	print(STDERR "$subname: use_start_end_str: $use_start_end_str\n");
    }

    # Go through the file, look for lines such as:
    #
    #  pgf90 -c -Mfreeform -byteswapio -I../util datint.f
    #  pgf90-linux86: LICENSE MANAGER PROBLEM: Invalid returned data from license server
    #  Feature:       pgf90-linux86
    #  Hostname:      license1.ucar.edu
    #  License path:  /tools/pgi/license.dat
    #  FLEXlm error:  -12,122
    #  For further information, refer to the FLEXlm End User Manual,
    #  available at "www.macrovision.com".
    #  make[5]: [datint.o] Error 2 (ignored)
   
    $wanted_section=0;
    $counter=0;
    while ($line = <IN_FILE>) {

	# if the use_start_end is set, skip lines until we get to a flag_start_str

	if (($use_start_end_str) && ($line =~ /$flag_start_str/)) {
	    $wanted_section=1;
	    next;
	}

	if (($use_start_end_str) && ($line =~ /$flag_end_str/)) {
	    $wanted_section=0;
	    next;
	}

	if (($use_start_end_str) && (!$wanted_section)) {
	    next;
	}

	# Convert the line to upper case so can find matches

	$orig_line=$line;
	$line =~ tr/[a-z]/[A-Z]/;     

	# Need to keep track of where we are so save the last 
	# Entering directory line. These are lines such as:
	#   make[5]: Entering directory /d1/CVS_reference/cvs/apps/4dwx/RTFDDA/src/RT_REGRID_V3.6/pregrid/grib.misc

	$search_str="ENTERING DIRECTORY";
	if ($line =~ /$search_str/) {
	    $save_line=$orig_line;

	    # Parse out the subdir

	    chop($save_line);
	    $slash=index($save_line, "/");
	    $sub_dir=substr($save_line, $slash);
	    $quote=index($sub_dir, "'");
	    if ($quote > 0) {
		$sub_dir=substr($sub_dir, 0, $quote);
	    }
	    
	    if ($dbg3) {
		print(STDERR "$subname, save Entering directory: $sub_dir\n");
	    }

	    next;
	}

	# Look for the License Manager error string

	$search_str="LICENSE MANAGER PROBLEM";
	if ($line =~ /$search_str/) {
	    if ($dbg2) {
		print(STDERR "$subname: found line with PGI License Manager error, counter: $counter\n");
	    }
	    $out_arr->[$counter]=$sub_dir;
	    $counter++;
	    next;
	}

    } #endwhile

    close(IN_FILE);

    # Set return values

    $return_val=1;
    $narr=$counter;
    if ($narr > 0) {
	$found_error=1;
    }

    # Debug

    if ($debug) {
	print(STDERR "$subname: found_error: $found_error, narr: $narr\n");
	if ($dbg2) {
	    for ($i=0; $i<$narr; $i++) {
		print(STDERR "$subname: i: $i, out_arr: $out_arr->[$i]\n");
	    }
	}
    }

    # Done

    return($return_val, $found_error, $narr);
}

#---------------------------------------------------------------------
# Subroutine: replaceMM5RuntimeSystem
#
# Usage: ($return_val, $newline)
#             =replaceMM5RuntimeSystem($inline, $debug);
#
# Function:   Replace the RUNTIME_SYSTEM setting in the current line
#
# Input:      $inline          line to change
#             $debug           debug flag 
#
# Output:     $return_val      0 if errors in function, 1 on error
#             $newline         modified line
# 

sub replaceMM5RuntimeSystem {
    my ($inline, $debug) = @_;

    # Local variables

    my ($newline);
    
    my $return_val=0;
    my $subname="replaceMM5RuntimeSystem";
    my $dbg2=0;
    my $dbg3=0;

    # Debugging

    if ($debug == 2) {
	$dbg2=1;
    }
    if ($debug == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    if ($dbg2) {
	print(STDERR "$subname: Input...\n");
	print(STDERR "\tinline: $inline");
    }

    $newline=$inline;
    $return_val=1;

    if (($MM5ConfigureUser_RuntimeSystem =~ /$MM5ConfigureUser_UseRuntimeSystem/) &&
	(length($MM5ConfigureUser_RuntimeSystem) == length($MM5ConfigureUser_UseRuntimeSystem))) {
	if ($dbg3) {
	    print(STDERR "$subname: No change to RUNTIME_SYSTEM, MM5ConfigureUser_RuntimeSystem and MM5ConfigureUser_UseRuntimeSystem are the same: $MM5ConfigureUser_RuntimeSystem, $MM5ConfigureUser_RuntimeSystem\n");
	}
    } else {
	if ($dbg3) {
	    print(STDERR "$subname: change RUNTIME_SYSTEM to $MM5ConfigureUser_UseRuntimeSystem\n");
	}
	$newline =~ s/$MM5ConfigureUser_RuntimeSystem/$MM5ConfigureUser_UseRuntimeSystem/;
    }

    if ($dbg2) {
	print(STDERR "$subname: newline: $newline");
    }

    return($return_val, $newline);
}


#---------------------------------------------------------------------
# Subroutine: matchMM5RuntimeSystem
#
# Usage: ($return_val, $is_match)
#             =matchMM5RuntimeSystem($inline, $debug);
#
# Function:   Is the RUNTIME_SYSTEM setting in the inline an exact match?
#
# Input:      $inline          line to check
#             $debug           debug flag 
#
# Output:     $return_val      0 if errors in function, 1 on error
#             $is_match        1 if exact match, 0 otherwise
# 

sub matchMM5RuntimeSystem {
    my ($inline, $debug) = @_;

    # Local variables

    my ($junk, $wanted);
    
    my $return_val=0;
    my $subname="matchMM5RuntimeSystem";
    my $dbg2=0;
    my $dbg3=0;
    my $dbg4=0;
    my $is_match=0;

    # Debugging

    if ($debug == 2) {
	$dbg2=1;
    }
    if ($debug == 3) {
	$dbg2=1;
	$dbg3=1;
    }
    if ($debug == 4) {
	$dbg2=1;
	$dbg3=1;
	$dbg4=1;
    }

    if ($dbg3) {
	print(STDERR "$subname: Input...\n");
	print(STDERR "\tinline: $inline");
    }

    # Start with a basic test

    $return_val=1;

    if ($inline !~ /RUNTIME_SYSTEM/) {
	if ($dbg4) {
	    print(STDERR "$subname: no match for RUNTIME_SYSTEM\n");
	}
	return($return_val, $is_match);
    }

    if ($inline !~ /$MM5ConfigureUser_RuntimeSystem/) {
	if ($dbg4) {
	    print(STDERR "$subname: no match for MM5ConfigureUser_RuntimeSystem: $MM5ConfigureUser_RuntimeSystem\n");
	}
	return($return_val, $is_match);
    }

    if ($inline !~ /\=/) {
	if ($dbg4) {
	    print(STDERR "$subname: no match for equals sign\n");
	}
	return($return_val, $is_match);
    }

    if ((index($inline, "=")) < (index($inline, "RUNTIME_SYSTEM"))) {
	if ($dbg4) {
	    print(STDERR "$subname: RUNTIME_SYSTEM is after the equals sign\n");
	}
	return($return_val, $is_match);
    }

    # Now need to extract the RUNTIME_SYSTEM value to do an exact cmp
    # The value after the equals sign may have quotes around it.

    chop($inline);
    ($junk, $wanted)=split('=', $inline);

    if ($dbg4) {
	print(STDERR "$subname: first split of equals sign, wanted: $wanted\n");
    }

    if ($wanted =~ /\"/) {
	($junk, $wanted)=split('\"', $wanted);
	if ($dbg4) {
	    print(STDERR "$subname: second split of quotes, wanted: ..$wanted..\n");
	}
    }
    
    if ($dbg4) {
	print(STDERR "$subname: try match test on length and string\n");
    }

    if (($is_match < 1) && 
	($wanted =~ /$MM5ConfigureUser_RuntimeSystem/) &&
	(length($wanted) == length($MM5ConfigureUser_RuntimeSystem))) {

	if ($dbg4) {
	    print(STDERR "\tfound match\n");
	}
	$is_match=1;
    }

    if ($dbg3) {
	print(STDERR "$subname: is_match: $is_match, wanted: $wanted\n");
    }

    return($return_val, $is_match);
}


#---------------------------------------------------------------------
# Subroutine: preBuildSetup
#
# Usage: ($return_val) = preBuildSetup($test, $debug)
#
# Function:   Do additional prebuild setup
#
# Input:      $test            test flag (1=yes, 0=no)
#             $debug           debug flag 
#
# Output:     $return_val      0 if errors in function, 1 on error
# 

sub preBuildSetup {
    my ($test, $debug) = @_;

    # Local variables

    my $return_val=1;
    my $subname="preBuildSetup";

    my ($use_cmd, $is_ok, $cmd_ok);

    # Create the tmp dir if it does not exist

    if ((!$test) && (!-e $TmpDir)) {
	$use_cmd="mkdir -p $TmpDir";
	($is_ok, $cmd_ok)=execSystemCall($use_cmd, $test, $debug);
    }

    # Done

    return($return_val);
}

#========================================= EOF =====================================
