#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	AcPolygonStats \
	AdvectPolygon \
	AverageSpdbSndg \
	FixStationAltitudes \
	GenPoly2Mdv \
	Hash \
	NetCDF2Spdb \
	PrintGenPt1D \
	PrintHydroStation \
	PrintSigAirMet \
	PrintSpdbMetar \
	Rhi2Spdb \
	Sigmet2Mdv \
	Sndg2Ascii \
	SpdbBdry2Axf \
	SpdbMultSim \
	StationCompare \
	extractStationLocationList \
	run_PrintSigAirMet \
	spdb_merge \
	spdb_repair \
	spdbLatencies \
	spdbIntervalCount \
	station2ascii

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets
