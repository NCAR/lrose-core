###########################################################################
#
# Makefile for apar support code
#
# Mike Dixon, EOL, NCAR, Boulder, Co, USA, 80307
#
# July 2019
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = ../libradar.a

LOC_INCLUDES = -I../include -I$(LROSE_INSTALL_DIR)/include
LOC_CFLAGS =

HDRS = \
	../include/radar/apar_ts_data.h \
	../include/radar/apar_ts_functions.hh \
	../include/radar/AparAltModeVel.hh \
	../include/radar/AparGateData.hh \
	../include/radar/AparMomFields.hh \
	../include/radar/AparMoments.hh \
	../include/radar/AparNoiseLocator.hh \
	../include/radar/AparTsCalib.hh \
	../include/radar/AparTsInfo.hh \
	../include/radar/AparTsPulse.hh \
	../include/radar/AparTsReader.hh \

CPPC_SRCS = \
	apar_ts_functions.cc \
	AparAltModeVel.cc \
	AparGateData.cc \
	AparMomFields.cc \
	AparMoments.cc \
	AparNoiseLocator.cc \
	AparTsCalib.cc \
	AparTsInfo.cc \
	AparTsPulse.cc \
	AparTsReader.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
