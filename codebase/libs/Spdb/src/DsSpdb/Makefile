# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:33 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for DsSpdb
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Dec 1999
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libSpdb.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS = \
	../include/Spdb/DsSpdb.hh \
	../include/Spdb/DsSpdbInput.hh \
	../include/Spdb/DsSpdbThreaded.hh \
	../include/Spdb/DsSpdbMsg.hh

CPPC_SRCS = \
	DsSpdb.cc \
	DsSpdbInput.cc \
	DsSpdbThreaded.cc \
	DsSpdbMsg.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
