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

#include "VertWidget.hh"
#include "VertWindow.hh"
#include <toolsa/toolsa_macros.h>
#include <Radx/RadxRay.hh>
#include "cidd.h"
using namespace std;

VertWidget::VertWidget(QWidget* parent,
                       const CartManager &manager,
                       const VertWindow &vertWindow) :
        CartWidget(parent, manager),
        _vertWindow(vertWindow),
        _beamsProcessed(0)
        
{
  
  _prevElev = -9999.0;
  _prevAz = -9999.0;
  _prevTime = 0;
  
  // _aspectRatio = _params.vert_aspect_ratio;
  _colorScaleWidth = _params.vert_color_scale_width;

  setGrids(_params.vert_grids_on_at_startup);
  setRings(_params.vert_range_rings_on_at_startup);
  setAngleLines(_params.vert_elevation_lines_on_at_startup);

  // initialize world view

  _maxHeightKm = _params.vert_max_height_km;
  _xGridSpacing = 0.0;
  _yGridSpacing = 0.0;
  // configureRange(_params.max_range_km);

  // set up ray locators

  // _rayLoc.resize(RayLoc::RAY_LOC_N);

  // archive mode

  _isArchiveMode = false;
  _isStartOfSweep = true;

  _plotStartTime.set(0);
  _plotEndTime.set(0);
  _meanAz = -9999.0;
  _sumAz = 0.0;
  _nRays = 0.0;

}


/*************************************************************************
 * Destructor
 */

VertWidget::~VertWidget()
{

  // delete all of the dynamically created beams

  // for (size_t i = 0; i < _vertBeams.size(); ++i) {
  //   Beam::deleteIfUnused(_vertBeams[i]);
  // }
  // _vertBeams.clear();

}


#ifdef JUNK
/*************************************************************************
 * addBeam()
 */

void VertWidget::addBeam(const RadxRay *ray,
                        const std::vector< std::vector< double > > &beam_data,
                        const std::vector< DisplayField* > &fields)
{

  // compute the angle limits, and store the location of this ray

  _computeAngleLimits(ray);
  _storeRayLoc(ray);

  // Add the beam to the beam list

  double instHtKm = _manager.getPlatform().getAltitudeKm();
  if (instHtKm < -1.0) {
    instHtKm = 0.0;
  }

  // VertBeam* beam = new VertBeam(_params, ray, instHtKm,
  //                             _fields.size(), _startElev, _endElev);
  // beam->addClient();

  // if ((int) _vertBeams.size() == _params.vert_beam_queue_size) {
  //   VertBeam *oldBeam = _vertBeams.front();
  //   Beam::deleteIfUnused(oldBeam);
  //   _vertBeams.pop_front();
  // }
  // _vertBeams.push_back(beam);

  // compute angles and times in archive mode
  
  if (_isArchiveMode) {

    if (_isStartOfSweep) {
      _plotStartTime = ray->getRadxTime();
      _meanAz = -9999.0;
      _sumAz = 0.0;
      _nRays = 0.0;
      _isStartOfSweep = false;
    }
    _plotEndTime = ray->getRadxTime();
    _sumAz += ray->getAzimuthDeg();
    _nRays++;
    _meanAz = _sumAz / _nRays;

  } // if (_isArchiveMode) 
    
  // Render the beam data.
  
  // Set up the brushes for all of the fields in this beam.  This can be
  // done independently of a Painter object.
    
  // beam->fillColors(beam_data, fields, &_backgroundBrush);

  // Add the new beams to the render lists for each of the fields
  
  // for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
  //   if (field == _selectedField ||
  //       _fieldRenderers[field]->isBackgroundRendered()) {
  //     _fieldRenderers[field]->addBeam(beam);
  //   } else {
  //     beam->setBeingRendered(field, false);
  //   }
  // }
  
  // Start the threads to render the new beams
  
  _performRendering();

  // if (_params.debug >= Params::DEBUG_VERBOSE &&
  //     _vertBeams.size() % 10 == 0) {
  //   cerr << "==>> _vertBeams.size(): " << _vertBeams.size() << endl;
  // }

}
#endif

/*************************************************************************
 * configureWorldCoords()
 */

void VertWidget::configureWorldCoords(int zoomLevel)
{

  // Save the specified values
  
  // _maxRangeKm = max_range;

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
  
  _fullWorld.setWindowGeom(width(), height(), 0, 0);
  
  _fullWorld.setWorldLimits(0.0, 0.0,
                            _maxRangeKm, _maxHeightKm);
  
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

void VertWidget::mouseReleaseEvent(QMouseEvent *e)
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

const RadxRay *VertWidget::_getClosestRay(double xx, double yy)

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

void VertWidget::_setGridSpacing()
{

  double xRange = _zoomWorld.getXMaxWorld() - _zoomWorld.getXMinWorld();
  double yRange = _zoomWorld.getYMaxWorld() - _zoomWorld.getYMinWorld();

  _ringSpacing = _getSpacing(xRange);
  _xGridSpacing = _getSpacing(xRange);
  _yGridSpacing = _getSpacing(yRange);

}

/*************************************************************************
 * Get spacing for a given distance range
 */

double VertWidget::_getSpacing(double range)
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

void VertWidget::_drawOverlays(QPainter &painter)
{

  // save painter state

  painter.save();
  
  // store font
  
  QFont origFont = painter.font();
  
  // Set the painter to use the right color and font

  // painter.setWindow(_zoomWindow);
  
  painter.setPen(_gridRingsColor);

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
  
  // Draw rings

  if (_ringSpacing > 0.0 && _ringsEnabled) {

    // Draw the rings

    painter.save();
    painter.setTransform(_zoomTransform);

    // set narrow line width
    QPen pen = painter.pen();
    pen.setWidth(0);
    painter.setPen(pen);

    double ringRange = _ringSpacing;
    while (ringRange <= _maxRangeKm) {
      QRectF rect(-ringRange, -ringRange, ringRange * 2.0, ringRange * 2.0);
      painter.drawEllipse(rect);
      ringRange += _ringSpacing;
    }
    painter.restore();

    // Draw the labels
    
    QFont font = painter.font();
    font.setPointSizeF(_params.range_ring_label_font_size);
    painter.setFont(font);
    // painter.setWindow(0, 0, width(), height());
    
    ringRange = _ringSpacing;
    while (ringRange <= _maxRangeKm) {
      double labelPos = ringRange * SIN_45;
      const string &labelStr = _scaledLabel.scale(ringRange);
      _zoomWorld.drawText(painter, labelStr, labelPos, labelPos, Qt::AlignCenter);
      _zoomWorld.drawText(painter, labelStr, -labelPos, labelPos, Qt::AlignCenter);
      _zoomWorld.drawText(painter, labelStr, labelPos, -labelPos, Qt::AlignCenter);
      _zoomWorld.drawText(painter, labelStr, -labelPos, -labelPos, Qt::AlignCenter);
      ringRange += _ringSpacing;
    }

  } /* endif - draw rings */
  
  // Draw the azimuth lines

  if (_angleLinesEnabled) {

    // Draw the lines along the X and Y axes

    _zoomWorld.drawLine(painter, 0, -_maxRangeKm, 0, _maxRangeKm);
    _zoomWorld.drawLine(painter, -_maxRangeKm, 0, _maxRangeKm, 0);

    // Draw the lines along the 30 degree lines

    double end_pos1 = SIN_30 * _maxRangeKm;
    double end_pos2 = COS_30 * _maxRangeKm;
    
    _zoomWorld.drawLine(painter, end_pos1, end_pos2, -end_pos1, -end_pos2);
    _zoomWorld.drawLine(painter, end_pos2, end_pos1, -end_pos2, -end_pos1);
    _zoomWorld.drawLine(painter, -end_pos1, end_pos2, end_pos1, -end_pos2);
    _zoomWorld.drawLine(painter, end_pos2, -end_pos1, -end_pos2, end_pos1);

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

  int fieldNum = gd.h_win.page;
  const ColorMap &colorMap = *(gd.mrec[fieldNum]->colorMap);
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

///////////////////////////////////////////////////////////
// Compute the limits of the ray angles

void VertWidget::_computeAngleLimits(const RadxRay *ray)
  
{
  
  // double beamWidth = _platform.getRadarBeamWidthDegV();
  double elev = ray->getElevationDeg();

  // Determine the extent of this ray
  
  // double elevDiff = Radx::computeAngleDiff(elev, _prevElev);
  // if (ray->getIsIndexed() || fabs(elevDiff) > beamWidth * 4.0) {
    
  double halfAngle = ray->getAngleResDeg() / 2.0;
  if (_params.vert_override_rendering_beam_width) {
    halfAngle = _params.vert_rendering_beam_width / 2.0;
  }

  _startElev = elev - halfAngle;
  _endElev = elev + halfAngle;
    
  // } else {
    
  //   double maxHalfAngle = beamWidth / 2.0;
  //   double prevOffset = maxHalfAngle;
    
  //   double halfElevDiff = elevDiff / 2.0;
    
  //   if (prevOffset > halfElevDiff) {
  //     prevOffset = halfElevDiff;
  //   }
    
  //   _startElev = elev - prevOffset;
  //   _endElev = elev + maxHalfAngle;
    
  // }

  _prevElev = elev;

  double az = ray->getAzimuthDeg();
  double azDiff = Radx::computeAngleDiff(az, _prevAz);
  _prevAz = az;

  RadxTime rtime = ray->getRadxTime();
  double timeDiff = rtime - _prevTime;
  _prevTime = rtime;

  if (azDiff > 2.0 || timeDiff > 30) {
    clear();
  }
    
}

///////////////////////////////////////////////////////////
// store ray location

void VertWidget::_storeRayLoc(const RadxRay *ray)
{

#ifdef NOTNOW
  // compute start and end indices for _rayLoc
  // VERTs elevation range from -180 to 180
  // so add 180 to put the index in 0 to 3599 range

  int startIndex = (int) ((_startElev + 180.0) * RayLoc::RAY_LOC_RES);
  int endIndex = (int) ((_endElev + 180.0) * RayLoc::RAY_LOC_RES + 1);

  // Clear out any rays in the locations list that are overlapped by the
  // new ray
    
  _clearRayOverlap(startIndex, endIndex);
  
  // Set the locations associated with this ray

  for (int ii = startIndex; ii <= endIndex; ii++) {
    _rayLoc[ii].ray = ray;
    _rayLoc[ii].active = true;
    _rayLoc[ii].startIndex = startIndex;
    _rayLoc[ii].endIndex = endIndex;
  }
#endif

}

#ifdef NOTNOW
///////////////////////////////////////////////////////////
// clear any locations that are overlapped by the given ray

void VertWidget::_clearRayOverlap(const int startIndex,
                                 const int endIndex)
{
  // Loop through the ray locations, clearing out old information

  int i = startIndex;
  
  while (i <= endIndex)
  {
    RayLoc &loc = _rayLoc[i];
    
    // If this location isn't active, we can skip it

    if (!loc.active)
    {
      ++i;
      continue;
    }
    
    int locStartIndex = loc.startIndex;
    int locEndIndex = loc.endIndex;
      
    // If we get here, this location is active.  We now have 4 possible
    // situations:

    if (loc.startIndex < startIndex && loc.endIndex <= endIndex)
    {
      // The overlap area covers the end of the current beam.  Reduce the
      // current beam down to just cover the area before the overlap area.

      for (int j = startIndex; j <= locEndIndex; ++j)
      {
	_rayLoc[j].ray = NULL;
	_rayLoc[j].active = false;
      }

      // Update the end indices for the remaining locations in the current
      // beam

      for (int j = locStartIndex; j < startIndex; ++j)
	_rayLoc[j].endIndex = startIndex - 1;
    }
    else if (loc.startIndex < startIndex && loc.endIndex > endIndex)
    {
      // The current beam is bigger than the overlap area.  This should never
      // happen, so go ahead and just clear out the locations for the current
      // beam.

      for (int j = locStartIndex; j <= locEndIndex; ++j)
      {
        _rayLoc[j].clear();
      }
    }
    else if (loc.endIndex > endIndex)
    {
      // The overlap area covers the beginning of the current beam.  Reduce the
      // current beam down to just cover the area after the overlap area.

      for (int j = locStartIndex; j <= endIndex; ++j)
      {
	_rayLoc[j].ray = NULL;
	_rayLoc[j].active = false;
      }

      // Update the start indices for the remaining locations in the current
      // beam

      for (int j = endIndex + 1; j <= locEndIndex; ++j)
	_rayLoc[j].startIndex = endIndex + 1;
    }
    else
    {
      // The current beam is completely covered by the overlap area.  Clear
      // out all of the locations for the current beam.

      for (int j = locStartIndex; j <= locEndIndex; ++j)
      {
        _rayLoc[j].clear();
      }
    }
    
    i = locEndIndex + 1;
  } /* endwhile - i */
  
}
#endif

/*************************************************************************
 * _refreshImages()
 */

void VertWidget::_refreshImages()
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

void VertWidget::clear()
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

void VertWidget::refresh()
{
  _refreshImages();
}

/*************************************************************************
 * unzoom the view
 */

void VertWidget::unzoomView()
{
  
  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _setGridSpacing();
  _refreshImages();
  // _updateRenderers();

}

/*************************************************************************
 * adjust pixel scale for correct aspect ratio etc
 */
void VertWidget::adjustPixelScales()
{

  cerr << "==>> hhhhhh VertWidget::adjustPixelScales() <<==" << endl;

}

/*************************************************************************
 * resize()
 */

void VertWidget::resize(int width, int height)
{
  
  setGeometry(0, 0, width, height);
  _resetWorld(width, height);
  _refreshImages();

}

/*************************************************************************
 * paintEvent()
 */

void VertWidget::paintEvent(QPaintEvent *event)
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

void VertWidget::selectVar(const size_t index)
{

  // If the field index isn't actually changing, we don't need to do anything
  
  if (_selectedField == index) {
    return;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========>> VertWidget::selectVar() for field index: " 
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

int VertWidget::renderVMovieFrame(int index, QPainter &painter)
{
  int stat = 0;
#ifdef NOTYET
  int c_field = gd.v_win.page;
  
  if(gd.debug2) fprintf(stderr, "Rendering Vertical movie_frame %d - field %d\n", index, c_field);

  
  switch(gd.movie.mode) {
    case REALTIME_MODE:
    case ARCHIVE_MODE:
      stat = render_vert_display(xid, c_field,
                                 gd.movie.frame[index].time_start,
                                 gd.movie.frame[index].time_end);
      break;
         
  }
#endif
  return stat;
}

#ifdef NOTYET

/**********************************************************************
 * RENDER_VERT_DISPLAY: Render the vertical cross section display
 */

int VertWidget::renderVertDisplay(QPaintDevice *pdev,
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

  if(gd.debug2) fprintf(stderr,"Rendering Vertical Image, page :%d\n",page);
  /* Clear drawing area */
  XFillRectangle(gd.dpy,xid,gd.legends.background_color->gc,
                 0,0,gd.v_win.can_dim.width,gd.v_win.can_dim.height);

  if(!_params.draw_main_on_top) { 
    if(gd.mrec[page]->render_method == LINE_CONTOURS) {
      cont.min = gd.mrec[page]->cont_low;
      cont.max = gd.mrec[page]->cont_high;
      cont.interval = gd.mrec[page]->cont_interv;
      cont.active = 1;
      cont.field = page;
      cont.labels_on = _params.label_contours;
      cont.color = gd.legends.foreground_color;
      cont.vcm = &gd.mrec[page]->v_vcm;
      if (0) {   // Taiwan HACK - Buggy - do not use RenderLineContours()
        //if (gd.layers.use_alt_contours) {
        RenderLineContours(xid, &cont, true);
      } else {
        render_xsect_line_contours(xid,&cont);
      }
    } else {
      render_xsect_grid(xid,gd.mrec[page],start_time,end_time,0);
      // stat =  render_xsect_grid(xid,gd.mrec[page],start_time,end_time,0);
    }
  }
    
  /* Render each of the gridded_overlay fields */
  for(i=0; i < NUM_GRID_LAYERS; i++) {           
    if(gd.layers.overlay_field_on[i]) {
      render_xsect_grid(xid,gd.mrec[gd.layers.overlay_field[i]],start_time,end_time,1);
    }
  } 

  if(_params.draw_main_on_top) { 
    if(gd.mrec[page]->render_method == LINE_CONTOURS) {
      cont.min = gd.mrec[page]->cont_low;
      cont.max = gd.mrec[page]->cont_high;
      cont.interval = gd.mrec[page]->cont_interv;
      cont.active = 1;
      cont.field = page;
      cont.labels_on = _params.label_contours;
      cont.color = gd.legends.foreground_color;
      cont.vcm = &gd.mrec[page]->v_vcm;
      if (0) {   // Taiwan HACK - Buggy - do not use RenderLineContours()
        // if (gd.layers.use_alt_contours) {
        RenderLineContours(xid, &cont, true);
      } else {
        render_xsect_line_contours(xid,&cont);
      }
    } else {
      // stat =  render_xsect_grid(xid,gd.mrec[page],start_time,end_time,0);
      render_xsect_grid(xid,gd.mrec[page],start_time,end_time,0);
    }
  }

  /* render contours if selected */
  for(i=0; i < NUM_CONT_LAYERS; i++) {
    if(gd.layers.cont[i].active) {    
      if (0) {   // Taiwan HACK - Buggy - do not use RenderLineContours()
 	// if (gd.layers.use_alt_contours) {
        RenderLineContours(xid, &(gd.layers.cont[i]), true);
      } else {
        render_xsect_line_contours(xid, &(gd.layers.cont[i]));
      }
    }
  }

  /* render Winds if selected */
  if(gd.layers.wind_vectors) {
    render_vert_wind_vectors(xid);
  }

  // Render masking terrain
  if(gd.layers.earth.terrain_active) {
    render_v_terrain(xid);
  }

  render_xsect_top_layers(xid,page);

  // render_vert_products(xid);

  /* clear margin areas */
  XFillRectangle(gd.dpy,xid,gd.legends.background_color->gc,
                 0,0,gd.v_win.can_dim.width,gd.v_win.margin.top);

  XFillRectangle(gd.dpy,xid,gd.legends.background_color->gc,
                 0,gd.v_win.can_dim.height - gd.v_win.margin.bot,
                 gd.v_win.can_dim.width,gd.v_win.margin.bot);

  XFillRectangle(gd.dpy,xid,gd.legends.background_color->gc,
                 0,0,gd.v_win.margin.left,gd.v_win.can_dim.height);

  XFillRectangle(gd.dpy,xid,gd.legends.background_color->gc,
                 gd.v_win.can_dim.width - gd.v_win.margin.right,
                 0,gd.v_win.margin.right,gd.v_win.can_dim.height);


  draw_vwin_right_margin(xid,page);
  draw_vwin_top_margin(xid,page);
  draw_vwin_left_margin(xid,page);
  draw_vwin_bot_margin(xid,page);

  /* Add a border */
  x1 = gd.v_win.margin.left -1;
  y1 = gd.v_win.margin.top -1;
  wd = gd.v_win.img_dim.width +1;
  ht = gd.v_win.img_dim.height +1;
  /* Add a border around the plot */
  XDrawRectangle(gd.dpy,xid,gd.legends.foreground_color->gc,x1,y1,wd,ht);
 

  if(_params.show_data_messages) {
    gui_label_h_frame(gd.frame_label,-1);
  } else {
    set_busy_state(0); 
  }

  return CIDD_SUCCESS;
}

#endif
