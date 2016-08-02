###########################################################################
#
# makefile for BeamBlock library routines from BOM code
#
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INC_DIR = ..
LOC_CPPC_CFLAGS = -I$(LOC_INC_DIR) -Wall -std=c++11
LOC_CFLAGS = $(LOC_CPPC_CFLAGS)
MODULE_TYPE=library
TARGET_FILE = ../libBeamBlock.a
LOC_INCLUDES = $(NETCDF4_INCS)

HDRS = \

CPPC_SRCS = \
        angle.cc \
	beam_power.cc \
	beam_propagation.cc \
	digital_elevation.cc \
	field.cc \
	latlon.cc \
	metadata.cc \
	spheroid.cc \
	string_utils.cc  \
	timestamp.cc \
	trace.cc \
	xml.cc

#
# general  targets
#
include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

# DO NOT DELETE THIS LINE -- make depend depends on it.

