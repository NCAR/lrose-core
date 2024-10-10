###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: dsdata
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/dsdata/src
###############################################################

project (libdsdata)

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
  add_compile_options(-I/usr/include)
  
endif(DEFINED MAMBA_BUILD)
# source files

set (SRCS
      ./AcarsDataPoint/AcarsDataPoint.cc
      ./DsTrigger/DsDirListTrigger.cc
      ./DsTrigger/DsEnsembleAnyTrigger.cc
      ./DsTrigger/DsEnsembleDataTrigger.cc
      ./DsTrigger/DsEnsembleGenTrigger.cc
      ./DsTrigger/DsEnsembleLeadTrigger.cc
      ./DsTrigger/DsFcstTime.cc
      ./DsTrigger/DsFcstTimeListTrigger.cc
      ./DsTrigger/DsFileListTrigger.cc
      ./DsTrigger/DsFmqTrigger.cc
      ./DsTrigger/DsInputDirTrigger.cc
      ./DsTrigger/DsInputPathTrigger.cc
      ./DsTrigger/DsIntervalTrigger.cc
      ./DsTrigger/DsLdataIntTrigger.cc
      ./DsTrigger/DsLdataTrigger.cc
      ./DsTrigger/DsMultFcstTrigger.cc
      ./DsTrigger/DsMultInputDirTrigger.cc
      ./DsTrigger/DsMultipleTrigger.cc
      ./DsTrigger/DsMultTrigElem.cc
      ./DsTrigger/DsOneFileTrigger.cc
      ./DsTrigger/DsOneTimeTrigger.cc
      ./DsTrigger/DsSpecificFcstLdataTrigger.cc
      ./DsTrigger/DsSpecificFcstTimeListTrigger.cc
      ./DsTrigger/DsSpecificGenLdataTrigger.cc
      ./DsTrigger/DsTimeListTrigger.cc
      ./DsTrigger/DsTrigger.cc
      ./DsTrigger/DsUrlTrigger.cc
      ./DsTrigger/DsUrlTriggerArchive.cc
      ./DsTrigger/DsUrlTriggerRealtime.cc
      ./DsTrigger/DsUrlTriggerObject.cc
      ./DsTrigger/DsUrlTriggerObjectDerived.cc
      ./DsTrigger/DsUrlTriggerSubsample.cc
      ./DsTrigger/MdvTimeListHandler.cc
      ./DsTrigger/SpdbTimeListHandler.cc
      ./DsTrigger/TimeListHandler.cc
      ./DsTrigger/TriggerInfo.cc
      ./Tstorm/Tstorm.cc
      ./Tstorm/TstormGrid.cc
      ./Tstorm/TstormGroup.cc
      ./Tstorm/TstormMgr.cc
   )

if(APPLE)
# for apple, force static build
  add_library (dsdata STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (dsdata SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS dsdata
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/dsdata
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

