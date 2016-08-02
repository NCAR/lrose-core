###########################################################################
#
# Makefile for kdp 
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Feb 2008
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include -I../include/radar
LOC_CFLAGS =

TARGET_FILE = ../libradar.a

#
# file lists
#

HDRS =

CPPC_SRCS = \
	KdpBringi.cc \
	KdpCompute.cc \
	KdpFilt.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
