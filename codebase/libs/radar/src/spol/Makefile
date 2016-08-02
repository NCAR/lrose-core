###########################################################################
#
# Makefile for spol
#
# Mike Dixon, RAL, NCAR, Boulder, Co, USA, 80307
#
# Feb 2011
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libradar.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS = \
	../include/radar/spol_angles.hh

CPPC_SRCS = \
	spol_angles.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
