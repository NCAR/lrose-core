# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/18 17:44:55 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for Mdv module of the rapformats library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# January 1997
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS = 
LOC_CPPFLAGS = 

TARGET_FILE = ../libmdv.a

HDRS = \
	$(INCDIR)/rapformats/Mdv.h

CPPC_SRCS = \
	MdvDump.cc \
	MdvPrivate.cc \
	MdvPublic.cc \
	MdvReadFile.cc \
	MdvFieldData.cc \
	MdvGrid.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
