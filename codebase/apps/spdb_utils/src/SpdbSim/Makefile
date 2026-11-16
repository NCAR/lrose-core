include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = SpdbSim

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = \
	-lSpdb -lrapformats -ldsserver -ldidss -leuclid -lrapmath -ltoolsa \
	-ldataport -ltdrp -lphysics -lpthread $(NETCDF4_LIBS) -lbz2 -lz

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

HDRS = \
	$(PARAMS_HH) \
	SpdbSim.hh \
	PayloadRetimer.hh \
	Args.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	SpdbSim.cc \
	PayloadRetimer.cc \
	Args.cc \
	Main.cc

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

# Parallel builds must generate Params.hh before compiling its consumers.
Args.o Main.o SpdbSim.o: $(PARAMS_HH)

# DO NOT DELETE THIS LINE -- make depend depends on it.

