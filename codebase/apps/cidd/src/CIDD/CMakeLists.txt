###############################################################
#
# CMakeLists.txt file for cmake
#
# app name: CIDD
#
# written by script createCMakeLists.py
#
# dir: lrose-core/codebase/apps/cidd/src/CIDD
###############################################################

project (CIDD)

# source files

set (SRCS
      Csyprod_P.cc
      Cdraw_P.cc
      Cgui_P.cc
      Cimages_P.cc
      Cterrain_P.cc
      Croutes_P.cc
      Product.cc
      ProductMgr.cc
      RenderContext.cc
      SymprodRender.cc
      SymprodRenderObj.cc
      TimeList.cc
      TimePlot.cc
      cidd_coords.cc
      cidd_font.cc
      cidd_init.cc
      cidd_main.cc
      cidd_params.cc
      cidd_remote_commands.cc
      cidd_reset.cc
      cidd_signals.cc
      cmd_pu_proc.cc
      colorscales.cc
      contours.cc
      data_gather.cc
      data_init.cc
      data_io.cc
      data_pu_proc.cc
      draw_export_init.cc
      draw_pu_proc.cc
      past_pu_proc.cc
      page_pu_proc.cc
      prod_pu_proc.cc
      fcast_pu_proc.cc
      fields_pu_proc.cc
      gen_time_pu_proc.cc
      graphic_canvas_events.cc
      graphic_canvas_events_bkwd.cc
      graphic_check.cc
      graphic_compute.cc
      graphic_cross_section.cc
      graphic_dump_image.cc
      graphic_manage.cc
      graphic_margin_events.cc
      graphic_panel.cc
      graphic_reset.cc
      graphic_resize.cc
      graphic_xsect_canvas_events.cc
      graphic_xsect_resize.cc
      graphic_zoom.cc
      gui_destroy.cc
      gui_init.cc
      gui_label_frame.cc
      gui_modify.cc
      h_win_proc.cc
      mdvx_data_request.cc
      movie_control.cc
      movie_frame_retrieve.cc
      movie_frame_save.cc
      movie_pu_proc.cc
      overlay_init.cc
      over_pu_proc.cc
      process_args.cc
      render_azimuths.cc
      render_bottom_margin.cc
      render_cart_grid.cc
      render_cbar.cc
      render_click_marks.cc
      render_control.cc
      render_distorted_grid.cc
      render_filled_contours.cc
      render_filled_image.cc
      render_grids.cc
      render_ht_sel.cc
      render_left_margin.cc
      render_legends.cc
      render_line_contours.cc
      render_margins.cc
      render_movie_frame.cc
      render_overlays.cc
      render_polar_grid.cc
      render_products.cc
      render_range_rings.cc
      render_right_margin.cc
      render_route_winds.cc
      render_terrain.cc
      render_top_layers.cc
      render_top_margin.cc
      render_winds.cc
      render_xsect_control.cc
      render_xsect_grids.cc
      render_xsect_line_contours.cc
      render_xsect_margins.cc
      render_xsect_products.cc
      render_xsect_top_layers.cc
      render_xsect_winds.cc
      route_pu_proc.cc
      route_winds_init.cc
      save_pu_proc.cc
      shmem_init.cc
      status_pu_proc.cc
      symprod_init.cc
      timer_control.cc
      v_win_proc.cc
      winds_init.cc
      xview_file_access.cc
      zoom_pu_proc.cc
      cmd_pu_ui.cc
      data_pu_ui.cc
      draw_pu_ui.cc
      fcast_pu_ui.cc
      fields_pu_ui.cc
      gen_time_pu_ui.cc
      h_win_ui.cc
      movie_pu_ui.cc
      over_pu_ui.cc
      past_pu_ui.cc
      page_pu_ui.cc
      prod_pu_ui.cc
      route_pu_ui.cc
      save_pu_ui.cc
      status_pu_ui.cc
      v_win_ui.cc
      zoom_pu_ui.cc
      Uparams.cc
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
link_libraries (devguide)
link_libraries (xview)
link_libraries (olgx)
link_libraries (X11)
link_libraries (Xext)
link_libraries (png)
link_libraries (netcdf)
link_libraries (hdf5_hl)
link_libraries (hdf5)
link_libraries (z)
link_libraries (bz2)
link_libraries (pthread)
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

add_executable (CIDD ${SRCS})

# add tdrp_gen as a dependency
add_dependencies(${PROJECT_NAME} tdrp_gen)

# install

INSTALL(TARGETS ${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
       )

