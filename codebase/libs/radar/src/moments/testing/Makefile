###########################################################################
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Jan 2008
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I.
LOC_CFLAGS =

TARGET_FILE = ../../libradar.a

#
# file lists
#

HDRS = \
	./FftManager.hh \
	./PolyFit.hh \
	./RegrFilter.hh

CPPC_SRCS = \
	./FftManager.cc \
	./PolyFit.cc \
	./RegrFilter.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
