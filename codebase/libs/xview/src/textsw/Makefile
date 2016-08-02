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
	es_file.c \
	es_mem.c \
	es_util.c \
	es_attr.c \
	ps_impl.c \
	ev_display.c \
	ev_op_bdry.c \
	ev_edit.c \
	ev_once.c \
	ev_attr.c \
	ev_field.c \
	ev_update.c \
	es_cp_file.c \
	ei_text.c \
	ei_attr.c \
	finger_tbl.c \
	txt_data.c \
	txt_once.c \
	txt_input.c \
	txt_edit.c \
	txt_menu.c \
	txt_disp.c \
	txt_event.c \
	txt_find.c \
	txt_file.c \
	txt_sel.c \
	txt_selsvc.c \
	txt_again.c \
	txt_getkey.c \
	txt_putkey.c \
	txt_filter.c \
	text.c \
	txt_attr.c \
	txt_dbx.c \
	txt_tool.c \
	txt_scroll.c \
	txt_field.c \
	txt_caret.c \
	txt_search.c \
	txt_e_menu.c \
	txt_move.c \
	txt_view.c \
	txt_popup.c \
	txt_store.c \
	txt_match.c \
	txt_load.c \
	txt_line.c \
	txt_incl.c \
	txt_cb.c \
	convpos.c

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
