###############################################################
#
# Codebase-level CMakeLists file for lrose-core
#
# dir: lrose-core/codebase
#
# written by script createCMakeLists.py
#
###############################################################

cmake_minimum_required(VERSION 3.7)

project (lrose-core)

# set policy CMP0100 so that AUTOMOC finds .hh files
if(${CMAKE_VERSION} VERSION_GREATER "3.17.0")
  cmake_policy(SET CMP0100 NEW)
endif()
if(${CMAKE_VERSION} VERSION_GREATER "3.10.0")
  cmake_policy(SET CMP0071 NEW)
endif()

set(CMAKE_C_COMPILER_NAMES clang gcc icc cc fcc)
set(CMAKE_CXX_COMPILER_NAMES clang++ g++ icpc c++ cxx FCC CC)

# do not scan anaconda directories
set(CMAKE_IGNORE_PREFIX_PATH "$ENV{HOME}/anaconda3;$ENV{HOME}/anaconda2")
message("CMAKE_IGNORE_PREFIX_PATH: ${CMAKE_IGNORE_PREFIX_PATH}")

set(CMAKE_VERBOSE_MAKEFILE OFF)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../build/cmake/"
     CACHE INTERNAL "Location of our custom CMake modules.")

SET(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/cmake")

set(FETCHCONTENT_QUIET false CACHE BOOL "" FORCE)

set(PACKAGE "LROSE-CORE" CACHE STRING "")

# Finding pkg-config
find_package(PkgConfig REQUIRED)

set(MINIFORGE_ROOT "$ENV{HOME}/miniforge3")
if (DEFINED MINIFORGE_DIR)
  set(MINIFORGE_ROOT "${MINIFORGE_DIR}")
endif()

if (DEFINED MAMBA_BUILD)
# MAMBA builds ignore system includes and libs, use mamba dirs
  set(MAMBA_INCLUDE_PATH ${MINIFORGE_ROOT}/include)
  set(MAMBA_LIBRARY_PATH ${MINIFORGE_ROOT}/lib)
  set(QT_HOST_PATH ${MINIFORGE_ROOT})
  set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)
  set(CMAKE_SYSTEM_IGNORE_PREFIX_PATH "/usr;/usr/local;/usr/lib64;/lib;/lib64;/opt/homebrew")
endif(DEFINED MAMBA_BUILD)

find_package (X11)
if (DEFINED X11_X11_LIB)
  get_filename_component(X11_LIB_DIR ${X11_X11_LIB} DIRECTORY)
endif()

# Finding Qt

find_package (Qt6 COMPONENTS Core QUIET)
if (NOT Qt6_FOUND)
  find_package (Qt5 COMPONENTS Core QUIET)
endif()
if (Qt5_FOUND)
  message(STATUS "Found Qt5: ${Qt5_VERSION}")
elseif (Qt6_FOUND)
    message(STATUS "Found Qt6: ${Qt6_VERSION}")
else ()
  message(FATAL_ERROR, "Qt not found.")
endif(Qt5_FOUND)

if(APPLE)
  if (DEFINED MAMBA_BUILD)
# MAMBA builds ignore system libs, use mamba libs
    if (Qt5_FOUND)
      find_path(Qt5_DIR NAMES Qt5Config.cmake qt5-config.cmake HINTS ${MINIFORGE_ROOT} NO_DEFAULT_PATH)
    elseif (Qt6_FOUND)
      find_path(Qt6_DIR NAMES Qt6Config.cmake qt6-config.cmake HINTS ${MINIFORGE_ROOT} NO_DEFAULT_PATH)
    endif(Qt5_FOUND)
  else ()
    if (Qt5_FOUND)
      find_path(Qt5_DIR NAMES Qt5Config.cmake qt5-config.cmake HINTS /usr/local/Cellar/qt/*/lib/cmake/Qt5 /opt/homebrew/Cellar/qt/*/lib/cmake/Qt5 $ENV{HOME}/homebrew/Cellar/qt/*/lib/cmake/Qt5 NO_DEFAULT_PATH)
    elseif (Qt6_FOUND)
      find_path(Qt6_DIR NAMES Qt6Config.cmake qt6-config.cmake HINTS /usr/local/Cellar/qt/*/lib/cmake/Qt6 /opt/homebrew/Cellar/qt/*/lib/cmake/Qt6 $ENV{HOME}/homebrew/Cellar/qt/*/lib/cmake/Qt6 NO_DEFAULT_PATH)
    endif(Qt5_FOUND)
  endif(DEFINED MAMBA_BUILD)
endif(APPLE)

find_package (HDF5)

# find_package (NETCDF)
# find_package (LROSE)

if (DEFINED HDF5_hdf5_LIBRARY_RELEASE)
  get_filename_component(HDF5_INSTALL_PREFIX ${HDF5_hdf5_LIBRARY_RELEASE} DIRECTORY)
endif()
if (NOT IS_DIRECTORY netCDF_INSTALL_PREFIX)
  set (netCDF_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()
if (NOT IS_DIRECTORY HDF5_INSTALL_PREFIX)
  set (HDF5_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()
if (NOT IS_DIRECTORY HDF5_C_INCLUDE_DIR)
  set (HDF5_C_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)
endif()
message("X11_X11_INCLUDE_PATH: ${X11_X11_INCLUDE_PATH}")
message("X11_LIB_DIR: ${X11_LIB_DIR}")
message("HDF5_INSTALL_PREFIX: ${HDF5_INSTALL_PREFIX}")
message("HDF5_C_INCLUDE_DIR: ${HDF5_C_INCLUDE_DIR}")

# If user did not provide CMAKE_INSTALL_PREFIX, use ~/lrose
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/lrose" CACHE PATH "..." FORCE)
endif()
message("CMAKE_INSTALL_PREFIX is ${CMAKE_INSTALL_PREFIX}")

# Function for creating TDRP Params.cc and Params.hh files

function(makeTdrpParams)

  # Add a custom generator for TDRP Params.cc and Params.hh files
  # from their associated paramdef.<app> file

  set(TDRP_EXECUTABLE ${CMAKE_BINARY_DIR}/codebase/apps/tdrp/src/tdrp_gen/tdrp_gen)

  add_custom_command (
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Params.hh ${CMAKE_CURRENT_SOURCE_DIR}/Params.cc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/paramdef.${PROJECT_NAME}

    # Run tdrp_gen, explicitly using the one from this build instead of
    # any version that may be in the user's path.
    COMMAND export LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/codebase/libs/tdrp/src &&
      cd ${CMAKE_CURRENT_SOURCE_DIR} &&
      ${CMAKE_BINARY_DIR}/codebase/apps/tdrp/src/tdrp_gen/tdrp_gen
        -c++
        -f paramdef.${PROJECT_NAME}
        -prog ${PROJECT_NAME}
        -add_ncar_copyright
    COMMENT "Generating/updating Params.hh and Params.cc for ${PROJECT_NAME}"
  )

endFunction()

# Function for creating SINGLETON TDRP Params.cc and Params.hh files

function(makeTdrpSingleton)

  # Add a custom generator for TDRP Params.cc and Params.hh files
  # from their associated paramdef.<app> file

  set(TDRP_EXECUTABLE ${CMAKE_BINARY_DIR}/codebase/apps/tdrp/src/tdrp_gen/tdrp_gen)

  add_custom_command (
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Params.hh ${CMAKE_CURRENT_SOURCE_DIR}/Params.cc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/paramdef.${PROJECT_NAME}

    # Run tdrp_gen, explicitly using the one from this build instead of
    # any version that may be in the user's path.
    COMMAND export LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/codebase/libs/tdrp/src &&
      cd ${CMAKE_CURRENT_SOURCE_DIR} &&
      ${CMAKE_BINARY_DIR}/codebase/apps/tdrp/src/tdrp_gen/tdrp_gen
        -c++ -singleton
        -f paramdef.${PROJECT_NAME}
        -prog ${PROJECT_NAME}
        -add_ncar_copyright
    COMMENT "Generating/updating Params.hh and Params.cc for ${PROJECT_NAME}"
  )

endFunction()

enable_testing()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC ")
if (Qt5_FOUND)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++17 ")
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++17 ")
endif(Qt5_FOUND)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_JASPER_LIB ")

# Set RPATH so that executablefinds lrose libraries without needing LD_LIBRARY_PATH

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
endif()

add_subdirectory(libs)
add_subdirectory(apps)

