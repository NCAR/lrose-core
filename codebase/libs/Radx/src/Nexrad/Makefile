# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:35 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for Nexrad modules for Radx
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Nov 2010
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = ../libRadx.a

LOC_INCLUDES = -I../include $(NETCDF4_INCS)
LOC_CFLAGS = 

HDRS = \
	../include/Radx/NexradCmdRadxFile.hh \
	../include/Radx/NexradData.hh \
	../include/Radx/NexradLoc.hh \
	../include/Radx/NexradRadxFile.hh \
	../include/Radx/NexradVcp.hh \
	../include/Radx/NidsRadxFile.hh

CPPC_SRCS = \
	NexradCmdRadxFile.cc \
	NexradData.cc \
	NexradLoc.cc \
	NexradRadxFile.cc \
	NexradVcp.cc \
	NidsData.cc \
	NidsRadxFile.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
