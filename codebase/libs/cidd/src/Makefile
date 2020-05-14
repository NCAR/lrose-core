#
# Recursive make - makes the subdirectory code
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = cidd

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	cdata_util \
	images

include $(LROSE_MAKE_INC_DIR)/lrose_make_recursive_dir_targets

include $(LROSE_MAKE_INC_DIR)/lrose_make_inc_targets

# install_include: FORCE
# 	if $(TEST) -d $(LROSE_INC_DIR)/cidd; then :; else \
# 		$(MKDIR) $(LROSE_INC_DIR)/cidd; fi
# 	$(CP) include/cidd/*.h $(LROSE_INC_DIR)/cidd

include $(LROSE_MAKE_INC_DIR)/lrose_make_lib_targets

FORCE:


