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
#include <QPainterPath>
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
#include <toolsa/pjg_flat.h>
#include <toolsa/uusleep.h>
#include <toolsa/LogStream.hh>
#include <Mdv/MdvxContour.hh>
#include <Mdv/MdvxField.hh>

#include "GuiManager.hh"
#include "VertView.hh"
#include "VertManager.hh"
#include "GlobalData.hh"
#include "MdvReader.hh"

using namespace std;

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
        _heightGridEnabled(_params.vsection_ht_grid_on_at_startup),
        _distanceGridEnabled(_params.vsection_distance_grid_on_at_startup),
        _rubberBand(0),
        _sectionReader(nullptr)
        
{
  
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
  _leftDragActive = false;
  _sectionPixmapDirty = true;
  _showValue = false;
  _mouseValueVisible = false;
  _mouseValueX = 0;
  _mouseValueY = 0;
  _mousePressX = 0;
  _mousePressY = 0;
  _mouseReleaseX = 0;
  _mouseReleaseY = 0;
  _zoomCornerX = 0;
  _zoomCornerY = 0;
  
  _colorScaleWidth = _params.vsection_color_scale_width;
  
  // initialize world view
  
  _maxHeightKm = _params.max_ht_km;
  _xGridSpacing = 0.0;
  _yGridSpacing = 0.0;

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

  int leftMargin = _params.vsection_left_margin;
  int rightMargin = _params.vsection_right_margin;
  int topMargin = _params.vsection_top_margin;
  int bottomMargin = _params.vsection_bottom_margin;
  int colorScaleWidth = _params.vsection_color_scale_width;
  int axisTickLen = _params.vsection_axis_tick_len;
  int nTicksIdeal = _params.vsection_n_ticks_ideal;
  int titleTextMargin = _params.vsection_title_text_margin;
  int legendTextMargin = _params.vsection_legend_text_margin;
  int axisTextMargin = _params.vsection_axis_text_margin;
  
  _fullWorld.setName("VertView-full");
  _fullWorld.setWindowGeom(width(), height(), 0, 0);
  
  double xMax = 100.0;
  double yMin = _params.min_ht_km;
  double yMax = _maxHeightKm;
  if (_sectionReader != nullptr && _sectionReader->isValidV()) {
    xMax = _sectionReader->getVertTotalLengthKm();
    if (xMax <= 0.0) {
      xMax = (_sectionReader->v_fhdr.nx - 1) *
             _sectionReader->v_fhdr.grid_dx;
    }
    // Height limits are in km MSL. Preserve native limits for sections whose
    // vertical coordinate uses other units, such as pressure or elevation.
    if (_sectionReader->v_fhdr.vlevel_type != Mdvx::VERT_TYPE_Z) {
      const vector<double> edges = _computeVerticalEdges();
      if (edges.size() >= 2) {
        const auto limits = std::minmax_element(edges.begin(), edges.end());
        yMin = *limits.first;
        yMax = *limits.second;
      }
    }
  }
  if (xMax <= 0.0) {
    xMax = 1.0;
  }
  if (yMax <= yMin) {
    yMax = yMin + 1.0;
  }

  _fullWorld.setPreserveAspectRatio(false);
  _fullWorld.setWorldLimits(0.0, yMin, xMax, yMax);
  
  _fullWorld.setLeftMargin(leftMargin);
  _fullWorld.setRightMargin(rightMargin);
  _fullWorld.setTopMargin(topMargin);
  _fullWorld.setBottomMargin(bottomMargin * 2);
  _fullWorld.setTitleTextMargin(titleTextMargin);
  _fullWorld.setLegendTextMargin(legendTextMargin);
  _fullWorld.setAxisTextMargin(axisTextMargin);
  _fullWorld.setColorScaleWidth(colorScaleWidth);

  _fullWorld.setXAxisTickLen(axisTickLen);
  _fullWorld.setXNTicksIdeal(nTicksIdeal);
  _fullWorld.setYAxisTickLen(axisTickLen);
  _fullWorld.setYNTicksIdeal(nTicksIdeal);

  _fullWorld.setTitleFontSize(_params.vsection_title_font_size);
  _fullWorld.setAxisLabelFontSize(_params.vsection_axis_label_font_size);
  _fullWorld.setTickValuesFontSize(_params.vsection_tick_values_font_size);
  _fullWorld.setLegendFontSize(_params.vsection_legend_font_size);

  _fullWorld.setTitleColor(_params.vsection_title_color);
  _fullWorld.setAxisLineColor(_params.vsection_axes_color);
  _fullWorld.setAxisTextColor(_params.vsection_axes_color);
  _fullWorld.setGridColor(_params.vsection_grid_color);

  _fullWorld.setYAxisLabelsInside(_params.vsection_tick_values_inside);
  _fullWorld.setXAxisLabelsInside(_params.vsection_tick_values_inside);

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

#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  if (!_leftDragActive || e->button() != Qt::LeftButton) {
    return;
  }
  _leftDragActive = false;

  _mouseReleaseX = pos.x();
  _mouseReleaseY = pos.y();

  const double deltaX = _mouseReleaseX - _mousePressX;
  const double deltaY = _mouseReleaseY - _mousePressY;
  const double distanceMoved = std::hypot(deltaX, deltaY);

  if (distanceMoved <= 20.0) {
    _worldReleaseX = _zoomWorld.getXWorld(_mouseReleaseX);
    _worldReleaseY = _zoomWorld.getYWorld(_mouseReleaseY);

    SectionSample sample;
    if (_sampleSection(_mouseReleaseX, _mouseReleaseY, sample)) {
      _pointClicked = true;
      emit clickReportAvailable(_createClickReport(sample));
    }
  
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
  if (_showValue) {
    _updateMouseValue(_mouseReleaseX, _mouseReleaseY);
  }
  update();

}

/*************************************************************************
 * _setGridSpacing()
 */

void VertView::_setGridSpacing()
{

  double xRange = _zoomWorld.getXMaxWorld() - _zoomWorld.getXMinWorld();
  double yRange = _zoomWorld.getYMaxWorld() - _zoomWorld.getYMinWorld();

  _xGridSpacing = _getSpacing(xRange, _zoomWorld.getPlotWidth());
  _yGridSpacing = _getSpacing(yRange, _zoomWorld.getPlotHeight());

}

/*************************************************************************
 * Get spacing for a given distance range
 */

double VertView::_getSpacing(double range, int pixelExtent)
{

  if (range <= 0.0) {
    return 1.0;
  }

  // Keep adjacent labels roughly 50 pixels apart, then round to familiar
  // 1/2/5 intervals.  This makes long routes readable and also adapts when
  // the window is resized or the user zooms.
  const double targetPixels = 50.0;
  const double targetIntervals =
    std::max(1.0, pixelExtent / targetPixels);
  const double rawSpacing = range / targetIntervals;
  const double magnitude = pow(10.0, floor(log10(rawSpacing)));
  const double normalized = rawSpacing / magnitude;

  double niceNormalized = 1.0;
  if (normalized < 1.5) {
    niceNormalized = 1.0;
  } else if (normalized < 3.0) {
    niceNormalized = 2.0;
  } else if (normalized < 7.0) {
    niceNormalized = 5.0;
  } else {
    niceNormalized = 10.0;
  }
  return niceNormalized * magnitude;

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
  
  painter.setPen(_params.vsection_grid_color);

  // Draw the axes

  double xMin = _zoomWorld.getXMinWorld();
  double yMin = _zoomWorld.getYMinWorld();
  
  QFont font = painter.font();
  font.setPointSizeF(_params.vsection_label_font_size);
  painter.setFont(font);
  
  _zoomWorld.specifyXTicks(xMin, _xGridSpacing);
  _zoomWorld.specifyYTicks(yMin, _yGridSpacing);

  // X ticks produce distance lines; Y ticks produce height lines.
  _zoomWorld.drawAxisTop(painter, "km", true, true, false, _distanceGridEnabled,
                         false);
  _zoomWorld.drawAxisBottom(painter, "km", true, true, true, false);
  
  const string verticalUnits = _verticalUnits.empty() ? "km" : _verticalUnits;
  _zoomWorld.drawAxisLeft(painter, verticalUnits, true, true, true,
                          _heightGridEnabled);
  _zoomWorld.drawAxisRight(painter, verticalUnits, true, true, false,
                           false);

  // Segment lengths describe the returned route in the same cumulative
  // kilometer coordinates as the section. Mark only internal joins, and
  // clip them to the current zoom so they never cross the axis margins.
  if (_sectionReader != nullptr && _sectionReader->isValidV()) {
    const vector<Mdvx::vsect_segment_t> &segments =
      _sectionReader->getVertSegments();
    painter.save();
    _zoomWorld.setClippingOn(painter);
    QPen boundaryPen(QColor(_params.vsection_axes_color));
    boundaryPen.setWidth(2);
    boundaryPen.setCosmetic(true);
    painter.setPen(boundaryPen);
    double distanceKm = 0.0;
    for (size_t ii = 0; ii + 1 < segments.size(); ++ii) {
      distanceKm += segments[ii].length;
      if (!std::isfinite(distanceKm)) {
        break;
      }
      if (distanceKm <= _zoomWorld.getXMinWorld() ||
          distanceKm >= _zoomWorld.getXMaxWorld()) {
        continue;
      }
      const double xPixel = _zoomWorld.getXPixel(distanceKm);
      painter.drawLine(QLineF(xPixel, _zoomWorld.getYMaxPixel(),
                              xPixel, _zoomWorld.getYMinPixel()));
    }
    painter.restore();
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

}

/*************************************************************************
 * _drawLegends()
 */

void VertView::_drawLegends(QPainter &painter)
{

  // add legends with time, field name and elevation angle
  
  vector<string> legends;
  char text[1024];
    
  // route distance
  
  snprintf(text, 1024, "Vsection length (km): %g", _routeLengthKm);
  legends.push_back(text);
    
  painter.save();
  painter.setPen(QColor(_params.text_color));
  
  switch (_params.vsection_main_legend_pos) {
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

  painter.restore();

}

/*************************************************************************
 * _refreshImages()
 */

void VertView::_refreshImages()
{

  _sectionPixmapDirty = true;

  update();
  
}


/*************************************************************************
 * clear()
 */

void VertView::clear()
{

  _pointClicked = false;
  _leftDragActive = false;
  _mouseValueVisible = false;
  _mouseValueText.clear();
  _sectionReader = nullptr;
  
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

}

/*************************************************************************
 * paintEvent()
 */

void VertView::paintEvent(QPaintEvent *event)
{

  QPainter painter(this);
  painter.fillRect(rect(), _backgroundBrush);

  // The section data changes only on a data/geometry refresh. Reusing this
  // layer keeps cursor-value updates independent of expensive contour
  // generation while overlays remain responsive to interaction state.

  if (_sectionPixmapDirty || _sectionPixmap.size() != size()) {
    _sectionPixmap = QPixmap(size());
    _sectionPixmap.fill(Qt::transparent);
    QPainter sectionPainter(&_sectionPixmap);
    _drawSection(sectionPainter);
    _sectionPixmapDirty = false;
  }
  painter.drawPixmap(0, 0, _sectionPixmap);

  // add overlays
  
  _drawOverlays(painter);

  // add legends

  _drawLegends(painter);

  // add color scale
  
  if (_sectionReader != nullptr) {
    // The compact vertical window needs the scale shifted into its plot area,
    // while its units sit above the highest numeric label instead of
    // competing with it.
    _zoomWorld.drawColorScale(_sectionReader->colorMap, painter,
                              _params.horiz_axis_label_font_size);
    _zoomWorld.drawTitleTopCenter(painter, _sectionReader->fieldLabelVert());
  }

  // show mouse value if needed

  if (_showValue && _mouseValueVisible) {
    _drawMouseValue(painter);
  }
  
}

/////////////////////////////////////////////////////////////
// install a gridded MDV vertical section

void VertView::setSection(const MdvReader *reader)
{
  _sectionReader = reader;
  _mouseValueVisible = false;
  _mouseValueText.clear();
  _verticalUnits.clear();
  if (_sectionReader != nullptr) {
    _verticalUnits = Mdvx::vertTypeZUnits(_sectionReader->v_fhdr.vlevel_type);
  }
  _savedZooms.clear();
  configureWorldCoords();
  update();
}

/////////////////////////////////////////////////////////////
// sample the underlying vertical-section grid at a widget location

bool VertView::_sampleGridValue(int mouseX, int mouseY,
                                SectionSample &sample) const
{
  if (_sectionReader == nullptr || !_sectionReader->isValidV()) {
    return false;
  }

  const int plotLeft = std::min(_zoomWorld.getXMinPixel(),
                                _zoomWorld.getXMaxPixel());
  const int plotRight = std::max(_zoomWorld.getXMinPixel(),
                                 _zoomWorld.getXMaxPixel());
  const int plotTop = std::min(_zoomWorld.getYMinPixel(),
                               _zoomWorld.getYMaxPixel());
  const int plotBottom = std::max(_zoomWorld.getYMinPixel(),
                                  _zoomWorld.getYMaxPixel());
  if (mouseX < plotLeft || mouseX > plotRight ||
      mouseY < plotTop || mouseY > plotBottom) {
    return false;
  }

  const int nx = _sectionReader->v_fhdr.nx;
  const int nz = _sectionReader->v_fhdr.nz;
  const double dx = _sectionReader->v_fhdr.grid_dx;
  if (nx <= 0 || nz <= 0 || !std::isfinite(dx) || dx == 0.0 ||
      _sectionReader->v_fl32_data.size() != (size_t) nx * (size_t) nz) {
    return false;
  }

  sample.distanceKm = _zoomWorld.getXWorld(mouseX);
  sample.verticalCoord = _zoomWorld.getYWorld(mouseY);
  double totalLengthKm = _sectionReader->getVertTotalLengthKm();
  if (!std::isfinite(totalLengthKm) || totalLengthKm <= 0.0) {
    totalLengthKm = (nx - 1) * fabs(dx);
  }
  if (!std::isfinite(totalLengthKm) || totalLengthKm <= 0.0 ||
      sample.distanceKm < 0.0 || sample.distanceKm > totalLengthKm) {
    return false;
  }
  int ix = (int) std::llround
    ((sample.distanceKm - _sectionReader->v_fhdr.grid_minx) / dx);

  // Rendering extends the first and last sample cells to the route ends.
  // Apply the same rule so endpoint clicks select visible data rather than
  // falling one rounded index outside the returned array.

  ix = std::max(0, std::min(ix, nx - 1));

  // Vertical levels may be irregular or descending. The two outer cell edges
  // bound the grid; computing them directly keeps cursor sampling allocation
  // free while retaining the same extent used by the renderer.

  double firstEdge;
  double lastEdge;
  if (nz == 1) {
    double halfWidth = fabs(_sectionReader->v_fhdr.grid_dz) / 2.0;
    if (halfWidth == 0.0) {
      halfWidth = 0.5;
    }
    firstEdge = _sectionReader->v_vhdr.level[0] - halfWidth;
    lastEdge = _sectionReader->v_vhdr.level[0] + halfWidth;
  } else {
    firstEdge = _sectionReader->v_vhdr.level[0] -
      (_sectionReader->v_vhdr.level[1] -
       _sectionReader->v_vhdr.level[0]) / 2.0;
    lastEdge = _sectionReader->v_vhdr.level[nz - 1] +
      (_sectionReader->v_vhdr.level[nz - 1] -
       _sectionReader->v_vhdr.level[nz - 2]) / 2.0;
  }
  if (sample.verticalCoord < std::min(firstEdge, lastEdge) ||
      sample.verticalCoord > std::max(firstEdge, lastEdge)) {
    return false;
  }

  int iz = 0;
  double closest = fabs(sample.verticalCoord -
                        _sectionReader->v_vhdr.level[0]);
  for (int ii = 1; ii < nz; ++ii) {
    const double difference = fabs(sample.verticalCoord -
                                   _sectionReader->v_vhdr.level[ii]);
    if (difference < closest) {
      closest = difference;
      iz = ii;
    }
  }

  const fl32 missing = _sectionReader->v_fhdr.missing_data_value;
  const fl32 bad = _sectionReader->v_fhdr.bad_data_value;
  double value = _sectionReader->v_fl32_data[iz * nx + ix];
  if (!std::isfinite(value) || value == missing || value == bad) {
    return false;
  }
  if (_sectionReader->v_fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
    value = exp(value);
  }
  if (!std::isfinite(value)) {
    return false;
  }
  if (fabs(value) < 0.0001) {
    value = 0.0;
  }
  sample.value = value;

  return true;
}

/////////////////////////////////////////////////////////////
// add geographic metadata needed only by a click report

bool VertView::_sampleSection(int mouseX, int mouseY,
                              SectionSample &sample) const
{
  if (!_sampleGridValue(mouseX, mouseY, sample)) {
    return false;
  }
  return _routeLocation(sample.distanceKm,
                        sample.latitude, sample.longitude);
}

/////////////////////////////////////////////////////////////
// locate a cumulative section distance along the returned route

bool VertView::_routeLocation(double distanceKm, double &latitude,
                              double &longitude) const
{
  if (_sectionReader == nullptr) {
    return false;
  }
  const WayPts &wayPoints = _sectionReader->getVertWayPts();
  if (wayPoints.getNPoints() < 2) {
    return false;
  }

  double remainingKm = std::max(0.0, distanceKm);
  const vector<Mdvx::vsect_segment_t> &segments =
    _sectionReader->getVertSegments();
  for (size_t ii = 0; ii + 1 < wayPoints.getNPoints(); ++ii) {
    double startLat, startLon, endLat, endLon;
    wayPoints.getPoint(ii, startLat, startLon);
    wayPoints.getPoint(ii + 1, endLat, endLon);

    double lengthKm = 0.0;
    double azimuthDeg = 0.0;
    if (ii < segments.size() && std::isfinite(segments[ii].length) &&
        segments[ii].length > 0.0 &&
        std::isfinite(segments[ii].azimuth)) {
      lengthKm = segments[ii].length;
      azimuthDeg = segments[ii].azimuth;
    } else {
      PJGLatLon2RTheta(startLat, startLon, endLat, endLon,
                      &lengthKm, &azimuthDeg);
    }
    if (!std::isfinite(lengthKm) || lengthKm <= 0.0) {
      continue;
    }

    if (remainingKm <= lengthKm || ii + 2 == wayPoints.getNPoints()) {
      PJGLatLonPlusRTheta(startLat, startLon,
                         std::min(remainingKm, lengthKm), azimuthDeg,
                         &latitude, &longitude);
      return std::isfinite(latitude) && std::isfinite(longitude);
    }
    remainingKm -= lengthKm;
  }

  return false;
}

/////////////////////////////////////////////////////////////
// create a click report for the current vertical field

ClickReport VertView::_createClickReport(const SectionSample &sample) const
{
  ClickReport report;
  report.dataTime = _sectionReader->v_mhdr.time_centroid;
  report.latitude = sample.latitude;
  report.longitude = sample.longitude;
  report.height = sample.verticalCoord;
  report.heightLabel =
    (_sectionReader->v_fhdr.vlevel_type == Mdvx::VERT_TYPE_Z) ?
    "Height" : "Vertical";
  report.heightUnits = _verticalUnits;
  report.x = sample.distanceKm;
  report.xLabel = "Distance";
  report.xUnits = "km";
  report.showY = false;

  ClickReportField field;
  field.name = _sectionReader->v_fhdr.field_name;
  field.units = _sectionReader->v_fhdr.units;
  field.value = sample.value;
  report.fields.push_back(field);
  return report;
}

/////////////////////////////////////////////////////////////
// update the transient value at the cursor

void VertView::_updateMouseValue(int mouseX, int mouseY)
{
  _mouseValueVisible = false;
  _mouseValueText.clear();
  if (!_showValue) {
    return;
  }

  // Hover reporting deliberately stops at the grid lookup. Route
  // interpolation is useful for click metadata but is unnecessary work for
  // every mouse-move event.

  SectionSample sample;
  if (!_sampleGridValue(mouseX, mouseY, sample)) {
    return;
  }

  char text[1024];
  snprintf(text, sizeof(text), "%s: %g %s",
           _sectionReader->v_fhdr.field_name, sample.value,
           _sectionReader->v_fhdr.units);
  _mouseValueText = text;
  _mouseValueX = mouseX;
  _mouseValueY = mouseY;
  _mouseValueVisible = true;
}

/////////////////////////////////////////////////////////////
// draw the transient value beside the cursor

void VertView::_drawMouseValue(QPainter &painter)
{
  constexpr int padding = 3;
  constexpr int cursorOffset = 8;
  const int plotLeft = std::min(_zoomWorld.getXMinPixel(),
                                _zoomWorld.getXMaxPixel());
  const int plotRight = std::max(_zoomWorld.getXMinPixel(),
                                 _zoomWorld.getXMaxPixel());
  const int plotTop = std::min(_zoomWorld.getYMinPixel(),
                               _zoomWorld.getYMaxPixel());
  const int plotBottom = std::max(_zoomWorld.getYMinPixel(),
                                  _zoomWorld.getYMaxPixel());

  const QString label = QString::fromStdString(_mouseValueText);
  const int textFlags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip;
  const QRect bounds = painter.fontMetrics().boundingRect
    (QRect(0, 0, plotRight - plotLeft + 1, plotBottom - plotTop + 1),
     textFlags, label);
  const int boxWidth = bounds.width() + 2 * padding;
  const int boxHeight = bounds.height() + 2 * padding;

  int boxX = _mouseValueX + cursorOffset;
  if (boxX + boxWidth > plotRight) {
    boxX = _mouseValueX - cursorOffset - boxWidth;
  }
  int boxY = _mouseValueY - cursorOffset - boxHeight;
  if (boxY < plotTop) {
    boxY = _mouseValueY + cursorOffset;
  }
  boxX = std::max(plotLeft,
                  std::min(boxX, std::max(plotLeft,
                                          plotRight - boxWidth + 1)));
  boxY = std::max(plotTop,
                  std::min(boxY, std::max(plotTop,
                                          plotBottom - boxHeight + 1)));

  const QRect box(boxX, boxY, boxWidth, boxHeight);
  QColor background(_params.background_color);
  background.setAlpha(210);
  painter.fillRect(box, background);
  painter.setPen(_params.foreground_color);
  painter.drawText(box.adjusted(padding, padding, -padding, -padding),
                   textFlags, label);
}

/////////////////////////////////////////////////////////////
// compute cell edges from the MDV vertical-level centers

vector<double> VertView::_computeVerticalEdges() const
{
  vector<double> edges;
  if (_sectionReader == nullptr || _sectionReader->v_fhdr.nz <= 0) {
    return edges;
  }

  const int nz = _sectionReader->v_fhdr.nz;
  edges.resize(nz + 1);
  if (nz == 1) {
    double halfWidth = fabs(_sectionReader->v_fhdr.grid_dz) / 2.0;
    if (halfWidth == 0.0) {
      halfWidth = 0.5;
    }
    edges[0] = _sectionReader->v_vhdr.level[0] - halfWidth;
    edges[1] = _sectionReader->v_vhdr.level[0] + halfWidth;
    return edges;
  }

  // MDV levels need not be uniformly spaced.  Midpoints preserve the extent
  // represented by each returned level without inventing a uniform grid.

  edges[0] = _sectionReader->v_vhdr.level[0] -
             (_sectionReader->v_vhdr.level[1] -
              _sectionReader->v_vhdr.level[0]) / 2.0;
  for (int iz = 1; iz < nz; ++iz) {
    edges[iz] = (_sectionReader->v_vhdr.level[iz - 1] +
                 _sectionReader->v_vhdr.level[iz]) / 2.0;
  }
  edges[nz] = _sectionReader->v_vhdr.level[nz - 1] +
              (_sectionReader->v_vhdr.level[nz - 1] -
               _sectionReader->v_vhdr.level[nz - 2]) / 2.0;
  return edges;
}

/////////////////////////////////////////////////////////////
// render the MDV section using the selected field's rendering mode

void VertView::_drawSection(QPainter &painter)
{
  if (_sectionReader == nullptr || !_sectionReader->isValidV()) {
    return;
  }

  const int nx = _sectionReader->v_fhdr.nx;
  const int nz = _sectionReader->v_fhdr.nz;
  if (nx <= 0 || nz <= 0 ||
      _sectionReader->v_fl32_data.size() != (size_t) nx * (size_t) nz) {
    return;
  }

  // Both cell and contour renderers leave missing data unpainted. Initialize
  // only the plot area so these holes use the missing color while margins
  // and axes retain the widget background.
  if (string(_params.missing_data_color) != "transparent") {
    painter.save();
    _zoomWorld.setClippingOn(painter);
    painter.fillRect(rect(), QColor(_params.missing_data_color));
    painter.restore();
  }

  if (_sectionReader->render_method == FILLED_CONTOURS ||
      _sectionReader->render_method == DYNAMIC_CONTOURS) {
    if (_drawSectionContours(painter)) {
      return;
    }
  }

  _drawSectionCells(painter);
}

/////////////////////////////////////////////////////////////
// render the MDV section as colored cells

void VertView::_drawSectionCells(QPainter &painter)
{
  const int nx = _sectionReader->v_fhdr.nx;
  const int nz = _sectionReader->v_fhdr.nz;
  const vector<double> yEdges = _computeVerticalEdges();
  const double totalLength = _fullWorld.getXMaxWorld();
  const double dx = _sectionReader->v_fhdr.grid_dx;
  const fl32 missing = _sectionReader->v_fhdr.missing_data_value;
  const fl32 bad = _sectionReader->v_fhdr.bad_data_value;

  painter.save();
  _zoomWorld.setClippingOn(painter);
  painter.setPen(Qt::NoPen);

  if (nz > 1) {
    for (int iz = 0; iz < nz; ++iz) {
      for (int ix = 0; ix < nx; ++ix) {
        const fl32 value = _sectionReader->v_fl32_data[iz * nx + ix];
        if (value == missing || value == bad || !std::isfinite(value) ||
            !_sectionReader->colorMap.withinValidRange(value)) {
          continue;
        }
        
        const double x0 = (ix == 0) ? 0.0 :
          _sectionReader->v_fhdr.grid_minx +
          (ix - 0.5) * dx;
        const double x1 = (ix == nx - 1) ? totalLength :
          _sectionReader->v_fhdr.grid_minx +
          (ix + 0.5) * dx;
        const QPointF p0 = _zoomWorld.getPixelPointF(x0, yEdges[iz]);
        const QPointF p1 = _zoomWorld.getPixelPointF(x1, yEdges[iz + 1]);
        QRectF cell(p0, p1);
        cell = cell.normalized();
        cell.adjust(0.0, 0.0, 1.0, 1.0);
        painter.fillRect(cell, *_sectionReader->colorMap.dataBrush(value));
      }
    }
  }

  painter.restore();
}

/////////////////////////////////////////////////////////////
// render the MDV section as filled polygon contours

bool VertView::_drawSectionContours(QPainter &painter)
{
  const int nx = _sectionReader->v_fhdr.nx;
  const int nz = _sectionReader->v_fhdr.nz;
  const vector<ColorMap::CmapEntry> &entries =
    _sectionReader->colorMap.getEntries();

  vector<double> contourValues;
  if (!entries.empty()) {
    contourValues.push_back(entries[0].minVal);
    for (const ColorMap::CmapEntry &entry : entries) {
      contourValues.push_back(entry.maxVal);
    }
  }

  bool geometryValid = nx >= 2 && nz >= 2 &&
    _sectionReader->v_fhdr.proj_type == Mdvx::PROJ_VSECTION &&
    contourValues.size() == entries.size() + 1;
  for (size_t ii = 1; ii < contourValues.size(); ++ii) {
    if (!std::isfinite(contourValues[ii - 1]) ||
        !std::isfinite(contourValues[ii]) ||
        contourValues[ii] <= contourValues[ii - 1]) {
      geometryValid = false;
      break;
    }
  }
  if (!geometryValid) {
    return false;
  }

  vector<fl32> contourData = _sectionReader->v_fl32_data;
  const fl32 missing = _sectionReader->v_fhdr.missing_data_value;
  const fl32 bad = _sectionReader->v_fhdr.bad_data_value;

  // Values outside the color scale are intentionally treated like missing
  // data in vertical sections. This prevents weak or sentinel-like values
  // from filling otherwise empty parts of the cross section with an endpoint
  // color; individual color scales can be widened when those values matter.
  for (fl32 &value : contourData) {
    if (!std::isfinite(value) || value == missing || value == bad) {
      value = missing;
      continue;
    }
    double displayValue = value;
    if (_sectionReader->v_fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
      displayValue = exp(displayValue);
    }
    if (!std::isfinite(displayValue) ||
        !_sectionReader->colorMap.withinValidRange(displayValue)) {
      value = missing;
    }
  }

  // Preserve the vertical-section projection: MdvxContour treats nz as its Y
  // dimension and uses vlevel_header.level[] for nonuniform row coordinates.
  Mdvx::field_header_t contourHeader = _sectionReader->v_fhdr;
  contourHeader.encoding_type = Mdvx::ENCODING_FLOAT32;
  contourHeader.compression_type = Mdvx::COMPRESSION_NONE;
  contourHeader.data_element_nbytes = sizeof(fl32);
  contourHeader.volume_size = nx * contourHeader.ny * nz * sizeof(fl32);
  MdvxField contourField(contourHeader, _sectionReader->v_vhdr,
                         contourData.data());

  MdvxContour contours;
  for (double value : contourValues) {
    contours.addVal(value);
  }
  if (contours.computeTriangles(contourField)) {
    cerr << "ERROR - VertView::_drawSectionContours" << endl;
    cerr << "  Cannot compute filled contour triangles; using cells" << endl;
    return false;
  }

  // Contour holes and the synthetic extra level are not data. Keep them
  // distinct from valid endpoint colors by painting the configured missing
  // color. Direct widget rendering uses the background for "transparent".
  const char *missingColor =
    string(_params.missing_data_color) == "transparent" ?
    _params.background_color : _params.missing_data_color;
  const QBrush missingBrush{QColor(missingColor)};
  painter.save();
  _zoomWorld.setClippingOn(painter);
  painter.setPen(Qt::NoPen);
  for (const MdvxContourLevel &level : contours.getLevels()) {
    if (level.index < 0) {
      continue;
    }

    const QBrush *levelBrush = &missingBrush;
    if (!level.extra && level.index < (int) entries.size()) {
      levelBrush = entries[level.index].brush;
    }
    if (levelBrush == nullptr) {
      continue;
    }

    QPainterPath levelPath;
    for (const MdvxContourTriangle &triangle : level.triangles) {
      QPolygonF polygon;
      for (int ivert = 0; ivert < 3; ++ivert) {
        const double x = triangle.vertex[ivert].x;
        const double y = triangle.vertex[ivert].y;
        polygon << _zoomWorld.getPixelPointF(x, y);
      }
      levelPath.addPolygon(polygon);
    }
    painter.fillPath(levelPath, *levelBrush);
  }
  painter.restore();
  return true;
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
 * setHeightGrid()
 */

void VertView::setHeightGrid(const bool enabled)
{
  _heightGridEnabled = enabled;
  update();
}

void VertView::setDistanceGrid(const bool enabled)
{
  _distanceGridEnabled = enabled;
  update();
}

////////////////////////////////////////////////////////
// set show value flag

void VertView::setShowValue(bool enabled)
{
  _showValue = enabled;
  setMouseTracking(_showValue);
  if (!_showValue) {
    _mouseValueVisible = false;
    _mouseValueText.clear();
  }
  update();
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
  _sectionPixmapDirty = true;
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

#if QT_VERSION >= 0x060000
  QPointF pos(e->position());
#else
  QPointF pos(e->pos());
#endif

  if (e->button() == Qt::RightButton) {
    _leftDragActive = false;
    
    //-------

    _mousePressX = pos.x();
    _mousePressY = pos.y();

    _worldPressX = _zoomWorld.getXWorld(_mousePressX);
    _worldPressY = _zoomWorld.getYWorld(_mousePressY);

  } else if (e->button() == Qt::LeftButton) {

    _leftDragActive = true;
    _mouseValueVisible = false;

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

  if (_showValue && !_leftDragActive) {
    _updateMouseValue(x, y);
    update();
  }
  if (!_leftDragActive) {
    return;
  }

  const int moveX = x - _mousePressX;
  const int moveY = y - _mousePressY;

  // Vertical sections have independent distance and height scales. Preserve
  // the user's drag rectangle instead of imposing a display aspect ratio.
  
  QRect newRect = QRect(_mousePressX, _mousePressY, moveX, moveY);

  _zoomCornerX = _mousePressX + moveX;
  _zoomCornerY = _mousePressY + moveY;

  newRect = newRect.normalized();
  _rubberBand->setGeometry(newRect);

}

////////////////////////////////////////////////////////
// clear the cursor value when leaving the canvas

void VertView::leaveEvent(QEvent *event)
{
  _mouseValueVisible = false;
  _mouseValueText.clear();
  update();
  QWidget::leaveEvent(event);
}

/*************************************************************************
 * resizeEvent()
 */

void VertView::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  // Rebuild the world-to-pixel transform from the geometry assigned by the
  // layout so data, grids, and labels all cover the same drawable area.
  _resetWorld(event->size().width(), event->size().height());
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
  

