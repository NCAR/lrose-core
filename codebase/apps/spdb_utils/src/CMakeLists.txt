###############################################################
#
# CMakeLists for cmake recursion
#
# dir: lrose-core/codebase/apps/spdb_utils/src
#
# written by script createCMakeLists.py
#
###############################################################

project (lrose-core)

# subdirectories

add_subdirectory (extractStationLocationList)
add_subdirectory (NetCDF2Spdb)
add_subdirectory (PrintGenPt1D)
add_subdirectory (PrintHydroStation)
add_subdirectory (PrintSigAirMet)
add_subdirectory (PrintSpdbMetar)
add_subdirectory (Rhi2Spdb)
add_subdirectory (spdb_merge)
add_subdirectory (Taf2Spdb)

