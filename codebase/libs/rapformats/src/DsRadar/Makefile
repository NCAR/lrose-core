# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:51 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for DsRadar
#
# Terri Betancourt, RAP, NCAR, Boulder, Co, USA, 80307
#
# April 1998
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../librapformats.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS = \
	../include/rapformats/DsRadar.h \
	../include/rapformats/DsRadarAz.hh \
	../include/rapformats/DsRadarCalib.hh \
	../include/rapformats/DsRadarElev.hh \
	../include/rapformats/DsRadarParams.hh \
	../include/rapformats/DsFieldParams.hh \
	../include/rapformats/DsRadarBeam.hh \
	../include/rapformats/DsRadarFlags.hh \
	../include/rapformats/DsRadarMsg.hh \
	../include/rapformats/DsRadarSweep.hh \
	../include/rapformats/DsPlatformGeoref.hh

C_SRCS = \
	ds_radar.c \
	ds_radar_calib.c \
	ds_radar_ts.c

CPPC_SRCS = \
	DsBeamData.cc \
	DsBeamDataFieldParms.cc \
	DsRadarAz.cc \
	DsRadarCalib.cc \
	DsRadarElev.cc \
	DsRadarParams.cc \
	DsFieldParams.cc \
	DsRadarBeam.cc \
	DsRadarFlags.cc \
	DsRadarMsg.cc \
	DsRadarPower.cc \
	DsRadarSweep.cc \
	DsPlatformGeoref.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
