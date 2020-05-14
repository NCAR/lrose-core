###########################################################################
#
# Makefile for xview library
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# July 2000
#
###########################################################################

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS = -DX11R6 $(SYS_X_INCLUDES)

TARGET_FILE = ../libxview.a

HDRS =

SRCS = \
	p_utl.c \
	panel.c \
	p_set.c \
	p_get.c \
	p_event.c \
	p_scroll.c \
	p_select.c \
	p_paint.c \
	p_image.c \
	p_public.c \
	p_compat.c \
	p_view.c \
	item.c \
	item_set.c \
	item_get.c \
	p_txt.c \
	p_choice.c \
	p_btn.c \
	p_msg.c \
	p_slider.c \
	p_list.c \
	p_list2.c \
	p_num_txt.c \
	p_gauge.c \
	p_ambtn.c \
	p_drop.c \
	p_drp_data.c \
	p_utl_data.c \
	panel_data.c \
	item_data.c \
	p_btn_data.c \
	p_chcedata.c \
	p_txt_data.c \
	p_msg_data.c \
	p_lst_data.c \
	p_ntx_data.c \
	p_amb_data.c \
	p_mlinetxt.c \
	p_mltxdata.c \
	p_imcb.c 

#
# general targets
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.
