#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	Dsr2Radx \
	FixCfradialPaths \
	FixRadxPointing \
	Radx2Dsr \
	RadxCheck \
	RadxClutMon \
	RadxCov2Mom \
	RadxDiff \
	RadxDwellCombine \
	RadxEvad \
	RadxFilter \
	RadxMergeFields \
	RadxMergeVols \
	RadxMon \
	RadxPrint

# add back in later

#Ascii2Radx
#Radx2Grid
#RadxConvert
#RadxPartRain
#RadxSunMon
#RadxQpe

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets
