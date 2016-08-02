#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	print_procmap \
	procmap \
	procmap_register \
	procmap_unregister \
	scripts \
	test_procmap

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets
