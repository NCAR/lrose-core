# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/18 17:48:15 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for GTGIndices class
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Dec 1999
#
###########################################################################

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros


LOC_CFLAGS =
 
LOC_INCLUDES = -I../include

TARGET_FILE = ../libmm5.a

#
# file lists
#

HDRS = \
	../include/mm5/GTGIndices.hh

CPPC_SRCS = \
	GTGIndices.cc
 
#
# general targets
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_opt_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
