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
	cim_change.c \
	cim_size.c \
	csr_change.c \
	csr_init.c \
	ttyansi.c \
	tty_gtty.c \
	tty_init.c \
	tty_ntfy.c \
	tty_stty.c \
	ttytl.c \
	ttyselect.c \
	tty_main.c \
	tty_menu.c \
	tty_mapkey.c \
	tty.c \
	tty_modes.c \
	tty_es.c \
	tty_data.c \
	term_ntfy.c \
	tty_compat.c \
	termsw.c \
	term_data.c \
	tty_newtxt.c \
	tty_cb.c

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
