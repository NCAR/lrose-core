
include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = testFileFetch

LOC_INCLUDES =
LOC_CFLAGS =
LOC_LDFLAGS = -L/usr/local/lib
LOC_LIBS = -ldidss -ltoolsa -ldataport -ltdrp \
	-lbz2 -lz

HDRS = $(PARAMS_HH) fileFetch.hh 

CPPC_SRCS = $(PARAMS_CC) fileFetch.cc \
	Main.cc 

#
# tdrp macros
#

##include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard C++ targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp targets
#

##include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.
