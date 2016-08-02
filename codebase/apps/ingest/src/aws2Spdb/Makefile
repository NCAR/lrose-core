###########################################################################
#
# Copyright (c) 1995, UCAR
# University Corporation for Atmospheric Research(UCAR)
# National Center for Atmospheric Research(NCAR)
# Research Applications Program(RAP)
# P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# All rights reserved. Licenced use only.
# Do not copy or distribute without authorization
# Tue 1 Jul 19103 21:20:6 UTC
#
# Makefile
#
# Niles Oien July 2003.
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = aws2Spdb

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lSpdb -ldsserver -ldidss -lrapformats \
	-lphysics -ltoolsa -ldataport -ltdrp \
	$(NETCDF4_LIBS) -lbz2 -lz -lpthread \
	-lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
        $(PARAMS_HH) \
	Args.hh \
	aws2Spdb.hh

CPPC_SRCS = \
        $(PARAMS_CC) \
	Main.cc \
	Args.cc \
	aws2Spdb.cc

#
# tdrp macros
#
 
include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard C++ targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp targets
#
 
include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.

