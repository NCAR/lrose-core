# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:32 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for Mdvx class
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# August 1999
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CPPC_CFLAGS =

TARGET_FILE = ../libMdv.a
MODULE_TYPE = library

#
# file lists
#

HDRS = \
	../include/Mdv/Mdvx.hh

CPPC_SRCS = \
	Mdvx.cc \
	Mdvx_BE.cc \
	Mdvx_ncf.cc \
	Mdvx_print.cc \
	Mdvx_read.cc \
	Mdvx_timelist.cc \
	Mdvx_vsect.cc \
	Mdvx_write.cc \
	Mdvx_xml.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
