#!/bin/sh --
#
# simple_src_build.sh
#
# Shell script to build and install RAL source code. Assumes that
# the source code already is checked out or staged.
#
# This is intentionally written in Bourne Shell to be as lightweight
# as possible.
# 
#=====================================================================
#
############### Defaults #######################
#
# Set defaults
#
Exit_success=1
Exit_failure=0

# Set command line defaults

Top_src_dir=/top/src
Top_install_dir=/top/build
Debug=FALSE
HOST_OS=LINUX

###########  Command line parsing ################

# Parse command line

while getopts dhs:i:o: option
do
    case "$option" in
         h) echo "Usage: $0 [-h] [-d] [-i <install-dir>] [-s <src-dir>] [-o <host_os>]"
	    echo "        Simple build script to compile and install RAL source"
	    echo "        code in the <src-dir> and install into the <install-dir>"
	    echo "        The source must alreay exist in the <src-dir>"
	    echo " -d               : Print debug messages"
	    echo " -h               : Print this usage message"
	    echo " -i <install-dir> : Top-level install dir. Default: $Top_install_dir"
	    echo " -o <host-os>     : HOST_OS for RAL Makefile system. Default: $HOST_OS"
	    echo " -s <src-dir>     : Top-level src dir. Default: $Top_src_dir"
            exit $Exit_failure
            ;;
        d) Debug=TRUE;
	    ;;
	i) Top_install_dir=$OPTARG
	    ;;
	o) HOST_OS=$OPTARG
	    ;;
	s) Top_src_dir=$OPTARG
	   ;;
    esac
done

############### Main #######################
#
# Set environment, this is very important for builds
#

RAP_MAKE_INC_DIR=$Top_src_dir/make_include
RAP_INC_DIR=$Top_install_dir/include
RAP_SHARED_INC_DIR=$RAP_INC_DIR
RAP_LIB_DIR=$Top_install_dir/lib
RAP_SHARED_LIB_DIR=$RAP_LIB_DIR
RAP_BIN_DIR=$Top_install_dir/bin
RAP_SHARED_BIN_DIR=$RAP_BIN_DIR

if test $Debug = TRUE ; then
    echo "Running $0 with environment..."
    echo "   HOST_OS: $HOST_OS"
    echo "   RAP_MAKE_INC_DIR: $RAP_MAKE_INC_DIR"
    echo "   RAP_INC_DIR: $RAP_INC_DIR"
    echo "   RAP_LIB_DIR: $RAP_LIB_DIR"
    echo "   RAP_BIN_DIR: $RAP_BIN_DIR"
fi

# Build tdrp_gen first if it exists

tdrp_dir=$Top_src_dir/apps/tdrp_gen/src/tdrp_gen
tdrp_lib_dir=$Top_src_dir/libs/tdrp
if test -d $tdrp_dir ; then
    if test $Debug = TRUE ; then
	echo "$0: Building tdrp_gen first"
    fi
    if test -d $tdrp_lib_dir ; then
	    cd $tdrp_lib_dir
	    make install
    fi
    cd $tdrp_dir
    make install
fi

# Build the rest

if test -d $Top_src_dir/incs ; then
    if test $Debug = TRUE ; then
	echo "$0: Installing the incs next"
    fi    
    cd $Top_src_dir/incs
    make install
fi

if test -d $Top_src_dir/libs ; then
    if test $Debug = TRUE ; then
	echo "$0: Building the libs next"
    fi
    cd $Top_src_dir/libs
    make install_include install
fi

if test -d $Top_src_dir/apps ; then
    if test $Debug = TRUE ; then
	echo "$0: Building the apps next"
    fi
    cd $Top_src_dir/apps
    make install
fi

# Done

exit $Exit_success
