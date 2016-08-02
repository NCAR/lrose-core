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

  // initialoze world view
  
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
  // After processing 10 beams, send a signal to resize the widget.  This
  // gets rid of the problem with the widget frame being cut off at the
  // bottom until the user explicitly resizes the window.  I know this is
  // screwy, but I couldn't find any other way that worked.  Note that I
  // tried setting this to 3 instead of 10, and it seemed to be too soon to
  // work.  If you can find the right way to get the widgets to size correctly,
  // please change this!

  if (_beamsProcessed == 10) {
    emit severalBeamsProcessed();
  }
  
  PolarWidget::addBeam(ray, start_angle, stop_angle, beam_data, fields);

  _beamsProcessed++;

}

/*************************************************************************
 * configureRange()
 */

void RhiWidget::configureRange(double max_range)
{

  // Save the specified values

  _maxRange = max_range;

  // Set the ring spacing.  This is dependent on the value of _maxRange.

  _setRingSpacing();
  
  // set world view

  int leftMargin = 0;
  int rightMargin = 0;
  int topMargin = 0;
  int bottomMargin = 0;
  int colorScaleWidth = 0;
  int axisTickLen = 7;
  int nTicksIdeal = 7;
  int textMargin = 5;

  if (_params.rhi_display_180_degrees) {
    _fullWorld.set(width(), height(),
                   leftMargin, rightMargin, topMargin, bottomMargin, colorScaleWidth,
                   -_maxRange, 0.0,
                   _maxRange, _maxRange,
                   axisTickLen, nTicksIdeal, textMargin);
    // _setWindow(QRect(-Beam::RENDER_PIXELS, -Beam::RENDER_PIXELS,
    //     	     Beam::RENDER_PIXELS * 2, Beam::RENDER_PIXELS));
  } else {
    _fullWorld.set(width(), height(),
                   leftMargin, rightMargin, topMargin, bottomMargin, colorScaleWidth,
                   0.0, 0.0,
                   _maxRange, _maxRange,
                   axisTickLen, nTicksIdeal, textMargin);
    // _setWindow(QRect(0, -Beam::RENDER_PIXELS,
    //     	     Beam::RENDER_PIXELS, Beam::RENDER_PIXELS));
  }

  _zoomWorld = _fullWorld;
  _isZoomed = false;
  _setTransform(_zoomWorld.getTransform());
  _setRingSpacing();

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

