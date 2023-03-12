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
#include <radar/FilterUtils.hh>

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
  _plotType = Params::SPECTRAL_POWER;
  _rxChannel = Params::CHANNEL_HC;
  _fftWindow = Params::FFT_WINDOW_VONHANN;
  _useAdaptFilt = false;
  _plotClutModel = false;
  _clutWidthMps = 0.75;
  _useRegrFilt = false;
  _regrOrder = 3;
  _regrFiltInterpAcrossNotch = true;
  _computePlotRangeDynamically = true;
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

///////////////////////////
// set the plot type

void IqPlot::setPlotType(Params::iq_plot_type_t val)
{
  _plotType = val;
  for (int ii = 0; ii < _params.iq_plot_static_ranges_n; ii++) {
    if (_plotType == _params._iq_plot_static_ranges[ii].plot_type) {
      _staticRange = _params._iq_plot_static_ranges[ii];
    }
  }
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
    case Params::I_VALS:
    case Params::Q_VALS: 
      {
        TaArray<double> iVals_;
        double *iVals = iVals_.alloc(nSamples);
        for (int ii = 0; ii < nSamples; ii++) {
          iVals[ii] = iq[ii].re;
        }
        TaArray<double> qVals_;
        double *qVals = qVals_.alloc(nSamples);
        for (int ii = 0; ii < nSamples; ii++) {
          qVals[ii] = iq[ii].im;
        }
        _plotIQVals(painter, beam, nSamples, selectedRangeKm,
                    gateNum, iVals, qVals);
      }
      break;
    case Params::I_VS_Q:
      _plotIvsQ(painter, beam, nSamples, selectedRangeKm, gateNum, iq);
      break;
    case Params::PHASOR:
      _plotPhasor(painter, beam, nSamples, selectedRangeKm, gateNum, iq);
      break;
    case Params::SPECTRAL_PHASE:
      _plotSpectralPhase(painter, beam, nSamples, selectedRangeKm, gateNum, iq);
      break;
    case Params::SPECTRAL_ZDR:
      _plotSpectralZdr(painter, beam, nSamples, selectedRangeKm, gateNum,
                       gateData->iqhcOrig, gateData->iqvcOrig);
      break;
    case Params::SPECTRAL_PHIDP:
      _plotSpectralPhidp(painter, beam, nSamples, selectedRangeKm, gateNum,
                         gateData->iqhcOrig, gateData->iqvcOrig);
      break;
    case Params::TS_POWER:
      _plotTsPower(painter, beam, nSamples, selectedRangeKm, gateNum, iq);
      break;
    case Params::TS_PHASE:
      _plotTsPhase(painter, beam, nSamples, selectedRangeKm, gateNum, iq);
      break;
    case Params::SPECTRAL_POWER:
    default:
      _plotSpectralPower(painter, beam, nSamples, selectedRangeKm, gateNum, iq);
  }

}

/*************************************************************************
 * plot the power spectrum
 */

void IqPlot::_plotSpectralPower(QPainter &painter,
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
  double totalPower = RadarComplex::meanPower(powerSpec, nSamples);

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
  moments.setClutterWidthMps(_clutWidthMps);
  moments.setClutterInitNotchWidthMps(3.0);
  
  double adaptPower = 1.0e-12;
  double adaptSpectralSnr = 1.0e-12;

  if (_useAdaptFilt) {
    
    TaArray<RadarComplex_t> filtAdaptWindowed_;
    RadarComplex_t *filtAdaptWindowed = filtAdaptWindowed_.alloc(nSamples);
    double filterRatio, spectralNoise;
    moments.applyAdaptiveFilter(nSamples, beam->getPrt(), fft,
                                iqWindowed, NULL,
                                calibNoise,
                                filtAdaptWindowed, NULL,
                                filterRatio,
                                spectralNoise,
                                adaptSpectralSnr);
    
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

    adaptPower = RadarComplex::meanPower(filtAdaptSpec, nSamples);
    
  } // if (_useAdaptFilt)

  // compute regression filter as appropriate

  TaArray<double> filtRegrDbm_;
  double *filtRegrDbm = filtRegrDbm_.alloc(nSamples);

  double regrPower = 0.0;
  if (_useRegrFilt) {
    
    RegressionFilter regrF;
    if (beam->getIsStagPrt()) {
      regrF.setupStaggered(nSamples, beam->getStagM(),
                           beam->getStagN(), _regrOrder);
    } else {
      regrF.setup(nSamples, _regrOrder);
    }
    
    TaArray<RadarComplex_t> filtered_;
    RadarComplex_t *filtered = filtered_.alloc(nSamples);
    double filterRatio, spectralNoise, spectralSnr;
    moments.applyRegressionFilter(nSamples, beam->getPrt(), fft,
                                  regrF, _windowCoeff,
                                  iq,
                                  calibNoise,
                                  _regrFiltInterpAcrossNotch,
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

    regrPower = RadarComplex::meanPower(filtRegrSpec, nSamples);
    
  } // if (_useRegrFilt)

  // set the Y axis range

  double rangeY = maxDbm - minDbm;
  double minY = minDbm - rangeY * 0.05;
  double maxY = maxDbm + rangeY * 0.125;
  if (!_isZoomed) {
    if (_computePlotRangeDynamically) {
      setWorldLimitsY(minY, maxY);
    } else {
      setWorldLimitsY(_staticRange.min_val, _staticRange.max_val);
    }
  }
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the unfiltered spectrum

  FilterUtils::applyMedianFilter(powerDbm, nSamples, _medianFiltLen);
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
  
  if (_useAdaptFilt) {
    
    FilterUtils::applyMedianFilter(filtAdaptDbm, nSamples, _medianFiltLen);
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

  } // if (_useAdaptFilt)

  // draw regr filter

  if (_useRegrFilt) {
    
    FilterUtils::applyMedianFilter(filtRegrDbm, nSamples, _medianFiltLen);
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

  } // if (_useRegrFilt)

  // plot clutter model

  if (_plotClutModel) {

    ClutFilter clut;
    TaArray<double> clutModel_;
    double *clutModel = clutModel_.alloc(nSamples);
    TaArray<double> powerReal_;
    double *powerReal = powerReal_.alloc(nSamples);
    for (int ii = 0; ii < nSamples; ii++) {
      powerReal[ii] = RadarComplex::power(powerSpec[ii]);
    }
    
    if (clut.computeGaussianClutterModel(powerReal,
                                         nSamples,
                                         _clutWidthMps,
                                         beam->getNyquist(),
                                         clutModel) == 0) {
      
      painter.save();
      pen = painter.pen();
      pen.setColor(_params.iqplot_clutter_model_color);
      pen.setStyle(Qt::DotLine);
      pen.setWidth(2);
      painter.setPen(pen);
      QVector<QPointF> clutPts;
      for (int ii = 0; ii < nSamples; ii++) {
        double power = clutModel[ii];
        double dbm = 10.0 * log10(power);
        if (dbm < minDbm) {
          dbm = minDbm;
        }
        QPointF pt(ii, dbm);
        clutPts.push_back(pt);
      }
      _zoomWorld.drawLines(painter, clutPts);
      painter.restore();

    } // if (clut.computeGaussianClutterModel

  } // if (_plotClutModel)

  // legends
  
  // const MomentsFields* fields = beam->getOutFields();
  // double dbm = fields[gateNum].dbm;
  // double dbz = fields[gateNum].dbz;
  // double vel = fields[gateNum].vel;

  char text[1024];

  vector<string> legendsLeft;
  legendsLeft.push_back(getFftWindowName());
  if (_medianFiltLen > 1) {
    snprintf(text, 1024, "Median filt len: %d", _medianFiltLen);
    legendsLeft.push_back(text);
  }
  if (_useRegrFilt) {
    snprintf(text, 1024, "Regr-order: %d", beam->getRegrOrder());
    legendsLeft.push_back(text);
  }
  if (_plotClutModel) {
    snprintf(text, 1024, "Clut-width: %g", _clutWidthMps);
    legendsLeft.push_back(text);
  }
  _zoomWorld.drawLegendsTopLeft(painter, legendsLeft);

  vector<string> legendsRight;
  double totalPowerDbm = 10.0 * log10(totalPower);
  snprintf(text, 1024, "TotalPower (dBm): %.2f", totalPowerDbm);
  legendsRight.push_back(text);
  if (_useAdaptFilt) {
    double adaptClutPower = totalPower - adaptPower;
    double adaptPowerDbm = 10.0 * log10(adaptPower);
    double adaptClutPowerDbm = 10.0 * log10(adaptClutPower);
    double adaptSpectralSnrDb = 10.0 * log10(adaptSpectralSnr);
    snprintf(text, 1024, "AdaptFiltPower: %.2f", adaptPowerDbm);
    legendsRight.push_back(text);
    snprintf(text, 1024, "AdaptClutPower: %.2f", adaptClutPowerDbm);
    legendsRight.push_back(text);
    snprintf(text, 1024, "AdaptCSR: %.2f", adaptClutPowerDbm - adaptPowerDbm);
    legendsRight.push_back(text);
    snprintf(text, 1024, "AdaptSpecSNR: %.2f", adaptSpectralSnrDb);
    legendsRight.push_back(text);
  }
  if (_useRegrFilt) {
    double regrClutPower = totalPower - regrPower;
    double regrPowerDbm = 10.0 * log10(regrPower);
    double regrClutPowerDbm = 10.0 * log10(regrClutPower);
    snprintf(text, 1024, "RegrFiltPower: %.2f", regrPowerDbm);
    legendsRight.push_back(text);
    snprintf(text, 1024, "RegrClutPower: %.2f", regrClutPowerDbm);
    legendsRight.push_back(text);
    snprintf(text, 1024, "RegrCSR: %.2f", regrClutPowerDbm - regrPowerDbm);
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

void IqPlot::_plotSpectralPhase(QPainter &painter,
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
    if (_computePlotRangeDynamically) {
      setWorldLimitsY(minVal - rangeY * 0.05, maxVal + rangeY * 0.125);
    } else {
      setWorldLimitsY(_staticRange.min_val, _staticRange.max_val);
    }
  }
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the spectrum phase

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
 * plot the spectrum ZDR
 */

void IqPlot::_plotSpectralZdr(QPainter &painter,
                              Beam *beam,
                              int nSamples,
                              double selectedRangeKm,
                              int gateNum,
                              const RadarComplex_t *iqHc,
                              const RadarComplex_t *iqVc)
  
{

  // compute ZDR spectrum
  
  TaArray<double> powerHc_, dbmHc_;
  double *powerHc = powerHc_.alloc(nSamples);
  double *dbmHc = dbmHc_.alloc(nSamples);
  _computePowerSpectrum(beam, nSamples, iqHc, powerHc, dbmHc);
  
  TaArray<double> powerVc_, dbmVc_;
  double *powerVc = powerVc_.alloc(nSamples);
  double *dbmVc = dbmVc_.alloc(nSamples);
  _computePowerSpectrum(beam, nSamples, iqVc, powerVc, dbmVc);
  
  TaArray<double> zdr_;
  double minVal = 9999.0, maxVal = -9999.0;
  double *zdr = zdr_.alloc(nSamples);
  for (int ii = 0; ii < nSamples; ii++) {
    zdr[ii] = dbmHc[ii] - dbmVc[ii];
    minVal = min(zdr[ii], minVal);
    maxVal = max(zdr[ii], maxVal);
  }

  // set the Y axis range
  
  double rangeY = maxVal - minVal;
  if (!_isZoomed) {
    if (_computePlotRangeDynamically) {
      setWorldLimitsY(minVal - rangeY * 0.05, maxVal + rangeY * 0.125);
    } else {
      setWorldLimitsY(_staticRange.min_val, _staticRange.max_val);
    }
  }
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the spectral zdr
  
  FilterUtils::applyMedianFilter(zdr, nSamples, _medianFiltLen);
  painter.save();
  QPen pen(painter.pen());
  pen.setColor(_params.iqplot_line_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(_params.iqplot_line_width);
  painter.setPen(pen);
  QVector<QPointF> pts;
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = (ii + nSamples) % nSamples;
    double val = zdr[ii];
    QPointF pt(jj, val);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // legends
  
  char text[1024];
  vector<string> legendsLeft;
  if (_useAdaptFilt) {
    snprintf(text, 1024, "Adapt filt, width: %.2f", _clutWidthMps);
    legendsLeft.push_back(text);
  } else if (_useRegrFilt) {
    snprintf(text, 1024, "Regr filt, order: %d", beam->getRegrOrder());
    legendsLeft.push_back(text);
  }
  if (_medianFiltLen > 1) {
    snprintf(text, 1024, "Median filt len: %d", _medianFiltLen);
    legendsLeft.push_back(text);
  }
  _zoomWorld.drawLegendsTopLeft(painter, legendsLeft);

  // draw the title
  
  painter.save();
  painter.setPen(_params.iqplot_title_color);
  _zoomWorld.drawTitleTopCenter(painter, "SPECTRAL ZDR (H-V)");
  painter.restore();

}

/*************************************************************************
 * plot the spectral phidp
 */

void IqPlot::_plotSpectralPhidp(QPainter &painter,
                                Beam *beam,
                                int nSamples,
                                double selectedRangeKm,
                                int gateNum,
                                const RadarComplex_t *iqHc,
                                const RadarComplex_t *iqVc)
  
{

  // apply window to time series
  
  TaArray<RadarComplex_t> iqWindowedHc_, iqWindowedVc_;
  RadarComplex_t *iqWindowedHc = iqWindowedHc_.alloc(nSamples);
  RadarComplex_t *iqWindowedVc = iqWindowedVc_.alloc(nSamples);
  _applyWindow(iqHc, iqWindowedHc, nSamples);
  _applyWindow(iqVc, iqWindowedVc, nSamples);
  
  // compute spectra
  
  TaArray<RadarComplex_t> specHc_, specVc_;
  RadarComplex_t *specHc = specHc_.alloc(nSamples);
  RadarComplex_t *specVc = specVc_.alloc(nSamples);
  RadarFft fft(nSamples);
  fft.fwd(iqWindowedHc, specHc);
  fft.shift(specHc);
  fft.fwd(iqWindowedVc, specVc);
  fft.shift(specVc);
  
  // compute phidp spectrum
  
  TaArray<double> phidp_;
  double minVal = 9999.0, maxVal = -9999.0;
  double *phidp = phidp_.alloc(nSamples);
  for (int ii = 0; ii < nSamples; ii++) {
    RadarComplex_t diff = RadarComplex::conjugateProduct(specHc[ii], specVc[ii]);
    phidp[ii] = RadarComplex::argDeg(diff);
    minVal = min(phidp[ii], minVal);
    maxVal = max(phidp[ii], maxVal);
  }

  // set the Y axis range
  
  double rangeY = maxVal - minVal;
  if (!_isZoomed) {
    if (_computePlotRangeDynamically) {
      setWorldLimitsY(minVal - rangeY * 0.05, maxVal + rangeY * 0.125);
    } else {
      setWorldLimitsY(_staticRange.min_val, _staticRange.max_val);
    }
  }
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the spectrum phidp
  
  FilterUtils::applyMedianFilter(phidp, nSamples, _medianFiltLen);
  painter.save();
  QPen pen(painter.pen());
  pen.setColor(_params.iqplot_line_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(_params.iqplot_line_width);
  painter.setPen(pen);
  QVector<QPointF> pts;
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = (ii + nSamples) % nSamples;
    double val = phidp[ii];
    QPointF pt(jj, val);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // legends

  const MomentsFields* fields = beam->getOutFields();
  char text[1024];
  vector<string> legendsLeft;
  snprintf(text, 1024, "phidp: %.2f", fields[gateNum].phidp);
  legendsLeft.push_back(text);
  if (_medianFiltLen > 1) {
    snprintf(text, 1024, "Median filt len: %d", _medianFiltLen);
    legendsLeft.push_back(text);
  }
  _zoomWorld.drawLegendsTopLeft(painter, legendsLeft);
  
  // draw the title
  
  painter.save();
  painter.setPen(_params.iqplot_title_color);
  _zoomWorld.drawTitleTopCenter(painter, "SPECTRAL PHIDP (H-V)");
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
    if (_computePlotRangeDynamically) {
      setWorldLimitsY(minDbm - rangeY * 0.05, maxDbm + rangeY * 0.125);
    } else {
      setWorldLimitsY(_staticRange.min_val, _staticRange.max_val);
    }
  }
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the spectrum

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
    if (_computePlotRangeDynamically) {
      setWorldLimitsY(minVal - rangeY * 0.05, maxVal + rangeY * 0.125);
    } else {
      setWorldLimitsY(_staticRange.min_val, _staticRange.max_val);
    }
  }
  
  // draw the overlays
  
  _drawOverlays(painter, selectedRangeKm);
  
  // draw the time series phase

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
                         const double *iVals,
                         const double *qVals)
  
{

  // prepare Forsythe compute object for regression
  
  vector<double> xVals;
  for (int ii = 0; ii < nSamples; ii++) {
    xVals.push_back(ii);
  }
  ForsytheFit forsythe;
  forsythe.prepareForFit(beam->getRegrOrder(), xVals);
  
  // compute polynomial regression fit
  
  vector<double> iRaw;
  for (int ii = 0; ii < nSamples; ii++) {
    iRaw.push_back(iVals[ii]);
  }
  forsythe.performFit(iRaw);
  vector<double> iSmooth = forsythe.getYEstVector();
  vector<double> iResid;
  for (int ii = 0; ii < nSamples; ii++) {
    iResid.push_back(iRaw[ii] - iSmooth[ii]);
  }

  vector<double> qRaw;
  for (int ii = 0; ii < nSamples; ii++) {
    qRaw.push_back(qVals[ii]);
  }
  forsythe.performFit(qRaw);
  vector<double> qSmooth = forsythe.getYEstVector();
  vector<double> qResid;
  for (int ii = 0; ii < nSamples; ii++) {
    qResid.push_back(qRaw[ii] - qSmooth[ii]);
  }

  // compute CSR

  double sumPowerUnfilt = 0.0;
  double sumPowerFilt = 0.0;
  for (int ii = 0; ii < nSamples; ii++) {
    sumPowerUnfilt += iVals[ii] * iVals[ii] + qVals[ii] * qVals[ii];
    sumPowerFilt += iResid[ii] * iResid[ii] + qResid[ii] * qResid[ii];
  }
  double meanPowerUnfilt = sumPowerUnfilt / nSamples;
  double meanPowerFilt = sumPowerFilt / nSamples;
  double dbmUnfilt = 10.0 * log10(meanPowerUnfilt);
  double dbmFilt = 10.0 * log10(meanPowerFilt);
  double clutPower = meanPowerUnfilt - meanPowerFilt;
  double csrDb = 0.0;
  if (clutPower > 0) {
    csrDb = 10.0 * log10(clutPower / meanPowerFilt);
  }

  // compute min and max vals for plot
  
  double minVal = 9999.0;
  double maxVal = -9999.0;
  for (int ii = 0; ii < nSamples; ii++) {
    minVal = min(iVals[ii], minVal);
    minVal = min(iSmooth[ii], minVal);
    minVal = min(iResid[ii], minVal);
    maxVal = max(iVals[ii], maxVal);
    maxVal = max(iSmooth[ii], maxVal);
    maxVal = max(iResid[ii], maxVal);
    minVal = min(qVals[ii], minVal);
    minVal = min(qSmooth[ii], minVal);
    minVal = min(qResid[ii], minVal);
    maxVal = max(qVals[ii], maxVal);
    maxVal = max(qSmooth[ii], maxVal);
    maxVal = max(qResid[ii], maxVal);
  }
  
  // set the Y axis range
  
  double rangeY = maxVal - minVal;
  if (!_isZoomed) {
    if (_computePlotRangeDynamically) {
      setWorldLimitsY(minVal - rangeY * 0.05, maxVal + rangeY * 0.125);
    } else {
      setWorldLimitsY(_staticRange.min_val, _staticRange.max_val);
    }
  }
  
  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // draw the data

  const double *vals = iVals;
  double *resid = iResid.data();
  double *smooth = iSmooth.data();
  if (_plotType == Params::Q_VALS) {
    vals = qVals;
    resid = qResid.data();
    smooth = qSmooth.data();
  }
  
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

  if (_useRegrFilt) {
    
    // plot the polynomial fit
    
    painter.save();
    QPen pen(painter.pen());
    pen.setColor(_params.iqplot_polynomial_fit_color);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(_params.iqplot_polynomial_line_width);
    painter.setPen(pen);
    QVector<QPointF> pts;
    for (int ii = 0; ii < nSamples; ii++) {
      QPointF pt(ii, smooth[ii]);
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
      QPointF pt(ii, resid[ii]);
      pts.push_back(pt);
    }
    _zoomWorld.drawLines(painter, pts);
    painter.restore();

    // Legend
    
    char text[1024];
    vector<string> legends;
    snprintf(text, 1024, "Regr-order: %d", beam->getRegrOrder());
    legends.push_back(text);
    snprintf(text, 1024, "PwrUnfilt(dBm): %.2f", dbmUnfilt);
    legends.push_back(text);
    snprintf(text, 1024, "PwrFilt(dBm): %.2f", dbmFilt);
    legends.push_back(text);
    snprintf(text, 1024, "CSR(dB): %.2f", csrDb);
    legends.push_back(text);
    _zoomWorld.drawLegendsTopLeft(painter, legends);

  } // if (_useRegrFilt)

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
    if (_computePlotRangeDynamically) {
      setWorldLimits(minIVal - rangeX * 0.05,
                     minQVal - rangeY * 0.05,
                     maxIVal + rangeX * 0.05,
                     maxQVal + rangeY * 0.125);
    } else {
      setWorldLimits(_staticRange.min_val,
                     _staticRange.min_val,
                     _staticRange.max_val,
                     _staticRange.max_val);
    }
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
    if (_computePlotRangeDynamically) {
      setWorldLimits(minISum - rangeX * 0.05,
                     minQSum - rangeY * 0.05,
                     maxISum + rangeX * 0.05,
                     maxQSum + rangeY * 0.125);
    } else {
      setWorldLimits(_staticRange.min_val,
                     _staticRange.min_val,
                     _staticRange.max_val,
                     _staticRange.max_val);
    }
  }
  
  // draw the overlays
  
  _drawOverlays(painter, selectedRangeKm);

  // draw the phasor data

  painter.save();
  QPen pen(painter.pen());
  pen.setColor(_params.iqplot_line_color);
  pen.setStyle(Qt::DashLine);
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

  QPen pointPen(painter.pen());
  pointPen.setColor(_params.iqplot_line_color);
  pointPen.setStyle(Qt::SolidLine);
  pen.setCapStyle(Qt::RoundCap);
  pointPen.setWidth(_params.iqplot_line_width * 2);
  painter.setPen(pointPen);
  _zoomWorld.drawPoints(painter, iqpts);

  painter.restore();

  // legends

  double cpa = RadarMoments::computeCpa(iq, nSamples);
  double cpaAlt = RadarMoments::computeCpaAlt(iq, nSamples);
  vector<string> legends;
  char text[1024];
  snprintf(text, 1024, "CPA: %.2f", cpa);
  legends.push_back(text);
  snprintf(text, 1024, "CPA-alt: %.2f", cpaAlt);
  legends.push_back(text);
  _zoomWorld.drawLegendsTopLeft(painter, legends);
  
  // draw the title

  painter.save();
  painter.setPen(_params.iqplot_title_color);
  _zoomWorld.drawTitleTopCenter(painter, getName());
  painter.restore();

}

/*************************************************************************
 * compute the spectrum
 */

void IqPlot::_computePowerSpectrum(Beam *beam,
                                   int nSamples,
                                   const RadarComplex_t *iq,
                                   double *power,
                                   double *dbm)
  
{

  // init
  
  for (int ii = 0; ii < nSamples; ii++) {
    power[ii] = 1.0e-12;
    dbm[ii] = -120.0;
  }

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
  
  // set up filtering
  
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
  moments.setClutterWidthMps(_clutWidthMps);
  moments.setClutterInitNotchWidthMps(3.0);
  
  // filter as appropriate
  
  if (_useAdaptFilt) {

    // adaptive filtering takes precedence over regression
    
    TaArray<RadarComplex_t> filtAdaptWindowed_;
    RadarComplex_t *filtAdaptWindowed = filtAdaptWindowed_.alloc(nSamples);
    double filterRatio, spectralNoise, spectralSnr;
    moments.applyAdaptiveFilter(nSamples, beam->getPrt(), fft,
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
      power[ii] = RadarComplex::power(filtAdaptSpec[ii]);
    }
    
  } else if (_useRegrFilt) {

    // regression
    
    RegressionFilter regrF;
    if (beam->getIsStagPrt()) {
      regrF.setupStaggered(nSamples, beam->getStagM(),
                           beam->getStagN(), _regrOrder);
    } else {
      regrF.setup(nSamples, _regrOrder);
    }
    
    TaArray<RadarComplex_t> filtered_;
    RadarComplex_t *filtered = filtered_.alloc(nSamples);
    double filterRatio, spectralNoise, spectralSnr;
    moments.applyRegressionFilter(nSamples, beam->getPrt(), fft,
                                  regrF, _windowCoeff,
                                  iq,
                                  calibNoise,
                                  true,
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
      power[ii] = RadarComplex::power(filtRegrSpec[ii]);
    }
    
  } else {

    // no filtering
    
    for (int ii = 0; ii < nSamples; ii++) {
      power[ii] = RadarComplex::power(powerSpec[ii]);
    }
    
  }

  // compute dbm
  
  for (int ii = 0; ii < nSamples; ii++) {
    if (power[ii] <= 1.0e-12) {
      dbm[ii] = -120.0;
    } else {
      dbm[ii] = 10.0 * log10(power[ii]);
    }
  }

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
    case Params::SPECTRAL_POWER:
      ptypeStr =  "SPECTRAL_POWER";
      break;
    case Params::SPECTRAL_PHASE:
      ptypeStr =  "SPECTRAL_PHASE";
      break;
    case Params::SPECTRAL_ZDR:
      ptypeStr =  "SPECTRAL_ZDR";
      break;
    case Params::SPECTRAL_PHIDP:
      ptypeStr =  "SPECTRAL_PHIDP";
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
    case Params::SPECTRAL_POWER:
    case Params::SPECTRAL_PHASE:
    case Params::SPECTRAL_ZDR:
    case Params::SPECTRAL_PHIDP:
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
    case Params::SPECTRAL_POWER:
      return "dbm";
    case Params::SPECTRAL_PHASE:
      return "deg";
    case Params::SPECTRAL_ZDR:
      return "dB";
    case Params::SPECTRAL_PHIDP:
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

