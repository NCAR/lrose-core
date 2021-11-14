###############################################################
#
# CMakeLists.txt file for cmake
#
# app name: MdvMerge
#
# written by script createCMakeLists.py
#
# dir: lrose-core/codebase/apps/mdv_utils/src/MdvMerge
###############################################################

project (MdvMerge)

# source files

set (SRCS
      Params.cc
      Args.cc
      InputFile.cc
      Main.cc
      OutputFile.cc
      MdvMerge.cc
      Trigger.cc
      MergerInt8.cc
      MergerFloat32.cc
    )

# include directories

include_directories (../../../../libs/FiltAlg/src/include)
include_directories (../../../../libs/FiltAlgVirtVol/src/include)
include_directories (../../../../libs/Fmq/src/include)
include_directories (../../../../libs/Mdv/src/include)
include_directories (../../../../libs/Ncxx/src/include)
include_directories (../../../../libs/Radx/src/include)
include_directories (../../../../libs/Refract/src/include)
include_directories (../../../../libs/Solo/src/include)
include_directories (../../../../libs/Spdb/src/include)
include_directories (../../../../libs/advect/src/include)
include_directories (../../../../libs/cidd/src/include)
include_directories (../../../../libs/contour/src/include)
include_directories (../../../../libs/dataport/src/include)
include_directories (../../../../libs/didss/src/include)
include_directories (../../../../libs/dsdata/src/include)
include_directories (../../../../libs/dsserver/src/include)
include_directories (../../../../libs/euclid/src/include)
include_directories (../../../../libs/grib/src/include)
include_directories (../../../../libs/grib2/src/include)
include_directories (../../../../libs/hydro/src/include)
include_directories (../../../../libs/kd/src/include)
include_directories (../../../../libs/physics/src/include)
include_directories (../../../../libs/radar/src/include)
include_directories (../../../../libs/rapformats/src/include)
include_directories (../../../../libs/rapmath/src/include)
include_directories (../../../../libs/rapplot/src/include)
include_directories (../../../../libs/shapelib/src/include)
include_directories (../../../../libs/tdrp/src/include)
include_directories (../../../../libs/titan/src/include)
include_directories (../../../../libs/toolsa/src/include)
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

link_libraries (Refract)
link_libraries (FiltAlg)
link_libraries (dsdata)
link_libraries (radar)
link_libraries (hydro)
link_libraries (titan)
link_libraries (Fmq)
link_libraries (Spdb)
link_libraries (Mdv)
link_libraries (advect)
link_libraries (rapplot)
link_libraries (Radx)
link_libraries (Ncxx)
link_libraries (rapformats)
link_libraries (dsserver)
link_libraries (didss)
link_libraries (grib)
link_libraries (grib2)
link_libraries (contour)
link_libraries (euclid)
link_libraries (rapmath)
link_libraries (kd)
link_libraries (physics)
link_libraries (toolsa)
link_libraries (dataport)
link_libraries (tdrp)
link_libraries (shapelib)
link_libraries (cidd)
link_libraries (pthread)
link_libraries (netcdf)
link_libraries (hdf5_hl)
link_libraries (hdf5)
link_libraries (z)
link_libraries (bz2)
link_libraries (m)
link_libraries (Ncxx)
link_libraries (netcdf)
link_libraries (hdf5_hl)
link_libraries (hdf5)
link_libraries (fftw3)
link_libraries (X11)
link_libraries (Xext)
link_libraries (pthread)
link_libraries (png)
link_libraries (z)
link_libraries (bz2)
link_libraries (m)

# If needed, generate TDRP Params.cc and Params.hh files
# from their associated paramdef.<app> file

makeTdrpParams()
# application

add_executable (MdvMerge ${SRCS})

# add tdrp_gen as a dependency
add_dependencies(${PROJECT_NAME} tdrp_gen)

# install

INSTALL(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
       )

