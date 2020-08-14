###########################################################################
#
# Makefile for FiltAlg
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INC_DIR = ../include
MY_CPPCHECKFLAGS = -E FiltAlgParams.cc -E ../include/FiltAlg/FiltAlgParams.hh -E FiltAlgParams.hh 

LOC_CPPC_CFLAGS = -I$(LOC_INC_DIR) -Wall 
LOC_CFLAGS = $(LOC_CPPC_CFLAGS)

MODULE_TYPE=library
TARGET_FILE= ../libFiltAlg.a
HDRS = 

CPPC_SRCS = \
	FiltAlgParams.cc \
	FiltAlgParms.cc \
	Algorithm.cc  \
	BasicInfo.cc \
	Boundaries.cc \
	Comb.cc \
	CombineData.cc \
	Data1d.cc \
	Data2d.cc \
	Data.cc \
	FiltCreate.cc \
	Filter.cc \
	Filt2d.cc \
	Filt2dNoOverlap.cc \
	FiltClump.cc \
	FiltCombine.cc \
	FiltDB.cc \
	FiltInfo.cc \
	FiltInfoInput.cc \
	FiltInfoOutput.cc \
	FiltMaxTrue.cc \
	FiltMask.cc \
	FiltMedian.cc \
	FiltMedianNoOverlap.cc \
	FiltPassThrough.cc \
	FiltRemap.cc \
	FiltRescale.cc \
	FiltReplace.cc \
	FiltScalar.cc \
	FiltSRemap.cc \
	FiltTrapRemap.cc \
	FiltVertComb.cc \
        Find.cc \
	FindSimple.cc \
	Fuzzy.cc \
	GridExpand.cc \
	GridProj.cc \
	Info.cc \
	InterfaceFiltAlgParm.cc \
	Looper.cc \
	Statics.cc \
	VlevelData.cc \
	VlevelSlice.cc


# Filter for which filter() is not const

#
# comment this out as it does stuff not in CVS but leave it around for Dave Al.
# include html_targets
#

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

# local targets

FiltAlgParams.cc: paramdef.FiltAlg
	tdrp_gen -f paramdef.FiltAlg -c++ -class FiltAlgParams -lib FiltAlg -add_ncar_copyright
	/bin/mv -f FiltAlgParams.hh ../include/FiltAlg

/bin/mv clean_tdrp:
	$(RM) FiltAlgParams.cc ../include/FiltAlg/FiltAlgParams.hh

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


