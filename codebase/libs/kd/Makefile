#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS) $(LIB_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = src

$(TARGETS): FORCE
	$(MAKE) $(SUB_DIRS) TARGET=$@

$(SUB_DIRS): FORCE
	cd $@; $(MAKE) $(TARGET)

include $(RAP_MAKE_INC_DIR)/rap_make_doc_targets

FORCE:





