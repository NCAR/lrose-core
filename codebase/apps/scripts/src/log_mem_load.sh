#!/bin/sh
#
# Function: Script to check the memory load on the current machine and
#           log when the load is over a certain amount. This allows
#           tracking of what is contributing to the memory problem
#           automatically.
#
# Usage:    Expect to be run under cron
#
# Input:    output of 'top', 'ps', 'free' on the current host
# 
# Output:   Logs contents of 'top' and 'ps' when free memory drops below
#           a specified value. Output is written to STDOUT. No output
#           is written until the load warn number is reached.
#
# Author:   Deirdre Garvey   NCAR/RAL/SDG
#           22-AUG-2005
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
Total_mem_free_warn=100
Physical_mem_free_warn=50
Need_both_warn=FALSE
Sleep_secs=60
Lines_from_top=20

###########  Command line parsing ################

# Parse command line

while getopts dhbs:xzm:t:p: option
do
    case "$option" in
        h) echo "Usage: $0 [-h] [-d] [-b] [-m mem_free] [-p mem_free] [-s sleep_secs] [-t lines] [-x] [-z]"
            echo "Purpose: To check the memory usage for less than ${Total_mem_free_warn} MB total"
	    echo "         memory free and/or less than ${Physical_mem_free_warn} MB physical memory free."
	    echo "         Check for zombies. Log to STDOUT when memory usage warning"
	    echo "         boundary crossed"
            echo " "
	    echo "  -b            : Need to cross both total and physical memory free warning"
	    echo "                  levels to log a warning."
	    echo "                  Default is to warn when either total or physical warning"
	    echo "                  level is crossed"
	    echo "  -d            : Print debug messages"
            echo "  -h            : Print this usage message"
	    echo "  -m mem_free   : Specify the total free memory (in MB) below which to warn"
	    echo "                  Default is $Total_mem_free_warn MB"
	    echo "  -p mem_free   : Specify the physical free memory (in MB) below which to warn"
	    echo "                  Default is $Physical_mem_free_warn MB"
	    echo "  -s sleep_secs : Specify the sleep seconds between checks."
	    echo "                  Default is $Sleep_secs"
	    echo "  -t lines      : Specify the number of lines from 'top' to display"
	    echo "                  Default is $Lines_from_top"
            echo "  -x            : Run check once and exit."
            echo "                  Default is loop forever sleeping Sleep_secs secs between checks"
	    echo "  -z            : Do not check zombies. Default is to check for zombies"
            exit $Exit_failure
            ;;
        b) Need_both_warn=TRUE
	    ;;
        d) Debug=TRUE
	    ;;
        m) Total_mem_free_warn=$OPTARG
	    ;;
        p) Physical_mem_free_warn=$OPTARG
	    ;;
	s) Sleep_secs=$OPTARG
	    ;;
	t) Lines_from_top=$OPTARG
	    ;;
        x) Run_once=TRUE
            ;;
        z) Check_zombies=FALSE
	    ;;
    esac
done

#
# Debug
#

if test $Debug = TRUE; then
    echo "Settings..."
    echo "  Total_mem_free_warn: $Total_mem_free_warn"
    echo "  Physical_mem_free_warn: $Physical_mem_free_warn"
    echo "  Need_both_warn: $Need_both_warn"
    echo "  Sleep_secs: $Sleep_secs"
    echo "  Lines_from_top: $Lines_from_top"
    echo "  Run_once: $Run_once"
    echo "  Check_zombies: $Check_zombies"
fi

# Start looping

Done=FALSE
while [ $Done = FALSE ] ; do

    # Extract the current total memory free

    total_mem_free_mb=`free -m | awk '{print $4}' | tail -1`

    # Extract the current physical memory free

    physical_mem_free_mb=`free -m | awk '{print $4}' | head -2 | tail -1` 

    # Debug

     if test $Debug = TRUE ; then
	echo "Total memory free: $total_mem_free_mb MB"
	echo "Physical memory free: $physical_mem_free_mb MB"
     fi

    # Check against warning levels

    do_print=FALSE
    warn_total=TRUE
    if test $total_mem_free_mb -lt $Total_mem_free_warn ; then
	warn_total=TRUE
	if test $Need_both_warn = FALSE; then
	    do_print=TRUE
	fi
    fi

    if test $physical_mem_free_mb -lt $Physical_mem_free_warn ; then
	if test $Need_both_warn = FALSE; then
	    do_print=TRUE
	else
	    if test $warn_total = TRUE ; then
		do_print=TRUE
	    fi
	fi
    fi

    # Print

    if test $do_print = TRUE ; then

	    #       Print header
	    now=`date -u`
	    echo "======= Time: $now"
	    echo "        Total memory free: $total_mem_free_mb MB"
	    echo "        Physical memory free: $physical_mem_free_mb MB"

	    # Now we are going to mess with the output so we can get just the
	    # header from 'top' but print the actual processes using 'ps'
	    # because we want the processes that are using memory NOT cpu and I
	    # do not know how to get that from 'top' in non-interactive mode

	    actual_top_header=7
	    chop_top_header=5
	    lines_from_ps=`expr $Lines_from_top - $actual_top_header`

	    # Print the header from 'top'
	    
	    top -n 1 -b | head -$actual_top_header | tail -$chop_top_header

	    # Print the processes sorted by memory usage (Thanks to Tor for this
	    # slick trick of sorting+awk+sed):
	    ## ps auxww | sed -e 1d | awk '{ print $4 " " $0 }' | sort -nr | head -$lines_from_ps

	    echo " "
	    echo "Processes sorted by memory usage in column 4..."
	    ps auxww | sort -nrk 4 | head -$lines_from_ps

	    # Check for zombies
	    if test $Check_zombies = TRUE ; then
		echo "---- Checking for zombies ----"
		ps auxww | grep defunct
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
