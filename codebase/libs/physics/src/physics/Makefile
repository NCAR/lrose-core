# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:43 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for physics library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# March 1996
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS = 

TARGET_FILE = ../libphysics.a

#
# file lists
#

HDRS = \
	$(INCDIR)/physics/AdiabatTempLookupTable.hh \
	$(INCDIR)/physics/PhysicsLib.hh \
	$(INCDIR)/physics/IcaoStdAtmos.hh \
	$(INCDIR)/physics/density.h \
	$(INCDIR)/physics/physics.h \
	$(INCDIR)/physics/thermo.h \
	$(INCDIR)/physics/CapeCinSounding.hh

SRCS = \
	density.c \
	e_sub_s.c \
	humidity.c \
	lifted_index.c \
	stn_pressure.c \
	thermo.c \
	wind.c

CPPC_SRCS = \
	AdiabatTempLookupTable.cc \
	CapeCin.cc \
        CapeCinSounding.cc \
	IcaoStdAtmos.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_opt_lib_module_targets
#include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
