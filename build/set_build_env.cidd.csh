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

setenv HOST_OS LINUX_64_CIDD32

setenv LROSE_CORE_DIR ~/cidd/lrose-core
setenv LROSE_INSTALL_DIR ~/cidd

setenv LROSE_MAKE_INC_DIR $LROSE_CORE_DIR/build/make_include
setenv LROSE_MAKE_BIN_DIR $LROSE_CORE_DIR/codebase/make_bin

setenv LROSE_INC_DIR $LROSE_INSTALL_DIR/include
setenv LROSE_LIB_DIR $LROSE_INSTALL_DIR/lib
setenv LROSE_BIN_DIR $LROSE_INSTALL_DIR/bin
setenv LROSE_MAN_DIR $LROSE_INSTALL_DIR/man
setenv LROSE_DOC_DIR $LROSE_INSTALL_DIR/doc

setenv LROSE_INST_LIB_DIR $LROSE_INSTALL_DIR/lib
setenv LROSE_INST_BIN_DIR $LROSE_INSTALL_DIR/bin

set path = ($LROSE_BIN_DIR $path)

