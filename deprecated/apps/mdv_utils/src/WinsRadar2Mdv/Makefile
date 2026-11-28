###########################################################################
#
# Makefile for WinsRadar2Mdv program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 2000
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = WinsRadar2Mdv

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lMdv -lRadx -lNcxx -leuclid -ldsserver \
	-lrapformats -ldidss -ltoolsa -lpthread -ldataport \
	-ltdrp $(NETCDF4_LIBS) -lbz2 -lz \
	-lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

MODULE_TYPE = progcpp

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	OutputFile.hh \
	WinsRadar2Mdv.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	Main.cc \
	OutputFile.cc \
	WinsRadar2Mdv.cc

#
# tdrp macros
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard C++ targets
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

