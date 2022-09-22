###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: Fmq
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/Fmq/src
###############################################################

project (libFmq)

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
      ./DrawQueue/DrawQueue.cc
      ./DsFmq/DsFmq.cc
      ./DsFmq/DsFmqMsg.cc
      ./DsRadarQueue/DsRadarQueue.cc
      ./Fmq/Fmq.cc
      ./Fmq/FmqDevice.cc
      ./Fmq/FmqDeviceFile.cc
      ./Fmq/FmqDeviceShmem.cc
      ./GenericQueue/GenericQueue.cc
      ./NowcastQueue/NowcastProcess.cc
      ./NowcastQueue/NowcastQueue.cc
      ./RemoteUIQueue/RemoteUIQueue.cc
   )

if(APPLE)
# for apple, force static build
  add_library (Fmq STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (Fmq SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS Fmq
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/Fmq
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

