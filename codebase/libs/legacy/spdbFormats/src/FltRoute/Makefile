###########################################################################
#
# Makefile for file SPDB FltRoute data module
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# August 1999
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libspdbFormats.a

#
# file lists
#

HDRS = \
	$(RAP_INC_DIR)/spdbFormats/FltRoute.hh \
	$(RAP_INC_DIR)/spdbFormats/PosnRpt.hh \
	$(RAP_INC_DIR)/spdbFormats/WayPoint.hh

CPPC_SRCS = \
	FltRoute.cc \
	PosnRpt.cc \
	WayPoint.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
