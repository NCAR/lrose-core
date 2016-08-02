#!/bin/sh
#
# Function: Script to check the CPU load on the current machine and
#           log when the load is over a certain amount. This allows
#           tracking of what is contributing to the load problem
#           automatically.
#
# Usage:    Expect to be run under cron
#
# Input:    output of 'uptime' and 'top' on the current host
# 
# Output:   Logs contents of 'top' when 'uptime' load average crosses
#           a specified value. Output is written to STDOUT. No output
#           is written until the load warn number is reached.
#
# Author:   Deirdre Garvey   NCAR/RAL/SDG
#           16-AUG-2005
#
############### Defaults #######################
#
# Set defaults (season to taste)
#
Exit_success=1
Exit_failure=0

# Set command line defaults

Debug=FALSE
Run_once=FALSE
Check_zombies=TRUE
Load_warn=2
Sleep_secs=60
Lines_from_top=20
Print_details=FALSE

###########  Command line parsing ################

# Parse command line

while getopts dhs:xzpl:t: option
do
    case "$option" in
        h) echo "Usage: $0 [-h] [-d] [-l load_warn] [-p] [-s sleep_secs] [-t lines] [-x] [-z]"
            echo "Purpose: To check the CPU for a load higher than: $Load_warn"
	    echo "         Uses the top comand and logs to STDOUT"
            echo " "
	    echo "  -d            : Print debug messages"
            echo "  -h            : Print this usage message"
	    echo "  -l load_warn  : Specify the load warning level, must be an integer"
	    echo "                  Default is $Load_warn"
	    echo "  -p            : Print details about the top 10 processes"
	    echo "  -s sleep_secs : Specify the sleep seconds between checks."
	    echo "                  Default is $Sleep_secs"
	    echo "  -t lines      : Specify the number of lines from 'top' to display"
	    echo "                  Default is $Lines_from_top"
            echo "  -x            : Run check once and exit."
            echo "                  Default is loop forever sleeping Sleep_secs secs between checks"
	    echo "  -z            : Do not check zombies. Default is to check for zombies"
            exit $Exit_failure
            ;;
        d) Debug=TRUE
	    ;;
        l) Load_warn=$OPTARG
	    ;;
        p) Print_details=TRUE
	    ;;
	s) Sleep_secs=$OPTARG
	    ;;
	t) Lines_from_top=$OPTARG
	    ;;
        z) Check_zombies=FALSE
	    ;;
        x) Run_once=TRUE
            ;;
    esac
done

#
# Debug
#

if test $Debug = TRUE; then
    echo "Settings..."
    echo "  Load_warn: $Load_warn"
    echo "  Sleep_secs: $Sleep_secs"
    echo "  Lines_from_top: $Lines_from_top"
    echo "  Print_details: $Print_details"
    echo "  Run_once: $Run_once"
    echo "  Check_zombies: $Check_zombies"
fi

#
# Start looping
#

Done=FALSE
while [ $Done = FALSE ] ; do

    # Extract the current load average
    # The first sed removes the trailing ,
    # The second sed removes the second part of the load number after
    #   the . because an integer is required in the if test, not a float
    #   so do a cheap modulus on the number using sed

    load=`uptime | awk '{print $10}' | sed 's/,//' | sed 's/\..*$//'`

    # Run top

    if test $load -ge $Load_warn ; then
    
	#       Print header
	now=`date -u`
	echo "======= Checking CPU load at time: $now"

	#	Do the check
	top -n 1 -b | head -$Lines_from_top

	#       Check for zombies
	if test $Check_zombies = TRUE ; then
		echo "---- Checking for zombies ----"
		ps auxww | grep defunct
	fi

	#       If print details, get approx the top 10 PIDs and print details

	if test $Print_details = TRUE ; then
	        echo "---- Print top processes ----"
		pid_list=`top -n 1 -b | head -20 | tail -13 | awk '{print $1}'`
		first=TRUE
		for pid in $pid_list ; do
		    if test $first = TRUE; then
			ps uww --pid $pid
			first=FALSE
		    else
			ps uwwh --pid $pid
		    fi
		done
	fi
    fi

    # Sleep
    
    if test $Run_once = TRUE ; then
        Done=TRUE
    fi

    if test $Done = FALSE ; then
        sleep $Sleep_secs
    fi

done # Done

exit $Exit_success
