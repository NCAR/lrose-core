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
// IqPlot.cc
//
// Plotting of IQ data, as time series and spectra
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
#include <radar/GateData.hh>
#include <radar/RadarFft.hh>
#include <radar/RegressionFilter.hh>
#include <radar/ClutFilter.hh>

#include <QTimer>
#include <QBrush>
#include <QPalette>
#include <QPaintEngine>
#include <QPen>
#include <QResizeEvent>
#include <QStylePainter>

#include "IqPlot.hh"
#include "SpectraMgr.hh"
#include "Beam.hh"

using namespace std;

IqPlot::IqPlot(QWidget* parent,
               const Params &params,
               int id) :
        _parent(parent),
        _params(params),
        _id(id)
        
{
  _isZoomed = false;
  _xGridLinesOn = _params.iqplot_x_grid_lines_on;
  _yGridLinesOn = _params.iqplot_y_grid_lines_on;
}

/*************************************************************************
 * Destructor
 */

IqPlot::~IqPlot()
{

}


/*************************************************************************
 * clear()
 */

void IqPlot::clear()
{

}

/*************************************************************************
 * perform zoom
 */

void IqPlot::zoom(int x1, int y1, int x2, int y2)
{

  _zoomWorld.setZoomLimits(x1, y1, x2, y2);
  _isZoomed = true;

}

/*************************************************************************
 * unzoom the view
 */

void IqPlot::unzoom()
{

  _zoomWorld = _fullWorld;
  _isZoomed = false;

}

/*************************************************************************
 * plot a beam
 */

void IqPlot::plotBeam(QPainter &painter,
                      Beam *beam,
                      double selectedRangeKm)
  
{

  if (beam == NULL) {
    cerr << "WARNING - IqPlot::plotBeam() - got NULL beam, ignoring"
         << endl;
    return;
  }
  
  int gateNum = beam->getGateNum(selectedRangeKm);

  if(_params.debug) {
    cerr << "======== Iqplot - plotting beam data ================" << endl;
    DateTime beamTime(beam->getTimeSecs(), true, beam->getNanoSecs() * 1.0e-9);
    cerr << "  Beam time: " << beamTime.asString(3) << endl;
    cerr << "  selected range: " << selectedRangeKm << endl;
    cerr << "  gate num: " << gateNum << endl;
  }

  QBrush brush(_params.iqplot_fill_color);
  brush.setStyle(Qt::SolidPattern);
  
  // get data for this gate

  const GateData &gateData = *beam->getGateData()[gateNum];
  
  // create window for the iq data, for FFT operations

  int nSamples = beam->getNSamples();
  TaArray<double> windowCoeff_;
  double *windowCoeff = windowCoeff_.alloc(nSamples);
  switch (_params.window) {
    case Params::WINDOW_RECT:
    default:
      RadarMoments::initWindowRect(nSamples, windowCoeff);
      break;
    case Params::WINDOW_VONHANN:
      RadarMoments::initWindowVonhann(nSamples, windowCoeff);
      break;
    case Params::WINDOW_BLACKMAN:
      RadarMoments::initWindowBlackman(nSamples, windowCoeff);
      break;
    case Params::WINDOW_BLACKMAN_NUTTALL:
      RadarMoments::initWindowBlackmanNuttall(nSamples, windowCoeff);
      break;
    case Params::WINDOW_TUKEY_10:
      RadarMoments::initWindowTukey(0.1, nSamples, windowCoeff);
      break;
    case Params::WINDOW_TUKEY_20:
      RadarMoments::initWindowTukey(0.2, nSamples, windowCoeff);
      break;
    case Params::WINDOW_TUKEY_30:
      RadarMoments::initWindowTukey(0.3, nSamples, windowCoeff);
      break;
    case Params::WINDOW_TUKEY_50:
      RadarMoments::initWindowTukey(0.5, nSamples, windowCoeff);
      break;
  }

  // compute power spectrum
  
  TaArray<RadarComplex_t> iqWindowed_;
  RadarComplex_t *iqWindowed = iqWindowed_.alloc(nSamples);
  RadarMoments::applyWindow(gateData.iqhcOrig, windowCoeff, iqWindowed, nSamples);
  
  TaArray<RadarComplex_t> powerSpec_;
  RadarComplex_t *powerSpec = powerSpec_.alloc(nSamples);
  RadarFft fft(nSamples);
  fft.fwd(iqWindowed, powerSpec);
  fft.shift(powerSpec);
  
  // compute power

  TaArray<double> powerDbm_;
  double *powerDbm = powerDbm_.alloc(nSamples);
  for (int ii = 0; ii < nSamples; ii++) {
    double power = RadarComplex::power(powerSpec[ii]);
    if (power <= 0) {
      powerDbm[ii] = -120.0;
    } else {
      powerDbm[ii] = 10.0 * log10(power);
    }
  }
  
  double yMin = _zoomWorld.getYMinWorld();
  for (int ii = 1; ii < nSamples; ii++) {
    int jj = (ii + nSamples / 2) % nSamples;
    double valPrev = powerDbm[jj-1];
    double val = powerDbm[jj];
    _zoomWorld.fillTrap(painter, brush,
                        ii-1, yMin,
                        ii, yMin,
                        ii, val,
                        ii-1, valPrev);
  }

  // draw the spectrum - as line

  painter.save();
  painter.setPen(_params.iqplot_spectrum_line_color);
  QVector<QPointF> pts;
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = (ii + nSamples / 2) % nSamples;
    double val = powerDbm[jj];
    QPointF pt(ii, val);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the title

  painter.save();
  painter.setPen(_params.iqplot_title_color);
  char title[1024];
  snprintf(title, 1024, "%s range: %.3fkm",
           getName(_plotType).c_str(), selectedRangeKm);
  _zoomWorld.drawTitleTopCenter(painter, title);
  painter.restore();

}

//////////////////////////////////
// get a string for the field name

string IqPlot::getName(Params::iqplot_type_t ptype)
{
  switch (ptype) {
    case Params::SPECTRUM:
      return "SPECTRUM";
    case Params::TIME_SERIES:
      return "TIME_SERIES";
    case Params::I_VS_Q:
      return "I_VS_Q";
    case Params::PHASOR:
      return "PHASOR";
    default:
      return "UNKNOWN";
  }
}

//////////////////////////////////
// get a string for the X axis units

string IqPlot::getXUnits(Params::iqplot_type_t ptype)
{
  switch (ptype) {
    case Params::SPECTRUM:
      return "sample";
    case Params::TIME_SERIES:
      return "sample";
    case Params::I_VS_Q:
      return "volts";
    case Params::PHASOR:
      return "sample";
    default:
      return "";
  }
}

//////////////////////////////////
// get a string for the Y axis units

string IqPlot::getYUnits(Params::iqplot_type_t ptype)
{
  switch (ptype) {
    case Params::SPECTRUM:
      return "power";
    case Params::TIME_SERIES:
      return "volts";
    case Params::I_VS_Q:
      return "volts";
    case Params::PHASOR:
      return "volts";
    default:
      return "";
  }
}

////////////////////////////////////////////
// get min val for plotting

double IqPlot::getMinVal(Params::iqplot_type_t ptype)
{
  switch (ptype) {
    case Params::SPECTRUM:
      return -120;
    case Params::TIME_SERIES:
      return -120;
    case Params::I_VS_Q:
      return -120;
    case Params::PHASOR:
      return 0;
    default:
      return 0;
  }
}

////////////////////////////////////////////
// get max val for plotting

double IqPlot::getMaxVal(Params::iqplot_type_t ptype)
{
  switch (ptype) {
    case Params::SPECTRUM:
      return 20;
    case Params::TIME_SERIES:
      return 20;
    case Params::I_VS_Q:
      return 20;
    case Params::PHASOR:
      return 1;
    default:
      return 0;
  }
}

/*************************************************************************
 * set the geometry - unzooms
 */

void IqPlot::setWindowGeom(int width, int height,
                           int xOffset, int yOffset)
{
  _fullWorld.setWindowGeom(width, height, xOffset, yOffset);
  _zoomWorld = _fullWorld;
}

/*************************************************************************
 * set the world limits - unzooms
 */

void IqPlot::setWorldLimits(double xMinWorld,
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

void IqPlot::setZoomLimits(int xMin,
                           int yMin,
                           int xMax,
                           int yMax)
{
  _zoomWorld.setZoomLimits(xMin, yMin, xMax, yMax);
  _isZoomed = true;
}

void IqPlot::setZoomLimitsX(int xMin,
                            int xMax)
{
  _zoomWorld.setZoomLimitsX(xMin, xMax);
  _isZoomed = true;
}

void IqPlot::setZoomLimitsY(int yMin,
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

void IqPlot::_drawOverlays(QPainter &painter, double selectedRangeKm)
{

  // save painter state
  
  painter.save();
  
  // store font
  
  QFont origFont = painter.font();
  
  painter.setPen(_params.iqplot_axis_label_color);

  _zoomWorld.drawAxisBottom(painter, getXUnits(_plotType),
                            true, true, true, _xGridLinesOn);

  _zoomWorld.drawAxisLeft(painter, getYUnits(_plotType), 
                          true, true, true, _yGridLinesOn);

  // _zoomWorld.drawYAxisLabelLeft(painter, "Range");

  painter.restore();

}

