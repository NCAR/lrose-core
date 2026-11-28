#
# makefile as template for configure
# does not include scripts dir
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	print_procmap \
	procmap \
	procmap_register \
	procmap_unregister \
	test_procmap

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets
