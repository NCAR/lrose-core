###########################################################################
#
# Makefile for hsrl
#
# Mike Dixon, EOL, NCAR, Boulder, Co, USA, 80307
#
# April 2017
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = ../libradar.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS = \
	../include/radar/HsrlRawRay.hh

CPPC_SRCS = \
	HsrlRawRay.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
