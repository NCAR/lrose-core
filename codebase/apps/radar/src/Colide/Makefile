###########################################################################
#
# Makefile for Colide
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = Colide
LOC_INCLUDES = -I. $(NETCDF4_INCS)

LOC_LIBS = -lFiltAlgVirtVol -lradar -lphysics \
	-ldsdata -lSpdb -lMdv -lRadx -lNcxx \
	-ldsserver -ldidss -lrapformats -leuclid \
	-lrapmath -ltoolsa -ldataport -ltdrp \
	$(NETCDF4_LIBS) -lbz2 -lz -lpthread \
	-lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_INC_DIR = .
MY_CPPCHECKFLAGS = -E Params.cc -E Params.hh -AddStyleFAA -exclude-rules="4.16,4.09,4.14,4.12"

LOC_CFLAGS = 
LOC_CPPC_CFLAGS = 

HDRS = 

CPPC_SRCS = \
	Params.cc \
	HeaderParams.cc \
	Bad.cc \
	Colide.cc \
	EllipHandler.cc \
	Elliptical.cc \
	Enhance.cc \
	EnhanceHandler.cc \
	EnhancementOffsetD.cc \
	EnhancementD.cc \
	LineDetection.cc \
	LineDetect.cc \
	LineDetectOffsets.cc \
	ShearDetection.cc \
	ShearDetect.cc \
	ShearDetectOffsets.cc \
	Main.cc \
	OldDataHandler.cc \
	Parms.cc \
	Reg.cc \
	RegAtt.cc \
	RegCombHandler.cc \
	RegHandler.cc \
	Sweep.cc \
	SVD.cc \
	Volume.cc \
	Window.cc

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

HeaderParams.cc: paramdef.header
	tdrp_gen -f paramdef.header -c++ -class HeaderParams

# DO NOT DELETE THIS LINE -- make depend depends on it.

