###########################################################################
#
# Makefile for xview library
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# July 2000
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS = -DX11R6 $(SYS_X_INCLUDES)

TARGET_FILE = ../libxview.a

HDRS =

SRCS = \
	demorandom.c \
	getlogindr.c \
	expandname.c \
	expandpath.c \
	bitmask.c \
	hashfn.c \
	db_conv.c \
	gettext.c \
	quark.c \
	setlocale.c \
	windowloop.c \
	xv_deaf.c \
	xv_util.c \
	xv_version.c \
	xv_win_lp.c \
	drop.c \
	xv_casecmp.c \
	convdup.c \
	xv_i18n.c \
	xv_path.c

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
