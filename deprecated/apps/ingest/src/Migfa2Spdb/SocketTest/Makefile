###########################################################################
#
# Makefile
#
# Niles Oien, RAP, NCAR, Boulder, CO, 80307, USA
#
# February 2002
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = SocketTest

LOC_INCLUDES =
LOC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = -lSpdb -ldsserver \
           -leuclid -ldidss -lcidd -lphysics -lrapformats \
           -ltoolsa -ldataport -ltdrp -lrapmath -lm -lpthread -lctetwws

HDRS =  
CPPC_SRCS = SocketTest.cc

#
# tdrp macros
#

#include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp targets
#

#include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.









