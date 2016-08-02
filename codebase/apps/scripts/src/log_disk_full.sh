#!/bin/sh
#
# Function: Script to check the specified disk on the current machine and
#           log when the usage is over a certain percent. This allows
#           tracking of what is contributing to the disk full problem
#           automatically. Optionally send email to a list of top disk users.
#
# Usage:    Expect to be run under cron
#
# Input:    output of 'df' and 'du -sk' on the current host
# 
# Output:   Logs contents of 'du -sk' when 'df' disk usage percent crosses
#           a specified value. Output is written to STDOUT. No output
#           is written until the disk full number is reached.
#
# Weaknesses: Only knows how to send email to the directory name in the
#             disk being monitored, if that is not a username then email
#             will not be sent to valid address.
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
Tmp_dir=/tmp

# Set command line defaults

Debug=FALSE
Monitor_disk=/home
Disk_warn_pct=95
Sleep_secs=60
Run_once=FALSE
Send_mail=FALSE
NUsers=5
Wait_nchecks=10

###########  Command line parsing ################

# Parse command line

while getopts d:hs:xp:vmn:w: option
do
    case "$option" in
         h) echo "Usage: $0 [-h] [-d disk] [-m] [-n nusers] [-p pct] [-s sleep_secs] [-x] [-v] [-w wait_checks]"
            echo "Purpose: To check a disk for more than ${Disk_warn_pct}% full"
	    echo "         Check the disk for usage and log. Optionally send"
	    echo "         email."
            echo " "
	    echo "  -d disk       : Disk to monitor."
	    echo "                  Default is $Monitor_disk"
            echo "  -h            : Print this usage message"
	    echo "  -m            : If disk is more than the specified pct full,"
	    echo "                  send mail to top $NUsers users."
	    echo "                  Default is NOT to send mail."
	    echo "  -n nusers     : If send mail, number of users to send mail to."
	    echo "                  Default is: $NUsers"
	    echo "  -p pct        : Specify the disk full percent to warn on, must be an integer"
	    echo "                  Default is $Disk_warn_pct"
	    echo "  -s sleep_secs : Specify the sleep seconds between checks."
	    echo "                  Default is $Sleep_secs"

            echo "  -x            : Run check once and exit."
            echo "                  Default is loop forever sleeping $Sleep_secs secs between checks"
	    echo "  -v            : Print debug messages"
	    echo "  -w wait_checks: If send mail, wait these number of check/sleep cycles"
	    echo "                  before sending mail again."
	    echo "                  Default is: $Wait_nchecks"
            exit $Exit_failure
            ;;
	d) Monitor_disk=$OPTARG
	    ;;
	m) Send_mail=TRUE
	    ;;
	n) NUsers=$OPTARG
	   Send_mail=TRUE
	    ;;
	p) Disk_warn_pct=$OPTARG
	    ;;
	s) Sleep_secs=$OPTARG
	    ;;
        x) Run_once=TRUE
            ;;
        v) Debug=TRUE
	    ;;
	w) Wait_nchecks=$OPTARG
	   Send_mail=TRUE
	    ;;
    esac
done

#
# Debug
#

if test $Debug = TRUE; then
    echo "Settings..."
    echo "  Monitor_disk: $Monitor_disk"
    echo "  Disk_warn_pct: $Disk_warn_pct"
    echo "  Sleep_secs: $Sleep_secs"
    echo "  Run_once: $Run_once"
    echo "  Send_mail: $Send_mail"
    echo "     NUsers: $NUsers"
    echo "     Wait_nchecks: $Wait_nchecks"
fi

#
# Start looping
#

Done=FALSE
nchecks=-1
do_send=FALSE
while [ $Done = FALSE ] ; do

    # Generate a timestamp

    timestamp=`date +%s`

    # Extract the %used for the disk and remove the percent sign (sed part)

    pct=`df -k $Monitor_disk | tail -1 $Tmp_file | awk '{print $4}' | sed 's/%//'`

    # Run the check on which users are using what

    if test $pct -ge $Disk_warn_pct ; then
    
	#       Print header
	now=`date -u`
	echo "======= Checking disk usage in $Monitor_disk at time: $now"
	df -h $Monitor_disk
	echo "==========================================================="

	#	Do the check and output to STDOUT
	cd ${Monitor_disk}
	du -sk * | sort -rn

	#       Send mail

	if test $Send_mail = TRUE; then

	    do_send=FALSE

	    # Have we waited enough loops to send mail?

	    if test $nchecks -lt 0 ; then
		do_send=TRUE
	    fi

	    if test $nchecks -ge $Wait_nchecks ; then
		do_send=TRUE
	    fi

	    # Okay, we need to send mail

	    if test $do_send = TRUE ; then

		# Generate the output list of disk usage. 
		# Create a file so can reuse it

		tmp_file=${Tmp_dir}/${timestamp}.du_output
		du -sk * | sort -rn | head -20 > $tmp_file

		# Generate the list of top N users

		user_list=`egrep --max-count $NUsers '^[0-9]' $tmp_file | awk '{print $2}'`
		
		# Generate the mail message text

		tmp_mail_file=${Tmp_dir}/${timestamp}.mail_msg

		echo "***** WARNING: ${Monitor_disk} is ${pct}% full. Please cleanup now!" > $tmp_mail_file
		cat $tmp_file >> $tmp_mail_file

		# Send email to the list of users

		for user in $user_list ; do
		    if test $Debug = TRUE ; then
			echo "Sending mail to: $user"
		    fi

		    mail $user -s "Warning: ${Monitor_disk} is ${pct}% full" < $tmp_mail_file
		done

		# Remove the temp files

		/bin/rm -f $tmp_file
		/bin/rm -f $tmp_mail_file

		# Reset the counter. Needs to be -1 rather than 0
		# so will be 0 after the increment at the end of the loop

		nchecks=-1

	    else

		# We are in-between checks.

		if test $Debug = TRUE; then
		    wait_checks=`expr ${Wait_nchecks} - 1`
		    echo "Skip sending email. On check number $nchecks. At check number ${wait_checks} will send email."
		fi

	    fi  #endif do_send=TRUE

	fi  #endif Send_mail=TRUE

	# Done with check

	cd
    fi

    # Sleep
    
    if test $Run_once = TRUE ; then
        Done=TRUE
    fi

    if test $Done = FALSE ; then
        sleep $Sleep_secs
    fi

    # Increment the number of checks made

    nchecks=`expr $nchecks + 1`

done # Done

exit $Exit_success