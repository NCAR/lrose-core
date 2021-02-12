###############################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: tdrp
#
# written by script createCMakeLists.lib.py
#
# created 2021-02-12 14:00:33.026992
#
###############################################

project (libtdrp)

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
if (DEFINED HDF5_CXX_INCLUDE_DIR)
  include_directories (${HDF5_CXX_INCLUDE_DIR})
endif()
if (DEFINED HDF5_C_INCLUDE_DIR)
  include_directories (${HDF5_C_INCLUDE_DIR})
endif()
if(IS_DIRECTORY /usr/local/include)
  include_directories (/usr/local/include)
endif()

# source files

set (SRCS
      ./tdrp/check.c
      ./tdrp/convert.c
      ./tdrp/copy.c
      ./tdrp/find_item.c
      ./tdrp/load.c
      ./tdrp/mem.c
      ./tdrp/override.c
      ./tdrp/print.c
      ./tdrp/read_item.c
      ./tdrp/tdrp.c
      ./tdrp/tdrp_obsolete.c
      ./tdrp/tdrpbuf.c
      ./tdrp/token_list.c
      ./tdrp/tokenize.c
      ./tdrp/warn_on_extra_params.c
   )

if(APPLE)
# for apple, force static build
  add_library (tdrp STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (tdrp SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS tdrp
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/tdrp
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

