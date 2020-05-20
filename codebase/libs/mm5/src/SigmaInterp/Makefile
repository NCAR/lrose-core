# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2001 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Program(RAP) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2001/11/19 23:8:36 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for SigmaInterp class
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# March 2003
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libmm5.a

#
# file lists
#

HDRS = ../include/mm5/SigmaInterp.hh

CPPC_SRCS = \
	SigmaInterp.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_opt_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
