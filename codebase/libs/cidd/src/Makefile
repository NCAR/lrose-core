#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = cidd

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	cdata_util \
	images

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets

include $(RAP_MAKE_INC_DIR)/rap_make_inc_targets

# install_include: FORCE
# 	if $(TEST) -d $(RAP_INC_DIR)/cidd; then :; else \
# 		$(MKDIR) $(RAP_INC_DIR)/cidd; fi
# 	$(CP) include/cidd/*.h $(RAP_INC_DIR)/cidd

include $(RAP_MAKE_INC_DIR)/rap_make_lib_targets

FORCE:


