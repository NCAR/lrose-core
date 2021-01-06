###########################################################################
#
# Makefile
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Jan 2008
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include -I../include/radar
LOC_CFLAGS =

TARGET_FILE = ../libradar.a

#
# file lists
#

HDRS =

CPPC_SRCS = \
	PrecipRateParams.cc \
	PrecipRate.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

PrecipRateParams.cc: paramdef.PrecipRate
	tdrp_gen -f paramdef.PrecipRate -c++ -class PrecipRateParams -lib radar -add_ncar_copyright
	/bin/mv -f PrecipRateParams.hh ../include/radar

clean_tdrp:
	$(RM) PrecipRateParams.cc ../include/radar/PrecipRateParams.hh

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
