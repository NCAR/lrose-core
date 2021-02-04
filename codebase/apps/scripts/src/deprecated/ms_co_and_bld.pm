package    ms_co_and_bld;
require    Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(execSystemCall expandEnvVar checkKeywordValue removeSpaces runExternalScript doCheckoutFiles doDeleteDirs doCreateDirs);

#
# Externals
#
# The sys_wait_h is required to get the correct return codes from the system() calls.

use POSIX 'sys_wait_h'; 
use Env;
use Cwd;
Env::import();
use Time::Local;                   # make 'date' available
use strict;

1;   # Perl requires this

# =============================== SUBROUTINES ===========================
#
# Subroutine: execSystemCall
#
# Usage:      ($return_val, $cmd_return_val)= execSystemCall($cmd, $test, $dbg)
#
# Function:   Execute the $cmd in a system() call
#
# Input:      $cmd           command to execute
#             $test          test flag, if =1 then only report on what would have done
#             $dbg         debug flag 
#
# Output:     $return_val      1 on success or 0 on error from system()
#             $cmd_return_val  return value from $cmd
# 

sub execSystemCall {
    my ($cmd, $test, $dbg) = @_;

    # Local variables

    my $return_val =0;
    my $subname="execSystemCall";
    my $dbg2=0;
    my $dbg3=0;
    my $cmd_return_val=0;
    my $is_ok=0;

    # Debugging

    if ($dbg == 2) {
	$dbg2=1;
    }
    if ($dbg == 3) {
	$dbg2=1;
	$dbg3=1;
    }

    if ($dbg2) {
	print(STDERR "$subname: Input...\n");
	print(STDERR "\tcmd: $cmd, test: $test\n");
    }

    # Do it

    if ($test) {
	print(STDERR "$subname: Would execute cmd: $cmd\n");
	$return_val=1;
	$cmd_return_val=1;
    } else {
	$return_val=system($cmd);
	$cmd_return_val=WEXITSTATUS($?);
    }

    # Debug

    if ($dbg2) {
	print(STDERR "$subname: return_val: $return_val, return from cmd: $cmd_return_val\n");
    }

    return($return_val, $cmd_return_val);

}

#---------------------------------------------------------------------
# Subroutine: expandEnvVar()
#
# Usage:      ($return_val, $expanded_string) = 
#                   expandEnvVar($string, $dbg)
#
# Function:   Expand the environment variable on the input string
#
# Input:      $string        environment variable string to expand.
#             $dbg         debug flag
#
# Output:     $return_val    1 on success or 0 on error (e.g., env var
#                            syntax used in string but env var not defined)
#             $expanded_string  the expanded environment variable, 
#                               or if not an enviroment variable, just returns 
#                               the input string.
# 

sub expandEnvVar1 {
    my ($string, $dbg) = @_;

    # Local variables

    my ($return_val, $subname);
    my ($dbg2, $dbg3);
    my ($prior_to_dollar, $after_dollar, $dollar_sign, $env_var, $remainder);
    my ($expanded_string, $expanded_env_var);

    # set defaults

    $subname="expandEnvVar";
    $return_val=0;
    $expanded_string=$string;

    # Debugging

    $dbg2=0;
    $dbg3=0;
    if ($dbg == 2) {
	$dbg2=1;
    }
    if ($dbg == 3) {
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
	print(STDERR "WARNING: $subname: the environment variable $string is NOT defined\n");
	$return_val=0;
    }

    # Debug

    if ($dbg2) {
	print(STDERR "$subname: string: $string, expanded_string: $expanded_string\n");
    }

    # Return

    return($return_val, $expanded_string);
}


#------------------------------------------------------------------------
sub expandEnvVar {
    my ($string, $dbg) = @_;

    # Local variables

    my ($return_val, $subname);
    my ($dbg2, $dbg3);
    my ($prior_to_dollar, $after_dollar, $dollar_sign, $env_var, $remainder);
    my ($expanded_string, $expanded_env_var);
    my (@how_many_dollars);
    my ($n);

    # set defaults

    $subname="expandEnvVar";
    $return_val=0;
    $expanded_string=$string;

    # Debugging

    $dbg2=0;
    $dbg3=0;
    if ($dbg == 2) {
        $dbg2=1;
    }
    if ($dbg == 3) {
        $dbg2=1;
        $dbg3=1;
    }


    # Return if not an env var

    if ($string !~ /\$\(/) {
        $return_val=1;
        return($return_val, $string);
    }

    # Okay, we have a $() structure
    @how_many_dollars = split('\$', $string);

    $expanded_string = "";
    $return_val = 0;

    for ($n = 0; $n <= $#how_many_dollars; $n++){

    if ($how_many_dollars[$n] !~ /\(|\)/) {
        $expanded_string = $expanded_string.$how_many_dollars[$n];
        next;
    }

    $how_many_dollars[$n] = "\$" . $how_many_dollars[$n];

    # First split on the $

    ($prior_to_dollar, $after_dollar) = split('\$', $how_many_dollars[$n]);

    # Add the dollar back on. This is needed for the next split to work correctly

    $after_dollar="\$" . $after_dollar;

    # Okay now split on the left paren

    ($dollar_sign, $env_var, $remainder) = split(/\(|\)/, $after_dollar,3);

    if ($dbg3) {
        print(STDERR "$subname: env_var: $env_var, remainder: $remainder\n");
    }

    if (defined $ENV{$env_var}) {
        $expanded_env_var=$ENV{$env_var};
        $return_val ++;
        $expanded_string=$expanded_string.$prior_to_dollar . $expanded_env_var . $remainder;
    }
    else {
        print(STDERR "WARNING: $subname: the environment variable $string is NOT defined\n");
    }

    # Debug

    if ($dbg2) {
        print(STDERR "$subname: string: $string, expanded_string: $expanded_string\n");
    }

    }
    # Return

    return($return_val, $expanded_string);
}
#------------------------------------------------------------------------
#
# Subroutine checkKeywordValue
#
# Usage: ($return_val, $valid_keyword_value) = 
#              checkKeywordValue($keyword, $input_keyword_value, $type, 
#                                *out_arr, $dbg)
#
# Function: Check the $input_keyword_value against the $type and return
#           a valid value in $valid_keyword_value.
#
# Input:    $keyword             keyword, needed for error messages
#           $input_keyword_value value to check
#           $type                type to check
#           $dbg               flag for debugging (1=on, 0=off)
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
  my ($keyword, $input_keyword_value, $type, $out_arr, $dbg) = @_;

  # Local variables

  my($return_val, $subname);
  my($dbg2, $dbg3);
  my($found, $found_valid, $valid_keyword_value, $tmp_keyword_value);
  my($is_ok, $narr, $val, @tmp_arr, $i, $instr, $outstr);

  # Set defaults

  $return_val=0;
  $subname="checkKeywordValue";

  # Dbg

  if ($dbg == 2) {
      $dbg2=1;
  } else {
      $dbg2=0;
  }

  if ($dbg == 3) {
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

  elsif ($type eq "float"){
      if ($input_keyword_value < 0) {
	  print(STDERR "ERROR: $keyword $input_keyword_value is less than 0, will set to 0\n");
	  $valid_keyword_value=0.0;
      } 
  }

  # If a comma-delimited string, split the string

  elsif ($type eq "comma_delimited_string") {
      
      # Only parse if non-blank

      if ($input_keyword_value !~ /\w/) {
	  $narr=0;
	  $out_arr->[0]="";
      } else {
	  @tmp_arr=split('\,', $input_keyword_value);
	  $narr=@tmp_arr;

	  for ($i=0; $i<$narr; $i++) {
	      $instr=$tmp_arr[$i];
	      ($is_ok, $outstr) = &expandEnvVar($instr, $dbg);
	      $out_arr->[$i]=$outstr;
	  }

	  if ($dbg3) {
	      print(STDERR "$subname: keyword: $keyword, keyvalue: $input_keyword_value, numarr: $narr\n");
	      foreach ($i=0; $i<$narr; $i++) {
		  print(STDERR "\ti: $i, $out_arr->[$i]\n");
	      }
	  }
      }

  } 

  # Error condition

  else {
      if ($dbg) {
	  print(STDERR "$subname: Cannot resolve type: $type\n");
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
# Usage: ($return_val, $newstring)=removeSpaces($instring, $dbg) 
#
# Function: Remove leading and trailing spaces from $instring and
#           return in $newstring. If no spaces are found, returns
#           the $instring.
#
# Input:    $instring            string to remove blanks from
#           $dbg               flag for debugging (1=on, 0=off)
#
# Output:   $ret_val             1 on success, 0 on error
#           $newstring           $instring without leading/trailing
#                                   blanks
#
# Overview:
#

sub removeSpaces
{
  my ($instring, $dbg) = @_;

  # Local variables

  my($return_val, $subname);
  my($dbg2);
  my($is_ok, $newstring);

  # Set defaults

  $return_val=0;
  $subname="removeSpaces";
  $newstring=$instring;

  # Dbg

  if ($dbg == 2) {
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


#-------------------------------------------------------------------------
#
# Subroutine runExternalScript
#
# Usage: $return_val = runExternalScript($str, $dbg)
#
# Function: execute user external script
#
# Input:    $str            string to execute
#           $dbg          debug flag
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub runExternalScript
{
  my ($str, $test, $dbg) = @_;

  # Local variables

  my($subname, $return_val);
  my($dbg2, $dbg3);
  my($is_ok, $cmd_ok, $cmd);

  # Set defaults

  $return_val=0;
  $subname="runExternalScript";

  # Dbg

  $dbg2=0;
  $dbg3=0;

  if ($dbg == 2) {
      $dbg2=1;
  }

  if ($dbg == 3) {
      $dbg3=1;
      $dbg2=1;
  }

  # Run the user external script

  if ($dbg) {
      print(STDERR "$subname: Executing: $str\n");
  }
  ($is_ok, $cmd_ok)=execSystemCall($str, $test, $dbg);

  # Done
  
  $return_val=1;
  return($return_val);
}

#-------------------------------------------------------------------------
#
# Subroutine doDeleteDirs
#
# Usage: doDeleteDirs()
#
# Function: delete dirs
#
# Input:    @dir_arr       array of dirs to delete
#
# Output:   directories deleted
#
# Overview:
#

sub doDeleteDirs
{
  my ($dir_arr, $test, $dbg) = @_;

  # Local variables

  my($subname);
  my($dbg2, $dbg3);
  my($dir, $is_ok, $cmd, $cmd_ok, $narr, $i);

  # Set defaults

  $subname="doDeleteDirs";

  # Dbg

  $dbg2=0;
  $dbg3=0;

  if ($dbg == 2) {
      $dbg2=1;
  }

  if ($dbg == 3) {
      $dbg3=1;
      $dbg2=1;
  }

  # Delete the dirs

  $narr=@{$dir_arr};
  for ($i=0; $i<$narr; $i++) {

      $dir = $dir_arr->[$i];

      # Skip blanks

      if ($dir !~ /\w/) {
	  next;
      }

      if (-d $dir) {
	  print(STDERR "Removing $dir\n");
	  $cmd="/bin/rm -rf $dir";
	  ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $dbg);
      }
  }

}

#-------------------------------------------------------------------------
#
# Subroutine doCreateDirs
#
# Usage: doCreateDirs()
#
# Function: create dirs
#
# Input:    @dir_arr        array of dirs to create
#
# Output:   directories created
#
# Overview:
#

sub doCreateDirs
{
  my ($dir_arr, $test, $dbg) = @_;

  # Local variables

  my($subname, $return_val);
  my($dbg2, $dbg3);
  my($dir, $cmd, $is_ok, $cmd_ok, $found_errors, $narr, $i);

  # Set defaults

  $subname="doCreateDirs";
  $return_val=0;

  # Dbg

  $dbg2=0;
  $dbg3=0;

  if ($dbg == 2) {
      $dbg2=1;
  }

  if ($dbg == 3) {
      $dbg3=1;
      $dbg2=1;
  }

  # Create the dirs

  $found_errors=0;
  $narr=@{$dir_arr};

  for ($i=0;$i<$narr; $i++) {

      $dir=$dir_arr->[$i];

      # Skip blanks

      if ($dir !~ /\w/) {
	  next;
      }

      if (!-d $dir) {
	  print(STDERR "Creating $dir\n");
	  $cmd="mkdir -p $dir";
	  ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $dbg);
	  if ((!$test) && ($cmd_ok != 0)) {
	      print(STDERR "ERROR: Cannot create $dir and it does not exist\n");
	      $found_errors=1;
	  }
      }
  }

  if ($found_errors) {
      $return_val=0;
  } else {
      $return_val=1;
  }

  return($return_val);

}

#-------------------------------------------------------------------------
#
# Subroutine doCheckoutFiles
#
# Usage: $return_val = doCheckoutFiles()
#
# Function: checkout distribs file, modify with rtag if needed, then execute it
#
# Input:    @file_arr       array of distribs files
#           $co_dir         checkout directory
#           $do_rtag        flag to do cvs rtag (1=yes, 0=no)
#           $use_rtag       rtag string to use
#
# Output:   $return_val     1 on success, 0 on error
#
# Overview:
#

sub doCheckoutFiles
{
  my ($file_arr, $co_dir, $do_rtag, $use_rtag, $test, $dbg) = @_;

  # Local variables

  my($subname, $return_val);
  my($dbg2, $dbg3);
  my($is_ok, $co_ok, $cmd_ok, $cmd, $use_distribs_file, $dir, $file);
  my($narr, $i);

  # Set defaults

  $return_val=0;
  $subname="doCheckoutFiles";

  # Dbg

  $dbg2=0;
  $dbg3=0;

  if ($dbg == 2) {
      $dbg2=1;
  }

  if ($dbg == 3) {
      $dbg3=1;
      $dbg2=1;
  }

  # Checkout the distribs file so know what else to checkout

  chdir($co_dir);

  $narr=@{$file_arr};

  for ($i=0;$i<$narr; $i++) {

      $file=$file_arr->[$i];

      if ($dbg) {
	  print(STDERR "Checkout distribs file: $file\n");
      }

      if ($do_rtag) {
	  $cmd="cvs co -r $use_rtag $file";
      } else {
	  $cmd="cvs co $file";
      }
      ($is_ok, $co_ok)=execSystemCall($cmd, $test, $dbg);
      if ((!$test) && ($co_ok != 0)) {
	  print(STDERR "ERROR! Cannot checkout the distribs file: ${file}\n");
	  return ($return_val);
      }

      # If rtag is specified, create a copy of the distribs file with the rtag specified

      $use_distribs_file=$file;

      if ($do_rtag) {
	  if ($dbg) {
	      print(STDERR "Copying ${file} and modifying to use rtag checkout\n");
	  }
	
	  $cmd="sed s\/\"cvs co\"/\"cvs co -r ${use_rtag}\"/ ${file} \> ${file}\.${use_rtag}";
	  ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $dbg);
	  
	  $cmd="chmod 777 ${file}.${use_rtag}";
	  ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $dbg);

	  $use_distribs_file="${file}.${use_rtag}";
      }
  
      # Checkout all the files

      chdir($co_dir);
      if ($dbg) {
	  print(STDERR "Running the distribs file: ${use_distribs_file}\n");
      }

      $cmd="${use_distribs_file}";
      ($is_ok, $cmd_ok)=execSystemCall($cmd, $test, $dbg);
      
  } #endforeach

  # Done

  $return_val=1;
  return($return_val);

}


#========================================= EOF =====================================
