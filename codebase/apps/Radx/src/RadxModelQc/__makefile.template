###########################################################################
#
# Makefile for RadiaFilt
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=progcpp
TARGET_FILE=RadxModelQc
MY_CPPCHECKFLAGS = -E Params.cc -E Params.hh
LOC_INC_DIR = .

LOC_INCLUDES = $(NETCDF4_INCS) -I.

LOC_LIBS = \
          -ldsdata -lradar -lFmq -lSpdb -lMdv  -ldsserver -ldidss \
          -lRadx -lNcxx -lrapformats -leuclid -lrapmath \
          -lphysics -ltoolsa -ldataport -ltdrp \
          $(NETCDF4_LIBS) \
          -lfftw3 -lbz2 -lz -lpthread

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

CPPC_SRCS = \
	Params.cc \
	HeaderParams.cc \
	Parms.cc \
	Main.cc \
	RadxModelQc.cc \
	AzGradientStateSpecialData.cc \
	AzGradientFilter.cc \
	Clutter2dQualFilter.cc \
	FIRFilter.cc \
	QscaleFilter.cc \
	RayData1.cc \
	Special0Filter.cc \
	Special1Filter.cc \
	Variance1dFilter.cc \
	Volume.cc

# RayUnaryFilter.cc \

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# Standard c++ targets
#
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

#
# tdrp targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

HeaderParams.cc: paramdef.header
	tdrp_gen -f paramdef.header -c++ -class HeaderParams

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

