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
#include <algorithm>
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
                      int nSamples,
                      double selectedRangeKm)
  
{

  if (beam == NULL) {
    cerr << "WARNING - IqPlot::plotBeam() - got NULL beam, ignoring"
         << endl;
    return;
  }
  
  int gateNum = beam->getGateNum(selectedRangeKm);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "======== Iqplot - plotting beam data ================" << endl;
    DateTime beamTime(beam->getTimeSecs(), true, beam->getNanoSecs() * 1.0e-9);
    cerr << "  Beam time: " << beamTime.asString(3) << endl;
    cerr << "  selected range: " << selectedRangeKm << endl;
    cerr << "  gate num: " << gateNum << endl;
  }

  // get data for this gate

  const GateData *gateData = beam->getGateData()[gateNum];
  
  // perform the relevant plot

  switch (_plotType) {
    case Params::I_AND_Q:
      _plotIandQ(painter, beam, nSamples, selectedRangeKm,
                 gateNum, gateData);
      break;
    case Params::I_VS_Q:
      _plotIvsQ(painter, beam, nSamples, selectedRangeKm,
                gateNum, gateData);
      break;
    case Params::PHASOR:
      _plotPhasor(painter, beam, nSamples, selectedRangeKm,
                  gateNum, gateData);
      break;
    case Params::SPECTRUM:
    default:
      _plotSpectrum(painter, beam, nSamples, selectedRangeKm,
                    gateNum, gateData);
  }

}

/*************************************************************************
 * plot the spectrum
 */

void IqPlot::_plotSpectrum(QPainter &painter,
                           Beam *beam,
                           int nSamples,
                           double selectedRangeKm,
                           int gateNum,
                           const GateData *gateData)
  
{

  // set the windowing

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
  RadarMoments::applyWindow(gateData->iqhcOrig, windowCoeff, iqWindowed, nSamples);
  
  TaArray<RadarComplex_t> powerSpec_;
  RadarComplex_t *powerSpec = powerSpec_.alloc(nSamples);
  RadarFft fft(nSamples);
  fft.fwd(iqWindowed, powerSpec);
  fft.shift(powerSpec);
  
  // compute power, plus min and max

  TaArray<double> powerDbm_;
  double *powerDbm = powerDbm_.alloc(nSamples);
  double minDbm = 9999.0, maxDbm = -9999.0;
  for (int ii = 0; ii < nSamples; ii++) {
    double power = RadarComplex::power(powerSpec[ii]);
    double dbm = 10.0 * log10(power);
    if (power <= 0) {
      dbm = -120.0;
    }
    powerDbm[ii] = dbm;
    minDbm = min(dbm, minDbm);
    maxDbm = max(dbm, maxDbm);
  }
  
  // set the Y axis range

  double rangeY = maxDbm - minDbm;
  if (!_isZoomed) {
    setWorldLimitsY(minDbm - rangeY * 0.05, maxDbm + rangeY * 0.1);
  }
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the spectrum - as line

  painter.save();
  painter.setPen(_params.iqplot_line_color);
  QVector<QPointF> pts;
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = (ii + nSamples) % nSamples;
    double val = powerDbm[ii];
    QPointF pt(jj, val);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // draw the title

  painter.save();
  painter.setPen(_params.iqplot_title_color);
  char title[1024];
  snprintf(title, 1024, "%s range: %.3fkm",
           getName(_plotType).c_str(), selectedRangeKm);
  _zoomWorld.drawTitleTopCenter(painter, title);
  painter.restore();

}

/*************************************************************************
 * plot the I and Q time series
 */

void IqPlot::_plotIandQ(QPainter &painter,
                        Beam *beam,
                        int nSamples,
                        double selectedRangeKm,
                        int gateNum,
                        const GateData *gateData)
  
{

  // get I and Q vals

  TaArray<double> iVals_, qVals_;
  double *iVals = iVals_.alloc(nSamples);
  double *qVals = qVals_.alloc(nSamples);
  double minVal = 9999.0;
  double maxVal = -9999.0;
  for (int ii = 0; ii < nSamples; ii++) {
    double iVal = gateData->iqhcOrig[ii].re;
    double qVal = gateData->iqhcOrig[ii].im;
    qVals[ii] = iVal;
    iVals[ii] = qVal;
    minVal = min(iVal, minVal);
    minVal = min(qVal, minVal);
    maxVal = max(iVal, maxVal);
    maxVal = max(qVal, maxVal);
  }
  
  // set the Y axis range

  double rangeY = maxVal - minVal;
  if (!_isZoomed) {
    setWorldLimitsY(minVal - rangeY * 0.05, maxVal + rangeY * 0.1);
  }
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the I and Q data

  painter.save();

  painter.setPen(_params.iqplot_ival_line_color);
  QVector<QPointF> ipts;
  for (int ii = 0; ii < nSamples; ii++) {
    double ival = iVals[ii];
    QPointF pt(ii, ival);
    ipts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, ipts);

  painter.setPen(_params.iqplot_qval_line_color);
  QVector<QPointF> qpts;
  for (int ii = 0; ii < nSamples; ii++) {
    double qval = qVals[ii];
    QPointF pt(ii, qval);
    qpts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, qpts);

  painter.restore();
  
  // draw the title

  painter.save();
  painter.setPen(_params.iqplot_title_color);
  char title[1024];
  snprintf(title, 1024, "%s range: %.3fkm",
           getName(_plotType).c_str(), selectedRangeKm);
  _zoomWorld.drawTitleTopCenter(painter, title);
  painter.restore();

}

/*************************************************************************
 * plot I vs Q
 */

void IqPlot::_plotIvsQ(QPainter &painter,
                       Beam *beam,
                       int nSamples,
                       double selectedRangeKm,
                       int gateNum,
                       const GateData *gateData)
  
{

  // get I and Q vals

  TaArray<double> iVals_, qVals_;
  double *iVals = iVals_.alloc(nSamples);
  double *qVals = qVals_.alloc(nSamples);
  double minIVal = 9999.0;
  double maxIVal = -9999.0;
  double minQVal = 9999.0;
  double maxQVal = -9999.0;
  for (int ii = 0; ii < nSamples; ii++) {
    double iVal = gateData->iqhcOrig[ii].re;
    double qVal = gateData->iqhcOrig[ii].im;
    iVals[ii] = iVal;
    qVals[ii] = qVal;
    minIVal = min(iVal, minIVal);
    maxIVal = max(iVal, maxIVal);
    minQVal = min(qVal, minQVal);
    maxQVal = max(qVal, maxQVal);
  }
  
  // set the Y axis range

  double rangeX = maxIVal - minIVal;
  double rangeY = maxQVal - minQVal;
  if (!_isZoomed) {
    setWorldLimits(minIVal - rangeX * 0.05,
                   minQVal - rangeY * 0.05,
                   maxIVal + rangeX * 0.05,
                   maxQVal + rangeY * 0.1);
  }
  
  // draw the overlays
  
  _drawOverlays(painter, selectedRangeKm);

  // draw the I vs Q data

  painter.save();
  painter.setPen(_params.iqplot_line_color);
  QVector<QPointF> iqpts;
  for (int ii = 0; ii < nSamples; ii++) {
    double ival = iVals[ii];
    double qval = qVals[ii];
    QPointF pt(ival, qval);
    iqpts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, iqpts);
  painter.restore();
  
  // draw the title

  painter.save();
  painter.setPen(_params.iqplot_title_color);
  char title[1024];
  snprintf(title, 1024, "%s range: %.3fkm",
           getName(_plotType).c_str(), selectedRangeKm);
  _zoomWorld.drawTitleTopCenter(painter, title);
  painter.restore();

}

/*************************************************************************
 * plot PHASOR
 */

void IqPlot::_plotPhasor(QPainter &painter,
                         Beam *beam,
                         int nSamples,
                         double selectedRangeKm,
                         int gateNum,
                         const GateData *gateData)

{
                           
  // get I and Q vals

  TaArray<double> iSums_, qSums_;
  double *iSums = iSums_.alloc(nSamples);
  double *qSums = qSums_.alloc(nSamples);
  double minISum = 9999.0;
  double maxISum = -9999.0;
  double minQSum = 9999.0;
  double maxQSum = -9999.0;
  double iSum = 0.0;
  double qSum = 0.0;
  for (int ii = 0; ii < nSamples; ii++) {
    iSum += gateData->iqhcOrig[ii].re;
    qSum += gateData->iqhcOrig[ii].im;
    iSums[ii] = iSum;
    qSums[ii] = qSum;
    minISum = min(iSum, minISum);
    maxISum = max(iSum, maxISum);
    minQSum = min(qSum, minQSum);
    maxQSum = max(qSum, maxQSum);
  }
  
  // set the Y axis range

  double rangeX = maxISum - minISum;
  double rangeY = maxQSum - minQSum;
  if (!_isZoomed) {
    setWorldLimits(minISum - rangeX * 0.05,
                   minQSum - rangeY * 0.05,
                   maxISum + rangeX * 0.05,
                   maxQSum + rangeY * 0.1);
  }
  
  // draw the overlays
  
  _drawOverlays(painter, selectedRangeKm);

  // draw the I vs Q data

  painter.save();
  painter.setPen(_params.iqplot_line_color);
  QVector<QPointF> iqpts;
  for (int ii = 0; ii < nSamples; ii++) {
    double ival = iSums[ii];
    double qval = qSums[ii];
    QPointF pt(ival, qval);
    iqpts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, iqpts);
  painter.restore();
  
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
    case Params::I_AND_Q:
      return "I_AND_Q";
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
    case Params::I_AND_Q:
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
    case Params::I_AND_Q:
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
    case Params::I_AND_Q:
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
    case Params::I_AND_Q:
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
  _isZoomed = false;
}

void IqPlot::setWorldLimitsX(double xMinWorld,
                             double xMaxWorld)
{
  _fullWorld.setWorldLimitsX(xMinWorld, xMaxWorld);
  _zoomWorld = _fullWorld;
  _isZoomed = false;
}

void IqPlot::setWorldLimitsY(double yMinWorld,
                             double yMaxWorld)
{
  _fullWorld.setWorldLimitsY(yMinWorld, yMaxWorld);
  _zoomWorld = _fullWorld;
  _isZoomed = false;
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

