###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: grib2
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/grib2/src
###############################################################

project (libgrib2)

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
if (DEFINED X11_X11_INCLUDE_PATH)
  include_directories (${X11_X11_INCLUDE_PATH})
endif()
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
      ./grib2/Grib2Record.cc
      ./grib2/GribSection.cc
      ./grib2/IdSec.cc
      ./grib2/IndicatorSec.cc
      ./grib2/LocalUseSec.cc
      ./grib2/GribProj.cc
      ./grib2/LatLonProj.cc
      ./grib2/RotLatLonProj.cc
      ./grib2/MercatorProj.cc
      ./grib2/PolarStereoProj.cc
      ./grib2/LambertConfProj.cc
      ./grib2/GausLatLonProj.cc
      ./grib2/SpaceViewProj.cc
      ./grib2/RotLatLonProjArakawaNonE.cc
      ./grib2/GDS.cc
      ./grib2/PDS.cc
      ./grib2/ProdDefTemp.cc
      ./grib2/Template4.0.cc
      ./grib2/Template4.1.cc
      ./grib2/Template4.2.cc
      ./grib2/Template4.5.cc
      ./grib2/Template4.6.cc
      ./grib2/Template4.7.cc
      ./grib2/Template4.8.cc
      ./grib2/Template4.9.cc
      ./grib2/Template4.10.cc
      ./grib2/Template4.11.cc
      ./grib2/Template4.12.cc
      ./grib2/Template4.15.cc
      ./grib2/Template4.30.cc
      ./grib2/DataRepTemp.cc
      ./grib2/Template5.41.cc
      ./grib2/Template5.4000.cc
      ./grib2/Template5.0.cc
      ./grib2/Template5.2.cc
      ./grib2/Template5.3.cc
      ./grib2/DRS.cc
      ./grib2/BMS.cc
      ./grib2/DS.cc
      ./grib2/DataTemp.cc
      ./grib2/Template7.41.cc
      ./grib2/Template7.4000.cc
      ./grib2/Template7.0.cc
      ./grib2/Template7.2.cc
      ./grib2/ES.cc
      ./grib2/Grib2File.cc
   )

if(APPLE)
# for apple, force static build
  add_library (grib2 STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (grib2 SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS grib2
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/grib2
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

