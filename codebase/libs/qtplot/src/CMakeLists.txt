###############################################################
#
# CMakeLists.txt - auto generated from Makefile
#
# library name: qtplot
#
# written by script createCMakeLists.lib.py
#
# dir: lrose-core/codebase/libs/qtplot/src
###############################################################

project (libqtplot)

#Finding Qt
find_package (Qt6 COMPONENTS Core QUIET)
if (NOT Qt6_FOUND)
  find_package (Qt5 COMPONENTS Core REQUIRED)
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
    if (Qt5_FOUND)
      find_path(Qt5_DIR NAMES Qt5Config.cmake qt5-config.cmake HINTS ${MAMBA_ROOT} NO_DEFAULT_PATH)
    elseif (Qt6_FOUND)
      find_path(Qt6_DIR NAMES Qt6Config.cmake qt6-config.cmake HINTS ${MAMBA_ROOT} NO_DEFAULT_PATH)
    endif(Qt5_FOUND)
  else()
    if (Qt5_FOUND)
      find_path(Qt5_DIR NAMES Qt5Config.cmake qt5-config.cmake HINTS /usr/local/Cellar/qt/*/lib/cmake/Qt5 /opt/homebrew/Cellar/qt/*/lib/cmake/Qt5 $ENV{HOME}/homebrew/Cellar/qt/*/lib/cmake/Qt5 NO_DEFAULT_PATH)
    elseif (Qt6_FOUND)
      find_path(Qt6_DIR NAMES Qt6Config.cmake qt6-config.cmake HINTS /usr/local/Cellar/qt/*/lib/cmake/Qt6 /opt/homebrew/Cellar/qt/*/lib/cmake/Qt6 $ENV{HOME}/homebrew/Cellar/qt/*/lib/cmake/Qt6 NO_DEFAULT_PATH)
    endif(Qt5_FOUND)
  endif(MAMBA_BUILD)
endif(APPLE)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC ON)
if (Qt5_FOUND)
#QT5
  if (DEFINED MAMBA_BUILD)
    find_package (Qt5 COMPONENTS Widgets Network Qml REQUIRED PATHS ${MAMBA_ROOT} NO_DEFAULT_PATH)
  else()
    find_package (Qt5 COMPONENTS Widgets Network Qml REQUIRED PATHS /usr /usr/local/opt/qt NO_DEFAULT_PATH)
  endif(MAMBA_BUILD)
  pkg_search_module(Qt5Core REQUIRED)
  pkg_search_module(Qt5Gui REQUIRED)
  pkg_search_module(Qt5Widgets REQUIRED)
  pkg_search_module(Qt5Network REQUIRED)
  pkg_search_module(Qt5Qml REQUIRED)
else()
#QT6
  if (DEFINED MAMBA_BUILD)
    find_package (Qt6 COMPONENTS Widgets Network Qml REQUIRED PATHS ${MAMBA_ROOT} NO_DEFAULT_PATH)
  else()
    find_package (Qt6 COMPONENTS Widgets Network Qml REQUIRED PATHS /usr /usr/local/opt/qt NO_DEFAULT_PATH)
  endif(MAMBA_BUILD)
  pkg_search_module(Qt6Core REQUIRED)
  pkg_search_module(Qt6Gui REQUIRED)
  pkg_search_module(Qt6Widgets REQUIRED)
  pkg_search_module(Qt6Network REQUIRED)
  pkg_search_module(Qt6Qml REQUIRED)
endif(Qt5_FOUND)

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
  include_directories (${MAMBA_INCLUDE_PATH})
else()
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
  # NOTE: cannot add /usr/include using include_directories()
  add_compile_options(-I/usr/include)
  
endif(MAMBA_BUILD)
#Including Qt
if (Qt5_FOUND)
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})
include_directories(${Qt5Qml_INCLUDE_DIRS})
else()
include_directories(${Qt6Core_INCLUDE_DIRS})
include_directories(${Qt6Gui_INCLUDE_DIRS})
include_directories(${Qt6Widgets_INCLUDE_DIRS})
include_directories(${Qt6Network_INCLUDE_DIRS})
include_directories(${Qt6Qml_INCLUDE_DIRS})
endif(Qt5_FOUND)

# source files

set (SRCS
      ./ColorMap/ColorMap.cc
      ./ColorMap/ColorBar.cc
      ./ColorMap/X11ColorMap.cc
   )

if(APPLE)
# for apple, force static build
  add_library (qtplot STATIC ${SRCS})
else(APPLE)
# build shared library
  add_library (qtplot SHARED ${SRCS})
endif(APPLE)

# install

INSTALL(TARGETS qtplot
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )
INSTALL(DIRECTORY include/qtplot
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
       )

