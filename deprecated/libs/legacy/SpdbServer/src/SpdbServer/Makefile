###########################################################################
#
# Makefile for SpdbServer module of the SpdbServer library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# Sept 1996
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS = 
LOC_CPPC_CFLAGS = 

TARGET_FILE = ../libSpdbServer.a
MODULE_TYPE=library

HDRS = \
	$(INCDIR)/SpdbServer/SpdbServer.h \
	$(INCDIR)/SpdbServer/AcSpdbServer.h

CPPC_SRCS = \
	SpdbServer.cc \
	AcSpdbServer.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
