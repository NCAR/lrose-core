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
	frame.c \
	frame_init.c \
	fm_get.c \
	fm_set.c \
	fm_destroy.c \
	fm_layout.c \
	fm_cmdline.c \
	fm_display.c \
	fm_win.c \
	fm_geom.c \
	fm_input.c \
	fm_bdry.c \
	frame_sw.c \
	fm_rescale.c \
	frame_base.c \
	fmbs_set.c \
	fmbs_get.c \
	fmbs_dsty.c \
	frame_cmd.c \
	fmcmd_set.c \
	fmcmd_get.c \
	fmcmd_dsty.c \
	frame_help.c \
	fmhlp_set.c \
	fmhlp_get.c \
	fmhlp_dsty.c \
	fm_data.c \
	fm_rodata.c

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
