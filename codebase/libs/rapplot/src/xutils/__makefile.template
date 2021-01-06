# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/18 21:3:43 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for X utilities module of the graphics utility library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# April 1997
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include $(SYS_X_INCLUDES)
LOC_CFLAGS =

TARGET_FILE = ../librapplot.a

HDRS = \
	../include/rapplot/xutils.h \
	../include/rapplot/rascon.h \
	../include/rapplot/xrs.h \
	../include/rapplot/xudr.h \
	../include/rapplot/X_Resource.hh

SRCS = \
	DumpXid.c \
	rascon.c \
	xrs.c \
	xudr.c

CC_SRCS = \
	X_Resource.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
