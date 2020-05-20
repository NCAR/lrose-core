###########################################################################
#
# Makefile for xview library
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# July 2000
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS = -DX11R6 $(SYS_X_INCLUDES)

TARGET_FILE = ../libxview.a

HDRS =

SRCS = \
	openwin.c \
	ow_evt.c \
	ow_get.c \
	ow_paint.c \
	ow_resize.c \
	ow_set.c \
	ow_view.c \
	ow_rescale.c \
	ow_data.c

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
