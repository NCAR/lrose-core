###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: FiltAlg
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/FiltAlg/src
###############################################################

project (libFiltAlg)

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

# source files

set (SRCS
      ./FiltAlg/FiltAlgParams.cc
      ./FiltAlg/FiltAlgParms.cc
      ./FiltAlg/Algorithm.cc
      ./FiltAlg/BasicInfo.cc
      ./FiltAlg/Boundaries.cc
      ./FiltAlg/Comb.cc
      ./FiltAlg/CombineData.cc
      ./FiltAlg/Data1d.cc
      ./FiltAlg/Data2d.cc
      ./FiltAlg/Data.cc
      ./FiltAlg/FiltCreate.cc
      ./FiltAlg/Filter.cc
      ./FiltAlg/Filt2d.cc
      ./FiltAlg/Filt2dNoOverlap.cc
      ./FiltAlg/FiltClump.cc
      ./FiltAlg/FiltCombine.cc
      ./FiltAlg/FiltDB.cc
      ./FiltAlg/FiltInfo.cc
      ./FiltAlg/FiltInfoInput.cc
      ./FiltAlg/FiltInfoOutput.cc
      ./FiltAlg/FiltMaxTrue.cc
      ./FiltAlg/FiltMask.cc
      ./FiltAlg/FiltMedian.cc
      ./FiltAlg/FiltMedianNoOverlap.cc
      ./FiltAlg/FiltPassThrough.cc
      ./FiltAlg/FiltRemap.cc
      ./FiltAlg/FiltRescale.cc
      ./FiltAlg/FiltReplace.cc
      ./FiltAlg/FiltScalar.cc
      ./FiltAlg/FiltSRemap.cc
      ./FiltAlg/FiltTrapRemap.cc
      ./FiltAlg/FiltVertComb.cc
      ./FiltAlg/Find.cc
      ./FiltAlg/FindSimple.cc
      ./FiltAlg/Fuzzy.cc
      ./FiltAlg/GridExpand.cc
      ./FiltAlg/GridProj.cc
      ./FiltAlg/Info.cc
      ./FiltAlg/InterfaceFiltAlgParm.cc
      ./FiltAlg/Looper.cc
      ./FiltAlg/Statics.cc
      ./FiltAlg/VlevelData.cc
      ./FiltAlg/VlevelSlice.cc
   )

if(APPLE)
# for apple, force static build
  add_library (FiltAlg STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (FiltAlg SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS FiltAlg
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/FiltAlg
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

