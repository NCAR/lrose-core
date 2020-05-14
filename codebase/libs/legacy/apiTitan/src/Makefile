#
# Recursive make - makes the subdirectory code
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = apiTitan

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	dataport_bigend \
	dataport_smallend \
	dataport_swap \
	didss_DataFileNames \
	didss_DsMessage \
	didss_DsURL \
	didss_LdataInfo \
	didss_RapDataDir \
	dsserver_DataMapper \
	dsserver_DsClient \
	dsserver_DsLdataInfo \
	dsserver_DsLocator \
	dsserver_DsServer \
	dsserver_DsSvrMgrSocket \
	titan_DsTitan \
	titan_file_io \
	titan_tserver \
	rapformats_titan \
	toolsa_Path \
	toolsa_URL \
	toolsa_compress \
	toolsa_file_io \
	toolsa_fmq \
	toolsa_mem \
	toolsa_pjg \
	toolsa_pmu \
	toolsa_port \
	toolsa_sockutil \
	toolsa_str \
	toolsa_utim\
	toolsa_umisc

include $(LROSE_MAKE_INC_DIR)/lrose_make_recursive_dir_targets

install_include: mk_inc_dir FORCE
	$(CP) include/*.h $(LROSE_INC_DIR)
	$(CP) include/dataport/*h $(LROSE_INC_DIR)/dataport
	$(CP) include/didss/*h $(LROSE_INC_DIR)/didss
	$(CP) include/dsserver/*h $(LROSE_INC_DIR)/dsserver
	$(CP) include/rapformats/*h $(LROSE_INC_DIR)/rapformats
	$(CP) include/rapmath/*h $(LROSE_INC_DIR)/rapmath
	$(CP) include/titan/*h $(LROSE_INC_DIR)/titan
	$(CP) include/toolsa/*h $(LROSE_INC_DIR)/toolsa

mk_inc_dir:
	if $(TEST) -d $(LROSE_INC_DIR); then :; else \
		$(MKDIR) $(LROSE_INC_DIR); fi
	if $(TEST) -d $(LROSE_INC_DIR)/didss; then :; else \
		$(MKDIR) $(LROSE_INC_DIR)/didss; fi
	if $(TEST) -d $(LROSE_INC_DIR)/dataport; then :; else \
		$(MKDIR) $(LROSE_INC_DIR)/dataport; fi
	if $(TEST) -d $(LROSE_INC_DIR)/dsserver; then :; else \
		$(MKDIR) $(LROSE_INC_DIR)/dsserver; fi
	if $(TEST) -d $(LROSE_INC_DIR)/rapformats; then :; else \
		$(MKDIR) $(LROSE_INC_DIR)/rapformats; fi
	if $(TEST) -d $(LROSE_INC_DIR)/rapmath; then :; else \
		$(MKDIR) $(LROSE_INC_DIR)/rapmath; fi
	if $(TEST) -d $(LROSE_INC_DIR)/titan; then :; else \
		$(MKDIR) $(LROSE_INC_DIR)/titan; fi
	if $(TEST) -d $(LROSE_INC_DIR)/toolsa; then :; else \
		$(MKDIR) $(LROSE_INC_DIR)/toolsa; fi

install_lib: mk_lib_dir all
	ar $(ARFLAGS) $(LIBNAME) */*.o
	$(CP) $(LIBNAME) $(LROSE_LIB_DIR)
	if $(TEST) -f $(RANLIB);\
	then\
		$(RANLIB) $(LROSE_LIB_DIR)/$(LIBNAME);\
	fi

mk_lib_dir:
	if $(TEST) -d $(LROSE_LIB_DIR); then :; else \
		$(MKDIR) $(LROSE_LIB_DIR); fi

install: \
        install_include install_lib

clean_lib:
	$(RM) *.a

clean_all: clean clean_lib







