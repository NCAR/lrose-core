# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:40 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for Template module of euclid library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# May 1999
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libeuclid.a
MODULE_TYPE=library

#
# file lists
#

HDRS = \
	$(RAP_INC_DIR)/euclid/CircularTemplate.hh \
	$(RAP_INC_DIR)/euclid/CircularTemplateList.hh \
	$(RAP_INC_DIR)/euclid/EllipticalTemplate.hh \
	$(RAP_INC_DIR)/euclid/GridOffset.hh \
	$(RAP_INC_DIR)/euclid/GridPoint.hh \
	$(RAP_INC_DIR)/euclid/GridTemplate.hh \
	$(RAP_INC_DIR)/euclid/GridTemplateList.hh \
	$(RAP_INC_DIR)/euclid/GlobalCircularTemplate.hh \
	$(RAP_INC_DIR)/euclid/GlobalGridTemplate.hh \
	$(RAP_INC_DIR)/euclid/RectangularTemplate.hh \
	$(RAP_INC_DIR)/euclid/CenteredRectangularTemplate.hh

CPPC_SRCS = \
	CircularTemplate.cc \
	CircularTemplateList.cc \
	EllipticalTemplate.cc \
	GridOffset.cc \
	GridPoint.cc \
	GridTemplate.cc \
	GridTemplateList.cc \
	GlobalCircularTemplate.cc \
	GlobalGridTemplate.cc \
	RectangularTemplate.cc \
	CenteredRectangularTemplate.cc 

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
