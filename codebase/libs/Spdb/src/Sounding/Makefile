# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:33 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libSpdb.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS = \
	../include/Spdb/sounding.h \
	../include/Spdb/Sounding.hh \
	../include/Spdb/SoundingGet.hh \
	../include/Spdb/SoundingPut.hh


C_SRCS =

CPPC_SRCS = \
	Sounding.cc \
	SoundingUtils.cc \
	SoundingGet.cc \
	SoundingPut.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
