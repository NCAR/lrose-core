# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:34 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for DsRadar
#
# Terri Betancourt, RAP, NCAR, Boulder, Co, USA, 80307
#
# April 1998
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libdidss.a

LOC_INCLUDES = -I../include
LOC_CFLAGS =

HDRS = \
	$(RAP_INC_DIR)/didss/ds_message.h \
	$(RAP_INC_DIR)/didss/ds_msg_handle.h \
	$(RAP_INC_DIR)/didss/DsMessage.hh \
	$(RAP_INC_DIR)/didss/DsMsgPart.hh

C_SRCS = \
	ds_message.c \
	ds_msg_handle.c

CPPC_SRCS = \
	DsMessage.cc \
	DsMsgPart.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
