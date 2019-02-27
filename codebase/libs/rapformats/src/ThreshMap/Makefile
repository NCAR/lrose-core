# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:50 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
######################################################################
#
# Makefile for Windshear
#
#
######################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =
LOC_INC_DIR = ../include 

TARGET_FILE = ../librapformats.a

#
# file lists
#

HDRS = \
	../include/rapformats/FieldThresh.hh \
	../include/rapformats/FieldThresh2.hh \
	../include/rapformats/FieldThresh2WithGrid.hh \
	../include/rapformats/MultiThresh.hh \
	../include/rapformats/MultiThreshItem.hh \
	../include/rapformats/MultiThreshFcstBiasMapping.hh \
	../include/rapformats/MultiThresholdsBiasMapping.hh \
	../include/rapformats/TiledMultiThresh.hh \
	../include/rapformats/TileInfo.hh \
	../include/rapformats/TileLatLon.hh

CPPC_SRCS = \
	FieldThresh.cc \
	FieldThresh2.cc \
	FieldThresh2WithGrid.cc \
	MultiThresh.cc \
	MultiThreshFcstBiasMapping.cc \
	MultiThreshItem.cc \
	MultiThresholdsBiasMapping.cc \
	TiledMultiThresh.cc \
	TileInfo.cc \
	TileLatLon.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
