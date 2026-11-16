###########################################################################
#
# Makefile for RadxSim
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = RadxSim

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = \
	-ldsdata -lradar -lMdv -lSpdb \
	-lFmq -lrapformats -ldsserver -ldidss \
	-leuclid -lrapmath -ltoolsa -ldataport \
	-ltdrp -lRadx -lNcxx -lphysics \
	$(NETCDF4_LIBS) -lfftw3 -lbz2 -lz \
	-lpthread

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	$(PARAMS_HH) \
	Params.hh \
	Args.hh \
	RadxSim.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	Main.cc \
	RadxSim.cc

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

depend: depend_generic

Args.o Main.o RadxSim.o: $(PARAMS_HH)

# DO NOT DELETE THIS LINE -- make depend depends on it.

