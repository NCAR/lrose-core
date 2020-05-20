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
	pw_btchrop.c \
	pw_cms.c \
	pw_get.c \
	pw_line.c \
	pw_plygon2.c \
	pw_plyline.c \
	pw_plypt.c \
	pw_put.c \
	pw_read.c \
	pw_traprop.c \
	pw_vector.c \
	xv_olgx.c \
	xv_text.c \
	xv_stencil.c \
	xv_rop.c \
	mem.c \
	mem_ops.c

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
