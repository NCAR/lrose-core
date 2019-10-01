###########################################################################
#
# Makefile for ReadEgm2008 program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Sept 1998
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ReadEgm2008

LOC_INCLUDES = -I../include
LOC_CFLAGS =
LOC_LDFLAGS = -L..
LOC_LIBS = -lradar -lMdv -lRadx -lNcxx -leuclid -ldsserver \
	-ldidss -lrapformats -ltoolsa -ldataport \
	-ltdrp $(NETCDF4_LIBS) -lbz2 -lz

HDRS = \
	$(PARAMS_HH) \
	Params.hh \
	ReadEgm2008.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	Main.cc \
	ReadEgm2008.cc

#
# tdrp macros
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# C++ targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

