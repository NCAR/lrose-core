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
        _gridsEnabled(false),
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
  
  double xMax = 100.0;
  double yMin = 0.0;
  double yMax = _maxHeightKm;
  if (_sectionReader != nullptr && _sectionReader->isValidV()) {
    xMax = _sectionReader->getVertTotalLengthKm();
    if (xMax <= 0.0) {
      xMax = (_sectionReader->v_fhdr.nx - 1) *
             _sectionReader->v_fhdr.grid_dx;
    }
    const vector<double> edges = _computeVerticalEdges();
    if (edges.size() >= 2) {
      const auto limits = std::minmax_element(edges.begin(), edges.end());
      yMin = *limits.first;
      yMax = *limits.second;
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

  _fullWorld.setTitleFontSize(_params.vert_title_font_size);
  _fullWorld.setAxisLabelFontSize(_params.vert_axis_label_font_size);
  _fullWorld.setTickValuesFontSize(_params.vert_tick_values_font_size);
  _fullWorld.setLegendFontSize(_params.vert_legend_font_size);

  _fullWorld.setTitleColor(_params.vert_title_color);
  _fullWorld.setAxisLineColor(_params.vert_axes_color);
  _fullWorld.setAxisTextColor(_params.vert_axes_color);
  _fullWorld.setGridColor(_params.vert_grid_color);

  _fullWorld.setYAxisLabelsInside(_params.vert_tick_values_inside);
  _fullWorld.setXAxisLabelsInside(_params.vert_tick_values_inside);

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
  
  painter.setPen(_params.vert_grid_color);

  // Draw the axes

  double xMin = _zoomWorld.getXMinWorld();
  double yMin = _zoomWorld.getYMinWorld();
  
  QFont font = painter.font();
  font.setPointSizeF(_params.vert_label_font_size);
  painter.setFont(font);
  
  _zoomWorld.specifyXTicks(xMin, _xGridSpacing);
  _zoomWorld.specifyYTicks(yMin, _yGridSpacing);

  _zoomWorld.drawAxisTop(painter, "km", true, true, false, _gridsEnabled,
                         false);
  _zoomWorld.drawAxisBottom(painter, "km", true, true, true, false);
  
  const string verticalUnits = _verticalUnits.empty() ? "km" : _verticalUnits;
  _zoomWorld.drawAxisLeft(painter, verticalUnits, true, true, true,
                          _gridsEnabled);
  _zoomWorld.drawAxisRight(painter, verticalUnits, true, true, false,
                           false);
    
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

  if (_sectionReader != nullptr) {
    // The compact vertical window needs the scale shifted into its plot area,
    // while its units sit above the highest numeric label instead of
    // competing with it.
    _zoomWorld.drawColorScale(_sectionReader->colorMap, painter,
                              _params.horiz_axis_label_font_size);
    _zoomWorld.drawTitleTopCenter(painter, _sectionReader->fieldLabelVert());
  }
  
  // add legends with time, field name and elevation angle

#ifdef NOTNOW
  
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

#endif

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
  // _updateRenderers();

}

/*************************************************************************
 * adjust pixel scale for correct aspect ratio etc
 */
void VertView::updatePixelScales()
{
}

/*************************************************************************
 * paintEvent()
 */

void VertView::paintEvent(QPaintEvent *event)
{

  QPainter painter(this);
  painter.fillRect(rect(), _backgroundBrush);
  _drawSection(painter);
  _drawOverlays(painter);

}

/////////////////////////////////////////////////////////////
// install a gridded MDV vertical section

void VertView::setSection(const MdvReader *reader)
{
  _sectionReader = reader;
  _verticalUnits.clear();
  if (_sectionReader != nullptr) {
    _verticalUnits = Mdvx::vertTypeZUnits(_sectionReader->v_fhdr.vlevel_type);
  }
  _savedZooms.clear();
  configureWorldCoords();
  update();
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
        if (value == missing || value == bad || !std::isfinite(value)) {
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
  const fl32 minDataValue = _sectionReader->v_fhdr.min_value;

  // Match horizontal contour behavior: missing values remain holes unless
  // the mapping parameters explicitly place them at the data minimum.
  for (fl32 &value : contourData) {
    if (value == bad) {
      value = _params.map_bad_to_min_value ? minDataValue : missing;
    }
    if (value == missing && _params.map_missing_to_min_value) {
      value = minDataValue;
    }
  }

  bool haveValidData = false;
  double validDataMax = 0.0;
  for (fl32 value : contourData) {
    if (value == missing) {
      continue;
    }
    double displayValue = value;
    if (_sectionReader->v_fhdr.transform_type == Mdvx::DATA_TRANSFORM_LOG) {
      displayValue = exp(displayValue);
    }
    if (!std::isfinite(displayValue)) {
      continue;
    }
    if (!haveValidData) {
      validDataMax = displayValue;
      haveValidData = true;
    } else {
      validDataMax = MAX(validDataMax, displayValue);
    }
  }
  if (haveValidData) {
    // Keep the lower color boundary as a physical contour. Extending it to
    // the data minimum colors weak model reflectivity all the way to the
    // model top instead of leaving the region below the scale unfilled.
    contourValues.back() = MAX(contourValues.back(), validDataMax);
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

  const QBrush backgroundBrush(_params.background_color);
  painter.save();
  _zoomWorld.setClippingOn(painter);
  painter.setPen(Qt::NoPen);
  for (const MdvxContourLevel &level : contours.getLevels()) {
    if (level.index < 0) {
      continue;
    }

    const QBrush *levelBrush = &backgroundBrush;
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

void VertView::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);

  // Rebuild the world-to-pixel transform from the geometry assigned by the
  // layout so data, grids, and labels all cover the same drawable area.
  _resetWorld(event->size().width(), event->size().height());
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


