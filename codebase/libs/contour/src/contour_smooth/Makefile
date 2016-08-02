# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/7 19:48:45 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for contour_smooth module of contour library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# Sept 2004
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libcontour.a
MODULE_TYPE=library

#
# file lists
#

HDRS = \
	../include/contour/BinarySmoother.hh \
	../include/contour/ContourSmoother.hh \
	../include/contour/DouglasPeuckerSmoother.hh

CPPC_SRCS = \
	BinarySmoother.cc \
	ContourSmoother.cc \
	DouglasPeuckerSmoother.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
