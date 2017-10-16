#
# makefile as template for configure
# does not include scripts dir
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	print_procmap \
	procmap \
	procmap_register \
	procmap_unregister \
	test_procmap

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets
