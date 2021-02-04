package    Monitor;
require    Exporter;
use Env qw(ADDSHOME);
@ISA = qw(Exporter);
@EXPORT = qw(printProfileStats getIntermedProfileTimes readLdataInfo registerDataMapper checkExeExist writeLdataInfo writeLdataInfoToolsa getDirAndFname);

#
# This module contains PERL routines for monitoring the ADDS system
#
# Deirdre Garvey NCAR/RAP     1-MAY-2001
#

# External libraries

use Time::Local;

#------------------------------------------------------------------------------------
# Subroutine: printProfileStats()
#
# Usage:   printProfileStats($logfile, $start_utime, *intermed_times, $nintermed_times)
#
# Purpose: Prints summary stats
#
# Input:   logfile            logfile to write to
#          start_utime        start time (UNIX time)
#          *intermed_times    array of intermediate time strings
#          nintermed_times    size of the *intermed_times array
#
# Output:  Writes the profile stats to $logfile;
#
#------------------------------------------------------------------------------------
#
sub printProfileStats {
    local ($logfile, $start_utime, *intermed_times, $nintermed_times) = @_;

    # Local variables

    local($user, $system, $cuser, $csystem, $end_time, $elapsed_clock_time, $end_date);
    local($is_ok, $i);

    # Get the timing stats

    ($user,$system,$cuser,$csystem) = times;
    $end_time = time;

    if ($start_utime > 0) {
	$elapsed_clock_time=$end_time - $start_utime;
    } else {
	$elapsed_clock_time="*************";
    }

    $end_date = `date`;
    
    # Open the output log file

    $is_ok=open (OUTLOG, ">> $logfile");
    if (!$is_ok) {
	print "Cannot open timing profile log: $logfile\n";
	return;
    }
    
    # Print

    print(OUTLOG "$prog: time in seconds...\n");
    print(OUTLOG "\tprocess - user time: $user\n");
    print(OUTLOG "\tprocess - system time: $system\n");
    print(OUTLOG "\tprocess children - user time: $cuser\n");
    print(OUTLOG "\tprocess children - system time: $csystem\n");
    print(OUTLOG "\telapsed clock time: $elapsed_clock_time secs\n");
    print(OUTLOG "\texit time: $end_date");

    # Print intermediate times

    if ($nintermed_times > 0) {
	print(OUTLOG "\t...Elapsed clock time for intermediate steps...\n");
	for ($i=0; $i<$nintermed_times; $i++) {
	    print(OUTLOG "\t$intermed_times[$i]");
	}
    }

    # Close

    close(OUTLOG);

    # Done

    return;
}

#
#------------------------------------------------------------------------------------
# Subroutine: getIntermedProfileTimes()
#
# Usage:   ($new_time, $nintermed_times) 
#               = getIntermedProfileTimes($usetime, $string, *intermed_times, 
#                                            $nintermed_times)
#
# Purpose: Get the time elapsed since the $usetime and now. Return $now
#          as $new_time. Build an array of intermediate profile times to print
#          out at the end of this script.
#
# Input:   usetime            time to use as start time for this intermediate time
#          string             identifying string for this intermediate time
#          *intermed_times    array of intermediate time strings
#          nintermed_times    size of the *intermed_times array
#
# Output:  new_time           time to use as start time for NEXT call to this function
#          *intermed_times    array of intermediate time strings
#          nintermed_times    size of the *intermed_times array
#          
#
#------------------------------------------------------------------------------------
#
sub getIntermedProfileTimes {
    local ($usetime, $string, *intermed_times, $nintermed_times) = @_;

    # Local variables

    local($now, $elapsed_secs);

    # Get the time

    $now=time;
    $elapsed_secs = $now - $usetime;
    
    # Add to array

    $intermed_times[$nintermed_times] = "$string: $elapsed_secs secs\n";
    $nintermed_times++;

    # Done

    return($now, $nintermed_times);
}

#
#-------------------------------------------------------------------------------------
# Subroutine: readLdataInfo
#
# Usage:      ($return_val, $fname) = readLdataInfo($ldata_handle, $ldata_dir, 
#                                    $max_valid_age, $debug)
#
# Function:   Read the latest_data_info file pointed to by $ldata_handle
#             and retrieve the data $fname.
#
# Input:      $ldata_handle     handle for LDATA file
#             $ldata_dir        directory where LDATA file lives
#             $max_valid_age    maximum number of seconds that data can be old
#             $debug            debug flag
#
# Output:     $return_val       1 on success, 0 on error
#             $fname            filename from latest_data_info file
#
# Overview:   
#

sub readLdataInfo {
  local ($ldata_handle, $ldata_dir, $max_valid_age, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($is_ok, $file);
  local($utime, $year, $month, $day, $hour, $min, $sec, $fname, $fname_ext);
  local($fcast_secs, $n_fcasts, $full_fname, $dbg2, $debug_ldata_info);

  # Set defaults

  $return_val=0;
  $subname="readLdataInfo";
  $file="**UNKNOWN**";

  $start_sub_time=time;                 # For profiling

  # Additional debug flags

  if ($debug == 2) {
      $dbg2=1;                          # Set to 1 to print input fields
      $debug_ldata_info=1;              # Set to 1 to get debug info from
                                        #    LDATA_read_info
  } else {
      $dbg2=0;
      $debug_ldata_info=0;
  }

  # Debug

  if ($dbg2) {
    print(STDERR "input to $subname...\n");
    print(STDERR "\tldata_handle: $ldata_handle, dir: $ldata_dir, max_age: $max_valid_age\n");
  }

  # Get the LDATA info file

  ($is_ok, $utime, $year, $month, $day, $hour, $min, $sec, $fname_ext, $fname, $full_fname, $n_fcasts, $fcast_secs)=Toolsa::LDATA_info_read($ldata_handle, $ldata_dir, $max_valid_age, $debug_ldata_info);
  if ($is_ok < 0) {
      return($return_val, $file);
  }

  if ($debug == 1) {
      print(STDERR "Returned from LDATA info file: $Today:...\n");
      print(STDERR "\tsrc dir: $ldata_dir\n");
      print(STDERR "\tdate: $year $month $day ${hour}:${min}:${sec} utime: $utime\n");
      print(STDERR "\tfname_ext: $fname_ext, fname: $fname, full_fname: $full_fname\n");
      print(STDERR "\tn_fcasts: $n_fcasts, fcast secs: $fcast_secs\n");
  }

  # Done

  $return_val = 1;
  $file=$full_fname;
  return($return_val, $file);
}


#---------------------------------------------------------------------------
# Subroutine: registerDataMapper
#
# Usage:      $return_val = registerDataMapper($data_time, $data_dir, $fcast_secs,
#                                              $dmap_type, $debug)
#
# Function:   Register with the DataMapper on the current host
#
# Input:      $data_time               data time as YYYYMMDDHH
#             $data_dir                data directory
#             $fcast_secs              forecast (lead) secs, if specified then
#                                         the $data_time should be the generation time
#             $dmap_type               data mapper data type
#             $debug                   debug flag
#
# Output:     $return_val       1 on success, 0 on error
#
# Overview:   
#

sub registerDataMapper {
  local ($data_time, $data_dir, $fcast_secs, $dmap_type, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($data_map_time, $cmd, $needed_exe, $is_ok, $force_path, $dmap_active);
  local($dbg2);

  # Set defaults

  $return_val=0;
  $subname="registerDataMapper";

  # Set hardwired defaults

  $needed_exe="LdataInformer";
  $force_path="$ADDSHOME/bin";

  # Additional debug flags

  if ($debug == 2) {
      $dbg2=1;
  } else {
      $dbg2=0;
  }

  # Debug

  if ($dbg2) {
      print(STDERR "$prog: $subname: Input...\n");
      print(STDERR "\tdate: $data_time, fcast_secs: $fcast_secs, data_dir: $data_dir, dmap_type: $dmap_type\n");
  }

  # ------------------- LDM HACKS --------------------
  # HACK to handle users which do not have LdataInformer in the path

  $is_ok=&checkExeExist($needed_exe, $debug);
  if (!$is_ok) {
      print(STDERR "WARNING: $needed_exe not found in the current user path. Will use $force_path\n");
      $needed_exe="$force_path/$needed_exe";
  }

  # HACK to force registration even if the user (e.g., ldm) does not have
  # the DATA_MAPPER_ACTIVE environment variable set to TRUE

  $dmap_active=$ENV{'DATA_MAPPER_ACTIVE'};
  if ($dmap_active ne "TRUE") {
      print(STDERR "WARNING: DATA_MAPPER_ACTIVE env var not set to TRUE by current use. Will force to TRUE\n");
      $ENV{'DATA_MAPPER_ACTIVE'}="TRUE";
  }
  # ----------------------------------------------------


  # Convert the valid date/hour into DataMapper style time
  #       date is YYYYMMDDHH, DataMapper needs YYYYMMDDHHMMSS

  $data_map_time="${data_time}0000";

  # Do the registration
  # If fcast (lead) secs were specified, pass them to the informer

  $cmd="$needed_exe -t $data_map_time -d $data_dir -D $dmap_type";

  if ($fcast_secs > 0) {
      $cmd="$cmd -l $fcast_secs";
  }

  if ($debug) {
      print(STDERR "$prog: $subname: Running $cmd\n");
  }
  system($cmd);

  # Done
  
  $return_val=1;
  return($return_val);
}


#---------------------------------------------------------------------------
# Subroutine: checkExeExist
#
# Usage:      $return_val= checkExeExist($exe, $debug)
#
# Function:   Check whether $exe exists in the current user's path
#
# Input:      $exe               name of exe to search for
#             $debug             debug flag
#
# Output:     $return_val        1 on success, 0 on error
#
# Overview:   
#

sub checkExeExist {
  local ($exe, $debug) = @_;

  # Local variables

  local($return_val, $subname);
  local($check);

  # Set defaults

  $return_val=0;
  $subname="checkExeExist";

  # Do the check

  $check=`which $exe`;

  if (!$check) {
    $return_val = 0;
  } else {
    $return_val = 1;
  }

  return($return_val);
}

#---------------------------------------------------------------------------
# Subroutine: writeLdataInfo
#
# Usage:      $return_val = writeLdataInfo($data_file, $nsubdirs, $data_time, 
#                                          $lead_secs, $debug)
#
# Function:   Write a latest data info file for the $data_file using 
#             LdataWriter.
#
# Input:      $data_file               full data file name
#             $nsubdirs                number of subdirs above the filename to back-up to
#                                         write the latest_data_info file. For example, if
#                                         the $data_file has a date subdirectory above the
#                                         filename, set $nsubdirs to 1 to write the _latest_data_info
#                                         file at the directory with the date subdirs in it
#             $data_time               data time as YYYYMMDDHHMMSS, or 0 to use current time
#             $fcast_secs              forecast (lead) secs, if specified then
#                                         the $data_time should be the generation time
#             $debug                   debug flag
#
# Output:     $return_val       1 on success, 0 on error
#
# Overview:   
#

sub writeLdataInfo {
    local ($data_file, $nsubdirs, $data_time, $fcast_secs, $debug) = @_;

    # Local variables

    local($return_val, $subname);
    local($cmd, $needed_exe, $is_ok, $force_path);
    local($tmpstring, $ldata_dir, $fname1, $fname2, $ext);
    local($dbg2);

    # Set defaults

    $return_val=0;
    $subname="writeLdataInfo";
    $debug=1;

    # Set hardwired defaults

    $needed_exe="LdataWriter";
    $force_path="$ADDSHOME/bin";

    # Additional debug flags

    if ($debug == 2) {
	$dbg2=1;
    } else {
	$dbg2=0;
    }

    # Debug
    
    if ($dbg2) {
	print(STDERR "$prog: $subname: Input...\n");
	print(STDERR "\tdata_file: $data_file, nsubdirs: $nsubdirs, data_time: $data_time, fcast_secs: $fcast_secs\n");
    }

    # ------------------- LDM HACKS --------------------
    # HACK to handle users which do not have LdataInformer in the path

    $is_ok=&checkExeExist($needed_exe, $debug);
    if (!$is_ok) {
	print(STDERR "WARNING: $needed_exe not found in the current user path. Will use $force_path\n");
	$needed_exe="$force_path/$needed_exe";
    }
    # ----------------------------------------------------


    # Get the filename and data directories from the input $data_file

    ($is_ok, $ldata_dir, $fname1, $fname2, $ext)=&getDirAndFname($data_file, $nsubdirs, $debug);

    # Build the command to run the LdataWriter

    $cmd="$needed_exe -dir $ldata_dir -ext $ext -info1 $fname1 -info2 $fname2";

    if ($data_time > 0) {
	$cmd="$cmd -ltime $data_time";
    }

    if ($fcast_secs > 0) {
	$cmd="$cmd -lead $fcast_secs";
    }

    if ($debug) {
	print(STDERR "$prog: $subname: Running $cmd\n");
    }
    system($cmd);
}


#---------------------------------------------------------------------------
# Subroutine: writeLdataInfoToolsa
#
# Usage:      $return_val = writeLdataInfoToolsa($handle, $data_file, $nsubdirs,
#                                                $data_dir, $data_time,
#                                                $ext, $fname, $ext,
#                                                $fcast_secs, $debug)
#
# Function:   Write a latest data info file for the $data_file using a 
#             Toolsa::LDATA_info_write call
#
# Input:      $handle                  handle from previous call to LDATA_init_handle()
#             $data_file               full data file name, or -1 if specifying the
#                                         $data_dir, $data_time, $ext, $fname1, $fname2,
#                                         $nfcasts
#             $nsubdirs                ignored if $data_file = -1.
#                                         number of subdirs above the filename to back-up to
#                                         write the latest_data_info file. For example, if
#                                         the $data_file has a date subdirectory above the
#                                         filename, set $nsubdirs to 1 to write the _latest_data_info
#                                         file at the directory with the date subdirs in it
#             $data_dir                ignored if $data_file != -1
#                                         data directory to write latest_data_info file to
#             $data_time               data time as YYYYMMDDHHMMSS, or 0 to use current time
#             $fname                   ignored if $data_file != 1
#                                         filename to write to latest_data_info file, includes $ext
#             $ext                     ignored if $data_file != -1
#                                         extension to write to latest_data_info file or set to -1
#                                         to extract it from $fname
#             $fcast_secs              forecast (lead) secs, if specified then
#                                         the $data_time should be the generation time
#             $debug                   debug flag
#
# Output:     $return_val       1 on success, 0 on error
#
# Overview:   
#

sub writeLdataInfoToolsa {
    local ($handle, $data_file, $nsubdirs, $data_dir, $data_time, $fname, $inext, $fcast_secs, $debug) = @_;

    # Local variables

    local($return_val, $subname);
    local($cmd, $needed_exe, $is_ok, $force_path);
    local($ldata_dir, $fname1, $fname2, $ext, $udate, $nfcasts);
    local($year, $mon, $day, $hour, $min, $sec);
    local($dbg2);

    # Set defaults

    $return_val=0;
    $subname="writeLdataInfoToolsa";

    # Additional debug flags

    if ($debug == 2) {
	$dbg2=1;
    } else {
	$dbg2=0;
    }

    $dbg2=1;

    # Debug
    
    if ($dbg2) {
	print(STDERR "$prog: $subname: Input...\n");
	print(STDERR "\tdata_file: $data_file, nsubdirs: $nsubdirs\n");
	print(STDERR "\tdata_dir: $data_dir, data_time: $data_time\n");
	print(STDERR "\tfname: $fname, ext: $inext, fcast_secs: $fcast_secs\n");
    }

    # Get the filename and data directories from the input $data_file
    
    if ($data_file ne -1) {
	($is_ok, $ldata_dir, $fname1, $fname2, $ext)=&getDirAndFname($data_file, $nsubdirs, $debug);
    } else {
	# Set variables for ldata
	#   fname1=fname (no extension)
	#   fname2=fname.ext

	$fname2=$fname;
	
	# Parse the file extension out

	if ($inext eq -1) {
	    $last_dot=rindex($fname, '.');
	    $ext=substr($fname, $last_dot+1);
	} else {
	    $ext=$inext;
	    $last_dot=rindex($fname, $ext);
	}

	if ($inext eq "") {
	    $fname1=$fname2;
	} else {
	    $fname1=substr($fname2, 0, $last_dot);
	}

	$ldata_dir=$data_dir;
    }

    # Convert the input time to UNIX time
       
    if ($data_time <= 0) {
	$udate=time;
    } else {
	$year=substr($data_time, 0, 4);
	$mon=substr($data_time, 4, 2);
	$day=substr($data_time, 6, 2);
	$mon=$mon-1;
	$hour=substr($data_time, 8, 2);
	$min=substr($data_time, 10, 2);
	$sec=substr($data_time, 12, 2);
	$udate=timegm($sec, $min, $hour, $day, $mon, $year);
    }
	
    # Set forecast variables for LDATA  -- not yet implemented

    $nfcasts=0;

    # Write the LDATA file

    if ($dbg2) {
	print(STDERR "Calling LDATA_info_write with: handle: $handle, ldata_dir: $ldata_dir\n");
	print(STDERR "\tudate: $udate, ext: $ext, fname1: $fname1, fname2: $fname2, nfcasts: $nfcasts, fcast_secs: $fcast_secs\n");
    }

    $is_ok=Toolsa::LDATA_info_write($handle, $ldata_dir, $udate, $ext, $fname1, $fname2, $nfcasts, $fcast_secs);

    # Done

    $return_val=$is_ok;
    return($return_val);
}


#---------------------------------------------------------------------------
# Subroutine: getDirAndFname
#
# Usage:      ($return_val, $ldata_dir, $fname1, $fname2, $ext) = 
#                     getDirAndFname($data_file, $nsubdirs, $debug)
#
# Function:   Parse the directory and filename from the input $data_file so it
#             can be used for latest_data_info
#
# Input:      $data_file               full data file name
#             $nsubdirs                number of subdirs above the filename to back-up to
#                                         write the latest_data_info file. For example, if
#                                         the $data_file has a date subdirectory above the
#                                         filename, set $nsubdirs to 1 to write the _latest_data_info
#                                         file at the directory with the date subdirs in it
#             $debug                   debug flag
#
# Output:     $return_val       1 on success, 0 on error
#
# Overview:   
#

sub getDirAndFname {
    local ($data_file, $nsubdirs, $debug) = @_;

    # Local variables

    local($return_val, $subname);
    local($last_slash, $tmpstring, $next2last_slash, $fname1, $fname2, $ext);
    local($last_dot, *dirs, $ndirs, $i, $first_time, $ldata_dir);
    local($dbg2);

    # Set defaults

    $return_val=0;
    $subname="getDirAndFname";

    # Additional debug flags

    if ($debug == 2) {
	$dbg2=1;
    } else {
	$dbg2=0;
    }

    # Debug
    
    if ($dbg2) {
	print(STDERR "$prog: $subname: Input...\n");
	print(STDERR "\tdata_file: $data_file, nsubdirs: $nsubdirs\n");
    }

    # Get the filename and data directories from the input $data_file
    
    $last_slash=rindex($data_file, '/');
    $tmpstring=substr($data_file, 0, $last_slash);

    if ($nsubdirs <= 0) {
	$ldata_dir=$tmpstring;
	$fname2=substr($data_file, $last_slash+1);
    } else {
	@dirs=split( '/', $data_file);
	$ndirs=@dirs;

	$start_dir=$ndirs - $nsubdirs - 1;
	$first_time=1;
	for ($i=$start_dir; $i<$ndirs; $i++) {
	    if ($first_time) {
		$fname2=$dirs[$i];
		$first_time=0;
	    } else {
		$fname2="${fname2}/$dirs[$i]";
	    }
	}
	$first_time=1;
	for ($i=0; $i<$start_dir; $i++) {
	    if ($first_time) {
		$ldata_dir=$dirs[$i];
		$first_time=0;
	    } else {
		$ldata_dir="$ldata_dir/$dirs[$i]";
	    }
	}
    }

    # Parse the file extension out

    $last_dot=rindex($fname2, '.');
    $ext=substr($fname2, $last_dot+1);

    # Get the filenames
    # fname1=YYYYMMDD/fname (no extension)
    # fname2=YYYYMMDD/fname.ext
 
    $fname1=substr($fname2, 0, $last_dot);

    # Debugging

    if ($dbg2) {
	print(STDERR "$subname: ldata_dir: $ldata_dir, fname1: $fname1, fname2: $fname2, ext: $ext\n");
    }

    # Done

    $return_val=1;
    return($return_val, $ldata_dir, $fname1, $fname2, $ext);
}

1;
