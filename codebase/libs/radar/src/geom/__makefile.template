###########################################################################
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# June 2012
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include $(NETCDF4_INCS)
LOC_CFLAGS =

TARGET_FILE = ../libradar.a

#
# file lists
#

HDRS = \
	../include/radar/BeamHeight.hh \
	../include/radar/Egm2008.hh

CPPC_SRCS = \
	BeamHeight.cc \
	Egm2008.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
