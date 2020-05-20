###########################################################################
#
# Makefile for RefParms
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INC_DIR = ../include
MY_CPPCHECKFLAGS = -E RefParams.hh -E RefParams.cc -E ../include/Refract/RefParams.hh

LOC_CPPC_CFLAGS = -I$(LOC_INC_DIR) -Wall 
LOC_CFLAGS = $(LOC_CPPC_CFLAGS)

MODULE_TYPE=library
TARGET_FILE= ../libRefract.a
HDRS = 

CPPC_SRCS = \
	RefParams.cc \
	RefParms.cc

#
# general targets
#
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_targets

RefParams.hh: RefParams.cc
	$(CP) RefParams.hh ../include/Refract
RefParams.cc: paramdef.Refract
	tdrp_gen -f paramdef.Refract -c++ -class RefParams
	$(CP) RefParams.hh ../include/Refract

# DO NOT DELETE THIS LINE -- make depend depends on it.


