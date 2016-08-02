###########################################################################
#
# Mike Dixon, EOL, NCAR, Boulder, Co, USA, 80307
#
# Dec 2015
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include -I../include/radar $(NETCDF4_INCS)
LOC_CFLAGS =

TARGET_FILE = ../libradar.a

#
# file lists
#

HDRS = \
	../include/radar/RadxApp.hh \
	../include/radar/RadxAppArgs.hh \
	../include/radar/RadxAppConfig.hh \
	../include/radar/RadxAppParams.hh \
	../include/radar/AtmosAttenTemplate.hh

CPPC_SRCS = \
	RadxAppParams.cc \
	RadxApp.cc \
	RadxAppArgs.cc \
	RadxAppConfig.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

RadxAppParams.hh: RadxAppParams.cc
	$(CP) RadxAppParams.hh ../include/radar

RadxAppParams.cc: paramdef.RadxApp
	tdrp_gen -f paramdef.RadxApp -c++ -class RadxAppParams

# DO NOT DELETE THIS LINE -- make depend depends on it.
