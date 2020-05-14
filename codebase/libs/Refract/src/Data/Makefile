# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/6 21:37:21 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for RefractInput module of the Refract library
#
###########################################################################

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_INC_DIR = ../include
LOC_CPPC_CFLAGS =

TARGET_FILE = ../libRefract.a
MODULE_TYPE = library

#
# file lists
#

HDRS = \
	$(LROSE_INC_DIR)/Refract/MdvCreate.hh \
	$(LROSE_INC_DIR)/Refract/RefractUtil.hh \
	$(LROSE_INC_DIR)/Refract/RefractConstants.hh \
	$(LROSE_INC_DIR)/Refract/FieldDataPair.hh \
	$(LROSE_INC_DIR)/Refract/FieldWithData.hh \
	$(LROSE_INC_DIR)/Refract/IQ.hh \
	$(LROSE_INC_DIR)/Refract/VectorIQ.hh

CPPC_SRCS = \
	IQ.cc \
	RefractUtil.cc \
	MdvCreate.cc \
	FieldDataPair.cc \
	FieldWithData.cc \
	VectorData.cc \
	VectorIQ.cc

#
# general targets
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
