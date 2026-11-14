###############################################################
# CMakeLists.txt for RadxSim
###############################################################

project (RadxSim)

set (SRCS
      Params.cc
      Args.cc
      Main.cc
      RadxSim.cc
    )

include_directories (../../../../libs/Fmq/src/include)
include_directories (../../../../libs/Mdv/src/include)
include_directories (../../../../libs/Ncxx/src/include)
include_directories (../../../../libs/Radx/src/include)
include_directories (../../../../libs/Spdb/src/include)
include_directories (../../../../libs/dataport/src/include)
include_directories (../../../../libs/didss/src/include)
include_directories (../../../../libs/dsdata/src/include)
include_directories (../../../../libs/dsserver/src/include)
include_directories (../../../../libs/euclid/src/include)
include_directories (../../../../libs/physics/src/include)
include_directories (../../../../libs/radar/src/include)
include_directories (../../../../libs/rapformats/src/include)
include_directories (../../../../libs/rapmath/src/include)
include_directories (../../../../libs/tdrp/src/include)
include_directories (../../../../libs/toolsa/src/include)
include_directories (${CMAKE_INSTALL_PREFIX}/include)

if (DEFINED MAMBA_BUILD)
  include_directories (${MAMBA_INCLUDE_PATH})
  link_directories (${MAMBA_LIBRARY_PATH})
else()
  if (DEFINED netCDF_INCLUDE_DIR)
    include_directories (${netCDF_INCLUDE_DIR})
  elseif (DEFINED netCDF_INSTALL_PREFIX)
    include_directories (${netCDF_INSTALL_PREFIX}/include)
  endif()
  if (DEFINED HDF5_C_INCLUDE_DIRS)
    include_directories (${HDF5_C_INCLUDE_DIRS})
  elseif (DEFINED HDF5_INCLUDE_DIRS)
    include_directories (${HDF5_INCLUDE_DIRS})
  endif()
  if (IS_DIRECTORY /usr/include/hdf5/serial)
    include_directories (/usr/include/hdf5/serial)
  endif()
endif()

set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
link_directories(${CMAKE_INSTALL_PREFIX}/lib)

link_libraries (dsdata radar Mdv Spdb Fmq rapformats dsserver didss)
link_libraries (euclid rapmath toolsa dataport tdrp Radx Ncxx physics)
link_libraries (netcdf hdf5_hl hdf5 fftw3 bz2 z pthread)

makeTdrpParams()

add_executable (RadxSim ${SRCS})
add_dependencies(${PROJECT_NAME} tdrp_gen)

install(
    TARGETS RadxSim
    RUNTIME DESTINATION bin
)
