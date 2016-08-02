# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/7 19:48:42 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for contour module of contour library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# November 2000
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
	../include/contour/Contour.hh \
	../include/contour/ContourLevel.hh \
	../include/contour/ContourPoint.hh \
	../include/contour/ContourPolyline.hh

CPPC_SRCS = \
	Contour.cc \
	ContourLevel.cc \
	ContourPoint.cc \
	ContourPolyline.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
