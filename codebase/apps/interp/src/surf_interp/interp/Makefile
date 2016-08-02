###########################################################################
#
# Makefile for interp program
#
# RAP, NCAR, Boulder, CO, 80307, USA
#
# June 1997
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = surf_interp

LOC_INCLUDES = 
LOC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = -lmdv -lsymprod -lrapformats -ltoolsa -ldataport -lm

MODULE_TYPE = progf77
FC =  g77

HDRS = \
	fort_stuff.h \
	pints.h \
	read_sounding_class.h


C_SRCS = \
	write_mdv.c \
	Main.c \
	read_sounding_class.c \
	calc_li.c \
	get_spdb_metars.c \
	pints.c \
	fort_stuff.c
#
# standard targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.






