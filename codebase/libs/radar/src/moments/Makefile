###########################################################################
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Jan 2008
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include -I../include/radar $(NETCDF4_INCS)
LOC_CFLAGS =

TARGET_FILE = ../libradar.a

#
# file lists
#

HDRS = \
	../include/radar/AlternatingVelocity.hh \
	../include/radar/AtmosAtten.hh \
	../include/radar/ClutFilter.hh \
	../include/radar/ForsytheRegrFilter.hh \
	../include/radar/GateData.hh \
	../include/radar/MomentsFields.hh \
	../include/radar/InterestMap.hh \
	../include/radar/MatrixRegrFilter.hh \
	../include/radar/NoiseLocator.hh \
	../include/radar/RadarFft.hh \
	../include/radar/RadarMoments.hh \
	../include/radar/Sz864.hh

CPPC_SRCS = \
	AlternatingVelocity.cc \
	AtmosAtten.cc \
	ClutFilter.cc \
	ForsytheRegrFilter.cc \
	GateData.cc \
	MomentsFields.cc \
	InterestMap.cc \
	IwrfMoments.cc \
	IwrfMomReader.cc \
	MatrixRegrFilter.cc \
	NoiseLocator.cc \
	PhaseCoding.cc \
	RadarFft.cc \
	RadarMoments.cc \
	Sz864.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
