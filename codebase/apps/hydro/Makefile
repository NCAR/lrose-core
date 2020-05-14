#
# Recursive make - makes the subdirectory code
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = src

$(TARGETS): FORCE
	$(MAKE) $(SUB_DIRS) TARGET=$@

$(SUB_DIRS): FORCE
	cd $@; $(MAKE) $(TARGET)

include $(LROSE_MAKE_INC_DIR)/lrose_make_doc_targets

FORCE:
