#
# Recursive make - makes the subdirectory code
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = SpdbServer

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	SpdbServer

include $(LROSE_MAKE_INC_DIR)/lrose_make_recursive_dir_targets

include $(LROSE_MAKE_INC_DIR)/lrose_make_inc_targets

include $(LROSE_MAKE_INC_DIR)/lrose_make_lib_targets

FORCE:
