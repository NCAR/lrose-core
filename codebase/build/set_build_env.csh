# Set environment variables for the RAP Makefile system
#
# for csh and tcsh
#
# Before sourcing, you need to set:
#
#   $LROSE_DIR
#
# to the top dir for installation.
#

setenv HOST_OS LINUX_64
uname -a | grep x86_64
if ($status == 1) then
    setenv HOST_OS LINUX
endif

setenv RAP_MAKE_INC_DIR $LROSE_DIR/make_include
setenv RAP_MAKE_BIN_DIR $LROSE_DIR/make_bin
setenv RAP_INC_DIR $LROSE_DIR/include
setenv RAP_LIB_DIR $LROSE_DIR/lib
setenv RAP_BIN_DIR $LROSE_DIR/bin
setenv RAP_MAN_DIR $LROSE_DIR/man
setenv RAP_DOC_DIR $LROSE_DIR/doc

setenv RAP_SHARED_INC_DIR $LROSE_DIR/include
setenv RAP_SHARED_LIB_DIR $LROSE_DIR/lib
setenv RAP_SHARED_BIN_DIR $LROSE_DIR/bin
setenv RAP_SHARED_MAN_DIR $LROSE_DIR/man
setenv RAP_SHARED_DOC_DIR $LROSE_DIR/doc

setenv RAP_INST_LIB_DIR $LROSE_DIR/lib
setenv RAP_INST_BIN_DIR $LROSE_DIR/bin

set path = ($RAP_BIN_DIR $path)

