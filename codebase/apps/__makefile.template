#
# makefile as template for configure
# does not include scripts
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS =  $(GENERAL_TARGETS) $(DOC_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	tdrp \
	scripts \
	procmap \
	radar \
        Radx 


	# spdb_utils \
        # physics \
	# dsserver \
	# trec \
	#mdv_utils \
	# didss \
	# hydro \
        # awc \
	#interp \
	# ingest \

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets
