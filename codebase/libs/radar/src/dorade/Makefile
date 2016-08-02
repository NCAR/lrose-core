###########################################################################
#
# Makefile for dorade portion of the radar lib
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# Jan 2008
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include -I../include/radar
LOC_CFLAGS =

TARGET_FILE = ../libradar.a

#
# file lists
#

HDRS =

C_SRCS = \
	spol_C_utils.c

CPPC_SRCS = \
	DFileInput.cc \
	dd_mapper.cc \
	dd_sweepfiles.cc \
	dd_testify.cc \
	dd_utils.cc \
	kdp_calcs.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
