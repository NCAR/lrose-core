###########################################################################
#
# Makefile
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# June 2014
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libradar.a

#
# file lists
#

HDRS = \
	../include/radar/ConvStrat.hh

CPPC_SRCS = \
	ConvStrat.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
