#
# Makefile
#
# Niles Oien
#
include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = adjunctAdjunct

LOC_INCLUDES =
LOC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = -lMdv -ldsserver \
           -leuclid -ldidss -lcidd -lrapformats \
           -ltoolsa -ldataport -ltdrp -lrapmath -lm -lpthread

HDRS = \
	$(PARAMS_HH)  

CPPC_SRCS = \
	$(PARAMS_CC) \
	main.cc 

#
# tdrp macros
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard C++ targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.











