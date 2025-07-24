###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: titan
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/titan/src
###############################################################

project (libtitan)

# include directories

include_directories (./include)
include_directories (../../FiltAlg/src/include)
include_directories (../../FiltAlgVirtVol/src/include)
include_directories (../../Fmq/src/include)
include_directories (../../Mdv/src/include)
include_directories (../../Ncxx/src/include)
include_directories (../../Radx/src/include)
include_directories (../../Refract/src/include)
include_directories (../../Solo/src/include)
include_directories (../../Spdb/src/include)
include_directories (../../advect/src/include)
include_directories (../../cidd/src/include)
include_directories (../../contour/src/include)
include_directories (../../dataport/src/include)
include_directories (../../didss/src/include)
include_directories (../../dsdata/src/include)
include_directories (../../dsserver/src/include)
include_directories (../../euclid/src/include)
include_directories (../../grib/src/include)
include_directories (../../grib2/src/include)
include_directories (../../hydro/src/include)
include_directories (../../kd/src/include)
include_directories (../../physics/src/include)
include_directories (../../qtplot/src/include)
include_directories (../../radar/src/include)
include_directories (../../rapformats/src/include)
include_directories (../../rapmath/src/include)
include_directories (../../rapplot/src/include)
include_directories (../../shapelib/src/include)
include_directories (../../tdrp/src/include)
include_directories (../../titan/src/include)
include_directories (../../toolsa/src/include)
include_directories (${CMAKE_INSTALL_PREFIX}/include)
if (DEFINED MAMBA_BUILD)
# MAMBA builds ignore system libs, use mamba libs
  include_directories (${MAMBA_INCLUDE_PATH})
else()
  if (DEFINED netCDF_INSTALL_PREFIX)
    include_directories (${netCDF_INSTALL_PREFIX}/include)
  endif()
  if (DEFINED HDF5_C_INCLUDE_DIR)
    include_directories (${HDF5_C_INCLUDE_DIR})
  endif()
  if(IS_DIRECTORY /usr/include/hdf5/serial)
    include_directories (/usr/include/hdf5/serial)
  endif()
  if(IS_DIRECTORY /usr/local/include)
    include_directories (/usr/local/include)
  endif()
  # NOTE: cannot add /usr/include using include_directories()
  #add_compile_options(-I/usr/include)
  
endif(DEFINED MAMBA_BUILD)
# source files

set (SRCS
      ./case_tracks/case_tracks.c
      ./case_tracks/SeedCaseTracks.cc
      ./DsTitan/DsTitan.cc
      ./DsTitan/DsTitanMsg.cc
      ./DsTitan/TitanSpdb.cc
      ./file_io/GateData.c
      ./file_io/RfHandle.c
      ./file_io/RfLabels.c
      ./file_io/RfPartialTrack.c
      ./file_io/RfPrintRadar.c
      ./file_io/RfPrintStorm.c
      ./file_io/RfPrintTrack.c
      ./file_io/RfScanTable.c
      ./file_io/RfStorm.c
      ./file_io/RfTables.c
      ./file_io/RfTrack.c
      ./file_io/RfUncompress.c
      ./file_io/RfUtilities.c
      ./file_io/RfZr.c
      ./file_io/TitanData.cc
      ./file_io/TitanFile.cc
      ./file_io/TitanStormFile.cc
      ./file_io/TitanTrackFile.cc
      ./mdv/RfDobson.c
      ./mdv/RfReadMDV.c
      ./mdv/RfWriteDobson.c
      ./mdv/RfWriteMdv.c
      ./tserver/tdata_partial_track.c
      ./tserver/tdata_product.c
      ./tserver/tserver.c
      ./tserver/TitanComplexTrack.cc
      ./tserver/TitanPartialTrack.cc
      ./tserver/TitanSimpleTrack.cc
      ./tserver/TitanServer.cc
      ./tserver/TitanTrackEntry.cc
      ./xml/Titan2Xml.cc
   )

if(APPLE)
# for apple, force static build
  add_library (titan STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (titan SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS titan
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/titan
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

