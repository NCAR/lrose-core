# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:31 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for ClimoStatCalc module of Mdv library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# August 2004
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libMdv.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS = \
	../include/Mdv/climo/MaxDateStatCalc.hh \
	../include/Mdv/climo/MaxStatCalc.hh \
	../include/Mdv/climo/MeanStatCalc.hh \
	../include/Mdv/climo/MinDateStatCalc.hh \
	../include/Mdv/climo/MinStatCalc.hh \
	../include/Mdv/climo/NumObsGEStatCalc.hh \
	../include/Mdv/climo/NumObsGTStatCalc.hh \
	../include/Mdv/climo/NumObsLEStatCalc.hh \
	../include/Mdv/climo/NumObsLTStatCalc.hh \
	../include/Mdv/climo/NumObsStatCalc.hh \
	../include/Mdv/climo/NumTimesStatCalc.hh \
	../include/Mdv/climo/PercentObsGEStatCalc.hh \
	../include/Mdv/climo/PercentObsGTStatCalc.hh \
	../include/Mdv/climo/PercentObsLEStatCalc.hh \
	../include/Mdv/climo/PercentObsLTStatCalc.hh \
	../include/Mdv/climo/PercentObsStatCalc.hh \
	../include/Mdv/climo/StatCalc.hh \
	../include/Mdv/climo/StatNamer.hh \
	../include/Mdv/climo/StdDevStatCalc.hh

CPPC_SRCS = \
	MaxDateStatCalc.cc \
	MaxStatCalc.cc \
	MeanStatCalc.cc \
	MinDateStatCalc.cc \
	MinStatCalc.cc \
	NumObsGEStatCalc.cc \
	NumObsGTStatCalc.cc \
	NumObsLEStatCalc.cc \
	NumObsLTStatCalc.cc \
	NumObsStatCalc.cc \
	NumTimesStatCalc.cc \
	PercentObsGEStatCalc.cc \
	PercentObsGTStatCalc.cc \
	PercentObsLEStatCalc.cc \
	PercentObsLTStatCalc.cc \
	PercentObsStatCalc.cc \
	StatCalc.cc \
	StatNamer.cc \
	StdDevStatCalc.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
