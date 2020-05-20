###########################################################################
#
# Makefile for multi_filter_app
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=progcpp
TARGET_FILE=multi_filter_app
LOC_INCLUDES = -I.
LOC_CFLAGS = 
LOC_CPPC_CFLAGS = 
LOC_LDFLAGS = 

LOC_LIBS = -lFiltAlg -lfreezing_level -lniwot_basic -ldsdata -lSpdb \
	   -lMdv -ldsserver -ldidss -lrapformats -leuclid -lrapmath -ltoolsa \
	   -ldataport -ltdrp -lm
HDRS = 

CPPC_SRCS = \
	$(PARAMS_CC) \
	Main.cc \
	Parms.cc \
	AppFiltCreate.cc \
	Filt1.cc \
	Filt2.cc \
	Filt3.cc


#
# tdrp support
#
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# Standard c++ targets
#
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

#
# tdrp targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.
