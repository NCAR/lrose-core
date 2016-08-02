# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:31 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = Mdv

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	ClimoFileFinder \
	ClimoStatCalc \
	DsMdvx \
	Mdvx \
	MdvxChunk \
	MdvxContour \
	MdvxField \
	MdvxGrid \
	MdvxProj \
	MdvxRadar \
	MdvxRemapLut \
	MdvxTimeList \
	MdvxTimeStamp \
	MdvxUrlWatcher \
	MdvxVsectLut \
	oldMdv \
	NcfMdv \
	Radx

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets

include $(RAP_MAKE_INC_DIR)/rap_make_inc_targets

include $(RAP_MAKE_INC_DIR)/rap_make_lib_targets





