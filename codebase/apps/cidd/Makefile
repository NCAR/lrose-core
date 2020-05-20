#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = src

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets

# ------------------------------------------------------------
# Special cases
#
# installation
#

install_titan:
	if $(TEST) -d $(TITAN_HOME); then :; else \
		$(MKDIR) -p $(TITAN_HOME); fi
	if $(TEST) -d $(TITAN_HOME)/output; then :; else \
		$(MKDIR) -p $(TITAN_HOME)/output; fi
	if $(TEST) -d $(TITAN_HOME)/tables; then :; else \
		$(MKDIR) -p $(TITAN_HOME)/tables; fi
	if $(TEST) -d $(TITAN_HOME)/cdata; then :; else \
		$(MKDIR) -p $(TITAN_HOME)/cdata; fi
	if $(TEST) -d $(TITAN_HOME)/storms; then :; else \
		$(MKDIR) -p $(TITAN_HOME)/storms; fi
	if $(TEST) -d $(TITAN_HOME)/clutter; then :; else \
		$(MKDIR) -p $(TITAN_HOME)/clutter; fi
	if $(TEST) -d $(TITAN_HOME)/precip_forecast; then :; else \
		$(MKDIR) -p $(TITAN_HOME)/precip_forecast; fi
	/bin/cp -r color_scales doc maps $(TITAN_HOME)
	/bin/cp -r params prologues scripts $(TITAN_HOME)
