# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/6 21:37:21 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for RefFile module of the Refract library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# January 2009
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_INC_DIR = ../include
LOC_CPPC_CFLAGS =

TARGET_FILE = ../libRefract.a
MODULE_TYPE = library

#
# file lists
#

HDRS = \
	$(LROSE_INSTALL_DIR)/include/Refract/RefFile.hh

CPPC_SRCS = \
	RefFile.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
