###########################################################################
#
# Makefile for iwrf
#
# Mike Dixon, RAL, NCAR, Boulder, Co, USA, 80307
#
# Feb 2009
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

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
	IwrfTsInfo.cc \
	IwrfTsPulse.cc \
	IwrfTsReader.cc \
	rsm_functions.cc \

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

test: ttest.o
	gcc -I$(RAP_INC_DIR) -o ttest ttest.c

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
