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

#include "PolarManager.hh"
#include "RhiWidget.hh"
#include "RhiWindow.hh"
#include <toolsa/toolsa_macros.h>

using namespace std;

RhiWidget::RhiWidget(QWidget* parent,
                     const PolarManager &manager,
                     const RhiWindow &rhiWindow,
                     const Params &params,
                     const RadxPlatform &platform,
                     size_t n_fields) :
        PolarWidget(parent, manager, params, platform, n_fields),
        _rhiWindow(rhiWindow),
        _beamsProcessed(0)

{
  
  _locArray = NULL;
  _prevElev = -9999.0;

  if (_params.rhi_display_180_degrees) {
    _aspectRatio = _params.rhi_aspect_ratio * 2.0;
  } else {
    _aspectRatio = _params.rhi_aspect_ratio;
  }
  _colorScaleWidth = _params.rhi_color_scale_width;

  setGrids(_params.rhi_grids_on_at_startup);
  setRings(_params.rhi_range_rings_on_at_startup);
  setAngleLines(_params.rhi_elevation_lines_on_at_startup);

  // initialize world view

  _maxHeightKm = _params.rhi_max_height_km;
  _xGridSpacing = 0.0;
  _yGridSpacing = 0.0;
  configureRange(_params.max_range_km);

  // set up ray locators

  _locArray = new RayLoc[RayLoc::RAY_LOC_N];
  _rayLoc = _locArray + RayLoc::RAY_LOC_OFFSET;

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

RhiWidget::~RhiWidget()
{

  // delete all of the dynamically created beams

  for (size_t i = 0; i < _rhiBeams.size(); ++i) {
    delete _rhiBeams[i];
  }
  _rhiBeams.clear();

  if (_locArray) {
    delete[] _locArray;
  }

}


/*************************************************************************
 * addBeam()
 */

void RhiWidget::addBeam(const RadxRay *ray,
                        const std::vector< std::vector< double > > &beam_data,
                        const std::vector< DisplayField* > &fields)
{

  // compute the angle limits, and store the location of this ray

  _computeAngleLimits(ray);
  _storeRayLoc(ray);

  // Just add the beam to the beam list.

  RhiBeam* beam = new RhiBeam(_params, ray,
                              _manager.getPlatform().getAltitudeKm(),
                              _nFields, _startElev, _endElev);
  _rhiBeams.push_back(beam);

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
    
  beam->fillColors(beam_data, fields, &_backgroundBrush);

  // Add the new beams to the render lists for each of the fields
  
  for (size_t field = 0; field < _fieldRenderers.size(); ++field) {
    if (field == _selectedField ||
        _fieldRenderers[field]->isBackgroundRendered()) {
      _fieldRenderers[field]->addBeam(beam);
    } else {
      beam->setBeingRendered(field, false);
    }
  }
  
  // Start the threads to render the new beams
  
  _performRendering();

}

/*************************************************************************
 * configureRange()
 */

void RhiWidget::configureRange(double max_range)
{

  // Save the specified values

  _maxRangeKm = max_range;

  // Set the ring spacing.  This is dependent on the value of _maxRange.

  _setGridSpacing();
  
  // set world view

  int leftMargin = _params.rhi_left_margin;
  int rightMargin = _params.rhi_right_margin;
  int topMargin = _params.rhi_top_margin;
  int bottomMargin = _params.rhi_bottom_margin;
  int colorScaleWidth = _params.rhi_color_scale_width;
  int axisTickLen = _params.rhi_axis_tick_len;
  int nTicksIdeal = _params.rhi_n_ticks_ideal;
  int textMargin = _params.rhi_text_margin;

  if (_params.rhi_display_180_degrees) {
    _fullWorld.set(width(), height(),
                   leftMargin, rightMargin, topMargin, bottomMargin, colorScaleWidth,
                   -_maxRangeKm, 0.0,
                   _maxRangeKm, _maxHeightKm,
                   axisTickLen, nTicksIdeal, textMargin);
  } else {
    _fullWorld.set(width(), height(),
                   leftMargin, rightMargin, topMargin, bottomMargin, colorScaleWidth,
                   0.0, 0.0,
                   _maxRangeKm, _maxHeightKm,
                   axisTickLen, nTicksIdeal, textMargin);
  }

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

void RhiWidget::mouseReleaseEvent(QMouseEvent *e)
{

  _pointClicked = false;

  QRect rgeom = _rubberBand->geometry();

  // If the mouse hasn't moved much, assume we are clicking rather than
  // zooming

  QPointF clickPos(e->pos());
  
  _mouseReleaseX = clickPos.x();
  _mouseReleaseY = clickPos.y();

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

    _zoomWorld.set(_worldPressX, _worldPressY, _worldReleaseX, _worldReleaseY);

    _setTransform(_zoomWorld.getTransform());

    _setGridSpacing();

    // enable unzoom button

    _rhiWindow.enableZoomButton();
    
    // Update the window in the renderers
    
    _refreshImages();

  }
    
  // hide the rubber band

  _rubberBand->hide();
  update();

}

////////////////////////////////////////////////////////////////////////////
// get ray closest to click point

const RadxRay *RhiWidget::_getClosestRay(double xx, double yy)

{

  _beamHt.setInstrumentHtKm(_platform.getAltitudeKm());
  double clickEl = _beamHt.computeElevationDeg(yy, xx);
  
  double minDiff = 1.0e99;
  const RadxRay *closestRay = NULL;
  for (size_t ii = 0; ii < _rhiBeams.size(); ii++) {
    const RadxRay *ray = _rhiBeams[ii]->getRay();
    double rayEl = ray->getElevationDeg();
    double diff = fabs(clickEl - rayEl);
    if (diff > 180.0) {
      diff = fabs(diff - 360.0);
    }
    if (diff < minDiff) {
      closestRay = ray;
      minDiff = diff;
    }
  }

  return closestRay;

}

/*************************************************************************
 * _setGridSpacing()
 */

void RhiWidget::_setGridSpacing()
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

double RhiWidget::_getSpacing(double range)
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

void RhiWidget::_drawOverlays(QPainter &painter)
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
  font.setPointSizeF(_params.rhi_label_font_size);
  painter.setFont(font);
  
  _zoomWorld.setSpecifyTicks(true, xMin, _xGridSpacing);
  _zoomWorld.drawAxisTop(painter, "km", true, true, true);
  _zoomWorld.drawAxisBottom(painter, "km", true, true, true);
  
  _zoomWorld.setSpecifyTicks(true, yMin, _yGridSpacing);
  _zoomWorld.drawAxisLeft(painter, "km", true, true, true);
  _zoomWorld.drawAxisRight(painter, "km", true, true, true);
    
  _zoomWorld.setSpecifyTicks(false);

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

  const DisplayField &field = _manager.getSelectedField();
  _zoomWorld.drawColorScale(field.getColorMap(), painter);
  
  // add legends with time, field name and elevation angle

  if (_archiveMode) {
    
    vector<string> legends;
    char text[1024];
    
    // time legend

    sprintf(text, "Start time: %s", _plotStartTime.asString(3).c_str());
    legends.push_back(text);
    
    // radar and site name legend

    string radarName(_platform.getInstrumentName());
    if (_params.override_radar_name) {
      radarName = _params.radar_name;
    }
    string siteName(_platform.getInstrumentName());
    if (_params.override_site_name) {
      siteName = _params.site_name;
    }
    string radarSiteLabel = radarName;
    if (siteName.size() > 0) {
      radarSiteLabel += "/";
      radarSiteLabel += siteName;
    }
    legends.push_back(radarSiteLabel);

    // field name legend

    string fieldName =
      _fieldRenderers[_selectedField]->getParams().label;
    sprintf(text, "Field: %s", fieldName.c_str());
    legends.push_back(text);
    
    // azimuth legend

    sprintf(text, "Azimuth(deg): %.2f", _meanAz);
    legends.push_back(text);

    // nrays legend

    sprintf(text, "NRays: %g", _nRays);
    legends.push_back(text);
    
    painter.save();
    painter.setPen(Qt::yellow);
    painter.setBrush(Qt::black);
    painter.setBackgroundMode(Qt::OpaqueMode);

    switch (_params.ppi_main_legend_pos) {
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

void RhiWidget::_computeAngleLimits(const RadxRay *ray)
  
{
  
  double beamWidth = _platform.getRadarBeamWidthDegV();
  double elev = ray->getElevationDeg();

  // Determine the extent of this ray
  
  double elevDiff = Radx::computeAngleDiff(elev, _prevElev);
  if (ray->getIsIndexed() || fabs(elevDiff) > beamWidth * 2.0) {

    double halfAngle = ray->getAngleResDeg() / 2.0;
    _startElev = elev - halfAngle;
    _endElev = elev + halfAngle;

  } else {

    double maxHalfAngle = beamWidth / 2.0;
    double prevOffset = maxHalfAngle;
      
    double halfElevDiff = elevDiff / 2.0;
	
    if (prevOffset > halfElevDiff) {
	prevOffset = halfElevDiff;
    }
      
    _startElev = elev - prevOffset;
    _endElev = elev + maxHalfAngle;

  }

  _prevElev = elev;
    
}

///////////////////////////////////////////////////////////
// store ray location

void RhiWidget::_storeRayLoc(const RadxRay *ray)
{

  int startIndex = (int) (_startElev * RayLoc::RAY_LOC_RES);
  int endIndex = (int) (_endElev * RayLoc::RAY_LOC_RES + 1);

  // Clear out any rays in the locations list that are overlapped by the
  // new ray
    
  _clearRayOverlap(startIndex, endIndex);
  
  // Set the locations associated with this ray

  for (int ii = startIndex; ii <= endIndex; ii++) {
    _rayLoc[ii].ray = ray;
    _rayLoc[ii].active = true;
    _rayLoc[ii].master = false;
    _rayLoc[ii].startIndex = startIndex;
    _rayLoc[ii].endIndex = endIndex;
  }

  // indicate which ray is the master
  // i.e. it is responsible for ray memory
    
  int midIndex = (int) (ray->getElevationDeg() * RayLoc::RAY_LOC_RES);
  _rayLoc[midIndex].master = true;
  ray->addClient();

}

///////////////////////////////////////////////////////////
// clear any locations that are overlapped by the given ray

void RhiWidget::_clearRayOverlap(const int startIndex,
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
	// If the master is in the overlap area, then it needs to be moved
	// outside of this area

	if (_rayLoc[j].master)
	  _rayLoc[startIndex-1].master = true;
	
	_rayLoc[j].ray = NULL;
	_rayLoc[j].active = false;
	_rayLoc[j].master = false;
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
	// If the master is in the overlap area, then it needs to be moved
	// outside of this area

	if (_rayLoc[j].master)
	  _rayLoc[endIndex+1].master = true;
	
	_rayLoc[j].ray = NULL;
	_rayLoc[j].active = false;
	_rayLoc[j].master = false;
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

/*************************************************************************
 * _refreshImages()
 */

void RhiWidget::_refreshImages()
{

  for (size_t ifield = 0; ifield < _fieldRenderers.size(); ++ifield) {
    
    FieldRenderer *field = _fieldRenderers[ifield];
    
    // If needed, create new image for this field
    
    if (size() != field->getImage()->size()) {
      field->createImage(width(), height());
    }

    // clear image

    field->getImage()->fill(_backgroundBrush.color().rgb());
    
    // set up rendering details

    field->setTransform(_zoomTransform);
    
    // Add pointers to the beams to be rendered
    
    if (ifield == _selectedField || field->isBackgroundRendered()) {

      std::vector< RhiBeam* >::iterator beam;
      for (beam = _rhiBeams.begin(); beam != _rhiBeams.end(); ++beam) {
	(*beam)->setBeingRendered(ifield, true);
	field->addBeam(*beam);
      }
      
    }
    
  } // ifield
  
  // do the rendering

  _performRendering();

  update();
}


/*************************************************************************
 * clear()
 */

void RhiWidget::clear()
{

  // Clear out the beam array
  
  for (size_t i = 0; i < _rhiBeams.size(); i++) {
    delete _rhiBeams[i];
  }
  _rhiBeams.clear();
  _pointClicked = false;
  
  // Now rerender the images
  
  _refreshImages();
  
}


/*************************************************************************
 * refresh()
 */

void RhiWidget::refresh()
{
  _refreshImages();
}

/*************************************************************************
 * unzoom the view
 */

void RhiWidget::unzoomView()
{
  
  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _setGridSpacing();
  _refreshImages();
  // _updateRenderers();

}

/*************************************************************************
 * resize()
 */

void RhiWidget::resize(int width, int height)
{
  
  setGeometry(0, 0, width, height);
  _resetWorld(width, height);
  _refreshImages();

}

/*************************************************************************
 * paintEvent()
 */

void RhiWidget::paintEvent(QPaintEvent *event)
{

  QPainter painter(this);
  painter.save();
  painter.eraseRect(0, 0, width(), height());
  _zoomWorld.setClippingOn(painter);
  painter.drawImage(0, 0, *(_fieldRenderers[_selectedField]->getImage()));
  painter.restore();
  _drawOverlays(painter);

}

/*************************************************************************
 * selectVar()
 */

void RhiWidget::selectVar(const size_t index)
{

  // If the field index isn't actually changing, we don't need to do anything
  
  if (_selectedField == index) {
    return;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========>> selectVar for field: "
         << _params._fields[index].label << endl;
  }

  // If this field isn't being rendered in the background, render all of
  // the beams for it

  if (!_fieldRenderers[index]->isBackgroundRendered()) {
    std::vector<RhiBeam*>::iterator beam;
    for (beam = _rhiBeams.begin(); beam != _rhiBeams.end(); ++beam) {
      (*beam)->setBeingRendered(index, true);
      _fieldRenderers[index]->addBeam(*beam);
    }
  }
  _performRendering();

  // Do any needed housekeeping when the field selection is changed

  _fieldRenderers[_selectedField]->unselectField();
  _fieldRenderers[index]->selectField();
  
  // Change the selected field index

  _selectedField = index;

  // Update the display

  update();
}



