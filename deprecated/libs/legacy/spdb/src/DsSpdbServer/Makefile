###########################################################################
#
# Makefile for DsSpdbServer
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# March 1999
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libspdb.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS = \
	$(RAP_INC_DIR)/dsserver/DsSpdbServer.hh

CPPC_SRCS = \
	DsSpdbServer.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
