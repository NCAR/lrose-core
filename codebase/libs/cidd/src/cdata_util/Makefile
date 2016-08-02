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

LOC_INCLUDES = -I../include
LOC_CFLAGS = -g

TARGET_FILE = ../libcidd.a

#
# file lists
#

HDRS = $(INCDIR)/cidd/cdata_util.h

SRCS = \
	binary_rep.c \
	byte_order.c \
	cdata_util.c \
	cidd_files.c \
	compress.c \
	daemon.c \
	ieee_native.c \
	file_index.c \
	latlon2xy.c \
	regist_serv.c

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
