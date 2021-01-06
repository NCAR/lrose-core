# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/7 17:43:53 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for GridAdvect module of the advect library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# March 2001
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CPPC_CFLAGS =

TARGET_FILE = ../libadvect.a
MODULE_TYPE = library

#
# file lists
#

HDRS = \
	$(LROSE_INSTALL_DIR)/include/advect/GridAdvect.hh \
	$(LROSE_INSTALL_DIR)/include/advect/Advector.hh \
	$(LROSE_INSTALL_DIR)/include/advect/SoundingAdvector.hh \
	$(LROSE_INSTALL_DIR)/include/advect/VectorsAdvector.hh

CPPC_SRCS = \
	GridAdvect.cc \
	SoundingAdvector.cc \
	VectorsAdvector.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
