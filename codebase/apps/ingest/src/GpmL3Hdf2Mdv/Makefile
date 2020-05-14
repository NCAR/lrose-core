###########################################################################
#
# Makefile for TrmmV7Hdf2Mdv
#
# Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
#
# Oct 2008
#
###########################################################################

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

MODULE_TYPE=progcpp

TARGET_FILE = GpmL3Hdf2Mdv 

LOC_INCLUDES = $(HDF5_INCLUDES) $(NETCDF4_INCS)

LOC_LIBS = -ldsdata -lMdv -lRadx -lNcxx -lSpdb \
	-ldsserver -ldidss -lrapformats -leuclid \
	-lphysics -ltoolsa -lpthread -ldataport -ltdrp \
	$(HDF5_LIBS) $(NETCDF4_LIBS) -lbz2 -lz

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
# standard targets
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_targets

#
# local targets
#

depend: depend_generic

#
# TDRP support
#

Params.cc: paramdef.$(TARGET_FILE)
	tdrp_gen -f paramdef.$(TARGET_FILE) -c++ \
		-prog $(TARGET_FILE)

clean_tdrp:
	$(RM) Params.hh Params.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.

