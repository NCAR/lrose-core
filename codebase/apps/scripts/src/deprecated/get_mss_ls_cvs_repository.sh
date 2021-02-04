#!/bin/sh
# 
# Name:	get_mss_ls_cvs_repository.sh 
#
# Purpose: Bourne shell script to get a listing of the RAP CVS repository
#          data files currently residing on the NCAR Mass Storage System.
#
# Dependencies: You must have access to:
#		1) MSS system as a valid user
#		2) 'nrnet' command to access MSS
#
# Author: Deirdre Garvey	18-SEP-1998	NCAR/RAP
#
# From a script 'get_merlin_mss_ls.sh'.
#
#=============================================================

# Constants

MSS_DIR=/RAPDMG/CVS			  # MSS target directory
MSLS_OPTIONS=RlF                          # msls options, l=long format,
					  #   F=put '/' after directories
# Local variables

# Parse command line

if test $# -lt 1 ; then
    $0 -h
    exit 1
fi

for var in $@ ; do
    case $var in
	-h) echo "Usage: $0 outfile [-h]"
	    echo "       outfile:   Output filename for returned directory listing"
	    echo "       -h:        Print this usage message"
	    echo " "
	    echo "Purpose: Get a listing of the current RAP CVS repository"
	    echo "  files on the NCAR Mass Storage System. The results of"
	    echo "  the listing will be placed in the specified outfile ON VIRGA."
	    echo "Note: You must have access to the nrnet command and MSS to"
	    echo "   use $0."
	    echo " "
	    exit 1 ;;
    esac
done

Outfile=$1

# Header

echo "===== Getting a list of RAP CVS MSS files into file: ${Outfile} ====="

# Send the command

nrnet msls ${Outfile} r flnm=${MSS_DIR} opts=${MSLS_OPTIONS}

# Done

echo "Exiting $0..."

exit 0
