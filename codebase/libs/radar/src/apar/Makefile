###########################################################################
#
# Makefile for apar time series
#
# Mike Dixon, EOL, NCAR, Boulder, Co, USA, 80307
#
# July 2019
#
###########################################################################

include $(APAR_MAKE_INC_DIR)/apar_make_macros

TARGET_FILE = ../libapardata.a

LOC_INCLUDES = -I../include -I$(LROSE_INSTALL_DIR)/include
LOC_CFLAGS =

HDRS = \
	../include/apardata/apar_ts_data.h \
	../include/apardata/apar_ts_functions.hh

CPPC_SRCS = \
	apar_ts_functions.cc \
	AparTsCalib.cc \
	AparTsInfo.cc \
	AparTsPulse.cc \
	AparTsReader.cc

#
# general targets
#

include $(APAR_MAKE_INC_DIR)/apar_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
