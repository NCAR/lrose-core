###########################################################################
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Jan 2008
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

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
	../include/radar/GateData.hh \
	../include/radar/MomentsFields.hh \
	../include/radar/InterestMap.hh \
	../include/radar/NoiseLocator.hh \
	../include/radar/RadarFft.hh \
	../include/radar/RadarMoments.hh \
	../include/radar/RegressionFilter.hh \
	../include/radar/Sz864.hh

CPPC_SRCS = \
	AlternatingVelocity.cc \
	AtmosAtten.cc \
	ClutFilter.cc \
	GateData.cc \
	MomentsFields.cc \
	InterestMap.cc \
	IwrfMoments.cc \
	IwrfMomReader.cc \
	NoiseLocator.cc \
	PhaseCoding.cc \
	RadarFft.cc \
	RadarMoments.cc \
	RegressionFilter.cc \
	Sz864.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
