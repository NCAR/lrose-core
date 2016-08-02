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
#include <cmath>
#include <iostream>
#include <fstream>
#include <toolsa/toolsa_macros.h>
#include <Radx/RadxGeoref.hh>

#include <qtimer.h>
#include <QBrush>
#include <QPalette>
#include <QPaintEngine>
#include <QPen>
#include <QResizeEvent>
#include <QStylePainter>

#include "BscanBeam.hh"

using namespace std;

/////////////////////
// bscan constructor

BscanBeam::BscanBeam(const Params &params,
                     const RadxRay *ray,
                     double instHtKm,
                     int n_fields,
                     const RadxTime &plot_start_time,
                     double plot_width_secs,
                     const RadxTime &beam_start_time, 
                     const RadxTime &beam_end_time) :
        Beam(params, ray, n_fields),
        _instHtKm(instHtKm),
        _plotStartTime(plot_start_time),
        _plotWidthSecs(plot_width_secs),
        _beamStartTime(beam_start_time),
        _beamEndTime(beam_end_time)

{

  // Now calculate the vertex values to be used for all fields.  We negate
  // the y values because the display coordinate system has y increasing down.

  _rangeRects.resize(_nGates);
  _heightRects.resize(_nGates);

  // time

  double startX = _beamStartTime - _plotStartTime;
  double endX = _beamEndTime - _plotStartTime;
  
  // widen a bit to avoid missing pixels
  
  double dX = (endX - startX) * 1.1;

  // sanity check - if beam too wide do not render

  if ((fabs(dX) / fabs(_plotWidthSecs)) > 0.2) {
    dX = 0.0;
  }

  // height

  double yy = ray->getStartRangeKm();
  double dY = ray->getGateSpacingKm();

  double sinEl = sin(ray->getElevationDeg() * DEG_TO_RAD);
  double ht = _instHtKm + yy * sinEl;
  double dHt = dY * sinEl;

  // set rectangles
  
  _instRect.x = startX;
  _instRect.y = _instHtKm;
  _instRect.width = dX;
  _instRect.height = dY;

  for (size_t j = 0; j < _nGates; j++, yy += dY, ht += dHt) {

    _rangeRects[j].x = startX;
    _rangeRects[j].y = yy;
    _rangeRects[j].width = dX;
    _rangeRects[j].height = dY;

    _heightRects[j].x = startX;
    _heightRects[j].y = ht;
    _heightRects[j].width = dX;
    _heightRects[j].height = dHt;

  }

}

////////////////////////////////////////////////////////////////

BscanBeam::~BscanBeam()
{
  _rangeRects.clear();
  _heightRects.clear();
}

//////////////////////////////
// reset the plot start time

void BscanBeam::resetPlotStartTime(const RadxTime &plot_start_time)
{

  _plotStartTime = plot_start_time;

  // Now calculate the vertex values to be used for all fields.  We negate
  // the y values because the display coordinate system has y increasing down.

  double startX = _beamStartTime - _plotStartTime;
  double endX = _beamEndTime - _plotStartTime;

  // widen a bit to avoid missing pixels
  
  double dX = (endX - startX) * 1.1;
  
  // sanity check - if beam too wide do not render

  if ((fabs(dX) / fabs(_plotWidthSecs)) > 0.2) {
    dX = 0.0;
  }

  // set rectangles

  _instRect.x = startX;
  _instRect.width = dX;

  for (size_t j = 0; j < _nGates; j++) {
    _rangeRects[j].x = startX;
    _rangeRects[j].width = dX;
    _heightRects[j].x = startX;
    _heightRects[j].width = dX;
  }

}

/////////////////////////////////////////////////////////////////////////////

void BscanBeam::paint(QImage *image,
                      const QTransform &transform,
                      size_t field,
                      bool useHeight,
                      bool drawInstHt)
{

  QPainter painter(image);
  
  painter.setTransform(transform);
  painter.setPen(Qt::NoPen);

  if (useHeight) {
    _paintRects(painter, _heightRects, field);
  } else {
    _paintRects(painter, _rangeRects, field);
  }
  
  if (useHeight && drawInstHt) {
    QRectF rect(_instRect.x, 0.0,
                _instRect.width, _instRect.height);
    rect.setRect(_instRect.x, _instRect.y,
                 _instRect.width, _instRect.height);
    painter.setPen(_params.bscan_instrument_height_color);
    painter.drawRect(rect);
    QBrush brush(QColor(_params.bscan_instrument_height_color));
    painter.fillRect(rect, brush);
  }
}

/////////////////////////////////////////////////////////////////////////////

void BscanBeam::_paintRects(QPainter &painter,
                            const vector<rect_t> &rects,
                            size_t field)
{

  QRectF qrect;
  qrect.setRect(rects[0].x, rects[0].y, rects[0].width, rects[0].height);
  
  for (size_t igate = 0; igate < _nGates; ++igate) {
    
    // get the brush color for this gate
    
    const QBrush *brush = _brushes[field][igate];
    
    // how many gates of this color?
    
    double height = rects[igate].height;
    int nGatesExtra = 0;
    for (size_t jgate = igate + 1; jgate < _nGates; ++jgate) {
      if (brush == _brushes[field][jgate]) {
        nGatesExtra++;
        height += rects[jgate].height;
      } else {
        break;
      }
    }

    QRectF rect(rects[igate].x, rects[igate].y,
                rects[igate].width, height);
    
    painter.setBrush(*brush);
    painter.drawRect(rect);
    
    // skip over extra gates of same color
    
    igate += nGatesExtra;
    
  } /* endfor - gate */

}

////////////////////////////////////////////////////////////////
void BscanBeam::print(ostream &out)

{
  out << "================= BscanBeam =================" << endl;
  RadxTime btime(_ray->getTimeSecs());
  out << "  time: " << btime.asString() << endl;
  out << "  elevation: " << _ray->getElevationDeg() << endl;
  out << "  azimuth: " << _ray->getAzimuthDeg() << endl;
  out << "  _plotStartTime: " << _plotStartTime.asString() << endl;
  out << "  _plotWidthSecs: " << _plotWidthSecs << endl;
  out << "  _beamStartTime: " << _beamStartTime.asString() << endl;
  out << "  _beamEndTime: " << _beamEndTime.asString() << endl;
  out << "=============================================" << endl;
}

