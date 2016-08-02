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
# Makefile for cdata_util library
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# March 1993
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =
LOC_CPPC_CFLAGS =
# LOC_CPPC_CFLAGS = -DDEBUG

TARGET_FILE = ../librapformats.a

#
# file lists
#

HDRS = 

C_SRCS = \
	station_reports.c \
	print_metar_data.c \
	decoded_metar_to_report.c

CPPC_SRCS = \
	HydroStation.cc \
	SsiFile.cc \
	station_file.cc \
	StationData.cc \
	WxObs.cc \
	WxObsField.cc \
	WxTypeField.cc \
	TaiwanAwos.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
