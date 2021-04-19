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
#include <rapmath/ForsytheFit.hh>
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
  _plotType = Params::SPECTRUM_POWER;
  _rxChannel = Params::CHANNEL_HC;
  _fftWindow = Params::FFT_WINDOW_VONHANN;
  _useAdaptiveFilter = false;
  _useRegressionFilter = false;
  _regressionOrder = 3;
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
  if (gateNum > beam->getNGates() - 1) {
    cerr << "ERROR - Iqplot::plotBeam()" << endl;
    cerr << "  range exceeds max: " << selectedRangeKm << endl;
    return;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "======== Iqplot - plotting beam data ================" << endl;
    DateTime beamTime(beam->getTimeSecs(), true, beam->getNanoSecs() * 1.0e-9);
    cerr << "  Beam time: " << beamTime.asString(3) << endl;
    cerr << "  selected range: " << selectedRangeKm << endl;
    cerr << "  gate num: " << gateNum << endl;
  }

  // get data for this gate

  const GateData *gateData = beam->getGateData()[gateNum];
  TaArray<RadarComplex_t> iq_;
  RadarComplex_t *iq = iq_.alloc(nSamples);
  switch (_rxChannel) {
    case Params::CHANNEL_HC:
      memcpy(iq, gateData->iqhcOrig, nSamples * sizeof(RadarComplex_t));
      break;
    case Params::CHANNEL_VC:
      memcpy(iq, gateData->iqvcOrig, nSamples * sizeof(RadarComplex_t));
      break;
    case Params::CHANNEL_HX:
      memcpy(iq, gateData->iqhxOrig, nSamples * sizeof(RadarComplex_t));
      break;
    case Params::CHANNEL_VX:
      memcpy(iq, gateData->iqvxOrig, nSamples * sizeof(RadarComplex_t));
  }
  
  // perform the relevant plot

  switch (_plotType) {
    case Params::I_VALS: {
      TaArray<double> iVals_;
      double *iVals = iVals_.alloc(nSamples);
      for (int ii = 0; ii < nSamples; ii++) {
        iVals[ii] = iq[ii].re;
      }
      _plotIQVals(painter, beam, nSamples, selectedRangeKm,
                  gateNum, iVals);
    }
      break;
    case Params::Q_VALS: {
      TaArray<double> qVals_;
      double *qVals = qVals_.alloc(nSamples);
      for (int ii = 0; ii < nSamples; ii++) {
        qVals[ii] = iq[ii].im;
      }
      _plotIQVals(painter, beam, nSamples, selectedRangeKm,
                  gateNum, qVals);
    }
      break;
    case Params::I_VS_Q:
      _plotIvsQ(painter, beam, nSamples, selectedRangeKm,
                gateNum, iq);
      break;
    case Params::PHASOR:
      _plotPhasor(painter, beam, nSamples, selectedRangeKm,
                  gateNum, iq);
      break;
    case Params::SPECTRUM_PHASE:
      _plotSpectrumPhase(painter, beam, nSamples, selectedRangeKm,
                         gateNum, iq);
      break;
    case Params::TS_POWER:
      _plotTsPower(painter, beam, nSamples, selectedRangeKm,
                   gateNum, iq);
      break;
    case Params::TS_PHASE:
      _plotTsPhase(painter, beam, nSamples, selectedRangeKm,
                   gateNum, iq);
      break;
    case Params::SPECTRUM_POWER:
    default:
      _plotSpectrumPower(painter, beam, nSamples, selectedRangeKm,
                         gateNum, iq);
  }

}

/*************************************************************************
 * plot the power spectrum
 */

void IqPlot::_plotSpectrumPower(QPainter &painter,
                                Beam *beam,
                                int nSamples,
                                double selectedRangeKm,
                                int gateNum,
                                const RadarComplex_t *iq)
  
{

  // apply window to time series
  
  TaArray<RadarComplex_t> iqWindowed_;
  RadarComplex_t *iqWindowed = iqWindowed_.alloc(nSamples);
  _applyWindow(iq, iqWindowed, nSamples);
  
  // compute power spectrum
  
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
    if (power <= 1.0e-12) {
      dbm = -120.0;
    }
    powerDbm[ii] = dbm;
    minDbm = min(dbm, minDbm);
    maxDbm = max(dbm, maxDbm);
  }
  
  // compute adaptive filter as appropriate
  
  TaArray<double> filtAdaptDbm_;
  double *filtAdaptDbm = filtAdaptDbm_.alloc(nSamples);
  
  const IwrfCalib &calib = beam->getCalib();
  double calibNoise = 0.0;
  switch (_rxChannel) {
    case Params::CHANNEL_HC:
      calibNoise = pow(10.0, calib.getNoiseDbmHc() / 10.0);
      break;
    case Params::CHANNEL_VC:
      calibNoise = pow(10.0, calib.getNoiseDbmVc() / 10.0);
      break;
    case Params::CHANNEL_HX:
      calibNoise = pow(10.0, calib.getNoiseDbmHx() / 10.0);
      break;
    case Params::CHANNEL_VX:
      calibNoise = pow(10.0, calib.getNoiseDbmVx() / 10.0);
      break;
  }
  
  RadarMoments moments;
  moments.setNSamples(nSamples);
  moments.init(beam->getPrt(), calib.getWavelengthCm() / 100.0,
               beam->getStartRangeKm(), beam->getGateSpacingKm());
  moments.setCalib(calib);
  
  if (_useAdaptiveFilter) {
    
    TaArray<RadarComplex_t> filtAdaptWindowed_;
    RadarComplex_t *filtAdaptWindowed = filtAdaptWindowed_.alloc(nSamples);
    double filterRatio, spectralNoise, spectralSnr;
    moments.applyAdaptiveFilter(nSamples, fft,
                                iqWindowed, NULL,
                                calibNoise,
                                filtAdaptWindowed, NULL,
                                filterRatio,
                                spectralNoise,
                                spectralSnr);
    
    TaArray<RadarComplex_t> filtAdaptSpec_;
    RadarComplex_t *filtAdaptSpec = filtAdaptSpec_.alloc(nSamples);
    fft.fwd(filtAdaptWindowed, filtAdaptSpec);
    fft.shift(filtAdaptSpec);
    
    for (int ii = 0; ii < nSamples; ii++) {
      double power = RadarComplex::power(filtAdaptSpec[ii]);
      double dbm = 10.0 * log10(power);
      if (power <= 1.0e-12) {
        dbm = -120.0;
      }
      filtAdaptDbm[ii] = dbm;
      minDbm = min(dbm, minDbm);
      maxDbm = max(dbm, maxDbm);
    }

  } // if (_useAdaptiveFilter)

  // compute regression filter as appropriate

  TaArray<double> filtRegrDbm_;
  double *filtRegrDbm = filtRegrDbm_.alloc(nSamples);

  if (_useRegressionFilter) {
    
    RegressionFilter regrF;
    if (beam->getIsStagPrt()) {
      regrF.setupStaggered(nSamples, beam->getStagM(),
                           beam->getStagN(), _regressionOrder);
    } else {
      regrF.setup(nSamples, _regressionOrder);
    }
    
    TaArray<RadarComplex_t> filtered_;
    RadarComplex_t *filtered = filtered_.alloc(nSamples);
    double filterRatio, spectralNoise, spectralSnr;
    moments.applyRegressionFilter(nSamples, fft,
                                  regrF, _windowCoeff,
                                  iq,
                                  calibNoise,
                                  _params.regression_filter_interp_across_notch,
                                  filtered, NULL,
                                  filterRatio,
                                  spectralNoise,
                                  spectralSnr);
    
    TaArray<RadarComplex_t> filtRegrWindowed_;
    RadarComplex_t *filtRegrWindowed = filtRegrWindowed_.alloc(nSamples);
    _applyWindow(filtered, filtRegrWindowed, nSamples);
  
    TaArray<RadarComplex_t> filtRegrSpec_;
    RadarComplex_t *filtRegrSpec = filtRegrSpec_.alloc(nSamples);
    fft.fwd(filtRegrWindowed, filtRegrSpec);
    fft.shift(filtRegrSpec);
    
    for (int ii = 0; ii < nSamples; ii++) {
      double power = RadarComplex::power(filtRegrSpec[ii]);
      double dbm = 10.0 * log10(power);
      if (power <= 1.0e-12) {
        dbm = -120.0;
      }
      minDbm = min(dbm, minDbm);
      maxDbm = max(dbm, maxDbm);
      filtRegrDbm[ii] = dbm;
    }
    
  } // if (_useRegressionFilter)

  // set the Y axis range

  double rangeY = maxDbm - minDbm;
  if (!_isZoomed) {
    setWorldLimitsY(minDbm - rangeY * 0.05, maxDbm + rangeY * 0.125);
  }
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the unfiltered spectrum

  painter.save();
  QPen pen(painter.pen());
  pen.setColor(_params.iqplot_line_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(_params.iqplot_line_width);
  painter.setPen(pen);
  QVector<QPointF> pts;
  for (int ii = 0; ii < nSamples; ii++) {
    double val = powerDbm[ii];
    QPointF pt(ii, val);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();

  // draw adaptive filter
  
  if (_useAdaptiveFilter) {
    
    painter.save();
    QPen pen(painter.pen());
    pen.setColor(_params.iqplot_adaptive_filtered_color);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(_params.iqplot_line_width);
    painter.setPen(pen);
    QVector<QPointF> filtPts;
    for (int ii = 0; ii < nSamples; ii++) {
      QPointF pt(ii, filtAdaptDbm[ii]);
      filtPts.push_back(pt);
    }
    _zoomWorld.drawLines(painter, filtPts);
    painter.restore();

  } // if (_useAdaptiveFilter)

  // draw regression filter

  if (_useRegressionFilter) {
    
    painter.save();
    QPen pen(painter.pen());
    pen.setColor(_params.iqplot_regression_filtered_color);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(_params.iqplot_line_width);
    painter.setPen(pen);
    QVector<QPointF> filtPts;
    for (int ii = 0; ii < nSamples; ii++) {
      QPointF pt(ii, filtRegrDbm[ii]);
      filtPts.push_back(pt);
    }
    _zoomWorld.drawLines(painter, filtPts);
    painter.restore();

  } // if (_useRegressionFilter)

  // plot clutter model

  double width = 2.0;
  
  // legends

  const MomentsFields* fields = beam->getOutFields();
  double dbm = fields[gateNum].dbm;
  double dbz = fields[gateNum].dbz;
  double vel = fields[gateNum].vel;

  char text[1024];
  vector<string> legendsLeft;
  snprintf(text, 1024, "Dbm: %.2f", dbm);
  legendsLeft.push_back(text);
  snprintf(text, 1024, "Dbz: %.2f", dbz);
  legendsLeft.push_back(text);
  snprintf(text, 1024, "Vel: %.2f", vel);
  legendsLeft.push_back(text);
  _zoomWorld.drawLegendsTopLeft(painter, legendsLeft);

  vector<string> legendsRight;
  legendsRight.push_back(getFftWindowName());
  if (_useRegressionFilter) {
    snprintf(text, 1024, "Regr-order: %d", _regressionOrder);
    legendsRight.push_back(text);
  }
  _zoomWorld.drawLegendsTopRight(painter, legendsRight);

  // draw the title

  painter.save();
  painter.setPen(_params.iqplot_title_color);
  _zoomWorld.drawTitleTopCenter(painter, getName());
  painter.restore();

}

/*************************************************************************
 * plot the spectrum phase
 */

void IqPlot::_plotSpectrumPhase(QPainter &painter,
                                Beam *beam,
                                int nSamples,
                                double selectedRangeKm,
                                int gateNum,
                                const RadarComplex_t *iq)
  
{

  // apply window to time series
  
  TaArray<RadarComplex_t> iqWindowed_;
  RadarComplex_t *iqWindowed = iqWindowed_.alloc(nSamples);
  _applyWindow(iq, iqWindowed, nSamples);
  
  // compute spectrum
  
  TaArray<RadarComplex_t> spec_;
  RadarComplex_t *spec = spec_.alloc(nSamples);
  RadarFft fft(nSamples);
  fft.fwd(iqWindowed, spec);
  fft.shift(spec);
  
  // compute phase

  TaArray<double> phase_;
  double *phase = phase_.alloc(nSamples);
  double minVal = 9999.0, maxVal = -9999.0;
  for (int ii = 0; ii < nSamples; ii++) {
    double phaseRad = atan2(spec[ii].im, spec[ii].re);
    double phaseDeg = phaseRad * RAD_TO_DEG;
    phase[ii] = phaseDeg;
    minVal = min(phaseDeg, minVal);
    maxVal = max(phaseDeg, maxVal);
  }
  
  // set the Y axis range
  
  double rangeY = maxVal - minVal;
  if (!_isZoomed) {
    setWorldLimitsY(minVal - rangeY * 0.05, maxVal + rangeY * 0.125);
  }
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the spectrum phase - as line

  painter.save();
  QPen pen(painter.pen());
  pen.setColor(_params.iqplot_line_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(_params.iqplot_line_width);
  painter.setPen(pen);
  QVector<QPointF> pts;
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = (ii + nSamples) % nSamples;
    double val = phase[ii];
    QPointF pt(jj, val);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // legends

  const MomentsFields* fields = beam->getOutFields();
  double vel = fields[gateNum].vel;
  
  char text[1024];
  vector<string> legendsLeft;
  snprintf(text, 1024, "Vel: %.2f", vel);
  legendsLeft.push_back(text);
  _zoomWorld.drawLegendsTopLeft(painter, legendsLeft);
  
  vector<string> legendsRight;
  legendsRight.push_back(getFftWindowName());
  _zoomWorld.drawLegendsTopRight(painter, legendsRight);

  // draw the title
  
  painter.save();
  painter.setPen(_params.iqplot_title_color);
  _zoomWorld.drawTitleTopCenter(painter, getName());
  painter.restore();

}

/*************************************************************************
 * plot the time series power
 */

void IqPlot::_plotTsPower(QPainter &painter,
                          Beam *beam,
                          int nSamples,
                          double selectedRangeKm,
                          int gateNum,
                          const RadarComplex_t *iq)
  
{

  // compute power, plus min and max
  
  TaArray<double> powerDbm_;
  double *powerDbm = powerDbm_.alloc(nSamples);
  double minDbm = 9999.0, maxDbm = -9999.0;
  for (int ii = 0; ii < nSamples; ii++) {
    double power = RadarComplex::power(iq[ii]);
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
    setWorldLimitsY(minDbm - rangeY * 0.05, maxDbm + rangeY * 0.125);
  }
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the spectrum - as line

  painter.save();
  QPen pen(painter.pen());
  pen.setColor(_params.iqplot_line_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(_params.iqplot_line_width);
  painter.setPen(pen);
  QVector<QPointF> pts;
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = (ii + nSamples) % nSamples;
    double val = powerDbm[ii];
    QPointF pt(jj, val);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // legends

  double power = RadarComplex::meanPower(iq, nSamples);
  double dbm = 10.0 * log10(power);
  char text[1024];
  snprintf(text, 1024, "DbmMean: %.2f", dbm);
  vector<string> legends;
  legends.push_back(text);
  _zoomWorld.drawLegendsTopLeft(painter, legends);
  
  // draw the title

  painter.save();
  painter.setPen(_params.iqplot_title_color);
  _zoomWorld.drawTitleTopCenter(painter, getName());
  painter.restore();

}

/*************************************************************************
 * plot the time series phase
 */

void IqPlot::_plotTsPhase(QPainter &painter,
                          Beam *beam,
                          int nSamples,
                          double selectedRangeKm,
                          int gateNum,
                          const RadarComplex_t *iq)
  
{

  // compute time series phases
  
  TaArray<double> phase_;
  double *phase = phase_.alloc(nSamples);
  double minVal = 9999.0, maxVal = -9999.0;
  for (int ii = 0; ii < nSamples; ii++) {
    double phaseRad = atan2(iq[ii].im, iq[ii].re);
    double phaseDeg = phaseRad * RAD_TO_DEG;
    phase[ii] = phaseDeg;
    minVal = min(phaseDeg, minVal);
    maxVal = max(phaseDeg, maxVal);
  }
  
  // set the Y axis range

  double rangeY = maxVal - minVal;
  if (!_isZoomed) {
    setWorldLimitsY(minVal - rangeY * 0.05, maxVal + rangeY * 0.125);
  }
  
  // draw the overlays
  
  _drawOverlays(painter, selectedRangeKm);
  
  // draw the time series phase - as line

  painter.save();
  QPen pen(painter.pen());
  pen.setColor(_params.iqplot_line_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(_params.iqplot_line_width);
  painter.setPen(pen);
  QVector<QPointF> pts;
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = (ii + nSamples) % nSamples;
    double val = phase[ii];
    QPointF pt(jj, val);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // draw the title
  
  painter.save();
  painter.setPen(_params.iqplot_title_color);
  _zoomWorld.drawTitleTopCenter(painter, getName());
  painter.restore();

}

/*************************************************************************
 * plot the time series for I or Q separately
 */

void IqPlot::_plotIQVals(QPainter &painter,
                         Beam *beam,
                         int nSamples,
                         double selectedRangeKm,
                         int gateNum,
                         const double *vals)
  
{

  // prepare Forsythe compute object for regression
  
  vector<double> xVals;
  for (int ii = 0; ii < nSamples; ii++) {
    xVals.push_back(ii);
  }
  ForsytheFit forsythe;
  forsythe.prepareForFit(_regressionOrder, xVals);
  
  // compute polynomial regression fit

  vector<double> raw;
  for (int ii = 0; ii < nSamples; ii++) {
    raw.push_back(vals[ii]);
  }
  forsythe.performFit(raw);
  vector<double> smoothed = forsythe.getYEstVector();
  vector<double> residual;
  for (int ii = 0; ii < nSamples; ii++) {
    residual.push_back(raw[ii] - smoothed[ii]);
  }

  // compute min and max vals for plot
  
  double minVal = 9999.0;
  double maxVal = -9999.0;
  for (int ii = 0; ii < nSamples; ii++) {
    minVal = min(vals[ii], minVal);
    minVal = min(smoothed[ii], minVal);
    minVal = min(residual[ii], minVal);
    maxVal = max(vals[ii], maxVal);
    maxVal = max(smoothed[ii], maxVal);
    maxVal = max(residual[ii], maxVal);
  }
  
  // set the Y axis range
  
  double rangeY = maxVal - minVal;
  if (!_isZoomed) {
    setWorldLimitsY(minVal - rangeY * 0.05, maxVal + rangeY * 0.125);
  }
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the I data

  painter.save();
  QPen pen(painter.pen());
  pen.setColor(_params.iqplot_line_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(_params.iqplot_line_width);
  painter.setPen(pen);
  QVector<QPointF> pts;
  for (int ii = 0; ii < nSamples; ii++) {
    QPointF pt(ii, vals[ii]);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // plot regression filter as appropriate

  if (_useRegressionFilter) {
    
    // plot the polynomial fit
    
    painter.save();
    QPen pen(painter.pen());
    pen.setColor(_params.iqplot_polynomial_fit_color);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(_params.iqplot_polynomial_line_width);
    painter.setPen(pen);
    QVector<QPointF> pts;
    for (int ii = 0; ii < nSamples; ii++) {
      QPointF pt(ii, smoothed[ii]);
      pts.push_back(pt);
    }
    _zoomWorld.drawLines(painter, pts);
    painter.restore();
    
    // draw the residuals from polynmial fit
    
    painter.save();
    pen.setColor(_params.iqplot_polynomial_residual_color);
    pen.setStyle(Qt::DotLine);
    pen.setWidth(_params.iqplot_polynomial_line_width);
    painter.setPen(pen);
    pts.clear();
    for (int ii = 0; ii < nSamples; ii++) {
      QPointF pt(ii, residual[ii]);
      pts.push_back(pt);
    }
    _zoomWorld.drawLines(painter, pts);
    painter.restore();

    // Legend
    
    char text[1024];
    snprintf(text, 1024, "Regr-order: %d", _regressionOrder);
    vector<string> legends;
    legends.push_back(text);
    _zoomWorld.drawLegendsTopLeft(painter, legends);

  } // if (_useRegressionFilter)

  // draw the title

  painter.save();
  painter.setPen(_params.iqplot_title_color);
  _zoomWorld.drawTitleTopCenter(painter, getName());
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
                       const RadarComplex_t *iq)
  
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
    double iVal = iq[ii].re;
    double qVal = iq[ii].im;
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
                   maxQVal + rangeY * 0.125);
  }
  
  // draw the overlays
  
  _drawOverlays(painter, selectedRangeKm);

  // draw the I vs Q data

  painter.save();
  QPen pen(painter.pen());
  pen.setColor(_params.iqplot_line_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(_params.iqplot_line_width);
  painter.setPen(pen);
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
  _zoomWorld.drawTitleTopCenter(painter, getName());
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
                         const RadarComplex_t *iq)

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
    iSum += iq[ii].re;
    qSum += iq[ii].im;
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
                   maxQSum + rangeY * 0.125);
  }
  
  // draw the overlays
  
  _drawOverlays(painter, selectedRangeKm);

  // draw the I vs Q data

  painter.save();
  QPen pen(painter.pen());
  pen.setColor(_params.iqplot_line_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(_params.iqplot_line_width);
  painter.setPen(pen);
  QVector<QPointF> iqpts;
  for (int ii = 0; ii < nSamples; ii++) {
    double ival = iSums[ii];
    double qval = qSums[ii];
    QPointF pt(ival, qval);
    iqpts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, iqpts);
  painter.restore();

  // legends

  double cpa = RadarMoments::computeCpa(iq, nSamples);
  char text[1024];
  snprintf(text, 1024, "CPA: %.2f", cpa);
  vector<string> legends;
  legends.push_back(text);
  _zoomWorld.drawLegendsTopLeft(painter, legends);
  
  // draw the title

  painter.save();
  painter.setPen(_params.iqplot_title_color);
  _zoomWorld.drawTitleTopCenter(painter, getName());
  painter.restore();

}

///////////////////////////////////////
// Apply the window to the time series

void IqPlot::_applyWindow(const RadarComplex_t *iq, 
                          RadarComplex_t *iqWindowed,
                          int nSamples)
{
  
  // initialize the window
  
  _windowCoeff = _windowCoeff_.alloc(nSamples);
  switch (_fftWindow) {
    case Params::FFT_WINDOW_RECT:
    default:
      RadarMoments::initWindowRect(nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_VONHANN:
      RadarMoments::initWindowVonhann(nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_BLACKMAN:
      RadarMoments::initWindowBlackman(nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_BLACKMAN_NUTTALL:
      RadarMoments::initWindowBlackmanNuttall(nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_TUKEY_10:
      RadarMoments::initWindowTukey(0.1, nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_TUKEY_20:
      RadarMoments::initWindowTukey(0.2, nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_TUKEY_30:
      RadarMoments::initWindowTukey(0.3, nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_TUKEY_50:
      RadarMoments::initWindowTukey(0.5, nSamples, _windowCoeff);
  }

  // compute power spectrum
  
  RadarMoments::applyWindow(iq, _windowCoeff, iqWindowed, nSamples);

}
  
//////////////////////////////////
// get a string for the field name

string IqPlot::getName()
{

  string ptypeStr;
  switch (_plotType) {
    case Params::SPECTRUM_POWER:
      ptypeStr =  "SPECTRUM_POWER";
      break;
    case Params::SPECTRUM_PHASE:
      ptypeStr =  "SPECTRUM_PHASE";
      break;
    case Params::TS_POWER:
      ptypeStr =  "TS_POWER";
      break;
    case Params::TS_PHASE:
      ptypeStr =  "TS_PHASE";
      break;
    case Params::I_VALS:
      ptypeStr =  "I_VALS";
      break;
    case Params::Q_VALS:
      ptypeStr =  "Q_VALS";
      break;
    case Params::I_VS_Q:
      ptypeStr =  "I_VS_Q";
      break;
    case Params::PHASOR:
      ptypeStr =  "PHASOR";
      break;
  }
  
  string chanStr;
  switch (_rxChannel) {
    case Params::CHANNEL_HC:
      chanStr =  "HC";
      break;
    case Params::CHANNEL_VC:
      chanStr =  "VC";
      break;
    case Params::CHANNEL_HX:
      chanStr =  "HX";
      break;
    case Params::CHANNEL_VX:
      chanStr =  "VX";
      break;
  }

  string name = ptypeStr + " " + chanStr;

  return name;

}

//////////////////////////////////
// get a string for the X axis units

string IqPlot::getXUnits()
{
  switch (_plotType) {
    case Params::SPECTRUM_POWER:
    case Params::SPECTRUM_PHASE:
    case Params::TS_POWER:
    case Params::TS_PHASE:
    case Params::I_VALS:
      return "sample";
    case Params::I_VS_Q:
      return "volts";
    case Params::PHASOR:
      return "volts";
    default:
      return "";
  }
}

//////////////////////////////////
// get a string for the Y axis units

string IqPlot::getYUnits()
{
  switch (_plotType) {
    case Params::SPECTRUM_POWER:
      return "dbm";
    case Params::SPECTRUM_PHASE:
      return "deg";
    case Params::TS_POWER:
      return "dbm";
    case Params::TS_PHASE:
      return "deg";
    case Params::I_VALS:
    case Params::Q_VALS:
      return "volts";
    case Params::I_VS_Q:
      return "volts";
    case Params::PHASOR:
      return "volts";
    default:
      return "";
  }
}

//////////////////////////////////
// get fft window name

string IqPlot::getFftWindowName()
{
  switch (_fftWindow) {
    case Params::FFT_WINDOW_RECT:
      return "Rectangular";
    case Params::FFT_WINDOW_VONHANN:
      return "VonHann";
    case Params::FFT_WINDOW_BLACKMAN:
      return "Blackman";
    case Params::FFT_WINDOW_BLACKMAN_NUTTALL:
      return "Blackman-Nuttall";
    case Params::FFT_WINDOW_TUKEY_10:
      return "Tukey-10";
    case Params::FFT_WINDOW_TUKEY_20:
      return "Tukey-20";
    case Params::FFT_WINDOW_TUKEY_30:
      return "Tukey-30";
    case Params::FFT_WINDOW_TUKEY_50:
      return "Tukey-50";
    default:
      return "";
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

  _zoomWorld.drawAxisBottom(painter, getXUnits(),
                            true, true, true, _xGridLinesOn);

  _zoomWorld.drawAxisLeft(painter, getYUnits(), 
                          true, true, true, _yGridLinesOn);

  // _zoomWorld.drawYAxisLabelLeft(painter, "Range");

  painter.restore();

}

