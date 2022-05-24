#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = src

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_doc_targets

# -------------------------------------------------------------------
# Special cases...
#
# installation
#

install_titan:
	if $(TEST) -d $(TITAN_HOME); then :; else \
		$(MKDIR) $(TITAN_HOME); fi
	if $(TEST) -d $(TITAN_HOME)/output; then :; else \
		$(MKDIR) $(TITAN_HOME)/output; fi
	if $(TEST) -d $(TITAN_HOME)/tables; then :; else \
		$(MKDIR) $(TITAN_HOME)/tables; fi
	if $(TEST) -d $(TITAN_HOME)/cdata; then :; else \
		$(MKDIR) $(TITAN_HOME)/cdata; fi
	if $(TEST) -d $(TITAN_HOME)/storms; then :; else \
		$(MKDIR) $(TITAN_HOME)/storms; fi
	if $(TEST) -d $(TITAN_HOME)/clutter; then :; else \
		$(MKDIR) $(TITAN_HOME)/clutter; fi
	if $(TEST) -d $(TITAN_HOME)/precip_forecast; then :; else \
		$(MKDIR) $(TITAN_HOME)/precip_forecast; fi
	$(CP) -r color_scales doc maps $(TITAN_HOME)
	$(CP) -r params prologues scripts $(TITAN_HOME)

install_rtest:
	if $(TEST) -d $(TITAN_HOME); then :; else \
		$(MKDIR) $(TITAN_HOME); fi
	$(CP) -r rtest $(TITAN_HOME)


