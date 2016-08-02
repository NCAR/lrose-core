#!/bin/sh
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 2000 - 2005 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Program(RAP) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2005/11/22 18:29:47 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#
# Purpose: Check whether can read all the files on a DVD/CD
#
#===========================================================================
#
# Set defaults

Exit_failure=-1
Exit_success=1
Today=`date -u`

# Set command line options and defaults

CdDvd_device="/dvd"
Nerrors=1
CheckNdays=FALSE
Ndays=-1
Debug=FALSE

# Parse command line

while getopts d:hn:x:v option
do
    case "$option" in
         h) echo "Usage: $0 [-h] [-d cd_dvd_device] [-n nerrors] [-v] [-x ndays]"
            echo "Purpose: To mount the cd_dvd_device and try to copy each file"
	    echo "         to /dev/null. If nerrors detected, exit. Optionally,"
	    echo "         if top directory on cd_dvd_device is older than ndays,"
	    echo "         exit."
	    echo " -d cd_dvd_device : CD or DVD device to mount, default: $CdDvd_device"
	    echo " -h               : Print this usage message"
	    echo " -n nerrors       : How many copy errors before exit this script"
	    echo "                    default: $Nerrors"
	    echo " -x ndays         : (Optional) Exit if $CdDvd_device top directory"
	    echo "                    is older than ndays"
	    echo " -v               : Print debug messages"
            exit $Exit_failure
            ;;
        d) CdDvd_device=$OPTARG
	    ;;
	n) Nerrors=$OPTARG
	    ;;
	x) Ndays=$OPTARG
	   CheckNdays=TRUE
	   ;;
	v) Debug=TRUE
	    ;;
   esac
done

# Print header

echo "--------- output from $0 -------------"
echo "Device: $CdDvd_device"
echo "$Today"

# Set loop defaults

exit_val=$Exit_success
error_count=0

# Read the DVD

mount $CdDvd_device
mount_status=$?
if test $mount_status -ne 0; then
    echo "***** ERROR: Cannot mount $CdDvd_device"
    exit $Exit_failure
fi

# If set, check the date on the top device directory and exit if
# too old

if test $CheckNdays = TRUE; then
    dirdate=`ls -dils /media/dvd | awk '{print $8}'`
    dir_udate=`date --date $dirdate +%s`
    today_udate=`date +%s`
    secs_in_day=86400
    secs_old=`expr $today_udate - $dir_udate`
    days_old=`expr ${secs_old} / ${secs_in_day}`
    if test $days_old -gt $Ndays ; then
	echo "***** ERROR: Data on $CdDvd_device is $days_old days old"
	echo "             This is more than $Ndays days old"
	umount $CdDvd_device
	exit $Exit_failure
    fi
fi

# Get the list of files, need to substitute all blanks in filenames
# with a different character or will have problems with for loop below

cd $CdDvd_device
files=`find . -type f -print | sed 's/ /%/g'`

# Loop through all the files on the DVD

nfiles=0
for file in $files; do

    # Increment the file counter

    nfiles=`expr $nfiles + 1`

    # Remove the special character

    real_file=`echo $file | sed 's/%/ /g'`
    
    if test $Debug = TRUE ; then
	echo $real_file
    fi
    
    # Copy the file to /dev/null

    cp "$real_file" /dev/null
    retval=$?

    # Count errors and exit if we've reached the max

    if test $retval = 1 ; then
	echo "***** COPY ERROR on $real_file"
	error_count=`expr $error_count + 1`
    fi

    if test $error_count -ge $Nerrors ; then
	echo "***** Maximum number of errors reached. Exiting"
	exit_val=$Exit_failure
	break
    fi
done

# Cleanup

cd
umount $CdDvd_device

# Done

echo "Number of files: $nfiles"
echo "Number of errors detected: $error_count"
exit $exit_val
