###########################################################################
#
# Makefile
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# June 2014
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libradar.a

#
# file lists
#

HDRS = \
	../include/radar/ConvStratFinder.hh

CPPC_SRCS = \
	ConvStratParams.cc \
	ConvStratFinder.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

ConvStratParams.cc: paramdef.ConvStrat
	tdrp_gen -f paramdef.ConvStrat -c++ -class ConvStratParams -lib radar -add_ncar_copyright
	/bin/mv -f ConvStratParams.hh ../include/radar

clean_tdrp:
	$(RM) ConvStratParams.cc ../include/radar/ConvStratParams.hh

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
