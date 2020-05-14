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
# Makefile for MM5Data class
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Dec 1999
#
###########################################################################

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libmm5.a

#
# file lists
#

HDRS = \
	../include/mm5/MM5DataUtil.hh

CPPC_SRCS = \
	MM5DataUtil.cc

#
# general targets
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_opt_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
