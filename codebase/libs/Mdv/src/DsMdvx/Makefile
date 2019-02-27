# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# Copyright, University Corporation for Atmospheric Research (UCAR) 2009-2017. 
# The Government's right to use this data and/or software is restricted per 
# the terms of Cooperative Agreement between UCAR and the National  Science 
# Foundation, to government use only which includes the nonexclusive, 
# nontransferable, irrevocable, royalty-free license to exercise or have 
# exercised for or on behalf of the U.S. Government throughout the world. 
# All other rights are reserved. 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for DsMdvx
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Oct 1999
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libMdv.a

LOC_INCLUDES = -I../include $(NETCDF4_INCS)
MY_CFLAGS = -DNO_RADX_DATA

HDRS = \
	../include/Mdv/DsMdvx.hh \
	../include/Mdv/DsMdvxInput.hh \
	../include/Mdv/DsMdvxMsg.hh \
	../include/Mdv/DsMdvxThreaded.hh \
	../include/Mdv/DsMdvxTimes.hh

CPPC_SRCS = \
	DsMdvx.cc \
	DsMdvxInput.cc \
	DsMdvxMsg.cc \
	DsMdvxThreaded.cc \
	DsMdvxTimes.cc \
	msg_add.cc \
	msg_assemble.cc \
	msg_disassemble.cc \
	msg_get.cc \
	msg_print.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
