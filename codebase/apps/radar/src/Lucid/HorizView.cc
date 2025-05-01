// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

#include "HorizView.hh"
#include "VertView.hh"
#include "GuiManager.hh"

#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/LogStream.hh>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QErrorMessage>
#include <QRect>
#include <QPainterPath>
#include <algorithm>

#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace std;

HorizView::HorizView(QWidget* parent,
                     GuiManager &manager) :
        QWidget(parent),
        _parent(parent),
        _manager(manager),
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _selectedField(0),
        _rubberBand(nullptr)
        
{

  // initialize

  _archiveMode = _params.start_mode == Params::MODE_ARCHIVE;
  setAttribute(Qt::WA_TranslucentBackground);
  setAutoFillBackground(false);
  
  // Allow the widget to get focus
  
  setFocusPolicy(Qt::StrongFocus);

  // Allow the size_t type to be passed to slots

  qRegisterMetaType<size_t>("size_t");

  // init other values

  _worldPressX = 0.0;
  _worldPressY = 0.0;
  _worldReleaseX = 0.0;
  _worldReleaseY = 0.0;

  _mousePressX = 0;
  _mousePressY = 0;
  _mouseReleaseX = 0;
  _mouseReleaseY = 0;

  _zoomCornerX = 0;
  _zoomCornerY = 0;

  _pointClicked = false;
  _worldClickX = 0.0;
  _worldClickY = 0.0;
  _worldClickLat = 0.0;
  _worldClickLon = 0.0;
  
  _colorScaleWidth = _params.horiz_color_scale_width;

  // initialize world view

  configureWorldCoords(0);

  setGrids(_params.horiz_grids_on_at_startup);
  setRingsFixed(_params.plot_range_rings_fixed);
  setRingsDataDriven(_params.plot_range_rings_from_data);

  _isArchiveMode = false;
  _isStartOfSweep = true;

  _renderFrameIndex = 0;
  _renderFramePage = 0;

  _renderInvalidImages = false;
  _invalidImagesFrameIndex = 0;
  _vert = NULL;

  _gridsReady = false;
  _mapsReady = false;
  _zoomChanged = true;
  _sizeChanged = true;
  
  //fires every 50ms. used for boundary editor to
  // (1) detect shift key down (changes cursor)
  // (2) get notified if user zooms in or out so the boundary can be rescaled
  // startTimer(50);

}

/*************************************************************************
 * Destructor
 */

HorizView::~HorizView()
{

  if (!_rubberBand) {
    delete _rubberBand;
  }

}

/*************************************************************************
 * clear()
 */

void HorizView::clear()
{
  _renderMaps();
}

/*************************************************************************
 * configureWorldCoords()
 */

void HorizView::configureWorldCoords(int zoomLevel)

{

  // set world view

  _fullWorld.setName("fullWorld");
  
  _fullWorld.setWindowGeom(width(), height(), 0, 0);
  
  _fullWorld.setWorldLimits(_gd.h_win.cmin_x, _gd.h_win.cmin_y,
                            _gd.h_win.cmax_x, _gd.h_win.cmax_y);
  
  _fullWorld.setLeftMargin(_params.horiz_left_margin);
  _fullWorld.setRightMargin(_params.horiz_right_margin);
  _fullWorld.setTopMargin(_params.horiz_top_margin);
  _fullWorld.setBottomMargin(_params.horiz_bot_margin);
  _fullWorld.setTitleTextMargin(_params.horiz_title_text_margin);
  _fullWorld.setLegendTextMargin(_params.horiz_legend_text_margin);
  _fullWorld.setAxisTextMargin(_params.horiz_axis_text_margin);
  _fullWorld.setColorScaleWidth(_params.horiz_color_scale_width);

  _fullWorld.setXAxisTickLen(_params.horiz_axis_tick_len);
  _fullWorld.setXNTicksIdeal(_params.horiz_n_ticks_ideal);
  _fullWorld.setYAxisTickLen(_params.horiz_axis_tick_len);
  _fullWorld.setYNTicksIdeal(_params.horiz_n_ticks_ideal);

  _fullWorld.setTitleFontSize(_params.horiz_title_font_size);
  _fullWorld.setAxisLabelFontSize(_params.horiz_axis_label_font_size);
  _fullWorld.setTickValuesFontSize(_params.horiz_tick_values_font_size);
  _fullWorld.setLegendFontSize(_params.horiz_legend_font_size);

  _fullWorld.setTitleColor(_params.horiz_title_color);
  _fullWorld.setAxisLineColor(_params.horiz_axes_color);
  _fullWorld.setAxisTextColor(_params.horiz_axes_color);
  _fullWorld.setGridColor(_params.horiz_grid_color);
  
  // initialize the projection
  
  _initProjection();
  
  // set other members
  
  _zoomWorld = _fullWorld;
  _zoomWorld.setName("zoomWorld");
  
  _isZoomed = false;
  // _setTransform(_zoomWorld.getTransform());

  // _setGridSpacing();

  setXyZoom(_zoomWorld.getYMinWorld(),
            _zoomWorld.getYMaxWorld(),
            _zoomWorld.getXMinWorld(),
            _zoomWorld.getXMaxWorld()); 
  
}

////////////////////////////////////////////////////////////////////////
// Used to notify BoundaryPointEditor if the user has zoomed in/out
// or is pressing the Shift key
// Todo: investigate implementing a listener pattern instead

void HorizView::timerEvent(QTimerEvent *event)
{

  bool doUpdate = false;
  this->setCursor(Qt::ArrowCursor);
  
  if (doUpdate) {  //only update if something has changed
    update();
  }
}


/*************************************************************************
 * adjust pixel scale for correct aspect ratio etc
 */
void HorizView::updatePixelScales()
{

  _zoomWorld.setProjection(_gd.proj);
  _zoomWorld.updatePixelScales();
  
}

/*************************************************************************
 * print current time - for debugging
 */

void HorizView::_printNow(int ndecimals,
                          ostream &out)
{

  // Get current system time
  auto now = std::chrono::system_clock::now();
  
  // Convert to time_t for calendar time (date + hour:min:sec)
  auto now_time_t = std::chrono::system_clock::to_time_t(now);
  
  // Extract fractional seconds (microseconds)
  auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
  auto fraction = now_us.time_since_epoch().count() % 1000000; // microseconds part
  
  // Format and print time
  out << std::put_time(std::localtime(&now_time_t), "%H:%M:%S");
  out << "." << std::setw(ndecimals) << std::setfill('0') << fraction << std::endl;
  
}

/*************************************************************************
 * paintEvent()
 */

void HorizView::paintEvent(QPaintEvent *event)
{

  int fieldNum = _gd.h_win.page;

  // check zoom

  XyBox currentZoom(_zoomWorld.getYMinWorld(),
                    _zoomWorld.getYMaxWorld(),
                    _zoomWorld.getXMinWorld(),
                    _zoomWorld.getXMaxWorld());
  if (currentZoom != _zoomXy) {
    _zoomWorld.setWorldLimits(_zoomXy.getMinX(),
                              _zoomXy.getMinY(),
                              _zoomXy.getMaxX(),
                              _zoomXy.getMaxY());
    _zoomChanged = true;
    cerr << "zzzzzzzzzzzzzzzzzzzzzzzzzzz" << endl;
    _zoomWorld.print(cerr);
    cerr << "zzzzzzzzzzzzzzzzzzzzzzzzzzz" << endl;
  } else if (_gd.h_win.zoom_level != _gd.h_win.prev_zoom_level) {
    _zoomWorld.setWorldLimits(_gd.h_win.cmin_x, _gd.h_win.cmin_y,
                              _gd.h_win.cmax_x, _gd.h_win.cmax_y);
    _gd.h_win.prev_zoom_level = _gd.h_win.zoom_level;
    _savedZooms.clear();
    _zoomChanged = true;
    cerr << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZ" << endl;
    _zoomWorld.print(cerr);
    cerr << "ZZZZZZZZZZZZZZZZZZZZZZZZZZZ" << endl;
  }
  
  // render data grids to grid image in WorldPlot

  if (_sizeChanged || _zoomChanged) {
    _renderGrids();
    _renderMaps();
  }

  // render invalid images
  
  // if (_renderInvalidImages) {
  //   _doRenderInvalidImages(_invalidImagesFrameIndex, _vert);
  //   _renderInvalidImages = false;
  // }
  
  // copy rendered grid and map images into this widget
  
  QPainter painter(this);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  if (_gridsReady && _mapsReady) {
    painter.drawImage(0, 0, *_zoomWorld.getGridImage());
    painter.drawImage(0, 0, *_zoomWorld.getMapsImage());
  }

  // render the range rings
  
  if (_params.plot_range_rings_fixed || _params.plot_range_rings_from_data) {
    MdvReader *mr = _gd.mread[fieldNum];
    _zoomWorld.drawRangeRingsHoriz(fieldNum, mr);
    painter.drawImage(0, 0, *_zoomWorld.getRingsImage());
  }
  
  // set axis areas to background color
  
  _zoomWorld.fillMargins(painter, _params.background_color);
  
  // draw axes
  
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  string projUnits("km");
  if (_gd.proj.getProjType() == Mdvx::PROJ_LATLON) {
    projUnits = "deg";
  }
  _zoomWorld.setYAxisLabelsInside(_params.vert_tick_values_inside);
  _zoomWorld.setXAxisLabelsInside(_params.horiz_tick_values_inside);
  _zoomWorld.setAxisLineColor(_params.horiz_axes_color);
  _zoomWorld.setAxisTextColor(_params.horiz_axes_color);
  _zoomWorld.setGridColor(_params.horiz_grid_color);
  _zoomWorld.drawAxisLeft(painter, projUnits, true, true, true, _params.horiz_grids_on_at_startup);
  _zoomWorld.drawAxisTop(painter, projUnits, true, true, false, false);
  _zoomWorld.drawAxisRight(painter, projUnits, true, true, false, false);
  _zoomWorld.drawAxisBottom(painter, projUnits, true, true, true, _params.horiz_grids_on_at_startup);
  
  // draw the color scale

  const ColorMap &colorMap(_gd.mread[fieldNum]->colorMap);
  _zoomWorld.drawColorScale(colorMap, painter, _params.horiz_axis_label_font_size);

  // title
  
  painter.setPen(_params.foreground_color);
  _zoomWorld.drawTitleTopCenter(painter, _params.horiz_frame_label);
  
  // click point cross hairs
  
  if (_pointClicked) {

    painter.save();

    int startX = _mouseReleaseX - _params.click_cross_size / 2;
    int endX = _mouseReleaseX + _params.click_cross_size / 2;
    int startY = _mouseReleaseY - _params.click_cross_size / 2;
    int endY = _mouseReleaseY + _params.click_cross_size / 2;

    QPen pen(painter.pen());
    pen.setColor(_params.click_cross_color);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(_params.click_cross_line_width);
    painter.setPen(pen);

    painter.drawLine(startX, _mouseReleaseY, endX, _mouseReleaseY);
    painter.drawLine(_mouseReleaseX, startY, _mouseReleaseX, endY);
    
    painter.restore();

    char titleStr[1024];
    if (_proj.getProjType() == Mdvx::PROJ_LATLON) {
      snprintf(titleStr, 1023, "Click lat,lon: (%.3f, %.3f)",
               _worldClickLat, _worldClickLon);
    } else {
      snprintf(titleStr, 1023, "Click lat,lon:(%.3f, %.3f) x,y:(%.3f, %.3f)",
               _worldClickLat, _worldClickLon, _worldClickX, _worldClickY);
    }
    _manager.setTitleBarStr(titleStr);

    _manager.showClick();
             
    cerr << "CCCCCCCCCCCCCCC click lat, lon: " << _worldClickLat << ", " << _worldClickLon << endl;
    
  }

  _sizeChanged = false;
  _zoomChanged = false;

}

/*************************************************************************
 * react to click point from remote display - Sprite
 * redraw the click point cursor
 */

void HorizView::setClickPoint(double azimuthDeg,
                              double elevationDeg,
                              double rangeKm)
{

  double x_km =
    rangeKm * sin(azimuthDeg * DEG_TO_RAD) * cos(elevationDeg * DEG_TO_RAD);
  double y_km =
    rangeKm * cos(azimuthDeg * DEG_TO_RAD) * cos(elevationDeg * DEG_TO_RAD);

  _mouseReleaseX = _zoomWorld.getIxPixel(x_km);
  _mouseReleaseY = _zoomWorld.getIyPixel(y_km);
  _pointClicked = true;

  update();

}

/*************************************************************************
 * initialize the geographic projection
 */

void HorizView::_initProjection()
{

  if (_params.proj_type == Params::PROJ_LATLON) {
    _proj.initLatlon(_params.proj_origin_lon);
  } else if (_params.proj_type == Params::PROJ_FLAT) {
    _proj.initFlat(_params.proj_origin_lat,
                   _params.proj_origin_lon,
                   _params.proj_rotation);
  } else if (_params.proj_type == Params::PROJ_LAMBERT_CONF) {
    _proj.initLambertConf(_params.proj_origin_lat,
                          _params.proj_origin_lon,
                          _params.proj_lat1,
                          _params.proj_lat2);
  } else if (_params.proj_type == Params::PROJ_POLAR_STEREO) {
    Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
    if (!_params.proj_pole_is_north) {
      poleType = Mdvx::POLE_SOUTH;
    }
    _proj.initPolarStereo(_params.proj_tangent_lon,
                          poleType,
                          _params.proj_central_scale);
  } else if (_params.proj_type == Params::PROJ_OBLIQUE_STEREO) {
    _proj.initObliqueStereo(_params.proj_origin_lat,
                            _params.proj_origin_lon,
                            _params.proj_tangent_lat,
                            _params.proj_tangent_lon,
                            _params.proj_central_scale);
  } else if (_params.proj_type == Params::PROJ_MERCATOR) {
    _proj.initMercator(_params.proj_origin_lat,
                       _params.proj_origin_lon);
  } else if (_params.proj_type == Params::PROJ_TRANS_MERCATOR) {
    _proj.initTransMercator(_params.proj_origin_lat,
                            _params.proj_origin_lon,
                            _params.proj_central_scale);
  } else if (_params.proj_type == Params::PROJ_ALBERS) {
    _proj.initAlbers(_params.proj_origin_lat,
                     _params.proj_origin_lon,
                     _params.proj_lat1,
                     _params.proj_lat2);
  } else if (_params.proj_type == Params::PROJ_LAMBERT_AZIM) {
    _proj.initLambertAzim(_params.proj_origin_lat,
                          _params.proj_origin_lon);
  }

}

//////////////////////////////////////////
// set flags to control frame rendering

void HorizView::triggerGridRendering(int page, int index)

{

  _renderFramePage = page;
  _renderFrameIndex = index;

  // render data grids to grid image in WorldPlot
  
  _renderGrids();
  
  // render invalid images
  
  if (_renderInvalidImages) {
    _doRenderInvalidImages(_invalidImagesFrameIndex, _vert);
    _renderInvalidImages = false;
  }
  
  update(); // call paint event
  
}
  
//////////////////////////////////////////
// set flags to check for invalid images

void HorizView::setRenderInvalidImages(int index, VertView *vert)

{
  _renderInvalidImages = true;
  _invalidImagesFrameIndex = index;
  _vert = vert;
  update(); // call paint event
}
  
/*************************************************************************
 * _renderGrid()
 */

void HorizView::_renderGrids()
{
  
  if(_gd.debug2) {
    fprintf(stderr,
            "Rendering Horizontal movie_frame %d - field: %d\n",
            _renderFrameIndex, _renderFramePage);
  }

  _controlRenderGrid(_renderFramePage,
                     _gd.movie.frame[_renderFrameIndex].time_start,
                     _gd.movie.frame[_renderFrameIndex].time_end);
  

  _gridsReady = true;
  
}

/************************************************************************
 * RENDER_INVALID_IMAGES: Check for images in which the data 
 * are no longer valid. Look for the "best" invalid  image to 
 *  render.
 *
 */

void HorizView::_doRenderInvalidImages(int index, VertView *vert)
{

#ifdef JUNK
  
  int i;
  int h_image,v_image;
  int stat;
  int none_found = 1;
  QPaintDevice *pdev;
  
  h_image = _gd.h_win.page + 1;
  v_image = _gd.v_win.page + 1;
  if(!_params.run_once_and_exit)  PMU_auto_register("Checking Images (OK)");
  
  /* look through the rest of the images  */
  for (i=0; i < _gd.num_datafields-1; i++) {    
    
    /*
     * Render horizontal image, if necessary.
     */
    
    if (h_image >= _gd.num_datafields) {
      h_image = 0;
    }
    
    if (_gd.mread[h_image]->currently_displayed && _gd.mread[h_image]->auto_render) {
      
      if (_gd.h_win.redraw_flag[h_image] || (_gd.mread[h_image]->h_data_valid == 0)) {
        none_found = 0;
        stat = gather_hwin_data(h_image,
                                _gd.movie.frame[index].time_start,
                                _gd.movie.frame[index].time_end);
        if (stat == CIDD_SUCCESS) {
          if(_gd.mread[h_image]->auto_render) {
            pdev = _gd.h_win.page_pdev[h_image];
          } else {
            pdev = _gd.h_win.tmp_pdev;
          }
          QPainter painter(this);
          _controlRenderGrid(painter,
                            h_image,
                            _gd.movie.frame[index].time_start,
                            _gd.movie.frame[index].time_end);
          
          save_h_movie_frame(index,pdev,h_image);
          
          _gd.h_win.redraw_flag[h_image] = 0;
        } else {
          return;
        }
        if (h_image == _gd.h_win.prev_page && _gd.h_win.redraw_flag[h_image] == 0) {
          _gd.h_copy_flag = 1;
        }
      } // if (_gd.h_win.redraw_flag[h_image] ...
    } // if (_gd.mread[h_image]->currently_displayed ...
    h_image++;

    /*
     * Render vertical image, if necessary.
     */

    if (v_image >= _gd.num_datafields) v_image = 0;

    if (_gd.mread[v_image]->currently_displayed && _gd.mread[v_image]->auto_render) {
      if ((_gd.v_win.active) && (_gd.v_win.redraw_flag[v_image] || (_gd.mread[v_image]->v_data_valid == 0))) {
        stat = gather_vwin_data(v_image, _gd.movie.frame[index].time_start,
                                _gd.movie.frame[index].time_end);
        if (stat == CIDD_SUCCESS) {
          if(_gd.mread[v_image]->auto_render) {
            pdev = _gd.v_win.page_pdev[v_image];
          } else {
            pdev = _gd.v_win.tmp_pdev;
          }
          QPainter painter(this);
#ifdef NOTYET
          vert->renderVertDisplay(painter, v_image, _gd.movie.frame[index].time_start,
                                  _gd.movie.frame[index].time_end);
#endif
          _gd.v_win.redraw_flag[v_image] = 0;
        } else {
          return;
        }
        if (v_image == _gd.v_win.prev_page && _gd.v_win.redraw_flag[v_image] == 0) _gd.v_copy_flag = 1;
      }
    }
        
    v_image++;
  }

  // At this point all background images have been rendered. and nothing else is
  //  happening

  // In html mode, cycle through all zooms and heights
  if(none_found && _params.html_mode && _gd.io_info.outstanding_request == 0) {
    
    /* If more zoom levels to render */
    if(_gd.h_win.zoom_level < (_gd.h_win.num_zoom_levels -  NUM_CUSTOM_ZOOMS - 2)) {

      /* Set zoom to next level */
      _gd.h_win.zoom_level++;
      // set_domain_proc(_gd.zoom_pu->domain_st,_gd.h_win.zoom_level,NULL);

      // If more heights to render
    } else if (_gd.cur_render_height < _gd.num_render_heights -1) {

      // Set height to next level
      _gd.cur_render_height++;
      if(_gd.debug) fprintf(stderr,"HTML_MODE: Height now: %g\n",_gd.h_win.cur_ht);
      _gd.h_win.cur_ht = _gd.height_array[_gd.cur_render_height];

      // Reset Zoom back to first  level
      _gd.h_win.zoom_level = 0;
      // set_domain_proc(_gd.zoom_pu->domain_st,_gd.h_win.zoom_level,NULL);

      // Make sure new data gets loaded
      reset_data_valid_flags(1,0);
      reset_terrain_valid_flags(1,0);
               
      // No more heights and no more zooms to render
    } else if(_params.run_once_and_exit) {
      if(!_gd.quiet_mode)  fprintf(stderr,"Exiting\n");
      // xv_destroy(_gd.h_win_horiz_bw->horiz_bw);
      exit(-1);
    }
  }
#endif
  return;
}

/**********************************************************************
 * RENDER_HORIZ_DISPLAY: Render a complete horizontal plane of data a
 *        and its associated overlays and labels  labels. 
 */

int HorizView::_controlRenderGrid(int page,
                                  time_t start_time,
                                  time_t end_time)
{

  if(!_params.run_once_and_exit)  PMU_auto_register("Rendering (OK)");
  if(_gd.debug2) fprintf(stderr,"Rendering Plan View Image, page :%d\n",page);

  // compute distance across the image for setting font sizes, etc.
  switch(_gd.display_projection) {
    default:
    case Mdvx::PROJ_FLAT :
    case Mdvx::PROJ_LAMBERT_CONF :
      /* compute km across the image */
      _gd.h_win.km_across_screen = (_gd.h_win.cmax_x - _gd.h_win.cmin_x);
      break;

    case Mdvx::PROJ_LATLON :
      _gd.h_win.km_across_screen = (_gd.h_win.cmax_x - _gd.h_win.cmin_x) * KM_PER_DEG_AT_EQ;
      break;
  }

  MdvReader *mr = _gd.mread[page];
 
  // Clear time lists
  // if(_gd.time_plot) _gd.time_plot->clear_grid_tlist();
  // if(_gd.time_plot) _gd.time_plot->clear_prod_tlist();

  // if(_params.show_data_messages) {
  //   // gui_label_h_frame("Rendering",-1);
  // }

  // RENDER the LAND_USE field first
  if(_gd.layers.earth.landuse_active && _gd.layers.earth.land_use != NULL) {
    _renderGrid(page, _gd.layers.earth.land_use, start_time, end_time,1);
    // render_grid(xid,_gd.layers.earth.land_use,start_time,end_time,1);
  }

  if(!_params.draw_main_on_top) {
    if(mr->render_method == LINE_CONTOURS) {
#ifdef NOTYET
      contour_info_t cont; // contour params 
      cont.min = mr->cont_low;
      cont.max = mr->cont_high;
      cont.interval = mr->cont_interv;
      cont.active = 1;
      cont.field = page;
      cont.labels_on = _params.label_contours;
      cont.color = _gd.legends.foreground_color;
      cont.vcm = &mr->h_vcm;
      if (_gd.layers.use_alt_contours) {
        RenderLineContours(xid,&cont);
      } else {
        render_line_contours(xid,&cont);
      }
#endif
    } else {
      _renderGrid(page, mr, start_time, end_time, 0);
    }
    if(_gd.layers.earth.terrain_active && 
       ((mr->vert[mr->ds_fhdr.nz -1].max - mr->vert[0].min) != 0.0) &&
       (mr->composite_mode == FALSE) && (mr->ds_fhdr.nz > 1) &&
       mr->ds_fhdr.vlevel_type != Mdvx::VERT_TYPE_ELEV) {
#ifdef HAVE_XID
      render_h_terrain(xid, page);
#endif
    }
  }
     
  /* Render each of the gridded_overlay fields */
  for(int i=0; i < Constants::NUM_GRID_LAYERS; i++) {
    if(_gd.layers.overlay_field_on[i] && _gd.mread[_gd.layers.overlay_field[i]] != NULL) {
      _renderGrid(page, _gd.mread[_gd.layers.overlay_field[i]],start_time,end_time,1);
      // render_grid(xid,_gd.mread[_gd.layers.overlay_field[i]],start_time,end_time,1);
    }
  }
  
  if(_params.draw_main_on_top) {
    if(mr->render_method == LINE_CONTOURS) {
#ifdef NOTYET
      contour_info_t cont; // contour params 
      cont.min = mr->cont_low;
      cont.max = mr->cont_high;
      cont.interval = mr->cont_interv;
      cont.active = 1;
      cont.field = page;
      cont.labels_on = _params.label_contours;
      cont.color = _gd.legends.foreground_color;
      cont.vcm = &mr->h_vcm;
      if (_gd.layers.use_alt_contours) {
        RenderLineContours(xid,&cont);
      } else {
        render_line_contours(xid,&cont);
      }
#endif
    } else {
      _renderGrid(page, mr, start_time, end_time, 0);
      // render_grid(xid,mr,start_time,end_time,0);
    }
    if(_gd.layers.earth.terrain_active && 
       ((mr->vert[mr->ds_fhdr.nz -1].max - mr->vert[0].min) != 0.0) &&
       (mr->composite_mode == FALSE) && (mr->ds_fhdr.nz > 1)) {

#ifdef HAVE_XID
      render_h_terrain(xid, page);
#endif
    }
  }

  /* render contours if selected */
  for(int i= 0; i < Constants::NUM_CONT_LAYERS; i++) {
    if(_gd.layers.cont[i].active) {
      if (_gd.layers.use_alt_contours) {
#ifdef HAVE_XID
        RenderLineContours(xid, &(_gd.layers.cont[i]));
#endif
      } else {
#ifdef HAVE_XID
        render_line_contours(xid, &(_gd.layers.cont[i]));
#endif
      }
    }
  }

  /* render Winds if selected */
  if(_gd.layers.wind_vectors) {
    switch(_gd.layers.wind_mode) {
      default:
      case WIND_MODE_ON:  /* winds get rendered in each frame */
#ifdef HAVE_XID
        render_wind_vectors(xid,start_time,end_time);
#endif
        break;
        
      case WIND_MODE_LAST: /* Winds get rendered in last farame only */
        if(_gd.movie.cur_frame == _gd.movie.end_frame)
#ifdef HAVE_XID
          render_wind_vectors(xid,start_time,end_time);
#endif
        break;

      case WIND_MODE_STILL: /* Winds get rendered in the last frame only
                             * if the movie loop is off
                             */
        if(!_gd.movie.movie_on && _gd.movie.cur_frame == _gd.movie.end_frame)
#ifdef HAVE_XID
          render_wind_vectors(xid,start_time,end_time);
#endif
        break;
    }
  }


#ifdef HAVE_XID
  render_top_layers(xid);  // Range rings X section reference etc.
#endif

  // Native Symprod products.
#ifdef HAVE_XID
  render_products(xid,start_time,end_time);
#endif

#ifdef HAVE_XID
  render_horiz_margins(xid,page,start_time,end_time);
#endif

  // update_frame_time_msg(_gd.movie.cur_frame);

  return Constants::CIDD_SUCCESS;    /* avaliable data has been rendered */
}

/**********************************************************************
 * RENDER_GRID: Render a horizontal plane of  gridded data 
 *    Returns 1 on success, 0 on failure
 */

int HorizView::_renderGrid(int page,
                           MdvReader *mr,
                           time_t start_time,
                           time_t end_time,
                           bool is_overlay_field)
{
  
  if (mr == NULL) {
    return Constants::CIDD_SUCCESS;
  }
  
  // Render with appropriate rendering routine
  
  Mdvx::Mdvx::projection_type_t projType = mr->proj.getProjType();
  const PjgMath &dataMath = mr->proj.getPjgMath();
  const PjgMath &displayMath = _gd.proj.getPjgMath();
  
  if (_gd.debug) {
    cerr << "-->> data projection <<--" << endl;
    dataMath.print(cerr);
    cerr << "-->> display projection <<--" << endl;
    displayMath.print(cerr);
    cerr << "----------------------------" << endl;
  }

  if (projType == Mdvx::PROJ_POLAR_RADAR) {
    _zoomWorld.renderGridRadarPolar(page, mr,
                                    start_time, end_time,
                                    is_overlay_field);
  } else if (dataMath == displayMath) {
    _zoomWorld.renderGridRect(page, mr,
                              start_time, end_time,
                              is_overlay_field);
  } else {
    _zoomWorld.renderGridDistorted(page, mr,
                                   start_time, end_time,
                                   is_overlay_field);
  }

#ifdef NOTYET
  
  int out_of_date = 0;
  int stretch_secs = (int) (60.0 * mr->time_allowance);
  char message[1024];    /* Error message area */
  
  if(_params.check_data_times) {
    if(mr->h_date.unix_time < start_time - stretch_secs) out_of_date = 1;
    if(mr->h_date.unix_time > end_time + stretch_secs) out_of_date = 1;
  }

  // Add the list of times to the time plot
  // if(mr->time_list.num_entries > 0) {
  //   if(_gd.time_plot) _gd.time_plot->add_grid_tlist(mr->legend_name,mr->time_list.tim,
  //                                                 mr->time_list.num_entries,
  //                                                 mr->h_date.unix_time);
  // }

  /* For the Main Field - Clear the screen and  Print a special message 
   * and draw the wall clock time if the data is not availible 
   */
  if(!is_overlay_field) {


    /* If no data in current response or data is way out of date */
    if( mr->h_data == NULL || out_of_date ) {

      if(strncasecmp(mr->button_name,"None",4) &&
         strncasecmp(mr->button_name,"Empty",5)) {
        /* display "Data Not Available" message */
        if(out_of_date) {
          snprintf(message, 1023, "%s - Data too Old", _params.no_data_message);
        } else {
          STRcopy(message, _params.no_data_message, 1023);
        }

        int xmid,ymid;
        Font font = choose_font(message, _gd.h_win.img_dim.width, _gd.h_win.img_dim.height, &xmid, &ymid);
        XSetFont(_gd.dpy,_gd.legends.foreground_color->gc,font);
        XDrawImageString(_gd.dpy,xid,_gd.legends.foreground_color->gc,
                         _gd.h_win.margin.left + (_gd.h_win.img_dim.width /2) + xmid  ,
                         _gd.h_win.margin.top + (_gd.h_win.img_dim.height /4) + ymid ,
                         message,strlen(message));

        if(_gd.debug2) {
          fprintf(stderr, "No data from service: %s\n",
                  _gd.io_info.mr->url );
        }

	if(_params.show_clock) {
          /* draw a clock */
          int ht = (int) (_gd.h_win.can_dim.height * 0.05);
          int startx = _gd.h_win.can_dim.width - _gd.h_win.margin.right - ht - 5;
          int starty = _gd.h_win.margin.top + ht + 5;
          XUDRdraw_clock(_gd.dpy,xid,_gd.legends.foreground_color->gc,
                         startx,starty,ht, (start_time + (end_time - start_time) /2),1);
	}
      }  // if mr->button_name != "None"

      return CIDD_FAILURE;
    }
  }
     
  set_busy_state(1);

#endif

#ifdef NOTNOW
  switch(mr->h_fhdr.proj_type) {
    default: // Projections which need only matching types and origins.
      // If the projections match - Can use fast Rectangle rendering.
      if(mr->h_fhdr.proj_type == _gd.proj.getProjType() &&
         (fabs(_gd.h_win.origin_lat - mr->h_fhdr.proj_origin_lat) < 0.001) &&
         (fabs(_gd.h_win.origin_lon - mr->h_fhdr.proj_origin_lon) < 0.001)) {
        if(_gd.debug2) fprintf(stderr,"renderGrid() selected\n");
        _zoomWorld.renderGridRect(page, mr,
                                  start_time, end_time,
                                  is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGrid() done\n");
      } else { // Must use polygon rendering
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() selected\n");
        _zoomWorld.renderGridDistorted(page, mr,
                                       start_time, end_time,
                                       is_overlay_field);
      }
      break;

    case  Mdvx::PROJ_FLAT: // Needs to test Param 1
      // If the projections match - Can use fast Rectangle rendering.
      if(mr->h_fhdr.proj_type == _gd.proj.getProjType() &&
         (fabs(_gd.proj_param[0] - mr->h_fhdr.proj_param[0]) < 0.001) &&
         (fabs(_gd.h_win.origin_lat - mr->h_fhdr.proj_origin_lat) < 0.001) &&
         (fabs(_gd.h_win.origin_lon - mr->h_fhdr.proj_origin_lon) < 0.001)) {
        
        if(_gd.debug2) fprintf(stderr,"renderGrid() selected\n");
        _zoomWorld.renderGridRect(page, mr,
                                  start_time, end_time,
                                  is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGrid() done\n");
      } else { // Must use polygon rendering
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() selected\n");
        _zoomWorld.renderGridDistorted(page, mr,
                                       start_time, end_time,
                                       is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() done\n");
      }
      break;
      
    case  Mdvx::PROJ_LAMBERT_CONF: // Needs to test param 1 & 2
    case  Mdvx::PROJ_POLAR_STEREO: 
    case  Mdvx::PROJ_OBLIQUE_STEREO:
    case  Mdvx::PROJ_MERCATOR:
      // If the projections match - Can use fast Rectangle rendering.
      if(mr->h_fhdr.proj_type == _gd.proj.getProjType() &&
         (fabs(_gd.proj_param[0] - mr->h_fhdr.proj_param[0]) < 0.001) &&
         (fabs(_gd.proj_param[1] - mr->h_fhdr.proj_param[1]) < 0.001) &&
         (fabs(_gd.h_win.origin_lat - mr->h_fhdr.proj_origin_lat) < 0.001) &&
         (fabs(_gd.h_win.origin_lon - mr->h_fhdr.proj_origin_lon) < 0.001)) {
        if(_gd.debug2) fprintf(stderr,"renderGrid() selected\n");
        _zoomWorld.renderGridRect(page, mr,
                                  start_time, end_time,
                                  is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGrid() done\n");
      } else { // Must use polygon rendering
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() selected\n");
        _zoomWorld.renderGridDistorted(page, mr,
                                       start_time, end_time,
                                       is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() done\n");
      }
      break;
      
    case  Mdvx::PROJ_LATLON:
      if(mr->h_fhdr.proj_type == _gd.proj.getProjType()) {
        if(_gd.debug2) fprintf(stderr,"renderGrid() selected\n");
        _zoomWorld.renderGridRect(page, mr,
                                  start_time, end_time,
                                  is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGrid() done\n");
      } else {
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() selected\n");
        _zoomWorld.renderGridDistorted(page, mr,
                                       start_time, end_time,
                                       is_overlay_field);
        if(_gd.debug2) fprintf(stderr,"renderGridDistorted() done\n");
      }
      break;
      
    case  Mdvx::PROJ_POLAR_RADAR:
      if(_gd.debug2) fprintf(stderr,"render_polar_grid() selected\n");
#ifdef NOTYET
      render_polar_grid(page, mr,start_time, end_time, 
                        is_overlay_field);
#endif
      if(_gd.debug2) fprintf(stderr,"render_polar_grid() done\n");
      break;
      
  }

  set_busy_state(0);

#endif

  return Constants::CIDD_SUCCESS;
}

/*************************************************************************
 * set archive mode
 */

void HorizView::setArchiveMode(bool state)
{
  _archiveMode = state;
}

/*************************************************************************
 * zoomBack the view
 */

void HorizView::zoomBackView()
{

  if (_savedZooms.size() == 0) {
    zoomOutView();
    return;
  }
  
  _zoomWorld = _savedZooms[_savedZooms.size()-1];
  // _zoomWorld.setName("fromSavedZoom");
  _savedZooms.pop_back();
  if (_savedZooms.size() == 0) {
    _isZoomed = false;
  }
  setXyZoom(_zoomWorld.getYMinWorld(),
            _zoomWorld.getYMaxWorld(),
            _zoomWorld.getXMinWorld(),
            _zoomWorld.getXMaxWorld()); 
}

/*************************************************************************
 * zoom all the way out
 */

void HorizView::zoomOutView()
{
  _zoomWorld = _fullWorld;
  _zoomWorld.setName("zoomWorld");
  _savedZooms.clear();
  _isZoomed = false;
  setXyZoom(_zoomWorld.getYMinWorld(),
            _zoomWorld.getYMaxWorld(),
            _zoomWorld.getXMinWorld(),
            _zoomWorld.getXMaxWorld()); 
}

//////////////////////////////////////////////////
// respond to zoom action, set the XY zoom limits

void HorizView::setXyZoom(double minY, double maxY,
                          double minX, double maxX)
{
  _prevZoomXy = _zoomXy;
  _zoomXy.setLimits(minY, maxY, minX, maxX);
  _gd.h_win.cmin_y = minY;
  _gd.h_win.cmax_y = maxY;
  _gd.h_win.cmin_x = minX;
  _gd.h_win.cmax_x = maxX;
  _zoomChanged = true;
  update();
}

/////////////////////////////////////////////////////////
// check for zoom change

bool HorizView::checkForZoomChange()
{
  if (_zoomXy != _prevZoomXy) {
    _prevZoomXy = _zoomXy;
    return true;
  }
  return false;
}

/*************************************************************************
 * setRings()
 */

void HorizView::setRingsFixed(const bool enabled)
{
  _params.plot_range_rings_fixed = (enabled?pTRUE:pFALSE);
  _manager.setOverlaysHaveChanged(true);
}

void HorizView::setRingsDataDriven(const bool enabled)
{
  _params.plot_range_rings_from_data = (enabled?pTRUE:pFALSE);
  _manager.setOverlaysHaveChanged(true);
}


/*************************************************************************
 * setGrids()
 */

void HorizView::setGrids(const bool enabled)
{
  _params.horiz_grids_on_at_startup = (enabled?pTRUE:pFALSE);
  _manager.setOverlaysHaveChanged(true);
}


/*************************************************************************
 * getImage()
 */

QImage* HorizView::getImage()
{
  QPixmap pixmap = grab();
  QImage* image = new QImage(pixmap.toImage());
  return image;
}


/*************************************************************************
 * getPixmap()
 */

QPixmap* HorizView::getPixmap()
{
  QPixmap* pixmap = new QPixmap(grab());
  return pixmap;
}


/*************************************************************************
 * Slots
 *************************************************************************/

/*************************************************************************
 * mousePressEvent()
 */

void HorizView::mousePressEvent(QMouseEvent *e)
{

#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  if (e->button() == Qt::RightButton) {
    
    //-------

    _mousePressX = pos.x();
    _mousePressY = pos.y();

    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);

    emit customContextMenuRequested(pos.toPoint()); // , closestRay);

  } else {

    if (!_rubberBand) {
      _rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    }
    _rubberBand->setGeometry(pos.x(), pos.y(), 0, 0);
    _rubberBand->show();

    _mousePressX = pos.x();
    _mousePressY = pos.y();

    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);
  }
}


/*************************************************************************
 * mouseMoveEvent(), mouse button is down and mouse is moving
 */

void HorizView::mouseMoveEvent(QMouseEvent * e)
{

  // Zooming with the mouse

#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  int x = pos.x();
  int y = pos.y();
  int deltaX = x - _mousePressX;
  int deltaY = y - _mousePressY;

  // Make the rubberband aspect ratio match that
  // of the window

  double aspectRatio =
    (double) _zoomWorld.getPlotHeight() / (double) _zoomWorld.getPlotWidth();
  
  double dx = fabs(deltaY / aspectRatio);
  double dy = fabs(dx * aspectRatio);

  // Preserve the signs

  dx *= fabs(deltaX)/deltaX;
  dy *= fabs(deltaY)/deltaY;

  int moveX = (int) floor(dx + 0.5);
  int moveY = (int) floor(dy + 0.5);

  QRect newRect = QRect(_mousePressX, _mousePressY, moveX, moveY);

  _zoomCornerX = _mousePressX + moveX;
  _zoomCornerY = _mousePressY + moveY;

  newRect = newRect.normalized();

  if (_rubberBand) {
    _rubberBand->setGeometry(newRect);
  }

}

/*************************************************************************
 * mouseReleaseEvent()
 */
void HorizView::mouseReleaseEvent(QMouseEvent *e)
{

  _pointClicked = false;

  // If the mouse hasn't moved much, assume we are clicking rather than
  // zooming

#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  _mouseReleaseX = pos.x();
  _mouseReleaseY = pos.y();

  double distX = _mouseReleaseX - _mousePressX;
  double distY = _mouseReleaseY - _mousePressY;
  double distMoved = sqrt(distX * distX + distY * distY);
  
  if (distMoved <= 20) {
    
    // mouse moved less than 20 pixels, so we interpret that as a click
    // get click location in world coords
    
    _worldReleaseX = _zoomWorld.getXWorld(_mouseReleaseX);
    _worldReleaseY = _zoomWorld.getYWorld(_mouseReleaseY);

    _pointClicked = true;
    _worldClickX = _worldReleaseX;
    _worldClickY = _worldReleaseY;
    _proj.xy2latlon(_worldClickX, _worldClickY, _worldClickLat, _worldClickLon);
    _manager.setOverlaysHaveChanged(true);
    
    // Emit a signal to indicate that the click location has changed

    emit locationClicked(_worldClickX, _worldClickY);
    
    update();

  } else {

    // mouse moved more than 20 pixels, so a zoom occurred

    // save current zoom
    
    char zoomName[1024];
    snprintf(zoomName, 1024, "saved-zoom-%d", (int) _savedZooms.size());
    _savedZooms.push_back(_zoomWorld);
    _savedZooms[_savedZooms.size()-1].setName(zoomName);

    // handle the zoom action

    _handleMouseZoom();
    
    // render
    
  }
    
  // hide the rubber band

  if (_rubberBand) {
    _rubberBand->hide();
  }

  return;
  
}

/*************************************************************************
 * handle zoom event
 */

void HorizView::_handleMouseZoom()
{

  _worldPressX = _zoomWorld.getXWorld(_mousePressX);
  _worldPressY = _zoomWorld.getYWorld(_mousePressY);
  _worldReleaseX = _zoomWorld.getXWorld(_zoomCornerX);
  _worldReleaseY = _zoomWorld.getYWorld(_zoomCornerY);
  
  // update world coords transform
  
  _zoomWorld.setWorldLimits(_worldPressX, _worldPressY,
                            _worldReleaseX, _worldReleaseY);
  
  _zoomChanged = true;
  cerr << "xxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
  _zoomWorld.print(cerr);
  cerr << "xxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
  
  updatePixelScales();
  
  // enable unzooms
  
  _manager.enableUnzoomButtons(true);
  
  setXyZoom(_zoomWorld.getYMinWorld(),
            _zoomWorld.getYMaxWorld(),
            _zoomWorld.getXMinWorld(),
            _zoomWorld.getXMaxWorld());
  
}

/*************************************************************************
 * resizeEvent()
 */

void HorizView::resizeEvent(QResizeEvent * e)
{

  _resetWorld(width(), height());
  _pixmap = _pixmap.scaled(width(), height());
  updatePixelScales();
  _sizeChanged = true;
  
}


/*************************************************************************
 * resize()
 */

void HorizView::resize(const int width, const int height)
{

  setGeometry(0, 0, width, height);
  _sizeChanged = true;
  
}

//////////////////////////////////////////////////////////////
// reset the pixel size of the world view

void HorizView::_resetWorld(int width, int height)

{

  _fullWorld.resize(width, height);
  _zoomWorld = _fullWorld;
  _zoomWorld.setName("zoomWorld");
  _sizeChanged = true;

}

/*************************************************************************
 * Protected methods
 *************************************************************************/

/*************************************************************************
 * render the maps
 */

void HorizView::_renderMaps()
{

  _zoomWorld.drawMaps();
  _mapsReady = true;
  
}

void HorizView::informationMessage()
{
  
  // QMessageBox::StandardButton reply;
  // QLabel *informationLabel;
  
  // reply = QMessageBox::information(this, "QMessageBox::information()", "Not implemented");
  QMessageBox::information(this, "QMessageBox::information()", "Not implemented");
  //  if (reply == QMessageBox::Ok)
  //  informationLabel->setText("OK");
  //else
  //  informationLabel->setText("Escape");

}

/////////////////////////////////////////////////////////////////////////////////////
// slots for context editing; create and show the associated modeless dialog and return                                   

void HorizView::contextMenuCancel()
{
  // informationMessage();
  // notImplemented();                                                                                                     
}

void HorizView::contextMenuParameterColors()
{

#ifdef NOTNOW
  
  LOG(DEBUG_VERBOSE) << "enter";

  //DisplayField selectedField;                                                                             

  // const DisplayField &field = _manager.getSelectedField();
  // const ColorMap &colorMapForSelectedField = field.getColorMap();
  ParameterColorView *parameterColorView = new ParameterColorView(this);
  vector<DisplayField *> displayFields = _manager.getDisplayFields(); // TODO: I guess, implement this as a signal and a slot? // getDisplayFields();
  DisplayField selectedField = _manager.getSelectedField();
  string emphasis_color = "white";
  string annotation_color = "white";
  DisplayFieldModel *displayFieldModel = 
    new DisplayFieldModel(displayFields, selectedField.getName(),
			  _params.grid_and_range_ring_color,
			  emphasis_color,
			  annotation_color,
			  _params.background_color);
  FieldColorController *fieldColorController = new FieldColorController(parameterColorView, displayFieldModel);
  // connect some signals and slots in order to retrieve information
  // and send changes back to display
                                                                         
  //  connect(parameterColorView, SIGNAL(retrieveInfo), &_manager, SLOT(InfoRetrieved()));
  connect(fieldColorController, SIGNAL(colorMapRedefineSent(string, ColorMap, QColor, QColor, QColor, QColor)),
  	  &_manager, SLOT(colorMapRedefineReceived(string, ColorMap, QColor, QColor, QColor, QColor))); // THIS IS NOT CALLED!!
  //  GuiManager::colorMapRedefineReceived(string, ColorMap)
  //connect(fieldColorController, SIGNAL(colorMapRedefined(string)),
  //	  this, SLOT(changeToDisplayField(string))); // THIS IS NOT CALLED!!

  /* TODO: combine with replot
     connect(fieldColorController, SIGNAL(backgroundColorSet(QColor)),
     this, SLOT(backgroundColor(QColor)));
  */

  fieldColorController->startUp(); 

  //connect(parameterColorView, SIGNAL(needFieldNames()), this, SLOT(getFieldNames()));
  //connect(this, SIGNAL(fieldNamesSupplied(vector<string>)), 
  //  parameterColorView, SLOT(fieldNamesSupplied(vector<string>));
  // TODO: move this call to the controller?                                                                
  // parameterColorView.exec();

  //  if(parameterColorController.Changes()) {
  // TODO: what are changes?  new displayField(s)?                                                        
  //}
 
  LOG(DEBUG_VERBOSE) << "exit ";

#endif
  
  informationMessage();
  
}

void HorizView::contextMenuView()
{
  informationMessage();
  //  notImplemented();                                                                                                   
}

void HorizView::contextMenuExamine()         
{
  informationMessage();                                                                                                 

}

void HorizView::contextMenuDataWidget()
{
  informationMessage();
  //  notImplemented();                                                                                                   
}

void HorizView::contextMenuEditor()
{
  LOG(DEBUG_VERBOSE) << "enter";

  // get click location in world coords
  // by using the location stored in class variables
#ifdef JUNK
  double x_km = _worldPressX;
  double y_km = _worldPressY;

  // get ray closest to click point
  const RadxRay *closestRay = _getClosestRay(x_km, y_km);
  // TODO: make sure the point is in the valid area
  if (closestRay == NULL) {
    // report error
    QMessageBox::information(this, QString::fromStdString(""), QString::fromStdString("No ray found at location clicked"));
    // TODO: move to this ...  errorMessage("", "No ray found at location clicked");
  } else {
    // ExamineEdit(closestRay);
  }
  LOG(DEBUG_VERBOSE) << "exit";
#endif
  
}

void HorizView::ShowContextMenu(const QPoint &pos/* , RadxVol *vol */)
{

  // _vol = vol;

  QMenu contextMenu("Context menu", this);
  
  QAction action1("Cancel", this);
  connect(&action1, SIGNAL(triggered()), this, SLOT(contextMenuCancel()));
  contextMenu.addAction(&action1);

  QAction action3("Parameters + Colors", this);
  connect(&action3, SIGNAL(triggered()), this, SLOT(contextMenuParameterColors()));
  contextMenu.addAction(&action3);

  QAction action4("View", this);
  connect(&action4, SIGNAL(triggered()), this, SLOT(contextMenuView()));
  contextMenu.addAction(&action4);

  QAction action5("Editor", this);
  connect(&action5, SIGNAL(triggered()), this, SLOT(contextMenuEditor()));
  contextMenu.addAction(&action5);
  
  QAction action6("Examine", this);
  connect(&action6, SIGNAL(triggered()), this, SLOT(contextMenuExamine()));
  contextMenu.addAction(&action6);

  /*
    QAction action7("Data Widget", this);
    connect(&action7, SIGNAL(triggered()), this, SLOT(contextMenuDataWidget()));
    contextMenu.addAction(&action7);
  */

  contextMenu.exec(this->mapToGlobal(pos));
}

  
