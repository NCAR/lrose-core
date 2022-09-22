###############################################################
#
# CMakeLists.txt file for cmake
#
# app name: Rview
#
# written by script createCMakeLists.py
#
# dir: lrose-core/codebase/apps/titan/src/Rview
###############################################################

project (Rview)

# source files

set (SRCS
      ProdParams.cc
      Rview.cc
      contours.cc
      convert_xwd_to_web.cc
      copy_cappi.cc
      copy_cappi_web.cc
      copy_vsection.cc
      copy_vsection_web.cc
      create_frames.cc
      data_check.cc
      draw_ac_posn.cc
      draw_cappi.cc
      draw_main_scale.cc
      draw_titan.cc
      draw_verify.cc
      draw_vsection.cc
      event_loop.cc
      free_resources.cc
      get_contour_intervals.cc
      get_ps_color_scales.cc
      get_titan_data.cc
      get_x_color_scales.cc
      help.cc
      map.cc
      parse_args.cc
      print_copy.cc
      read_field_control.cc
      read_params.cc
      set_sens.cc
      set_xfonts.cc
      set_xgcs.cc
      setup_cappi_page.cc
      setup_cappi_windows.cc
      setup_help_windows.cc
      setup_track_shmem.cc
      setup_vsection_page.cc
      setup_vsection_windows.cc
      setup_x.cc
      tidy_and_exit.cc
      xerror_handler.cc
      zoom_calc.cc
      ProductMgr.cc
      Product.cc
      RenderContext.cc
      SymprodRender.cc
      SymprodRenderObj.cc
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
link_libraries (qtplot)
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
link_libraries (X11)
link_libraries (netcdf)
link_libraries (hdf5_hl)
link_libraries (hdf5)
link_libraries (z)
link_libraries (bz2)
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

add_executable (Rview ${SRCS})

# add tdrp_gen as a dependency
add_dependencies(${PROJECT_NAME} tdrp_gen)

# install

INSTALL(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
       )

