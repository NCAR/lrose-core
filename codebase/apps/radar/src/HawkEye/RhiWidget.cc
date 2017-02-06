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

#include "RhiWidget.hh"
#include "PolarManager.hh"
#include <toolsa/toolsa_macros.h>

using namespace std;


RhiWidget::RhiWidget(QWidget* parent,
                     const PolarManager &manager,
                     const Params &params,
                     size_t n_fields) :
        PolarWidget(parent, manager, params, n_fields),
        _beamsProcessed(0)
{

  if (_params.rhi_display_180_degrees) {
    _aspectRatio = _params.rhi_aspect_ratio * 2.0;
  } else {
    _aspectRatio = _params.rhi_aspect_ratio;
  }
  _colorScaleWidth = _params.rhi_color_scale_width;

  // initialize world view

  _maxHeightKm = _params.rhi_max_height_km;
  _xGridSpacing = 0.0;
  _yGridSpacing = 0.0;
  configureRange(_params.max_range_km);

}


/*************************************************************************
 * Destructor
 */

RhiWidget::~RhiWidget()
{
}


/*************************************************************************
 * addBeam()
 */

void RhiWidget::addBeam(const RadxRay *ray,
                        const float start_angle,
                        const float stop_angle,
                        const std::vector< std::vector< double > > &beam_data,
                        const std::vector< DisplayField* > &fields)
{
  // add a new beam to the display. 
  // The steps are:
  // 1. preallocate mode: find the beam to be drawn, or dynamic mode:
  //    create the beam(s) to be drawn.
  // 2. fill the colors for all variables in the beams to be drawn
  // 3. make the display list for the selected variables in the beams
  //    to be drawn.
  // 4. call the new display list(s)

  std::vector< PolarBeam* > newBeams;

  // The start and stop angle MUST specify a clockwise fill for the sector.
  // Thus if start_angle > stop_angle, we know that we have crossed the 0
  // boundary, and must break it up into 2 beams.

  // Create the new beam(s), to keep track of the display information.
  // Beam start and stop angles are adjusted here so that they always 
  // increase clockwise. Likewise, if a beam crosses the 0 degree boundary,
  // it is split into two beams, each of them again obeying the clockwise
  // rule. Prescribing these rules makes the beam culling logic a lot simpler.

  // Normalize the start and stop angles.  I'm not convinced that this works
  // for negative angles, but leave it for now.

  float n_start_angle = start_angle - ((int)(start_angle/360.0))*360.0;
  float n_stop_angle = stop_angle - ((int)(stop_angle/360.0))*360.0;
  
  if (n_start_angle <= n_stop_angle) {

    // This beam does not cross the 0 degree angle.  Just add the beam to
    // the beam list.

    PolarBeam* b = new PolarBeam(_params, ray, _nFields, n_start_angle, n_stop_angle);
    _cullBeams(b);
    _beams.push_back(b);
    newBeams.push_back(b);

  } else {

    // The beam crosses the 0 degree angle.  First add the portion of the
    // beam to the left of the 0 degree point.

    PolarBeam* b1 = new PolarBeam(_params, ray, _nFields, n_start_angle, 360.0);
    _cullBeams(b1);
    _beams.push_back(b1);
    newBeams.push_back(b1);

    // Now add the portion of the beam to the right of the 0 degree point.

    PolarBeam* b2 = new PolarBeam(_params, ray, _nFields, 0.0, n_stop_angle);
    _cullBeams(b2);
    _beams.push_back(b2);
    newBeams.push_back(b2);

  }

  // newBeams has pointers to all of the newly added beams.  Render the
  // beam data.

  for (size_t ii = 0; ii < newBeams.size(); ii++) {

    PolarBeam *beam = newBeams[ii];
    
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
    
  } /* endfor - beam */
  
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
    // _setWindow(QRect(-Beam::RENDER_PIXELS, -Beam::RENDER_PIXELS,
    //     	     Beam::RENDER_PIXELS * 2, Beam::RENDER_PIXELS));
  } else {
    _fullWorld.set(width(), height(),
                   leftMargin, rightMargin, topMargin, bottomMargin, colorScaleWidth,
                   0.0, 0.0,
                   _maxRangeKm, _maxHeightKm,
                   axisTickLen, nTicksIdeal, textMargin);
    // _setWindow(QRect(0, -Beam::RENDER_PIXELS,
    //     	     Beam::RENDER_PIXELS, Beam::RENDER_PIXELS));
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

////////////////////////////////////////////////////////////////////////////
// get ray closest to click point

const RadxRay *RhiWidget::_getClosestRay(double x_km, double y_km)

{

  double clickEl = atan2(y_km, x_km) * DEG_TO_RAD;
  
  double minDiff = 1.0e99;
  const RadxRay *closestRay = NULL;
  for (size_t ii = 0; ii < _beams.size(); ii++) {
    const RadxRay *ray = _beams[ii]->getRay();
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
 * _setGridSpacing()
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

  if (_azLinesEnabled) {

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
  
}
