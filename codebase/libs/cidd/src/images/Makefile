###########################################################################
#
# Makefile for cdata_util library
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# March 1993
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

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

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# special targets
#

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
