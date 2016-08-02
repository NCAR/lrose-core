#
# Recursive make - makes the subdirectory code
#

include $(RAP_MAKE_INC_DIR)/rap_make_macros

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

include $(RAP_MAKE_INC_DIR)/rap_make_recursive_dir_targets

