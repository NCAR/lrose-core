###########################################################################
#
# Makefile for CwbRadar2Mdv program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 2000
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = CwbRadar2Mdv

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lMdv -lRadx -lNcxx -leuclid -ldsserver \
	-lrapformats -ldidss -ltoolsa -ldataport \
	-ltdrp $(NETCDF4_LIBS) -lbz2 -lz \
	-lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	CwbRadar2Mdv.hh \
	InputFile.hh \
	OutputFile.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	CwbRadar2Mdv.cc \
	InputFile.cc \
	Main.cc \
	OutputFile.cc

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

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

