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

LOC_INC_DIR = ../include
MY_CPPCHECKFLAGS = -E RadxAppParams.cc -E RadxAppParams.hh -E ../include/radar/RadxAppParams.hh 

#
# file lists
#

HDRS = \
	../include/radar/RadxApp.hh \
	../include/radar/RadxAppArgs.hh \
	../include/radar/RadxAppConfig.hh \
	../include/radar/RadxAppParams.hh \
	../include/radar/RadxAppRayData.hh \
	../include/radar/RadxAppRayLoopData.hh \
	../include/radar/AtmosAttenTemplate.hh

CPPC_SRCS = \
	RadxAppParams.cc \
	RadxAppParms.cc \
	RadxApp.cc \
	RadxAppArgs.cc \
	RadxAppCircularLookupHandler.cc \
	RadxAppConfig.cc \
	RadxAppVolume.cc \
	RadxAppRayData.cc  \
	RadxAppRayLoopData.cc \
	RadxAppSweepData.cc  \
	RadxAppSweepLoopData.cc 

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
	$(CP) RadxAppParams.hh ../include/radar

# DO NOT DELETE THIS LINE -- make depend depends on it.
