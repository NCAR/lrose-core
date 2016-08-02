# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
# ** Copyright UCAR (c) 1992 - 2010 */
# ** University Corporation for Atmospheric Research(UCAR) */
# ** National Center for Atmospheric Research(NCAR) */
# ** Research Applications Laboratory(RAL) */
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
# ** 2010/10/7 23:12:43 */
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
package    Toolsa;
require    Exporter;
require 5.002;
use POSIX 'sys_wait_h';
@ISA = qw(Exporter);
@EXPORT = qw(LDATA_init_handle LDATA_info_print LDATA_info_write LDATA_info_read PMU_auto_init PMU_auto_register PMU_force_register PMU_auto_unregister safe_system safe_system_sh);

#
# This module contains PERL routines that mirror the toolsa library
# routines.
#
# Nancy Rehak, RAP, NCAR, Boulder, CO, USA March 1998
#

###################################################################
# LDATA routines
###################################################################

###################################################################
#
# LDATA_init_handle(prog_name, debug)
#
# Initialize the handle.
#
# Inputs:
#
#     prog_name: program name
#
#     debug: flag, set to TRUE if you want debug printout
#
# Returns:
#
#     handle:  value to be used as handle in later LDATA calls
#

sub LDATA_init_handle
{
    local($prog_name, $debug) = @_;

    if (defined $LDATA_handles_used)
    {
        $handle = $LDATA_handles_used;
        $LDATA_handles_used++;
    }
    else
    {
        $handle = 0;
        $LDATA_handles_used = 1;
    }

    $LDATA_ltime_unix_time[$handle] = 0;
    $LDATA_ltime_year[$handle] = 0;
    $LDATA_ltime_month[$handle] = 0;
    $LDATA_ltime_day[$handle] = 0;
    $LDATA_ltime_hour[$handle] = 0;
    $LDATA_ltime_minute[$handle] = 0;
    $LDATA_ltime_second[$handle] = 0;
    $LDATA_init_flag[$handle] = true;
    $LDATA_debug[$handle] = $debug;
    $LDATA_prev_mod_time[$handle] = 0;
    $LDATA_n_fcasts_alloc[$handle] = 0;
    $LDATA_fcast_lead_times[$handle] = 0;
    $LDATA_prog_name[$handle] = $prog_name;
    $LDATA_source_str[$handle] = "";
    $LDATA_file_path[$handle] = "";
    $LDATA_temp_path[$handle] = "";
    $LDATA_latest_time[$handle] = 0;
    $LDATA_n_fcasts[$handle] = 0;
    $LDATA_file_ext[$handle] = "";
    $LDATA_user_info_1[$handle] = "";
    $LDATA_user_info_2[$handle] = "";

    return($handle);
}


###################################################################
#
# LDATA_info_print(handle, stream)
#
# Prints info to output stream
#
# Inputs:
#
#     handle:  handle value returned by LDATA_init_handle
#
#     stream:  output stream
#

sub LDATA_info_print
{
    local($handle, $stream) = @_;

    # Print the latest time

    print $stream "$LDATA_ltime_unix_time[$handle] " .
        "$LDATA_ltime_year[$handle] " .
        "$LDATA_ltime_month[$handle] " .
        "$LDATA_ltime_day[$handle] " .
        "$LDATA_ltime_hour[$handle] " .
        "$LDATA_ltime_minute[$handle] " .
        "$LDATA_ltime_second[$handle]\n";

    # Print file extension and user information fields

    print $stream "$LDATA_file_ext[$handle]\n";
    print $stream "$LDATA_user_info_1[$handle]\n";
    print $stream "$LDATA_user_info_2[$handle]\n";
    print $stream "$LDATA_n_fcasts[$handle]\n";
    print $stream "$LDATA_fcast_lead_times[$handle]\n";

}


###################################################################
#
# LDATA_info_write(handle, source_str, latest_time, file_ext,
#                  user_info_1, user_info_2, n_fcasts, fcast_lead_times)
#
# Writes latest info to file.
#
# Writes to a tmp file first, then moves the tmp file to the 
# final file name when done.
#
# Inputs:
#
#     handle:  handle value returned by LDATA_init_handle
#
#     source_str:
#             for file access, this is the data directory.
#             for network access, this is either
#                     port@host or
#                     type::subtype::instance
#
#     file_ext: file extension if applicable, otherwise set to NULL
#
#     user_info: set user information if applicable, otherwise NULL
#
#     n_fcasts: number of forecast times (Only the first forecast
#               time is currently processed, to match the processing
#               of forecast lead times in LDATA_info_read)
#
#     fcast_lead_times: array of forecast lead times,
#                       set this to NULL if n_fcasts == 0
#
# Side effects:
#    Fills out the file path in the handle.
#

sub LDATA_info_write
{
    local($handle, $source_str, $latest_time, $file_ext,
          $user_info_1, $user_info_2,
          $n_fcasts, $fcast_lead_times) = @_;

    # Set the file paths

    $LDATA_file_path[$handle] = "$source_str/_latest_data_info";
    $tmp_path = "$source_str/_latest_data_info.tmp";

    # Fill out latest data times

    $LDATA_latest_time[$handle] = $latest_time;
    $LDATA_ltime_unix_time[$handle] = $latest_time;
    ($LDATA_ltime_second[$handle],
     $LDATA_ltime_minute[$handle],
     $LDATA_ltime_hour[$handle],
     $LDATA_ltime_day[$handle],
     $LDATA_ltime_month[$handle],
     $LDATA_ltime_year[$handle],
     $wday, $yday, $isdst) = gmtime($latest_time);
    $LDATA_ltime_year[$handle] += 1900;
    $LDATA_ltime_month[$handle]++;

    # Copy in strings

    $LDATA_file_ext[$handle] = $file_ext;
    $LDATA_user_info_1[$handle] = $user_info_1;
    $LDATA_user_info_2[$handle] = $user_info_2;

    # Process forecast times

    $LDATA_n_fcasts[$handle] = $n_fcasts;

    if ($n_fcasts > 0)
    {
        if ($n_fcasts > 1)
        {
            print STDERR "Warning -- only processing first forecast in list!!!\n";
        }

        $LDATA_n_fcasts[$handle] = 1;
        $LDATA_fcast_lead_times[$handle] = $fcast_lead_times;
    }

    # Open the temp output file

    open LDATA_TMP_FILE, ">$tmp_path"
        or die "Can't open LDATA tmp file <$tmp_path>\n";

    # Write the info

    &LDATA_info_print($handle, LDATA_TMP_FILE);

    # Close the temp output file

    close LDATA_TMP_FILE;

    # Rename the temp file to the current file

    rename $tmp_path, $LDATA_file_path[$handle];
}

###################################################################
#
# LDATA_info_read()
#
# Usage: ($return_val, $lutime, $lyear, $lmonth, $lday, $lhour, $lminute,
#         $lsecond, $file_ext, $user_info1, $user_info2, $n_fcasts,
#         $fcast_lead_time) = LDATA_info_read($handle, $source_str, $max_valid_age)
#
# Read the struct data from the current file info, including forecast
# lead times if they are present.
#
# If the unix time in the file is not -1, the date and time is
# computed from the unix time.
# If the unix time in the file is -1, it is computed from the
# date and time.
#
# NOTE: This does NOT return all the values in the LDATA_* arrays since
#       we cannot guarantee that the latest_data_info file being read
#       was written by the Perl LDATA_info_write above. This only returns
#       values that LDATA_info_write above would write using LDATA_info_print
#       above.
#
# Inputs:
#
#   handle: see LDATA_init_handle()
#
#   source_str:
#
#     for file access, this is the data directory.
#
#   max_valid_age:
#
#     This is the max age (in secs) for which the 
#     latest data info is considered valid. If the info is
#     older than this, we need to wait for new info.
#
#     If max_valid_age is set negative, the age test is not done.
#
# Side effects:
#
#    (1) If new data found, sets handle->prev_mod_time to
#        file modify time.
#
#        NOTE: For this to work, the handle must be static between calls
#        since the prev_mod_time in the handle is used to determine when
#        the time of the file has changed.
#
#    (2) Fills out the file path in the handle.
#
# Returns:
#
#    $return_val       0 on success, -1 on failure.
#    ...
#

sub LDATA_info_read
{
    local($handle, $source_str, $max_valid_age, $debug) = @_;

    # Set local variables

    local($subname, $return_val);
    local($counter, $prev_time);
    local($lutime, $lyear, $lmonth, $lday, $lhour, $lminute, $lsecond);
    local($fname_ext, $user_info1, $user_info2, $n_fcasts, $fcast_lead_times);
    local($date, $utime);
    local($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks);
    local($dbg2);

    # Set defaults

    $subname="LDATA_info_read";
    $return_val=-1;

    # Additional debugging. Set to 1 for more detailed debugging

    $dbg2 = 0;

    # Print out inputs

    if ($debug) {
        print("$subname input:\n");
        print("\thandle: $handle\n");
        print("\tsource_str: $source_str\n");
        print("\tmax_valid_age: $max_valid_age\n");
    }
        
    # Initialize the return values. Cannot just return these values
    # since we are not sure that the LDATA_* arrays were filled if
    # these Perl functions were not used to build the LDATA file.

    $lutime = $LDATA_ltime_unix_time[$handle];
    $lyear = $LDATA_ltime_year[$handle];
    $lmonth = $LDATA_ltime_month[$handle];
    $lday = $LDATA_ltime_day[$handle];
    $lhour = $LDATA_ltime_hour[$handle];
    $lminute = $LDATA_ltime_minute[$handle];
    $lsecond = $LDATA_ltime_second[$handle];
    $fname_ext = $LDATA_file_ext[$handle];
    $user_info1 = $LDATA_user_info_1[$handle];
    $user_info2 = $LDATA_user_info_2[$handle];
    $n_fcasts = $LDATA_n_fcasts[$handle];
    $fcast_lead_times = $LDATA_fcast_lead_times[$handle];

    # Set the file path

    $LDATA_file_path[$handle] = "$source_str/_latest_data_info";

    # Does the file exist

    if (!-e $LDATA_file_path[$handle]) {
        print(STDERR "ERROR: $subname: File does not exist $LDATA_file_path[$handle]\n");
        return($return_val, $lutime, $lyear, $lmonth, $lday, $lhour, $lminute, $lsecond, $fname_ext, $user_info1, $user_info2, $n_fcasts, $fcast_lead_times);
    }

    # Stat the file to get its modify time

    ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks) = stat($LDATA_file_path[$handle]);

    if ($debug) {
        print(STDERR "$subname: file $LDATA_file_path[$handle] modify time: $mtime\n");
    }

    # Compute file age and check for max valid age

    if ($max_valid_age >= 0) {
        $now = time;
        $file_age = $now - $mtime;
        if ($file_age > $max_valid_age) {
            if ($debug) {
                print(STDERR "$subname: info file $LDATA_file_path[$handle] too old\n");
            }
            return($return_val, $lutime, $lyear, $lmonth, $lday, $lhour, $lminute, $lsecond, $fname_ext, $user_info1, $user_info2, $n_fcasts, $fcast_lead_times);
        }
    }

    # Check for modified file time

    if ($mtime == $LDATA_prev_mod_time[$handle]) {
        if ($debug) {
            print(STDERR "$subname: info file $LDATA_file_path[$handle]\n");
            print(STDERR "\tnot modified, last mod time: $LDATA_prev_mod_time[$handle]\n");
        }
        
        return($return_val, $lutime, $lyear, $lmonth, $lday, $lhour, $lminute, $lsecond, $fname_ext, $user_info1, $user_info2, $n_fcasts, $fcast_lead_times);
    }
            
    # Open the LDATA file for reading

    if (!open (LDATA_FILE, $LDATA_file_path[$handle])) {
        print(STDERR "ERROR: $subname: Can't open LDATA file $LDATA_file_path[$handle]\n");
        return($return_val, $lutime, $lyear, $lmonth, $lday, $lhour, $lminute, $lsecond, $fname_ext, $user_info1, $user_info2, $n_fcasts, $fcast_lead_times);
    }

    # Debug

    if ($debug) {
        print(STDERR "$subname: Opened the $LDATA_file_path[$handle] file for reading\n");
    }

    # Read through the file line by line

    $counter=0;

    while ($line = <LDATA_FILE>) {

        if ($dbg2) {
            print(STDERR "$subname: counter: $counter, line: $line");
        }

        # Parse the first line containing the unix time and the
        # year, month, day, hour, minute, seconds

        if ($counter == 0) {
            ($lutime, $lyear, $lmonth, $lday, $lhour, $lminute, $lsecond) = split (' ', $line);

        } 

        # Parse the second line containing the filename extension

        elsif ($counter == 1) {
            chomp($line);
            $fname_ext=$line;
        }

        # Parse the third line containing the user-info field
        
        elsif ($counter == 2) {
            chomp($line);
            $user_info1=$line;
        }

        # Parse the fourth line containing the user-info field
        
        elsif ($counter == 3) {
            chomp($line);
            $user_info2=$line;
        }

        # Parse the fifth line containing the number of forecasts

        elsif ($counter == 4) {
            chomp($line);
            $n_fcasts = $line;
        }

        # Parse the fifth line containing the forecast lead time

        elsif ($counter == 5) {
            chomp($line);
            $fcast_lead_times = $line;
        }

        $counter++;
    }

    close(LDATA_FILE);

    # If the UNIX time is -1, compute it from the date and time 

    if ($lutime == -1) {
        $date="${lyear}${lmonth}${lday} ${lhour}:${lminute}:${lsecond}";
        $utime = `date -u --date '$date' +%s`;
        $lutime = chop($utime);
    }

    # If the unix time in the file is not -1, compute the date and time
    # from the unix time

    else {
        ($lsecond,$lminute,$lhour,$mday,$lmonth,$lyear,$wday,$yday,$isdst)=gmtime($lutime);
        $lyear = $lyear + 1900;
        $lmonth = $lmonth + 1;
    }

    # Set the array contents

    $LDATA_ltime_unix_time[$handle] = $lutime;
    $LDATA_ltime_year[$handle] = $lyear;
    $LDATA_ltime_month[$handle] = $lmonth;
    $LDATA_ltime_day[$handle] = $lday;
    $LDATA_ltime_hour[$handle] = $lhour;
    $LDATA_ltime_minute[$handle] = $lminute;
    $LDATA_ltime_second[$handle] = $lsecond;
    $LDATA_file_ext[$handle] = $fname_ext;
    $LDATA_user_info_1[$handle] = $user_info1;
    $LDATA_user_info_2[$handle] = $user_info2;
    $LDATA_n_fcasts[$handle] = $n_fcasts;
    $LDATA_prev_mod_time[$handle] = $mtime;
    $LDATA_fcast_lead_times[$handle] = $fcast_lead_times;

    $return_val = 0;

    # Debug

    if ($dbg2) {
        print(STDERR "$subname: return_val: $return_val\n");
        print(STDERR "\tReturning... $lutime, $lyear, $lmonth, $lday, $lhour, $lminute, $lsecond, $fname_ext, $user_info1, $user_info2, $n_fcasts, $fcast_lead_times\n");
    }

    # Done

    return($return_val, $lutime, $lyear, $lmonth, $lday, $lhour, $lminute, $lsecond, $fname_ext, $user_info1, $user_info2, $n_fcasts, $fcast_lead_times);
}


###################################################################
# PMU routines
###################################################################

###################################################################
#
# PMU_auto_init(prog_name, instance, reg_interval)
#
# Set up statistics for procmap automatic registeration
#

sub PMU_auto_init
{
    local($prog_name, $instance, $reg_interval) = @_;

    # Save the information in global variables

    $PMU_prog_name = $prog_name;
    $PMU_instance = $instance;
    $PMU_reg_interval = $reg_interval;
    $PMU_last_register = -1;
    $PMU_start_time = time;

    if ($opt_debug)
    {
        print "PMU_auto_init:\n";
        print "prog_name = $PMU_prog_name\n";
        print "instance = $PMU_instance\n";
        print "reg_interval = $PMU_reg_interval\n";
    }
}

###################################################################
#
# PMU_auto_register(status_string)
#
# Automatically registers if reg_interval seconds have expired
# since the previous registration.
#
# This routine may be called frequently, registration will only
# occur at the specified reg_interval.
#

sub PMU_auto_register
{
    local($status_str) = @_;

    local($command, $now);

    $now = time;

    if ($now - $PMU_last_register > $PMU_reg_interval)
    {
        $command =
            "procmap_register -name $PMU_prog_name -instance $PMU_instance" .
                " -status_str \"$status_str\" -pid $$" .
                    " -reg_int $PMU_reg_interval -start $PMU_start_time";

        if ($opt_debug)
        {
            print "$command\n";
        }

        system($command);

        $PMU_last_register = $now;
    }
}

###################################################################
#
# PMU_force_register(status_str)
#
# Forced registration.
#
# This routine should only be called from places in the code which do
# not run frequently.  Call PMU_auto_register() from most places
# in the code.
#

sub PMU_force_register
{
    local($status_str) = @_;

    local($command, $now);

    $now = time;

    $command =
        "procmap_register -name $PMU_prog_name -instance $PMU_instance" .
            " -status_str \"$status_str\" -pid $$" .
                " -reg_int $PMU_reg_interval -start $PMU_start_time";

    if ($opt_debug)
    {
        print "$command\n";
    }

    system($command);

    $PMU_last_register = $now;
}

###################################################################
#
# PMU_auto_unregister()
#
# Automatically unregisters - remembers process name and instance
#

sub PMU_auto_unregister
{
    local($command);

    $command =
        "procmap_unregister -name $PMU_prog_name -instance $PMU_instance" .
            " -pid $$";

    if ($opt_debug)
    {
        print "$command\n";
    }

    system($command);
}
 
##############################################################################
# &safe_system: Run a command with a timeout.
# Arguments:
#     $cmd: a string containing the command (with arguments) to be executed.
#     $timeout: the timeout, in seconds.
#     (OTH: Added Sat Sep  4 16:41:01 EST 1999)
#         $debug: enable debugging output if non-zero.
#
# Returns: The undefined value on timeout or if the cmd exits with a
#     non-zero value, a string containing any output from the 
#     command otherwise (probable success).
#
# F. Hage NCAR/RAP 1998; Stolen from &rsh() by
#     Tres "Giant Brain" Hofmeister, NCAR/RAP
#
# Modified March 2007 to try more moderate kill signals than 9 first.
# Niles Oien.
#

sub safe_system {
    my($cmd, $timeout, $debug) = @_;
    my( $ruser, $return, $cpid, $tmp, $signal, $ztime, $cexit, $exit,
       @signalsToSend, $signalIndex, $go );

    local($is_ok);

    undef $return;
 
    # Child code.
    unless ($cpid = fork) {
        # Temporary file for output.
        $tmp = "/tmp/safe_sys_stdout.$$";
        open(STDOUT, ">$tmp") || die "open: $tmp: $!";
        open(STDERR, ">&STDOUT");
        exec("$cmd");
        die "exec: $cmd : $!";
    }
 
    # The Child's output goes here
    $tmp = "/tmp/safe_sys_stdout.$cpid";

    # Set up the timeout.
    $ztime = time + $timeout;
 
    # Loop until the child times out or exits.
    while (time < $ztime) {
        $cexit = 1, last if waitpid($cpid, WNOHANG) > 0;
        select(undef, undef, undef, .5)
    }
 
    # Kill the child process if necessary.
    unless ($cexit) {


      ##  kill(9, $cpid); Replaced by incremental signal kill - Niles.

	@signalsToSend = (1, 2, 15, 11, 9, -1 );
	$signalIndex = 0;
	$go = 1;
	
	do {
	    $canSignal = kill(0, $cpid);
	    if (0 == $canSignal){
		$go = 0;
	    } else {
		if ($debug){
		    print (STDERR "Trying to kill process $gpid with signal $signalsToSend[$signalIndex]\n");
		}
		kill($signalsToSend[$signalIndex], $cpid);
		sleep 1;
		$signalIndex++;
		if ($signalsToSend[$signalIndex] < 0){
		    $go = 0;
		}
	    }
	} while( $go );	
	
        # End of incremental signal kill
	
        warn "$$: safe_system timeout: PID $cpid killed.\n" if $debug;

        #  Is it a bad idea to block here?
        warn "wait: $!" if waitpid($cpid, 0) < 0;
    }
 
    # Collect exit status: The child exited normally.
    if (WIFEXITED($?)) {
        $exit = WEXITSTATUS($?);
        if ($exit != 0) {
            warn "$$: $cpid (safe_system) exit status: $exit\n" if $debug;
        }
        # Gather the output of the remote command.  A null string must be
        # returned when the command succeeds but produces no output.
        else {
            $return = "";
            open(TMP, $tmp) || warn "open: $tmp: $!";
            while (<TMP>) {
                $return .= $_;
                $return .= "\n" unless /\n$/;
            }
            close(TMP) || warn "close: $tmp: $!";
        }
    }
    # The child was signaled.
    elsif (WIFSIGNALED($?)) {
        $signal = WTERMSIG($?);
        warn "$$: $cpid (safe_system) exited on signal $signal\n" if $debug;
    }
    # The child was stopped for some reason.  Note: we should never get
    # here with a blocking wait...
    else {
        $signal = WSTOPSIG($?);
        warn "$$: $cpid (safe_system) stopped on signal $signal, killing $cpid\n"
            if $debug;
 
        kill(9, $cpid);
 
        # Is it a bad idea to block here?
        warn "wait: $!" if waitpid($cpid, 0) < 0;
    }
 
    unlink($tmp) || warn "unlink: $tmp: $!" if -f $tmp;
 
    # Return the result, or undef on failure.
    $return;
}

##############################################################################
# &safe_system_sh: Run a command with a timeout.
# Arguments:
#     $cmd: a string containing the command (with arguments) to be executed.
#     $timeout: the timeout, in seconds.
#     (OTH: Added Sat Sep  4 16:41:01 EST 1999)
#         $debug: enable debugging output if non-zero.
#
# Returns: The undefined value on timeout or if the cmd exits with a
#     non-zero value, a string containing any output from the 
#     command otherwise (probable success).
#
# F. Hage NCAR/RAP 1998; Stolen from &rsh() by
#     Tres "Giant Brain" Hofmeister, NCAR/RAP
# Modified Jan 2002 by Mike Dixon and Deirdre Garvey to use sh to execute
#     the input command and therefore not reassign STDOUT
#
# Modified March 2007 to try more moderate kill signals than 9 first.
# Niles Oien.
#
sub safe_system_sh {
    my($cmd, $timeout, $debug) = @_;
    my( $ruser, $return, $cpid, $tmp, $signal, $ztime, $cexit, $exit,
        @signalsToSend, $signalIndex, $go );

    local($full_cmd);
    undef $return;

    # Child code.
    unless ($cpid = fork) {
        # Temporary file for output.
        $tmp = "/tmp/safe_sys_stdout.$$";

        # Build the full command string using Bourne Shell and redirection

        $full_cmd="sh -c \"$cmd\" > $tmp 2>&1";

        if ($debug) {
            print(STDERR "in safe_system_sh: full_cmd: $full_cmd\n");
        }

        # Original safe_system() calls which reassigned STDOUT
##
##        open(STDOUT, ">$tmp") || die "open: $tmp: $!";
##        open(STDERR, ">&STDOUT");

        exec("$full_cmd");
        die "exec: $!";
    }

    # The Child's output goes here
    $tmp = "/tmp/safe_sys_stdout.$cpid";

    # Set up the timeout.
    $ztime = time + $timeout;

    # Loop until the child times out or exits.
    while (time < $ztime) {
        $cexit = 1, last if waitpid($cpid, WNOHANG) > 0;
        select(undef, undef, undef, .5)
    }

    # Kill the child process if necessary.
    unless ($cexit) {
        kill(9, $cpid);
        warn "$$: safe_system timeout: PID $cpid killed.\n" if $debug;

        #  Is it a bad idea to block here?
        warn "wait: $!" if waitpid($cpid, 0) < 0;
    }

    # Collect exit status: The child exited normally.
    if (WIFEXITED($?)) {
        $exit = WEXITSTATUS($?);
        if ($exit != 0) {
            warn "$$: $cpid (safe_system) exit status: $exit\n" if $debug;
        }
        # Gather the output of the remote command.  A null string must be
        # returned when the command succeeds but produces no output.
        else {
            $return = "";
            open(TMP, $tmp) || warn "open: $tmp: $!";
            while (<TMP>) {
                $return .= $_;
                $return .= "\n" unless /\n$/;
            }
            close(TMP) || warn "close: $tmp: $!";
        }
    }
    # The child was signaled.
    elsif (WIFSIGNALED($?)) {
        $signal = WTERMSIG($?);
        warn "$$: $cpid (safe_system) exited on signal $signal\n" if $debug;
    }
    # The child was stopped for some reason.  Note: we should never get
    # here with a blocking wait...
    else {
        $signal = WSTOPSIG($?);
        warn "$$: $cpid (safe_system) stopped on signal $signal, killing $cpid\n"
            if $debug;


        ## kill(9, $cpid); ## Kill with -9 replaced by incremental signal kill - Niles.

	@signalsToSend = (1, 2, 15, 11, 9, -1 );
	$signalIndex = 0;
	$go = 1;
	
	do {
	    $canSignal = kill(0, $cpid);
	    if (0 == $canSignal){
		$go = 0;
	    } else {
		if ($debug){
		    print (STDERR "Trying to kill process $gpid with signal $signalsToSend[$signalIndex]\n");
		}
		kill($signalsToSend[$signalIndex], $cpid);
		sleep 1;
		$signalIndex++;
		if ($signalsToSend[$signalIndex] < 0){
		    $go = 0;
		}
	    }
	} while( $go );	

        # End of incremental signal kill


        # Is it a bad idea to block here?
        warn "wait: $!" if waitpid($cpid, 0) < 0;
    }

    unlink($tmp) || warn "unlink: $tmp: $!" if -f $tmp;

    # Return the result, or undef on failure.

    if ($debug) {
        print(STDERR "==== in safe_system_sh: cmd: $cmd, return: $return\n");
    }

    $return;
}

##############################################################################
# &safeSystem : Run a command with a timeout.
# Arguments:
#     $cmd: a string containing the command (with arguments) to be executed.
#     $timeout: the timeout, in seconds.
#     (OTH: Added Sat Sep  4 16:41:01 EST 1999)
#         $debug: enable debugging output if non-zero.
#
# Returns: The undefined value if we cannot start
#          a new process or if the process starts
#          and cannot be killed. Otherwise any output
#          from the command is returned in a string,
#          even if it timed out and was successfully terminated.
#
# History :
#
# safe_system was written by F. Hage NCAR/RAP 1998; Stolen from &rsh() by
#             Tres "Giant Brain" Hofmeister, NCAR/RAP
#
# safe_system_sh was written by Jan 2002 by Mike Dixon and Deirdre
#                Garvey to use sh to execute the input command and 
#                therefore not reassign STDOUT
#
# safeSystem was written March 2007 by Niles Oien to double-fork
#            for a more daemon-like approach, and
#            to work up to using the kill 9 signal
#            rather than using that signal
#            right off the bat for timed out processes. It
#            developed into safeSystemSignalsSpecified(), which
#            allows one to specify which signals to use (in order)
#            ending with -1 to indicate the end of the array.
#            safeSystem() now just calls safeSystemSignalsSpecfied with
#            fairly sensible arguments. Callers wanting to use
#            their own set of signals to kill a process can call
#            safeSystemSignalsSpecified() directly.
#

sub safeSystem {
    my($cmd, $timeout, $debug) = @_;
    my (@signalsToSend, $return);

    #
    # Array of kill signals to try. End this array with -1
    #
    @signalsToSend = (1, 2, 15, 11, 9, -1 );

    $return = Toolsa::safeSystemSignalsSpecified($cmd, $timeout, $debug, @signalsToSend);

    $return;

}

#######################################

sub safeSystemSignalsSpecified {
    my($cmd, $timeout, $debug, @signalsToSend) = @_;

    my($return, $pid, $gpid, $tmp, $full_cmd, $endTime, $go, $processIsRunning, $numSec, $canSignal,
       $signalIndex, $ptk );

    #
    # Spawn a process, then watch it, if it takes too long
    # to execute, then assume something is hung and kill it. Done
    # by forking twice, to get "init" to own the child process,
    # the trick being to pass the grandchild's (the
    # child's child) PID back to the parent. This is done with
    # a nifty "open" that I found online. Niles Oien March 2007.
    #
    
    undef $return;

    if ($debug){
	print (STDERR "In safeSystemSignalsSpecified() with command \"$cmd\"\n");
	$signalIndex = 0;
        $go = 1;
        do {
            if ( @signalsToSend[$signalIndex] > 0){
		printf(STDERR "  Kill signal number $signalIndex is @signalsToSend[$signalIndex]\n");
		$signalIndex++;
	    } else {
		$go = 0;
	    }
	} while ( $go );
    }
    #
    # Spawn a child process in such a way that the child's
    # STDOUT will be accessable to us via a filehandle, see
    #
    # http://perldoc.perl.org/perlipc.html#Using-open()-for-IPC
    #
    $pid = open(CHILD_PROCESS_STDOUT, "-|");
    unless (defined $pid) {
	print(STDERR "Failed to fork!\n"); # Highly unlikely.
	$return;                           # Return the undefined value.
    }

    if ($pid == 0){ 
	#
	# Child. This is convoluted. Fork another child, write that PID to STDOUT.
	# Parent reads STDOUT via CHILD_PROCESS_STDOUT, then closes
	# CHILD_PROCESS_STDOUT and monitors the child's child (grandchild).
	#
	$gpid = fork; # Grandchild PID.
	#
	if ($gpid == 0){
	    #
	    # Grandchild. Exec what we have to exec here.
	    #
	    # Temporary file for output.
            #
	    $tmp = "/tmp/safeSysOut.$$";
            # 
	    # Build the full command string using Bourne Shell and redirection
            #
	    $full_cmd="$cmd > $tmp 2>&1";
            #
	    if ($debug) {
		print(STDERR "in safeSystem : full_cmd: $full_cmd\n");
	    }

	    exec("$full_cmd");
	    die "exec: $!";

	} else {
	    #
	    # Still attached to child. Write the PID we just
	    # spawned back to the main thread via our STDOUT, which
	    # appears to the parent as CHILD_PROCESS_STDOUT. Then exit.
	    #
	    print (STDOUT "$gpid\n");
	    exit( 0 );
	}
	
    } else { # Parent.
	
	#
	# Read the grandchild PID back from the child.
	#
	chop($gpid = <CHILD_PROCESS_STDOUT>);
	close(CHILD_PROCESS_STDOUT);
        if ($debug){
	    print (STDERR "Grandchild PID is $gpid, monitoring for $timeout seconds...\n");
	}
	#
	# Wait for the child process child to die (this won't take long,
        # since it is the child process, not the grandchild process).
	#
	$ret = waitpid($pid, WNOHANG);
	
	#
	# In the parent process. Monitor the grandchild process.
	#
	$endTime = time + $timeout;
	$go = 1;
	$processIsRunning = 1;
	$numSec = 0;

	do {

	    #
	    # See if we can signal the grandchild process.
	    # Do this by sending a kill level 0 to the process,
	    # which does not actually bother the process at all, see
	    # http://perldoc.perl.org/functions/kill.html
	    #
	    $canSignal = kill(0, $gpid);
            if ($debug){
		if ($canSignal != 0){
		    print (STDERR "Grandchild process is running after $numSec seconds.\n");
		}
	    }
	  	    
	    #
	    # If $canSignal is 0, the process has exited.
	    #
	    if ($canSignal == 0){
		$go = 0;
		$processIsRunning = 0;
	    }
	    
	    #
	    # See if we are out of time.
	    #
	    if (time > $endTime){
		$go = 0;
	    }
	    
	    #
	    # If we are still going, wait for a second before next loop.
	    #
	    if ($go == 1){
		sleep 1;
		$numSec++;
	    }
	    
	} while ( $go); # End of loop monitoring the grandchild process.
	
	if ($debug){
	    if ($processIsRunning == 1){
		print (STDERR "The grandchild process is still running after $timeout seconds.\n");
	    } else {
		print (STDERR "The grandchild process exited before timeout.\n");
	    }
	}

	if ($processIsRunning == 1){
	    #
	    # We need to kill the grandchild process. This is
	    # a little turgid. What is done is to try sending
	    # it signals in the order specified. After
	    # each of these signals, wait a second and then "ping"
	    # the process with signal 0 - if it is still there,
	    # send the next signal in the sequence of nastiness,
	    # if it is not, we're done.
	    #

	    $signalIndex = 0;
	    $go = 1;

            #
            # It turns out that the process to kill is actually number
            # $gpid plus one. I'm not entirely sure why this is, but it looks like
            # perl throws another fork in there somewhere.
            #
            $ptk = $gpid + 1; # Process to kill.
	    do {
		$canSignal = kill(0, $gpid);
		if (0 == $canSignal){
		    $go = 0;
		} else {
		    if ($debug){
			print (STDERR "Trying to kill process $ptk with signal $signalsToSend[$signalIndex]\n");
		    }
		    kill($signalsToSend[$signalIndex], $ptk);
		    sleep 1;
		    $signalIndex++;
		    if ($signalsToSend[$signalIndex] < 0){
			$go = 0;
		    }
		}
	    } while( $go );	

	    if ($debug){
		$canSignal = kill(0, $gpid);
		if (0 == $canSignal){
		    print (STDERR "Process $gpid killed successfully.\n");
		} else {
		    print (STDERR "Unable to kill process $gpid\n");
		}
	    }
	}

        #
        # Return undefined if the process cannot be terminated (unlikely).
        #
        $canSignal = kill(0, $gpid);
	if (0 != $canSignal){
	    $return;
	}

        #
	# Otherwise gather the output of the remote command for return value.  A null string must be
        # returned when the command succeeds but produces no output.
        #
        $return = "";
	$tmp = "/tmp/safeSysOut.$gpid";

	open(TMP, $tmp) || warn "open: $tmp: $!";
	while (<TMP>) {
	    $return .= $_;
	    $return .= "\n" unless /\n$/;
	}
	close(TMP) || warn "close: $tmp: $!";
	unlink($tmp) || warn "unlink: $tmp: $!" if -f $tmp;

	if ($debug){
	    print (STDERR "----------------------------------------\n");
	    print (STDERR "Return from safeSystem(): $return\n");
	    print (STDERR "----------------------------------------\n");
	}
    }

    $return;

}


#######################################
#
# safeSystemKillScript() is an attempt to allow a caller to
# specify a script to kill the process they have started rather
# than relying on sending the process kill signals. 
#
# It is up to the caller to specify an effective kill script.
# No checks are made that the kill script has worked. If the kill
# script hangs, this routine hangs.
#
# Niles Oien April 2007
#
sub safeSystemKillScript {
    my($cmd, $timeout, $debug, $killScript) = @_;
    
    my($return, $pid, $gpid, $tmp, $full_cmd, $endTime, $go, $processIsRunning, $numSec, $canSignal,
       $signalIndex, $ptk );
    
    #
    # Spawn a process, then watch it, if it takes too long
    # to execute, then assume something is hung and kill it. 
    # Process is killed by doing a system call to execute
    # a kill script that the caller specifies. Niles Oien,
    # mid April 2007.
    #
    
    undef $return;
    
    if ($debug){
	print (STDERR "In safeSystemKillScript() with command \"$cmd\"\n");
	print (STDERR "Kill script $killScript to be used after $timeout seconds.\n");
    }
    #
    # Spawn a child process in such a way that the child's
    # STDOUT will be accessable to us via a filehandle, see
    #
    # http://perldoc.perl.org/perlipc.html#Using-open()-for-IPC
    #
    $pid = open(CHILD_PROCESS_STDOUT, "-|");
    unless (defined $pid) {
	print(STDERR "Failed to fork!\n"); # Highly unlikely.
	$return;                           # Return the undefined value.
    }

    if ($pid == 0){ 
	#
	# Child. This is convoluted. Fork another child, write that PID to STDOUT.
	# Parent reads STDOUT via CHILD_PROCESS_STDOUT, then closes
	# CHILD_PROCESS_STDOUT and monitors the child's child (grandchild).
	#
	$gpid = fork; # Grandchild PID.
	#
	if ($gpid == 0){
	    #
	    # Grandchild. Exec what we have to exec here.
	    #
	    # Temporary file for output.
            #
	    $tmp = "/tmp/safeSysOut.$$";
            # 
	    # Build the full command string using Bourne Shell and redirection
            #
	    $full_cmd="$cmd > $tmp 2>&1";
            #
	    if ($debug) {
		print(STDERR "in safeSystemKillScript : full_cmd: $full_cmd\n");
	    }
	    
	    exec("$full_cmd");
	    die "exec: $!";
	    
	} else {
	    #
	    # Still attached to child. Write the PID we just
	    # spawned back to the main thread via our STDOUT, which
	    # appears to the parent as CHILD_PROCESS_STDOUT. Then exit.
	    #
	    print (STDOUT "$gpid\n");
	    exit( 0 );
	}
	
    } else { # Parent.
	
	#
	# Read the grandchild PID back from the child.
	#
	chop($gpid = <CHILD_PROCESS_STDOUT>);
	close(CHILD_PROCESS_STDOUT);
        if ($debug){
	    print (STDERR "Grandchild PID is $gpid, monitoring for $timeout seconds...\n");
	}
	#
	# Wait for the child process child to die (this won't take long,
        # since it is the child process, not the grandchild process).
	#
	$ret = waitpid($pid, WNOHANG);
	
	#
	# In the parent process. Monitor the grandchild process.
	#
	$endTime = time + $timeout;
	$go = 1;
	$processIsRunning = 1;
	$numSec = 0;
	
	do {
	    
	    #
	    # See if we can signal the grandchild process.
	    # Do this by sending a kill level 0 to the process,
	    # which does not actually bother the process at all, see
	    # http://perldoc.perl.org/functions/kill.html
	    #
	    $canSignal = kill(0, $gpid);
            if ($debug){
		if ($canSignal != 0){
		    print (STDERR "Grandchild process is running after $numSec seconds.\n");
		}
	    }
	    
	    #
	    # If $canSignal is 0, the process has exited.
	    #
	    if ($canSignal == 0){
		$go = 0;
		$processIsRunning = 0;
	    }
	    
	    #
	    # See if we are out of time.
	    #
	    if (time > $endTime){
		$go = 0;
	    }
	    
	    #
	    # If we are still going, wait for a second before next loop.
	    #
	    if ($go == 1){
		sleep 1;
		$numSec++;
	    }
	    
	} while ( $go); # End of loop monitoring the grandchild process.
	
	if ($debug){
	    if ($processIsRunning == 1){
		print (STDERR "The grandchild process is still running after $timeout seconds.\n");
	    } else {
		print (STDERR "The grandchild process exited before timeout.\n");
	    }
	}

	if ($processIsRunning == 1){
	    #
	    # We need to kill the process. 
            #
	    if ($debug){
		print (STDERR "Killing the process with $killScript\n");
		#
		# No way to know if this won't hang, too - have to have some faith
		# that caller has set up an effective kill script.
		#
		system( $killScript );
	    }
	}

        #
	# Gather the output of the remote command for return value.  A null string must be
        # returned when the command succeeds but produces no output.
        #
        $return = "";
	$tmp = "/tmp/safeSysOut.$gpid";
	
	open(TMP, $tmp) || warn "open: $tmp: $!";
	while (<TMP>) {
	    $return .= $_;
	    $return .= "\n" unless /\n$/;
	}
	close(TMP) || warn "close: $tmp: $!";
	unlink($tmp) || warn "unlink: $tmp: $!" if -f $tmp;
	
	if ($debug){
	    print (STDERR "----------------------------------------\n");
	    print (STDERR "Return from safeSystemKillScript(): $return\n");
	    print (STDERR "----------------------------------------\n");
	}
    }
    
    $return;
    
}


# Make sure the file returns a 1 because PERL seems to require this

1;
