#!/bin/sh
# 
# Name:	save_cvs_repository_to_mss.sh 
#
# Purpose: Bourne shell script to (optionally) checkout, tar, 
#  	   and copy the contents of a CVS repository to the NCAR 
#	   Mass Storage System.
#
# Dependencies: You must have access to the following:
#               1) OWWS CVS Repository (if saving)
#		2) RAP CVS Repository (if saving)
#		3) /scratch
#		4) MSS system as a valid user
#		5) 'nrnet' command to access MSS
#
# Notes: I made a conscious decision to place tar files (but NOT
#	 compressed tar files) on the MSS. This has an implicit
#	 assumption that tar will be available in the future. 
#	 Since compression schemes do change (and may be subject
#	 to licensing, etc.) I decided NOT to save compressed
#	 files by default. An option to save the gzipped files is
#        available on the command line.
#
# Author: Deirdre Garvey	22-OCT-1997	NCAR/RAP
#
#=============================================================

# Constants

W_PASSWD=cvs					# MSS write password
RETAIN_NDAYS=4000				# MSS retention num of days
MSS_DIR=/RAPDMG/CVS				# MSS target directory

SCRATCH_DIR=/scratch/mss_cvs			# local temp directory
OWWS_CVS_CHECKOUT_DIR=${SCRATCH_DIR}/owws	# local OWWS temp directory
RAP_CVS_CHECKOUT_DIR=${SCRATCH_DIR}/rap		# local RAP temp directory

# Today's date (to put into tarfile name)

TARFILE_DATE=`date +%Y%b%d`			# tarfile date string

# Defaults for command line

Do_checkout=FALSE				# do 'cvs checkout'
Do_owws=FALSE					# save OWWS CVS repository
Do_rap=FALSE                                    # save RAP CVS repository
Do_zip=FALSE                                    # gzip the tar files
Do_split=FALSE                                  # split the RAP CVS tar files

# Parse command line

if test $# -lt 1 ; then
        echo "Usage: $0 -o|r|b [-c] [-h]"
	exit 1
fi

for var in $@ ; do

  case $var in 
	-h) echo "Usage: $0 -o|r|b [-c] [-h]"
	echo "      -b: to save both the OWWS and RAP CVS repositories"
	echo "      -c: to checkout the files from CVS then save. Default"
	echo "          is to save the repository itself"
	echo "      -h: to print this usage message"
	echo "      -o: to save the OWWS CVS repository"
	echo "      -r: to save the RAP  CVS repository"
	echo "      -s: split the RAP CVS files into apps and not apps"
	echo "      -z: to gzip the tar files"
	echo " Purpose is to save a CVS repository to the NCAR Mass Storage"
	echo " system. Note that building a tarfile of the RAP CVS repository"
	echo " will take ~6-8 hours to complete."
	exit 1 ;;
	-c) Do_checkout=TRUE ;;
	-o) Do_owws=TRUE
	    Do_rap=FALSE ;;
	-r) Do_owws=FALSE
	    Do_rap=TRUE ;;
	-b) Do_owws=TRUE; 
	    Do_rap=TRUE ;;
	-s) Do_split=TRUE ;;
	-z) Do_zip=TRUE ;;
	*) echo "Error, repository flag not specified"
	   echo "Must be [-o|r|b]"
	   echo "Exiting..."
	   exit 1
  esac
done

# create directories on scratch for tar files

mkdir -p $SCRATCH_DIR
mkdir -p $OWWS_CVS_CHECKOUT_DIR
mkdir -p $RAP_CVS_CHECKOUT_DIR

# ----- OWWS CVS Repository -----

if test ${Do_owws} = TRUE ; then

	OWWS_CVS_DIRS="applications config data doc fat include libraries make_bin make_include man startup static_data support test tools"

	# Set the CVS Root env var

	CVSROOT=/hk/local/cvsroot
	export CVSROOT

	# Get the files if doing a checkout

	if test ${Do_checkout} = TRUE ; then

		# checkout repository

		cd $OWWS_CVS_CHECKOUT_DIR
		cvs checkout $OWWS_CVS_DIRS
		OWWS_CVS_REP_DIRS=

		# set tarfile name

		owws_tarfile=owws_cvs_co_${TARFILE_DATE}.tar
	else
		cd ${CVSROOT}
		OWWS_CVS_REP_DIRS="CVSROOT"

		# set tarfile name

		owws_tarfile=owws_cvs_repository_${TARFILE_DATE}.tar
	fi

	# create tar file

	tar cvf ${SCRATCH_DIR}/${owws_tarfile} ${OWWS_CVS_DIRS} ${OWWS_CVS_REP_DIRS}

	# zip

	if test ${Do_zip} = TRUE ; then
		echo "gzipping the tarfile..."
		gzip ${SCRATCH_DIR}/${owws_tarfile}

		# Overload the filename so will be found by nrnet command

		owws_tarfile=${owws_tarfile}.gz
	fi

	# put tar file onto MSS

	cd ${SCRATCH_DIR}
	nrnet msput ${owws_tarfile} r w=${W_PASSWD} rtpd=${RETAIN_NDAYS} flnm=${MSS_DIR}/OWWS/${owws_tarfile}

fi

# ----- RAP CVS Repository -----

# create tar file

cd $RAP_CVS_CHECKOUT_DIR

if test ${Do_rap} = TRUE ; then

	# Set the CVS Root env var

	CVSROOT=/cvs
	export CVSROOT

	# May need to do 'apps' separately from the rest of the dirs since
	# the directory is so large (>250Mb) and takes so long to checkout
	# (>6h) that need to do this several pieces. The 'demo' area is mostly
	# data. 

	RAP_CVS_DIRS_0="awpg"
	RAP_CVS_DIRS_1="apps"
	RAP_CVS_DIRS_2="demo"
	RAP_CVS_DIRS_3="config distribs doc example_src htdocs incs java_packages libs make_bin make_include pat projects test utilities www"

##	21-SEP-1998: Due to permissions problems, cannot checkout jing_orig
##      or obsolete so remove them from the list
##	RAP_CVS_DIRS_3="config distribs doc example_src htdocs incs java_packages jing_orig libs make_bin make_include pat obsolete projects test utilities www"

	# handle split into multiple files
	#
	# Note that AWPG will be skipped since it is HUGE and hasn't 
	# changed in years. A copy of the AWPG file was put onto the 
	# MSS on 18-SEP-1998.

	if test ${Do_split} = TRUE ; then
		RAP_CVS_N_FILES="apps demo notapps"
	##	RAP_CVS_N_FILES="awpg apps demo notapps"	
	else
		RAP_CVS_N_FILES="all"
	fi

	# For each separate tar file from the repository, checkout the
	# files, build a tar file and send to MSS
	# Note that "all" skips AWPG since it is HUGE and hasn't changed
	# in years!

	for file in ${RAP_CVS_N_FILES} ; do
	
		case ${file} in
		awpg) rap_cvs_dirs=${RAP_CVS_DIRS_0} ;;
		apps) rap_cvs_dirs=${RAP_CVS_DIRS_1} ;;
		demo) rap_cvs_dirs=${RAP_CVS_DIRS_2} ;;
		notapps) rap_cvs_dirs=${RAP_CVS_DIRS_3} ;;
		all) rap_cvs_dirs="${RAP_CVS_DIRS_1} ${RAP_CVS_DIRS_2} ${RAP_CVS_DIRS_3}" ;;
		esac

		# are we checking out files from CVS?

		if test ${Do_checkout} = TRUE ; then

			# checkout the files

			cd ${RAP_CVS_CHECKOUT_DIR}
			cvs checkout ${rap_cvs_dirs}

			rap_tarfile=rap_cvs_co_${file}_${TARFILE_DATE}.tar
			rap_cvs_rep_dirs=

		else

			cd ${CVSROOT}
			rap_cvs_rep_dirs="CVSROOT"

			# set tarfile name

			rap_tarfile=rap_cvs_repository_${file}_${TARFILE_DATE}.tar
		fi #endif Do_checkout

		# build the tarfile

		tar cvf ${SCRATCH_DIR}/${rap_tarfile} ${rap_cvs_dirs} ${rap_cvs_rep_dirs}

		# zip the tarfile

		if test ${Do_zip} = TRUE ; then
			echo "gzipping the tarfile..."
			gzip ${SCRATCH_DIR}/${rap_tarfile}

			# Overload the filename so will be found by nrnet command
			rap_tarfile=${rap_tarfile}.gz
		fi

		# put the file to the MSS

		cd ${SCRATCH_DIR}
		nrnet msput ${rap_tarfile} r w=${W_PASSWD} rtpd=${RETAIN_NDAYS} flnm=${MSS_DIR}/RAP/${rap_tarfile}
		
	done  #endfor

fi #endif Do_rap

# Done

echo "Exiting $0..."

exit



