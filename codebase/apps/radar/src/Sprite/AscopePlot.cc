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
/////////////////////////////////////////////////////////////
// AscopePlot.cc
//
// Plotting for power vs range in an ascope
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////

#include <assert.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/pjg.h>

#include <QTimer>
#include <QBrush>
#include <QPalette>
#include <QPaintEngine>
#include <QPen>
#include <QResizeEvent>
#include <QStylePainter>

#include "AscopePlot.hh"
#include "SpectraMgr.hh"
#include "Beam.hh"

using namespace std;

AscopePlot::AscopePlot(QWidget* parent,
                       const Params &params,
                       int id) :
        _parent(parent),
        _params(params),
        _id(id)
        
{
  _isZoomed = false;
  _xGridLinesOn = _params.ascope_x_grid_lines_on;
  _yGridLinesOn = _params.ascope_y_grid_lines_on;
}

/*************************************************************************
 * Destructor
 */

AscopePlot::~AscopePlot()
{

}


/*************************************************************************
 * clear()
 */

void AscopePlot::clear()
{

}

/*************************************************************************
 * perform zoom
 */

void AscopePlot::zoom(int x1, int y1, int x2, int y2)
{

  _zoomWorld.setZoomLimits(x1, y1, x2, y2);
  _isZoomed = true;

}

/*************************************************************************
 * unzoom the view
 */

void AscopePlot::unzoom()
{

  _zoomWorld = _fullWorld;
  _isZoomed = false;

}

/*************************************************************************
 * plot a beam
 */

void AscopePlot::plotBeam(QPainter &painter,
                          Beam *beam,
                          double selectedRangeKm)
  
{

  if (beam == NULL) {
    cerr << "WARNING - AscopePlot::plotBeam() - got NULL beam, ignoring"
         << endl;
    return;
  }
  
  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "======== Ascope - plotting beam data ================" << endl;
    DateTime beamTime(beam->getTimeSecs(), true, beam->getNanoSecs() * 1.0e-9);
    cerr << "  Beam time: " << beamTime.asString(3) << endl;
    cerr << "  Max range: " << beam->getMaxRange() << endl;
  }

  const MomentsFields* fields = beam->getOutFields();
  int nGates = beam->getNGates();
  double startRange = beam->getStartRangeKm();
  double gateSpacing = beam->getGateSpacingKm();
  if (_params.set_max_range) {
    int nGatesMax = (_params.max_range_km - startRange) / gateSpacing;
    if (nGatesMax < nGates) {
      nGates = nGatesMax;
    }
  }

  // first use filled polygons (trapezia)
  
  double xMin = _zoomWorld.getXMinWorld();
  QBrush brush(_params.ascope_fill_color);
  brush.setStyle(Qt::SolidPattern);
  
  for (int ii = 1; ii < nGates; ii++) {
    double rangePrev = startRange + gateSpacing * (ii-1);
    double range = startRange + gateSpacing * (ii);
    double valPrev = getFieldVal(_momentType, fields[ii-1]);
    double val = getFieldVal(_momentType, fields[ii]);
    if (val > -9990 && valPrev > -9990) {
      _zoomWorld.fillTrap(painter, brush,
                          xMin, rangePrev,
                          valPrev, rangePrev,
                          val, range,
                          xMin, range);
    }
  }

  // draw the reflectivity field vs range - as line

  painter.save();
  painter.setPen(_params.ascope_line_color);
  QVector<QPointF> pts;
  for (int ii = 0; ii < nGates; ii++) {
    double range = startRange + gateSpacing * ii;
    double val = getFieldVal(_momentType, fields[ii]);
    if (val > -9990) {
      QPointF pt(val, range);
      pts.push_back(pt);
    }
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();

  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the title

  painter.save();
  painter.setPen(_params.ascope_title_color);
  string title("Ascope:");
  title.append(getName(_momentType));
  _zoomWorld.drawTitleTopCenter(painter, title);
  painter.restore();

}

//////////////////////////////////
// get a string for the field name

string AscopePlot::getName(Params::moment_type_t mtype)
{
  switch (mtype) {
    case Params::DBZ:
      return "DBZ";
    case Params::VEL:
      return "VEL";
    case Params::WIDTH:
      return "WIDTH";
    case Params::NCP:
      return "NCP";
    case Params::SNR:
      return "SNR";
    case Params::DBM:
      return "DBM";
    case Params::ZDR:
      return "ZDR";
    case Params::LDR:
      return "LDR";
    case Params::RHOHV:
      return "RHOHV";
    case Params::PHIDP:
      return "PHIDP";
    case Params::KDP:
      return "KDP";
    default:
      return "UNKNOWN";
  }
}

//////////////////////////////////
// get a string for the field units

string AscopePlot::getUnits(Params::moment_type_t mtype)
{
  switch (mtype) {
    case Params::DBZ:
      return "dBZ";
    case Params::VEL:
      return "m/s";
    case Params::WIDTH:
      return "m/s";
    case Params::NCP:
      return "";
    case Params::SNR:
      return "dB";
    case Params::DBM:
      return "dBm";
    case Params::ZDR:
      return "dB";
    case Params::LDR:
      return "dB";
    case Params::RHOHV:
      return "";
    case Params::PHIDP:
      return "deg";
    case Params::KDP:
      return "deg/km";
    default:
      return "";
  }
}

////////////////////////////////////////////
// get a field value based on the field type

double AscopePlot::getFieldVal(Params::moment_type_t mtype,
                               const MomentsFields &fields)
{
  switch (mtype) {
    case Params::DBZ:
      return fields.dbz;
    case Params::VEL:
      return fields.vel;
    case Params::WIDTH:
      return fields.width;
    case Params::NCP:
      return fields.ncp;
    case Params::SNR:
      return fields.snr;
    case Params::DBM:
      return fields.dbm;
    case Params::ZDR:
      return fields.zdr;
    case Params::LDR:
      return fields.ldr;
    case Params::RHOHV:
      return fields.rhohv;
    case Params::PHIDP:
      return fields.phidp;
    case Params::KDP:
      return fields.kdp;
    default:
      return -9999.0;
  }
}

////////////////////////////////////////////
// get min val for plotting

double AscopePlot::getMinVal(Params::moment_type_t mtype)
{
  switch (mtype) {
    case Params::DBZ:
      return -30;
    case Params::VEL:
      return -40;
    case Params::WIDTH:
      return 0;
    case Params::NCP:
      return 0;
    case Params::SNR:
      return -20;
    case Params::DBM:
      return -120;
    case Params::ZDR:
      return -4;
    case Params::LDR:
      return -40;
    case Params::RHOHV:
      return 0;
    case Params::PHIDP:
      return -180;
    case Params::KDP:
      return -2;
    default:
      return 0;
  }
}

////////////////////////////////////////////
// get max val for plotting

double AscopePlot::getMaxVal(Params::moment_type_t mtype)
{
  switch (mtype) {
    case Params::DBZ:
      return 80;
    case Params::VEL:
      return 40;
    case Params::WIDTH:
      return 20;
    case Params::NCP:
      return 1;
    case Params::SNR:
      return 80;
    case Params::DBM:
      return 10;
    case Params::ZDR:
      return 16;
    case Params::LDR:
      return 15;
    case Params::RHOHV:
      return 1;
    case Params::PHIDP:
      return 180;
    case Params::KDP:
      return 8;
    default:
      return 10;
  }
}

/*************************************************************************
 * set the geometry - unzooms
 */

void AscopePlot::setWindowGeom(int width, int height,
                               int xOffset, int yOffset)
{
  _fullWorld.setWindowGeom(width, height, xOffset, yOffset);
  _zoomWorld = _fullWorld;
}

/*************************************************************************
 * set the world limits - unzooms
 */

void AscopePlot::setWorldLimits(double xMinWorld,
                                double yMinWorld,
                                double xMaxWorld,
                                double yMaxWorld)
{
  _fullWorld.setWorldLimits(xMinWorld, yMinWorld,
                            xMaxWorld, yMaxWorld);
  _zoomWorld = _fullWorld;
}

/*************************************************************************
 * set the zoom limits, from pixel space
 */

void AscopePlot::setZoomLimits(int xMin,
                               int yMin,
                               int xMax,
                               int yMax)
{
  _zoomWorld.setZoomLimits(xMin, yMin, xMax, yMax);
  _isZoomed = true;
}

void AscopePlot::setZoomLimitsX(int xMin,
                                int xMax)
{
  _zoomWorld.setZoomLimitsX(xMin, xMax);
  _isZoomed = true;
}

void AscopePlot::setZoomLimitsY(int yMin,
                                int yMax)
{
  _zoomWorld.setZoomLimitsY(yMin, yMax);
  _isZoomed = true;
}

/*************************************************************************
 * Protected methods
 *************************************************************************/

/*************************************************************************
 * Draw the overlays, axes, legends etc
 */

void AscopePlot::_drawOverlays(QPainter &painter, double selectedRangeKm)
{

  // save painter state
  
  painter.save();
  
  // store font
  
  QFont origFont = painter.font();
  
  painter.setPen(_params.ascope_axis_label_color);

  _zoomWorld.drawAxisBottom(painter, getUnits(_momentType),
                            true, true, true, _xGridLinesOn);

  _zoomWorld.drawAxisLeft(painter, "km", 
                          true, true, true, _yGridLinesOn);

  _zoomWorld.drawYAxisLabelLeft(painter, "Range");

  // selected range line
  
  QPen pen(painter.pen());
  pen.setColor(_params.ascope_selected_range_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(2);
  painter.setPen(pen);
  _zoomWorld.drawLine(painter,
                      _zoomWorld.getXMinWorld(), selectedRangeKm,
                      _zoomWorld.getXMaxWorld(), selectedRangeKm);

  painter.restore();

}

