# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:39 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for polygon simplification routine
#
# Gerry Wiener, RAP, NCAR, Boulder, Co, USA, 80301
#
# September 2004
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =
LOC_LDFLAGS = -L..

TARGET_FILE = ../libeuclid.a
MODULE_TYPE=library

#
# file lists
#

HDRS = \
	$(LROSE_INSTALL_DIR)/include/euclid/DPbasic.hh

CPPC_SRCS = \
	DPbasic.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

basic: DPbasic_main.o
	$(CPPC) $(LOC_CPPC_CFLAGS) DPbasic_main.o $(LOC_LDFLAGS) -leuclid  -o basic



depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
