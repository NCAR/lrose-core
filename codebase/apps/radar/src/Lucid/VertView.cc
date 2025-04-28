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

#include <assert.h>
#include <cmath>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include <QTimer>
#include <QBrush>
#include <QPalette>
#include <QPaintEngine>
#include <QPen>
#include <QResizeEvent>
#include <QStylePainter>
#include <QToolTip>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QErrorMessage>

#include <toolsa/toolsa_macros.h>
#include <toolsa/uusleep.h>
#include <toolsa/LogStream.hh>

#include "GuiManager.hh"
#include "VertView.hh"
#include "VertManager.hh"
#include "GlobalData.hh"

using namespace std;

const double VertView::SIN_45 = sin(45.0 * DEG_TO_RAD);
const double VertView::SIN_30 = sin(30.0 * DEG_TO_RAD);
const double VertView::COS_30 = cos(30.0 * DEG_TO_RAD);

VertView::VertView(QWidget* parent,
                   const GuiManager &manager,
                   const VertManager &vertWindow) :
        QWidget(parent),
        _parent(parent),
        _manager(manager),
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _vertWindow(vertWindow),
        _selectedField(0),
        _backgroundBrush(QColor(_params.background_color)),
        _gridsEnabled(false),
        _rubberBand(0)
        
{
  
  // mode

  _archiveMode = _params.start_mode == Params::MODE_ARCHIVE;

  // Set up the background color

  QPalette new_palette = palette();
  new_palette.setColor(QPalette::Dark, _backgroundBrush.color());
  setPalette(new_palette);
  
  setBackgroundRole(QPalette::Dark);
  setAutoFillBackground(true);
  setAttribute(Qt::WA_OpaquePaintEvent);
  
  // Allow the widget to get focus
  
  setFocusPolicy(Qt::StrongFocus);

  // create the rubber band
  
  _rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

  // Allow the size_t type to be passed to slots

  qRegisterMetaType<size_t>("size_t");

  // init other values

  _worldPressX = 0.0;
  _worldPressY = 0.0;
  _worldReleaseX = 0.0;
  _worldReleaseY = 0.0;
  _pointClicked = false;
  _mousePressX = 0;
  _mousePressY = 0;
  _mouseReleaseX = 0;
  _mouseReleaseY = 0;
  _zoomCornerX = 0;
  _zoomCornerY = 0;
  
  _prevElev = -9999.0;
  _prevAz = -9999.0;
  _prevTime.setToZero();
  
  _colorScaleWidth = _params.vert_color_scale_width;

  setGrids(_params.vert_grids_on_at_startup);
  
  // initialize world view
  
  _maxHeightKm = _params.vert_max_height_km;
  _xGridSpacing = 0.0;
  _yGridSpacing = 0.0;

  // archive mode

  _isArchiveMode = false;
  _isStartOfSweep = true;
  
  _plotStartTime.setToZero();
  _plotEndTime.setToZero();
  
}


/*************************************************************************
 * Destructor
 */

VertView::~VertView()
{

}

/*************************************************************************
 * configureWorldCoords()
 */

void VertView::configureWorldCoords(int zoomLevel)
{

  // Set the ring spacing.  This is dependent on the value of _maxRange.

  _setGridSpacing();
  
  // set world view

  int leftMargin = _params.vert_left_margin;
  int rightMargin = _params.vert_right_margin;
  int topMargin = _params.vert_top_margin;
  int bottomMargin = _params.vert_bottom_margin;
  int colorScaleWidth = _params.vert_color_scale_width;
  int axisTickLen = _params.vert_axis_tick_len;
  int nTicksIdeal = _params.vert_n_ticks_ideal;
  int titleTextMargin = _params.vert_title_text_margin;
  int legendTextMargin = _params.vert_legend_text_margin;
  int axisTextMargin = _params.vert_axis_text_margin;
  
  _fullWorld.setName("VertView-full");
  _fullWorld.setWindowGeom(width(), height(), 0, 0);
  
  _fullWorld.setWorldLimits(0.0, 0.0, 100.0, _maxHeightKm);
  
  _fullWorld.setLeftMargin(leftMargin);
  _fullWorld.setRightMargin(rightMargin);
  _fullWorld.setTopMargin(topMargin);
  _fullWorld.setBottomMargin(bottomMargin);
  _fullWorld.setTitleTextMargin(titleTextMargin);
  _fullWorld.setLegendTextMargin(legendTextMargin);
  _fullWorld.setAxisTextMargin(axisTextMargin);
  _fullWorld.setColorScaleWidth(colorScaleWidth);

  _fullWorld.setXAxisTickLen(axisTickLen);
  _fullWorld.setXNTicksIdeal(nTicksIdeal);
  _fullWorld.setYAxisTickLen(axisTickLen);
  _fullWorld.setYNTicksIdeal(nTicksIdeal);

  _fullWorld.setTitleFontSize(_params.vert_title_font_size);
  _fullWorld.setAxisLabelFontSize(_params.vert_axis_label_font_size);
  _fullWorld.setTickValuesFontSize(_params.vert_tick_values_font_size);
  _fullWorld.setLegendFontSize(_params.vert_legend_font_size);

  _fullWorld.setTitleColor(_params.vert_title_color);
  _fullWorld.setAxisLineColor(_params.vert_axes_color);
  _fullWorld.setAxisTextColor(_params.vert_axes_color);
  _fullWorld.setGridColor(_params.vert_grid_color);

  _zoomWorld = _fullWorld;
  _zoomWorld.setName("VertView-zoomed");
  
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());

  _setGridSpacing();

  // Initialize the images used for double-buffering.  For some reason,
  // the window size is incorrect at this point, but that will be corrected
  // by the system with a call to resize().

  _refreshImages();
  
}

/*************************************************************************
 * mouseReleaseEvent()
 */

void VertView::mouseReleaseEvent(QMouseEvent *e)
{

  _pointClicked = false;

  QRect rgeom = _rubberBand->geometry();

  // If the mouse hasn't moved much, assume we are clicking rather than
  // zooming

#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  _mouseReleaseX = pos.x();
  _mouseReleaseY = pos.y();

  // get click location in world coords

  if (rgeom.width() <= 20) {
    
    // Emit a signal to indicate that the click location has changed
    
    _worldReleaseX = _zoomWorld.getXWorld(_mouseReleaseX);
    _worldReleaseY = _zoomWorld.getYWorld(_mouseReleaseY);

    double x_km = _worldReleaseX;
    double y_km = _worldReleaseY;
    _pointClicked = true;

    // get ray closest to click point

    const RadxRay *closestRay = _getClosestRay(x_km, y_km);
    
    // emit signal

    emit locationClicked(x_km, y_km, closestRay);
  
  } else {

    // mouse moved more than 20 pixels, so a zoom occurred
    
    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);

    _worldReleaseX = _zoomWorld.getXWorld(_zoomCornerX);
    _worldReleaseY = _zoomWorld.getYWorld(_zoomCornerY);

    _zoomWorld.setWorldLimits(_worldPressX, _worldPressY, _worldReleaseX, _worldReleaseY);

    _setTransform(_zoomWorld.getTransform());

    _setGridSpacing();

    // enable unzoom button

    _vertWindow.enableZoomButton();
    
    // Update the window in the renderers
    
    _refreshImages();

  }
    
  // hide the rubber band

  _rubberBand->hide();
  update();

}

////////////////////////////////////////////////////////////////////////////
// get ray closest to click point

const RadxRay *VertView::_getClosestRay(double xx, double yy)

{

  // if (_platform.getAltitudeKm() > -1.0) {
  //   _beamHt.setInstrumentHtKm(_platform.getAltitudeKm());
  // }
  // double clickEl = _beamHt.computeElevationDeg(yy, xx);
  
  // double minDiff = 1.0e99;
  const RadxRay *closestRay = NULL;
  // for (size_t ii = 0; ii < _vertBeams.size(); ii++) {
  //   const RadxRay *ray = _vertBeams[ii]->getRay();
  //   double rayEl = ray->getElevationDeg();
  //   double diff = fabs(clickEl - rayEl);
  //   if (diff > 180.0) {
  //     diff = fabs(diff - 360.0);
  //   }
  //   if (diff < minDiff) {
  //     closestRay = ray;
  //     minDiff = diff;
  //   }
  // }

  return closestRay;

}

/*************************************************************************
 * _setGridSpacing()
 */

void VertView::_setGridSpacing()
{

  double xRange = _zoomWorld.getXMaxWorld() - _zoomWorld.getXMinWorld();
  double yRange = _zoomWorld.getYMaxWorld() - _zoomWorld.getYMinWorld();

  _xGridSpacing = _getSpacing(xRange);
  _yGridSpacing = _getSpacing(yRange);

}

/*************************************************************************
 * Get spacing for a given distance range
 */

double VertView::_getSpacing(double range)
{

  if (range <= 1.0) {
    return 0.1;
  } else if (range <= 2.0) {
    return 0.2;
  } else if (range <= 5.0) {
    return 0.5;
  } else if (range <= 10.0) {
    return 1.0;
  } else if (range <= 20.0) {
    return 2.0;
  } else if (range <= 50.0) {
    return 5.0;
  } else if (range <= 100.0) {
    return 10.0;
  } else {
    return 20.0;
  }

}


/*************************************************************************
 * _drawOverlays()
 */

void VertView::_drawOverlays(QPainter &painter)
{

  // save painter state

  painter.save();
  
  // store font
  
  QFont origFont = painter.font();
  
  // Set the painter to use the right color and font

  // painter.setWindow(_zoomWindow);
  
  painter.setPen(_params.vert_grid_color);

  // Draw the axes

  double xMin = _zoomWorld.getXMinWorld();
  double yMin = _zoomWorld.getYMinWorld();
  
  double xMax = _zoomWorld.getXMaxWorld();
  double yMax = _zoomWorld.getYMaxWorld();
  
  QFont font = painter.font();
  font.setPointSizeF(_params.vert_label_font_size);
  painter.setFont(font);
  
  _zoomWorld.specifyXTicks(xMin, _xGridSpacing);
  _zoomWorld.specifyYTicks(yMin, _yGridSpacing);

  _zoomWorld.drawAxisTop(painter, "km", true, true, true, true);
  _zoomWorld.drawAxisBottom(painter, "km", true, true, true, true);
  
  _zoomWorld.drawAxisLeft(painter, "km", true, true, true, true);
  _zoomWorld.drawAxisRight(painter, "km", true, true, true, true);
    
  // Draw the grid
  
  if (_xGridSpacing > 0.0 && _gridsEnabled)  {

    const vector<double> &topTicks = _zoomWorld.getTopTicks();
    for (size_t ii = 0; ii < topTicks.size(); ii++) {
      _zoomWorld.drawLine(painter, topTicks[ii], yMin, topTicks[ii], yMax);
    }

    const vector<double> &rightTicks = _zoomWorld.getRightTicks();
    for (size_t ii = 0; ii < rightTicks.size(); ii++) {
      _zoomWorld.drawLine(painter, xMin, rightTicks[ii], xMax, rightTicks[ii]);
    }

  }
  
  // click point cross hairs
  
  if (_pointClicked) {

    int startX = _mouseReleaseX - _params.click_cross_size / 2;
    int endX = _mouseReleaseX + _params.click_cross_size / 2;
    int startY = _mouseReleaseY - _params.click_cross_size / 2;
    int endY = _mouseReleaseY + _params.click_cross_size / 2;

    painter.drawLine(startX, _mouseReleaseY, endX, _mouseReleaseY);
    painter.drawLine(_mouseReleaseX, startY, _mouseReleaseX, endY);

  }

  // reset painter state
  
  painter.restore();

  // draw the color scale

  int fieldNum = _gd.h_win.page;
  const ColorMap &colorMap(_gd.mread[fieldNum]->colorMap);
  _zoomWorld.drawColorScale(colorMap, painter, _params.label_font_size);
  
  // add legends with time, field name and elevation angle

  if (_archiveMode) {
    
    vector<string> legends;
    char text[1024];
    
    // time legend

    snprintf(text, 1024, "Start time: %s", _plotStartTime.asString(3).c_str());
    legends.push_back(text);
    
    // radar and site name legend

    string radarName("unknown");
    // string radarName(_platform.getInstrumentName());
    // if (_params.override_radar_name) {
    //   radarName = _params.radar_name;
    // }
    string siteName("unknown");
    // string siteName(_platform.getInstrumentName());
    // if (_params.override_site_name) {
    //   siteName = _params.site_name;
    // }
    string radarSiteLabel = radarName;
    if (siteName.size() > 0) {
      radarSiteLabel += "/";
      radarSiteLabel += siteName;
    }
    legends.push_back(radarSiteLabel);

    // field name legend

    // string fieldName = _fieldRenderers[_selectedField]->getField().getLabel();
    // snprintf(text, "Field: %s", fieldName.c_str());
    legends.push_back(text);
    
    // azimuth legend

    snprintf(text, 1024, "Azimuth(deg): %.2f", _meanAz);
    legends.push_back(text);

    // nrays legend

    snprintf(text, 1024, "NRays: %g", _nRays);
    legends.push_back(text);
    
    painter.save();
    painter.setPen(QColor(_params.text_color)); //Qt::yellow);
    painter.setBrush(Qt::black);
    painter.setBackgroundMode(Qt::OpaqueMode);

    switch (_params.horiz_main_legend_pos) {
      case Params::LEGEND_TOP_LEFT:
        _zoomWorld.drawLegendsTopLeft(painter, legends);
        break;
      case Params::LEGEND_TOP_RIGHT:
        _zoomWorld.drawLegendsTopRight(painter, legends);
        break;
      case Params::LEGEND_BOTTOM_LEFT:
        _zoomWorld.drawLegendsBottomLeft(painter, legends);
        break;
      case Params::LEGEND_BOTTOM_RIGHT:
        _zoomWorld.drawLegendsBottomRight(painter, legends);
        break;
      default: {}
    }

    // painter.setBrush(Qt::white);
    // painter.setBackgroundMode(Qt::TransparentMode);
    painter.restore();

  } // if (_archiveMode) {

}

/*************************************************************************
 * _refreshImages()
 */

void VertView::_refreshImages()
{

  // for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
    
  //   FieldRenderer *field = _fieldRenderers[ifield];
    
  //   // If needed, create new image for this field
    
  //   if (size() != field->getImage()->size()) {
  //     field->createImage(width(), height());
  //   }

  //   // clear image

  //   field->getImage()->fill(_backgroundBrush.color().rgb());
    
  //   // set up rendering details

  //   field->setTransform(_zoomTransform);
    
  //   // Add pointers to the beams to be rendered
    
  //   if (ifield == _selectedField || field->isBackgroundRendered()) {

  //     std::deque<VertBeam*>::iterator beam;
  //     for (beam = _vertBeams.begin(); beam != _vertBeams.end(); ++beam) {
  //       (*beam)->setBeingRendered(ifield, true);
  //       field->addBeam(*beam);
  //     }
      
  //   }
    
  // } // ifield
  
  // do the rendering

  _performRendering();

  update();
}


/*************************************************************************
 * clear()
 */

void VertView::clear()
{

  // Clear out the beam array
  
  // for (size_t i = 0; i < _vertBeams.size(); i++) {
  //   Beam::deleteIfUnused(_vertBeams[i]);
  // }
  // _vertBeams.clear();
  _pointClicked = false;
  
  // Now rerender the images
  
  _refreshImages();
  
}


/*************************************************************************
 * refresh()
 */

void VertView::refresh()
{
  _refreshImages();
}

/*************************************************************************
 * unzoom the view
 */

void VertView::unzoomView()
{
  
  _zoomWorld = _fullWorld;
  _zoomWorld.setName("VertView-zoomed");
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _setGridSpacing();
  _refreshImages();
  // _updateRenderers();

}

/*************************************************************************
 * adjust pixel scale for correct aspect ratio etc
 */
void VertView::updatePixelScales()
{

  cerr << "==>> hhhhhh VertView::updatePixelScales() <<==" << endl;

}

/*************************************************************************
 * resize()
 */

void VertView::resize(const int width, const int height)
{
  
  setGeometry(0, 0, width, height);
  _resetWorld(width, height);
  _refreshImages();

}

/*************************************************************************
 * paintEvent()
 */

void VertView::paintEvent(QPaintEvent *event)
{

  QPainter painter(this);
  painter.save();
  painter.eraseRect(0, 0, width(), height());
  _zoomWorld.setClippingOn(painter);
  // painter.drawImage(0, 0, *(_fieldRenderers[_selectedField]->getImage()));
  painter.restore();
  _drawOverlays(painter);

}

/*************************************************************************
 * selectVar()
 */

void VertView::selectVar(const size_t index)
{

  // If the field index isn't actually changing, we don't need to do anything
  
  if (_selectedField == index) {
    return;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========>> VertView::selectVar() for field index: " 
         << index << endl;
  }

  // If this field isn't being rendered in the background, render all of
  // the beams for it

  // if (!_fieldRenderers[index]->isBackgroundRendered()) {
  //   std::deque<VertBeam*>::iterator beam;
  //   for (beam = _vertBeams.begin(); beam != _vertBeams.end(); ++beam) {
  //     (*beam)->setBeingRendered(index, true);
  //     _fieldRenderers[index]->addBeam(*beam);
  //   }
  // }
  _performRendering();

  // Do any needed housekeeping when the field selection is changed

  // _fieldRenderers[_selectedField]->unselectField();
  // _fieldRenderers[index]->selectField();
  
  // Change the selected field index

  _selectedField = index;

  // Update the display

  update();
}

/*************************************************************************
 * RENDER_V_MOVIE_FRAME:
 */

int VertView::renderVMovieFrame(int index, QPainter &painter)
{
  int stat = 0;
#ifdef NOTYET
  int c_field = _gd.v_win.page;
  
  if(_gd.debug2) fprintf(stderr, "Rendering Vertical movie_frame %d - field %d\n", index, c_field);

  
  switch(_gd.movie.mode) {
    case REALTIME_MODE:
    case ARCHIVE_MODE:
      stat = render_vert_display(xid, c_field,
                                 _gd.movie.frame[index].time_start,
                                 _gd.movie.frame[index].time_end);
      break;
         
  }
#endif
  return stat;
}

#ifdef NOTYET

/**********************************************************************
 * RENDER_VERT_DISPLAY: Render the vertical cross section display
 */

int VertView::renderVertDisplay(QPaintDevice *pdev,
                                int page,
                                time_t start_time,
                                time_t end_time)
{

  int i;
  int x1,y1,ht,wd;    /* boundries of image area */
  // int stat;
  contour_info_t cont; // contour params
  
  if(xid == 0) return CIDD_FAILURE;

  if(_params.show_data_messages) {
    gui_label_h_frame("Rendering",-1);
  } else {
    set_busy_state(1);
  }

  if(_gd.debug2) fprintf(stderr,"Rendering Vertical Image, page :%d\n",page);
  /* Clear drawing area */
  XFillRectangle(_gd.dpy,xid,_gd.legends.background_color->gc,
                 0,0,_gd.v_win.can_dim.width,_gd.v_win.can_dim.height);

  if(!_params.draw_main_on_top) { 
    if(_gd.mread[page]->render_method == LINE_CONTOURS) {
      cont.min = _gd.mread[page]->cont_low;
      cont.max = _gd.mread[page]->cont_high;
      cont.interval = _gd.mread[page]->cont_interv;
      cont.active = 1;
      cont.field = page;
      cont.labels_on = _params.label_contours;
      cont.color = _gd.legends.foreground_color;
      cont.vcm = &_gd.mread[page]->v_vcm;
      if (0) {   // Taiwan HACK - Buggy - do not use RenderLineContours()
        //if (_gd.layers.use_alt_contours) {
        RenderLineContours(xid, &cont, true);
      } else {
        render_xsect_line_contours(xid,&cont);
      }
    } else {
      render_xsect_grid(xid,_gd.mread[page],start_time,end_time,0);
      // stat =  render_xsect_grid(xid,_gd.mread[page],start_time,end_time,0);
    }
  }
    
  /* Render each of the gridded_overlay fields */
  for(i=0; i < NUM_GRID_LAYERS; i++) {           
    if(_gd.layers.overlay_field_on[i]) {
      render_xsect_grid(xid,_gd.mread[_gd.layers.overlay_field[i]],start_time,end_time,1);
    }
  } 

  if(_params.draw_main_on_top) { 
    if(_gd.mread[page]->render_method == LINE_CONTOURS) {
      cont.min = _gd.mread[page]->cont_low;
      cont.max = _gd.mread[page]->cont_high;
      cont.interval = _gd.mread[page]->cont_interv;
      cont.active = 1;
      cont.field = page;
      cont.labels_on = _params.label_contours;
      cont.color = _gd.legends.foreground_color;
      cont.vcm = &_gd.mread[page]->v_vcm;
      if (0) {   // Taiwan HACK - Buggy - do not use RenderLineContours()
        // if (_gd.layers.use_alt_contours) {
        RenderLineContours(xid, &cont, true);
      } else {
        render_xsect_line_contours(xid,&cont);
      }
    } else {
      // stat =  render_xsect_grid(xid,_gd.mread[page],start_time,end_time,0);
      render_xsect_grid(xid,_gd.mread[page],start_time,end_time,0);
    }
  }

  /* render contours if selected */
  for(i=0; i < NUM_CONT_LAYERS; i++) {
    if(_gd.layers.cont[i].active) {    
      if (0) {   // Taiwan HACK - Buggy - do not use RenderLineContours()
 	// if (_gd.layers.use_alt_contours) {
        RenderLineContours(xid, &(_gd.layers.cont[i]), true);
      } else {
        render_xsect_line_contours(xid, &(_gd.layers.cont[i]));
      }
    }
  }

  /* render Winds if selected */
  if(_gd.layers.wind_vectors) {
    render_vert_wind_vectors(xid);
  }

  // Render masking terrain
  if(_gd.layers.earth.terrain_active) {
    render_v_terrain(xid);
  }

  render_xsect_top_layers(xid,page);

  // render_vert_products(xid);

  /* clear margin areas */
  XFillRectangle(_gd.dpy,xid,_gd.legends.background_color->gc,
                 0,0,_gd.v_win.can_dim.width,_gd.v_win.margin.top);

  XFillRectangle(_gd.dpy,xid,_gd.legends.background_color->gc,
                 0,_gd.v_win.can_dim.height - _gd.v_win.margin.bot,
                 _gd.v_win.can_dim.width,_gd.v_win.margin.bot);

  XFillRectangle(_gd.dpy,xid,_gd.legends.background_color->gc,
                 0,0,_gd.v_win.margin.left,_gd.v_win.can_dim.height);

  XFillRectangle(_gd.dpy,xid,_gd.legends.background_color->gc,
                 _gd.v_win.can_dim.width - _gd.v_win.margin.right,
                 0,_gd.v_win.margin.right,_gd.v_win.can_dim.height);


  draw_vwin_right_margin(xid,page);
  draw_vwin_top_margin(xid,page);
  draw_vwin_left_margin(xid,page);
  draw_vwin_bot_margin(xid,page);

  /* Add a border */
  x1 = _gd.v_win.margin.left -1;
  y1 = _gd.v_win.margin.top -1;
  wd = _gd.v_win.img_dim.width +1;
  ht = _gd.v_win.img_dim.height +1;
  /* Add a border around the plot */
  XDrawRectangle(_gd.dpy,xid,_gd.legends.foreground_color->gc,x1,y1,wd,ht);
 

  if(_params.show_data_messages) {
    gui_label_h_frame(_gd.frame_label,-1);
  } else {
    set_busy_state(0); 
  }

  return CIDD_SUCCESS;
}

#endif

/*************************************************************************
 * set archive mode
 */

void VertView::setArchiveMode(bool state)
{
  _archiveMode = state;
}

/*************************************************************************
 * zoomBack the view
 */

void VertView::zoomBackView()
{
  if (_savedZooms.size() == 0) {
    _zoomWorld = _fullWorld;
  } else {
    _zoomWorld = _savedZooms[_savedZooms.size()-1];
    _savedZooms.pop_back();
  }
  if (_savedZooms.size() == 0) {
    _isZoomed = false;
  }
  _setTransform(_zoomWorld.getTransform());
  _setGridSpacing();
  _refreshImages();
}


/*************************************************************************
 * setGrids()
 */

void VertView::setGrids(const bool enabled)
{
  _gridsEnabled = enabled;
  update();
}


/*************************************************************************
 * turn on archive-style rendering - all fields
 */

void VertView::activateArchiveRendering()
{
  // for (size_t ii = 0; ii < _fieldRenderers.size(); ii++) {
  //   _fieldRenderers[ii]->setBackgroundRenderingOn();
  // }
}


/*************************************************************************
 * turn on reatlime-style rendering - non-selected fields in background
 */

void VertView::activateRealtimeRendering()
{
  
  // for (size_t ii = 0; ii < _fieldRenderers.size(); ii++) {
  //   if (ii != _selectedField) {
  //     _fieldRenderers[ii]->activateBackgroundRendering();
  //   }
  // }

}

/*************************************************************************
 * displayImage()
 */

void VertView::displayImage(const size_t field_num)
{
  // If we weren't rendering the current field, do nothing
  if (field_num != _selectedField) {
    return;
  }
  cerr << "DISPLAY IMAGE" << endl;
  update();
}


/*************************************************************************
 * backgroundColor()
 */

void VertView::backgroundColor(const QColor &color)
{
  _backgroundBrush.setColor(color);
  QPalette new_palette = palette();
  new_palette.setColor(QPalette::Dark, _backgroundBrush.color());
  setPalette(new_palette);
  _refreshImages();
}


/*************************************************************************
 * getImage()
 */

QImage* VertView::getImage()
{
  QPixmap pixmap = grab();
  QImage* image = new QImage(pixmap.toImage());
  return image;
}


/*************************************************************************
 * getPixmap()
 */

QPixmap* VertView::getPixmap()
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

void VertView::mousePressEvent(QMouseEvent *e)
{

  // cerr << "cccc mousePressEvent" << endl;
  
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

void VertView::mouseMoveEvent(QMouseEvent * e)
{

  // cerr << "ccccc mouseMoveEvent" << endl;
  
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

  // double dx = fabs(deltaY * _aspectRatio);
  // double dy = fabs(dx / _aspectRatio);
  double dx = fabs(deltaY);
  double dy = fabs(dx);

  // Preserve the signs

  dx *= fabs(deltaX)/deltaX;
  dy *= fabs(deltaY)/deltaY;

  int moveX = (int) floor(dx + 0.5);
  int moveY = (int) floor(dy + 0.5);

  moveX = deltaX;
  moveY = deltaY;
  
  QRect newRect = QRect(_mousePressX, _mousePressY, moveX, moveY);

  _zoomCornerX = _mousePressX + moveX;
  _zoomCornerY = _mousePressY + moveY;

  newRect = newRect.normalized();
  _rubberBand->setGeometry(newRect);

}

#ifdef NOTNOW
/**************   testing ******/

void VertView::smartBrush(int xPixel, int yPixel) 
{

  //int xp = _ppi->_zoomWorld.getIxPixel(xkm);
  //int yp = _ppi->_zoomWorld.getIyPixel(ykm);
  QImage qImage;
  qImage.load("/h/eol/brenda/octopus.jpg");
  // get the Image from somewhere ...   
  //qImage->convertToFormat(QImage::Format_ARGB32);
  //qImage->invertPixels();
  QPainter painter(this);
  painter.drawImage(0, 0, qImage);
  _drawOverlays(painter);

}
#endif

/*************************************************************************
 * resizeEvent()
 */

void VertView::resizeEvent(QResizeEvent * e)
{
  cerr << "RRRRRRRRRRRRRRRRRR width, height: " << width() << ", " << height() << endl;
  _resetWorld(width(), height());
  updatePixelScales();
  _refreshImages();
  update();
}

//////////////////////////////////////////////////////////////
// reset the pixel size of the world view

void VertView::_resetWorld(int width, int height)

{

  _fullWorld.resize(width, height);
  _zoomWorld = _fullWorld;
  _setTransform(_fullWorld.getTransform());
  _setGridSpacing();

}

/*************************************************************************
 * Protected methods
 *************************************************************************/

////////////////////
// set the transform

void VertView::_setTransform(const QTransform &transform)
{
  // float worldScale = _zoomWorld.getXMaxWindow() - _zoomWorld.getXMinWindow();
  // BoundaryPointEditor::Instance()->setWorldScale(worldScale);

  _fullTransform = transform;
  _zoomTransform = transform;
}
  
/*************************************************************************
 * perform the rendering
 */

void VertView::_performRendering()
{

  // start the rendering
  
  // for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
  //   if (ifield == _selectedField ||
  //       _fieldRenderers[ifield]->isBackgroundRendered()) {
  //     _fieldRenderers[ifield]->signalRunToStart();
  //   }
  // } // ifield

  // wait for rendering to complete
  
  // for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
  //   if (ifield == _selectedField ||
  //       _fieldRenderers[ifield]->isBackgroundRendered()) {
  //     _fieldRenderers[ifield]->waitForRunToComplete();
  //   }
  // } // ifield

  update();

}

void VertView::informationMessage()
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

// void VertView::notImplemented()
// {
//   cerr << "inside notImplemented() ... " << endl;

//   QErrorMessage *errorMessageDialog = new QErrorMessage(_parent);
//   // QLabel *informationLabel = new QLabel();

//   errorMessageDialog->showMessage("This option is not implemented yet.");
//   QLabel errorLabel;
//   int frameStyle = QFrame::Sunken | QFrame::Panel;
//   errorLabel.setFrameStyle(frameStyle);
//   errorLabel.setText("If the box is unchecked, the message "
// 		     "won't appear again.");

//   cerr << "exiting notImplemented() " << endl;

// }


// slots for context editing; create and show the associated modeless dialog and return                                   

void VertView::contextMenuCancel()
{
  // informationMessage();
  // notImplemented();                                                                                                     
}

void VertView::contextMenuParameterColors()
{
  /*
    LOG(DEBUG_VERBOSE) << "enter";

    //DisplayField selectedField;

    const DisplayField &field = _manager.getSelectedField();
    const ColorMap &colorMapForSelectedField = field.getColorMap();
    ParameterColorView *parameterColorView = new ParameterColorView(this);
    vector<DisplayField> displayFields = _manager.getDisplayFields();
    DisplayFieldModel *displayFieldModel = new DisplayFieldModel(displayFields);
    FieldColorController fieldColorController(parameterColorView, displayFieldModel);
    // connect some signals and slots in order to retrieve information
    // and send changes back to display 
    connect(&parameterColorView, SIGNAL(retrieveInfo), &_manager, SLOT(InfoRetrieved()));
    connect(&parameterColorView, SIGNAL(changesToDisplay()), &_manager, SLOT(changesToDisplayFields()));

    // TODO: move this call to the controller?
    parameterColorView.exec();

    if(parameterColorController.Changes()) {
    // TODO: what are changes?  new displayField(s)?
    }
  
    // TODO: where to delete the ParameterColor objects & disconnect the signals and slots??
    delete parameterColorView;
    delete parameterColorModel;

    LOG(DEBUG_VERBOSE) << "exit ";
  */
  informationMessage();
   
}

void VertView::contextMenuView()
{
  informationMessage();
  //  notImplemented();                                                                                                   
}

void VertView::contextMenuEditor()
{
  informationMessage();
  //  notImplemented();                                                                                                   
}


void VertView::contextMenuExamine()         
{
  informationMessage();                                                                                                 

}

void VertView::contextMenuDataWidget()
{
  informationMessage();

  //  notImplemented();                                                                                                   
}

void VertView::ShowContextMenu(const QPoint &pos, RadxVol *vol)
{

}


