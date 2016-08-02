# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/18 22:17:29 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for file utilities
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Nov 1994
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libtitan.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS =

SRCS = \
	GateData.c \
	RfHandle.c \
	RfLabels.c \
	RfPartialTrack.c \
	RfPrintRadar.c \
	RfPrintStorm.c \
	RfPrintTrack.c \
	RfScanTable.c \
	RfStorm.c \
	RfTables.c \
	RfTrack.c \
	RfUncompress.c \
	RfUtilities.c \
	RfZr.c

CPPC_SRCS = \
	TitanStormFile.cc \
	TitanTrackFile.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
