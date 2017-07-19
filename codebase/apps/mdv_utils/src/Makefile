#
# Recursive make - makes the subdirectory code
#
# Broken:
#	WorldTerrain \

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	Affirm \
	distBetweenTwoPoints \
	GribNc2Mdv \
	KochSurface2Mdv \
	LatLonBIL2Mdv \
	Mdv2Axf \
	Mdv2AxfCont \
	Mdv2Grib \
	Mdv2Medoc \
	Mdv2NetCDF \
	Mdv2NewZealand \
	Mdv2Plain \
	Mdv2Raob \
	Mdv2SweepNetCDF \
	Mdv2TextHdr \
	Mdv2Vil \
	Mdv3Dto2D \
	MdvAccumulate \
	MdvArithmetic \
	MdvBlend \
	MdvClump \
	MdvCombine \
	MdvComp \
	MdvCompare \
	MdvContour \
	MdvConvert \
	MdvExtractPoint \
	MdvExtract1Point \
	MdvFillMissing \
	MdvFine2Coarse \
	MdvFlat2Fcst \
	MdvFromCircles \
	MdvGrad \
	MdvGridDiagnostic \
	MdvHeaders2Xml \
	MdvHist \
	MdvHowBad \
	MdvKonvert \
	MdvLinearComb \
	MdvMakeCurrent \
	MdvMatch \
	MdvMask \
	MdvMedian \
	MdvMerge \
	MdvMerge2 \
	MdvMosaic \
	MdvMultFilt \
	MdvOffsetField \
	MdvOutlier \
	MdvPatch \
	MdvPointExtract \
	MdvPull \
	mdvRadialDecimate \
	MdvRepeatDay \
	MdvResample \
	MdvResampleTerrain \
	MdvScatterPlot \
	MdvSigmaP2P \
	MdvSmooth \
	MdvSpdDir2UV \
	MdvStats \
	MdvTComp \
	MdvTerrainDistance \
	MdvThresh \
	MdvTrim \
	MdvTSmooth \
	MdvtoGrib2 \
	MdvxHowBad \
	NetCDF2Mdv \
	PrintMdv \
	ReadDEM \
	ReadRUCDEM \
	UpdateMdvFieldNames \
	UpdateMdvOrigin \
	UpdateMdvVlevelTypes \
	WinsRadar2Mdv \
	WorldTerrain \
	adjointDump2mdv \
	ascii2mdv \
	cedric2mdv \
	extractDay \
	gint2mdv \
	kav6h2mdv \
	kav6l2mdv \
	medoc2mdv \
	mdvDeleteMissing \
	mdvFindGaps \
	mdvForecastStatistics \
	mdv_bin_data \
	mdv_thin \
	mdv_thresh_cnt \
	mdv_constant_data \
	mdv_repeat_day \
	mdv_update_levels \
	mdv_update_origin \
	mdvRhiExtract \
	mdv2prf \
	mdv2VectorFile \
	mdvMonitorYesterday \
	nsslMosaic2Mdv \
	ppi2comp \
	print_mdv \
	rescale_mdv \
	smooth_mdv \
	stage4ToMdv \
	tec2mdv \
	threshOnHeight \
	unisysm2mdv \
	update_mdv_times \
	wrfFreezingLevel \
	wrfLtgPotential

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets

