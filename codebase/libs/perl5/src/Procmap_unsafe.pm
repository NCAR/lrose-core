# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
# ** Copyright UCAR (c) 1992 - 2010 */
# ** University Corporation for Atmospheric Research(UCAR) */
# ** National Center for Atmospheric Research(NCAR) */
# ** Research Applications Laboratory(RAL) */
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
# ** 2010/10/7 23:12:43 */
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
package    Procmap;
require    Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(abs rsh exec_cmd restart kill_by_pid kill_by_script kill_by_snuff_inst start PMP_check_entry_line PMP_expand_env_var PMP_get_entry_procname PMP_get_entry_instance PMP_get_entry_start_script PMP_get_entry_kill_script PMP_get_entry_hostname PMP_get_entry_priority PMP_get_entry_all PMP_get_entry_include_filename PMP_read_entry_include_file PMP_expand_entry_line PMP_read_proc_list_file PMP_email_file PMP_get_num_procs_machines increment_process_array PMP_get_process_stats PMP_get_machine_proc_stats PMP_get_log_first_last_line PMP_get_datestrings PMP_extract_utime_from_logline PMP_line_time_valid Parse_line_for_restart PMP_get_process_restart_time PMP_get_pct_time_impaired PMP_get_sys_time_gaps PMP_conv_utime_to_human PMP_get_category_time_gaps PMP_get_cat_impaired_pct PMP_calc_impaired_pct PMP_get_sys_impaired_pct);

#
# This module has a set of subroutines for use in the procmap
# Perl scripts that do not use Toolsa::safe_system.  In all other
# respects, this file is identical to Procmap.pm.
#
# Author: Nancy Rehak & Deirdre Garvey		NCAR/RAP/SDG
#========================================================================
#
# Externals
#

use Env;
Env::import();

use Sys::Hostname;
use Time::Local;
use Env qw(RAP_LIB_DIR);
use lib "$RAP_LIB_DIR/perl5/";

#
# Allow override of rsh (usu. to ssh) via env. var.
#
if ( $ENV{RSH} ) {
   $rsh = $ENV{RSH};
}
else {
   $rsh = "rsh";
}

# Set $debug=0 to turn off debug messages, set to $debug=1 to turn on messages

$debug=0;

# Local constants

$Use_local=1;		# flag value for timezone (Local)
$Use_GMT=2;		# flag value for timezone (GMT)    

$Ave_maxint=120;	# average maxint value returned from print_procmap

$Miss_restart=1;       # flag value for restart due to missing process
$Hung_restart=2;       # flag value for restart due to hung process

$Proc_idx=0;           # index into proc_info for process name
$Inst_idx=1;           # index into proc_info for process instance
$Mach_idx=2;           # index into proc_info for process hostname
$Miss_idx=3;           # index into proc_info for restart due to missing (bin)
$Hung_idx=4;           # index into proc_info for restart due to hung (bin)

$Gap_start=0;		# index into arr_proc_gaps for gap start time
$Gap_end=1;		# index into arr_proc_gaps for gap end time
$Gap_number=2;		# index into arr_proc_gaps for gap number
$Gap_proc_name=3;	# index into arr_proc_gaps for process name
$Gap_proc_instance=4;	# index into arr_proc_gaps for process instance
$Gap_proc_machine=5;	# index into arr_proc_gaps for process hostname

$Pri_idx=0;             # index into arr_priorities_downtime for process priority NUMBER
$Pri_descrip_idx=1;     # index into arr_priorities_downtime for process priority DESCRIPTION
$Pri_timespan_idx=2;    # index into arr_priorities_downtime for process priority downtime in secs
$Pri_start_idx=3;       # index into arr_priorities_downtime for start time (ignore, used as temp)
$Pri_end_idx=4;         # index into arr_priorities_downtime for end time (ignore, used as temp)
$Pri_weight_idx=5;      # index into arr_priorities_downtime for process category WEIGHT
$Pri_num_procs=6;       # index into arr_priorities_downtime for number of processes in this category
$Pri_pct_impaired=7;    # index into arr_priorities_downtime for percent time impaired

# Make sure the file returns a 1 because PERL seems to require this

1;

#---------------------------------------------------------------------
# subroutine to get the absolute value of a number
#
# usage: $abs_value = abs($value)
#

sub abs
{
    local ($value) = @_;

    if ($value < 0)
    {
	return -$value;
    }

    return $value;
}


#---------------------------------------------------------------------
# &rsh: Run a command on a remote system, with a timeout.
# Arguments:
#     $remote: the name of the remote system.
#     $timeout: the timeout, in seconds.
#     @rcmd: an array consisting of shell commands to be passed to the
#         remote system.  The elements of the array are (implicitly)
#         joined with a space character before being passed to exec().
# Returns: the output of rsh(1), or undef on timeout or authentication
#     failure.
sub rsh {
    my($remote, $timeout, @rcmd) = @_;
    my($ruser, $result, $cpid, $tmp);

    # Has a different remote user been specified?
    ($ruser, $remote) = split(/@/, $remote, 2) if $remote =~ /@/;

    if ($ruser) {
	$rcmd = "$rsh $remote -n -l $ruser @rcmd";
    }
    else {
	$rcmd = "$rsh $remote -n @rcmd";
    }

    system( "$rcmd" );

    # Return an empty string
    " ";
}

#---------------------------------------------------------------------
# Subroutine to execute a command on the given host.  If
# the host is "local" or "localhost", the command is
# executed locally.
#
# usage: exec_cmd($host, $timeout, $cmd)
#

sub exec_cmd
{
    local ($host, $timeout, $cmd) = @_;
    local($result, $local_hostname, $request_hostname);

    # first try local - no need to contact DNS

    if ($host eq "local" ||
	$host eq "localhost" ||
        $host eq hostname())
    {
        $result = Toolsa::safe_system( "$cmd", $timeout );
        return $result;
    }

    # now check if host is local

    ($local_hostname) = gethostbyname( hostname() );
    ($request_hostname) = gethostbyname( $host );

    # Now execute the command

    if ($request_hostname eq $local_hostname)
    {
        $result = Toolsa::safe_system( "$cmd", $timeout );
    }
    else
    {
	$result = rsh($host, $timeout, $cmd);
    }

    $result;
}

#---------------------------------------------------------------------
# Subroutine to restart a process.  Kills the old process on the given
# host if $kill_pid > 0.
#
# usage: restart($host, $proc_name, $instance,
#                $start_script, $kill_script, $kill_pid,
#                $kill_process_flag, $override_timeout)
#

sub restart
{
    local ($host, $proc_name, $instance,
	   $start_script, $kill_script, $kill_pid,
	   $kill_process_flag, $override_timeout) = @_;
    local ($timeout);

    if ( $override_timeout ) {
       $timeout = $override_timeout;     # seconds
    } 
    else {
       $timeout = 10;     # seconds
    } 

    #
    # Make sure the old process is dead.  First try to kill it
    # using its PID, then use the kill script.
    #

    if ($kill_process_flag)
    {
	if ($kill_pid > 0)
	{
	    if ($debug)
	    {
		print "killing pid $kill_pid on host $host\n";
	    }

	    exec_cmd($host, $timeout, "kill -9 $kill_pid");
	}

	sleep($kill_sleep);

	if ($debug)
	{
	    print "killing process with script $kill_script on host $host\n";
	}

	if ($kill_script eq "snuff_inst")
	{
	    exec_cmd($host, $timeout,
		     "snuff \"$proc_name(.+)$instance\"");
	}
	else
	{
	    exec_cmd($host, $timeout, $kill_script);
	}

	sleep($kill_script_sleep);
    }

    #
    # Now restart the process using the given script.
    # Redirects are put in to prevent the process from hanging.
    #

    if ($debug)
    {
	print "restarting process with script $start_script on host $host\n";
    }

    $result = exec_cmd($host, $timeout, "$start_script ");
    print "Output from restart of $start_script: $result\n";
}


#---------------------------------------------------------------------
# Kills the process on the given host if $kill_pid > 0.
#
# usage: kill_by_pid($host, $kill_pid, $override_timeout)
#

sub kill_by_pid
{
    local ($host, $kill_pid, $override_timeout) = @_;
    local ($timeout);

    if ( $override_timeout ) {
       $timeout = $override_timeout;     # seconds
    }
    else {
       $timeout = 10;     # seconds
    }


    if ($kill_pid > 0)
    {
        print "killing pid $kill_pid on host $host\n";
        exec_cmd($host, $timeout, "kill -TERM $kill_pid");
        exec_cmd($host, $timeout, "kill -QUIT $kill_pid");
        exec_cmd($host, $timeout, "kill -INT $kill_pid");
    }
}


#---------------------------------------------------------------------
# Kills the process on the given host using the kill script.
#
# usage: kill_by_script($host, $kill_script, $override_timeout)
#

sub kill_by_script
{
    local ($host, $kill_script, $override_timeout) = @_;
    local ($timeout);

    if ( $override_timeout ) {
       $timeout = $override_timeout;     # seconds
    }
    else {
       $timeout = 10;     # seconds
    }


    print "killing process with script $kill_script on host $host\n";

    exec_cmd($host, $timeout, $kill_script);
}

#---------------------------------------------------------------------
# Kills the process on the given host using the snuff_inst script.
#
# usage: kill_by_snuff_inst($host, $proc_name, $instance, $override_timeout)
#

sub kill_by_snuff_inst
{
    local ($host, $proc_name, $instance, $override_timeout) = @_;
    local ($timeout);

    if ( $override_timeout ) {
       $timeout = $override_timeout;     # seconds
    }
    else {
       $timeout = 10;     # seconds
    }

    print "killing process $proc_name $instance with snuff_inst on host $host\n";

    exec_cmd($host, $timeout,
	     "snuff_inst " . $proc_name . " " . $instance);
}


#---------------------------------------------------------------------
# Subroutine to start a process.  
#
# usage: start($host, $start_script, $override_timeout)
#

sub start
{
    local ($host, $start_script, $override_timeout) = @_;
    local ($timeout);

    if ( $override_timeout ) {
       $timeout = $override_timeout;     # seconds
    }
    else {
       $timeout = 10;     # seconds
    }

    print "starting process with script $start_script on host $host\n";

    $result = exec_cmd($host, $timeout, "$start_script ");
    print "Output from restart of $start_script: $result\n";
}


#---------------------------------------------------------------------
# Subroutine: PMP_check_entry_line
#
# Usage:      PMP_check_entry_line($entry_line)
# Function:   Check an entry line from a proc_file_list file to see whether to
#             parse it or skip it.
# Input:      An entry line from a proc_file_list
# Output:     Returns 1 if a line to parse, 0 if a line to skip.
# 
# Overview:   Reads the input entry_line and tests whether it contains text to
#             parse. Returns 0 if a blank line or a comment.
#

sub PMP_check_entry_line {
    local ($entry_line) = @_;

    # Skip lines with leading comment characters or blank lines

    if ($entry_line =~ /^#/) {
	 return(0);
     }

    if ($entry_line !~ /\S/) {
	return(0);
    }
    
    return(1);
}

#---------------------------------------------------------------------
# Subroutine: PMP_expand_env_var()
#
# Usage:      ($return_val, $expanded_string) = PMP_expand_env_var($string)
# Function:   Expand the environment variable on the input string
# Input:      An environment variable string to expand.
# Output:     Returns (in return_val), 1 on success or 0 on error (e.g., env var
#                syntax used in string but env var not defined)
#             Returns (in expanded_string), the expanded environment variable, 
#                or if not an enviroment variable, just returns the input string.
# 
# Overview:   Reads the string searching for a "$()" and then parses the
#             enclosed text as an environment variable.  Note, this does not
#             handle an enviroment variable occurring in the middle of a text
#             string, only at the beginning.
#

sub PMP_expand_env_var {
    local ($string) = @_;

    # Initialize

    local($return_val, $sub_name);
    $sub_name="PMP_expand_env_var";
    $return_val=0;
    $expanded_string=$string;

    if ($string =~ /\$\(/) {
	($dollar_sign, $env_var, $remainder) = split(/\(|\)/, $string);

	if (defined $ENV{$env_var}) { 
	    $expanded_env_var=$ENV{$env_var};
	    $expanded_string=join "",$expanded_env_var,$remainder;
	    $return_val=1;
	}
	else {
	    print(STDERR "WARNING: $sub_name: the environment variable $string is NOT defined\n");
	    $return_val=0;
	}
    }

    # It's not an environment variable

    else {
	$return_val=1;
    }

    return($return_val, $expanded_string);
}

#---------------------------------------------------------------------
# Subroutine: PMP_get_entry_procname($entry_line)
#
# Usage:      ($return_val, $procname) = PMP_get_entry_procname($entry_line)
# Function:   Reads the input proc_file_list entry string and returns
#             the process_name string. Expands env var if needed.
# Input:      An entry line from a proc_file_list.
# Output:     Returns (in return_val), 1 on success, 0 on failure.
#             Returns (in procname) the process_name string.
# 
# Overview:   Reads the input proc_file_list entry string and parses
#             it for the process_name string. Calls PMP_expand_env_var()
#             to expand enviromnent variable if needed.
#

sub PMP_get_entry_procname {
    local ($entry_line) = @_;

    # Initialize

    local($return_val, $sub_name);

    ($expected_name, $expected_instance, $expected_start_script,
     $expected_kill_script, $expected_host) =
	 split(/\s+/, $entry_line);
    
    ($return_val, $expand_line)=&PMP_expand_env_var($expected_name);
    return($return_val, $expand_line);
}


#---------------------------------------------------------------------
# Subroutine: PMP_get_entry_instance($entry_line)
#
# Usage:      ($return_val, $instance) = PMP_get_entry_instance($entry_line)
# Function:   Reads the input proc_file_list entry string and returns
#             the instance string. Expands env var if needed.
# Input:      An entry line from a proc_file_list.
# Output:     Returns (in return_val), 1 on success, 0 on failure.
#             Returns (in instance) the instance string.
# 
# Overview:   Reads the input proc_file_list entry string and parses
#             it for the instance string. Calls PMP_expand_env_var()
#             to expand environment variable if needed.
#

sub PMP_get_entry_instance {
    local ($entry_line) = @_;

    # Initialize

    local($return_val, $sub_name);

    ($expected_name, $expected_instance, $expected_start_script,
     $expected_kill_script, $expected_host) =
	 split(/\s+/, $entry_line);
    
    ($return_val, $expand_line)=&PMP_expand_env_var($expected_instance);
    return($return_val, $expand_line);
}


#---------------------------------------------------------------------
# Subroutine: PMP_get_entry_start_script($entry_line)
#
# Usage:      ($return_val, $start_script) = PMP_get_entry_start_script($entry_line)
# Function:   Reads the input proc_file_list entry string and returns
#             the start_script string. Expands env var if needed.
# Input:      An entry line from a proc_file_list.
# Output:     Returns (in return_val), 1 on success, 0 on failure.
#             Returns (in start_script), the start_script string.
# 
# Overview:   Reads the input proc_file_list entry string and parses
#             it for the start_script string. Calls PMP_expand_env_var()
#             to expand enviromnent variable if needed.
#

sub PMP_get_entry_start_script {
    local ($entry_line) = @_;

    # Initialize

    local($return_val, $sub_name);

    ($expected_name, $expected_instance, $expected_start_script,
     $expected_kill_script, $expected_host) =
	 split(/\s+/, $entry_line);
    
    ($return_val, $expand_line)=&PMP_expand_env_var($expected_start_script);
    return($return_val, $expand_line);
}


#---------------------------------------------------------------------
# Subroutine: PMP_get_entry_kill_script($entry_line)
#
# Usage:      ($return_val, $kill_script) = PMP_get_entry_kill_script($entry_line)
# Function:   Reads the input proc_file_list entry string and returns
#             the kill_script string. Expands env var if needed.
# Input:      An entry line from a proc_file_list.
# Output:     Returns (in return_val), 1 on success, 0 on failure.
#             Returns (in kill_script), the kill_script string.
# 
# Overview:   Reads the input proc_file_list entry string and parses
#             it for the kill_script string. Calls PMP_expand_env_var()
#             to expand enviromnent variable if needed.
#

sub PMP_get_entry_kill_script {
    local ($entry_line) = @_;

    # Initialize

    local($return_val, $sub_name);

    ($expected_name, $expected_instance, $expected_start_script,
     $expected_kill_script, $expected_host) =
	 split(/\s+/, $entry_line);
    
    ($return_val, $expand_line)=&PMP_expand_env_var($expected_kill_script);
    return($return_val, $expand_line);
}


#---------------------------------------------------------------------
# Subroutine: PMP_get_entry_hostname($entry_line)
#
# Usage:      ($return_val, $hostname) = PMP_get_entry_hostname($entry_line)
# Function:   Reads the input proc_file_list entry string and returns
#             the hostname string. Expands env var if needed.
# Input:      An entry line from a proc_file_list.
# Output:     Returns (in return_val), 1 on success, 0 on failure.
#             Returns (in hostame), the hostname string.
# 
# Overview:   Reads the input proc_file_list entry string and parses
#             it for the hostname string. Calls PMP_expand_env_var()
#             to expand enviromnent variable if needed. Modifies the
#             string to hostname() if set to a variation of "local".
#

sub PMP_get_entry_hostname {
    local ($entry_line) = @_;

    # Initialize

    local($return_val, $sub_name);

    ($expected_name, $expected_instance, $expected_start_script,
     $expected_kill_script, $expected_host) =
	 split(/\s+/, $entry_line);

    ($return_val, $expand_line)=&PMP_expand_env_var($expected_host);
    
    if ($expand_line eq "local" || $expand_line eq "localhost") {
	$expand_line = hostname();
    }
    return($return_val, $expand_line);
}


#---------------------------------------------------------------------
# Subroutine: PMP_get_entry_priority($entry_line)
#
# Usage:      ($return_val, $priority) = PMP_get_entry_priority($entry_line)
#
# Function:   Reads the input proc_file_list entry string and returns
#             the priority string. Expands env var if needed.
# Input:      An entry line from a proc_file_list.
# Output:     Returns (in return_val), 1 on success, 0 on failure.
#             Returns (in priority), the priority string.
# 
# Overview:   Reads the input proc_file_list entry string and parses
#             it for the priority string. Calls PMP_expand_env_var()
#             to expand enviromnent variable if needed. Some special
#             error handling is required since the priority is NOT
#             required in the proc_list file except for statistics.
#

sub PMP_get_entry_priority {
    local ($entry_line) = @_;

    # Initialize

    local($return_val, $expand_line, $sub_name);
    $return_val=0;
    $expand_line="(null)";

    # Are there 5 items in the line? Only try to parse if there are

    @array = split(/\s+/, $entry_line);

    if ($#array == 5) {
	($expected_name, $expected_instance, $expected_start_script,
	 $expected_kill_script, $expected_host, $expected_priority) =
	     split(/\s+/, $entry_line);

	($return_val, $expand_line)=&PMP_expand_env_var($expected_priority);
    }
    
    return($return_val, $expand_line);
}


#---------------------------------------------------------------------
# Subroutine: PMP_get_entry_all($entry_line)
#
# Usage:      ($return_val, $procname, $instance, $start_script, 
#             $kill_script, $host) = PMP_get_entry_all($entry_line)
# Function:   Reads the input proc_file_list entry string and returns
#             all the individual field strings. Expands env vars if needed.
# Input:      An entry line from a proc_file_list.
# Output:     Returns (in return_val), 1 on success, 0 on failure.
#             Returns (in procname), the process name string.
#             Returns (in instance), the instance string.
#             Returns (in start_script), the start_script string.
#             Returns (in kill_script), the kill_script string.
#             Returns (in host), the hostname string.
#             Returns (in priority), the priority string.
# 
# Overview:   Reads the input proc_file_list entry string and parses
#             it for each of the field strings. Calls PMP_get_entry_*() to
#             get each field.
#

sub PMP_get_entry_all {
    local ($entry_line) = @_;

    # Initialize

    local($return_val, $sub_name);
    $return_val=0;

    ($return_procname, $procname) = &PMP_get_entry_procname($entry_line);
    ($return_instance, $instance) = &PMP_get_entry_instance($entry_line);
    ($return_start, $start_script) = &PMP_get_entry_start_script($entry_line);
    ($return_kill, $kill_script) = &PMP_get_entry_kill_script($entry_line);
    ($return_host, $hostname) = &PMP_get_entry_hostname($entry_line);
    ($return_priority, $priority) = &PMP_get_entry_priority($entry_line);

    # The priority is NOT a required field

    if (($return_procname<1) || ($return_instance<1) || ($return_start<1) || ($return_kill<1) || ($return_host<1)) {
	$return_val=0;
    }
    else {
	$return_val=1;
    }

    return($return_val, $procname, $instance, $start_script, $kill_script, $hostname, $priority);
}


#---------------------------------------------------------------------
# Subroutine: PMP_get_entry_include_filename($entry_line)
#
# Usage:      ($return_val, $include_fname) = PMP_get_entry_include_filename($entry_line)
# Function:   Reads the input proc_file_list entry string and returns
#             the include filename string. Expands env var if needed.
# Input:      An entry line from a proc_file_list.
# Output:     Returns (in return_val), 1 if an include filename directive, 0 if not.
#             Returns (in include_fname), the include filename string.
# 
# Overview:   Reads the input proc_file_list entry string and parses
#             it for the include filename string. Calls PMP_expand_env_var()
#             to expand enviromnent variable if needed. 
#

sub PMP_get_entry_include_filename {
    local ($entry_line) = @_;

    # Initialize

    local($return_val, $sub_name);
    $expand_line=$entry_line;
    $return_val=0;

    if ($entry_line =~ /^include/) {
	
	($include_cmd, $include_file)=split(/\s+/, $entry_line);
	($return_val, $expand_line)=&PMP_expand_env_var($include_file);

    }
    return($return_val, $expand_line);
}


#---------------------------------------------------------------------
# Subroutine: PMP_read_entry_include_file()
#
# Usage:      ($return_val, $new_array_idx)=PMP_read_entry_include_file(*arr_entry_lines, $arr_start_idx, *arr_start_scripts, $entry_line)
# Input:      An entry line from a proc_file_list containing an include directive.
#             An array (usually $expected_entries) to append the contents of the
#                include file into, line by line.
#             The starting array index to use for arr_entry_lines[]. This is to
#                handle the array being overwritten, not appended to, on each
#                loop through this script.
#             An array (usually $started) to add the start_script names to.
#
# Output:     Returns 1 if okay, 0 on error (e.g., cannot open include file).
#             Returns the new input_array starting array index.
#             Returns the appended-to array in arr_entry_lines.
#             Returns the updated start_script array in arr_start_scripts.
#
# Overview:   Gets the include filename from the entry_line. Opens the file
#             and reads the contents. Expands each entry line in the include
#             file and appends the expanded line to the arr_entry_lines[].
#             Gets the start script name from the entry_line and updates the
#             arr_start_scripts[].
#
sub PMP_read_entry_include_file{
    local (*arr_entry_lines, $arr_start_idx, *arr_start_scripts, $entry_line) = @_;

    # Initialize

    local($return_val, $sub_name);
    $sub_name="PMP_read_entry_include_file";
    $return_val=0;

    # Get the include file name

    ($is_ok, $include_fname)=&PMP_get_entry_include_filename($entry_line);

    # Error return

    if ($is_ok != 1) {
	print(STDERR "ERROR: $sub_name: Cannot parse the include filename in line $entry_line\n");
	return($return_val, $arr_start_idx);
    }
	
    # Open the include file 

    unless (open(EXPECTED_INCLUDE, "$include_fname"))
    {
	print (STDERR "ERROR: $sub_name: Cannot open process list include file $include_fname at $datetime.\n");
	return($return_val, $arr_start_idx);
	    
    } # endunless - open expected process list file

    # Set the array index counter for the input array so we can append to it

    $arr_idx=$arr_start_idx;

    # Parse the include file and append to the input array

    while ($entry = <EXPECTED_INCLUDE>)
    {
	# Read the entry line, skip if not what we want

	$read_line=&PMP_check_entry_line($entry);

	# Parse the line

	if ($read_line == 1) {
	    
	    ($ok_parse, $is_include, $full_entry_line) = &PMP_expand_entry_line($entry);

	    if ($is_include == 1) {

		print(STDERR "WARNING: $sub_name: Cannot parse a nested include file... yet...\n");
		return($return_val, $arr_start_idx);
	    }

	    if ($ok_parse == 1) { 

		$arr_entry_lines[$arr_idx]=$full_entry_line;
		$arr_idx++;

		($ok_get, $expanded_start_script)=&PMP_get_entry_start_script($entry);

		if ($ok_get < 1) {
		    print(STDERR "ERROR: $sub_name: Cannot get start_script name from line $entry\n");
		    return($return_val, $arr_start_idx);
		}

		$arr_start_scripts{$expanded_start_script} = 0;
	    }

	    # Log an error if cannot parse the line

	    else {
		printf(STDERR "WARNING: $sub_name: file %s, line %s incorrect format\n", $include_fname, $entry); 
	    }

	} #endif ($read_line == 1)
    } #endwhile

    close(EXPECTED_INCLUDE);

    $return_val=1;
    return($return_val, $arr_idx);
}


#---------------------------------------------------------------------
# Subroutine: PMP_expand_entry_line()
#
# Usage:      ($return_val, $is_include, $expanded_line)=PMP_expand_entry_line($entry_line)
# Function:   Expand the entry line enviroment variables (if any). Do error
#             checks on the line.
# Input:      An entry line from a proc_file_list
# Output:     Returns (in return_val), 1 if able to parse the line or 0 on error.
#             Returns (in is_include), 1 if an include directive or 0 if not.
#             Returns (in expanded_line), the expanded entry line or the original 
#             line if no expansion needed.
# 
# Overview:   Reads the input entry_line and does error checks. Calls 
#             PMP_expand_env_var() to expand environment variables as needed 
#             for each field. Modifies other fields as needed. Returns a
#             3-part item.
#

sub PMP_expand_entry_line{
    local ($entry_line) = @_;

    # Initialize

    local($return_val, $sub_name);
    $sub_name="PMP_expand_entry_line";
    $return_val=0;
    $is_include=0;
    $expanded_line=$entry_line;

    # Are there 4-5 items on the entry line (as expected)?

    @array = split(/\s+/, $entry_line);

    if ($#array >= 4) {

	$return_val=1;

	# Get the individual expanded fields

	($is_ok, $expanded_name, $expanded_instance, $expanded_start_script, $expanded_kill_script, $expanded_host, $expanded_priority)=&PMP_get_entry_all($entry_line);

	if ($is_ok == 0) {
	    $return_val=0;
	    print(STDERR "ERROR: $sub_name: Problem parsing entry line: $entry_line\n");
	    return($return_val, $is_include, $entry_line);
	}

	# Create a line with all the expanded fields. Include priority
	# only if there are 5 items on the line.

	$expanded_line=join " ",$expanded_name, $expanded_instance, $expanded_start_script, $expanded_kill_script, $expanded_host;

	if ($#array == 5) {
	    $expanded_line=join " ", $expanded_line, $expanded_priority;
	}

	if ($debug) {
		print(STDERR "Expanded entry line is $expanded_line\n");
	}
    }

    else {

	# Is this an include directive ?
	# If so, expand the directive and create a new line with it.

	if ($entry_line =~ /^include/) {
	    $is_include=1;

	    $expanded_line=join " ", $include_cmd, $expanded_field;

	    $return_val=1;
	}
    } #endelse ($#array >= 4)

    return($return_val, $is_include, $expanded_line);
}

#---------------------------------------------------------------------
# Subroutine: PMP_read_proc_list_file()
#
# Usage:      ($return_val, $num_entry_lines)=
#                  PMP_read_proc_list_file($proc_list_fname,
#                  *arr_entry_lines, *arr_start_scripts, $logfile)
# Input:      The proc_file_list to read.
#             An array (usually $expected_entries) to put the expected proc
#                lines from the proc_list_file into, line by line.
#             An array (usually $started) to add the start_script names to.
#             A logfile file handle to write to, set to (null) if none wanted.
#                The logfile must have been already opened before calling this
#                subroutine.
#
# Output:     Returns (in $return_val), 1 if okay, 0 on error.
#             Returns (in $num_entry_lines), the total number of processes.
#             Returns the filled array in arr_entry_lines.
#             Returns the start_script array in arr_start_scripts.
#             Writes debug log messages to logfile if set.
#
# Overview:   Opens the proc_list_file and reads the contents. Checks and
#             expands each entry line in the file and adds the expanded 
#             line to the specified arr_entry_lines. Gets the start script name 
#             from each entry_line and adds it to the arr_start_scripts.
#
sub PMP_read_proc_list_file{
    local ($proc_list_fname, *arr_entry_lines, *arr_start_scripts, $logfile) = @_;

    # Initialize

    local($return_val, $sub_name);
    $sub_name="PMP_read_proc_list_file";
    $return_val = 0;
    $num_entry_lines = 0;
    $write_to_log=0;

    if ($logfile !~ (null)) {
	$write_to_log=1;
	}


    # Open the proc_list file

    unless (open(EXPECTED, "$proc_list_fname"))
    {
	print (STDERR "ERROR: $sub_name: Cannot open expected process list file $proc_list_fname\n");
	if ($write_to_log) {
		print($logfile "ERROR: $sub_name: Cannot open expected process list file $proc_list_fname\n");
	}

	return($return_val, $num_entry_lines);

    } # endunless - open expected process list file

    #
    # loop through the expected entries, ignoring
    # comments and blank lines
    #

     if (($debug) && ($write_to_log))
     {
 	print($logfile "\nExpected processes:\n");
     }
 
    while ($entry = <EXPECTED>)
    {
	# Read the entry line, skip if not what we want

	$read_line=&PMP_check_entry_line($entry);

	# Parse the line. If okay, update the expected_entries[] and started[]
	# arrays. These are used later on for comparison.

	if ($read_line == 1) {
	    
	    ($ok_parse, $is_include, $full_entry_line) = &PMP_expand_entry_line($entry);

	    if ($ok_parse == 1) { 

		# The line is an include directive, need to open the included
		# file and parse it. read_entry_include_file() will update the
		# expected_entries[] and started[] arrays.

		if ($is_include == 1) {

		    ($is_ok, $new_num_entry)=&PMP_read_entry_include_file(*arr_entry_lines, $num_entry_lines, *arr_start_scripts, $entry);

		    # Error exit

		    if ($is_ok == 0) {
			printf(STDERR "WARNING: $sub_name: Problem reading included file in line %s\n", $entry);
			if ($write_to_log) {
				printf ($logfile "WARNING: $sub_name: Problem reading included file in line %s\n", $entry);
			}

			return($return_val, $num_entry_lines);
		    }

		    # get the new number of entries in the expected_entries[] array
		    # so can use later

		    $num_entry_lines=$new_num_entry;
		}
		
		else {
 		    if (($debug) && ($write_to_log))
 		    {
 			print($logfile "   $full_entry_line");
 		    }
		    
		    $arr_entry_lines[$num_entry_lines] = $full_entry_line;
		    $num_entry_lines++;

		    ($is_ok, $return_start_script)=&PMP_get_entry_start_script($entry);
		    $arr_start_scripts{$return_start_script} = 0;
		}

	    } #endif ($ok_parse == 1)

	    else {
		print(STDERR "WARNING: $sub_name: line has incorrect format\n");
		print(STDERR "\t$entry"); 
		if ($write_to_log) {
			printf ($logfile "WARNING: $sub_name: line %s incorrect format\n", $entry);
		}
	    }
	} #endif (read_line == 1)

    } #endwhile

    close(EXPECTED);

    $return_val = 1;
    return($return_val, $num_entry_lines);
}


#---------------------------------------------------------------------
# Subroutine: PMP_email_file()
#
# Usage:      $return_val=PMP_email_file($email_address_file, $infile, $subject, $debug_flag)
#
# Function:   Send the input $infile to the list of email addresses in the
#             $email_address_file.
#
# Input:      $email_address_file     file containing a list of email 
#                                        addresses (one name per line)
#             $infile                 file to be mailed
#             $subject                subject for email
#             $debug_flag             1 to print debug messages
#
# Output:     $return_val             1 if successful or 0 on error.
# 
# Overview:   Opens, reads, and parses the email address file and
#             sends the input infile to each address.
#
sub PMP_email_file
{
    local ($email_address_file, $infile, $subject, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name = "PMP_email_file";
    $ret_val = 0;

    # Open the email address file

    if (!open(MAIL_LISTFILE, $email_address_file)) {
	print(STDERR "ERROR: $sub_name: Cannot open email address file $email_address_file\n");
	return($ret_val);
    }

    # Read the file, skip lines with leading # (comments) or blank lines

    while ($line = <MAIL_LISTFILE>) {

	if (($line !~ /^#/) && ($line =~ /\w/)) {

	     # Chop the trailing newline

	     chop($line);

	     # Debug output

	     if ($debug_flag) {
		 print(STDERR "Running: mail -s '$subject' $line < $infile\n");
	     }

	     # Send the file to the listed address

	     system("mail -s '$subject' $line < $infile");
	 }
	}

    $ret_val=1;
    return($ret_val);
}

#---------------------------------------------------------------------
# Subroutine: PMP_get_num_procs_machines()
#
# Usage:      ($return_val, $num_procs, $num_machines)=Get_num_procs_machines
#                 ($process_list_file, $debug_flag)
#
# Function:   Read the process_list file and count up the total number of
#             processes and machines.
# Input:      A process list file.
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in num_procs), the number of processes in the process list.
#             Returns (in num_machines), the number of machines in the process list.
# 
# Overview:   Opens, reads, and parses the process list file using Procmap lib
#             routines. to count the total number of processes and the total number of
#             machines.
#
sub PMP_get_num_procs_machines
{
    local ($proc_list_file, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_get_num_procs_machines";
    $ret_val=0;
    $num_procs=0;
    $num_machines=0;

    # Open the process list file and get the array of expanded entry lines

    ($is_ok, $num_entry_lines) = PMP_read_proc_list_file($proc_list_file, *entries, *started, (null));

    # Error return

    if ($is_ok == 0) {
	return($ret_val, $num_procs, $num_machines);
    }

    # Read through the entry lines and build bins of number of machines
    # and number of processes

    $counter=0;
    foreach $entry (@entries) {
	
	# Go through the entry lines and retrieve the machine for each process

	($is_ok, $machine) = PMP_get_entry_hostname($entry);
	if ($is_ok == 0) {
	    print(STDERR "ERROR: $sub_name: on parsing hostname from entry line $entry\n");
	    return($ret_val, $num_procs, $num_machines);
	}

	# Go through the array of machines looking for a match

	$found_it=0;
	$arr_size=@machine_arr;

	for ($i=0; $i<$arr_size; $i++) {
	    if ($machine_arr[$i] eq $machine) {
		$found_it = 1;
	    }
	}

	# Never found a match, increment the array

	if ($found_it == 0) {
	    $machine_arr[$counter] = $machine;
	    $counter++;
	}

    } #endforeach (entries)

    $num_machines = @machine_arr;
    $num_procs = $num_entry_lines;
    $ret_val = 1;

    # Return

    return($ret_val, $num_procs, $num_machines);
}

#---------------------------------------------------------------------
# Subroutine: increment_process_array()
#
# Usage:      $return_val = increment_process_array($proc_name, $instance, $machine, 
#                           $why_restart, *proc_info);
#
# Function:   Increment the proc_info array bins for the input process.
#             The bins are: number of restarts due to process missing, 
#             number of restarts due to process hung.
#
# Input:      proc_name: the process name string
#             instance:  the process instance
#             machine:   the machine the process is running on (if known)
#             why_restart: reason for restart, 1=missing, 2=hung
# Output:     Returns 1 on success, 0 on error.
#             Returns the incremented proc_info array.
# 
# Overview:   Search the existing proc_info array for the input process
#             and instance and increment the array or an existing process/instance
#             bins as appropriate. Sorry about the kludgey multi-dimensional
#             array usage, there is probably a much better way to do this!
#
#
sub increment_process_array
{
    local ($proc_name, $instance, $machine, $why_restart, *proc_info) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $num_entries=@proc_info;
    $found_it=0;

    # Search through the array for the process and instance, if find
    # it then increment the bins

    for ($i=0; $i<$num_entries; $i++) {
	if (($proc_name eq $proc_info[$i][$Proc_idx]) && ($instance eq $proc_info[$i][$Inst_idx])) {
	    $found_it=1;
	    if ($why_restart == $Miss_restart) {
		$proc_info[$i][$Miss_idx] = $proc_info[$i][$Miss_idx] + 1;
	    }
	    if ($why_restart == $Hung_restart) {
		$proc_info[$i][$Hung_idx] = $proc_info[$i][$Hung_idx] + 1;
	    }

	    # force drop out of loop

	    last;
	}
    }

    # never found a matching entry, need to increment the array

    if ($found_it == 0) {
	$proc_info[$num_entries][$Proc_idx]=$proc_name;
	$proc_info[$num_entries][$Inst_idx]=$instance;
	$proc_info[$num_entries][$Mach_idx]=$machine;
	$proc_info[$num_entries][$Miss_idx]=0;
	$proc_info[$num_entries][$Hung_idx]=0;
	
	if ($why_restart == $Miss_restart) {
	    $proc_info[$num_entries][$Miss_idx] = 1;
	}
	if ($why_restart == $Hung_restart) {
	    $proc_info[$num_entries][$Hung_idx] = 1;
	}
    }

    return(1);
}

#---------------------------------------------------------------------
# Subroutine: increment_system_array()
#
# Usage:      $return_val = increment_system_array($line, *machine_proc_info);
#
# Function:   Increment the machine_proc_info system restart bin.
#
# Input:      line: "Checking procmap on..." 
#             why_restart: reason for restart, 1=missing, 2=hung
# Output:     Returns 1 on success, 0 on error.
#             Returns the incremented proc_info array.
# 
# Overview:   Search the existing machine_proc_info array for the machine
#             on the current $line and increment the system restart
#             bin as appropriate.
#
#
sub increment_system_array
{
    local ($line, *machine_proc_info) = @_;

    # Initialize

    local($ret_val);
    $num_entries=@machine_proc_info;
    $found_it=0;

    $mach_idx=$Mach_idx - 2;
    $miss_idx=$Miss_idx - 2;
    $hung_idx=$Hung_idx - 2;
    $syst_idx=$hung_idx + 1;

    # Get the machine name from the log line
    $line =~ /Checking procmap on (\w+)\s*(\(\w+\))?/;
    if ( $1 eq "local" or $1 eq "localhost" ) {
       $2 =~ /\((\w+)\)/;
       $machine = $1;
    }
    else {
       $machine = $1;
    }

    # Search through the machine array,
    # if found increment the system restart bin

    for ($i=0; $i<$num_entries; $i++) {
	if ( $machine eq $machine_proc_info[$i][$mach_idx] ) {
	    $found_it=1;
	    $machine_proc_info[$i][$syst_idx] = $machine_proc_info[$i][$syst_idx] + 1;

	    # force drop out of loop

	    last;
	}
    }

    # never found a matching entry, need to increment the array

    if ($found_it == 0) {
	$machine_proc_info[$num_entries][$mach_idx]=$machine;
	$machine_proc_info[$num_entries][$miss_idx]=0;
	$machine_proc_info[$num_entries][$hung_idx]=0;
	$machine_proc_info[$num_entries][$syst_idx]=1;
    }

    return(1);
}
#---------------------------------------------------------------------
# Subroutine: PMP_extract_utime_from_logline()
#
# Usage:      ($return_val, $utime)=extract_utime_from_logline($line, $debug_flag)
#
# Function:   Extract the UNIX time from the procmap_auto_restart.log
#             line. This has a very specific format.
#
# Input:      A procmap_auto_restart log file line to search
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in utime), the parsed UNIX time from the line.
# 
# Overview:   Parse the input line to search for the string: [utime:xxxxx]
#
#
sub PMP_extract_utime_from_logline
{
    local ($line, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_extract_utime_from_logline";
    $ret_val=0;
    $ret_utime=0;

    # Parse the line

    if ($line =~ /\[utime\:/) {
	($junk, $timeplus_str)=split(/\[utime\:/, $line);
	($ret_utime)=split(/\]/, $timeplus_str);
    }
    else {
	print(STDERR "WARNING: $sub_name: Cannot extract a time from the log line\n");
	print(STDERR "\tline:$line\n");
	return($ret_val, $ret_utime);
    }

    # Return

    $ret_val=1;
    return($ret_val, $ret_utime);
}


#---------------------------------------------------------------------
# Subroutine: PMP_line_time_valid()
#
# Usage:      ($return_val, $is_valid)=PMP_line_time_valid($line, $start_utime, 
#                                            $end_utime, $debug_flag)
#
# Function:   Check whether the logfile line falls within the start/end
#             time.
#
# Input:      A procmap_auto_restart log file line to check
#             The start time (UNIX time)
#             The end time (UNIX time)
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in is_valid), 1 if valid or 0 if not
# 
# Overview:   Extract the UNIX time from the line and check whether it
#             falls within the start/end time
#
#
sub PMP_line_time_valid
{
    local ($line, $start_utime, $end_utime, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_line_time_valid";
    $ret_val=0;
    $is_valid=0;

    # Get the UNIX time from the line

    ($is_ok, $line_time)=PMP_extract_utime_from_logline($line, $debug_flag);
    if ($is_ok == 0) {
	print(STDERR "WARNING: $sub_name: Cannot extract utime from line: $line\n");
	return($ret_val, $is_valid);
    }

    # Check if it falls within the start/end times

    if (($line_time >= $start_utime) && ($line_time <= $end_utime)) {
	$is_valid = 1;
    }
   
    if (($is_valid == 0) && ($debug_flag == 1)) {
	print (STDERR "$sub_name: Time: $line_time, is outside start/end times: $start_utime, $end_utime\n");
	print (STDERR "     line: $line\n");
    }

    # Return

    $ret_val=1;
    return($ret_val, $is_valid);
}

#-------------------------------------------------------------------------------
# Subroutine: Parse_line_for_restart()
#
# Usage:      ($return_val, $type, $proc_name, $proc_instance, $proc_host) =
#              Parse_line_for_restart($line, $debug_flag)
#
# Function:   Parse a procmap_auto_restart file line to search for strings
#             indicating the process was restarted.
#
# Input:      A procmap_auto_restart log file line
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in type), 1 if process was restarted because it was missing,
#                   or 2 if the process was restarted because it was hung.
#             Returns (in proc_name), the process name.
#             Returns (in proc_instance), the process instance.
#             Returns (in proc_host), the process host machine name.
#
# Overview:   Parse the line and search for key strings. Split the line
#             to get the process name, instance, and host.
#
#
sub Parse_line_for_restart
{
    local ($line, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="Parse_line_for_restart";
    $ret_val=0;
    $type=-1;
    $proc_name="";
    $instance="";
    $machine="";

    # Parse the line

    if ($line =~ /Missing process/) {

	$type=$Miss_restart;
	$ret_val=1;

	# Find the process that was restarted
	# Parse the line YYYYmmdd hh:mm:ss zzz [utime:uu]: Missing process <proc> <inst> on <host>

	($junk, $junk, $junk, $junk, $junk, $junk, $proc_name, $instance, $junk, $machine) = split(' ', $line);
    }

    if ($line =~ /Restarting process/) {

	$type=$Hung_restart;
	$ret_val=1;

	# Find the process that was restarted
	# Parse: "YYYYmmdd hh:mm:ss zzz [utime:uu]: Restarting process <proc-name> <proc-instance> on <proc-host>"

	($junk, $junk, $junk, $junk, $junk, $junk, $proc_name, $instance, $junk, $machine) = split(' ', $line);
    }

    return($ret_val, $type, $proc_name, $instance, $machine);
}

#---------------------------------------------------------------------
# Subroutine: PMP_get_process_stats
#
# Usage:      ($return_val, $num_sys_restarts, $num_proc_miss_restarts, 
#              $num_proc_hung_restarts) = 
#              PMP_get_process_stats($logfile, $start_utime, $end_utime, 
#              *proc_info, $debug_flag)
#
# Function:   Read the logfile and within the start/end times, count:
#             (1) the number of system restarts, 
#             (2) the number of process missing restarts, 
#             (3) the number of process hung restarts, 
#             (4) statistics on each process that had to be restarted.
#
# Input:      A procmap_auto_restart log file
#             The start time (in UNIX time)
#             The end time (in UNIX time)
#             A proc_info array to fill
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in num_sys_restarts), the number of system restarts.
#             Returns (in num_proc_miss_restarts), the number of process missing
#                   restarts
#             Returns (in num_proc_hung_restarts), the number of process hung
#                   restarts
#             Returns (in proc_info), an array of information about each
#                   process that had to be restarted.
# 
# Overview:   Open the logfile and read through it line by line. If the
#             entry falls within the start/end time, increment each of the 
#             bins as appropriate.
#
#
sub PMP_get_process_stats
{
    local ($logfile, $start_utime, $end_utime, *proc_info, *machine_proc_info, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_get_process_stats";
    $ret_val=0;
    $num_sys_restarts=0;
    $num_proc_miss_restarts=0;
    $num_proc_hung_restarts=0;

    # Open and read the logfile

    if (!open(AUTORESTART_LOGFILE, $logfile)) {
	print(STDERR "ERROR: $sub_name: Cannot open logfile $logfile\n");
	return($ret_val, $num_sys_restarts, $num_proc_miss_restarts, $num_proc_hung_restarts);
    }

    while  ($line = <AUTORESTART_LOGFILE>) {

	# Check for system restarts

	if ($line =~ /Checking procmap/) {

	    # Check that the time of the line falls within the start/end time

	    ($is_ok, $is_valid)=PMP_line_time_valid($line, $start_utime, $end_utime, $debug_flag);

	    if ($is_valid == 1) {
		$num_sys_restarts++;

                $is_ok = increment_system_array($line, *machine_proc_info);
                if ($is_ok == 0) {
                    print(STDERR "ERROR: $sub_name: problem with incrementing system array, line=$line\n");
                    return($ret_val, $num_sys_restarts, $num_proc_miss_restarts, $num_proc_hung_restarts);
                }
	    }
	}

	# Check for process restarts

	($wanted, $type, $proc_name, $instance, $machine)=Parse_line_for_restart($line, $debug_flag);
	if ($wanted == 1) {

	    # Check that the time of the line falls within the start/end time

	    ($is_ok, $is_valid)=PMP_line_time_valid($line, $start_utime, $end_utime, $debug_flag);

	    if ($is_valid == 1) {

		if ($type == $Miss_restart) {
		    $num_proc_miss_restarts++;
		}
		if ($type == $Hung_restart) {
		    $num_proc_hung_restarts++;
		}

		$is_ok = increment_process_array($proc_name, $instance, $machine, $type, *proc_info);
		if ($is_ok == 0) {
		    print(STDERR "ERROR: $sub_name: problem with incrementing process array, proc=$proc_name, instance=$instance\n");
		    return($ret_val, $num_sys_restarts, $num_proc_miss_restarts, $num_proc_hung_restarts);
		}
	    } #endif (is_valid)
	} #endif (wanted)

    } #endwhile

    close (AUTORESTART_LOGFILE);
    
    $ret_val=1;
    return($ret_val, $num_sys_restarts, $num_proc_miss_restarts, $num_proc_hung_restarts);
}


#---------------------------------------------------------------------
# Subroutine: PMP_get_machine_proc_stats
#
# Usage:      $return_val = PMP_get_machine_proc_stats(*proc_info, 
#                           *machine_proc_info, $debug_flag)
#
# Function:   Read the proc_info array and generate statistics on the
#             number of processes that restart per machine.
#
# Input:      A proc_info array to read.
#             A machine_proc_info array to fill.
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in machine_proc_info), an array of machine names and
#                   the number of processes that were restarted on each machine.
# 
# Overview:   Read through the proc_info array and increment bins for each
#             machine. Store the machine names and bins in the machine_proc_info
#             array.
#
sub PMP_get_machine_proc_stats
{
    local (*proc_info, *machine_proc_info, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_get_machine_proc_stats";
    $ret_val=0;
    $num_proc_info=@proc_info;
    $counter=0;

    # Loop through the proc_info array

    for ($i=0; $i<$num_proc_info; $i++) {
	
	$machine=$proc_info[$i][$Mach_idx];
	$num_miss=$proc_info[$i][$Miss_idx];
	$num_hung=$proc_info[$i][$Hung_idx];

	# Loop through the machine_proc_info array to increment the bins

	$found_it=0;
	$mach_idx=$Mach_idx - 2;
	$miss_idx=$Miss_idx - 2;
	$hung_idx=$Hung_idx - 2;

	for ($j=0; $j<$counter; $j++) {
	    if ($machine_proc_info[$j][$mach_idx] eq $machine) {
		
		# Increment bins

		$machine_proc_info[$j][$miss_idx] = $machine_proc_info[$j][$miss_idx] + $num_miss;
		$machine_proc_info[$j][$hung_idx] = $machine_proc_info[$j][$hung_idx] + $num_hung;

		# Force drop out of loop

		$found_it=1;
		last;
	    }
	}

	# Didn't find machine in machine_proc_info array, so add it

	if ($found_it == 0) {
	    $machine_proc_info[$counter][$mach_idx] = $machine;
	    $machine_proc_info[$counter][$miss_idx] = $num_miss;
	    $machine_proc_info[$counter][$hung_idx] = $num_hung;
	    $counter++;
	}
    } #endfor (i=0...)

    $ret_val=1;
    return($ret_val);
}

#---------------------------------------------------------------------
# Subroutine: PMP_get_log_first_last_line()
#
# Usage:      ($return_val, $first_line, $last_line, $nlines)=
#                  PMP_get_log_first_last_line($logfile, $debug_flag)
#
# Function:   Read the logfile and get the first and last lines in the
#             file that contain timestamps.
#
# Input:      A procmap_auto_restart log file
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in first_line), the first line of the logfile with a
#                  timestamp in it.
#             Returns (in last_line), the last line of the logfile with a
#                  timestamp in it.
#             Returns (in nlines), the number of lines in the file.
# 
# Overview:   Open the logfile and read through it line by line. Save
#             the first and last lines with timestamps in them. The
#             timestamps are defined by xx:xx:xx strings.
#
#
sub PMP_get_log_first_last_line
{
    local ($logfile, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_get_log_first_last_line";
    $ret_val=0;
    $first_line="(null)";
    $last_line="(null)";
    $nlines=-1;

    # Open and read the logfile

    if (!open(AUTORESTART_LOGFILE, $logfile)) {
	print(STDERR "ERROR: $sub_name: Cannot open logfile $logfile\n");
	return($ret_val, $first_line, $last_line, $nlines);
    }

    # Find the first and last lines with time entries in them

    $nlines=0;
    while  ($line = <AUTORESTART_LOGFILE>) {

	# Increment the number of lines in the file

	$nlines++;

	# Search for the start time string

	if ($line =~ /\d\d\:\d\d\:\d\d/) {
	    if ($first_line eq "(null)") {
		$first_line = $line;
		$ret_val = 1;
	    }
	    $last_line=$line;
	}

    } #endwhile
    
    close(AUTORESTART_LOGFILE);
    
    # Return

    return($ret_val, $first_line, $last_line, $nlines);
}


#---------------------------------------------------------------------
# Subroutine: PMP_get_datestrings()
#
# Usage:      ($return_val, $datestring, $unixdate, $expand_date) =
#                  PMP_get_datestrings($tz_flag, $debug_flag)
#
# Function:   Get the current time and return it in a variety of
#             string formats for use in the procmap_auto_restart.log.
#             Returns the LOCAL time.
#
# Input:      A timezone flag (1=local, 2=gmt) -- ignored
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in datestring), the date in YYYYmmdd hh:mm:ss zzz [unixtime]
#             Returns (in unixdate), the date in UNIX time format
#             Returns (in expand_date), the date in human-readable format,
#                   "Mon Nov 3 1995 hh:mm:ss zzz"
# 
# Overview:   Get the current time using the 'date' command. Create
#             a concatenated string with the various dates.
#
#
sub PMP_get_datestrings
{	
    local ($tz_flag, $debug_flag) = @_;

    # Initialize
	
    local($ret_val, $sub_name);
    $ret_val=0;
    $sub_name="PMP_get_datestrings";
    $datestring="(null)";
    $dateutime="(null)";
    $datetime="(null)";

    # Error checks

    if (($tz_flag != $Use_local) && ($tz_flag != $Use_GMT)) {
	print(STDERR "ERROR: $sub_name: invalid timezone flag\n");
	return($ret_val, $datestring, $dateutime, $datetime);
    }

    #
    # Save the current date and time for messages. Need both
    # UNIX time and human-readable time.
    #

    if ($tz_flag == $Use_local) {
	(@timearr)=localtime;
	$dateutime=&timelocal(@timearr);
	$datetime = `date`;
	$dateshort = `date '+%Y%m%d %X %Z'`;
    }
    if ($tz_flag == $Use_GMT) {
	(@timearr)=gmtime;
	$dateutime=&timegm(@timearr);
	$datetime = `date -u`;
	$dateshort = `date -u '+%Y%m%d %X %Z'`;
    }

    chop($dateshort);
    chop($datetime);

    # Build a time string for use at the beginning of each line in the log
    # need a format with both short human-readable and UNIX time

    $datestring = $dateshort . " [utime:" . $dateutime . "]";

    # Return

    $ret_val=1;
    return($ret_val, $datestring, $dateutime, $datetime);
}


#---------------------------------------------------------------------
# Subroutine: PMP_get_process_restart_time()
#
# Usage:      ($return_val, $time_impaired) =
#                  PMP_get_process_restart_time($proc_name, $instance, $machine, 
#                                                $procmap_host, $restart_sleep_secs,
#                                                $debug_flag)
#
# Function:   Find the length of time a process takes to restart, based 
#             on the MAXINT returned from 'print_procmap' and the restart_sleep_secs
#             passed in (should match the startup of procmap_auto_restart).
#
# Input:      The process name.
#             The process instance.
#             The process host machine.
#             The PROCMAP_HOST to run 'print_procmap -maxint' on.
#             The number of sleep seconds used for procmap_auto_restart.
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in time_impaired), the time the process takes to restart
#                    in seconds.
# 
# Overview:   Search the print_procmap output file for a match with the 
#             process/instance/machine and retrieve the MAXINT. 
#             Calculate the process "down" or "restart" time as 
#             a function of the MAXINT and restart_sleep_secs. If a match
#             is not found, return a default.
#
# NOTE:       The calculation used for "process restart time" or "process
#             down time" is: (MAXINT + restart_sleep_secs)/2. This is an
#             average of the worst case situation: where a process goes down 
#             just AFTER the procmap_auto_restart has checked it; it will take 
#             the procmap_auto_restart restart_sleep_secs + the maxint 
#             for the process to be detected as "down" and needing to be 
#             restarted.
#
#
sub PMP_get_process_restart_time
{	
    local ($proc_name, $instance, $machine, $procmap_host, $restart_sleep_secs, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_get_process_restart_time";
    $ret_val=0;
    $time_impaired=$Ave_maxint;     # average maxint

    # Weirdness with print_procmap output, truncates to 12 chars in a field
    # for process name, 12 for process instance, 8 for machine(?)

    $short_proc_name=substr($proc_name,0,11);
    $short_instance=substr($instance,0,11);
    $short_machine=substr($machine,0,8);

    # Run print_procmap

    unless (open(PROCMAP, "print_procmap -maxint -host $procmap_host |"))
    {
	print(STDERR "ERROR: $sub_name: Cannot run print_procmap on $procmap_host\n");
	return($ret_val, $time_impaired);
    } # endunless - open print_procmap command

    # Read the output searching for a match with the process/instance/host.
    # If found, then get the MAXINT entry.
    # Calculate the process "down" or "restart" time (see NOTE above).

    $found=0;
    while  ($line = <PROCMAP>) {

	($p_proc, $p_inst, $p_machine, $p_user, $p_pid, $p_maxint)=split(' ', $line);
	
	$short_p_machine=substr($p_machine,0,8);

	if (($p_proc eq $short_proc_name) && ($p_inst eq $short_instance) && ($short_p_machine eq $short_machine)) {
	    if ($debug_flag == 1) {
		print(STDERR "Found a print_procmap entry match for $short_proc_name $short_instance $short_machine\n");
	    }

	    # Calculate the "impaired" time

	    $time_impaired=($p_maxint + $restart_sleep_secs) / 2.0 ;
	    $found=1;
	}

    } #endwhile
    
    close(PROCMAP);

    # Was a match ever found?

    if ($found == 0) {
	print(STDERR "WARNING: $sub_name: A match was never found in print_procmap for $proc_name $instance $machine\n");
	return($ret_val, $time_impaired);
    }
    
    # Return

    $ret_val=1;
    return($ret_val, $time_impaired);
}

#---------------------------------------------------------------------
# Subroutine: PMP_get_pct_time_impaired()
#
# Usage:      ($return_val, $pct_impaired) =
#                  PMP_get_pct_time_impaired($start_time, $end_time, $down_time,
#                                            $debug_flag)
#
# Function:   Calculate the length of downtime as a percentage of a given 
#             length of time.
#
# Input:      The start time for evaluation (in UNIX time)
#             The end time for evaluation (in UNIX time)
#             The time (in seconds) that the system was impaired
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in pct_impaired), the percentage of time that the
#                    system was impaired.
# 
# Overview:   Based on the start and end time, calculate the amount of time the
#             system was "down".
#
sub PMP_get_pct_time_impaired
{	
    local ($start_time, $end_time, $down_time, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_get_pct_time_impaired";
    $ret_val=0;
    $pct_impaired=-1;

    # Calculate down time percentage

    $timespan=$end_time - $start_time;

    if ($timespan <0) {
	print(STDERR "ERROR: $sub_name: Invalid start and end times, timespan is <0\n");
	return($ret_val, $pct_impaired);
    }

    # Calculate the % time the system was impaired.
    #
    # Handle case where downtime > timespan. This can happen since downtime includes
    # the restart time and the specified endtime may be less than the restart time
    # so throws the calculation off.

    if ($down_time >= $timespan) {
	$pct_impaired=100.0;
    }
    else {
	$pct_impaired=($down_time/$timespan) * 100.0;
    }

    if (($pct_impaired >100) || ($pct_impaired < 0)) {
	print(STDERR "ERROR: $sub_name: Cannot calculate percent downtime, >100 $pct_impaired <0\n");
	return($ret_val, $pct_impaired);
    }

    # Return

    $ret_val=1;
    return($ret_val, $pct_impaired);
}


#---------------------------------------------------------------------
# Subroutine: PMP_conv_utime_to_human()
#
# Usage:      ($return_val, $timestring) =
#                  PMP_conv_utime_to_human($intime, $tzflag, $debug_flag)
#
# Function:   Convert the input UNIX time to a human-readable
#             string.
#
# Input:      A UNIX time.
#             The timezone flag (1=local, 2=GMT).
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in timestring), a string with the human readable
#                   time.
# 
# Overview:   Use localtime() or gmtime() to convert the date. Build
#             a string and return it.
#
#
sub PMP_conv_utime_to_human
{	
    local ($intime, $tz_flag, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_conv_utime_to_human";
    $ret_val=0;

    # Error checks

    if (($tz_flag != $Use_local) && ($tz_flag != $Use_GMT)) {
	print(STDERR "ERROR: $sub_name: invalid timezone flag\n");
	return($ret_val, $datestring, $dateutime, $datetime);
    }

    # Convert the times

    if ($tz_flag == $Use_local) {
	($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst)=localtime($intime);
    }
    if ($tz_flag == $Use_GMT) {
	($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst)=gmtime($intime);
    }

    # Build a time string 

    # Increment month as it is returned zero-based, not one-based
    # and make sure that all strings are padded with zeros if needed

    $mon++;
    if ($mon <= 9) {
	$mon="0" . $mon;
    }

    if ($mday <= 9) {
	$mday="0" . $mday;
    }

    if ($hour <= 9) {
	$hour="0" . $hour;
    }

    if ($min <= 9) {
	$min="0" . $min;
    }

    if ($sec <= 9) {
	$sec="0" . $sec;
    }

    # Kludgey Year 2000 handling

    #if ($year < 98) {
    #   $year="20" . $year;
    #}
    #else {
    #   $year="19" . $year;
    #}

    #
    # localtime and gmtime return the year since 1900, which
    # means that they return 100 for the year 2000.  This means
    # that the following should work for the year 2000 and beyond
    #
    $year = 1900 + $year;

    $timestring = $year . $mon . $mday . " " . $hour . ":" . $min . ":" . $sec;

    # Return

    $ret_val=1;
    return($ret_val, $timestring);
}


#---------------------------------------------------------------------
# Subroutine: PMP_get_sys_time_gaps()
#
# Usage:      ($return_val, $total_down_time) =
#                  PMP_get_sys_time_gaps($logfile, $procmap_host, $start_time,
#                                 $end_time,
#                                 *arr_gaps, *arr_proc_gaps, $restart_sleep_secs,
#                                 $debug_flag)
#
# Function:   Read the procmap_auto_restart.log to parse out the
#             number of times and length of time that the system was
#             impaired (a process was restarted). Return the total
#             down time and details on each time gap.
#
# Input:      A procmap_auto_restart logfile.
#             The PROCMAP_HOST to run 'print_procmap' on.
#             The logfile start time.
#             The logfile end time.
#             An array (arr_gaps) which will be returned.
#             An array (arr_proc_gaps) which will be returned.
#             The number of sleep seconds used for procmap_auto_restart.
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in total_down_time), the total number of seconds that
#                  the system was "down/impaired" from the logfile.
#             Returns (in arr_gaps), an array of time gaps (time the system
#                  was down/impaired) including start and end times.
#                  $arr_gaps[gap_number][start_time]
#                  $arr_gaps[gap_number][end_time]
#             Returns (in arr_proc_gaps), an array of process time gaps
#                  (time the process was restarted) including start and end
#                  times for each process down time.
#                  $arr_proc_gaps[process_gap_number][start_time]
#                  $arr_proc_gaps[process_gap_number][end_time]
#                  $arr_proc_gaps[process_gap_number][gap_number]
#                  $arr_proc_gaps[process_gap_number][process]
#                  $arr_proc_gaps[process_gap_number][instance]
#                  $arr_proc_gaps[process_gap_number][machine]
# 
# Overview:   Opens and reads the logfile. Parse the logfile line by line
#             looking for process restarts. Get the process "restart time"
#             (or "impaired time"). Build arrays of system time gaps and
#             individual time gaps. Total the system time gaps.
#
#
sub PMP_get_sys_time_gaps
{	
    local ($logfile, $procmap_host, $start_utime, $end_utime, *arr_gaps, *arr_proc_gaps, $restart_sleep_secs, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_get_sys_time_gaps";
    $ret_val=0;
    $total_down_time=-1;
    $arr_gaps[0][0]=-1;
    $arr_proc_gaps[0][0]=-1;
    $proc_gap_counter=0;
    $gap_counter=0;
    $current_gap_start=0;
    $current_gap_end=0;
    $first_time_sys=0;
    $first_time_proc=0;

    # Open and read the logfile

    if (!open(AUTORESTART_LOGFILE, $logfile)) {
	print(STDERR "ERROR: $sub_name: Cannot open logfile $logfile\n");
	return($ret_val, $total_down_time);
    }

    while  ($line = <AUTORESTART_LOGFILE>) {

	# Check for process restarts. If find a line with a restart
	# in it, then process the line looking for the time the 
	# process is "down". This is defined in PMP_get_process_restart_time()

	($wanted, $type, $proc_name, $instance, $machine)=Parse_line_for_restart($line, $debug_flag);

	# Check that the time of the line falls within the start/end time

	($is_ok, $is_valid)=PMP_line_time_valid($line, $start_utime, $end_utime, $debug_flag);

	if (($is_valid == 1) && ($wanted == 1)) {

	    # Get the time the process began restarting

	    ($is_ok, $proc_restart_time)=PMP_extract_utime_from_logline($line, $debug_flag);
	    
	    if ($debug_flag) {
		print(STDERR "$sub_name: proc: $proc_name, inst: $instance, host: $machine, restart_time: $proc_restart_time\n");
	    }

	    # Get the number of seconds the process takes to restart

	    ($is_ok, $time_impaired)=PMP_get_process_restart_time($proc_name, $instance, $machine, $procmap_host, $restart_sleep_secs, $debug_flag);

	    if ($debug_flag) {
		print(STDERR "    impaired time: $time_impaired\n");
	    }
		    
	    # Get the time the process will be done restarting

	    $proc_done_restart=$proc_restart_time + $time_impaired;

	    if ($debug_flag) {
		print(STDERR "    time process done restarting: $proc_done_restart\n");
		print(STDERR "    current system gap start: $current_gap_start, end: $current_gap_end\n");
	    }

	    # Build arrays of time gaps for the SYSTEM

	    # ... first time through ...

	    if ($first_time_sys == 0) {

		if ($debug_flag) {
		    print(STDERR "\tFirst time thru, create system gap array\n");
		}

		$current_gap_start=$proc_restart_time;
		$current_gap_end=$proc_done_restart;

		$arr_gaps[$gap_counter][$Gap_start]=$current_gap_start;
		$arr_gaps[$gap_counter][$Gap_end]=$current_gap_end;

		$first_time_sys=1;
	    }

	    # ... restart occurred within current gap ...

	    elsif (($proc_restart_time > $current_gap_start) && 
		   ($proc_restart_time <= $current_gap_end)) {
		$current_gap_end=$proc_done_restart;
		$arr_gaps[$gap_counter][$Gap_end]=$current_gap_end;

		if ($debug_flag) {
		    print(STDERR "\tRestart occurred within current SYSTEM gap, move gap end to $current_gap_end\n");
		}
	    }

	    # ... restart occurred outside current gap, NEW GAP ...

	    elsif ($proc_restart_time > $current_gap_end) {
		$gap_counter++;
		$current_gap_start=$proc_restart_time;
		$current_gap_end=$proc_done_restart;

		$arr_gaps[$gap_counter][$Gap_start]=$current_gap_start;
		$arr_gaps[$gap_counter][$Gap_end]=$current_gap_end;

		if ($debug_flag) {
		    print(STDERR "\tRestart occurred outside current SYSTEM gap, new gap $gap_counter, start:$current_gap_start, end:$current_gap_end\n");
		}
	    }    
	    
	    # ... skip it, likely the same time span as the current gap ...
	    else {
		if ($debug_flag) {
		    print(STDERR "\tMatch with current system gap, skip it\n");
		}
	    }

	    # Build arrays of time gaps by PROCESS

	    # ... first time through ...

	    if ($first_time_proc == 0) {
		if ($debug_flag) {
		    print(STDERR "\tFirst time thru, create process gap array\n");
		}
		$arr_proc_gaps[$proc_gap_counter][$Gap_start]=$proc_restart_time;
		$arr_proc_gaps[$proc_gap_counter][$Gap_end]=$proc_done_restart;
		$arr_proc_gaps[$proc_gap_counter][$Gap_number]=$gap_counter;
		$arr_proc_gaps[$proc_gap_counter][$Gap_proc_name]=$proc_name;
		$arr_proc_gaps[$proc_gap_counter][$Gap_proc_instance]=$instance;
		$arr_proc_gaps[$proc_gap_counter][$Gap_proc_machine]=$machine;

		$proc_gap_counter++;

		$first_time_proc=1;
	    }

	    # ... restart occurred ...

	    else {

		# ... Did restart occur within a current PROCESS time gap ...

		$found_it=0;
		for ($i=0; $i < $proc_gap_counter; $i++) {

		    # Find the current gap entry in the array for this process/instance

		    if (($arr_proc_gaps[$i][$Gap_proc_name] eq $proc_name) &&
			($arr_proc_gaps[$i][$Gap_proc_instance] eq $instance) &&
			($arr_proc_gaps[$i][$Gap_proc_machine] eq $machine) &&
			($arr_proc_gaps[$i][$Gap_number] == $gap_counter)) {

			# Check the time
			
			if (($proc_restart_time > $arr_proc_gaps[$i][$Gap_start] ) &&
			    ($proc_restart_time <= $arr_proc_gaps[$i][$Gap_end] )) {
			    
			    $found_it=1;
			    $arr_proc_gaps[$i][$Gap_end]=$proc_done_restart;
			    
			    if ($debug_flag) {
				print(STDERR "\tRestart occurred within an existing PROCESS gap entry $i, move gap end to $proc_done_restart\n");
			    }
			}

			# Force drop out of loop
			if ($found_it == 1) {
			    $i=$proc_gap_counter;
			}
		    }
		} #endfor

		# ... Restart occurred outside the current PROCESS time gap...

		if ($found_it == 0) {
		    if ($debug_flag) {
			print(STDERR "\tRestart occurred outside current PROCESS gap, new process gap, start:$proc_restart_time, end:$proc_done_restart\n");
		    }
		    $arr_proc_gaps[$proc_gap_counter][$Gap_start]=$proc_restart_time;
		    $arr_proc_gaps[$proc_gap_counter][$Gap_end]=$proc_done_restart;
		    $arr_proc_gaps[$proc_gap_counter][$Gap_number]=$gap_counter;
		    $arr_proc_gaps[$proc_gap_counter][$Gap_proc_name]=$proc_name;
		    $arr_proc_gaps[$proc_gap_counter][$Gap_proc_instance]=$instance;
		    $arr_proc_gaps[$proc_gap_counter][$Gap_proc_machine]=$machine;
		    $proc_gap_counter++;
		}
	    }  #endelse (first_time_proc == 0)
	} #endif (wanted & valid)

    } #endwhile

    close (AUTORESTART_LOGFILE);

    # Total the overall downtime

    $total_down_time=0;
    for ($i=0; $i <= $gap_counter; $i++) {
	$gap=$arr_gaps[$i][$Gap_end] - $arr_gaps[$i][$Gap_start];
	$total_down_time=$total_down_time + $gap;
    }

    # Return

    $ret_val=1;
    return($ret_val, $total_down_time);
}



#---------------------------------------------------------------------
# Subroutine: PMP_get_category_time_gaps()
#
# Usage:      $return_val=PMP_get_category_time_gaps($proc_list_file, 
#                         $category_list_file,
#                         $arr_proc_gaps, *arr_priorities_downtime,
#                         $debug_flag)
#
# Function:   Read the proc_list file to parse out the priority
#             of each process/instance/hostname entry. Match each 
#             system impaired time entry in $arr_proc_gaps with the
#             priority and total up the impaired time for each
#             priority. Return the total time down for each 
#             priority category.
#
# Input:      A process list file.
#             A process category list file.
#             The array of process time gaps.
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in arr_priority_downtime), a set of bins with
#                  total time processes in that category were "down"
#                  $arr_priorities_downtime[i][priority]
#                  $arr_priorities_downtime[i][description]
#                  $arr_priorities_downtime[i][timespan]
#                  $arr_priorities_downtime[i][start]
#                  $arr_priorities_downtime[i][end]
#                  $arr_priorities_downtime[i][weight]
# 
# Overview:   Opens and reads the category_list_file to build an
#             array to hold the bins for downtime by priority.
#             Opens and reads the proc_list file line by line. Parse
#             out the priority entry and save to an array. Go through
#             the arr_proc_gaps array and match with the priority
#             array building up bins by priority category.
#
sub PMP_get_category_time_gaps
{	
    local ($proc_list_file, $category_list_file, *arr_proc_gaps, *arr_priorities_downtime, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name, $category);
    $sub_name="PMP_get_category_time_gaps";
    $ret_val=0;
    $priority_idx=$Mach_idx+1;
    $no_downtime=0;

    # Open the priority list file, parse it, and build an array

    if (!open(CATEGORY_LIST, $category_list_file)) {
	print(STDERR "ERROR: $sub_name: Cannot process category list file $category_list_file\n");
	return($ret_val);
    }

    $counter=0;
    while  ($line = <CATEGORY_LIST>) {

	# Skip comments and blank lines

	$line_to_read = PMP_check_entry_line($line);
  
	if ($line_to_read) {

	    ($category, $description, $junk)=split(' ',$line);

	    $arr_priorities_downtime[$counter][$Pri_idx]=$category;
	    $arr_priorities_downtime[$counter][$Pri_descrip_idx]=$description;
	    $arr_priorities_downtime[$counter][$Pri_timespan_idx]=$no_downtime;
	    $arr_priorities_downtime[$counter][$Pri_start_idx]=$no_downtime;
	    $arr_priorities_downtime[$counter][$Pri_end_idx]=$no_downtime;
	    $arr_priorities_downtime[$counter][$Pri_num_procs]=0;
	    $arr_priorities_downtime[$counter][$Pri_pct_impaired]=0.0;
	    
	    if ($debug_flag) {
		print(STDERR "Process categories array i:$counter, pri:$arr_priorities_downtime[$counter][$Pri_idx], desc:$arr_priorities_downtime[$counter][$Pri_descrip_idx]\n");
	    }

	    $counter++;
	}
    } #endwhile

    # Close the category list file
    
    close(CATEGORY_LIST);

    # Number of categories

    $num_priorities_downtime = @arr_priorities_downtime;

    # Open the process list file and get the array of expanded entry lines

    ($is_ok, $num_entry_lines) = PMP_read_proc_list_file($proc_list_file, *entries, *started, (null));

    # Error return

    if ($is_ok == 0) {
	return($ret_val);
    }

    # Read through the process list file entry lines to build a lookup array with 
    # process/instance/host and priority for each entry. Also total up the
    # number of processes in each process category.

    $counter=0;
    foreach $entry (@entries) {
	
	# Go through the entry lines and retrieve the process, instance,
	# host machine, and priority for each process to build an lookup table

	($is_ok, $procname, $instance, $start_script, $kill_script, $host, $priority)=&PMP_get_entry_all($entry);
	if ($is_ok == 0) {
	    print("ERROR: $sub_name: Problem with parsing the entry line $entry\n");
	    return($ret_val);
	}

	# Priority is NOT required to be in the proc_list file for any other
	# operation besides statistics... so we need to do an error check here

	if ($priority eq "(null)") {
	    print(STDERR "ERROR: $sub_name: No priority for proc list file entry\n");
	    print(STDERR "\t$entry\n");
	    return($ret_val);
	}

	$arr_priorities[$counter][$Proc_idx]=$procname;
	$arr_priorities[$counter][$Inst_idx]=$instance;
	$arr_priorities[$counter][$Mach_idx]=$host;
	$arr_priorities[$counter][$priority_idx]=$priority;

	# Increment the number of processes per category

	for ($k=0; $k < $num_priorities_downtime; $k++) {

	    $k_priority=$arr_priorities_downtime[$k][$Pri_idx];
	    $k_num_procs=$arr_priorities_downtime[$k][$Pri_num_procs];

	    if ($k_priority eq $priority) {
		$k_num_procs++;
		$arr_priorities_downtime[$k][$Pri_num_procs]=$k_num_procs;
	    }
	}

	$counter++;

    } #endforeach

    # Go through the arr_proc_gaps array and match up the priorities, create
    # bins for each priority to hold the total down-time for that category

    $num_gaps = @arr_proc_gaps;
    $num_priorities = @arr_priorities;
    $num_priorities_downtime = @arr_priorities_downtime;
    $found_priority=0;

    for ($i=0; $i<$num_gaps; $i++) {

	# Looking for a match for priority entry in each gap proc/instance/host

	$found_priority=0;

	# Easier to handle short variable names

	$gap_procname=$arr_proc_gaps[$i][$Gap_proc_name];
	$gap_instance=$arr_proc_gaps[$i][$Gap_proc_instance];
	$gap_machine=$arr_proc_gaps[$i][$Gap_proc_machine];
	$gap_start=$arr_proc_gaps[$i][$Gap_start];
	$gap_end=$arr_proc_gaps[$i][$Gap_end];
	$timespan=$gap_end - $gap_start;

	for ($j=0; $j<$num_priorities; $j++) {

	    # Look for a match with the priority lookup table array

	    if (($arr_priorities[$j][$Proc_idx] eq $gap_procname) &&
		($arr_priorities[$j][$Inst_idx] eq $gap_instance) &&
		($arr_priorities[$j][$Mach_idx] eq $gap_machine)) {
		
		# Found a match with the proc/instance/host in the priorities array

		$found_priority=1;
		$priority=$arr_priorities[$j][$priority_idx];

		if ($debug_flag) {
		    print(STDERR "Found a priority entry match for $gap_procname $gap_instance $gap_machine category: $priority start:$gap_start, end:$gap_end\n");
		}

		# Increment the bins

		for ($k=0; $k <= $num_priorities_downtime; $k++) {

		    # Shorthand

		    $k_priority=$arr_priorities_downtime[$k][$Pri_idx];
		    $k_timespan=$arr_priorities_downtime[$k][$Pri_timespan_idx];
		    $k_gap_start=$arr_priorities_downtime[$k][$Pri_start_idx];
		    $k_gap_end=$arr_priorities_downtime[$k][$Pri_end_idx];

		    if ($k_priority eq $priority) {

			# Start time of gap is after the end of the last gap 
			# we counted, this is a new gap so add the full downtime

			if ($gap_start > $k_gap_end) {
			    $arr_priorities_downtime[$k][$Pri_timespan_idx] = $k_timespan + $timespan;
			    $arr_priorities_downtime[$k][$Pri_start_idx] = $gap_start;
			    $arr_priorities_downtime[$k][$Pri_end_idx] = $gap_end;
			    if ($debug_flag) {
				print(STDERR "\tNew gap, start: $gap_start, end: $gap_end, add: $timespan secs\n");
			    }
			}
			
			# End time is before last gap we counted's end time,
			# don't include this gap as we've already got it
			    
			elsif ($gap_end <= $k_gap_end) {
			    if ($debug_flag) {
				print(STDERR "\tGap falls within current timespan, skip it\n");
			    }
			}

			# Some overlap between last gap and this gap, so we
			# only want to add in the additional down time not the
			# whole gap time

			elsif (($gap_start >= $k_gap_start) && ($gap_end > $k_gap_end)) {
			    $time_to_add=$gap_end - $k_gap_end;
			    $arr_priorities_downtime[$k][$Pri_timespan_idx] = $k_timespan + $time_to_add;
			    $arr_priorities_downtime[$k][$Pri_end_idx] = $gap_end;
			    if ($debug_flag) {
				print(STDERR "\tFound overlap, increase gap by $time_to_add secs, new end: $gap_end, total span:$arr_priorities_downtime[$k][$Pri_end_idx]\n");
			    }
			}
		    } #endif search for priority
		} #endfor

	    } #endif (match proc/instance/machine with priorities array)

	} #endfor (j=0)

	if ($found_priority == 0) {
	    print(STDERR "WARNING: $sub_name: No match found in process list file $proc_list_file\n");
	    print(STDERR "\tfor $gap_procname $gap_instance $gap_machine. Cannot calculate down-time by priority.\n");
	    return($ret_val);
	}
    } #endfor (i=0)

    $ret_val = 1;
    
    # Return

    return($ret_val);
}


#---------------------------------------------------------------------
# Subroutine: PMP_get_cat_impaired_pct()
#
# Usage:      $return_val = PMP_get_cat_impaired_pct($start_time, $end_time,
#                           *arr_priorities_downtime, $debug_flag)
#
# Function:   Use PMP_calc_impaired_pct() to get the percentage of downtime
#             for each process category.
#
# Input:      The logfile start time.
#             The logfile end time.
#             An array (arr_priorities_downtime) which contains the details
#                   on the category time gaps and number of procs. This is
#                   constructed in PMP_get_category_time_gaps().
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in *arr_priorities_downtime), the percentage of time
#                   each category was "down/impaired", in
#                   $arr_priorities_downtime[$i][$Pri_pct_impaired]
# 
# Overview:   Go through arr_priorities_downtime and for each category, use
#             to PMP_calc_impaired_pct() to get the impaired percent.
#
sub PMP_get_cat_impaired_pct
{	
    local ($start_utime, $end_utime, *arr_priorities_downtime, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_get_cat_impaired_pct";
    $ret_val=-99;

    # Go through the input array and get the impaired percent for each category

    $num_cats = @arr_priorities_downtime;

    for ($i=0; $i<$num_cats-1; $i++) {

	if ($debug_flag) {
	    printf(STDERR "Categories array i:$i, cat:$arr_priorities_downtime[$i][$Pri_idx], downsecs:$arr_priorities_downtime[$i][$Pri_timespan_idx], nprocs:$arr_priorities_downtime[$i][$Pri_num_procs]\n");
	}

	($is_ok, $pct_impaired)=PMP_calc_impaired_pct($start_utime, $end_utime, $arr_priorities_downtime[$i][$Pri_timespan_idx], $arr_priorities_downtime[$i][$Pri_num_procs], $debug_flag);
	$arr_priorities_downtime[$i][$Pri_pct_impaired]=$pct_impaired;

	if ($is_ok < 1) {
	    $ret_val = $is_ok;
	}
    }

    if ($ret_val == -99) {
	$ret_val = 1;
    }

    return($ret_val);
}



#---------------------------------------------------------------------
# Subroutine: PMP_calc_impaired_pct()
#
# Usage:      ($return_val, $total_pct_impaired) =
#                  PMP_calc_impaired_pct($start_time, $end_time, 
#                  $total_process_down_time, $num_procs, $debug_flag)
#
# Function:   Calculate the percentage of time that the system was
#             impaired. This is based on the number of processes in
#             the system, the total process seconds and the total
#             process down seconds. Returns the percentage of time
#             that the system was "impaired".
#
# Input:      The logfile start time.
#             The logfile end time.
#             The total process down time (in seconds).
#             The number of processes in the system.
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in total_pct_impaired), the percentage of time the
#                  the system was "down/impaired".
# 
# Overview:   Multiply the system timespan (end-start) by the number
#             of processes in the system to get the total system process
#             seconds. Divide the total process down seconds by the
#             total system process seconds to come up with the "down
#             percent".
#
sub PMP_calc_impaired_pct
{	
    local ($start_utime, $end_utime, $total_proc_downsecs, $num_procs, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_calc_impaired_pct";
    $ret_val=0;
    $total_pct_impaired=0.0;

    # Total system process seconds

    $timespan=$end_utime - $start_utime;
    if ($timespan <0) {
	print(STDERR "ERROR: $sub_name: Invalid start and end times, timespan is <0\n");
	return($ret_val, $total_pct_impaired);
    }

    $total_sys_proc_secs=$num_procs * $timespan;

    if ($debug_flag) {
	print(STDERR "num_procs: $num_procs, timespan: $timespan, proc_secs: $total_sys_proc_secs\n");
    }

    # Calculate the % time the system was impaired.

    if ($total_proc_downsecs >= $total_sys_proc_secs) {
	$total_pct_impaired=100.0;
    }
    else {
	$total_pct_impaired=($total_proc_downsecs/$total_sys_proc_secs) * 100.0;
    }

    if ($debug_flag) {
	print(STDERR "total_proc_downsecs: $total_proc_downsecs, total_sys_secs:$total_sys_proc_secs, total_pct_impaired: $total_pct_impaired\n");
    }

    if (($total_pct_impaired >100) || ($total_pct_impaired < 0)) {
	print(STDERR "ERROR: $sub_name: Cannot calculate percent downtime, >100 $total_pct_impaired <0\n");
	return($ret_val, $total_pct_impaired);
    }

    # Return
    
    $ret_val=1;
    return($ret_val, $total_pct_impaired);
}



#---------------------------------------------------------------------
# Subroutine: PMP_get_sys_impaired_pct()
#
# Usage:      ($return_val, $total_pct_impaired) =
#                  PMP_get_sys_impaired_pct($start_time, $end_time,
#                                 *arr_process_gaps, $num_procs, $debug_flag)
#
# Function:   Uses PMP_calc_impaired_pct() to calculate the total system
#             downtime based on the logfile start/end time and the
#             array of process time gaps (downtime).
#
# Input:      The logfile start time.
#             The logfile end time.
#             An array (arr_process_gaps) which contains the details
#                   on time gaps for each process.
#                   $arr_process_gaps[$i][$Gap_start]
#                   $arr_process_gaps[$i][$Gap_end]
#             The number of processes in the system.
# Output:     Returns (in return_val), 1 if successful or 0 on error.
#             Returns (in total_pct_impaired), the percentage of time the
#                  the system was "down/impaired".
# 
# Overview:   Go through arr_process_gaps and total up the process down
#             seconds (this is NOT the same as the system time gaps).
#             Uses PMP_calc_impaired_pct to get the impaired pct to return.
#
sub PMP_get_sys_impaired_pct
{	
    local ($start_utime, $end_utime, *arr_process_gaps, $num_procs, $debug_flag) = @_;

    # Initialize

    local($ret_val, $sub_name);
    $sub_name="PMP_get_sys_impaired_pct";
    $ret_val=0;

    $sys_impaired_pct=0.0;

    # Go through the array to find the total process down time

    $num_arr=@arr_process_gaps;

    for($i=0; $i<$num_arr; $i++) {
	$down_secs=$arr_process_gaps[$i][$Gap_end] - $arr_process_gaps[$i][$Gap_start];
	$total_proc_downsecs=$total_proc_downsecs + $down_secs;
    }

    # Get impaired percent

    ($ret_val, $sys_impaired_pct)=PMP_calc_impaired_pct($start_utime, $end_utime, $total_proc_downsecs, $num_procs, $debug_flag);

    # Return
    
    return($ret_val, $sys_impaired_pct);
}

#---------------------------- End of Subroutines ---------------------------

