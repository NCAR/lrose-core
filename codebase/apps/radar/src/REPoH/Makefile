###########################################################################
#
# Makefile for RadiaFilt
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

MODULE_TYPE=progcpp
TARGET_FILE=REPoH
LOC_INCLUDES = -I. $(NETCDF4_INCS)

LOC_LIBS = -lFiltAlgVirtVol -lradar -lphysics \
	-ldsdata -lSpdb -lMdv -lRadx -lNcxx \
	-ldsserver -ldidss -lrapformats -leuclid \
	-lrapmath -ltoolsa -ldataport -ltdrp \
	$(NETCDF4_LIBS) -lbz2 -lz -lpthread \
	-lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_INC_DIR = .
MY_CPPCHECKFLAGS = -E Params.cc -E Params.hh -AllowPrivateNoDoxygen 

LOC_CFLAGS = 
LOC_CPPC_CFLAGS = 
HDRS = 

CPPC_SRCS = \
	RepohParams.cc  \
	HeaderParams.cc \
	AsciiOutputs.cc \
	AsciiOutput.cc \
	CloudGap.cc \
	CloudGaps.cc \
	ClumpRegions.cc \
	ClumpAssociate.cc \
	ClumpAssociate1.cc \
	FIRFilter.cc \
	Kernel.cc \
	KernelData.cc \
	KernelPoints.cc \
	Kernels.cc \
	KernelGrids.cc \
	KernelOutputs.cc \
	KernelOutput.cc \
	KernelPair.cc \
	KernelTemplate.cc \
	Main.cc \
	RayCloudEdge.cc \
	Repoh.cc \
	Parms.cc  \
	Volume.cc  \
	VolumeMdvInfo.cc \
	VolumeTrigger.cc \
	Sweep.cc

#
# Standard c++ targets
#
include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

HeaderParams.cc: paramdef.header
	tdrp_gen -f paramdef.header -c++ -class HeaderParams

RepohParams.hh: RepohParams.cc

RepohParams.cc: paramdef.REPoH
	tdrp_gen -f paramdef.REPoH -c++ -class RepohParams


#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.

