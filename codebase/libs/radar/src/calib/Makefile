###########################################################################
#
# Makefile for calibration
#
# Mike Dixon, EOL, NCAR, Boulder, Co, USA, 80307
#
# Oct 2018
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libradar.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS = \
	../include/radar/RadarCalib.hh

CPPC_SRCS = \
	RadarCalib.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
