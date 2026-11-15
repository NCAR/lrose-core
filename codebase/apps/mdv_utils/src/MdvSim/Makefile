include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = MdvSim

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = \
	-lMdv -lrapformats -ldsserver -ldidss \
	-leuclid -lrapmath -ltoolsa -ldataport \
	-ltdrp -lRadx -lNcxx -lphysics \
	-lpthread $(NETCDF4_LIBS) -lbz2 -lz

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

HDRS = \
	$(PARAMS_HH) \
	MdvSim.hh \
	Args.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	MdvSim.cc \
	Args.cc \
	Main.cc

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

# Parallel builds must finish generating Params.hh before compiling sources
# that include it; the generic TDRP rules only express this for Params.o.
Args.o Main.o MdvSim.o: $(PARAMS_HH)

# DO NOT DELETE THIS LINE -- make depend depends on it.
