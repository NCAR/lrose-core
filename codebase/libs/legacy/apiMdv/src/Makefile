#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

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

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets

install_include: mk_inc_dir FORCE
	$(CP) include/*.h $(RAP_INC_DIR)
	$(CP) include/Mdv/*h $(RAP_INC_DIR)/Mdv
	$(CP) include/dataport/*h $(RAP_INC_DIR)/dataport
	$(CP) include/didss/*h $(RAP_INC_DIR)/didss
	$(CP) include/dsserver/*h $(RAP_INC_DIR)/dsserver
	$(CP) include/rapformats/*h $(RAP_INC_DIR)/rapformats
	$(CP) include/toolsa/*h $(RAP_INC_DIR)/toolsa

mk_inc_dir:
	if $(TEST) -d $(RAP_INC_DIR); then :; else \
		$(MKDIR) $(RAP_INC_DIR); fi
	if $(TEST) -d $(RAP_INC_DIR)/Mdv; then :; else \
		$(MKDIR) $(RAP_INC_DIR)/Mdv; fi
	if $(TEST) -d $(RAP_INC_DIR)/didss; then :; else \
		$(MKDIR) $(RAP_INC_DIR)/didss; fi
	if $(TEST) -d $(RAP_INC_DIR)/dataport; then :; else \
		$(MKDIR) $(RAP_INC_DIR)/dataport; fi
	if $(TEST) -d $(RAP_INC_DIR)/dsserver; then :; else \
		$(MKDIR) $(RAP_INC_DIR)/dsserver; fi
	if $(TEST) -d $(RAP_INC_DIR)/rapformats; then :; else \
		$(MKDIR) $(RAP_INC_DIR)/rapformats; fi
	if $(TEST) -d $(RAP_INC_DIR)/toolsa; then :; else \
		$(MKDIR) $(RAP_INC_DIR)/toolsa; fi

install_lib: mk_lib_dir all
	ar $(ARFLAGS) $(LIBNAME) */*.o
	$(CP) $(LIBNAME) $(RAP_LIB_DIR)
	if $(TEST) -f $(RANLIB);\
	then\
		$(RANLIB) $(RAP_LIB_DIR)/$(LIBNAME);\
	fi

mk_lib_dir:
	if $(TEST) -d $(RAP_LIB_DIR); then :; else \
		$(MKDIR) $(RAP_LIB_DIR); fi

install: \
        install_include install_lib

clean_lib:
	$(RM) *.a

clean_all: clean clean_lib







