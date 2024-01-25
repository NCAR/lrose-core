###############################################################
#
# CMakeLists.txt file for cmake
#
# app name: HawkEye
#
# written by script createCMakeLists.py
#
# dir: lrose-core/codebase/apps/radar/src/HawkEye
###############################################################

project (HawkEye)

# source files

set (SRCS
      Params.cc
      AllocCheck.cc
      Args.cc
      BoundaryPointEditor.cc
      Beam.cc
      BscanBeam.cc
      ColorTableManager.cc
      DisplayField.cc
      DisplayFieldModel.cc
      FlowLayout.cc
      HawkEye.cc
      HawkEyeLogger.cc
      Main.cc
      PaletteManager.cc
      PpiBeam.cc
      Reader.cc
      RhiBeam.cc
      ScaledLabel.cc
      SiiPalette.cc
      SoloDefaultColorWrapper.cc
      SoloFunctionsModel.cc
      SpreadSheetItem.cc
      SpreadSheetModel.cc
      SpreadSheetUtils.cc
      SweepManager.cc
      WorldPlot.cc
      BscanManager.cc
      BscanWidget.cc
      ClickableLabel.cc
      ColorBar.cc
      ColorMapTemplates.cc
      DisplayManager.cc
      FieldColorController.cc
      FieldRenderer.cc
      FunctionEditor.cc
      ParameterColorView.cc
      PolarManager.cc
      PolarWidget.cc
      PpiWidget.cc
      RhiWidget.cc
      RhiWindow.cc
      SoloFunctions.cc
      SpreadSheetController.cc
      SpreadSheetDelegate.cc
      SpreadSheetView.cc
      TextEdit.cc
      TimeScaleWidget.cc
    )

#Finding Qt
find_package (Qt6 COMPONENTS Core)
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
  if (Qt5_FOUND)
    find_path(Qt5_DIR NAMES Qt5Config.cmake qt5-config.cmake HINTS /usr/local/Cellar/qt/*/lib/cmake/Qt5 /opt/homebrew/Cellar/qt/*/lib/cmake/Qt5 $ENV{HOME}/homebrew/Cellar/qt/*/lib/cmake/Qt5 NO_DEFAULT_PATH)
  elseif (Qt6_FOUND)
    find_path(Qt6_DIR NAMES Qt6Config.cmake qt6-config.cmake HINTS /usr/local/Cellar/qt/*/lib/cmake/Qt6 /opt/homebrew/Cellar/qt/*/lib/cmake/Qt6 $ENV{HOME}/homebrew/Cellar/qt/*/lib/cmake/Qt6 NO_DEFAULT_PATH)
  endif(Qt5_FOUND)
endif(APPLE)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_AUTOUIC ON)
if (Qt5_FOUND)
#QT5
  find_package (Qt5 COMPONENTS Widgets Network Qml REQUIRED PATHS /usr /usr/local/opt/qt NO_DEFAULT_PATH)
  pkg_search_module(Qt5Core REQUIRED)
  pkg_search_module(Qt5Gui REQUIRED)
  pkg_search_module(Qt5Widgets REQUIRED)
  pkg_search_module(Qt5Network REQUIRED)
  pkg_search_module(Qt5Qml REQUIRED)
else()
#QT6
  find_package (Qt6 COMPONENTS Widgets Network Qml REQUIRED PATHS /usr /usr/local/opt/qt NO_DEFAULT_PATH)
  pkg_search_module(Qt6Core REQUIRED)
  pkg_search_module(Qt6Gui REQUIRED)
  pkg_search_module(Qt6Widgets REQUIRED)
  pkg_search_module(Qt6Network REQUIRED)
  pkg_search_module(Qt6Qml REQUIRED)
endif(Qt5_FOUND)

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
include_directories (../../../../libs/qtplot/src/include)
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
if(${CMAKE_VERSION} VERSION_GREATER "3.13.0")
  add_link_options( -L${CMAKE_INSTALL_PREFIX}/lib )
endif()

# link libs

link_libraries (dsdata)
link_libraries (radar)
link_libraries (Mdv)
link_libraries (Spdb)
link_libraries (Fmq)
link_libraries (rapformats)
link_libraries (dsserver)
link_libraries (didss)
link_libraries (euclid)
link_libraries (rapmath)
link_libraries (qtplot)
link_libraries (toolsa)
link_libraries (dataport)
link_libraries (tdrp)
link_libraries (Radx)
link_libraries (Ncxx)
link_libraries (physics)
link_libraries (netcdf)
link_libraries (hdf5_hl)
link_libraries (hdf5)
link_libraries (z)
link_libraries (bz2)
link_libraries (fftw3)
link_libraries (bz2)
link_libraries (z)
link_libraries (pthread)
#Linking Qt
if (Qt5_FOUND)
  link_libraries (${Qt5Core_LIBRARIES})
  link_libraries (${Qt5Gui_LIBRARIES})
  link_libraries (${Qt5Widgets_LIBRARIES})
  link_libraries (${Qt5Network_LIBRARIES})
  link_libraries (${Qt5Qml_LIBRARIES})
else()
  link_libraries (${Qt6Core_LIBRARIES})
  link_libraries (${Qt6Gui_LIBRARIES})
  link_libraries (${Qt6Widgets_LIBRARIES})
  link_libraries (${Qt6Network_LIBRARIES})
  link_libraries (${Qt6Qml_LIBRARIES})
endif(Qt5_FOUND)


# If needed, generate TDRP Params.cc and Params.hh files
# from their associated paramdef.<app> file

makeTdrpParams()
# application

add_executable (HawkEye ${SRCS})

# add tdrp_gen as a dependency
add_dependencies(${PROJECT_NAME} tdrp_gen)

# install

INSTALL(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
       )

