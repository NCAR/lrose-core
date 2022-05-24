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
	titan \
	trec \
	didss \
	ingest \
	dsserver \
	mdv_utils \
        physics \
        satApps \
	spdb_utils \
	filters \
        awc \
	hydro \
	interp \
	radar \
	refractt \
        Radx \
        cidd \
        spol \
        xpol

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets
