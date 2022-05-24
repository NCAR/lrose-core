###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: tdrp
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/tdrp/src
###############################################################

project (libtdrp)

# include directories

include_directories (./include)
include_directories (../../tdrp/src/include)
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

