# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2001 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Program(RAP) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2001/11/19 23:10:30 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for tdrp library
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Oct 1998
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libtdrp.a
MODULE_TYPE=library

HDRS = ../include/tdrp/tdrp.h \
	../include/tdrp/tdrpbuf.h \
	../include/tdrp/tdrp_p.h \
	../include/tdrp/tdrp_obsolete.h

SRCS = \
	check.c \
	convert.c \
	copy.c \
	find_item.c \
	load.c \
	mem.c \
	override.c \
	print.c \
	read_item.c \
	tdrp.c \
	tdrp_obsolete.c \
	tdrpbuf.c \
	token_list.c \
	tokenize.c \
	warn_on_extra_params.c



#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

clean_tdrp:

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


