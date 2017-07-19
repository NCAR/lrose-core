###########################################################################
#
# Makefile for Spdb
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Jan 1999
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libspdb.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS = \
	$(RAP_INC_DIR)/dsserver/Spdb.hh \
	$(RAP_INC_DIR)/dsserver/SpdbMsg.hh \
	$(RAP_INC_DIR)/dsserver/SpdbPutArgs.hh

CPPC_SRCS = \
	Spdb.cc \
	SpdbMsg.cc \
	SpdbPutArgs.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
