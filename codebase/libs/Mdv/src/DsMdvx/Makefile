# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:31 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
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
LOC_CFLAGS =

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
