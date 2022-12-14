###########################################################################
#
# Makefile for CwbRadar2Mdv program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 2000
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = CwbRadar2Mdv

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = \
	-lMdv -lrapformats -ldsserver -ldidss \
	-leuclid -lrapmath -ltoolsa -ldataport \
	-ltdrp -lRadx -lNcxx -lphysics \
	-lpthread $(NETCDF4_LIBS) -lbz2 -lz \
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


