#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	AlertMeta2Spdb \
	AlertNet2Spdb \
	BasinGenPt2Symprod \
	BasinPrecip \
	BasinPrecipSpdb2Xml \
	BasinSelect \
	Dbz2Precip \
	Dbz2RainRate \
	GetHttpData \
	HailKEswath \
	PrecipAccum \
	QNet2Spdb \
	Shape2Map

$(TARGETS): FORCE
	$(MAKE) $(SUB_DIRS) TARGET=$@

$(SUB_DIRS): FORCE
	cd $@; $(MAKE) $(TARGET)

FORCE:






