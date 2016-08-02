###########################################################################
#
# Makefile for spdb library
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# June 1996
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS = $(SYS_X_INCLUDES)

TARGET_FILE = ../libolgx.a

HDRS = \
	../include/olgx/olgx.h \
	../include/olgx_private/busy.h \
	../include/olgx_private/grey.h \
	../include/olgx_private/olgx_impl.h

SRCS = \
	ol_button.c \
	ol_color.c \
	ol_draw.c \
	ol_init.c \
	ol_misc.c \
	ol_sb.c \
	ol_slider.c

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
