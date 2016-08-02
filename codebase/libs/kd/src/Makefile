#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = kd

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	kd

$(TARGETS): FORCE
	$(MAKE) $(SUB_DIRS) TARGET=$@

$(SUB_DIRS): FORCE
	cd $@; $(MAKE) $(TARGET)

include $(RAP_MAKE_INC_DIR)/rap_make_inc_targets

include $(RAP_MAKE_INC_DIR)/rap_make_lib_targets

FORCE:
