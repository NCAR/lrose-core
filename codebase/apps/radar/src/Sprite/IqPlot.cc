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
#include <rapmath/ForsytheFit.hh>
#include <radar/ForsytheRegrFilter.hh>
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
#include "SpriteMgr.hh"
#include "Beam.hh"

using namespace std;

IqPlot::IqPlot(QWidget* parent,
               const Params &params,
               int id) :
        SpritePlot(parent, params, id),
        _params(params)
        
{
  _plotType = Params::SPECTRAL_POWER;
  _rxChannel = Params::CHANNEL_HC;
  _fftWindow = Params::FFT_WINDOW_VONHANN;
  _clutterFilterType = RadarMoments::CLUTTER_FILTER_NONE;
  _plotClutModel = false;
  _clutModelWidthMps = 0.75;
  _regrOrder = _params.regression_filter_specified_polynomial_order;
  _regrOrderInUse = _regrOrder;
  _regrClutWidthFactor = _params.regression_filter_clutter_width_factor;
  _regrCnrExponent = _params.regression_filter_cnr_exponent;
  _regrNotchInterpMethod = RadarMoments::INTERP_METHOD_GAUSSIAN;
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
                      size_t nSamples,
                      double selectedRangeKm)
  
{

  if (beam == NULL) {
    cerr << "WARNING - IqPlot::plotBeam() - got NULL beam, ignoring"
         << endl;
    return;
  }

  _beam = beam;
  _nSamples = nSamples;
  
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
  RadarComplex_t *iq = iq_.alloc(_nSamples);
  switch (_rxChannel) {
    case Params::CHANNEL_HC:
      memcpy(iq, gateData->iqhcOrig, _nSamples * sizeof(RadarComplex_t));
      break;
    case Params::CHANNEL_VC:
      memcpy(iq, gateData->iqvcOrig, _nSamples * sizeof(RadarComplex_t));
      break;
    case Params::CHANNEL_HX:
      memcpy(iq, gateData->iqhxOrig, _nSamples * sizeof(RadarComplex_t));
      break;
    case Params::CHANNEL_VX:
      memcpy(iq, gateData->iqvxOrig, _nSamples * sizeof(RadarComplex_t));
  }
  
  // perform the relevant plot

  switch (_plotType) {
    case Params::I_VALS:
    case Params::Q_VALS: 
      {
        TaArray<double> iVals_;
        double *iVals = iVals_.alloc(_nSamples);
        for (size_t ii = 0; ii < _nSamples; ii++) {
          iVals[ii] = iq[ii].re;
        }
        TaArray<double> qVals_;
        double *qVals = qVals_.alloc(_nSamples);
        for (size_t ii = 0; ii < _nSamples; ii++) {
          qVals[ii] = iq[ii].im;
        }
        _plotIQVals(painter,  selectedRangeKm,
                    gateNum, iVals, qVals);
      }
      break;
    case Params::I_VS_Q:
      _plotIvsQ(painter, selectedRangeKm, gateNum, iq);
      break;
    case Params::PHASOR:
      _plotPhasor(painter, selectedRangeKm, gateNum, iq);
      break;
    case Params::SPECTRAL_PHASE:
      _plotSpectralPhase(painter, selectedRangeKm, gateNum, iq);
      break;
    case Params::SPECTRAL_ZDR:
      _plotSpectralZdr(painter, selectedRangeKm, gateNum,
                       gateData->iqhcOrig, gateData->iqvcOrig);
      break;
    case Params::SPECTRAL_PHIDP:
      _plotSpectralPhidp(painter, selectedRangeKm, gateNum,
                         gateData->iqhcOrig, gateData->iqvcOrig);
      break;
    case Params::TS_POWER:
      _plotTsPower(painter, selectedRangeKm, gateNum, iq);
      break;
    case Params::TS_PHASE:
      _plotTsPhase(painter, selectedRangeKm, gateNum, iq);
      break;
    case Params::SPECTRAL_POWER:
    default:
      _plotSpectralPower(painter, selectedRangeKm, gateNum, iq);
  }

}

/*************************************************************************
 * plot the power spectrum
 */

void IqPlot::_plotSpectralPower(QPainter &painter,
                                double selectedRangeKm,
                                int gateNum,
                                const RadarComplex_t *iq)
  
{

  // compute the power spectrum
  // applying the appropriate clutter filter
  
  TaArray<RadarComplex_t> iqWindowed_, iqFilt_, iqNotched_;
  TaArray<double> dbm_, dbmFilt_;

  RadarComplex_t *iqWindowed = iqWindowed_.alloc(_nSamples);
  RadarComplex_t *iqFilt = iqFilt_.alloc(_nSamples);
  RadarComplex_t *iqNotched = iqNotched_.alloc(_nSamples);

  double *dbm = dbm_.alloc(_nSamples);
  double *dbmFilt = dbmFilt_.alloc(_nSamples);
  
  double filterRatio, spectralNoise, spectralSnr;
  
  _computePowerSpectrum(iq, iqWindowed, iqFilt, iqNotched,
                        dbm, dbmFilt,
                        filterRatio, spectralNoise, spectralSnr);

  // compute power limits
  
  double minDbm = 9999.0, maxDbm = -9999.0;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    minDbm = min(dbm[ii], minDbm);
    minDbm = min(dbmFilt[ii], minDbm);
    maxDbm = max(dbm[ii], maxDbm);
    maxDbm = max(dbmFilt[ii], maxDbm);
  }

  // set the Y axis range

  double rangeY = maxDbm - minDbm;
  double minY = minDbm - rangeY * 0.05;
  double maxY = maxDbm + rangeY * 0.25;
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

  FilterUtils::applyMedianFilter(dbm, _nSamples, _medianFiltLen);
  {
    painter.save();
    QPen pen(painter.pen());
    pen.setColor(_params.iqplot_line_color);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(_params.iqplot_line_width);
    painter.setPen(pen);
    QVector<QPointF> pts;
    for (size_t ii = 0; ii < _nSamples; ii++) {
      double val = dbm[ii];
      QPointF pt(ii, val);
      pts.push_back(pt);
    }
    _zoomWorld.drawLines(painter, pts);
    painter.restore();
  }

  // apply median filter to the spectrum if needed

  if (_medianFiltLen > 1) {
    FilterUtils::applyMedianFilter(dbmFilt, _nSamples, _medianFiltLen);
  }

  // draw filtered spectrum
  
  {
    painter.save();
    QPen pen(painter.pen());
    if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_ADAPTIVE) {
      pen.setColor(_params.iqplot_adaptive_filtered_color);
    } else {
      pen.setColor(_params.iqplot_regression_filtered_color);
    }
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(_params.iqplot_line_width);
    painter.setPen(pen);
    QVector<QPointF> filtPts;
    for (size_t ii = 0; ii < _nSamples; ii++) {
      QPointF pt(ii, dbmFilt[ii]);
      filtPts.push_back(pt);
    }
    _zoomWorld.drawLines(painter, filtPts);
    painter.restore();
  }

  // plot clutter model

  if (_plotClutModel) {

    ClutFilter clut;
    TaArray<double> clutModel_;
    double *clutModel = clutModel_.alloc(_nSamples);
    TaArray<double> powerReal_;
    double *powerReal = powerReal_.alloc(_nSamples);
    RadarComplex::loadPower(iq, powerReal, _nSamples);
    
    if (clut.computeGaussianClutterModel(powerReal,
                                         _nSamples,
                                         _clutModelWidthMps,
                                         _beam->getNyquist(),
                                         clutModel, true) == 0) {
      
      painter.save();
      QPen pen = painter.pen();
      pen.setColor(_params.iqplot_clutter_model_color);
      pen.setStyle(Qt::DotLine);
      pen.setWidth(2);
      painter.setPen(pen);
      QVector<QPointF> clutPts;
      for (size_t ii = 0; ii < _nSamples; ii++) {
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
  
  char text[1024];

  vector<string> legendsLeft;
  legendsLeft.push_back(getFftWindowName(_fftWindow) + " window");
  if (_medianFiltLen > 1) {
    snprintf(text, 1024, "Median filt len: %d", _medianFiltLen);
    legendsLeft.push_back(text);
  }
  if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_ADAPTIVE) {
    snprintf(text, 1024, "Adaptive filter");
    legendsLeft.push_back(text);
  } else if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_REGRESSION) {
    snprintf(text, 1024, "Regression filter");
    legendsLeft.push_back(text);
  } else if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_NOTCH) {
    snprintf(text, 1024, "Notch filter");
    legendsLeft.push_back(text);
  }
  if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_REGRESSION) {
    snprintf(text, 1024, "Regr-order: %d", _regrOrderInUse);
    legendsLeft.push_back(text);
  }
  if (_plotClutModel) {
    snprintf(text, 1024, "Clut-model-width: %g", _clutModelWidthMps);
    legendsLeft.push_back(text);
  }
  if (_legendsOn) {
    _zoomWorld.drawLegendsTopLeft(painter, legendsLeft);
  }

  vector<string> legendsRight;
  double unfiltPower = RadarComplex::meanPower(iq, _nSamples);
  double unfiltPowerDbm = 10.0 * log10(unfiltPower);
  snprintf(text, 1024, "UnfiltPower (dBm): %.2f", unfiltPowerDbm);
  legendsRight.push_back(text);
  if (_clutterFilterType != RadarMoments::CLUTTER_FILTER_NONE) {
    double filtPower = RadarComplex::meanPower(iqFilt, _nSamples);
    double filtPowerDbm = 10.0 * log10(filtPower);
    snprintf(text, 1024, "FiltPower: %.2f", filtPowerDbm);
    legendsRight.push_back(text);
    double clutPower = unfiltPower - filtPower;
    if (clutPower > 0.0) {
      double clutPowerDbm = 10.0 * log10(clutPower);
      snprintf(text, 1024, "ClutPower: %.2f", clutPowerDbm);
      legendsRight.push_back(text);
      snprintf(text, 1024, "CSR: %.2f", clutPowerDbm - filtPowerDbm);
      legendsRight.push_back(text);
    }
    // snprintf(text, 1024, "SpecSNR: %.2f", spectralSnr);
    // legendsRight.push_back(text);
  }
  
  if (_legendsOn) {
    _zoomWorld.drawLegendsTopRight(painter, legendsRight);
  }

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
                                double selectedRangeKm,
                                int gateNum,
                                const RadarComplex_t *iq)
  
{

  // apply window to time series
  
  TaArray<RadarComplex_t> iqWindowed_;
  RadarComplex_t *iqWindowed = iqWindowed_.alloc(_nSamples);
  _applyWindow(iq, iqWindowed, _nSamples);
  
  // compute spectrum
  
  TaArray<RadarComplex_t> spec_;
  RadarComplex_t *spec = spec_.alloc(_nSamples);
  RadarFft fft(_nSamples);
  fft.fwd(iqWindowed, spec);
  fft.shift(spec);
  
  // compute phase

  TaArray<double> phase_;
  double *phase = phase_.alloc(_nSamples);
  double minVal = 9999.0, maxVal = -9999.0;
  for (size_t ii = 0; ii < _nSamples; ii++) {
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
  for (size_t ii = 0; ii < _nSamples; ii++) {
    int jj = (ii + _nSamples) % _nSamples;
    double val = phase[ii];
    QPointF pt(jj, val);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // legends

  const MomentsFields* fields = _beam->getOutFields();
  double vel = fields[gateNum].vel;
  
  char text[1024];
  vector<string> legendsLeft;
  snprintf(text, 1024, "Vel: %.2f", vel);
  legendsLeft.push_back(text);
  if (_legendsOn) {
    _zoomWorld.drawLegendsTopLeft(painter, legendsLeft);
  }
  
  vector<string> legendsRight;
  legendsRight.push_back(getFftWindowName(_fftWindow));
  if (_legendsOn) {
    _zoomWorld.drawLegendsTopRight(painter, legendsRight);
  }
  
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
                              double selectedRangeKm,
                              int gateNum,
                              const RadarComplex_t *iqHc,
                              const RadarComplex_t *iqVc)
  
{

  // allocate variables
  
  TaArray<RadarComplex_t> iqWindowedHc_, iqWindowedVc_;
  TaArray<RadarComplex_t> iqFiltHc_, iqFiltVc_;
  TaArray<RadarComplex_t> iqNotchedHc_, iqNotchedVc_;
  TaArray<double> dbmHc_, dbmVc_;
  TaArray<double> dbmFiltHc_, dbmFiltVc_;

  RadarComplex_t *iqWindowedHc = iqWindowedHc_.alloc(_nSamples);
  RadarComplex_t *iqWindowedVc = iqWindowedVc_.alloc(_nSamples);
  RadarComplex_t *iqFiltHc = iqFiltHc_.alloc(_nSamples);
  RadarComplex_t *iqFiltVc = iqFiltVc_.alloc(_nSamples);
  RadarComplex_t *iqNotchedHc = iqNotchedHc_.alloc(_nSamples);
  RadarComplex_t *iqNotchedVc = iqNotchedVc_.alloc(_nSamples);

  double *dbmHc = dbmHc_.alloc(_nSamples);
  double *dbmVc = dbmVc_.alloc(_nSamples);
  double *dbmFiltHc = dbmFiltHc_.alloc(_nSamples);
  double *dbmFiltVc = dbmFiltVc_.alloc(_nSamples);

  double filterRatioHc, filterRatioVc;
  double spectralNoiseHc, spectralNoiseVc;
  double spectralSnrHc, spectralSnrVc;
  
  // compute Hc and Vc spectra
  
  _computePowerSpectrum(iqHc, iqWindowedHc, iqFiltHc, iqNotchedHc,
                        dbmHc, dbmFiltHc,
                        filterRatioHc, spectralNoiseHc, spectralSnrHc);
  
  _computePowerSpectrum(iqVc, iqWindowedVc, iqFiltVc, iqNotchedVc,
                        dbmVc, dbmFiltVc,
                        filterRatioVc, spectralNoiseVc, spectralSnrVc);
  
  // compute ZDR spectrum
  
  TaArray<double> zdr_;
  double minVal = 9999.0, maxVal = -9999.0;
  double *zdr = zdr_.alloc(_nSamples);
  for (size_t ii = 0; ii < _nSamples; ii++) {
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
  
  FilterUtils::applyMedianFilter(zdr, _nSamples, _medianFiltLen);
  painter.save();
  QPen pen(painter.pen());
  pen.setColor(_params.iqplot_line_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(_params.iqplot_line_width);
  painter.setPen(pen);
  QVector<QPointF> pts;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    int jj = (ii + _nSamples) % _nSamples;
    double val = zdr[ii];
    QPointF pt(jj, val);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // legends
  
  char text[1024];
  vector<string> legendsLeft;
  if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_ADAPTIVE) {
    snprintf(text, 1024, "Adapt filt, clut width: %.2f", _clutModelWidthMps);
    legendsLeft.push_back(text);
  } else if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_REGRESSION) {
    snprintf(text, 1024, "Regr filt, order: %d", _beam->getRegrOrder());
    legendsLeft.push_back(text);
  }
  if (_medianFiltLen > 1) {
    snprintf(text, 1024, "Median filt len: %d", _medianFiltLen);
    legendsLeft.push_back(text);
  }
  if (_legendsOn) {
    _zoomWorld.drawLegendsTopLeft(painter, legendsLeft);
  }

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
                                double selectedRangeKm,
                                int gateNum,
                                const RadarComplex_t *iqHc,
                                const RadarComplex_t *iqVc)
  
{

  // apply window to time series
  
  TaArray<RadarComplex_t> iqWindowedHc_, iqWindowedVc_;
  RadarComplex_t *iqWindowedHc = iqWindowedHc_.alloc(_nSamples);
  RadarComplex_t *iqWindowedVc = iqWindowedVc_.alloc(_nSamples);
  _applyWindow(iqHc, iqWindowedHc, _nSamples);
  _applyWindow(iqVc, iqWindowedVc, _nSamples);
  
  // compute spectra
  
  TaArray<RadarComplex_t> specHc_, specVc_;
  RadarComplex_t *specHc = specHc_.alloc(_nSamples);
  RadarComplex_t *specVc = specVc_.alloc(_nSamples);
  RadarFft fft(_nSamples);
  fft.fwd(iqWindowedHc, specHc);
  fft.shift(specHc);
  fft.fwd(iqWindowedVc, specVc);
  fft.shift(specVc);
  
  // compute phidp spectrum
  
  TaArray<double> phidp_;
  double minVal = 9999.0, maxVal = -9999.0;
  double *phidp = phidp_.alloc(_nSamples);
  for (size_t ii = 0; ii < _nSamples; ii++) {
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
  
  FilterUtils::applyMedianFilter(phidp, _nSamples, _medianFiltLen);
  painter.save();
  QPen pen(painter.pen());
  pen.setColor(_params.iqplot_line_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(_params.iqplot_line_width);
  painter.setPen(pen);
  QVector<QPointF> pts;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    int jj = (ii + _nSamples) % _nSamples;
    double val = phidp[ii];
    QPointF pt(jj, val);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // legends

  const MomentsFields* fields = _beam->getOutFields();
  char text[1024];
  vector<string> legendsLeft;
  snprintf(text, 1024, "phidp: %.2f", fields[gateNum].phidp);
  legendsLeft.push_back(text);
  if (_medianFiltLen > 1) {
    snprintf(text, 1024, "Median filt len: %d", _medianFiltLen);
    legendsLeft.push_back(text);
  }
  if (_legendsOn) {
    _zoomWorld.drawLegendsTopLeft(painter, legendsLeft);
  }
  
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
                          double selectedRangeKm,
                          int gateNum,
                          const RadarComplex_t *iq)
  
{

  // compute power, plus min and max
  
  TaArray<double> powerDbm_;
  double *powerDbm = powerDbm_.alloc(_nSamples);
  double minDbm = 9999.0, maxDbm = -9999.0;
  for (size_t ii = 0; ii < _nSamples; ii++) {
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
  for (size_t ii = 0; ii < _nSamples; ii++) {
    int jj = (ii + _nSamples) % _nSamples;
    double val = powerDbm[ii];
    QPointF pt(jj, val);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // legends

  double power = RadarComplex::meanPower(iq, _nSamples);
  double dbm = 10.0 * log10(power);
  char text[1024];
  snprintf(text, 1024, "DbmMean: %.2f", dbm);
  vector<string> legends;
  legends.push_back(text);
  if (_legendsOn) {
    _zoomWorld.drawLegendsTopLeft(painter, legends);
  }
  
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
                          double selectedRangeKm,
                          int gateNum,
                          const RadarComplex_t *iq)
  
{

  // compute time series phases
  
  TaArray<double> phase_;
  double *phase = phase_.alloc(_nSamples);
  double minVal = 9999.0, maxVal = -9999.0;
  for (size_t ii = 0; ii < _nSamples; ii++) {
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
  for (size_t ii = 0; ii < _nSamples; ii++) {
    int jj = (ii + _nSamples) % _nSamples;
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
                         double selectedRangeKm,
                         int gateNum,
                         const double *iVals,
                         const double *qVals)
  
{

  // prepare Forsythe compute object for regression
  
  vector<double> xVals;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    xVals.push_back(ii);
  }
  ForsytheFit forsythe;
  forsythe.prepareForFit(_beam->getRegrOrder(), xVals);
  
  // compute polynomial regression fit
  
  vector<double> iRaw;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    iRaw.push_back(iVals[ii]);
  }
  forsythe.performFit(iRaw);
  vector<double> iSmooth = forsythe.getYEstVector();
  vector<double> iResid;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    iResid.push_back(iRaw[ii] - iSmooth[ii]);
  }

  vector<double> qRaw;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    qRaw.push_back(qVals[ii]);
  }
  forsythe.performFit(qRaw);
  vector<double> qSmooth = forsythe.getYEstVector();
  vector<double> qResid;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    qResid.push_back(qRaw[ii] - qSmooth[ii]);
  }

  // compute CSR

  double sumPowerUnfilt = 0.0;
  double sumPowerFilt = 0.0;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    sumPowerUnfilt += iVals[ii] * iVals[ii] + qVals[ii] * qVals[ii];
    sumPowerFilt += iResid[ii] * iResid[ii] + qResid[ii] * qResid[ii];
  }
  double meanPowerUnfilt = sumPowerUnfilt / _nSamples;
  double meanPowerFilt = sumPowerFilt / _nSamples;
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
  for (size_t ii = 0; ii < _nSamples; ii++) {
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
  for (size_t ii = 0; ii < _nSamples; ii++) {
    QPointF pt(ii, vals[ii]);
    pts.push_back(pt);
  }
  _zoomWorld.drawLines(painter, pts);
  painter.restore();
  
  // plot regression filter as appropriate

  if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_REGRESSION) {
    
    // plot the polynomial fit
    
    painter.save();
    QPen pen(painter.pen());
    pen.setColor(_params.iqplot_polynomial_fit_color);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(_params.iqplot_polynomial_line_width);
    painter.setPen(pen);
    QVector<QPointF> pts;
    for (size_t ii = 0; ii < _nSamples; ii++) {
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
    for (size_t ii = 0; ii < _nSamples; ii++) {
      QPointF pt(ii, resid[ii]);
      pts.push_back(pt);
    }
    _zoomWorld.drawLines(painter, pts);
    painter.restore();

    // Legend
    
    char text[1024];
    vector<string> legends;
    snprintf(text, 1024, "Regr-order: %d", _regrOrderInUse);
    legends.push_back(text);
    snprintf(text, 1024, "PwrUnfilt(dBm): %.2f", dbmUnfilt);
    legends.push_back(text);
    snprintf(text, 1024, "PwrFilt(dBm): %.2f", dbmFilt);
    legends.push_back(text);
    snprintf(text, 1024, "CSR(dB): %.2f", csrDb);
    legends.push_back(text);
    if (_legendsOn) {
      _zoomWorld.drawLegendsTopLeft(painter, legends);
    }

  } // if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_REGRESSION)

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
                       double selectedRangeKm,
                       int gateNum,
                       const RadarComplex_t *iq)
  
{

  // get I and Q vals

  TaArray<double> iVals_, qVals_;
  double *iVals = iVals_.alloc(_nSamples);
  double *qVals = qVals_.alloc(_nSamples);
  double minIVal = 9999.0;
  double maxIVal = -9999.0;
  double minQVal = 9999.0;
  double maxQVal = -9999.0;
  for (size_t ii = 0; ii < _nSamples; ii++) {
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
  for (size_t ii = 0; ii < _nSamples; ii++) {
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
                         double selectedRangeKm,
                         int gateNum,
                         const RadarComplex_t *iq)

{
                           
  // get I and Q vals

  TaArray<double> iSums_, qSums_;
  double *iSums = iSums_.alloc(_nSamples);
  double *qSums = qSums_.alloc(_nSamples);
  double minISum = 9999.0;
  double maxISum = -9999.0;
  double minQSum = 9999.0;
  double maxQSum = -9999.0;
  double iSum = 0.0;
  double qSum = 0.0;
  for (size_t ii = 0; ii < _nSamples; ii++) {
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
  for (size_t ii = 0; ii < _nSamples; ii++) {
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

  double cpa = RadarMoments::computeCpa(iq, _nSamples);
  double cpaAlt = RadarMoments::computeCpaAlt(iq, _nSamples);
  vector<string> legends;
  char text[1024];
  snprintf(text, 1024, "CPA: %.2f", cpa);
  legends.push_back(text);
  snprintf(text, 1024, "CPA-alt: %.2f", cpaAlt);
  legends.push_back(text);
  if (_legendsOn) {
    _zoomWorld.drawLegendsTopLeft(painter, legends);
  }
  
  // draw the title

  painter.save();
  painter.setPen(_params.iqplot_title_color);
  _zoomWorld.drawTitleTopCenter(painter, getName());
  painter.restore();

}

/*************************************************************************
 * compute the spectrum
 */

void IqPlot::_computePowerSpectrum(const RadarComplex_t *iq,
                                   RadarComplex_t *iqWindowed,
                                   RadarComplex_t *iqFilt,
                                   RadarComplex_t *iqNotched,
                                   double *dbm,
                                   double *dbmFilt,
                                   double &filterRatio,
                                   double &spectralNoise,
                                   double &spectralSnr)
  
{

  // init
  
  for (size_t ii = 0; ii < _nSamples; ii++) {
    dbm[ii] = -120.0;
    dbmFilt[ii] = -120.0;
  }

  // apply window to time series
  
  _applyWindow(iq, iqWindowed, _nSamples);
  
  // compute power spectrum
  
  TaArray<RadarComplex_t> powerSpec_;
  RadarComplex_t *powerSpec = powerSpec_.alloc(_nSamples);
  RadarFft fft(_nSamples);
  fft.fwd(iqWindowed, powerSpec);
  fft.shift(powerSpec);
  
  // compute infiltered power
  
  for (size_t ii = 0; ii < _nSamples; ii++) {
    double power = RadarComplex::power(powerSpec[ii]);
    if (power <= 1.0e-12) {
      dbm[ii] = -120.0;
    } else {
      dbm[ii] = 10.0 * log10(power);
    }
  }
  
  // set up filtering
  
  const IwrfCalib &calib = _beam->getCalib();
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
  moments.setNSamples(_nSamples);
  moments.init(_beam->getPrt(), calib.getWavelengthCm() / 100.0,
               _beam->getStartRangeKm(), _beam->getGateSpacingKm());
  moments.setCalib(calib);
  moments.setClutterWidthMps(_clutModelWidthMps);
  moments.setClutterInitNotchWidthMps(3.0);
  moments.setAntennaRate(_beam->getAntennaRate());
  
  // filter as appropriate
  
  TaArray<double> powerFilt_;
  double *powerFilt = powerFilt_.alloc(_nSamples);

  _regrOrderInUse = 0;
  if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_ADAPTIVE) {

    // adaptive spectral filter

    ClutFilter clutFilt;
    moments.applyAdaptiveFilter(_nSamples, _beam->getPrt(),
                                clutFilt, fft,
                                iqWindowed,
                                calibNoise,
                                _beam->getNyquist(),
                                iqFilt, iqNotched,
                                filterRatio,
                                spectralNoise,
                                spectralSnr);
    
    TaArray<RadarComplex_t> filtAdaptSpec_;
    RadarComplex_t *filtAdaptSpec = filtAdaptSpec_.alloc(_nSamples);
    fft.fwd(iqFilt, filtAdaptSpec);
    fft.shift(filtAdaptSpec);
    
    for (size_t ii = 0; ii < _nSamples; ii++) {
      powerFilt[ii] = RadarComplex::power(filtAdaptSpec[ii]);
    }
    
  } else if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_REGRESSION) {

    // regression filter

    moments.setUseRegressionFilter(_regrNotchInterpMethod, -120.0);
    
    ForsytheRegrFilter regrF;
    bool orderAuto = true;
    if (_regrOrder > 2) {
      orderAuto = false;
    }
 
    if (_beam->getIsStagPrt()) {
      regrF.setupStaggered(_nSamples, _beam->getStagM(), _beam->getStagN(),
                           orderAuto, _regrOrder,
                           _regrClutWidthFactor,
                           _regrCnrExponent,
                           _beam->getCalib().getWavelengthCm() / 100.0);
    } else {
      regrF.setup(_nSamples,
                  orderAuto, _regrOrder,
                  _regrClutWidthFactor,
                  _regrCnrExponent,
                  _beam->getCalib().getWavelengthCm() / 100.0);
    }
    
    moments.applyRegressionFilter(_nSamples, _beam->getPrt(), fft,
                                  regrF, iqWindowed,
                                  calibNoise,
                                  iqFilt, NULL,
                                  filterRatio,
                                  spectralNoise,
                                  spectralSnr);
    _regrOrderInUse = regrF.getPolyOrder();
    
    TaArray<RadarComplex_t> filtRegrWindowed_;
    RadarComplex_t *filtRegrWindowed = filtRegrWindowed_.alloc(_nSamples);
    _applyWindow(iqFilt, filtRegrWindowed, _nSamples);
    
    TaArray<RadarComplex_t> filtRegrSpec_;
    RadarComplex_t *filtRegrSpec = filtRegrSpec_.alloc(_nSamples);
    fft.fwd(filtRegrWindowed, filtRegrSpec);
    fft.shift(filtRegrSpec);
    
    for (size_t ii = 0; ii < _nSamples; ii++) {
      powerFilt[ii] = RadarComplex::power(filtRegrSpec[ii]);
    }
    
  } else if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_NOTCH) {

    // simple notch filter
    
    moments.applyNotchFilter(_nSamples, _beam->getPrt(), fft,
                             iqWindowed,
                             calibNoise,
                             iqFilt,
                             filterRatio, spectralNoise, spectralSnr);
    
    TaArray<RadarComplex_t> filtNotchSpec_;
    RadarComplex_t *filtNotchSpec = filtNotchSpec_.alloc(_nSamples);
    fft.fwd(iqFilt, filtNotchSpec);
    fft.shift(filtNotchSpec);

    memcpy(iqNotched, iqFilt, _nSamples * sizeof(RadarComplex_t));

    for (size_t ii = 0; ii < _nSamples; ii++) {
      powerFilt[ii] = RadarComplex::power(filtNotchSpec[ii]);
    }
    
  } else {

    // no filtering
    
    for (size_t ii = 0; ii < _nSamples; ii++) {
      powerFilt[ii] = RadarComplex::power(powerSpec[ii]);
    }
    
  }

  // compute dbmFilt
  
  for (size_t ii = 0; ii < _nSamples; ii++) {
    if (powerFilt[ii] <= 1.0e-12) {
      dbmFilt[ii] = -120.0;
    } else {
      dbmFilt[ii] = 10.0 * log10(powerFilt[ii]);
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

  painter.restore();

}

