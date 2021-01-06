#
# Recursive make - makes the subdirectory code
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGETS = $(GENERAL_TARGETS) $(INSTALL_TARGETS)

SUB_DIRS = \
	extractStationLocationList \
	NetCDF2Spdb \
	PrintGenPt1D \
	PrintHydroStation \
        PrintSigAirMet \
        PrintSpdbMetar \
	Rhi2Spdb \
	spdb_merge \
        Taf2Spdb

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_recursive_dir_targets

