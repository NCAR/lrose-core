###########################################################################
#
# Makefile for ips support code
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
	../include/radar/ips_ts_data.h \
	../include/radar/ips_ts_functions.hh \
	../include/radar/IpsAltModeVel.hh \
	../include/radar/IpsGateData.hh \
	../include/radar/IpsMomFields.hh \
	../include/radar/IpsMoments.hh \
	../include/radar/IpsNoiseLocator.hh \
	../include/radar/IpsTsCalib.hh \
	../include/radar/IpsTsInfo.hh \
	../include/radar/IpsTsPulse.hh \
	../include/radar/IpsTsReader.hh \

CPPC_SRCS = \
	ips_ts_functions.cc \
	IpsAltModeVel.cc \
	IpsGateData.cc \
	IpsMomFields.cc \
	IpsMoments.cc \
	IpsNoiseLocator.cc \
	IpsTsCalib.cc \
	IpsTsInfo.cc \
	IpsTsPulse.cc \
	IpsTsReader.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
