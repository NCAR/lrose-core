#!/bin/sh --
#
# kill_hung_spong_df.sh
#
# Shell script to look for hung spong /bin/df processes. This should
# be run periodically. Only kills processes over 24 hours old.
#
# This is intentionally written in Bourne Shell to be as lightweight
# as possible.
# 
#=====================================================================
#
############### Defaults #######################
#
# Set defaults (season to taste)
#
Exit_success=1
Exit_failure=0

Today=`date -u`
Tmp_dir="/tmp"

# Set command line defaults

Do_kill=TRUE

###########  Command line parsing ################

# Parse command line

for var in $@ ; do
    case $var in
        -h) echo "Usage: $0 [-h] [-nokill]"
            echo "Purpose: To check the process table for hung spong df processes"
	    echo "         and kill them."
	    echo "  -h      : Print this usage message"
	    echo "  -nokill : Report only, do not kill the processes"
	    exit $Exit_failure
	    ;;
	-nokill) Do_kill=FALSE
	    ;;
    esac
done

############### Define functions #######################
#
function LogAdd
{
     echo $1
}

function DoKill
{
    if test $Do_kill = TRUE ; then
	LogAdd "$Today: Killing PID: $1"
	kill -9 $1
    else
	LogAdd "$Today: Would kill PID: $1"
    fi
}

################# End of functions #####################


############### Main #######################
#
# Is there even a df process in the process table?

ps -f U spong | grep df | head -1
ret_val=$?

if test $ret_val -eq 1 ; then
    echo "$Today: No hung spong df process"
    exit $Exit_success
fi

# Define the output columns of the ps -f command

ps_header_file=$Tmp_dir/ps_header.tmp
if test -e $ps_header_file ; then
    /bin/rm -f $ps_header_file
fi

ps -f | head -1 > $ps_header_file
col1=`awk '{print $1}' $ps_header_file`
col2=`awk '{print $2}' $ps_header_file`
col3=`awk '{print $3}' $ps_header_file`
col4=`awk '{print $4}' $ps_header_file`
col5=`awk '{print $5}' $ps_header_file`
##echo "col1: $col1, col2: $col2, col3: $col3, col4: $col4, col5: $col5"
pid_col=-1
stime_col=-1
if test $col2 = PID ; then
    pid_col=2
fi
if test $col5 = STIME ; then
    stime_col=5
fi

if test $pid_col -lt 0 ; then
    LogAdd "ERROR: Cannot determine the ps column for PID"
    exit $Exit_failure
fi

if test $stime_col -lt 0 ; then
    LogAdd "ERROR: Cannot determine the ps column for STIME"
    exit $Exit_failure
fi

#
# Get the pid of the ps -f U spong command

tmp_file=$Tmp_dir/kill_hung_spong_df.tmp
if test -e $tmp_file ; then
    /bin/rm -f $tmp_file
fi

ps -f U spong | grep df | head -1 > $tmp_file
pid=`awk '{print $2}' $tmp_file`

#
# Get the date part of the ps -f U spong | grep df command

tmp_file=$Tmp_dir/kill_hung_spong_df.tmp
if test -e $tmp_file ; then
    /bin/rm -f $tmp_file
fi

ps -f U spong | grep df | head -1 | awk '{print $stime_col}' > $tmp_file

#
# If the date has a : in it, then it is likely hours and minutes.
# If the date does not have a : in it then is likely a date and the
# process is at least 24 hours old.
# grep returns 0 on success, 1 on failure

grep ":" $tmp_file
ret_val=$?

##echo "found a semi-colon in date? $ret_val"

if test $ret_val -eq 0 ; then
    LogAdd "$Today: Found a df process at least 24 hours old"
    DoKill $pid
    exit $Exit_success
fi

exit $Exit_success
