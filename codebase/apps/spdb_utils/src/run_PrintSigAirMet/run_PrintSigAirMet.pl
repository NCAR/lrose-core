#!/usr/bin/perl
#
# Name: run_PrintSigAirMet.pl
#
# Function:
#	Perl script to run PrintSigAirMet on specific domains
#
# Overview:
#       Reads a param file and generates ascii output files
#
# Usage:
#       run_PrintSigAirMet.pl
#
# Input:
#       1) An ASCII param file for this script
#       2) A SIGMET SPDB url
#
# Output:
#
# Dependencies:
#	Perl must be available in the location at the top of this file.
#       Need PrintSigAirMet binary in the path.
#       Need LdataWriter binary in the path.
# 	
# Author: Deirdre Garvey - NCAR/RAP/SDG
#
# 15-JUL-2003
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
# RAP Perl modules
#

use Env qw(RAP_SHARED_LIB_DIR);
use Env qw(RAP_LIB_DIR);
use Env qw(RAP_INST_LIB_DIR);

use lib "$RAP_SHARED_LIB_DIR/perl5/";
use lib "$RAP_LIB_DIR/perl5/";
use lib "$RAP_INST_LIB_DIR/perl5/";

use Toolsa;
use Aoaws_webcontent;

#
# --------------------------- Defaults and globals --------------
#
# Get the program basename.

($prog = $0) =~ s|.*/||;

# ... Like pound-defines ...
$ExitSuccess=0;
$ExitFailure=1;

$StringDefault="UNKNOWN";
$IntDefault=-1;
$FloatDefault=-1.0;

$BE_DescIdx=0;
$BE_DirIdx=1;
$BE_MinLatIdx=2;
$BE_MinLonIdx=3;
$BE_MaxLatIdx=4;
$BE_MaxLonIdx=5;

$DoUTC=2;
$DoLocaltime=1;

#
# Set command line defaults
#

$Debug=0;                           # Flag to print debug messages to STDERR
$Debug_level=0;                     # Level of debugging
$DoPrintParams=0;                   # Flag for print_params
$DoUseLdata=0;                      # Flag to use --ldata input for time

#
# Save the usage to print to the user if there is a problem
#
$usage =                                                 
    "\n" .
    "Usage: $prog -p|-c <file> [-dh] <-v level>\n" .
    "Purpose: To run PrintSigAirMet for specific locations and generate\n" .
    "         output files\n" .
    "   -c --config <file>    : (Required.) The name of the config/param\n" .
    "                           file to read. NOT required if running\n" .
    "                           --print_params\n" .
    "   -d --debug            : Print debugging messages\n" .
    "   -h --help             : Print this usage message\n" .
    "   -l --ldata <info>     : Pass the info from the _latest_data_info file in\n" .
    "                           on the command line. This matches the order of items passed\n" .
    "                           from LdataWatcher. Must be specified inside a quoted string.\n" .
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
		      'ldata=s',
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
  print(STDERR "\tldata: $opt_ldata\n");
  print(STDERR "\tverbosedebuglevel: $opt_verbose\n");
}

if ($opt_config) {
    $ConfigFile= $opt_config;
}

if ($opt_ldata) {
    $Ldatainfo=$opt_ldata;
    $DoUseLdata=1;
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

if ($FoundErrors) {
    exit $ExitFailure;
}

# --------------------------- Initialization -----------------------
#
# Get today

$Today=`date`;
chop($Today);

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

# Read the config/param file and set globals

($is_ok, $NBoxes) = &readParamFile($ConfigFile, *BoxesArr, $Debug_level);
if (!$is_ok) {
    print(STDERR "Exiting...\n");
    exit $ExitFailure;
}

# Create directories

$is_ok=&createDirs($NBoxes, *BoxesArr, $Debug_level);
if (!$is_ok) {
    exit $ExitFailure;
}

# Generate output files

$is_ok=&createOutput($NBoxes, *BoxesArr, $Debug_level);
if (!$is_ok) {
    exit $ExitFailure;
}

# Done

exit $ExitSuccess;


# =============================== SUBROUTINES ===========================

#-------------------------------------------------------------------------
#
# Subroutine createOutput
#
# Usage: $return_val = createOutput($nboxes, *boxes_arr, $debug)
#
# Function: 
#
# Input:    globals
#           $nboxes         size of *boxes_arr
#           *boxes_arr      array
#           $debug          debug flag
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub createOutput
{
  local ($nboxes, *boxes_arr, $debug) = @_;

  # Local variables

  local($subname, $return_val);
  local($dbg2, $dbg3);
  local($is_ok, $run, $cmd, $file, $dir, $ddir, $adir);
  local($be_desc, $be_dir, $be_min_lat, $be_min_lon, $be_max_lat, $be_max_lon);
  local($now, $date, $hms, $dir, $do_relative_dir, $src_name, $link_name);
  local($user_info1, $user_info2, $errors, $valid_time);
  local($lutime, $lyear, $lmonth, $lday, $lhour, $lmin, $lsec, $lfname_ext);
  local($luser_info1, $luser_info2, $ln_fcasts);

  # Set defaults

  $return_val=0;
  $subname="createOutput";

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

  # Get the current time OR use the input ldata info time from the command line

  if ($DoUseLdata) {
      ($is_ok, $lutime, $lyear, $lmonth, $lday, $lhour, $lmin, $lsec, $lfname_ext, $luser_info1, $luser_info2, $ln_fcasts)=&AW_parseLdataInfoString($Ldatainfo, *lfcast_lead_secs, $debug);
      $date=sprintf("%4d%02d%02d", $lyear, $lmonth, $lday);
      $hms=sprintf("%02d%02d%02d", $lhour, $lmin, $lsec);
      $valid_time="${lyear} ${lmonth} ${lday} ${lhour} ${lmin} ${lsec}";
  }
  else {
      $now=time;
      ($is_ok, $year, $mon, $mday, $hour, $min, $sec)=AW_utime2date($now, $DoUTC, $debug);
      $date="${year}${mon}${mday}";
      $hms="${hour}${min}${sec}";
      $valid_time="$year $mon $mday $hour $min $sec";
  }

  # Generate the output file(s)

  $errors=0;

  for ($i=0; $i<$nboxes; $i++) {
      $be_desc=$boxes_arr[$BE_DescIdx][$i];
      $be_dir=$boxes_arr[$BE_DirIdx][$i];
      $be_min_lat=$boxes_arr[$BE_MinLatIdx][$i];
      $be_min_lon=$boxes_arr[$BE_MinLonIdx][$i];
      $be_max_lat=$boxes_arr[$BE_MaxLatIdx][$i];
      $be_max_lon=$boxes_arr[$BE_MaxLonIdx][$i];

      if ($dbg2) {
	  print(STDERR "$subname: be_desc: $be_desc, be_dir: $be_dir\n");
	  print(STDERR "\tbe_min_lon: $be_min_lon, be_max_lon: $be_max_lon, be_min_lat: $be_min_lat, be_max_lat: $be_max_lat\n");
      }
      
      # Create the dated output directory, if specified

      $dir="${OutputDir}/${be_dir}";

      if ($DoOverwrite) {
	  $ddir=$dir;
      } else {
	  $ddir="${dir}/${date}";
      }
      $is_ok=makeDir($ddir, $debug);
      if (!$is_ok) {
	  $errors=1;
	  next;
      }

      # Define the output filename

      if ($DoOverwrite) {
	  $file="sigairmet.txt";
      } else {
	  $file="${hms}.txt";
      }

      # Setup the command

      $cmd="PrintSigAirMet -params $ParamFile -bound \"$be_min_lon $be_max_lon $be_min_lat $be_max_lat\" -valid \"$valid_time\"";

      if ($be_desc =~ /\S/) {
	  $cmd="$cmd -header_text \"$be_desc\"";
      }
      
      if ($debug) {
	  $cmd="$cmd -debug";
      }

      $cmd="$cmd > $ddir/$file";

      if ($dbg2) {
	  print(STDERR "$subname: Running: $cmd\n");
      }

      # Run the command

      $is_ok=system("$cmd");
      $run=WEXITSTATUS($?);
      if ($run != 0) {
	  print(STDERR "ERROR: $subname: Cannot run PrintSigAirMet for $be_dir, return: $run\n");
	  $errors=1;
	  next;
      }

      # Do ldata (if set)

      if (DoLdata) {
	  $user_info2="${date}/${file}";
	  $user_info1="${date}/${hms}";

	  $cmd="LdataWriter -dtype txt -ext txt -ltime ${date}${hms} -info1 $user_info1 -info2 $user_info2 -dir $dir";

	  if ($dbg2) {
	      print(STDERR "$subname: Running: $cmd\n");
	  }

	  $is_ok=system("$cmd");
	  $run=WEXITSTATUS($?);
	  if ($run != 0) {
	      print(STDERR "ERROR: $subname: Cannot run LdataWriter for $be_dir, return: $run\n");
	      $errors=1;
	  }
      }

      # Copy to the archive directory (if set)
      
      if ($DoArchive) {

	  # Create the dated directory

	  $adir="${ArchiveDir}/${be_dir}/${date}";
	  $is_ok=makeDir($adir, $debug);
	  if (!$is_ok) {
	      $errors=1;
	      next;
	  }
	  
	  # Copy the file

	  $cmd="cp $ddir/$file $adir/$file";

	  if ($dbg2) {
	      print(STDERR "$subname: Running: $cmd\n");
	  }

	  $is_ok=system("$cmd");
	  $run=WEXITSTATUS($?);
	  if ($run != 0) {
	      $errors=1;
	      print(STDERR "ERROR: $subname: Cannot copy $dir/file to $adir, return: $run\n");
	  }
      }

      # Create the link (if set)

      if ($DoLink) {
	  if ($DoRelativeDir) {
	      $src_name="$RelativeLinkDir/$be_dir/$date/$file";
	  } else {
	      $src_name="$ddir/$file";
	  }
	  $link_name="$be_dir.txt";
	  $is_ok=AW_makeLink($DoRelativeDir, $LinkDir, $src_name, $link_name, $debug);
      }

  }

  # Done

  if (!$errors) {
      $return_val=1;
  }
  return($return_val);

}



#-------------------------------------------------------------------------
#
# Subroutine createDirs
#
# Usage: $return_val=createDirs($nboxes, *boxes_arr, $debug)
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
  local ($nboxes, $debug) = @_;

  # Local variables

  local($subname, $return_val);
  local($dbg2, $dbg3);
  local($dir, $is_ok, $make_dir, $i);

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

  if (!-d $OutputDir) {
      $is_ok=&makeDir($OutputDir, $debug);
      if (!$is_ok) {
	  return($return_val);
      }
      for ($i=0; $i<$nboxes; $i++) {
	  $dir="${OutputDir}/$boxes_arr[$BE_DirIdx][$i]";
	  $is_ok=&makeDir($OutputDir, $debug);
	  if (!$is_ok) {
	      return($return_val);
	  }
      }
  }

  if ($DoAchive) {
      if (!-d $ArchiveDir) {
	  $is_ok=&makeDir($ArchiveDir, $debug);
	  if (!$is_ok) {
	      return($return_val);
	  }
      }
      for ($i=0; $i<$nboxes; $i++) {
	  $dir="${ArchiveDir}/$boxes_arr[$BE_DirIdx][$i]";
	  $is_ok=&makeDir($ArchiveDir, $debug);
	  if (!$is_ok) {
	      return($return_val);
	  }
      }
  }

  if ($DoLink) {
      if (!-d $LinkDir) {
	  $is_ok=&makeDir($LinkDir, $debug);
	  if (!$is_ok) {
	      return($return_val);
	  }
      }
  }

  $return_val=1;
  return($return_val);

}

#-------------------------------------------------------------------------
#
# Subroutine readParamFile
#
# Usage: ($return_val, $nboxes) = readParamFile($param_file, *boxes_arr, $debug)
#
# Function: Read the $param_file.
#
# Input:    $param_file          file to read
#           $debug               flag for debugging (1=on, 0=off)
#
# Output:   $ret_val             1 on success, 0 on error
#           $nboxes              number of boxes
#           *boxes_arr           array of boxes
#              [$BE_DirIdx][n]         = output subdir
#              [$BE_MinLatIdx][n]      = min lat
#              [$BE_MinLonIdx][n]      = min lon
#              [$BE_MaxLatIdx][n]      = max lat
#              [$BE_MaxLonIdx][n]      = max lon
#
# Overview:
#

sub readParamFile
{
  local ($param_file, *boxes_arr, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($dbg2, $dbg3);
  local($is_ok, $keyword, $keyvalue, *out_arr, $type, $unstripped_keyvalue);
  local($found_box_list, $found_be_list, $nboxes, $box_counter);
  local($be_desc, $be_dir, $be_min_lat, $be_min_lon, $be_max_lat, $be_max_lon);

  # Set defaults

  $return_val=0;
  $subname="readParamFile";
  $nboxes=0;

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
      return($return_val, $nboxes);
  }

  # Set defaults before loop

  $found_box_list=0;
  $found_be_list=0;
  $box_counter=0;

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

	  if ($keyword eq "param_file") {
	      $type="string";
	      $ParamFile= $keyvalue;
	      ($is_ok, $ParamFile) = &AW_expandEnvVar($ParamFile, $debug);
	  }

	  elsif ($keyword eq "out_dir") {
	      $type="string";
	      $OutputDir = $keyvalue;
	      ($is_ok, $OutputDir) = &AW_expandEnvVar($OutputDir, $debug);
	  }

	  elsif ($keyword eq "do_overwrite") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoOverwrite=1;
	      } else {
		  $DoOverwrite=0;
	      }
	  }

	  elsif ($keyword eq "archive_dir") {
	      $type="string";
	      $ArchiveDir = $keyvalue;
	      ($is_ok, $ArchiveDir) = &AW_expandEnvVar($ArchiveDir, $debug);
	      if (($ArchiveDir =~ /\w/) && ($ArchiveDir ne $StringDefault)){
		  $DoArchive=1;
	      } else {
		  $DoArchive=0;
	      }
	  }

	  elsif ($keyword eq "link_dir") {
	      $type="string";
	      $LinkDir = $keyvalue;
	      ($is_ok, $LinkDir) = &AW_expandEnvVar($LinkDir, $debug);
	      if (($LinkDir =~ /\w/) && ($LinkDir ne $StringDefault)){
		  $DoLink=1;
	      } else {
		  $DoLink=0;
	      }
	  }

	  elsif ($keyword eq "relative_link_dir") {
	      $type="string";
	      $RelativeLinkDir = $keyvalue;
	      ($is_ok, $RelativeLinkDir) = &AW_expandEnvVar($RelativeLinkDir, $debug);
	      if (($RelativeLinkDir =~ /\w/) && ($RelativeLinkDir ne $StringDefault)){
		  $DoRelativeDir=1;
	      } else {
		  $DoRelativeDir=0;
	      }
	  }

	  elsif ($keyword eq "do_ldata") {
	      $type="bool";
	      if ($keyvalue eq "TRUE") {
		  $DoLdata=1;
	      } else {
		  $DoLdata=0;
	      }
	  }

	  elsif (($found_box_list) && ($keyword eq "be_outdir")) {
	      $type="string";
	      $be_dir = $keyvalue;
	  }

	  elsif (($found_box_list) && ($keyword eq "be_desc")) {
	      $type="string";
	      $be_desc = $unstripped_keyvalue;
	  }

	  elsif (($found_box_list) && ($keyword eq "be_minlat")) {
	      $type="string";
	      $be_min_lat = $keyvalue;
	  }

	  elsif (($found_box_list) && ($keyword eq "be_minlon")) {
	      $type="string";
	      $be_min_lon = $keyvalue;
	  }

	  elsif (($found_box_list) && ($keyword eq "be_maxlat")) {
	      $type="string";
	      $be_max_lat = $keyvalue;
	  }

	  elsif (($found_box_list) && ($keyword eq "be_maxlon")) {
	      $type="string";
	      $be_max_lon = $keyvalue;
	  }

	  else {
	      print(STDERR "ERROR: Cannot parse keyword line: $line");
	  }
	  
	  # End of keyword processing

	  next;
      }

      # Are we inside a table directive?

      if ($line =~ /\<BOX_LIST\>/) {
	  $found_box_list=1;
	  next;
      }

      if ($line =~ /\<\\BOX_LIST\>/) {
	  $found_box_list=0;
	  next;
      }

      # Are we inside a table entry list?

      if ($line =~ /\<BOX_ENTRY\>/) {
	  $found_be_list=1;
	  $be_dir=$StringDefault;
	  $be_desc="";
	  $be_min_lat=$IntDefault;
	  $be_min_lon=$IntDefault;
	  $be_max_lat=$IntDefault;
	  $be_max_lon=$IntDefault;
	  next;
      }

      # Update the tables array if at the end of a table entry list

      if (($found_be_list) && ($line =~ /\<\\BOX_ENTRY\>/)) {

	  # Fill the array
	  $boxes_arr[$BE_DirIdx][$box_counter]=$be_dir;
	  $boxes_arr[$BE_DescIdx][$box_counter]=$be_desc;
	  $boxes_arr[$BE_MinLatIdx][$box_counter]=$be_min_lat;
	  $boxes_arr[$BE_MinLonIdx][$box_counter]=$be_min_lon;
	  $boxes_arr[$BE_MaxLatIdx][$box_counter]=$be_max_lat;
	  $boxes_arr[$BE_MaxLonIdx][$box_counter]=$be_max_lon;
	  $found_be_list=0;
	  $box_counter++;

	  if ($Debug_level > 1) {
	      print(STDERR "$subname: box_counter: $box_counter, be_dir: $be_dir\n");
	      print(STDERR "\tbe_desc: $be_desc\n");
	      print(STDERR "\tbe_min_lon: $be_min_lon, be_max_lon: $be_max_lon, be_min_lat: $be_min_lat, be_max_lat: $be_max_lat\n");
	  }

	  next;
      }
	  
  } #endwhile

  close(PARAM_FILE);

  $nboxes=$box_counter;

  # ...Handle special cases...

  if ($DoOverwrite && ($DoArchive || $DoLink || $DoRelativeDir)) {
      print(STDERR "ERROR: Invalid combination of options! You cannot have\n");
      print(STDERR "\tdo_overwrite TRUE and do archive, link, or relative link\n");
  }

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

  return($return_val, $nboxes);
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

  # ... header ...

  print(STDOUT "# Parameters for $prog\n");
  print(STDOUT "#\n");
  print(STDOUT "# This param file was generated on: $Today\n");
  print(STDOUT "#=====================================================\n");

  # ... input param file ...

  print(STDOUT "#------------ input param file ----------------\n");
  print(STDOUT "# Specify the input param file for PrintSigAirMet.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "param_file = \n\n");

  # ... output dir ...

  print(STDOUT "#------------ output dir ----------------\n");
  print(STDOUT "# Specify the top-level output directory.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "out_dir = \$(RAP_DATA_DIR)/www_content/text/sigmet\n\n");

  # ... overwrite ...

  print(STDOUT "#------------ overwrite output files ----------------\n");
  print(STDOUT "# Specify whether to overwrite the output file(s) and not have\n");
  print(STDOUT "# dates in the filenames or dated subdirectories. Do not specify\n");
  print(STDOUT "# an archive dir, link_dir, or relative_link_dir if you set this\n");
  print(STDOUT "# option to TRUE. Default is FALSE: output files to dated\n");
  print(STDOUT "# subdirectories and include the time stamp in the output\n");
  print(STDOUT "# filenames.\n");
  print(STDOUT "# Type: bool, options: TRUE, FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_overwrite = FALSE\n\n");

  # ... archive dir ...

  print(STDOUT "#------------ archive dir ----------------\n");
  print(STDOUT "# Specify the top-level archive directory. Leave blank to not\n");
  print(STDOUT "# do archive.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "archive_dir = \$(RAP_DATA_DIR)/www_content/archive/text/sigmet\n\n");

  # ... link dir ...

  print(STDOUT "#------------ link dir ----------------\n");
  print(STDOUT "# Specify the top-level link directory. Leave blank to not\n");
  print(STDOUT "# create links.\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "link_dir = \$(RAP_DATA_DIR)/www_content/textWeb/sigmet\n\n");

  print(STDOUT "#------------ relative link dir ----------------\n");
  print(STDOUT "# Specify the top-level link directory relative to the source\n");
  print(STDOUT "# directory. Leave blank to not create relative links. You must\n");
  print(STDOUT "# also specify the link_dir above\n");
  print(STDOUT "# Type: string\n");
  print(STDOUT "#\n");
  print(STDOUT "relative_link_dir = ../../text/sigmet\n\n");

  # ... do_ldata ...

  print(STDOUT "#------------ ldata  ----------------\n");
  print(STDOUT "# Specify whether to write a _latest_data_info file in the\n");
  print(STDOUT "# out_dir.\n");
  print(STDOUT "# Type: bool, options: TRUE, FALSE\n");
  print(STDOUT "#\n");
  print(STDOUT "do_ldata = TRUE\n\n");

  # ... bounding boxes ...

  print(STDOUT "#---------- bounding boxes --------------\n");
  print(STDOUT "# Specify the bounding boxes to generate output files for.\n");
  print(STDOUT "# The list must begin with the <BOX_LIST> tag and end with the\n");
  print(STDOUT "# the <\\BOX_LIST> tag. Each box must begin with the <BOX_ENTRY>\n");
  print(STDOUT "# tag and end with the <//BOX_ENTRY> tag. Each tag and each item\n");
  print(STDOUT "# must be on a separate line. Do not use quotes. Use the following\n");
  print(STDOUT "# format for each box entry (inside the <BOX_ENTRY> tags.\n");
  print(STDOUT "# be_desc = the box description\n");
  print(STDOUT "# be_outdir = the subdir under the top-level out_dir for this box\n");
  print(STDOUT "# be_minlat = the minimum latitude for the box\n");
  print(STDOUT "# be_minlon = the minimum longitude for the box\n");
  print(STDOUT "# be_maxlat = the maximum latitude for the box\n");
  print(STDOUT "# be_maxlon = the maximum longitude for the box\n");
  print(STDOUT "#\n");
  print(STDOUT "<BOX_LIST>\n");
  print(STDOUT "<BOX_ENTRY>\n");
  print(STDOUT "<\\BOX_ENTRY>\n");
  print(STDOUT "<\\BOX_LIST>\n");

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

  $ParamFile=$StringDefault;
  $OutputDir=$StringDefault;
  $ArchiveDir=$StringDefault;
  $LinkDir=$StringDefault;
  $RelativeLinkDir=$StringDefault;
  $DoArchive=0;
  $DoLink=0;
  $DoLdata=0;
  $DoRelativeDir=0;
  $DoOverwrite=0;

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

  if ($ParamFile eq $StringDefault) {
      print(STDERR "WARNING: param_file not set\n");
      $missing++;
  }
  if ($OutputDir eq $StringDefault) {
      print(STDERR "WARNING: out_dir not set\n");
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
  print(STDERR "\tparam_file: $ParamFile\n");
  print(STDERR "\tout_dir: $OutputDir\n");
  print(STDERR "\tarchive_dir: $ArchiveDir\n");
  print(STDERR "\tlink_dir: $LinkDir\n");
  print(STDERR "\trelative_link_dir: $RelativeLinkDir\n");
  print(STDERR "\tdo_ldata: $DoLdata\n");
  
  # Done

  $return_val=1;
  return($return_val);
}


#---------------------------------------------------------------------
# Subroutine: makeDir
#
# Usage:      $return_val=makeDir($dir, $debug)
#
# Function:   Make the $dir
#
# Input:      $dir           directory to create
#             $debug         debug flag
#
# Output:     $return_val    1 on success or 0 on error
# 

sub makeDir {
    local ($dir, $debug) = @_;

    # Local variables

    local($return_val, $subname);
    local($dbg2, $dbg3);
    local($is_ok, $make_dir);

    # set defaults

    $subname="makeDir";
    $return_val=0;

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

    $is_ok=system("mkdir -p $dir");
    $make_dir=WEXITSTATUS($?);
    if ($make_dir != 0) {
	print(STDERR "ERROR: Cannot create $dir and it does not exist\n");
	return($return_val);
    }
    
    $return_val=1;
    return($return_val);

}

#========================================= EOF =====================================
