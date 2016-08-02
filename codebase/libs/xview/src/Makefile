#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = xview

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	attr \
	base \
	canvas \
	color \
	cursor \
	defaults \
	dnd \
	file_chooser \
	filter \
	font \
	frame \
	fullscreen \
	help \
	icon \
	imcb \
	io_stream \
	menu \
	misc \
	notice \
	notify \
	openwin \
	panel \
	pw \
	rect \
	screen \
	scrollbar \
	sel \
	selection \
	server \
	server_image \
	string_utils \
	textsw \
	ttysw \
	win \
	window \
	wmgr

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets

install_include: FORCE
	if $(TEST) -d $(RAP_INC_DIR)/xview; then :; else \
		$(MKDIR) $(RAP_INC_DIR)/xview; fi
	$(CP) include/xview/*.h $(RAP_INC_DIR)/xview
	if $(TEST) -d $(RAP_INC_DIR)/pixrect; then :; else \
		$(MKDIR) $(RAP_INC_DIR)/pixrect; fi
	$(CP) include/pixrect/*.h $(RAP_INC_DIR)/pixrect

include $(RAP_MAKE_INC_DIR)/rap_make_lib_targets
