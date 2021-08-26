###########################################################################
#
# Makefile for FiltAlg
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = $(NETCDF4_INCS)
LOC_INC_DIR = ../include
MY_CPPCHECKFLAGS = -E AlgorithmParams.cc -E VirtVolParams.cc -E UrlParams.cc -E ../include/FiltAlgVirtVol/AlgorithmParams.hh -E ../include/FiltAlgVirtVol/VirtVolParams.hh  -E ../include/FiltAlgVirtVol/UrlParams.hh  -E AlgorithmParams.hh -E VirtVolParams.hh  -E UrlParams.hh -AllowPrivateNoDoxygen

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
	Histo.cc \
	InterfaceAlgorithm.cc \
	LookupOffsets.cc \
	PolarCircularFilter.cc \
	PolarCircularTemplate.cc \
	ShapePolygons.cc \
	UrlParams.cc \
	UrlParms.cc \
	VertData2d.cc \
	VirtVolMdv.cc \
	VirtVolParams.cc \
	VirtVolParms.cc \
	VirtVolSweep.cc \
	VirtVolVolume.cc \
	VolumeInfo.cc \
	VolumeTime.cc

#	UrlSpec.cc \

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

# local targets

AlgorithmParams.cc: paramdef.Algorithm
	tdrp_gen -f paramdef.Algorithm -c++ -class AlgorithmParams -lib FiltAlgVirtVol -add_ncar_copyright
	/bin/mv -f AlgorithmParams.hh ../include/FiltAlgVirtVol

VirtVolParams.cc: paramdef.VirtVol
	tdrp_gen -f paramdef.VirtVol -c++ -class VirtVolParams -lib FiltAlgVirtVol -add_ncar_copyright
	/bin/mv -f VirtVolParams.hh ../include/FiltAlgVirtVol

UrlParams.hh: UrlParams.cc
	$(CP) UrlParams.hh ../include/FiltAlgVirtVol
UrlParams.cc: paramdef.Url
	tdrp_gen -f paramdef.Url -c++ -class UrlParams -lib FiltAlgVirtVol -add_ncar_copyright
	/bin/mv -f UrlParams.hh ../include/FiltAlgVirtVol

/bin/mv clean_tdrp:
	$(RM) AlgorithmParams.cc ../include/FiltAlgVirtVol/AlgorithmParams.hh
	$(RM) VirtVolParams.cc ../include/FiltAlgVirtVol/VirtVolParams.hh

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


