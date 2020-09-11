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
#include <toolsa/LogStream.hh>

#include <QTimer>
#include <QBrush>
#include <QPalette>
#include <QPaintEngine>
#include <QPen>
#include <QResizeEvent>
#include <QStylePainter>

#include "PpiBeam.hh"
#include <Radx/Radx.hh>

using namespace std;

/////////////////////
// polar constructor

PpiBeam::PpiBeam(const Params &params,
                 const RadxRay *ray,
                 int n_fields,
                 double start_angle,
                 double stop_angle) :
        Beam(params, ray, n_fields),
        startAngle(start_angle),
        stopAngle(stop_angle),
        leftEnd(-1.0),
        rightEnd(-1.0),
        hidden(false)

{

  // Calculate the cosine and sine of the angles.  Divide each value by the
  // number of gates since we will be rendering the beams in a circle with a
  // radius of 1.0.  These values will represent the increments along the X
  // and Y axes for the vertices of the gates for this beam.

  double sin1, cos1;
  double sin2, cos2;
  
  Radx::sincos(start_angle * DEG_TO_RAD, sin1, cos1);
  Radx::sincos(stop_angle * DEG_TO_RAD, sin2, cos2);
  
  // Now calculate the vertex values to be used for all fields.  We negate
  // the y values because the display coordinate system has y increasing down.
  
  _polygons.resize(_nGates);

  double startRangeKm = _ray->getStartRangeKm();
  double gateSpacingKm = _ray->getGateSpacingKm();

  double innerRange = startRangeKm;
  double outerRange = innerRange + gateSpacingKm;

  for (size_t jj = 0; jj < _nGates;
       jj++, innerRange += gateSpacingKm, outerRange += gateSpacingKm) {
    if (innerRange < 0) {
      _polygons[jj].doPaint = false;
    } else {
      _polygons[jj].doPaint = true;
    }
    _polygons[jj].pts[0].x = innerRange * sin1;
    _polygons[jj].pts[0].y = innerRange * cos1;
    _polygons[jj].pts[1].x = innerRange * sin2;
    _polygons[jj].pts[1].y = innerRange * cos2;
    _polygons[jj].pts[2].x = outerRange * sin2;
    _polygons[jj].pts[2].y = outerRange * cos2;
    _polygons[jj].pts[3].x = outerRange * sin1;
    _polygons[jj].pts[3].y = outerRange * cos1;
  }

}

////////////////////////////////////////////////////////////////

PpiBeam::~PpiBeam()
{
  _polygons.clear();
}

////////////////////////////////////////////////////////////////
void PpiBeam::paint(QImage *image,
                    const QTransform &transform,
                    size_t field,
                    bool useHeight,
                    bool drawInstHt)
{
  // LOG(DEBUG) << "enter field = " << field;

  // TODO: fix HACK
  if ((field >= _nFields) || (field >= _brushes.size())) {
    LOG(DEBUG) << " aborting: field " << field << " is >= _nFields=" << _nFields
	       << " or >= _brushes.size()=" << _brushes.size();
    throw "number of fields NOT equal to number of brushes"; 
    //return;
  }
  QPainter painter(image);
  
  painter.setTransform(transform);
  painter.setPen(Qt::NoPen);
  
  QPolygonF polygon(4);

  if (_polygons.size() <= 0) return;

  polygon[0] = QPointF(_polygons[0].pts[0].x, _polygons[0].pts[0].y);
  polygon[1] = QPointF(_polygons[0].pts[1].x, _polygons[0].pts[1].y);
  polygon[2] = QPointF(_polygons[0].pts[2].x, _polygons[0].pts[2].y);
  polygon[3] = QPointF(_polygons[0].pts[3].x, _polygons[0].pts[3].y);
  
  const QBrush *prev_brush = _brushes[field][0];
  const QBrush *curr_brush = 0;
  
  for (size_t igate = 0; igate < _nGates; ++igate) {

    if (!_polygons[igate].doPaint) {
      continue;
    }

    curr_brush = _brushes[field][igate];
    /*
    if ((field == 1) && (igate < 10)) {
      QColor qcolor = curr_brush->color();
      int r,g,b,a;
      qcolor.getRgb(&r, &g, &b, &a);
      LOG(DEBUG) << "rgb = " << r << "," << g << "," << b;
    }
    */

    if (curr_brush != prev_brush) {

      polygon[2] = QPointF(_polygons[igate].pts[1].x, _polygons[igate].pts[1].y);
      polygon[3] = QPointF(_polygons[igate].pts[0].x, _polygons[igate].pts[0].y);
    
      painter.setBrush(*prev_brush);
      painter.drawPolygon(polygon);

      prev_brush = curr_brush;
      polygon[0] = QPointF(_polygons[igate].pts[0].x, _polygons[igate].pts[0].y);
      polygon[1] = QPointF(_polygons[igate].pts[1].x, _polygons[igate].pts[1].y);

    }
    
  } /* endfor - gate */

  // Draw the last polygon left in the queue

  if (curr_brush != 0 && _polygons[_nGates-1].doPaint) {
    polygon[2] = QPointF(_polygons[_nGates-1].pts[2].x,
			 _polygons[_nGates-1].pts[2].y);
    polygon[3] = QPointF(_polygons[_nGates-1].pts[3].x,
			 _polygons[_nGates-1].pts[3].y);
  
    painter.setBrush(*curr_brush);
    painter.drawPolygon(polygon);
  }

}

////////////////////////////////////////////////////////////////
void PpiBeam::print(ostream &out)

{
  out << "================= PpiBeam =================" << endl;
  RadxTime btime(_ray->getTimeSecs());
  out << "  time: " << btime.asString() << endl;
  out << "  elevation: " << _ray->getElevationDeg() << endl;
  out << "  azimuth: " << _ray->getAzimuthDeg() << endl;
  out << "  startAngle: " << startAngle << endl;
  out << "  stopAngle: " << stopAngle << endl;
  out << "  nFields: " << _nFields << endl;
  out << "=============================================" << endl;
}

