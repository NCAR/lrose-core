#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	Ascii2Radx \
	Dsr2Radx \
	FixCfradialPaths \
	FixRadxPointing \
	FixRadxValues \
	Radx2Dsr \
	Radx2Grid \
	RadxBeamBlock \
	RadxBufr \
	RadxCalUpdate \
	RadxCheck \
	RadxClutMon \
	RadxConvert \
	RadxCov2Mom \
	RadxCreateVol \
	RadxDiffFields \
	RadxDiffVol \
	RadxDwellCombine \
	RadxEvad \
	RadxFilter \
	RadxHca \
	RadxKdp \
	RadxMergeFields \
	RadxMergeVols \
	RadxMesoCyclone \
	RadxModelQc \
	RadxMon \
	RadxPartRain \
	RadxPersistentClutter \
	RadxPid \
	RadxPrint \
	RadxQc \
	RadxQpe \
	RadxRate \
	RadxSunMon \
        RadxTimeMedian \
        RadxVolTimeStats

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets
