###########################################################################
#
# Makefile for RefParms
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =
TARGET_FILE = ../libRefract.a

#
# file lists
#

HDRS =

CPPC_SRCS = \
	RefParams.cc \
	RefParms.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

RefParams.cc: paramdef.Refract
	tdrp_gen -f paramdef.Refract -c++ -class RefParams -lib Refract -add_ncar_copyright
	/bin/mv -f RefParams.hh ../include/Refract

clean_tdrp:
	$(RM) RefParams.cc ../include/Refract/RefParams.hh

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

