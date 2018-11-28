# Set environment variables for the RAP Makefile system
#
# for csh and tcsh
#
# Before sourcing, you need to set:
#
#   $LROSE_CORE_DIR
#   $LROSE_INSTALL_DIR
#
# to the top dir for installation.
#

# Complain and bail out if LROSE_CORE_DIR and LROSE_INSTALL_DIR are not both set
if ( ! $?LROSE_CORE_DIR || ! $?LROSE_INSTALL_DIR ) then
	echo "Environment variables LROSE_CORE_DIR and LROSE_INSTALL_DIR must be"
	echo "set before using this script!"
	exit
endif

setenv HOST_OS LINUX_LROSE
uname -a | grep x86_64
if ($status == 1) then
    setenv HOST_OS LINUX
endif

setenv RAP_MAKE_INC_DIR $LROSE_CORE_DIR/codebase/make_include
setenv RAP_MAKE_BIN_DIR $LROSE_CORE_DIR/codebase/make_bin

setenv RAP_INC_DIR $LROSE_INSTALL_DIR/include
setenv RAP_LIB_DIR $LROSE_INSTALL_DIR/lib
setenv RAP_BIN_DIR $LROSE_INSTALL_DIR/bin
setenv RAP_MAN_DIR $LROSE_INSTALL_DIR/man
setenv RAP_DOC_DIR $LROSE_INSTALL_DIR/doc

setenv RAP_SHARED_INC_DIR $LROSE_INSTALL_DIR/include
setenv RAP_SHARED_LIB_DIR $LROSE_INSTALL_DIR/lib
setenv RAP_SHARED_BIN_DIR $LROSE_INSTALL_DIR/bin
setenv RAP_SHARED_MAN_DIR $LROSE_INSTALL_DIR/man
setenv RAP_SHARED_DOC_DIR $LROSE_INSTALL_DIR/doc

setenv RAP_INST_LIB_DIR $LROSE_INSTALL_DIR/lib
setenv RAP_INST_BIN_DIR $LROSE_INSTALL_DIR/bin

set path = ($RAP_BIN_DIR $path)

