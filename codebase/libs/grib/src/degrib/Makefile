# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:29 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for degribbing code
#
# Mike Dixon/Jaimi Yee, RAP, NCAR, Boulder, Co, USA, 80307
#
# December 1998
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libgrib.a
MODULE_TYPE=library

#
# file lists
#

HDRS = \
	$(RAP_INC_DIR)/grib/BDS.hh \
	$(RAP_INC_DIR)/grib/BMS.hh \
	$(RAP_INC_DIR)/grib/ES.hh \
	$(RAP_INC_DIR)/grib/EquidistantCylind.hh \
	$(RAP_INC_DIR)/grib/PolarStereographic.hh \
        $(RAP_INC_DIR)/grib/GDS.hh \
        $(RAP_INC_DIR)/grib/gds.hh \
	$(RAP_INC_DIR)/grib/GribFile.hh \
	$(RAP_INC_DIR)/grib/GribRecord.hh \
	$(RAP_INC_DIR)/grib/gribrecord.hh \
	$(RAP_INC_DIR)/grib/GribSection.hh \
	$(RAP_INC_DIR)/grib/GribVertType.hh \
	$(RAP_INC_DIR)/grib/IdSec.hh \
	$(RAP_INC_DIR)/grib/PDS.hh

CPPC_SRCS = \
	BDS.cc \
	BMS.cc \
	ES.cc \
	EquidistantCylind.cc \
	PolarStereographic.cc \
        GDS.cc \
        gds.cc \
	GribFile.cc \
	GribRecord.cc \
	gribrecord.cc \
	GribSection.cc \
	GribVertType.cc \
	IdSec.cc \
	PDS.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

