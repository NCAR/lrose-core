#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS = $(GENERAL_TARGETS)

MODULE_NAME = apiMdv

LIBNAME = lib$(MODULE_NAME).a

SUB_DIRS = \
	Mdv_ClimoFileFinder \
	Mdv_ClimoStatCalc \
	Mdv_DsMdvx \
	Mdv_Mdvx \
	Mdv_MdvxChunk \
	Mdv_MdvxContour \
	Mdv_MdvxField \
	Mdv_MdvxGrid \
	Mdv_MdvxProj \
	Mdv_MdvxRadar \
	Mdv_MdvxRemapLut \
	Mdv_MdvxTimeList \
	Mdv_MdvxTimeStamp \
	Mdv_MdvxVsectLut \
	dataport_bigend \
	dataport_smallend \
	dataport_swap \
	didss_DataFileNames \
	didss_DsAccess \
	didss_DsInputPath \
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
	rapformats_DsRadar \
	rapformats_var_elev \
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

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets

install_include: mk_inc_dir FORCE
	$(CP) include/*.h $(LROSE_INSTALL_DIR)/include
	$(CP) include/Mdv/*h $(LROSE_INSTALL_DIR)/include/Mdv
	$(CP) include/dataport/*h $(LROSE_INSTALL_DIR)/include/dataport
	$(CP) include/didss/*h $(LROSE_INSTALL_DIR)/include/didss
	$(CP) include/dsserver/*h $(LROSE_INSTALL_DIR)/include/dsserver
	$(CP) include/rapformats/*h $(LROSE_INSTALL_DIR)/include/rapformats
	$(CP) include/toolsa/*h $(LROSE_INSTALL_DIR)/include/toolsa

mk_inc_dir:
	if $(TEST) -d $(LROSE_INSTALL_DIR)/include; then :; else \
		$(MKDIR) $(LROSE_INSTALL_DIR)/include; fi
	if $(TEST) -d $(LROSE_INSTALL_DIR)/include/Mdv; then :; else \
		$(MKDIR) $(LROSE_INSTALL_DIR)/include/Mdv; fi
	if $(TEST) -d $(LROSE_INSTALL_DIR)/include/didss; then :; else \
		$(MKDIR) $(LROSE_INSTALL_DIR)/include/didss; fi
	if $(TEST) -d $(LROSE_INSTALL_DIR)/include/dataport; then :; else \
		$(MKDIR) $(LROSE_INSTALL_DIR)/include/dataport; fi
	if $(TEST) -d $(LROSE_INSTALL_DIR)/include/dsserver; then :; else \
		$(MKDIR) $(LROSE_INSTALL_DIR)/include/dsserver; fi
	if $(TEST) -d $(LROSE_INSTALL_DIR)/include/rapformats; then :; else \
		$(MKDIR) $(LROSE_INSTALL_DIR)/include/rapformats; fi
	if $(TEST) -d $(LROSE_INSTALL_DIR)/include/toolsa; then :; else \
		$(MKDIR) $(LROSE_INSTALL_DIR)/include/toolsa; fi

install_lib: mk_lib_dir all
	ar $(ARFLAGS) $(LIBNAME) */*.o
	$(CP) $(LIBNAME) $(LROSE_INSTALL_DIR)/lib
	if $(TEST) -f $(RANLIB);\
	then\
		$(RANLIB) $(LROSE_INSTALL_DIR)/lib/$(LIBNAME);\
	fi

mk_lib_dir:
	if $(TEST) -d $(LROSE_INSTALL_DIR)/lib; then :; else \
		$(MKDIR) $(LROSE_INSTALL_DIR)/lib; fi

install: \
        install_include install_lib

clean_lib:
	$(RM) *.a

clean_all: clean clean_lib







