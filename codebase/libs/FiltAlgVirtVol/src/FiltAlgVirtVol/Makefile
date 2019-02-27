###########################################################################
#
# Makefile for FiltAlg
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INC_DIR = ../include
MY_CPPCHECKFLAGS = -E AlgorithmParams.cc -E VirtVolParams.cc -E ../include/FiltAlgVirtVol/AlgorithmParams.hh -E ../include/FiltAlgVirtVol/VirtVolParams.hh  -E AlgorithmParams.hh -E VirtVolParams.hh  -AllowPrivateNoDoxygen

LOC_CPPC_CFLAGS = -I$(LOC_INC_DIR) -Wall 
LOC_CFLAGS = $(LOC_CPPC_CFLAGS)

MODULE_TYPE=library
TARGET_FILE= ../libFiltAlgVirtVol.a
HDRS = 

CPPC_SRCS = \
	Algorithm.cc  \
	AlgorithmParams.cc \
	AlgorithmParms.cc \
	FiltAlgParms.cc \
	GriddedData.cc \
	GridUserData.cc \
	InterfaceAlgorithm.cc \
	UrlSpec.cc \
	VirtVolParams.cc \
	VirtVolParms.cc \
	VirtVolSweep.cc \
	VirtVolVolume.cc \
	VolumeInfo.cc

#
# general targets
#
include $(RAP_MAKE_INC_DIR)/rap_make_targets

AlgorithmParams.hh: AlgorithmParams.cc
	$(CP) AlgorithmParams.hh ../include/FiltAlgVirtVol
AlgorithmParams.cc: paramdef.Algorithm
	tdrp_gen -f paramdef.Algorithm -c++ -class AlgorithmParams
	$(CP) AlgorithmParams.hh ../include/FiltAlgVirtVol


VirtVolParams.hh: VirtVolParams.cc
	$(CP) VirtVolParams.hh ../include/FiltAlgVirtVol
VirtVolParams.cc: paramdef.VirtVol
	tdrp_gen -f paramdef.VirtVol -c++ -class VirtVolParams
	$(CP) VirtVolParams.hh ../include/FiltAlgVirtVol

# DO NOT DELETE THIS LINE -- make depend depends on it.


