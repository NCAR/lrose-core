# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/18 17:44:31 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for portability module
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Nov 1994
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CPPC_CFLAGS =

TARGET_FILE = ../libmdv.a
MODULE_TYPE=library

#
# file lists
#

HDRS = \
	../include/mdv/MdvChunk.hh \
	../include/mdv/MdvField.hh \
	../include/mdv/MdvFile.hh \
	../include/mdv/MdvMasterHdr.hh \
	../include/mdv/MdvVlevel.hh \
	../include/mdv/MdvVsection.hh \
	../include/mdv/MdvVsectionField.hh

CPPC_SRCS = \
	MdvChunk.cc \
	MdvField.cc \
	MdvFile.cc \
	MdvMasterHdr.cc \
	MdvVlevel.cc \
	MdvVsection.cc \
	MdvVsectionField.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
