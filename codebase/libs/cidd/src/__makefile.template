#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = cidd

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	cdata_util \
	images

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_inc_targets

# install_include: FORCE
# 	if $(TEST) -d $(LROSE_INSTALL_DIR)/include/cidd; then :; else \
# 		$(MKDIR) $(LROSE_INSTALL_DIR)/include/cidd; fi
# 	$(CP) include/cidd/*.h $(LROSE_INSTALL_DIR)/include/cidd

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_targets

FORCE:


