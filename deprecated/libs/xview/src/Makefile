#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

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

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets

install_include: FORCE
	if $(TEST) -d $(LROSE_INSTALL_DIR)/include/xview; then :; else \
		$(MKDIR) $(LROSE_INSTALL_DIR)/include/xview; fi
	$(CP) include/xview/*.h $(LROSE_INSTALL_DIR)/include/xview
	if $(TEST) -d $(LROSE_INSTALL_DIR)/include/pixrect; then :; else \
		$(MKDIR) $(LROSE_INSTALL_DIR)/include/pixrect; fi
	$(CP) include/pixrect/*.h $(LROSE_INSTALL_DIR)/include/pixrect

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_targets
