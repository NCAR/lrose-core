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
#include "PpiWidget.hh"
#include "PolarManager.hh"
#include <toolsa/toolsa_macros.h>

using namespace std;



PpiWidget::PpiWidget(QWidget* parent,
                     const PolarManager &manager,
                     const Params &params,
                     const RadxPlatform &platform,
                     size_t n_fields) :
        PolarWidget(parent, manager, params, platform, n_fields)
        
{

  _aspectRatio = _params.ppi_aspect_ratio;
  _colorScaleWidth = _params.color_scale_width;

  // initialoze world view

  configureRange(_params.max_range_km);

  setGrids(_params.ppi_grids_on_at_startup);
  setRings(_params.ppi_range_rings_on_at_startup);
  setAngleLines(_params.ppi_azimuth_lines_on_at_startup);

}

/*************************************************************************
 * Destructor
 */

PpiWidget::~PpiWidget()
{

}


/*************************************************************************
 * addBeam()
 */

void PpiWidget::addBeam(const RadxRay *ray,
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

void PpiWidget::configureRange(double max_range)
{

  // Save the specified values

  _maxRangeKm = max_range;

  // Set the ring spacing.  This is dependent on the value of _maxRange.

  _setGridSpacing();
  
  // set world view

  int leftMargin = 0;
  int rightMargin = 0;
  int topMargin = 0;
  int bottomMargin = 0;
  int colorScaleWidth = _params.color_scale_width;
  int axisTickLen = 7;
  int nTicksIdeal = 7;
  int textMargin = 5;

  if (_params.ppi_display_type == Params::PPI_AIRBORNE) {

    _fullWorld.set(width(), height(),
                   leftMargin, rightMargin, topMargin, bottomMargin, colorScaleWidth,
                   -_maxRangeKm, 0.0,
                   _maxRangeKm, _maxRangeKm,
                   axisTickLen, nTicksIdeal, textMargin);
    // _setWindow(QRect(-Beam::RENDER_PIXELS, -Beam::RENDER_PIXELS,
    //                  Beam::RENDER_PIXELS * 2, Beam::RENDER_PIXELS * 2));
    
  } else {
    
    _fullWorld.set(width(), height(),
                   leftMargin, rightMargin, topMargin, bottomMargin, colorScaleWidth,
                   -_maxRangeKm, -_maxRangeKm,
                   _maxRangeKm, _maxRangeKm,
                   axisTickLen, nTicksIdeal, textMargin);
    // _setWindow(QRect(-Beam::RENDER_PIXELS, -Beam::RENDER_PIXELS,
    //                  Beam::RENDER_PIXELS * 2, Beam::RENDER_PIXELS));

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

const RadxRay *PpiWidget::_getClosestRay(double x_km, double y_km)

{

  double clickAz = atan2(x_km, y_km) * DEG_TO_RAD;

  double minDiff = 1.0e99;
  const RadxRay *closestRay = NULL;
  for (size_t ii = 0; ii < _beams.size(); ii++) {
    const RadxRay *ray = _beams[ii]->getRay();
    double rayAz = ray->getAzimuthDeg();
    double diff = fabs(clickAz - rayAz);
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

void PpiWidget::_setGridSpacing()
{

  double xRange = _zoomWorld.getXMaxWorld() - _zoomWorld.getXMinWorld();
  double yRange = _zoomWorld.getYMaxWorld() - _zoomWorld.getYMinWorld();
  double diagonal = sqrt(xRange * xRange + yRange * yRange);

  if (diagonal <= 1.0) {
    _ringSpacing = 0.05;
  } else if (diagonal <= 2.0) {
    _ringSpacing = 0.1;
  } else if (diagonal <= 5.0) {
    _ringSpacing = 0.2;
  } else if (diagonal <= 10.0) {
    _ringSpacing = 0.5;
  } else if (diagonal <= 20.0) {
    _ringSpacing = 1.0;
  } else if (diagonal <= 50.0) {
    _ringSpacing = 2.0;
  } else if (diagonal <= 100.0) {
    _ringSpacing = 5.0;
  } else if (diagonal <= 200.0) {
    _ringSpacing = 10.0;
  } else {
    _ringSpacing = 20.0;
  }

}


/*************************************************************************
 * _drawOverlays()
 */

void PpiWidget::_drawOverlays(QPainter &painter)
{

  // Don't try to draw rings if we haven't been configured yet or if the
  // rings or grids aren't enabled.
  
  if (!_ringsEnabled && !_gridsEnabled && !_angleLinesEnabled) {
    return;
  }

  // save painter state

  painter.save();
  
  // store font
  
  QFont origFont = painter.font();
  
  // Set the painter to use the right color and font

  // painter.setWindow(_zoomWindow);
  
  painter.setPen(_gridRingsColor);

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
  
  // Draw the grid

  if (_ringSpacing > 0.0 && _gridsEnabled)  {

    double ringRange = _ringSpacing;
    double maxRingRange = ringRange;
    while (ringRange <= _maxRangeKm) {

      _zoomWorld.drawLine(painter, ringRange, -_maxRangeKm, ringRange, _maxRangeKm);
      _zoomWorld.drawLine(painter, -ringRange, -_maxRangeKm, -ringRange, _maxRangeKm);
      _zoomWorld.drawLine(painter, -_maxRangeKm, ringRange, _maxRangeKm, ringRange);
      _zoomWorld.drawLine(painter, -_maxRangeKm, -ringRange, _maxRangeKm, -ringRange);
      
      maxRingRange = ringRange;
      ringRange += _ringSpacing;
    }

    _zoomWorld.setSpecifyTicks(true, -maxRingRange, _ringSpacing);

    _zoomWorld.drawAxisLeft(painter, "km", true, true, true);
    _zoomWorld.drawAxisRight(painter, "km", true, true, true);
    _zoomWorld.drawAxisTop(painter, "km", true, true, true);
    _zoomWorld.drawAxisBottom(painter, "km", true, true, true);
    
    _zoomWorld.setSpecifyTicks(false);

  }
  
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
  
}
