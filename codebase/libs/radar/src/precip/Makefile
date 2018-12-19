###########################################################################
#
# Makefile
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Jan 2008
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
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

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

PrecipRateParams.cc: paramdef.PrecipRate
	tdrp_gen -f paramdef.PrecipRate -c++ -class PrecipRateParams -lib radar
	/bin/mv -f PrecipRateParams.hh ../include/radar

clean_tdrp:
	$(RM) PrecipRateParams.cc ../include/radar/PrecipRateParams.hh

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
