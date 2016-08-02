###########################################################################
#
# Makefile for uf2mdv
#
# Dave Albo January 1998
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

MODULE_TYPE=program

TARGET_FILE = netcdfLook
LOC_INCLUDES = -I. -Ioutput -Iinput $(SYS_X_INCLUDES) 
LOC_CFLAGS = -Wall
LOC_CPPC_CFLAGS =
LOC_LDFLAGS = $(SYS_X_LDFLAGS) 

LOC_LIBS = \
	   -lnetcdf 

HDRS = 

C_SRCS = \
	netcdfLook.c

#
# tdrp support
#
#include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_targets

#
# local targets
#

#depend: $(TDRP_C) depend_generic

#depend : depend_generic

#$(TDRP_C): $(TDRP_DEFS)
#	tdrp_gen $(TDRP_MODULE) -f $(TDRP_DEFS)

#clean_tdrp::
#	$(RM) $(TDRP_HDRS) $(TDRP_C)

depend: depend_generic


# DO NOT DELETE THIS LINE -- make depend depends on it.

