# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:37 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for the DsTrigger module of the dsdata library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# October 2000
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libdsdata.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =
LOC_INC_DIR = ../include 
MY_CPPCHECKFLAGS = #-AddStyleFAA -exclude-rules="4.14,4.09,4.12,4.16"

HDRS = \
	../include/dsdata/DsDirListTrigger.hh \
	../include/dsdata/DsEnsembleAnyTrigger.hh \
	../include/dsdata/DsEnsembleDataTrigger.hh \
	../include/dsdata/DsEnsembleGenTrigger.hh \
	../include/dsdata/DsEnsembleLeadTrigger.hh \
	../include/dsdata/DsFcstTime.hh \
	../include/dsdata/DsFcstTimeListTrigger.hh \
	../include/dsdata/DsFileListTrigger.hh \
	../include/dsdata/DsFmqTrigger.hh \
	../include/dsdata/DsInputDirTrigger.hh \
	../include/dsdata/DsInputPathTrigger.hh \
	../include/dsdata/DsIntervalTrigger.hh \
	../include/dsdata/DsLdataIntTrigger.hh \
	../include/dsdata/DsLdataTrigger.hh \
	../include/dsdata/DsMultFcstTrigger.hh \
	../include/dsdata/DsMultInputDir.hh \
	../include/dsdata/DsMultipleTrigger.hh \
	../include/dsdata/DsMultTrigElem.hh \
	../include/dsdata/DsOneFileTrigger.hh \
	../include/dsdata/DsOneTimeTrigger.hh \
	../include/dsdata/DsSpecificFcstLdataTrigger.hh \
	../include/dsdata/DsSpecificFcstTimeListTrigger.hh \
	../include/dsdata/DsSpecificGenLdataTrigger.hh \
	../include/dsdata/DsTimeListTrigger.hh \
	../include/dsdata/DsTrigger.hh \
	../include/dsdata/MdvTimeListHandler.hh \
	../include/dsdata/DsUrlTrigger.hh \
	../include/dsdata/DsUrlTriggerArchive.hh \
	../include/dsdata/DsUrlTriggerRealtime.hh \
	../include/dsdata/DsUrlTriggerObject.hh \
	../include/dsdata/DsUrlTriggerObjectDerived.hh \
	../include/dsdata/DsUrlTriggerSubsample.hh \
	../include/dsdata/SpdbTimeListHandler.hh \
	../include/dsdata/TimeListHandler.hh \
	../include/dsdata/TriggerInfo.hh

CPPC_SRCS = \
	DsDirListTrigger.cc \
	DsEnsembleAnyTrigger.cc \
	DsEnsembleDataTrigger.cc \
	DsEnsembleGenTrigger.cc \
	DsEnsembleLeadTrigger.cc \
	DsFcstTime.cc \
	DsFcstTimeListTrigger.cc \
	DsFileListTrigger.cc \
	DsFmqTrigger.cc \
	DsInputDirTrigger.cc \
	DsInputPathTrigger.cc \
	DsIntervalTrigger.cc \
	DsLdataIntTrigger.cc \
	DsLdataTrigger.cc \
	DsMultFcstTrigger.cc \
	DsMultInputDirTrigger.cc \
	DsMultipleTrigger.cc \
	DsMultTrigElem.cc \
	DsOneFileTrigger.cc \
	DsOneTimeTrigger.cc \
	DsSpecificFcstLdataTrigger.cc \
	DsSpecificFcstTimeListTrigger.cc \
	DsSpecificGenLdataTrigger.cc \
	DsTimeListTrigger.cc \
	DsTrigger.cc \
	DsUrlTrigger.cc \
	DsUrlTriggerArchive.cc \
	DsUrlTriggerRealtime.cc \
	DsUrlTriggerObject.cc \
	DsUrlTriggerObjectDerived.cc \
	DsUrlTriggerSubsample.cc \
	MdvTimeListHandler.cc \
	SpdbTimeListHandler.cc \
	TimeListHandler.cc \
	TriggerInfo.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
