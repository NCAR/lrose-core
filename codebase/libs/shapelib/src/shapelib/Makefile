###########################################################################
#
# Makefile for the shapelib library.  Note that this makefile only makes
# the library.  Makefile.orig can be used to make the utility programs.
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# June 1000
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libshapelib.a
MODULE_TYPE=library

#
# file lists
#

HDRS = \
	$(LROSE_INSTALL_DIR)/include/shapelib/shapefil.h

SRCS = \
	shpopen.c \
	shptree.c \
	dbfopen.c

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
