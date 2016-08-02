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
# Purpose: Check whether the files were written successfully to a DVD/CD
#          by doing a checksum against the files used to write the DVD/CD
#
#===========================================================================
#
# Set defaults

Exit_failure=-1
Exit_success=1

symlinks_flag="-follow"   # Not currently used, here for reference
cksum_to_use=md5sum       # cksum app to use

# Set command line options and defaults

Source_dir=NOTSET
CdDvd_device=/dvd
Debug=FALSE
Nerrors=1

# Parse command line

while getopts d:hn:s:v option
do
    case "$option" in
         h) echo "Usage: $0 -s source_dir [-h] [-d cd_dvd_device] [-n nerrors] [-v]"
            echo "Purpose: To mount the cd_dvd_device and compare each file"
	    echo "         against the files in the source_dir."
	    echo "         If nerrors detected, exit."
	    echo " -d cd_dvd_device : CD or DVD device to mount, default: $CdDvd_device"
	    echo " -h               : Print this usage message"
	    echo " -n nerrors       : How many copy errors before exit this script"
	    echo "                    default: $Nerrors"
	    echo " -s source_dir    : (Required) Source directory to compare against"
	    echo "                    the cd_dvd_device."
	    echo " -v               : Print debug messages"
            exit $Exit_failure
            ;;
        d) CdDvd_device=$OPTARG
	    ;;
	n) Nerrors=$OPTARG
	    ;;
	s) Source_dir=$OPTARG
	    ;;
	v) Debug=TRUE
	    ;;
   esac
done

# Error check

if test $Source_dir = NOTSET ; then
    echo "***** ERROR: Source dir not set"
    exit $Exit_failure
fi

if test -d $Source_dir ; then
    echo "Okay, $Source_dir exists"
else
    echo "***** ERROR: Source dir does not exist: $Source_dir"
    exit $Exit_failure;
fi

# Set loop defaults

exit_val=$Exit_success
error_count=0

# Get a list of the files to compare

cd $Source_dir
files=`find . -type f -print`

# Mount the DVD device

mount $CdDvd_device
mount_status=$?
if test $mount_status -ne 0; then
    echo "***** ERROR: Cannot mount $CdDvd_device"
    exit $Exit_failure
fi

# Loop through the source and the DVD dir

for file in $files; do
    source_file=${Source_dir}/${file}
    dvd_file=${CdDvd_device}/${file}

    if test $Debug = TRUE ; then
	echo "source file: $source_file"
	echo "cd_dvd_file: $dvd_file"
    fi

    # Use awk to retrieve the desired number. cksum returns a CRC checksum
    # and a byte count on each file

    source_file_cksum=`${cksum_to_use} ${source_file} | awk '{print $1}'`
    dvd_file_cksum=`${cksum_to_use} ${dvd_file} | awk '{print $1}'`

    # Count errors and exit if we've reached the max

    if test $source_file_cksum != $dvd_file_cksum ; then
	echo "***** CD/DVD and source files do not match! $file"
	echo "      CD/DVD: $cd_dvd_cksum"
	echo "      source: $source_file_cksum"
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

echo "Number of errors detected: $error_count"
exit $exit_val