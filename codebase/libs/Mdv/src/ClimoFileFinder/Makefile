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
# Makefile for ClimoFileFinder module of Mdv library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# December 2004
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libMdv.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS = \
	../include/Mdv/climo/DailyByYearFileFinder.hh \
	../include/Mdv/climo/DailyFileFinder.hh \
	../include/Mdv/climo/ExternalDiurnalFileFinder.hh \
	../include/Mdv/climo/FileFinder.hh \
	../include/Mdv/climo/HourlyByMonthFileFinder.hh \
	../include/Mdv/climo/HourlyByYearFileFinder.hh \
	../include/Mdv/climo/HourlyDiurnalFileFinder.hh \
	../include/Mdv/climo/HourlyFileFinder.hh \
	../include/Mdv/climo/MonthlyByYearFileFinder.hh \
	../include/Mdv/climo/MonthlyFileFinder.hh \
	../include/Mdv/climo/TwoHourlyByMonthFileFinder.hh \
	../include/Mdv/climo/TwoHourlyByYearFileFinder.hh \
	../include/Mdv/climo/TwoHourlyDiurnalFileFinder.hh \
	../include/Mdv/climo/ThreeHourlyByMonthFileFinder.hh \
	../include/Mdv/climo/ThreeHourlyDiurnalFileFinder.hh

CPPC_SRCS = \
	DailyByYearFileFinder.cc \
	DailyFileFinder.cc \
	ClimoFileFinder.cc \
	ExternalDiurnalFileFinder.cc \
	HourlyByMonthFileFinder.cc \
	HourlyByYearFileFinder.cc \
	HourlyDiurnalFileFinder.cc \
	HourlyFileFinder.cc \
	MonthlyByYearFileFinder.cc \
	MonthlyFileFinder.cc \
	TwoHourlyByMonthFileFinder.cc \
	TwoHourlyByYearFileFinder.cc \
	TwoHourlyDiurnalFileFinder.cc \
	ThreeHourlyByMonthFileFinder.cc \
	ThreeHourlyDiurnalFileFinder.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
