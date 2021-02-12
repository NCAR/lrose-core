###########################################################################
#
# Makefile for TrmmV7Hdf2Mdv
#
# Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
#
# Oct 2008
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = GpmL3Hdf2Mdv 

LOC_INCLUDES = $(HDF5_INCLUDES) $(NETCDF4_INCS)

LOC_LIBS = -ldsdata -lMdv -lRadx -lNcxx -lSpdb \
	-ldsserver -ldidss -lrapformats -leuclid \
	-lphysics -ltoolsa -lpthread -ldataport -ltdrp \
	$(NETCDF4_LIBS) -lbz2 -lz

LOC_LDFLAGS = -L/usr/local/hdf5/lib -L/usr/local/szip/lib $(NETCDF4_LDFLAGS)

LOC_CFLAGS = 
LOC_CPPC_CFLAGS = 
HDRS = \
	Params.hh \
	Args.hh \
	GpmL3Hdf2Mdv.hh \
	HdfFile.hh

CPPC_SRCS = \
	Params.cc \
	Args.cc \
	Main.cc \
	GpmL3Hdf2Mdv.cc \
	HdfFile.cc

#
# tdrp macros
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

#
# tdrp targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

