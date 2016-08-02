# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:50 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
######################################################################
#
# Makefile for Windshear
#
#
######################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =
LOC_INC_DIR = ../include 

TARGET_FILE = ../librapformats.a

#
# file lists
#

HDRS = \
	../include/rapformats/WindshearArena.hh \
	../include/rapformats/Windshear.hh \
	../include/rapformats/WindshearAlpha.hh \
	../include/rapformats/WindshearAlphas.hh

CPPC_SRCS = \
	WindshearArena.cc \
	Windshear.cc \
	WindshearAlpha.cc \
	WindshearAlphas.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
