# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:40 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for grid classes
#
# Terri Betancourt, RAP, NCAR, Boulder, Co, USA, 80307
#
# Jan 1998
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_INC_DIR = ../include
MY_CPPCHECKFLAGS = -exclude-rules=4.16
LOC_CFLAGS =

TARGET_FILE = ../libeuclid.a
MODULE_TYPE=library

#
# file lists
#

HDRS = \
	../include/euclid/Box.hh \
	../include/euclid/GridAlgs.hh \
	../include/euclid/GridExpand.hh \
	../include/euclid/GridExpandX.hh \
	../include/euclid/Grid2d.hh \
	../include/euclid/Grid2dClump.hh \
	../include/euclid/Grid2dDistToNonMissing.hh \
	../include/euclid/Grid2dEdgeBuilder.hh \
	../include/euclid/Grid2dInside.hh \
	../include/euclid/Grid2dLoop.hh \
	../include/euclid/Grid2dLoopA.hh \
	../include/euclid/Grid2dLoopAlg.hh \
	../include/euclid/Grid2dMedian.hh \
	../include/euclid/Grid2dOffset.hh \
	../include/euclid/Grid2dPolyFinder.hh

CPPC_SRCS = \
	Box.cc \
	GridAlgs.cc \
	GridExpand.cc \
	GridExpandX.cc \
	Grid2d.cc \
	Grid2dClump.cc \
	Grid2dDistToNonMissing.cc \
	Grid2dEdgeBuilder.cc \
	Grid2dInside.cc \
	Grid2dLoop.cc \
	Grid2dLoopA.cc \
	Grid2dLoopAlg.cc \
	Grid2dMedian.cc \
	Grid2dOffset.cc \
	Grid2dPolyFinder.cc


#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
