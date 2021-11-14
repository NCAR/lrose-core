###############################################################
#
# CMakeLists.txt file for cmake
#
# app name: tdrp_gen
#
# written by script createCMakeLists.py
#
# dir: lrose-core/codebase/apps/tdrp/src/tdrp_gen
###############################################################

project (tdrp_gen)

# source files

set (SRCS
      ctype_list.c
      enum_list.c
      parse_defs.c
      set_item.c
      struct_list.c
      tdrp_gen.c
      write_c_files.c
      write_c++_files.c
    )

# include directories

include_directories (../../../../libs/tdrp/src/include)
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

# link directories

link_directories(${CMAKE_INSTALL_PREFIX}/lib)
if (DEFINED X11_LIB_DIR)
  link_directories (${X11_LIB_DIR})
endif()
if (DEFINED netCDF_INSTALL_PREFIX)
  link_directories (${netCDF_INSTALL_PREFIX}/lib)
endif()
if (DEFINED HDF5_INSTALL_PREFIX)
  link_directories (${HDF5_INSTALL_PREFIX}/lib)
endif()
if (DEFINED HDF5_LIBRARY_DIRS)
  link_directories(${HDF5_LIBRARY_DIRS})
endif()
# add serial, for odd Debian hdf5 install
if(IS_DIRECTORY /usr/lib/x86_64-linux-gnu/hdf5/serial)
  link_directories(/usr/lib/x86_64-linux-gnu/hdf5/serial)
endif()
if(IS_DIRECTORY /usr/local/lib)
  link_directories (/usr/local/lib)
endif()
add_link_options( -L${CMAKE_INSTALL_PREFIX}/lib )

# link libs

link_libraries (tdrp)

# If needed, generate TDRP Params.cc and Params.hh files
# from their associated paramdef.<app> file

makeTdrpParams()
# application

add_executable (tdrp_gen ${SRCS})

# add tdrp_gen as a dependency
add_dependencies(${PROJECT_NAME} tdrp_gen)

# install

INSTALL(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
       )

