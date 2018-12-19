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
	KdpFiltParams.cc \
	KdpBringi.cc \
	KdpCompute.cc \
	KdpFilt.cc \
	PhidpProc.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

KdpFiltParams.cc: paramdef.KdpFilt
	tdrp_gen -f paramdef.KdpFilt -c++ -class KdpFiltParams -lib radar
	/bin/mv -f KdpFiltParams.hh ../include/radar

clean_tdrp:
	$(RM) KdpFiltParams.cc ../include/radar/KdpFiltParams.hh

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
