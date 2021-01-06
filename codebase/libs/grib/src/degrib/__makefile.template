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

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libgrib.a
MODULE_TYPE=library

#
# file lists
#

HDRS = \
	../include/grib/BDS.hh \
	../include/grib/BMS.hh \
	../include/grib/ES.hh \
	../include/grib/EquidistantCylind.hh \
	../include/grib/PolarStereographic.hh \
        ../include/grib/GDS.hh \
        ../include/grib/gds_.hh \
	../include/grib/GribFile.hh \
	../include/grib/GribRecord.hh \
	../include/grib/gribrecord_.hh \
	../include/grib/GribSection.hh \
	../include/grib/GribVertType.hh \
	../include/grib/IdSec.hh \
	../include/grib/PDS.hh

CPPC_SRCS = \
	BDS.cc \
	BMS.cc \
	ES.cc \
	EquidistantCylind.cc \
	PolarStereographic.cc \
        GDS.cc \
        gds_.cc \
	GribFile.cc \
	GribRecord.cc \
	gribrecord_.cc \
	GribSection.cc \
	GribVertType.cc \
	IdSec.cc \
	PDS.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

