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
	win_cursor.c \
	win_env.c \
	win_geom.c \
	win_global.c \
	win_cntral.c \
	win_input.c \
	win_treeop.c \
	win_bell.c \
	win_damage.c \
	win_compat.c \
	win_keymap.c

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
