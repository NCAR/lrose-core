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
#include <algorithm>
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
#include "WindGlyphRenderer.hh"
#include "WindManager.hh"

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
  _windsPixmapDirty = true;
  _terrainPixmapDirty = true;
  _playbackFrameIndex = -1;
  _showValue = false;
  _mouseValueVisible = false;
  _mouseValueX = 0;
  _mouseValueY = 0;
  _plotSelectedHorizHeight = _params.vsection_plot_selected_horiz_height;
  _selectedHorizHeightKm = _params.start_ht_km;
  _mousePressX = 0;
  _mousePressY = 0;
  _mouseReleaseX = 0;
  _mouseReleaseY = 0;
  _zoomCornerX = 0;
  _zoomCornerY = 0;
  
  _colorScaleWidth = _params.vsection_color_scale_width;
  
  // initialize world view
  
  _minHeightKm = _params.vsection_min_ht_km;
  _maxHeightKm = _params.vsection_max_ht_km;
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
  double yMin = _minHeightKm;
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

  if (_manager.isMovieActive()) {
    _leftDragActive = false;
    _rubberBand->hide();
    e->accept();
    return;
  }

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

  _drawSelectedHorizHeight(painter);
    
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
  _windsPixmapDirty = true;
  _terrainPixmapDirty = true;

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
  _selectedHorizHeightCurve.clear();
  
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

void VertView::rerenderWinds()
{
  _windsPixmapDirty = true;
  update();
}

void VertView::rerenderTerrain()
{
  _terrainPixmapDirty = true;
  update();
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

  if (hasMovieFrame(_playbackFrameIndex)) {
    QPainter painter(this);
    painter.drawImage(0, 0, _movieFrames[_playbackFrameIndex]);
    return;
  }

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

  // Winds are cached separately because their paired asynchronous requests
  // often complete after the field section. Replacing this light overlay
  // must not force filled-contour geometry to be regenerated.
  if (_windsPixmapDirty || _windsPixmap.size() != size()) {
    _windsPixmap = QPixmap(size());
    _windsPixmap.fill(Qt::transparent);
    QPainter windsPainter(&_windsPixmap);
    _drawWinds(windsPainter);
    _windsPixmapDirty = false;
  }
  painter.drawPixmap(0, 0, _windsPixmap);

  // Ground is an opaque mask, not a basemap. Draw it after field and wind
  // data so nothing below the sampled surface remains visible.
  if (_terrainPixmapDirty || _terrainPixmap.size() != size()) {
    _terrainPixmap = QPixmap(size());
    _terrainPixmap.fill(Qt::transparent);
    QPainter terrainPainter(&_terrainPixmap);
    _drawTerrain(terrainPainter);
    _terrainPixmapDirty = false;
  }
  painter.drawPixmap(0, 0, _terrainPixmap);

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
  _rebuildSelectedHorizHeightCurve();
  _savedZooms.clear();
  configureWorldCoords();
  update();
}

/////////////////////////////////////////////////////////////
// manage immutable composites used by vertical movie playback

void VertView::clearMovieFrames(int frameCount)
{
  _playbackFrameIndex = -1;
  _movieFrames.assign(std::max(0, frameCount), QImage());
  update();
}

bool VertView::hasMovieFrame(int frameIndex) const
{
  return frameIndex >= 0 &&
    frameIndex < static_cast<int>(_movieFrames.size()) &&
    !_movieFrames[frameIndex].isNull();
}

QImage VertView::movieFrameImage(int frameIndex) const
{
  if (!hasMovieFrame(frameIndex)) {
    return QImage();
  }
  return _movieFrames[frameIndex];
}

bool VertView::captureMovieFrame(int frameIndex)
{
  if (frameIndex < 0 || frameIndex >= static_cast<int>(_movieFrames.size()) ||
      width() <= 0 || height() <= 0) {
    return false;
  }

  _playbackFrameIndex = -1;
  QImage frame(size(), QImage::Format_ARGB32_Premultiplied);
  frame.fill(_backgroundBrush.color());
  // QWidget::render() invokes paintEvent synchronously, so the expensive
  // section pixmap and all legends represent this exact MDV completion even
  // when the vertical window is obscured.
  render(&frame);
  _movieFrames[frameIndex] = std::move(frame);
  return !_movieFrames[frameIndex].isNull();
}

bool VertView::showMovieFrame(int frameIndex)
{
  if (!hasMovieFrame(frameIndex)) {
    return false;
  }
  _playbackFrameIndex = frameIndex;
  repaint();
  return true;
}

void VertView::stopMoviePlayback()
{
  _playbackFrameIndex = -1;
  repaint();
}

void VertView::setMovieInteractionActive(bool active)
{
  if (!active) {
    return;
  }
  _leftDragActive = false;
  _mouseValueVisible = false;
  _mouseValueText.clear();
  _rubberBand->hide();
  update();
}

/////////////////////////////////////////////////////////////
// find a valid radar location for the displayed polar field

bool VertView::_getRadarLocation(double &latitude, double &longitude,
                                 double &altitudeKm) const
{
  if (_sectionReader == nullptr) {
    return false;
  }

  // The vertical response normally retains the source sensor metadata. Use
  // the horizontal volume as a fallback because some servers omit sensor
  // details while constructing a vertical-section response.

  const auto locationIsValid = [](const Mdvx::master_header_t &mhdr) {
    return std::isfinite(mhdr.sensor_lat) &&
      std::isfinite(mhdr.sensor_lon) &&
      std::isfinite(mhdr.sensor_alt) &&
      mhdr.sensor_lat >= -90.0 && mhdr.sensor_lat <= 90.0 &&
      mhdr.sensor_lon >= -360.0 && mhdr.sensor_lon <= 360.0 &&
      mhdr.sensor_alt > -10.0 && mhdr.sensor_alt < 100.0;
  };

  const Mdvx::master_header_t *mhdr = nullptr;
  if (locationIsValid(_sectionReader->v_mhdr)) {
    mhdr = &_sectionReader->v_mhdr;
  } else if (locationIsValid(_sectionReader->h_mhdr)) {
    mhdr = &_sectionReader->h_mhdr;
  }
  if (mhdr == nullptr) {
    return false;
  }

  latitude = mhdr->sensor_lat;
  longitude = mhdr->sensor_lon;
  altitudeKm = mhdr->sensor_alt;
  return true;
}

/////////////////////////////////////////////////////////////
// cache the selected polar beam height along the section route

void VertView::_rebuildSelectedHorizHeightCurve()
{
  _selectedHorizHeightCurve.clear();
  if (_sectionReader == nullptr || !_sectionReader->isValidV() ||
      _sectionReader->h_fhdr.proj_type != Mdvx::PROJ_POLAR_RADAR ||
      !std::isfinite(_selectedHorizHeightKm)) {
    return;
  }

  double radarLat, radarLon, radarAltitudeKm;
  if (!_getRadarLocation(radarLat, radarLon, radarAltitudeKm)) {
    return;
  }

  const double cosElevation =
    cos(_selectedHorizHeightKm * DEG_TO_RAD);
  if (!std::isfinite(cosElevation) || cosElevation <= 1.0e-6) {
    return;
  }

  const int nx = _sectionReader->v_fhdr.nx;
  const double dx = _sectionReader->v_fhdr.grid_dx;
  double totalLengthKm = _sectionReader->getVertTotalLengthKm();
  if (nx <= 0 || !std::isfinite(dx)) {
    return;
  }
  if (!std::isfinite(totalLengthKm) || totalLengthKm <= 0.0) {
    totalLengthKm = (nx - 1) * fabs(dx);
  }
  if (!std::isfinite(totalLengthKm) || totalLengthKm <= 0.0) {
    return;
  }

  // Include both route endpoints in addition to the grid-column centers so
  // the reference reaches the displayed section bounds. Sorting also handles
  // unusual responses whose X spacing is negative.

  vector<double> distancesKm;
  distancesKm.reserve(nx + 2);
  distancesKm.push_back(0.0);
  for (int ix = 0; ix < nx; ++ix) {
    const double distanceKm = _sectionReader->v_fhdr.grid_minx + ix * dx;
    if (std::isfinite(distanceKm)) {
      distancesKm.push_back(std::max(0.0,
                                     std::min(distanceKm, totalLengthKm)));
    }
  }
  distancesKm.push_back(totalLengthKm);
  sort(distancesKm.begin(), distancesKm.end());
  distancesKm.erase(unique(distancesKm.begin(), distancesKm.end()),
                    distancesKm.end());

  _beamHt.setInstrumentHtKm(radarAltitudeKm);
  _selectedHorizHeightCurve.reserve(distancesKm.size());
  for (double distanceKm : distancesKm) {
    double latitude, longitude;
    if (!_routeLocation(distanceKm, latitude, longitude)) {
      _selectedHorizHeightCurve.clear();
      return;
    }

    double groundRangeKm, azimuthDeg;
    PJGLatLon2RTheta(radarLat, radarLon, latitude, longitude,
                    &groundRangeKm, &azimuthDeg);
    if (!std::isfinite(groundRangeKm) || groundRangeKm < 0.0) {
      _selectedHorizHeightCurve.clear();
      return;
    }
    const double slantRangeKm = groundRangeKm / cosElevation;
    const double beamHeightKm =
      _beamHt.computeHtKm(_selectedHorizHeightKm, slantRangeKm);
    if (!std::isfinite(slantRangeKm) || slantRangeKm < 0.0 ||
        !std::isfinite(beamHeightKm)) {
      _selectedHorizHeightCurve.clear();
      return;
    }
    _selectedHorizHeightCurve.push_back
      (QPointF(distanceKm, beamHeightKm));
  }
}

/////////////////////////////////////////////////////////////
// draw the selected horizontal level as a line or polar beam curve

void VertView::_drawSelectedHorizHeight(QPainter &painter)
{
  if (_sectionReader == nullptr || !_sectionReader->isValidV() ||
      !_plotSelectedHorizHeight ||
      !std::isfinite(_selectedHorizHeightKm)) {
    return;
  }

  const bool polarRadar = _sectionReader != nullptr &&
    _sectionReader->h_fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR;
  if (polarRadar && _selectedHorizHeightCurve.size() < 2) {
    // Do not fall back to a constant line: for a polar field the selected
    // value is an elevation angle, and a missing radar location makes its
    // corresponding height unknown.
    return;
  }

  painter.save();
  _zoomWorld.setClippingOn(painter);
  QPen selectedHeightPen(QColor(_params.route_path_color));
  selectedHeightPen.setWidth(2);
  selectedHeightPen.setCapStyle(Qt::RoundCap);
  selectedHeightPen.setJoinStyle(Qt::RoundJoin);
  painter.setPen(selectedHeightPen);

  if (polarRadar) {
    QPainterPath path;
    path.moveTo(_zoomWorld.getPixelPointF
                (_selectedHorizHeightCurve[0].x(),
                 _selectedHorizHeightCurve[0].y()));
    for (size_t ii = 1; ii < _selectedHorizHeightCurve.size(); ++ii) {
      path.lineTo(_zoomWorld.getPixelPointF
                  (_selectedHorizHeightCurve[ii].x(),
                   _selectedHorizHeightCurve[ii].y()));
    }
    painter.drawPath(path);
  } else {
    const double yPixel = _zoomWorld.getYPixel(_selectedHorizHeightKm);
    painter.drawLine(QLineF(_zoomWorld.getXMinPixel(), yPixel,
                            _zoomWorld.getXMaxPixel(), yPixel));
  }
  painter.restore();
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

  if (_sectionReader->render_method == FILLED_CONTOURS) {
    // A contour failure falls back to cells so valid data is not lost. NONE
    // deliberately bypasses both fill paths while retaining optional lines.
    if (!_drawSectionContours(painter)) {
      _drawSectionCells(painter);
    }
  } else if (_sectionReader->render_method == POLYGONS) {
    _drawSectionCells(painter);
  }
  if (_sectionReader->overlay_line_contours) {
    _drawSectionContourLines(painter);
  }
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

/////////////////////////////////////////////////////////////
// draw line contours over the selected vertical-section fill

void VertView::_drawSectionContourLines(QPainter &painter)
{
  const int nx = _sectionReader->v_fhdr.nx;
  const int nz = _sectionReader->v_fhdr.nz;
  if (nx < 2 || nz < 2 ||
      _sectionReader->v_fl32_data.size() != (size_t) nx * (size_t) nz ||
      !std::isfinite(_sectionReader->cont_low) ||
      !std::isfinite(_sectionReader->cont_high) ||
      !std::isfinite(_sectionReader->cont_interv) ||
      _sectionReader->cont_interv <= 0.0 ||
      _sectionReader->cont_high < _sectionReader->cont_low) {
    return;
  }

  MdvxContour contours;
  const int maxContourLevels = 10000;
  int nlevels = 0;
  for (double value = _sectionReader->cont_low;
       value <= _sectionReader->cont_high +
                  _sectionReader->cont_interv * 1.0e-6 &&
         nlevels < maxContourLevels;
       value = _sectionReader->cont_low +
                 (++nlevels) * _sectionReader->cont_interv) {
    contours.addVal(value);
  }
  if (nlevels == 0 || nlevels >= maxContourLevels) {
    return;
  }

  // Keep the VSECTION header and nonuniform level coordinates intact so the
  // line geometry shares exactly the same distance/height space as the fill.
  Mdvx::field_header_t contourHeader = _sectionReader->v_fhdr;
  contourHeader.encoding_type = Mdvx::ENCODING_FLOAT32;
  contourHeader.compression_type = Mdvx::COMPRESSION_NONE;
  contourHeader.data_element_nbytes = sizeof(fl32);
  contourHeader.volume_size = nx * contourHeader.ny * nz * sizeof(fl32);
  MdvxField contourField(contourHeader, _sectionReader->v_vhdr,
                         _sectionReader->v_fl32_data.data());
  if (contours.computeLines(contourField)) {
    cerr << "ERROR - VertView::_drawSectionContourLines" << endl;
    cerr << "  Cannot compute contour lines" << endl;
    return;
  }

  painter.save();
  _zoomWorld.setClippingOn(painter);
  QPen pen(QColor(_params.foreground_color));
  pen.setWidth(std::max(1, _params.contour_line_width));
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);

  for (const MdvxContourLevel &level : contours.getLevels()) {
    for (const MdvxContourLine &line : level.lines) {
      if (line.pts.size() < 2) {
        continue;
      }
      QPainterPath path;
      path.moveTo(_zoomWorld.getPixelPointF(line.pts[0].x,
                                             line.pts[0].y));
      for (size_t ii = 1; ii < line.pts.size(); ++ii) {
        path.lineTo(_zoomWorld.getPixelPointF(line.pts[ii].x,
                                               line.pts[ii].y));
      }
      painter.drawPath(path);
    }
  }
  painter.restore();
}

/////////////////////////////////////////////////////////////
// render earth-relative horizontal winds over the vertical section

void VertView::_drawWinds(QPainter &painter)
{
  if (_sectionReader == nullptr || !_sectionReader->isValidV() ||
      _gd.wind_mgr == nullptr || !_gd.wind_mgr->isEnabled()) {
    return;
  }

  painter.save();
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setRenderHint(QPainter::TextAntialiasing, true);
  _zoomWorld.setClippingOn(painter);

  const double xMin = min(_zoomWorld.getXMinWorld(),
                          _zoomWorld.getXMaxWorld());
  const double xMax = max(_zoomWorld.getXMinWorld(),
                          _zoomWorld.getXMaxWorld());
  const double yMin = min(_zoomWorld.getYMinWorld(),
                          _zoomWorld.getYMaxWorld());
  const double yMax = max(_zoomWorld.getYMinWorld(),
                          _zoomWorld.getYMaxWorld());
  const double pixelsPerKm = fabs(_zoomWorld.getXPixel(1.0) -
                                  _zoomWorld.getXPixel(0.0));
  const double scaleSeconds = _params.wind_scaler *
    _params.wind_time_scale_interval * 60.0;
  const double plotWidth = fabs(_zoomWorld.getXMaxPixel() -
                                _zoomWorld.getXMinPixel());
  const double plotHeight = fabs(_zoomWorld.getYMaxPixel() -
                                 _zoomWorld.getYMinPixel());

  bool referenceDrawn = false;
  for (int layerIndex = 0;
       layerIndex < _gd.wind_mgr->layerCount(); ++layerIndex) {
    if (!_gd.wind_mgr->verticalLayerReady(layerIndex)) {
      continue;
    }

    const wind_data_t &wind = _gd.layers.wind[layerIndex];
    const MdvReader *uReader = wind.wind_u;
    const MdvReader *vReader = wind.wind_v;
    if (uReader == nullptr || vReader == nullptr ||
        uReader->v_fhdr.vlevel_type != _sectionReader->v_fhdr.vlevel_type) {
      // A glyph can be compass-oriented independently of the plot axes, but
      // its anchor height cannot be translated reliably between unlike MDV
      // vertical coordinates. Suppress such a layer rather than misplacing it.
      if (_params.debug >= Params::DEBUG_VERBOSE && uReader != nullptr &&
          uReader->v_fhdr.vlevel_type != _sectionReader->v_fhdr.vlevel_type) {
        cerr << "Vertical wind layer " << layerIndex
             << " uses an incompatible vertical coordinate" << endl;
      }
      continue;
    }

    const int nx = uReader->v_fhdr.nx;
    const int nz = uReader->v_fhdr.nz;
    vector<int> visibleX;
    vector<int> visibleZ;
    visibleX.reserve(nx);
    visibleZ.reserve(nz);
    for (int ix = 0; ix < nx; ++ix) {
      const double distanceKm = uReader->v_fhdr.grid_minx +
        ix * uReader->v_fhdr.grid_dx;
      if (distanceKm >= xMin && distanceKm <= xMax) {
        visibleX.push_back(ix);
      }
    }
    for (int iz = 0; iz < nz; ++iz) {
      const double level = uReader->v_vhdr.level[iz];
      if (level >= yMin && level <= yMax) {
        visibleZ.push_back(iz);
      }
    }
    if (visibleX.empty() || visibleZ.empty()) {
      continue;
    }

    // Density follows the visible subsection so zooming reveals detail
    // without making a large downloaded model domain expensive to draw.
    const int xStride = max(1, static_cast<int>(floor(
      visibleX.size() /
        static_cast<double>(max(1, _params.wind_ideal_x_vectors)) + 0.5)));
    const int zStride = max(1, static_cast<int>(floor(
      visibleZ.size() /
        static_cast<double>(max(1, _params.wind_ideal_y_vectors)) + 0.5)));

    QColor color(wind.color_name);
    if (!color.isValid()) {
      color = Qt::white;
    }
    const double lineWidth = max(1, wind.line_width);
    const fl32 uMissing = uReader->v_fhdr.missing_data_value;
    const fl32 uBad = uReader->v_fhdr.bad_data_value;
    const fl32 vMissing = vReader->v_fhdr.missing_data_value;
    const fl32 vBad = vReader->v_fhdr.bad_data_value;

    for (size_t jz = 0; jz < visibleZ.size(); jz += zStride) {
      const int iz = visibleZ[jz];
      const double level = uReader->v_vhdr.level[iz];
      for (size_t jx = 0; jx < visibleX.size(); jx += xStride) {
        const int ix = visibleX[jx];
        const size_t dataIndex = static_cast<size_t>(iz) * nx + ix;
        const fl32 rawU = uReader->v_fl32_data[dataIndex];
        const fl32 rawV = vReader->v_fl32_data[dataIndex];
        if (!isfinite(rawU) || !isfinite(rawV) || rawU == uMissing ||
            rawU == uBad || rawV == vMissing || rawV == vBad) {
          continue;
        }

        const double uMps = rawU * wind.data_units_scale_to_mps;
        const double vMps = rawV * wind.data_units_scale_to_mps;
        const double speedMps = hypot(uMps, vMps);
        if (!isfinite(speedMps)) {
          continue;
        }
        const double distanceKm = uReader->v_fhdr.grid_minx +
          ix * uReader->v_fhdr.grid_dx;
        const QPointF anchor =
          _zoomWorld.getPixelPointF(distanceKm, level);

        // These are compass symbols superimposed on a distance-height plot.
        // Use the distance-axis pixel scale for both components: east is
        // screen-right and true north is screen-up. The height transform must
        // not rotate or rescale the horizontal wind direction.
        const QPointF screenVelocity(
          uMps * scaleSeconds / 1000.0 * pixelsPerKm,
          -vMps * scaleSeconds / 1000.0 * pixelsPerKm);
        if (wind.marker_type == ARROWS || wind.marker_type == VECTOR ||
            wind.marker_type == TUFT || wind.marker_type == TICKVECTOR) {
          if (speedMps < 0.1 ||
              fabs(screenVelocity.x()) >= plotWidth / 4.0 ||
              fabs(screenVelocity.y()) >= plotHeight / 4.0) {
            continue;
          }
          const QPointF tail = wind.marker_type == ARROWS ?
            anchor - 0.5 * screenVelocity : anchor;
          const QPointF head = wind.marker_type == ARROWS ?
            anchor + 0.5 * screenVelocity : anchor + screenVelocity;
          if (wind.marker_type == TUFT) {
            painter.setPen(QPen(color, lineWidth));
            painter.drawEllipse(anchor, 1.5, 1.5);
            painter.drawLine(anchor, head);
          } else {
            WindGlyphRenderer::drawArrow(
              painter, tail, head, color, lineWidth,
              _params.wind_head_size + 3.0 * (lineWidth - 1.0),
              _params.wind_head_angle);
            if (wind.marker_type == TICKVECTOR) {
              const int tickCount = max(1, _params.wind_scaler - 1);
              const double length = hypot(screenVelocity.x(),
                                          screenVelocity.y());
              if (length > 0.5) {
                const QPointF unit = screenVelocity / length;
                const QPointF side(-unit.y(), unit.x());
                painter.setPen(QPen(color, lineWidth));
                for (int itick = 1; itick <= tickCount; ++itick) {
                  const QPointF at = anchor + screenVelocity *
                    (itick / static_cast<double>(tickCount + 1));
                  painter.drawLine(at - 2.0 * side, at + 2.0 * side);
                }
              }
            }
          }
          continue;
        }

        QPointF toward(uMps, -vMps);
        if (speedMps < 1.0e-6) {
          toward = QPointF(0.0, -1.0);
        }
        const double directionDeg = fmod(
          atan2(-uMps, -vMps) * 180.0 / M_PI + 360.0, 360.0);
        bool southern = false;
        WindGlyphRenderer::LabelStyle labelStyle =
          WindGlyphRenderer::LabelStyle::None;
        if (wind.marker_type == BARB_SH ||
            wind.marker_type == LABELEDBARB_SH) {
          southern = true;
        } else if (wind.marker_type == METBARB) {
          double lat = 0.0;
          double lon = 0.0;
          if (_routeLocation(distanceKm, lat, lon)) {
            southern = lat < 0.0;
          }
        }
        if (wind.marker_type == LABELEDBARB ||
            wind.marker_type == LABELEDBARB_SH) {
          labelStyle = WindGlyphRenderer::LabelStyle::DirectionAtOrigin;
        } else if (wind.marker_type == METBARB) {
          labelStyle = WindGlyphRenderer::LabelStyle::DirectionTensAtTip;
        }
        WindGlyphRenderer::drawBarb(
          painter, anchor, -toward, speedMps * 1.9438444924406,
          _params.wind_barb_shaft_len, lineWidth, color, southern,
          labelStyle, directionDeg);
      }
    }

    if (!referenceDrawn && _params.display_ref_lines &&
        _params.wind_legends_on && pixelsPerKm > 0.0 &&
        (wind.marker_type == ARROWS || wind.marker_type == VECTOR ||
         wind.marker_type == TUFT || wind.marker_type == TICKVECTOR)) {
      const double referencePixels = wind.reference_speed *
        wind.units_scale_factor * scaleSeconds / 1000.0 * pixelsPerKm;
      if (referencePixels > 1.0 && referencePixels < plotWidth / 2.0) {
        const QPointF tail(min(_zoomWorld.getXMinPixel(),
                               _zoomWorld.getXMaxPixel()) + 12.0,
                           min(_zoomWorld.getYMinPixel(),
                               _zoomWorld.getYMaxPixel()) + 12.0);
        const QPointF head = tail + QPointF(referencePixels, 0.0);
        WindGlyphRenderer::drawArrow(
          painter, tail, head, color, lineWidth,
          _params.wind_head_size, _params.wind_head_angle);
        painter.setPen(color);
        painter.drawText(head + QPointF(5.0, 4.0),
                         QString("%1: %2 %3")
                           .arg(QString::fromStdString(
                             wind.wind_u->legend_name))
                           .arg(wind.reference_speed)
                           .arg(wind.units_label ? wind.units_label : ""));
        referenceDrawn = true;
      }
    }
  }
  painter.restore();
}

/////////////////////////////////////////////////////////////
// draw the sampled terrain surface and mask the earth below it

void VertView::_drawTerrain(QPainter &painter)
{
  const MdvReader *terrain = _gd.layers.earth.terr;
  if (_sectionReader == nullptr || !_sectionReader->isValidV() ||
      _sectionReader->v_fhdr.vlevel_type != Mdvx::VERT_TYPE_Z ||
      !_gd.layers.earth.terrain_active || terrain == nullptr ||
      !terrain->isValidV() ||
      !isfinite(_params.terrain_height_scaler)) {
    return;
  }

  const int nx = terrain->v_fhdr.nx;
  if (nx < 2 || terrain->v_fl32_data.size() < static_cast<size_t>(nx)) {
    return;
  }

  QColor skinColor(_params.terrain_earth_color1);
  QColor coreColor(_params.terrain_earth_color2);
  if (!skinColor.isValid()) {
    skinColor = QColor("sienna");
  }
  if (!coreColor.isValid()) {
    coreColor = QColor(139, 0, 0); // X11 red4, the legacy default
  }

  painter.save();
  painter.setRenderHint(QPainter::Antialiasing, true);
  _zoomWorld.setClippingOn(painter);

  const double plotBottom = max(_zoomWorld.getYMinPixel(),
                                _zoomWorld.getYMaxPixel());
  const fl32 missing = terrain->v_fhdr.missing_data_value;
  const fl32 bad = terrain->v_fhdr.bad_data_value;
  vector<QPointF> surface;
  surface.reserve(nx);

  // Invalid terrain samples delimit independent ground polygons. Bridging a
  // data gap would hide valid section data beneath an invented surface.
  const auto drawSurface = [&]() {
    if (surface.size() < 2) {
      surface.clear();
      return;
    }

    QPainterPath ground;
    ground.moveTo(surface.front());
    for (size_t ii = 1; ii < surface.size(); ++ii) {
      ground.lineTo(surface[ii]);
    }
    ground.lineTo(surface.back().x(), plotBottom);
    ground.lineTo(surface.front().x(), plotBottom);
    ground.closeSubpath();
    painter.fillPath(ground, coreColor);

    QPainterPath skin;
    skin.moveTo(surface.front());
    for (size_t ii = 1; ii < surface.size(); ++ii) {
      skin.lineTo(surface[ii]);
    }
    QPen skinPen(skinColor, 3.0);
    skinPen.setCapStyle(Qt::RoundCap);
    skinPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(skinPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(skin);
    surface.clear();
  };

  for (int ix = 0; ix < nx; ++ix) {
    const fl32 rawHeight = terrain->v_fl32_data[ix];
    if (!isfinite(rawHeight) || rawHeight == missing || rawHeight == bad) {
      drawSurface();
      continue;
    }

    const double distanceKm = terrain->v_fhdr.grid_minx +
      ix * terrain->v_fhdr.grid_dx;
    const double height = rawHeight * _params.terrain_height_scaler;
    if (!isfinite(distanceKm) || !isfinite(height)) {
      drawSurface();
      continue;
    }
    surface.push_back(_zoomWorld.getPixelPointF(distanceKm, height));
  }
  drawSurface();
  painter.restore();
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
// set the unzoomed height range

void VertView::setHeightRange(double minHeightKm, double maxHeightKm)
{
  if (maxHeightKm <= minHeightKm) {
    return;
  }

  // A configured range defines a new base view, so zoom history created
  // against the previous range is no longer meaningful.

  _minHeightKm = minHeightKm;
  _maxHeightKm = maxHeightKm;
  _savedZooms.clear();
  configureWorldCoords();
}

////////////////////////////////////////////////////////
// set the level selected in the horizontal view

void VertView::setSelectedHorizHeight(double heightKm)
{
  if (_selectedHorizHeightKm == heightKm) {
    return;
  }
  _selectedHorizHeightKm = heightKm;
  _rebuildSelectedHorizHeightCurve();
  update();
}

////////////////////////////////////////////////////////
// set visibility of the horizontal selected-height line

void VertView::setPlotSelectedHorizHeight(bool enabled)
{
  _plotSelectedHorizHeight = enabled;
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
  // The installed reader is the authoritative section identity. Comparing
  // reader pointers avoids a separate field index becoming stale whenever a
  // new field's asynchronous vertical request completes.
  if (field_num >= (size_t) _gd.num_datafields ||
      _gd.mread[field_num] != _sectionReader) {
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

  if (_manager.isMovieActive()) {
    e->accept();
    return;
  }

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

  if (_manager.isMovieActive()) {
    return;
  }

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
  

