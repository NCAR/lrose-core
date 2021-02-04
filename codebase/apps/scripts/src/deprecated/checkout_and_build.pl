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
# Name: checkout_and_build.pl
#
# Function:
#	Perl script to checkout source code from CVS and build it.
#
# Overview:
#       Checks out a distribs file from CVS and then executes it
#       to checkout a set of source code. Then does a series of
#       make installs on the checked out code.
#
# Usage:
#       checkout_and_build.pl -h
#
# Input:
#       1) An ASCII param file for this script
#       2) An ASCII distribs file with a set of 'cvs co' commands
#
# Output:
#
# Dependencies:
#	Perl must be available in the location at the top of this file.
# 	
# Author: Deirdre Garvey - NCAR/RAP/SDG
#
# 24-JUN-2003
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

sub badArg;
sub translateDataDir;
sub verifyDataDir;

# 
# External modules
#

use Time::Local;                   # make 'date' available

#
# --------------------------- Defaults and globals --------------
#
# Get the program basename.

($prog = $0) =~ s|.*/||;

# hostname

$HostName=`hostname`;
chop($HostName);

# host os

$HostOs=$ENV{'HOST_OS'};

# ... Like pound-defines ...
$ExitSuccess=0;
$ExitFailure=1;

$StringDefault="UNKNOWN";
$IntDefault=-1;
$FloatDefault=-1.0;

#
# Set command line defaults
#

$Debug=0;                           # Flag to print debug messages to STDERR
$Debug_level=0;                     # Level of debugging
$DoPrintParams=0;                   # Flag for print_params

#
# Save the usage to print to the user if there is a problem
#
$usage =                                                 
    "\n" .
    "Usage: $prog -p|-c <file> [-dh] <-v level>\n" .
    "Purpose: To checkout, build, and optionally install source code.\n" .
    "   -c --config <file>    : (Required.) The name of the config/param\n" .
    "                           file to read. NOT required if running\n" .
    "                           --print_params\n" .
    "   -d --debug            : Print debugging messages\n" .
    "   -h --help             : Print this usage message\n" .
    "   -p --print_params     : Generate an input config/param file for\n" .
    "                           this script. Writes to STDOUT.\n" .
    "   -v --verbose <num>    : A debug level number (1..2)\n" .
    "                           Will also turn on --debug\n";

# ----------------------------- Parse command line --------------------------
#
# Get the arguments from the command line
#
$result = &GetOptions('config=s',
		      'debug',
		      'help',
		      'print_params',
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

if (($opt_debug) || ($opt_level)) {
  $Debug=1;
  $Debug_level=1;
  print(STDERR "Input options specified...\n");
  print(STDERR "\tconfig: $opt_config\n");
  print(STDERR "\tverbosedebuglevel: $opt_verbose\n");
}

if ($opt_config) {
    $ConfigFile= $opt_config;
}

if ($opt_print_params) {
    $DoPrintParams=1;
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
$FoundErrors=0;

# Does the config file exist?

if ((!-e $ConfigFile) && (!$DoPrintParams)) {
  print(STDERR "ERROR: $prog: The config file $ConfigFile does not exist\n");
  $FoundErrors=1;
}

# Is the HOST_OS set to something?

if ($HostOs !~ /\w/) {
    print(STDERR "WARNING: $prog: HOST_OS environment variable is not set\n");
    print(STDERR "\twill set to a non-SUN default (affects cp options)\n");
    print(STDERR "\tif you are on a SUN, this will cause problems.\n");
    $HostOs=$StringDefault;
}

if ($FoundErrors) {
    exit $ExitFailure;
}

# --------------------------- Initialization -----------------------
#
# Get todays date for header

$Today=`date`;
chop($Today);

# Get the processing start time for statistics

$start_utime=time;

# Set the is_sun flag because of cp options (sigh...)

if ($HostOs =~ /SUNOS/) {
    $IsSun=1;
} else {
    $IsSun=0;
}
if ($Debug) {
    print(STDERR "Due to cp option differences, set IsSun: $IsSun, based on HOST_OS: $HostOs\n");
}

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

$gcpp_version=`g++ --version`;

# Get the PGI version since this is a source of confusion

$pgf90_version=`pgf90 -V`;

# Print banner

print(STDERR "*******************************************************************\n");
print(STDERR "Running $prog\n");
print(STDERR "\tHostname: ${HostName}\n");
print(STDERR "\tHOST_OS env var: ${HostOs}\n");
print(STDERR "\tg++ version: $gcpp_version");
print(STDERR "\tpgf90 version: $pgf90_version\n");
print(STDERR "\tStart time: ${Today}\n");
print(STDERR "*******************************************************************\n");

# Read the config/param file and set globals

$is_ok = &readParamFile($ConfigFile, $Debug_level);
if (!$is_ok) {
    print(STDERR "Exiting...\n");
    $ExitFailure;
}

# Reset env vars

if ($DoResetEnvToCheckout) {
    &resetEnvVars($CheckoutDir, $Debug_level);
}

if ($Debug) {
    &printEnvVars();
}

# Set the name of the source code checkout directory

$CheckoutCvsDir="${CheckoutDir}/cvs";

# Delete dirs before start

&printCurrentTime();
&deleteDirs($Debug_level);

# Create dirs

$is_ok=&createDirs($Debug_level);
if (!$is_ok) {
    print(STDERR "Exiting...\n");
    $ExitFailure;
}

# Checkout files

if ($DoCheckout) {
    &printCurrentTime();
    $is_ok=&checkoutFiles($Debug_level);
    if (!$is_ok) {
	print(STDERR "Exiting...\n");
	$ExitFailure;
    }
}

# Pre-Build

if ($DoPreBuild) {
    &printCurrentTime();
    $is_ok=&runExternalScript($DoMyPreBuildScript, $MyPreBuildScript, $Debug_level);
    if (!$is_ok) {
	print(STDERR "Exiting...\n");
	$ExitFailure;
    }
}

# Build

if ($DoBuild) {
    &printCurrentTime();
    $is_ok=&build($Debug_level);
    if (!$is_ok) {
	print(STDERR "Exiting...\n");
	$ExitFailure;
    }
}

# Remove non-static binaries

if ($DoRemoveNonStatic) {
    $is_ok=&removeNonStaticBinaries();
}

# Post-Build

if ($DoPostBuild) { 
    &printCurrentTime();
    $is_ok=&runExternalScript($DoMyPostBuildScript, $MyPostBuildScript, $Debug_level);
    if (!$is_ok) {
	print(STDERR "Exiting...\n");
	$ExitFailure;
    }
}

# Check build results

if ($DoBuildCheckScript) {
    &printCurrentTime();
    $is_ok=&runBuildCheckScript($Debug_level);
}

# Install

if ($DoInstall) {
    &printCurrentTime();
    $is_ok=&install($Debug_level);
    if (!$is_ok) {
	print(STDERR "Exiting...\n");
	$ExitFailure;
    }
}

# Cleanup

if ($DoMakeClean) {
    &printCurrentTime();
    $is_ok=makeClean();
}

# Done

&printCurrentTime();
$end_utime=time;
$elapsed_secs=$end_utime - $start_utime;
print(STDERR "Elapsed time (secs): $elapsed_secs\n");
exit $ExitSuccess;


# =============================== SUBROUTINES ===========================

#-------------------------------------------------------------------------
#
# Subroutine checkoutFiles
#
# Usage: $return_val = checkoutFiles()
#
# Function: checkout distribs file, modify with rtag if needed, then execute it
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
  local($is_ok, $co_ok, $cmd, $use_distribs_file, $dir, $file);

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

  chdir($CheckoutCvsDir);

  foreach $file (@DistribsFilesArr) {

      if ($debug) {
	  print(STDERR "Checkout distribs file: $file\n");
      }

      if ($DoRtag) {
	  $cmd="cvs co -r $Rtag $file";
      } else {
	  $cmd="cvs co $file";
      }
      $is_ok=system($cmd);
      $co_ok=WEXITSTATUS($?);
      if ($co_ok != 0) {
	  print(STDERR "ERROR! Cannot checkout the distribs file: ${file}\n");
	  return ($return_val);
      }

      # If rtag is specified, create a copy of the distribs file with the rtag specified

      $use_distribs_file=$file;

      if ($DoRtag) {
	  if ($debug) {
	      print(STDERR "Copying ${file} and modifying to use rtag checkout\n");
	  }
	
	  $cmd="sed s\/\"cvs co\"/\"cvs co -r ${Rtag}\"/ ${file} \> ${file}\.${Rtag}";
	  $is_ok=system($cmd);
	  
	  $cmd="chmod 777 ${file}.${Rtag}";
	  $is_ok=system($cmd);

	  $use_distribs_file="${file}.${Rtag}";
      }
  
      # Checkout all the files

      chdir($CheckoutCvsDir);
      if ($debug) {
	  print(STDERR "Running the distribs file: ${use_distribs_file}\n");
      }

      $cmd="${use_distribs_file}";
      $is_ok=system($cmd);
      
  } #endforeach

  # Special case -- we have to checkout empty directories explicitly since
  # explicitly since empty directories are NOT tagged in CVS.

  if ($DoRtag) {
      if ($debug) {
	  print(STDERR "Checking out empty directories explicitly\n");
	  print(STDERR "\tsince empty directories cannot be tagged. Must ignore tag\n");
      }
      chdir($CheckoutCvsDir);
      foreach $dir (@RtagEmptyDirsCoArr) {
	  $cmd="cvs co -A $dir";
	  $is_ok=system($cmd);
      }
  }

  # Done

  $return_val=1;
  return($return_val);

}

#-------------------------------------------------------------------------
#
# Subroutine runExternalScript
#
# Usage: $return_val = runExternalScript($flag, $script, $debug)
#
# Function: execute user external script
#
# Input:    $flag           flag to run the script (1=yes, 0=no)
#           $script         script to run
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub runExternalScript
{
  local ($flag, $script, $debug) = @_;

  # Local variables

  local($subname, $return_val);
  local($dbg2, $dbg3);
  local($is_ok, $cmd);

  # Set defaults

  $return_val=0;
  $subname="runExternalScript";

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

  # Run the user pre build script if set

  if ($flag == 1) {
      $cmd="$script";
      if ($debug) {
	  print(STDERR "$subname: Executing: $cmd\n");
      }
      system($cmd);
      $is_ok=WEXITSTATUS($?);
      print(STDERR "$subname: Return from $script: $is_ok\n");
  }

  # Done
  
  $return_val=1;
  return($return_val);
}

#-------------------------------------------------------------------------
#
# Subroutine build
#
# Usage: $return_val = build()
#
# Function: build libs, apps
#
# Input:    globals
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub build
{
  local ($debug) = @_;

  # Local variables

  local($subname, $return_val);
  local($dbg2, $dbg3);
  local($is_ok, $cmd, $nsdirs, *sdirs, $topsrcdir, $make_target);
  local($is_libs, $is_java, $ldir, $make_includes_target);

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

  # Special case, need to build tdrp_gen BEFORE can build
  # the libs directory

  if ($DoTdrpGenFirst) {
      if ($debug) {
	  print(STDERR "============== making tdrp_gen first ==========\n");
      }

      chdir("${CheckoutCvsDir}/libs/tdrp");
      $cmd="make -k ${MakeLibsTarget}";
      system($cmd);

      chdir("${CheckoutCvsDir}/apps/tdrp/src/tdrp_gen");
      $cmd="make -k ${MakeAppsTarget}";
      system($cmd);
  }

  # MAKE build -- do the build for the incs, libs, apps

  foreach $topsrcdir (@SrcDirsArr) {

      if ($debug) {
	  print(STDERR "============== making src dir: $topsrcdir ==========\n");
      }
      
      &printCurrentTime();

      if (($debug) && ($DoTdrpGenFirst)) {
	  print(STDERR "-- special check: do a which tdrp_gen here --\n");
	  $cmd="which tdrp_gen";
	  system($cmd);
      }

      # Setup the make targets

      $is_libs=0;
      $is_java=0;

      if ($topsrcdir eq "incs") {
	  $make_target="-k $MakeIncsTarget";
      }
      elsif ($topsrcdir =~ /libs/) {
	  $make_target="$MakeLibsTarget";
	  $make_includes_target="-k $MakeLibIncludesTarget";
	  $is_libs=1;
      }
      elsif ($topsrcdir eq "apps") {
	  $make_target="-k $MakeAppsTarget";
      }
      elsif ($topsrcdir =~ /java/) {
	  $make_target="$MakeJavaTarget";
	  $is_java=1;
      }
      elsif ($topsrcdir eq "matlab") {
	  $make_target="-k $MakeMatlabTarget";
      } else {
	  $make_target="-k $MakeGenericTarget";
      }

      # Make the libraries

      if ($is_libs) {

	  # Note that this is handled differently than the apps. We
	  # cannot just do a "make -k" here because this will erroneously
	  # install libraries that did not build completely.

	  # Install the library includes

	  chdir ("${CheckoutCvsDir}/${topsrcdir}");
	  $cmd="make ${make_includes_target}";
	  system($cmd);

	  # Get the list of libraries to build - try the little-m Makefile first,
	  # then the big-m Makefile if the first does not exist

	  ($is_ok, $nsdirs) = &parseMakefileSubdirs("${CheckoutCvsDir}/${topsrcdir}/makefile", *sdirs, $debug);
	  if (!$is_ok) {
	      ($is_ok, $nsdirs) = &parseMakefileSubdirs("${CheckoutCvsDir}/${topsrcdir}/Makefile", *sdirs, $debug);
	  }
	  if (!$is_ok) {
	      return($return_val);
	  }
	  
	  foreach $ldir (@sdirs) {
	      chdir ("${CheckoutCvsDir}/${topsrcdir}/${ldir}");
	      
	      # Special case for libjni.so -- fails to install due to
	      # archiver failure with the make install after the make opt.
	      # Cannot try to remake the .so file once it exists.
	      
	      if ($ldir eq "jni") {
		  $cmd="make install";
	      } else {
		  $cmd="make ${make_target}";
	      }
	      system($cmd);
	  }
      } #endif is_libs

      else {
	  chdir("${CheckoutCvsDir}/${topsrcdir}");

	  if ($is_java) {
	      $cmd="make -k";
	  }
	  $cmd="make ${make_target}";
	  system($cmd);
      }
  } #endforeach SrcDirs

  # Make sysview1 jar file, assumes you had java/src and java_apps in the
  # list of src_dirs

  if ($MakeSysView1JarFile) {

      $cmd="cp ${CheckoutCvsDir}/java_apps/SysView/src/*.class ${RAP_BIN_DIR}";
      system($cmd);
      chdir("${CheckoutCvsDir}/java/src/edu/ucar/rap/sysview");
      $cmd="make jar";
      system($cmd);
      $cmd="make install_jar";
      system($cmd);
      chdir("${CheckoutCvsDir}/java_apps/data_canvas_applets/src/new_taiwan");
      $cmd="ln -s ${CheckoutCvsDir}/java/src/edu edu";
      system($cmd);
      $cmd="make";
      system($cmd);
      $cmd="make jar";
      system($cmd);
      $cmd="make install_applet";
      system($cmd);
  }

  # Additional lib builds if set

  if ($DoProfileBuild) {
      if ($debug) {
	  print(STDERR "----- Building PROFILE libs -------\n");
      }

      &printCurrentTime();

      foreach $ldir (@sdirs) {
	  chdir ("${CheckoutCvsDir}/libs/${ldir}");
	  $cmd="make clean clean_all profile install_profile_lib clean_all";
	  system($cmd);
      }
  }

  if ($DoInsureBuild) {
      if ($debug) {
	  print(STDERR "----- Building INSURE libs -------\n");
      }
      foreach $ldir (@sdirs) {
	  chdir ("${CheckoutCvsDir}/libs/${ldir}");
	  $cmd="make clean clean_all insure install_insure_lib clean_all";
	  system($cmd);
      }
  }

  # Additional builds if set

  if ($DoPGIBuild) {
      if ($debug) {
	  print(STDERR "----- Building PGI libs -------\n");
      }

      &printCurrentTime();

      foreach $ldir (@sdirs) {
	  chdir ("${CheckoutCvsDir}/libs/${ldir}");
	  $cmd="make clean clean_all pgi install_pgi_lib clean_all";
	  system($cmd);
      }

      if ($debug) {
	  print(STDERR "----- Building PGI apps -------\n");
      }
      chdir ("${CheckoutCvsDir}/apps");
      $cmd="make -k clean clean_all pgi install_pgi_bin clean_all";
      system($cmd);
  }

  # ANT build -- do the build for java ant source

  foreach $topsrcdir (@AntSrcDirsArr) {

      if ($debug) {
	  print(STDERR "============== anting src dir: $topsrcdir ==========\n");
      }

      &printCurrentTime();

      chdir ("${CheckoutCvsDir}/${topsrcdir}");
      $cmd="ant ${AntJavaTarget}";
      system($cmd);

  } #endforeach AntSrcDirs

  # Run the user build script if set

  if ($DoMyBuildScript) {

      &printCurrentTime();

      $cmd="$MyBuildScript";
      if ($debug) {
	  print(STDERR "$subname: Executing: $cmd\n");
      }
      system($cmd);
      $is_ok=WEXITSTATUS($?);
      print(STDERR "$subname: Return from $MyBuildScript: $is_ok\n");
  }

  # Done
  
  $return_val=1;
  return($return_val);
}



#-------------------------------------------------------------------------
#
# Subroutine install
#
# Usage: $return_val = install()
#
# Function: install
#
# Input:    globals
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub install
{
  local ($debug) = @_;

  # Local variables

  local($subname, $return_val);
  local($dbg2, $dbg3);
  local($date_stamp, $dir, $cmd, $newdir, $is_ok, $source_dir, $target_dir);
  local($cp_options);

  # Set defaults

  $return_val=0;
  $subname="install";

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

  # Set the cp options depending on isSun

  if ($IsSun) {
      $cp_options="";
  } else {
      $cp_options="v";
  }

  # Move the install directories if set

  if ($DoMoveInstallDirs) {
      $date_stamp=`date -u '+%Y%m%d'`;
      chop($date_stamp);

      chdir($InstallTopDir);
      foreach $dir (@InstallDirsArr) {
	  if (!-d "${dir}") {
	      next;
	  }
	  $newdir="${dir}.${date_stamp}";
	  if (-d "${InstallTopDir}/${newdir}") {
	      print(STDERR "$subname: WARNING: $newdir already exists! moving to .bak\n");
	      $cmd="mv ${newdir} ${newdir}.bak";
	      system($cmd);
	  }

	  $cmd="mv $dir $newdir";
	  system($cmd);
	  $is_ok=WEXITSTATUS($?);
	  if ($is_ok != 0) {
	      print(STDERR "ERROR: $subname: Cannot run $cmd\n");
	      return($return_val);
	  }
      } #endforeach
  }

  # Copy the new files in

  foreach $dir (@InstallDirsArr) {
      $source_dir="${CheckoutDir}/${dir}";
      $cmd="cp -pr${cp_options} ${source_dir} $InstallTopDir";
      system($cmd);
  }
  foreach $dir (@InstallSrcDirsArr) {
      $source_dir="${CheckoutCvsDir}/${dir}";
      $cmd="cp -pr${cp_options} ${source_dir} $InstallTopDir";
      system($cmd);
  }

  # Run the user install script if set

  if ($DoMyInstallScript) {
      $cmd="$MyInstallScript";
      if ($debug) {
	  print(STDERR "$subname: Executing: $cmd\n");
      }
      system($cmd);
      $is_ok=WEXITSTATUS($?);
      print(STDERR "$subname: Return from $MyInstallScript: $is_ok\n");
  }

  # Done

  $return_val=1;
  return($return_val);
}


#-------------------------------------------------------------------------
#
# Subroutine runBuildCheckScript
#
# Usage: $return_val = runBuildCheckScript()
#
# Function: runs the build check script
#
# Input:    globals
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub runBuildCheckScript
{
  local ($debug) = @_;

  # Local variables

  local($subname, $return_val);
  local($dbg);

  # Set defaults

  $return_val=0;
  $subname="runBuildCheckScript";

  # Debugging

  $dbg=0;
  if ($debug > 0) {
      $dbg=1;
  }

  # Does the script exist?

  if (!-e $BuildCheckScript) {
      print(STDERR "ERROR: $subname: The build check script does not exist: $BuildCheckScript\n");
      return($return_val);
  }

  $cmd="${BuildCheckScript}";
  if (($BuildCheckParamApps ne $StringDefault) && ($BuildCheckParamApps =~ /\w/)) {
      $cmd="$cmd -a $BuildCheckParamApps";
  }

  if (($BuildCheckBinDir ne $StringDefault) && ($BuildCheckBinDir =~ /\w/))  {
      $cmd="$cmd -c $BuildCheckBinDir";
  }

  if (($BuildCheckLibDir ne $StringDefault) && ($BuildCheckLibDir =~ /\w/))  {
      $cmd="$cmd -i $BuildCheckLibDir";
  }

  if (($BuildCheckParamLibs ne $StringDefault) && ($BuildCheckParamLibs =~ /\w/)) {
      $cmd="$cmd -l $BuildCheckParamLibs";
  }

  if (($BuildCheckParamEmail ne $StringDefault) && ($BuildCheckParamEmail =~ /\w/))  {
      $cmd="$cmd -m $BuildCheckParamEmail";
  }

  if ($BuildCheckAlwaysSendEmail) {
      $cmd="$cmd -s";
  }

  if (($BuildCheckOutFile ne $StringDefault) && ($BuildCheckOutFile =~ /\w/)) {
      $cmd="$cmd -o $BuildCheckOutFile";
  }

  if (($BuildCheckWebLogFile ne $StringDefault) && ($BuildCheckWebLogFile =~ /\w/)) {
      $cmd="$cmd -w $BuildCheckWebLogFile";
  }

  if ($dbg) {
      $cmd="$cmd -v $debug";
  }

  if ($dbg) {
      print(STDERR "$subname: Running $cmd\n");
  }

  system($cmd);
  $return_val=WEXITSTATUS($?);
  if ($return_val < 0) {
      print(STDERR "ERROR: Could not run $cmd\n");
  }

  return($return_val);

}

#-------------------------------------------------------------------------
#
# Subroutine createDirs
#
# Usage: createDirs()
#
# Function: create dirs before checkout and build
#
# Input:    globals
#
# Output:   $return_val   1 on success, 0 on error
#
# Overview:
#

sub createDirs
{
  local ($debug) = @_;

  # Local variables

  local($subname, $return_val);
  local($dbg2, $dbg3);
  local($dir, $rdir, $is_ok, $make_dir);

  # Set defaults

  $return_val=0;
  $subname="createDirs";

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

  if (!-d $CheckoutDir) {
      $is_ok=system("mkdir -p $CheckoutDir");
      $make_dir=WEXITSTATUS($?);
      if ($make_dir != 0) {
	  print(STDERR "ERROR: Cannot create $CheckoutDir and it does not exist\n");
	  return($return_val);
      }
  }

  if (!-d $CheckoutCvsDir) {
      $is_ok=system("mkdir -p $CheckoutCvsDir");
      $make_dir=WEXITSTATUS($?);
      if ($make_dir != 0) {
	  print(STDERR "ERROR: Cannot create $CheckoutCvsDir and it does not exist\n");
	  return($return_val);
      }
  }

  # Create the build dirs

  if ($DoRemoveBuildDirs) {
      foreach $dir (@BuildDirsArr) {

	  $rdir="$CheckoutDir/$dir";
	  if (!-d $rdir) {
	      $is_ok=system("mkdir -p $rdir");
	      $make_dir=WEXITSTATUS($?);
	      if ($make_dir != 0) {
		  print(STDERR "ERROR: Cannot create $rdir and it does not exist\n");
		  return($return_val);
	      }
	  }
      }
  }

  # Create the install dirs

  if ($DoRemoveInstallDirs) {
      foreach $dir (@InstallDirsArr) {
	  $rdir="$InstallTopDir/$dir";
	  if (!-d $rdir) {
	      $is_ok=system("mkdir -p $rdir");
	      $make_dir=WEXITSTATUS($?);
	      if ($make_dir != 0) {
		  print(STDERR "ERROR: Cannot create $CheckoutCvsDir and it does not exist\n");
		  return($return_val);
	      }
	  }
      }
  }

  $return_val=1;
  return($return_val);

}

#-------------------------------------------------------------------------
#
# Subroutine deleteDirs
#
# Usage: deleteDirs()
#
# Function: delete dirs before checkout and build
#
# Input:    globals
#
# Output:   directories deleted
#
# Overview:
#

sub deleteDirs
{
  local ($debug) = @_;

  # Local variables

  local($subname);
  local($dbg2, $dbg3);
  local($dir, $rdir, $is_ok);

  # Set defaults

  $subname="deleteDirs";

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

  # Delete the source directory

  if ($DoRemoveSrcDirs) {
      $rdir="$CheckoutCvsDir";
      print(STDERR "Removing $rdir\n");
      $is_ok=system("/bin/rm -r $rdir/*");
  }

  # Delete the build dirs

  if ($DoRemoveBuildDirs) {
      foreach $dir (@BuildDirsArr) {
	  $rdir="$CheckoutDir/$dir";
	  print(STDERR "Removing $rdir\n");
	  $is_ok=system("/bin/rm -r $rdir/*");
      }
  }

  # Delete the install dirs

  if ($DoRemoveInstallDirs) {
      foreach $dir (@InstallDirsArr) {
	  $rdir="$InstallTopDir/$dir";
	  print(STDERR "Removing $rdir\n");
	  $is_ok=system("/bin/rm -r $rdir/*");
      }
      foreach $dir (@InstallSrcDirsArr) {
	  $rdir="$InstallTopDir/$dir";
	  print(STDERR "Removing $rdir\n");
	  $is_ok=system("/bin/rm -r $rdir/*");
      }
  }

}

#-------------------------------------------------------------------------
#
# Subroutine resetEnvVars
#
# Usage: resetEnvVars
#
# Function: Reset the env vars based on the checkout dir
#
# Input:    $checkout_dir     directory to reset env vars below
#           $debug
#
# Output:   all RAP_* env vars are reset
#
# Overview:
#

sub resetEnvVars
{
  local ($checkout_dir, $debug) = @_;

  # Local variables

  local($subname);
  local($dbg2, $dbg3);
  local($path);

  # Set defaults

  $subname="resetEnvVars";

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

  # Do the reset

  $ENV{'RAP_MAKE_INC_DIR'}="${checkout_dir}/cvs/make_include";
  $ENV{'RAP_MAKE_BIN_DIR'}="${checkout_dir}/cvs/make_bin";

  $ENV{'RAP_INC_DIR'}="${checkout_dir}/include";
  $ENV{'RAP_LIB_DIR'}="${checkout_dir}/lib";
  $ENV{'RAP_BIN_DIR'}="${checkout_dir}/bin";
  $ENV{'RAP_MAN_DIR'}="${checkout_dir}/man";
  $ENV{'RAP_DOC_DIR'}="${checkout_dir}/doc";
  $ENV{'RAP_JAVA_DIR'}="${checkout_dir}/java_packages";

  $ENV{'RAP_SHARED_INC_DIR'}="${checkout_dir}/include";
  $ENV{'RAP_SHARED_LIB_DIR'}="${checkout_dir}/lib";
  $ENV{'RAP_SHARED_BIN_DIR'}="${checkout_dir}/bin";
  $ENV{'RAP_SHARED_MAN_DIR'}="${checkout_dir}/man";
  $ENV{'RAP_SHARED_DOC_DIR'}="${checkout_dir}/doc";
  $ENV{'RAP_SHARED_JAVA_DIR'}="${checkout_dir}/java_packages";

  $ENV{'RAP_MATBIN_DIR'}="${checkout_dir}/matbin";
  $ENV{'RAP_SHARED_MATBIN_DIR'}="${checkout_dir}/matbin";

  $ENV{'RAP_DEBUG_LIB_DIR'}="${checkout_dir}/lib_debug";
  $ENV{'RAP_PROFILE_LIB_DIR'}="${checkout_dir}/lib_profile";
  $ENV{'RAP_INSURE_LIB_DIR'}="${checkout_dir}/lib_insure";
  $ENV{'RAP_PGI_LIB_DIR'}="${checkout_dir}/lib_pgi";

  # Reset the path to put the RAP_*_BIN first

  $path=$ENV{'PATH'};
  $ENV{'PATH'}="${checkout_dir}/bin:${path}";
}


#-------------------------------------------------------------------------
#
# Subroutine printEnvVars
#
# Usage: printEnvVars
#
# Function: Print the RAP_* env vars
#
# Input:    none
#
# Output:   
#
# Overview:
#

sub printEnvVars
{

  # Local variables

  local($subname);

  # Set defaults

  $subname="printEnvVars";

  print(STDERR "RAP_* environment variables...\n");
  print(STDERR "\tRAP_MAKE_INC_DIR: $RAP_MAKE_INC_DIR\n");
  print(STDERR "\tRAP_MAKE_BIN_DIR: $RAP_MAKE_BIN_DIR\n");

  print(STDERR "\tRAP_INC_DIR: $RAP_INC_DIR\n");
  print(STDERR "\tRAP_LIB_DIR: $RAP_LIB_DIR\n");
  print(STDERR "\tRAP_BIN_DIR: $RAP_BIN_DIR\n");
  print(STDERR "\tRAP_MAN_DIR: $RAP_MAN_DIR\n");
  print(STDERR "\tRAP_DOC_DIR: $RAP_DOC_DIR\n");
  print(STDERR "\tRAP_JAVA_DIR: $RAP_JAVA_DIR\n");

  print(STDERR "\tRAP_SHARED_INC_DIR: $RAP_SHARED_INC_DIR\n");
  print(STDERR "\tRAP_SHARED_LIB_DIR: $RAP_SHARED_LIB_DIR\n");
  print(STDERR "\tRAP_SHARED_BIN_DIR: $RAP_SHARED_BIN_DIR\n");
  print(STDERR "\tRAP_SHARED_MAN_DIR: $RAP_SHARED_MAN_DIR\n");
  print(STDERR "\tRAP_SHARED_DOC_DIR: $RAP_SHARED_DOC_DIR\n");
  print(STDERR "\tRAP_SHARED_JAVA_DIR: $RAP_SHARED_JAVA_DIR\n");

  print(STDERR "\tRAP_MATBIN_DIR: $RAP_MATBIN_DIR\n");
  print(STDERR "\tRAP_SHARED_MATBIN_DIR: $RAP_SHARED_MATBIN_DIR\n");
  
  print(STDERR "\tRAP_DEBUG_LIB_DIR: $RAP_DEBUG_LIB_DIR\n");
  print(STDERR "\tRAP_PROFILE_LIB_DIR: $RAP_PROFILE_LIB_DIR\n");
  print(STDERR "\tRAP_INSURE_LIB_DIR: $RAP_INSURE_LIB_DIR\n");
  print(STDERR "PATH: $PATH\n");
}


#-------------------------------------------------------------------------
#
# Subroutine readParamFile
#
# Usage: $return_val = readParamFile($param_file, $debug)
#
# Function: Read the $param_file.
#
# Input:    $param_file          file to read
#           $debug               flag for debugging (1=on, 0=off)
#
# Output:   $ret_val             1 on success, 0 on error
#
# Overview:
#

sub readParamFile
{
  local ($param_file, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($dbg2, $dbg3);
  local($is_ok, $keyword, $keyvalue, *out_arr, $type, $unstripped_keyvalue);
  local(*rtag_empty_dirs_arr);

  # Set defaults

  $return_val=0;
  $subname="readParamFile";

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

  # Debugging

  if ($dbg2) {
      print(STDERR "$prog: $subname: Input paramfile: $param_file\n");
  }

  # Set global parameter defaults

  $is_ok = &setAllParams2Defaults;

  # Open the param file

  $is_ok=open(PARAM_FILE, $param_file);
  if (!$is_ok) {
      print(STDERR "ERROR: $prog: $subname: Cannot open file $param_file\n");
      return($return_val);
  }

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

	  elsif ($keyword eq "rtag_empty_dirs_co") {
	      $type="comma_delimited_string";
	      ($is_ok, $RtagEmptyDirsCo) = &checkKeywordValue($keyword, $keyvalue, $type, *RtagEmptyDirsCoArr, $debug);
	  }

	  elsif ($keyword eq "rtag_empty_dirs_date") {
	      $type="string";
	      $RtagEmptyDirsDate = $keyvalue;
	  }

	  elsif ($keyword eq "do_reset_env_to_checkout") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoResetEnvToCheckout=1;
	      } else {
		  $DoResetEnvToCheckout=0;
	      }
	  }

	  elsif ($keyword eq "src_dirs") {
	      $type="comma_delimited_string";
	      ($is_ok, $SrcDirs) = &checkKeywordValue($keyword, $keyvalue, $type, *SrcDirsArr, $debug);
	  }

	  elsif ($keyword eq "ant_src_dirs") {
	      $type="comma_delimited_string";
	      ($is_ok, $AntSrcDirs) = &checkKeywordValue($keyword, $keyvalue, $type, *AntSrcDirsArr, $debug);
	  }

	  elsif ($keyword eq "do_build") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoBuild=1;
	      } else {
		  $DoBuild=0;
	      }
	  }

	  elsif ($keyword eq "do_install") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoInstall=1;
	      } else {
		  $DoInstall=0;
	      }
	  }

	  elsif ($keyword eq "install_top_dir") {
	      $type="string";
	      $InstallTopDir= $keyvalue;
	      ($is_ok, $InstallTopDir) = &expandEnvVar($InstallTopDir, $debug);
	  }

	  elsif ($keyword eq "build_dirs") {
	      $type="comma_delimited_string";
	      ($is_ok, $BuildDirs) = &checkKeywordValue($keyword, $keyvalue, $type, *BuildDirsArr, $debug);
	  }

	  elsif ($keyword eq "install_dirs") {
	      $type="comma_delimited_string";
	      ($is_ok, $InstallDirs) = &checkKeywordValue($keyword, $keyvalue, $type, *InstallDirsArr, $debug);
	  }

	  elsif ($keyword eq "install_src_dirs") {
	      $type="comma_delimited_string";
	      ($is_ok, $InstallSrcDirs) = &checkKeywordValue($keyword, $keyvalue, $type, *InstallSrcDirsArr, $debug);
	  }

	  elsif ($keyword eq "do_remove_src_dirs") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoRemoveSrcDirs=1;
	      } else {
		  $DoRemoveSrcDirs=0;
	      }
	  }

	  elsif ($keyword eq "do_remove_build_dirs") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoRemoveBuildDirs=1;
	      } else {
		  $DoRemoveBuildDirs=0;
	      }
	  }

	  elsif ($keyword eq "do_remove_install_dirs") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoRemoveInstallDirs=1;
	      } else {
		  $DoRemoveInstallDirs=0;
	      }
	  }

	  elsif ($keyword eq "do_move_install_dirs") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoMoveInstallDirs=1;
	      } else {
		  $DoMoveInstallDirs=0;
	      }
	  }

	  elsif ($keyword eq "do_make_clean") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoMakeClean=1;
	      } else {
		  $DoMakeClean=0;
	      }
	  }

	  elsif ($keyword eq "do_remove_nonstatic") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoRemoveNonStatic=1;
	      } else {
		  $DoRemoveNonStatic=0;
	      }
	  }

	  elsif ($keyword eq "make_incs_target") {
	      $type="string";
	      $MakeIncsTarget = $unstripped_keyvalue;
	  }

	  elsif ($keyword eq "make_libs_target") {
	      $type="string";
	      $MakeLibsTarget = $unstripped_keyvalue;
	  }

	  elsif ($keyword eq "make_libincs_target") {
	      $type="string";
	      $MakeLibIncludesTarget = $unstripped_keyvalue;
	  }

	  elsif ($keyword eq "make_apps_target") {
	      $type="string";
	      $MakeAppsTarget = $unstripped_keyvalue;
	  }

	  elsif ($keyword eq "make_matlab_target") {
	      $type="string";
	      $MakeMatlabTarget = $unstripped_keyvalue;
	  }

	  elsif ($keyword eq "make_generic_target") {
	      $type="string";
	      $MakeGenericTarget = $unstripped_keyvalue;
	  }

	  elsif ($keyword eq "make_java_target") {
	      $type="string";
	      $MakeJavaTarget = $unstripped_keyvalue;
	  }

	  elsif ($keyword eq "ant_java_target") {
	      $type="string";
	      $AntJavaTarget = $unstripped_keyvalue;
	  }

	  elsif ($keyword eq "do_tdrp_gen_first") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoTdrpGenFirst=1;
	      } else {
		  $DoTdrpGenFirst=0;
	      }
	  }

	  elsif ($keyword eq "make_sysview1_jarfile") {
	      print(STDERR "WARNING: make_sysview1_jarfile builds an obsolete application!\n");
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $MakeSysView1JarFile=1;
	      } else {
		  $MakeSysView1JarFile=0;
	      }
	  }

	  elsif ($keyword eq "do_insure_build") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoInsureBuild=1;
	      } else {
		  $DoInsureBuild=0;
	      }
	  }

	  elsif ($keyword eq "do_profile_build") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoProfileBuild=1;
	      } else {
		  $DoProfileBuild=0;
	      }
	  }

	  elsif ($keyword eq "do_pgi_build") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoPGIBuild=1;
	      } else {
		  $DoPGIBuild=0;
	      }
	  }

	  elsif ($keyword eq "my_prebuild_script") {
	      $type="string";
	      $MyPreBuildScript = $unstripped_keyvalue;
	      ($is_ok, $MyPreBuildScript) = &expandEnvVar($MyPreBuildScript, $debug);
	      if (($MyPreBuildScript =~ /\w/) && ($MyPreBuildScript ne $StringDefault)){
		  $DoMyPreBuildScript=1;
		  $DoPreBuild=1;
	      } else {
		  $DoMyPreBuildScript=0;
		  $DoPreBuild=0;
	      }
	  }

	  elsif ($keyword eq "my_build_script") {
	      $type="string";
	      $MyBuildScript = $unstripped_keyvalue;
	      ($is_ok, $MyBuildScript) = &expandEnvVar($MyBuildScript, $debug);
	      if (($MyBuildScript =~ /\w/) && ($MyBuildScript ne $StringDefault)){
		  $DoMyBuildScript=1;
	      } else {
		  $DoMyBuildScript=0;
	      }
	  }

	  elsif ($keyword eq "my_install_script") {
	      $type="string";
	      $MyInstallScript = $unstripped_keyvalue;
	      ($is_ok, $MyInstallScript) = &expandEnvVar($MyInstallScript, $debug);
	      if (($MyInstallScript =~ /\w/) && ($MyInstallScript ne $StringDefault)){
		  $DoMyInstallScript=1;
	      } else {
		  $DoMyInstallScript=0;
	      }
	  }

	  elsif ($keyword eq "my_postbuild_script") {
	      $type="string";
	      $MyPostBuildScript = $unstripped_keyvalue;
	      ($is_ok, $MyPostBuildScript) = &expandEnvVar($MyPostBuildScript, $debug);
	      if (($MyPostBuildScript =~ /\w/) && ($MyPostBuildScript ne $StringDefault)){
		  $DoMyPostBuildScript=1;
		  $DoPostBuild=1;
	      } else {
		  $DoMyPostBuildScript=0;
		  $DoPostBuild=0;
	      }
	  }

	  elsif ($keyword eq "build_check_script") {
	      $type="string";
	      $BuildCheckScript = $keyvalue;
	      ($is_ok, $BuildCheckScript) = &expandEnvVar($BuildCheckScript, $debug);
	      if (($BuildCheckScript =~ /\w/) && ($BuildCheckScript ne $StringDefault)){
		  $DoBuildCheckScript=1;
	      } else {
		  $DoBuildCheckScript=0;
	      }
	  }

	  elsif ($keyword eq "build_check_ndays") {
	      $type="int";
	      $NumDays=$keyvalue;
	      ($is_ok, $BuildCheckNDays) = &checkKeywordValue($keyword, $keyvalue, $type, *out_arr, $debug);
	  }

	  elsif ($keyword eq "build_check_always_send_email") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $BuildCheckAlwaysSendEmail=1;
	      } else {
		  $BuildCheckAlwaysSendEmail=0;
	      }
	  }

	  elsif ($keyword eq "build_check_lib_dir") {
	      $type="string";
	      $BuildCheckLibDir = $keyvalue;
	      ($is_ok, $BuildCheckLibDir) = &expandEnvVar($BuildCheckLibDir, $debug);
	  }

	  elsif ($keyword eq "build_check_bin_dir") {
	      $type="string";
	      $BuildCheckBinDir = $keyvalue;
	      ($is_ok, $BuildCheckBinDir) = &expandEnvVar($BuildCheckBinDir, $debug);
	  }

	  elsif ($keyword eq "build_check_param_email") {
	      $type="string";
	      $BuildCheckParamEmail = $keyvalue;
	      ($is_ok, $BuildCheckParamEmail) = &expandEnvVar($BuildCheckParamEmail, $debug);
	  }

	  elsif ($keyword eq "build_check_param_libs") {
	      $type="string";
	      $BuildCheckParamLibs = $keyvalue;
	      ($is_ok, $BuildCheckParamLibs) = &expandEnvVar($BuildCheckParamLibs, $debug);
	  }
	  elsif ($keyword eq "build_check_param_apps") {
	      $type="string";
	      $BuildCheckParamApps = $keyvalue;
	      ($is_ok, $BuildCheckParamApps) = &expandEnvVar($BuildCheckParamApps, $debug);
	  }

	  elsif ($keyword eq "build_check_out_file") {
	      $type="string";
	      $BuildCheckOutFile = $keyvalue;
	      ($is_ok, $BuildCheckOutFile) = &expandEnvVar($BuildCheckOutFile, $debug);
	  }

	  elsif ($keyword eq "build_check_weblog_file") {
	      $type="string";
	      $BuildCheckWebLogFile = $keyvalue;
	      ($is_ok, $BuildCheckWebLogFile) = &expandEnvVar($BuildCheckWebLogFile, $debug);
	  }

	  else {
	      print(STDERR "ERROR: Cannot parse keyword line: $line");
	  }
	  
	  # End of keyword processing

	  next;
      }
	  
  } #endwhile

  close(PARAM_FILE);

  # ...Handle special cases...

  # Okay, did everything get set???

  ($is_ok, $missing) = &checkAllParams;

  if ($debug) {
      &printAllParams;
  }

  # Debugging

  # Set return

  if ($missing > 0) {
      $return_val = 0;
  } else {
      $return_val = 1;
  }

  # Debug
  
  if ($dbg2) {
    print(STDERR "$subname: return: $return_val, missing: $missing\n");
  }

  # Done
  return($return_val);
}


#------------------------------------------------------------------------
#
# Subroutine checkKeywordValue
#
# Usage: ($return_val, $valid_keyword_value) = 
#              checkKeywordValue($keyword, $input_keyword_value, $type, 
#                                *out_arr, $debug)
#
# Function: Check the $input_keyword_value against the $type and return
#           a valid value in $valid_keyword_value.
#
# Input:    $keyword             keyword, needed for error messages
#           $input_keyword_value value to check
#           $type                type to check
#           $debug               flag for debugging (1=on, 0=off)
#
# Output:   $return_val          1 on success, 0 on error
#           $valid_keyword_value valid value for the keyword, may be the
#                                  same as the $input_keyword_value
#           *out_arr             output array (only used for some keywords)
#
# Overview:
#

sub checkKeywordValue
{
  local ($keyword, $input_keyword_value, $type, *out_arr, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($dbg2, $dbg3);
  local($found, $found_valid, $valid_keyword_value, $tmp_keyword_value);
  local($is_ok, $narr, $val);

  # Set defaults

  $return_val=0;
  $subname="checkKeywordValue";

  # Debug

  if ($debug == 2) {
      $dbg2=1;
  } else {
      $dbg2=0;
  }

  if ($debug == 3) {
      $dbg3=1;
      $dbg2=1;
  } else {
      $dbg3=0;
  }

  # Set the return value to equal the input value by default

  $valid_keyword_value=$input_keyword_value;

  # Deal with the various types if possible

  $found=1;

  # Is an int positive?

  if ($type eq "int"){

      if ($input_keyword_value < 0) {
	  print(STDERR "ERROR: $keyword $input_keyword_value is less than 0, will set to 0\n");
	  $valid_keyword_value=0;
      } 
  }

  # Is a float positive?

  if ($type eq "float"){

      if ($input_keyword_value < 0) {
	  print(STDERR "ERROR: $keyword $input_keyword_value is less than 0, will set to 0\n");
	  $valid_keyword_value=0.0;
      } 
  }

  # If a comma-delimited string, split the string

  elsif ($type = "comma_delimited_string") {
      ($is_ok, $input_keyword_value) = &expandEnvVar($input_keyword_value, $debug);
      @out_arr=split(',',$input_keyword_value);
      $narr=@out_arr;

      if ($dbg3) {
	  print(STDERR "$subname: keyword: $keyword, keyvalue: $keyvalue, numarr: $narr\n");
	  foreach $val (@out_arr) {
	      print(STDERR "\t$val\n");
	  }
      }

  } 

  # Error condition

  else {
      if ($debug) {
	  print(STDERR "$prog: $subname: Cannot resolve type: $type\n");
      }
      $found=0;
  }

  # Done

  $return_val=$found;
  return($return_val, $valid_keyword_value);
}

#------------------------------------------------------------------------------
#
# Subroutine removeSpaces
#
# Usage: ($return_val, $newstring)=removeSpaces($instring, $debug) 
#
# Function: Remove leading and trailing spaces from $instring and
#           return in $newstring. If no spaces are found, returns
#           the $instring.
#
# Input:    $instring            string to remove blanks from
#           $debug               flag for debugging (1=on, 0=off)
#
# Output:   $ret_val             1 on success, 0 on error
#           $newstring           $instring without leading/trailing
#                                   blanks
#
# Overview:
#

sub removeSpaces
{
  local ($instring, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($dbg2);
  local($is_ok, $newstring);

  # Set defaults

  $return_val=0;
  $subname="removeSpaces";
  $newstring=$instring;

  # Debug

  if ($debug == 2) {
      $dbg2=1;
  } else {
      $dbg2=0;
  }

  # Do it

  ($newstring) = split (' ', $instring);

  # Done

  $return_val=1;
  return($return_val, $newstring);
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

  local($return_val, $subname);

  # Set defaults

  $return_val=0;
  $subname="generateParamFile";

  # Start writing param stuff to STDOUT

  # ... print header ...

  print(STDOUT "# Parameters for $prog\n");
  print(STDOUT "#\n");
  print(STDOUT "# This param file was generated on: $Today\n");
  print(STDOUT "# Environment variables can be used but must be inside ()\n");
  print(STDOUT "#=====================================================\n");

  # ... do checkout ...

  print(STDOUT "#------------ Do checkout ----------------\n");
  print(STDOUT "# do_checkout is a flag to run the distribs file to checkout\n");
  print(STDOUT "# the source code from CVS using the distribs file.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_checkout = TRUE\n\n");

  # ... checkout dir ...

  print(STDOUT "#------------ Checkout dir ----------------\n");
  print(STDOUT "# checkout_dir is the directory to checkout the source code into.\n");
  print(STDOUT "# It is strongly suggested that you put this is a separate disk\n");
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

  # ... distribs file ...

  print(STDOUT "#------------ Distribs file ----------------\n");
  print(STDOUT "# distribs_files is the CVS paths to the distribs files to checkout\n");
  print(STDOUT "# from CVS and run and checkout the desired source code\n");
  print(STDOUT "# Type: string, must be a comma-separated list of CVS paths to\n");
  print(STDOUT "#       distribs files with no blanks or quotes\n");
  print(STDOUT "#\n");
  print(STDOUT "distribs_files = \n\n");

  # ... do reset env ...

  print(STDOUT "#------------ Do reset env ----------------\n");
  print(STDOUT "# do_reset_env_to_checkout is a flag to reset all the RAP_*\n");
  print(STDOUT "# environment variables relative to the checkout_dir and.\n");
  print(STDOUT "# ignore the current environment. Note that this will force\n");
  print(STDOUT "# your build to ignore any libs or includes in /rap and is\n");
  print(STDOUT "# strongly suggested to be sure you have a clean build.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_reset_env_to_checkout = TRUE\n\n");

  # ... src dirs ...

  print(STDOUT "#------------ Src dirs ----------------\n");
  print(STDOUT "# src_dirs is a list of top-level CVS directories to look\n");
  print(STDOUT "# for a top-level Makefile (not ant file). The order is very important\n");
  print(STDOUT "# as the build will use the order you specify and if you put\n");
  print(STDOUT "# libs after apps, the apps may not have any libraries.\n");
  print(STDOUT "# Type: string, must be a comma-separated list of CVS top-level\n");
  print(STDOUT "#       directories with no blanks or quotes\n");
  print(STDOUT "#\n");
  print(STDOUT "src_dirs = incs,libs,apps\n\n");

  print(STDOUT "#------------ Ant src dirs ----------------\n");
  print(STDOUT "# ant_src_dirs is a list of top-level CVS directories to look\n");
  print(STDOUT "# for a top-level ant file (not Makefile) in. The order is very important\n");
  print(STDOUT "# as the build will use the order you specify.\n");
  print(STDOUT "# Type: string, must be a comma-separated list of CVS top-level\n");
  print(STDOUT "#       directories with no blanks or quotes\n");
  print(STDOUT "#\n");
  print(STDOUT "ant_src_dirs = \n\n");

  # ... do build ...

  print(STDOUT "#------------ Do build ----------------\n");
  print(STDOUT "# do_build is a flag to build the source code checked out into the\n");
  print(STDOUT "# checkout_dir\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_build = TRUE\n\n");

  # ... build dirs ...

  print(STDOUT "#------------ Build dirs ----------------\n");
  print(STDOUT "# build_dirs is a list of directories under the checkout_dir\n");
  print(STDOUT "# that will contain built code. This is used in do_remove_build_dirs\n");
  print(STDOUT "# Type: string, must be a comma-separated list of top-level\n");
  print(STDOUT "#       directories under checkout_dir with no blanks or quotes\n");
  print(STDOUT "#\n");
  print(STDOUT "build_dirs = bin,include,lib\n\n");

  # ... do install ...

  print(STDOUT "#------------ Do install ----------------\n");
  print(STDOUT "# do_install is a flag to copy the install_dirs directories\n");
  print(STDOUT "# from the checkout_dir to the install_top_dir and the install_src_dirs\n");
  print(STDOUT "# from the checkout_dir/cvs to the install_top_dir\n");
  print(STDOUT "# This only makes sense if you set do_reset_env_to_checkout\n");
  print(STDOUT "# to TRUE; otherwise the code will be installed into your\n");
  print(STDOUT "# RAP_* areas by the make install commands in this script.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_install = FALSE\n\n");

  # ... install dirs ...

  print(STDOUT "#------------ Install dirs ----------------\n");
  print(STDOUT "# install_dirs is a list of directories under the install_top_dir\n");
  print(STDOUT "# This is used in do_remove_install_dirs and in do_install to copy\n");
  print(STDOUT "# files and directories from checkout_dir install_dirs to install_top_dir.\n");
  print(STDOUT "# Type: string, must be a comma-separated list of directories\n");
  print(STDOUT "#       under install_top_dir with no blanks or quotes\n");
  print(STDOUT "#\n");
  print(STDOUT "install_dirs = bin,include,lib\n\n");

  print(STDOUT "#------------ Install src dirs ----------------\n");
  print(STDOUT "# install_src_dirs is a list of directories under the install_top_dir\n");
  print(STDOUT "# This is used in do_remove_install_dirs and in do_install to copy\n");
  print(STDOUT "# files and directories from checkout_dir/cvs install_dirs to install_top_dir.\n");
  print(STDOUT "# This is different than install_dirs and would be used for directories\n");
  print(STDOUT "# like make_include and make_bin which are not \"built\"\n");
  print(STDOUT "# Type: string, must be a comma-separated list of directories\n");
  print(STDOUT "#       under install_top_dir with no blanks or quotes\n");
  print(STDOUT "#\n");
  print(STDOUT "install_src_dirs = \n\n");

  # ... install top dir ...

  print(STDOUT "#------------ Install top dir ----------------\n");
  print(STDOUT "# install_top_dir is the top-level directory under which to copy\n");
  print(STDOUT "# the include, lib, bin files from the checkout and build area\n");
  print(STDOUT "# into. Be aware that if do_reset_env_to_checkout is TRUE, all the\n");
  print(STDOUT "# RAP_* env vars will be reset in this script.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "install_top_dir = \n\n");

  # ... cleanup ...

  print(STDOUT "#====================== Cleanup ========================\n");
  print(STDOUT "#------------ Cleanup src dirs----------------\n");
  print(STDOUT "# do_remove_src_dirs is a flag to remove the checkout_dir/cvs\n");
  print(STDOUT "# directory (all source code) before doing a checkout. This is\n");
  print(STDOUT "# strongly suggested!\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_remove_src_dirs = TRUE\n\n");

  print(STDOUT "#------------ Cleanup build dirs----------------\n");
  print(STDOUT "# do_remove_build_dirs is a flag to remove the build directories\n");
  print(STDOUT "# under the checkout_dir before starting the build. The build_dirs\n");
  print(STDOUT "# list is set above. This is strongly suggested if \n");
  print(STDOUT "# do_reset_env_to_checkout is TRUE\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_remove_build_dirs = TRUE\n\n");

  print(STDOUT "#------------ Cleanup install dirs----------------\n");
  print(STDOUT "# do_remove_install_dirs is a flag to remove the install_dirs and\n");
  print(STDOUT "# install_src_dirs lists of directories under the install_top_dir\n");
  print(STDOUT "# before beginning the build.\n");
  print(STDOUT "# **** BE VERY CAREFUL with this option! ****\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_remove_install_dirs = FALSE\n\n");

  print(STDOUT "# do_move_install_dirs is a flag to move the existing install dirs\n");
  print(STDOUT "# under the install_top_dir to a subdirectory with the current date\n");
  print(STDOUT "# appended to it. The build dirs will then be copied into the\n");
  print(STDOUT "# install_top_dir. The move is done after the build is completed.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_move_install_dirs = FALSE\n\n");

  print(STDOUT "#------------ Do make clean ----------------\n");
  print(STDOUT "# do_make_clean is a flag to run make clean in the source dirs\n");
  print(STDOUT "# after the build has completed.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_make_clean = TRUE\n\n");

  print(STDOUT "#------------ Do remove non-static binaries ----------------\n");
  print(STDOUT "# do_remove_nonstatic is a flag to remove any nonstatic binaries\n");
  print(STDOUT "# from the checkout_dir bin directory before doing the install.\n");
  print(STDOUT "# This only makes sense if you are doing static builds and is\n");
  print(STDOUT "# useful because static build failures can be covered up by\n");
  print(STDOUT "# success from install_shared target\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_remove_nonstatic = FALSE\n\n");

  # ... make targets ...

  print(STDOUT "#========================= Make targets =====================\n");
  print(STDOUT "#------------ Make incs target ----------------\n");
  print(STDOUT "# make_incs_target is the make target to use when installing\n");
  print(STDOUT "# the incs (this is the CVS incs dir, not the libs include files\n");
  print(STDOUT "# This target must exist in the incs Makefiles.\n");
  print(STDOUT "# Type: string. Examples: install or install_shared\n");
  print(STDOUT "#\n");
  print(STDOUT "make_incs_target = install\n\n");

  print(STDOUT "#------------ Make libs target ----------------\n");
  print(STDOUT "# make_libs_target is the make target to use when compiling\n");
  print(STDOUT "# the libs. It will be used for directories in the libs tree.\n");
  print(STDOUT "# This target must exist in all your library Makefiles.\n");
  print(STDOUT "# Type: string. Examples: install, install_shared, or opt install\n");
  print(STDOUT "#               to do an optimized install\n");
  print(STDOUT "#\n");
  print(STDOUT "make_libs_target = install\n\n");

  print(STDOUT "# make_libincludes_target is the make target to use when compiling\n");
  print(STDOUT "# the libs to install the libs include files. This target must exist\n");
  print(STDOUT "# in all your library Makefiles.\n");
  print(STDOUT "# Type: string. Options: install_include or install_shared_include\n");
  print(STDOUT "#\n");
  print(STDOUT "make_libincs_target = install_include\n\n");

  print(STDOUT "#------------ Make apps target ----------------\n");
  print(STDOUT "# make_apps_target is the make target to use when compiling\n");
  print(STDOUT "# the apps. It will be used for directories in the apps tree\n");
  print(STDOUT "# This target must exist in all your apps Makefiles.\n");
  print(STDOUT "# Type: string. Examples: install, install_shared, or\n");
  print(STDOUT "#               staticopt strip copy_bin to do optimized static install\n");
  print(STDOUT "#\n");
  print(STDOUT "make_apps_target = install\n\n");

  print(STDOUT "#------------ Make matlab target ----------------\n");
  print(STDOUT "# make_matlab_target is the make target to use when compiling\n");
  print(STDOUT "# matlab. It will be used for directories in the matlab tree.\n");
  print(STDOUT "# This target must exist in all your matlab Makefiles.\n");
  print(STDOUT "# Type: string. Examples: install, install_shared\n");
  print(STDOUT "#\n");
  print(STDOUT "make_matlab_target = install\n\n");

  print(STDOUT "#------------ Make java target ----------------\n");
  print(STDOUT "# make_java_target is the make target to use when compiling\n");
  print(STDOUT "# java code. It will be used for directories in the java* trees.\n");
  print(STDOUT "# This target must exist in all your java Makefiles.\n");
  print(STDOUT "# You must include java/src or whatever in the src_dirs\n");
  print(STDOUT "# NOTE: If you are using ant for java, leave this value blank and\n");
  print(STDOUT "# set ant_java_target instead\n");
  print(STDOUT "# Type: string. Examples: install_java\n");
  print(STDOUT "#\n");
  print(STDOUT "make_java_target = install_java\n\n");

  # ... make targets ...

  print(STDOUT "#========================= Ant targets =====================\n");

  print(STDOUT "#------------ Ant java target ----------------\n");
  print(STDOUT "# ant_java_target is the ant target to use when compiling\n");
  print(STDOUT "# java code. It will be used for directories in the java* trees.\n");
  print(STDOUT "# This target must exist in all your java ant files.\n");
  print(STDOUT "# You must include java/src or whatever in the src_dirs\n");
  print(STDOUT "# Type: string. Examples: dist\n");
  print(STDOUT "#\n");
  print(STDOUT "ant_java_target = dist\n\n");

  # ... do tdrp_gen first ...

  print(STDOUT "# ======================== Special cases ====================\n");
  print(STDOUT "#------------ Do tdrp_gen first ----------------\n");
  print(STDOUT "# do_tdrp_gen_first  is a flag to build and install tdrp_gen\n");
  print(STDOUT "# before any other libs or apps. This is required because some\n");
  print(STDOUT "# libs and apps depend on tdrp_gen to compile\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_tdrp_gen_first = TRUE\n\n");

  # ... additional libs builds ...

  print(STDOUT "#===================== Additional lib builds ===============\n");
  print(STDOUT "#------------ Insure and profile builds ----------------\n");
  print(STDOUT "# do_insure_build is a flag to build and install the libs\n");
  print(STDOUT "# compiled with insure. Insure must be available on this host.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_insure_build = FALSE\n\n");

  print(STDOUT "# do_profile_build is a flag to build and install the libs\n");
  print(STDOUT "# compiled with profiling.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_profile_build = FALSE\n\n");

  print(STDOUT "# do_pgi_build is a flag to build and install the libs AND APPS\n");
  print(STDOUT "# compiled with PGI compilers.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_pgi_build = FALSE\n\n");
  
  # ... additional scripts ...

  print(STDOUT "#===================== External scripts ===============\n");
  print(STDOUT "#------------ My prebuild script ----------------\n");
  print(STDOUT "# my_prebuild_script is a script to call after the checkout\n");
  print(STDOUT "# but before the build. Leave blank to not run an additional\n");
  print(STDOUT "# script. Do not enclose in quotes, either single or double\n");
  print(STDOUT "# since this causes problems with the system() call\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "my_prebuild_script=\n\n");

  print(STDOUT "#------------ My build script ----------------\n");
  print(STDOUT "# my_build_script is a script to call after all the src_dirs\n");
  print(STDOUT "# have been compiled. Only valid if do_build is TRUE. Leave\n");
  print(STDOUT "# blank to not run an additional script. Do not enclose in\n");
  print(STDOUT "# quotes, either single or double since this causes problems with\n");
  print(STDOUT "# the system() call\n");
  print(STDOUT "# Type: string.\n");
  print(STDOUT "#\n");
  print(STDOUT "my_build_script = \n\n");

  print(STDOUT "#------------ My postbuild script ----------------\n");
  print(STDOUT "# my_postbuild_script is a script to call after the build\n");
  print(STDOUT "# but before the install. Leave blank to not run an additional\n");
  print(STDOUT "# script. Do not enclose in quotes, either single or double\n");
  print(STDOUT "# since this causes problems with the system() call\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "my_postbuild_script=\n\n");

  print(STDOUT "#------------ My install script ----------------\n");
  print(STDOUT "# my_install_script is a script to call after the install is\n");
  print(STDOUT "# completed. Only valid if do_install is TRUE. Leave\n");
  print(STDOUT "# blank to not run an additional script. Do not enclose in\n");
  print(STDOUT "# quotes, either single or double since this causes problems with\n");
  print(STDOUT "# the system() call\n");
  print(STDOUT "# Type: string.\n");
  print(STDOUT "#\n");
  print(STDOUT "my_install_script = \n\n");

  # ... rtag ...

  print(STDOUT "#========================== Rtag ===========================\n");
  print(STDOUT "#------------ Rtag ----------------\n");
  print(STDOUT "# rtag is the name of a CVS rtag to use when checking out the\n");
  print(STDOUT "# distribs file and all the cvs co commands in the distribs file.\n");
  print(STDOUT "# You must have previously tagged all your desired source code\n");
  print(STDOUT "# with the specified CVS tag. Leave blank to checkout the latest\n");
  print(STDOUT "# code and not use a CVS tag.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "rtag = \n\n");

  print(STDOUT "#------------ Rtag empty dirs ----------------\n");
  print(STDOUT "# rtag_empty_dirs_co is a special case to checkout any empty\n");
  print(STDOUT "# directories explicitly when using an rtag. These directories\n");
  print(STDOUT "# will be checked out using the specified date in rtag_empty_dirs_date\n");
  print(STDOUT "# or with what is current in CVS if rtag_empty_dirs_date is not set\n");
  print(STDOUT "# Only valid if rtag is set\n");
  print(STDOUT "# Type: string, must be a comma-separated list of CVS directories\n");
  print(STDOUT "#       with no blanks or quotes\n");
  print(STDOUT "#\n");
  print(STDOUT "rtag_empty_dirs_co = \n\n");

  print(STDOUT "#------------ Rtag empty dirs date ----------------\n");
  print(STDOUT "# rtag_empty_dirs_date is used when checking out the rtag_empty_dirs_co\n");
  print(STDOUT "# list of directories. Leave blank to checkout the latest from CVS\n");
  print(STDOUT "# Type: string, must be a valid CVS -D date option\n");
  print(STDOUT "#\n");
  print(STDOUT "rtag_empty_dirs_date = \n\n");

  # ... build check script ...

  print(STDOUT "#==================== Build check ========================\n");
  print(STDOUT "#------------ Build check script ----------------\n");
  print(STDOUT "# build_check_script is the check script to run to check the\n");
  print(STDOUT "# success/failure of the build of the libs and apps. Leave\n");
  print(STDOUT "# blank to not run a check. This must be a full path specification\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "build_check_script = /rap/bin/check_build_status.pl\n\n");

  print(STDOUT "#------------ Build check ndays ----------------\n");
  print(STDOUT "# build_check_ndays is the number of days old an app or lib has to be\n");
  print(STDOUT "# before it is logged as old.\n");
  print(STDOUT "# Type: int\n");
  print(STDOUT "#\n");
  print(STDOUT "build_check_ndays = 1\n\n");

  print(STDOUT "#------------ Build check always send email ----------------\n");
  print(STDOUT "# build_check_always_send_email is a flag to always send email from the\n");
  print(STDOUT "# build check script even if no libs or apps failed to build.\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "build_check_always_send_email = FALSE\n\n");

  print(STDOUT "#------------ Build check dirs ----------------\n");
  print(STDOUT "# build_check_lib_dir is the directory to check for libs build_check_ndays old\n");
  print(STDOUT "# Be aware that if do_reset_env_to_checkout is TRUE, all the RAP_*\n");
  print(STDOUT "# will be reset and passed along to the build_check_script.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "build_check_lib_dir = \n\n");

  print(STDOUT "# build_check_bin_dir is the directory to check for apps build_check_ndays old\n");

  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "build_check_bin_dir = \n\n");

  print(STDOUT "#------------ Build check params ----------------\n");
  print(STDOUT "# build_check_param_email is the full path to the email list param\n");
  print(STDOUT "# file to use as input to the build_check_script. Leave blank to not\n");
  print(STDOUT "# send email\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "build_check_param_email = \n\n");
  
  print(STDOUT "# build_check_param_apps is the full path to the apps list param\n");
  print(STDOUT "# file to use as input to the build_check_script. Leave blank to not\n");
  print(STDOUT "# check apps\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "build_check_param_apps = \n\n");

  print(STDOUT "# build_check_param_libs is the full path to the libs list param\n");
  print(STDOUT "# file to use as input to the build_check_script. Leave blank to not\n");
  print(STDOUT "# check libs\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "build_check_param_libs = \n\n");
  
  print(STDOUT "#------------ Build check output file ----------------\n");
  print(STDOUT "# build_check_out_file is the filename to output the results of the build check\n");
  print(STDOUT "# into. This is the same file that is emailed\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "build_check_out_file = \n\n");

  print(STDOUT "#------------ Build check weblog file ----------------\n");
  print(STDOUT "# build_check_weblog_file is the URL on a web server of the nightly checkout\n");
  print(STDOUT "# and build log so can put this link into the output email. Leave blank to not\n");
  print(STDOUT "# put a link into the email.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "build_check_weblog_file = \n\n");

  # ... OBSOLETE ...
  
  print(STDOUT "#============= OBSOLETE ITEMS =======================\n\n");

  # ... make sysview1 ...

  print(STDOUT "#------------ Make sysview1 jar file ----------------\n");
  print(STDOUT "# *** WARNING: sysview1 using make is now obsolete ****\n");
  print(STDOUT "# make_sysview1_jarfile is a flag to build the sysview1 jar\n");
  print(STDOUT "# file. You must have the correct files checked out and have\n");
  print(STDOUT "# included java/src and java_apps in the src_dirs list\n");
  print(STDOUT "# Type: enum, with the following options:\n");
  print(STDOUT "#         TRUE\n");
  print(STDOUT "#         FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "make_sysview1_jarfile = FALSE\n\n");

  # ... end of param file ...

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
# Output:   $ret_val             1 on success, 0 on error
#
# Overview:
#

sub setAllParams2Defaults
{
  # Local variables

  local($return_val, $subname);
  local($missing);

  # Set defaults

  $return_val=0;
  $subname="setAllParams2Defaults";
  $missing=0;

  $DoCheckout=$IntDefault;
  $CheckoutDir=$StringDefault;
  $DistribsFiles=$StringDefault;
  @DistribsFilesArr=("");
  $DoRtag=0;
  $TmpDir="/tmp";
  $Rtag=$StringDefault;
  $RtagEmptyDirsCo=$StringDefault;
  @RtagEmptyDirsCo=("");
  $RtagEmptyDirsDate=$StringDefault;
  $DoResetEnvToCheckout=1;
  $SrcDirs="incs,libs,apps";
  @SrcDirsArr=("incs", "libs", "apps");
  $AntSrcDirs="";
  @AntSrcDirsArr=();
  $DoPreBuild=0;
  $DoBuild=1;
  $DoPostBuild=0;
  $DoInstall=0;
  $InstallTopDir=$StringDefault;
  $BuildDirs="bin,include,lib";
  @BuildDirsArr=("bin", "include", "lib");
  $InstallDirs="bin,include,lib";
  @InstallDirsArr=("bin", "include", "lib");
  $InstallSrcDirs="";
  @InstallSrcDirsArr=("");
  $DoRemoveSrcDirs=1;
  $DoRemoveBuildDirs=1;
  $DoRemoveInstallDirs=0;
  $DoMoveInstallDirs=0;
  $DoMakeClean=1;
  $DoRemoveNonStatic=0;
  $MakeIncsTarget="install";
  $MakeLibsTarget="install";
  $MakeLibIncludesTarget="install_include";
  $MakeAppsTarget="install";
  $MakeMatlabTarget="install";
  $MakeGenericTarget="install";
  $MakeJavaTarget="install_java";
  $AntJavaTarget="dist";
  $DoTdrpGenFirst=1;
  $MakeSysView1JarFile=0;
  $DoInsureBuild=0;
  $DoProfileBuild=0;
  $DoPGIBuild=0;
  $MyPreBuildScript=$StringDefault;
  $MyBuildScript=$StringDefault;
  $MyPostBuildScript=$StringDefault;
  $MyInstallScript=$StringDefault;
  $DoMyPreBuildScript=0;
  $DoMyBuildScript=0;
  $DoMyPostBuildScript=0;
  $DoMyInstallScript=0;
  
  $DoBuildCheckScript=0;
  $BuildCheckScript=$StringDefault;
  $BuildCheckNdays=1;
  $BuildCheckAlwaysSendEmail=0;
  $BuildCheckLibDir=$StringDefault;
  $BuildCheckBinDir=$StringDefault;
  $BuildCheckParamEmail=$StringDefault;
  $BuildCheckParamApps=$StringDefault;
  $BuildCheckParamLibs=$StringDefault;
  $BuildCheckOutFile=$StringDefault;
  $BuildCheckWebLogFile=$StringDefault;

  # Done

  $return_val=1;
  return($return_val);
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

  local($return_val, $subname);
  local($missing, $num);

  # Set defaults

  $return_val=0;
  $subname="checkAllParams";
  $missing=0;

  # Do it

  if ($DoCheckout eq $IntDefault) {
      print(STDERR "WARNING: do_checkout not set\n");
      $missing++;
  }
  if ($CheckoutDir eq $StringDefault) {
      print(STDERR "WARNING: checkout_dir not set\n");
      $missing++;
  }
  if ($DistribsFiles eq $StringDefault) {
      print(STDERR "WARNING: distribs_file not set\n");
      $missing++;
  }
  if ($DoInstall && ($InstallTopDir eq $StringDefault)) {
      print(STDERR "WARNING: install_top_dir not set\n");
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

  local($subname, $dir);

  # Set defaults

  $subname="printAllParams";

  # Print all the params

  print(STDERR "Printing all global params...\n");
  print(STDERR "\tdo_checkout: $DoCheckout\n");
  print(STDERR "\tcheckout_dir: $CheckoutDir\n");
  print(STDERR "\tdistribs_files: $DistribsFiles\n");
  if ($Debug_level > 1) {
      foreach $dir (@DistribsFilesArr) {
	  print(STDERR "\t\tdistribs_file_arr: $dir\n");
      }
  }
  print(STDERR "\tdo_rtag: $DoRtag\n");
  print(STDERR "\trtag: $Rtag\n");
  print(STDERR "\trtag_empty_dirs_co: $RtagEmptyDirsCo\n");
  if ($Debug_level > 1) {
      foreach $dir (@RtagEmptyDirsCoArr) {
	  print(STDERR "\t\trtag_empty_dirs_co: $dir\n");
      }
  }
  print(STDERR "\trtag_empty_dirs_date: $RtagEmptyDirsDate\n");
  print(STDERR "\ttmp_dir: $TmpDir\n");
  print(STDERR "\tdo_reset_env_to_checkout: $DoResetEnvToCheckout\n");
  print(STDERR "\tsrc_dirs: $SrcDirs\n");
  if ($Debug_level > 1) {
      foreach $dir (@SrcDirsArr) {
	  print(STDERR "\t\tsrc_dir: $dir\n");
      }
  }
  print(STDERR "\tant_src_dirs: $AntSrcDirs\n");
  if ($Debug_level > 1) {
      foreach $dir (@AntSrcDirsArr) {
	  print(STDERR "\t\tant_src_dir: $dir\n");
      }
  }
  print(STDERR "\tdo_build: $DoBuild\n");
  print(STDERR "\tdo_install: $DoInstall\n");
  print(STDERR "\tinstall_top_dir: $InstallTopDir\n");
  print(STDERR "\tbuild_dirs: $BuildDirs\n");
  if ($Debug_level > 1) {
      foreach $dir (@BuildDirsArr) {
	  print(STDERR "\t\tbuild_dir: $dir\n");
      }
  }
  print(STDERR "\tinstall_dirs: $InstallDirs\n");
  if ($Debug_level > 1) {
      foreach $dir (@InstallDirsArr) {
	  print(STDERR "\t\tinstall_dir: $dir\n");
      }
  }
  print(STDERR "\tinstall_src_dirs: $InstallSrcDirs\n");
  if ($Debug_level > 1) {
      foreach $dir (@InstallSrcDirsArr) {
	  print(STDERR "\t\tinstall_src_dir: $dir\n");
      }
  }
  print(STDERR "\tdo_remove_src_dirs: $DoRemoveSrcDirs\n");
  print(STDERR "\tdo_remove_build_dirs: $DoRemoveBuildDirs\n");
  print(STDERR "\tdo_remove_install_dirs: $DoRemoveInstallDirs\n");
  print(STDERR "\tdo_move_install_dirs: $DoMoveInstallDirs\n");
  print(STDERR "\tdo_make_clean: $DoMakeClean\n");
  print(STDERR "\tdo_remove_nonstatic: $DoRemoveNonStatic\n");
  print(STDERR "\tmake_incs_target: $MakeIncsTarget\n");
  print(STDERR "\tmake_libs_target: $MakeLibsTarget\n");
  print(STDERR "\tmake_libincludes_target: $MakeLibIncludesTarget\n");
  print(STDERR "\tmake_apps_target: $MakeAppsTarget\n");
  print(STDERR "\tmake_matlab_target: $MakeMatlabTarget\n");
  print(STDERR "\tmake_generic_target: $MakeGenericTarget\n");
  print(STDERR "\tmake_java_target: $MakeJavaTarget\n");
  print(STDERR "\tant_java_target: $AntJavaTarget\n");
  print(STDERR "\tdo_tdrp_gen_first: $DoTdrpGenFirst\n");
  print(STDERR "\tmake_sysview1_jarfile: $MakeSysView1JarFile\n");
  print(STDERR "\tdo_insure_build: $DoInsureBuild\n");
  print(STDERR "\tdo_profile_build: $DoProfileBuild\n");
  print(STDERR "\tdo_pgi_build: $DoPGIBuild\n");
  print(STDERR "\tmy_prebuild_script: $MyPreBuildScript\n");
  print(STDERR "\tmy_build_script: $MyBuildScript\n");
  print(STDERR "\tmy_postbuild_script: $MyPostBuildScript\n");
  print(STDERR "\tmy_install_script: $MyInstallScript\n");

  print(STDERR "\tbuild_check_script: $BuildCheckScript\n");
  print(STDERR "\tbuild_check_ndays: $BuildCheckNdays\n");
  print(STDERR "\tbuild_check_always_send_email: $BuildCheckAlwaysSendEmail\n");
  print(STDERR "\tbuild_check_lib_dir: $BuildCheckLibDir\n");
  print(STDERR "\tbuild_check_bin_dir: $BuildCheckBinDir\n");
  print(STDERR "\tbuild_check_param_email: $BuildCheckParamEmail\n");
  print(STDERR "\tbuild_check_param_apps: $BuildCheckParamApps\n");
  print(STDERR "\tbuild_check_param_libs: $BuildCheckParamLibs\n");
  print(STDERR "\tbuild_check_out_file: $BuildCheckOutFile\n");
  print(STDERR "\tbuild_check_weblog_file: $BuildCheckWebLogFile\n");
  
  # Done

  $return_val=1;
  return($return_val);
}


#-------------------------------------------------------------------------
#
# Subroutine parseMakefileSubdirs
#
# Usage: ($return_val, $nsdirs) =parseMakefileSubdirs($infile, *sdirs_arr, $debug)
#
# Function: Reset the env vars based on the checkout dir
#
# Input:    $infile       file to parse
#           $debug
#
# Output:   $return_val   1 on success, 0 on error
#           $nsdirs       size of *sdirs_arr
#           *sdirs_arr    array of subdirs
#
# Overview:
#

sub parseMakefileSubdirs
{
  local ($infile, *sdirs_arr, $debug) = @_;

  # Local variables

  local($subname, $return_val);
  local($dbg2, $dbg3);
  local($nsdirs, $found, $found_start, $found_end, $counter, $output_list);
  local($line, @pieces, $npieces);

  # Set defaults

  $return_val=0;
  $subname="parseMakefileSubdirs";
  $nsdirs=0;

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

  # Error checking

  if (!-e $infile) {
      print(STDERR "ERROR: $subname: The input file $infile does not exist\n");
      return($return_val, $nsdirs);
  }

  # Open the input file and search for SUB_DIRS

  if (!open(INFILE, $infile)) {
      print(STDERR "$subname: Cannot open input file $infile\n");
      return($return_val, $nsdirs);
  }

  $found=0;
  $found_start=0;
  $found_end=0;
  $counter=0;
  $output_list="";
  while ($line = <INFILE>) {

      chop($line);

      # Skip the rest of the file

      if ($found_end) {
	  next;
      }

      # Look for the SUB_DIRS

      if ($line =~ /^SUB_DIRS\s+\=/) {
	  $found=1;
	  $found_start=1;

	  if ($dbg2) {
	      print(STDERR "Found the start of SUB_DIRS, line: $line\n");
	  }

	  # Is there anything after the = on this line?

	  @pieces=split(/\=/, $line);
	  $npieces=@pieces;

	  if ($dbg3) {
	      for ($i=0; $i<$npieces; $i++) {
		  print(STDERR "i: $i, piece: $pieces[$i]\n");
	      }
	  }

	  if ($npieces > 1) {

	      if ($dbg2) {
		  print(STDERR "Found entries on the SUB_DIRS line, npieces: $npieces\n");
	      }

	      # Skip the case where the only entry is a line continuation char

	      if ($pieces[1] =~ /\s+\\/) {
		  if ($dbg2) {
		      print(STDERR "Skipping this line, only a line continuation\n");
		  }
	      } else {
		  # Add to the output list

		  if ($dbg2){
		      print(STDERR "\tadding to list: $pieces[1], counter: $counter\n");
		  }

		  $output_list="$output_list $pieces[1]";
		  $counter++;
	      }
	  }

	  next;
      }

      # We are inside the SUB_DIRS list. Add to the array
    
      if (($found_start) && (!$found_end)) {
	  if ($dbg2) {
	      print(STDERR "Found SUB_DIR entry, line: $line\n");
	  }

	  # Need to remove leading whitespace, trailing continuation lines
	  # and to split multiple entries if there are any

	  @pieces=split(/\b/, $line);
	  $npieces=@pieces;

	  if ($dbg3) {
	      for ($i=0; $i<$npieces; $i++) {
		  print(STDERR "i: $i, piece: $pieces[$i]\n");
	      }
	  }
	
	  for ($i=0; $i<$npieces; $i++) {

	      # Skip line continuation characters

	      if ($pieces[$i] =~ /\\/) {
		  next;
	      }

	      # Skip blanks

	      if ($pieces[$i] =~ /\s+/) {
		  next;
	      }

	      # Add to the output array

	      if ($dbg2){
		  print(STDERR "\tadding to array: $pieces[$i], counter: $counter\n");
	      }

	      $sdirs_arr[$counter]=$pieces[$i];

	      $counter++;
	  }
      }

      # Look for the end of the SUB_DIRS list. Search for a line without
      # a line continuation character or a blank line

      if (($found_start) && (($line !~ /\\/) || ($line !~ /\w/))) {
	  $found_end=1;
	  if ($dbg2) {
	      print(STDERR "Found the end of the SUB_DIR entries, line: $line\n");
	  }
      }
  } #endwhile

  # Close the input file

  close(INFILE);

  # Did we find any SUB_DIRS entries?

  if (!$found) {
      print(STDERR "ERROR: $subname: Did not find any SUB_DIR entries in the input file, $infile\n");
      return($return_val, $nsdirs);
  }

  # Debug

  if ($dbg2) {
      print(STDERR "$subname: counter: $counter...\n");
      for ($i=0; $i<$counter; $i++) {
	  print(STDERR "i: $i, sdirs arr at i: $sdirs_arr[$i]\n");
      }
  }

  # Return

  $return_val=1;
  $nsdirs=$counter;
  return($return_val, $nsdirs);
}

#-------------------------------------------------------------------------
#
# Subroutine removeNonStaticBinaries
#
# Usage: removeNonStaticBinaries()
#
# Function: deletes static binaries
#
# Input:    globals
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub removeNonStaticBinaries
{

    # Local variables
    
    local($subname, $return_val);
    local($dir, $bindir, $cmd, $tmpfile, $file, $junk, $is_ok);

    # Set defaults

    $return_val=0;
    $subname="removeNonStaticBinaries";

    # Create the tmp dir if it does not exist

    if (!-d $TmpDir) {
	$cmd="mkdir -p $TmpDir";
	system($cmd);
	$is_ok=WEXITSTATUS($?);
	if ($is_ok != 0) {
	    print(STDERR "ERROR: Cannot create $TmpDir and it does not exist\n");
	    return($return_val);
	}
    }

    #  Remove any non-static binaries. Static build failures can be
    #  inadvertantly covered by by success with install_shared.

    foreach $dir (@BuildDirsArr) {
	if ($dir ne "bin") {
	    next;
	}
	
	$bindir="${CheckoutDir}/bin";

	print(STDERR "$subname: Removing any non-static binaries from ${bindir}\n");
	$tmpfile="${TmpDir}/dynamic_files";
	$cmd="file ${bindir}/* | grep dynamic > $tmpfile";
	system($cmd);
	$is_ok=WEXITSTATUS($?);
	print(STDERR "$subname: Return $is_ok, from cmd: $cmd\n");
	    
        # Error checking

	if (!-e $tmpfile) {
	    print(STDERR "ERROR: $subname: The file $tmpfile does not exist\n");
	    return($return_val);
	}

	# Open the input file and read the list of files

	if (!open(TMPFILE, $tmpfile)) {
	    print(STDERR "$subname: Cannot open file $tmpfile\n");
	    return($return_val);
	}

	while ($line = <TMPFILE>) {

	    # Will get lines like the following:
	    # /rap/bin/CalcCapeCin3D:  ELF 32-bit LSB executable, Intel 80386, version 1, dynamically linked (uses shared libs), stripped
	    # Only want the first part - the file name

	    ($file, $junk)=split('\:', $line);
	    if (-e $file) {
		print(STDERR "$subname: Removing $file, failed to build statically\n");
		$cmd="/bin/rm -f $file";
		system($cmd);
	    }
	} #endwhile

	close(TMPFILE);
    } #endforeach

    # Done
    
    $return_val=1;
    return($return_val);
}


#-------------------------------------------------------------------------
#
# Subroutine makeClean
#
# Usage: makeClean
#
# Function: runs make clean
#
# Input:    globals
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub makeClean
{

    # Local variables
    
    local($subname, $return_val);
    local($dir, $cmd);

    # Set defaults

    $return_val=0;
    $subname="makeClean";

    foreach $dir (@SrcDirsArr) {
	chdir("${CheckoutCvsDir}/${dir}");
	$cmd="make clean_all";
	system($cmd);
    }

    # Done
    
    $return_val=1;
    return($return_val);
}

#-------------------------------------------------------------------------
#
# Subroutine printCurrentTime
#
# Usage: printCurrentTime
#
# Function: Gets the current time and prints it to STDERR
#
# Input:    none
#
# Output:   none
#
# Overview:
#

sub printCurrentTime
{

    # Local variables
    
    local($subname);
    local($current_time);

    # Set defaults

    $subname="printCurrentTime";

    # Get and print the current time

    $current_time=`date`;
    print(STDERR "------ Current time: $current_time");

    # Done
    
    return;
}


#---------------------------------------------------------------------
# Subroutine: expandEnvVar()
#
# Usage:      ($return_val, $expanded_string) = 
#                   expandEnvVar($string, $debug)
#
# Function:   Expand the environment variable on the input string
#
# Input:      $string        environment variable string to expand.
#             $debug         debug flag
#
# Output:     $return_val    1 on success or 0 on error (e.g., env var
#                            syntax used in string but env var not defined)
#             $expanded_string  the expanded environment variable, 
#                               or if not an enviroment variable, just returns 
#                               the input string.
# 

sub expandEnvVar {
    local ($string, $debug) = @_;

    # Local variables

    local($return_val, $sub_name);
    local($dbg2, $dbg3);
    local($prior_to_dollar, $after_dollar, $dollar_sign, $env_var, $remainder);

    # set defaults

    $subname="expandEnvVar";
    $return_val=0;
    $expanded_string=$string;

    # Debugging

    $dbg2=0;
    $dbg3=0;
    if ($debug == 2) {
	$dbg2=1;
    }
    if ($debug == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    # Return if not an env var

    if ($string !~ /\$\(/) {
	$return_val=1;
	return($return_val, $string);
    }

    # Okay, we have a $() structure

    # First split on the $

    ($prior_to_dollar, $after_dollar) = split('\$', $string);

    # Add the dollar back on. This is needed for the next split to work correctly

    $after_dollar="\$" . $after_dollar;
    
    # Okay now split on the left paren

    ($dollar_sign, $env_var, $remainder) = split(/\(|\)/, $after_dollar);

    if ($dbg3) {
	print(STDERR "$subname: env_var: $env_var, remainder: $remainder\n");
    }

    if (defined $ENV{$env_var}) { 
	$expanded_env_var=$ENV{$env_var};
	$expanded_string=$prior_to_dollar . $expanded_env_var . $remainder;
	$return_val=1;
    }
    else {
	print(STDERR "WARNING: $sub_name: the environment variable $string is NOT defined\n");
	$return_val=0;
    }

    # Debug

    if ($dbg2) {
	print(STDERR "$subname: string: $string, expanded_string: $expanded_string\n");
    }

    # Return

    return($return_val, $expanded_string);
}


#========================================= EOF =====================================
