#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS =  $(GENERAL_TARGETS) $(DOC_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	converters \
	tdrp \
	filters \
	fuzzy_logic \
	general_algs \
	ocnd \
	procmap \
	servmap \
	spdb_utils \
	symprod \
	titan \
	titan_analysis \
	mdv_utils \
        satApps

$(TARGETS): FORCE
	$(MAKE) $(SUB_DIRS) TARGET=$@

$(SUB_DIRS): FORCE
	cd $@; $(MAKE) $(TARGET)

FORCE:







