###########################################################################
#
# Makefile for iwrf
#
# Mike Dixon, RAL, NCAR, Boulder, Co, USA, 80307
#
# Feb 2009
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = ../libradar.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS = \
	../include/radar/iwrf_data.h

CPPC_SRCS = \
	chill_to_iwrf.cc \
	iwrf_functions.cc \
	IwrfCalib.cc \
	IwrfTsBurst.cc \
	IwrfTsGet.cc \
	IwrfTsInfo.cc \
	IwrfTsPulse.cc \
	IwrfTsReader.cc \
	rsm_functions.cc \

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

test: ttest.o
	gcc -I$(LROSE_INSTALL_DIR)/include -o ttest ttest.c

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
