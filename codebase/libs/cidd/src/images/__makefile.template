###########################################################################
#
# Makefile for cdata_util library
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# March 1993
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include $(SYS_X_INCLUDES)
LOC_CFLAGS = -g

TARGET_FILE = ../libcidd.a

#
# file lists
#

HDRS = $(INCDIR)/cidd/png_images.hh

CPPC_SRCS = \
	png_images.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# special targets
#

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
