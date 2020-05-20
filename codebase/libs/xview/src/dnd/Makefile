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
	dnd.c \
	dnd_data.c \
	dnd_decode.c \
	dnd_dsdm.c \
	dnd_pblc.c \
	dndutil.c \
	site.c \
	site_data.c \
	site_pblc.c

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
