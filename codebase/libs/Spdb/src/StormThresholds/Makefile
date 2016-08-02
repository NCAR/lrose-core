# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:33 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for file SPDB ThresholdBiasMapping
#
# Dave Albo, RAP, NCAR, Boulder, Co, USA, 80307
#
# August, 2014
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_INC_DIR = ../include
MY_CPPCHECKFLAGS = -exclude-rules=4.16
LOC_CFLAGS =

TARGET_FILE = ../libSpdb.a

#
# file lists
#

HDRS = \
	../include/Spdb/ThresholdBiasMapping.hh

CPPC_SRCS = \
	ThresholdBiasMapping.cc \
	MultiThreshBiasMapping.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
