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
// WaterfallPlot.cc
//
// Plotting of spectra vs range
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2021
//
///////////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>
#include <toolsa/Path.hh>
#include <toolsa/TaArray2D.hh>
#include <toolsa/sincos.h>
#include <radar/FilterUtils.hh>
#include <radar/ClutFilter.hh>

#include "WaterfallPlot.hh"
#include "Beam.hh"

double WaterfallPlot::missingVal = -9999.0;

using namespace std;

WaterfallPlot::WaterfallPlot(QWidget* parent,
                             const Params &params,
                             int id) :
        SpritePlot(parent, params, id),
        _params(params)
        
{

  _plotType = Params::WATERFALL_HC;
  _fftWindow = Params::FFT_WINDOW_VONHANN;
  _clutterFilterType = RadarMoments::CLUTTER_FILTER_NONE;
  _plotClutModel = false;
  _clutModelWidthMps = 0.75;
  _regrOrder = _params.regression_filter_specified_polynomial_order;
  _regrOrderInUse = _regrOrder;
  _regrClutWidthFactor = _params.regression_filter_clutter_width_factor;
  _regrCnrExponent = _params.regression_filter_cnr_exponent;
  _regrNotchInterpMethod = RadarMoments::INTERP_METHOD_GAUSSIAN;
  _beam = NULL;
  _nSamples = 0;
  _nGates = 0;

  _setInterestMaps();
  
}

/*************************************************************************
 * Destructor
 */

WaterfallPlot::~WaterfallPlot()
{

}


/*************************************************************************
 * clear()
 */

void WaterfallPlot::clear()
{

}

/*************************************************************************
 * prepare a beam for plotting
 */

void WaterfallPlot::prepareBeam(Beam *beam,
                                size_t nSamples)
  
{

  _beam = beam;
  _nSamples = nSamples;
  
  if (_beam == NULL) {
    cerr << "WARNING - WaterfallPlot::prepareBeam() - got NULL beam, ignoring"
         << endl;
    return;
  }
  
  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "======== Waterfall - preparing beam data ================" << endl;
    DateTime beamTime(beam->getTimeSecs(), true, beam->getNanoSecs() * 1.0e-9);
    cerr << "  Beam time: " << beamTime.asString(3) << endl;
    cerr << "  Max range: " << beam->getMaxRange() << endl;
  }
  
  // compute ngates for plotting
  
  _nGates = _beam->getNGates();
  
  if (_params.set_max_range) {
    size_t nGatesMax =
      (_params.max_range_km - _beam->getStartRangeKm()) /
      _beam->getGateSpacingKm();
    if (nGatesMax < _nGates) {
      _nGates = nGatesMax;
    }
  }
  
  // load up time series for dwell spectra

  switch (_fftWindow) {
    case Params::FFT_WINDOW_VONHANN:
      _spectra.setWindowType(RadarMoments::WINDOW_VONHANN);
      break;
    case Params::FFT_WINDOW_BLACKMAN:
      _spectra.setWindowType(RadarMoments::WINDOW_BLACKMAN);
      break;
    case Params::FFT_WINDOW_BLACKMAN_NUTTALL:
      _spectra.setWindowType(RadarMoments::WINDOW_BLACKMAN_NUTTALL);
      break;
    case Params::FFT_WINDOW_TUKEY_10:
      _spectra.setWindowType(RadarMoments::WINDOW_TUKEY_10);
      break;
    case Params::FFT_WINDOW_TUKEY_20:
      _spectra.setWindowType(RadarMoments::WINDOW_TUKEY_20);
      break;
    case Params::FFT_WINDOW_TUKEY_30:
      _spectra.setWindowType(RadarMoments::WINDOW_TUKEY_30);
      break;
    case Params::FFT_WINDOW_TUKEY_50:
      _spectra.setWindowType(RadarMoments::WINDOW_TUKEY_50);
    case Params::FFT_WINDOW_RECT:
    default:
      _spectra.setWindowType(RadarMoments::WINDOW_RECT);
      break;
  }
  
  _spectra.setClutterFilterType(_clutterFilterType);
  _spectra.setRegrOrder(_regrOrder);
  _spectra.setRegrClutWidthFactor(_regrClutWidthFactor);
  _spectra.setRegrCnrExponent(_regrCnrExponent);
  _spectra.setRegrFiltNotchInterpMethod(_regrNotchInterpMethod);

  _beam->loadDwellSpectra(_spectra);

}

/*************************************************************************
 * plot a beam
 */

void WaterfallPlot::plotBeam(QPainter &painter,
                             double selectedRangeKm)
  
{

  if (_beam == NULL) {
    return;
  }

  // perform the relevant plot
 
  switch (_plotType) {

    case Params::WATERFALL_HC:
      _plotHc(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_VC:
      _plotVc(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_HX:
      _plotHx(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_VX:
      _plotVx(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_DBZ:
      _plotDbz(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_SNR:
      _plotSnr(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_ZDR:
      _plotZdr(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_PHIDP:
      _plotPhidp(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_RHOHV:
      _plotRhohv(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_TDBZ:
      _plotTdbz(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_SDEV_ZDR:
      _plotSdevZdr(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_SDEV_PHIDP:
      _plotSdevPhidp(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_TDBZ_INT:
      _plotTdbzInt(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_SDEV_ZDR_INT:
      _plotSdevZdrInt(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_SDEV_PHIDP_INT:
      _plotSdevPhidpInt(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_CMD:
      _plotCmd(painter, selectedRangeKm);
      break;
    case Params::WATERFALL_CMD_FRAC:
      _plotCmdFraction(painter, selectedRangeKm);
      break;
  }

  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

  // legends
  
  char text[1024];
  vector<string> legendsLeft;
  painter.setBackgroundMode(Qt::OpaqueMode);
  legendsLeft.push_back(getFftWindowName(_fftWindow));
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
  if (_legendsOn) {
    _zoomWorld.drawLegendsTopLeft(painter, legendsLeft);
  }

  // draw the title

  painter.save();
  painter.setPen(_params.waterfall_title_color);
  string title("Waterfall:");
  title.append(getName(_plotType));
  _zoomWorld.drawTitleTopCenter(painter, title);
  painter.restore();

}

/*************************************************************************
 * plot HC spectrum
 */

void WaterfallPlot::_plotHc(QPainter &painter,
                            double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_dbm_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }
  
  painter.save();

  // loop through the gates
  
  for (size_t igate = 0; igate < _nGates; igate++) {

    // set limits for plotting this gate
    
    double yy = startRange + gateSpacing * (igate-0.5);

    // get Hc dbm
    
    double *dbmHc = _spectra.getSpecDbmHc2D()[igate];
    
    // apply median filter
    
    FilterUtils::applyMedianFilter(dbmHc, _nSamples, _medianFiltLen);
      
    // plot the samples
    
    for (size_t ii = 0; ii < _nSamples; ii++) {

      // get color

      int red, green, blue;
      _cmap.dataColor(dbmHc[ii], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      
      // set x limits

      double xx = ii;
      
      // fill rectangle

      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);

    } // ii

  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot HC spectrum
 */

void WaterfallPlot::_plotVc(QPainter &painter,
                            double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_dbm_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }
  
  painter.save();

  // loop through the gates
  
  for (size_t igate = 0; igate < _nGates; igate++) {

    // set limits for plotting this gate
    
    double yy = startRange + gateSpacing * (igate-0.5);

    // get Vc dbm
    
    double *dbmVc = _spectra.getSpecDbmVc2D()[igate];

    // apply 3-pt median filter
    
    FilterUtils::applyMedianFilter(dbmVc, _nSamples, _medianFiltLen);
      
    // plot the samples
    
    for (size_t ii = 0; ii < _nSamples; ii++) {
      
      // get color

      int red, green, blue;
      _cmap.dataColor(dbmVc[ii], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      
      // set x limits

      double xx = ii;
      
      // fill rectangle

      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);

    } // ii

  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot Hx spectrum
 */

void WaterfallPlot::_plotHx(QPainter &painter,
                            double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_dbm_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }
  
  painter.save();

  // loop through the gates
  
  for (size_t igate = 0; igate < _nGates; igate++) {

    // set limits for plotting this gate
    
    double yy = startRange + gateSpacing * (igate-0.5);

    // get Hx dbm
    
    double *dbmHx = _spectra.getSpecDbmHx2D()[igate];

    // apply 3-pt median filter
    
    FilterUtils::applyMedianFilter(dbmHx, _nSamples, _medianFiltLen);
      
    // plot the samples
    
    for (size_t ii = 0; ii < _nSamples; ii++) {
      
      // get color

      int red, green, blue;
      _cmap.dataColor(dbmHx[ii], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      
      // set x limits

      double xx = ii;
      
      // fill rectangle

      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);

    } // ii

  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot VX spectrum
 */

void WaterfallPlot::_plotVx(QPainter &painter,
                            double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_dbm_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }
  
  painter.save();

  // loop through the gates
  
  for (size_t igate = 0; igate < _nGates; igate++) {

    // set limits for plotting this gate
    
    double yy = startRange + gateSpacing * (igate-0.5);
    
    // get dbm for Vx

    double *dbmVx = _spectra.getSpecDbmVx2D()[igate];

    // apply 3-pt median filter
    
    FilterUtils::applyMedianFilter(dbmVx, _nSamples, _medianFiltLen);
      
    // plot the samples
    
    for (size_t ii = 0; ii < _nSamples; ii++) {
      
      // get color

      int red, green, blue;
      _cmap.dataColor(dbmVx[ii], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      
      // set x limits

      double xx = ii;
      
      // fill rectangle

      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);

    } // ii

  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot DBZ spectrum
 */

void WaterfallPlot::_plotDbz(QPainter &painter,
                             double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_dbz_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }
  
  painter.save();

  // loop through the gates
  
  for (size_t igate = 0; igate < _nGates; igate++) {

    // set limits for plotting this gate
    
    double yy = startRange + gateSpacing * (igate-0.5);

    // get field
    
    double *dbz = _spectra.getSpecDbz2D()[igate];

    // apply 3-pt median filter
    
    FilterUtils::applyMedianFilter(dbz, _nSamples, _medianFiltLen);
      
    // plot the samples
    
    for (size_t ii = 0; ii < _nSamples; ii++) {

      // get color

      int red, green, blue;
      _cmap.dataColor(dbz[ii], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      
      // set x limits

      double xx = ii;
      
      // fill rectangle

      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);

    } // ii

  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot SNR spectrum
 */

void WaterfallPlot::_plotSnr(QPainter &painter,
                             double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_snr_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }
  
  painter.save();

  // loop through the gates
  
  for (size_t igate = 0; igate < _nGates; igate++) {

    // set limits for plotting this gate
    
    double yy = startRange + gateSpacing * (igate-0.5);

    // get field
    
    double *snr = _spectra.getSpecSnr2D()[igate];

    // apply 3-pt median filter
    
    FilterUtils::applyMedianFilter(snr, _nSamples, _medianFiltLen);
      
    // plot the samples
    
    for (size_t ii = 0; ii < _nSamples; ii++) {

      // get color

      int red, green, blue;
      _cmap.dataColor(snr[ii], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      
      // set x limits

      double xx = ii;
      
      // fill rectangle

      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);

    } // ii

  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot ZDR spectrum
 */

void WaterfallPlot::_plotZdr(QPainter &painter,
                             double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_zdr_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }
  
  painter.save();

  // loop through the gates
  
  for (size_t igate = 0; igate < _nGates; igate++) {

    // set limits for plotting this gate
    
    double yy = startRange + gateSpacing * (igate-0.5);
    
    // get zdr
    
    double *zdr = _spectra.getSpecZdr2D()[igate];

    // apply 3-pt median filter
    
    FilterUtils::applyMedianFilter(zdr, _nSamples, _medianFiltLen);
      
    // plot the samples
    
    for (size_t ii = 0; ii < _nSamples; ii++) {

      // get color

      int red, green, blue;
      _cmap.dataColor(zdr[ii], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      
      // set x limits

      double xx = ii;
      
      // fill rectangle

      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);

    } // ii

  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot PHIDP spectrum
 */

void WaterfallPlot::_plotPhidp(QPainter &painter,
                               double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_phidp_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }
  
  painter.save();

  // loop through the gates
  
  for (size_t igate = 0; igate < _nGates; igate++) {

    // set limits for plotting this gate
    
    double yy = startRange + gateSpacing * (igate-0.5);
    
    // get phidp spectrum
    
    double *phidp = _spectra.getSpecPhidp2D()[igate];
    
    // apply 3-pt median filter
    
    FilterUtils::applyMedianFilter(phidp, _nSamples, _medianFiltLen);
      
    // plot the samples
    
    for (size_t ii = 0; ii < _nSamples; ii++) {
      
      // get color

      int red, green, blue;
      _cmap.dataColor(phidp[ii], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      
      // set x limits

      double xx = ii;
      
      // fill rectangle

      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);

    } // ii

  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot RHOHV spectrum
 */

void WaterfallPlot::_plotRhohv(QPainter &painter,
                               double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_rhohv_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }
  
  painter.save();

  // loop through the gates
  
  for (size_t igate = 0; igate < _nGates; igate++) {

    // set limits for plotting this gate
    
    double yy = startRange + gateSpacing * (igate-0.5);

    // get spectral rhohv
    
    double *rhohv = _spectra.getSpecRhohv2D()[igate];
    
    // apply 3-pt median filter
    
    FilterUtils::applyMedianFilter(rhohv, _nSamples, _medianFiltLen);
      
    // plot the samples
    
    for (size_t ii = 0; ii < _nSamples; ii++) {
      
      // get color

      int red, green, blue;
      _cmap.dataColor(rhohv[ii], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      
      // set x limits

      double xx = ii;
      
      // fill rectangle

      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);

    } // ii

  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot TDBZ spectrum
 */

void WaterfallPlot::_plotTdbz(QPainter &painter,
                             double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_dbz_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }
  
  painter.save();

  // loop through the gates
  
  for (size_t igate = 0; igate < _nGates; igate++) {

    // set limits for plotting this gate
    
    double yy = startRange + gateSpacing * (igate-0.5);

    // get field
    
    double *tdbz = _spectra.getSpecTdbz2D()[igate];

    // apply 3-pt median filter
    
    FilterUtils::applyMedianFilter(tdbz, _nSamples, _medianFiltLen);
      
    // plot the samples
    
    for (size_t ii = 0; ii < _nSamples; ii++) {

      // get color

      int red, green, blue;
      _cmap.dataColor(tdbz[ii], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      
      // set x limits

      double xx = ii;
      
      // fill rectangle

      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);

    } // ii

  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot SDEV of ZDR spectrum
 */

void WaterfallPlot::_plotSdevZdr(QPainter &painter,
                                 double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_sdev_zdr_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }

  // plot the data

  painter.save();
  
  for (size_t igate = 0; igate < _nGates; igate++) {
    double *zdrSdev = _spectra.getSpecSdevZdr2D()[igate];
    double yy = startRange + gateSpacing * (igate-0.5);
    for (size_t isample = 0; isample < _nSamples; isample++) {
      // get color
      int red, green, blue;
      _cmap.dataColor(zdrSdev[isample], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      // set x limits
      double xx = isample;
      // fill rectangle
      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);
    } // isample
  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot sdev of PHIDP spectrum
 */

void WaterfallPlot::_plotSdevPhidp(QPainter &painter,
                                   double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_sdev_phidp_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }

  // plot the data

  painter.save();

  for (size_t igate = 0; igate < _nGates; igate++) {
    double *phidpSdev = _spectra.getSpecSdevPhidp2D()[igate];
    double yy = startRange + gateSpacing * (igate-0.5);
    for (size_t isample = 0; isample < _nSamples; isample++) {
      // get color
      int red, green, blue;
      _cmap.dataColor(phidpSdev[isample], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      // set x limits
      double xx = isample;
      // fill rectangle
      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);
    } // isample
  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot interest of sdev of TDBZ spectrum
 */

void WaterfallPlot::_plotTdbzInt(QPainter &painter,
                                 double selectedRangeKm)
  
{
  
  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_interest_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }

  // plot the data
  
  painter.save();

  for (size_t igate = 0; igate < _nGates; igate++) {
    double *interest = _spectra.getSpecTdbzInterest2D()[igate];
    double yy = startRange + gateSpacing * (igate-0.5);
    for (size_t isample = 0; isample < _nSamples; isample++) {
      // get color
      int red, green, blue;
      _cmap.dataColor(interest[isample], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      // set x limits
      double xx = isample;
      // fill rectangle
      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);
    } // isample
  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot interest of sdev of ZDR spectrum
 */

void WaterfallPlot::_plotSdevZdrInt(QPainter &painter,
                                      double selectedRangeKm)
  
{
  
  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_interest_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }

  // plot the data
  
  painter.save();

  for (size_t igate = 0; igate < _nGates; igate++) {
    double *interest = _spectra.getSpecSdevZdrInterest2D()[igate];
    double yy = startRange + gateSpacing * (igate-0.5);
    for (size_t isample = 0; isample < _nSamples; isample++) {
      // get color
      int red, green, blue;
      _cmap.dataColor(interest[isample], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      // set x limits
      double xx = isample;
      // fill rectangle
      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);
    } // isample
  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot interest of sdev of PHIDP spectrum
 */

void WaterfallPlot::_plotSdevPhidpInt(QPainter &painter,
                                      double selectedRangeKm)
  
{
  
  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_interest_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }

  // plot the data
  
  painter.save();

  for (size_t igate = 0; igate < _nGates; igate++) {
    double *interest = _spectra.getSpecSdevPhidpInterest2D()[igate];
    double yy = startRange + gateSpacing * (igate-0.5);
    for (size_t isample = 0; isample < _nSamples; isample++) {
      // get color
      int red, green, blue;
      _cmap.dataColor(interest[isample], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      // set x limits
      double xx = isample;
      // fill rectangle
      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);
    } // isample
  } // igate
  
  painter.restore();

}

/*************************************************************************
 * plot CMD spectrum
 */

void WaterfallPlot::_plotCmd(QPainter &painter,
                             double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_cmd_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }

#ifdef JUNK
  
  // get the noise value
  
  const IwrfCalib &calib = _beam->getCalib();
  double calibNoiseHcDb = calib.getNoiseDbmHc();
  
  // load up 2D arrays of spectral power, zdr and phidp

  TaArray2D<double> powerHc2D_;
  double **powerHc2D = powerHc2D_.alloc(_nGates, _nSamples);
  TaArray2D<double> zdr2D_;
  double **zdr2D = zdr2D_.alloc(_nGates, _nSamples);
  TaArray2D<double> phidp2D_;
  double **phidp2D = phidp2D_.alloc(_nGates, _nSamples);

  for (size_t igate = 0; igate < _nGates; igate++) {

    // get Iq data for this gate
    
    const GateData *gateData = _beam->getGateData()[igate];
    
    // compute ZDR spectrum
    
    TaArray<double> powerHc_, dbmHc_;
    double *powerHc = powerHc_.alloc(_nSamples);
    double *dbmHc = dbmHc_.alloc(_nSamples);
    _computePowerSpectrum(_beam, _nSamples, gateData->iqhcOrig, powerHc, dbmHc);
    
    TaArray<double> powerVc_, dbmVc_;
    double *powerVc = powerVc_.alloc(_nSamples);
    double *dbmVc = dbmVc_.alloc(_nSamples);
    _computePowerSpectrum(_beam, _nSamples, gateData->iqvcOrig, powerVc, dbmVc);
    
    for (size_t isample = 0; isample < _nSamples; isample++) {
      double zdr = dbmHc[isample] - dbmVc[isample];
      zdr2D[igate][isample] = zdr;
    } // isample

    // apply window to time series
    
    TaArray<RadarComplex_t> iqWindowedHc_, iqWindowedVc_;
    RadarComplex_t *iqWindowedHc = iqWindowedHc_.alloc(_nSamples);
    RadarComplex_t *iqWindowedVc = iqWindowedVc_.alloc(_nSamples);
    _applyWindow(gateData->iqhcOrig, iqWindowedHc, _nSamples);
    _applyWindow(gateData->iqvcOrig, iqWindowedVc, _nSamples);
    
    // compute spectra
    
    TaArray<RadarComplex_t> specHc_, specVc_;
    RadarComplex_t *specHc = specHc_.alloc(_nSamples);
    RadarComplex_t *specVc = specVc_.alloc(_nSamples);
    RadarFft fft(_nSamples);
    fft.fwd(iqWindowedHc, specHc);
    fft.shift(specHc);
    fft.fwd(iqWindowedVc, specVc);
    fft.shift(specVc);

    // compute phidp spectrum, store in 2D array

    for (size_t isample = 0; isample < _nSamples; isample++) {
      RadarComplex_t diff = RadarComplex::conjugateProduct(specHc[isample], specVc[isample]);
      phidp2D[igate][isample] = RadarComplex::argDeg(diff);
      powerHc2D[igate][isample] = RadarComplex::power(specHc[isample]);
    } // isample
    
  } // igate

  // compute the phidp folding range

  _computePhidpFoldingRange(_nGates, _nSamples, phidp2D);

  // compute 2D sdev of spectral zdr and phidp

  TaArray2D<double> sdevZdr2D_;
  double **sdevZdr2D = sdevZdr2D_.alloc(_nGates, _nSamples);
  TaArray2D<double> sdevPhidp2D_;
  double **sdevPhidp2D = sdevPhidp2D_.alloc(_nGates, _nSamples);
  int _nSamplesSdev = min(_params.waterfall_sdev_phidp_kernel_nsamples, (int) _nSamples);
  int _nGatesSdev = min(_params.waterfall_sdev_phidp_kernel_ngates, (int) _nGates);
  int _nSamplesSdevHalf = _nSamplesSdev / 2;
  int _nGatesSdevHalf = _nGatesSdev / 2;
  
  for (size_t igate = 0; igate < _nGates; igate++) {
    for (size_t isample = 0; isample < _nSamples; isample++) {

      // compute index limits for computing sdev

      int sampleStart = isample - _nSamplesSdevHalf;
      sampleStart = max(0, sampleStart);
      int sampleEnd = sampleStart + _nSamplesSdev;
      sampleEnd = min((int) _nSamples, sampleEnd);
      sampleStart = sampleEnd - _nSamplesSdev;

      int gateStart = igate - _nGatesSdevHalf;
      gateStart = max(0, gateStart);
      int gateEnd = gateStart + _nGatesSdev;
      gateEnd = min((int) _nGates, gateEnd);
      gateStart = gateEnd - _nGatesSdev;

      // load up zdr values for kernel region

      vector<double> zdrKernel;
      for (int jgate = gateStart; jgate < gateEnd; jgate++) {
        for (int jsample = sampleStart; jsample < sampleEnd; jsample++) {
          zdrKernel.push_back(zdr2D[jgate][jsample]);
        } // jsample
      } // jgate

      // compute sdev of zdr
      
      double zdrSdev = _computeSdevZdr(zdrKernel);
      sdevZdr2D[igate][isample] = zdrSdev;
      
      // load up phidp values for kernel region

      vector<double> phidpKernel;
      for (int jgate = gateStart; jgate < gateEnd; jgate++) {
        for (int jsample = sampleStart; jsample < sampleEnd; jsample++) {
          phidpKernel.push_back(phidp2D[jgate][jsample]);
        } // jsample
      } // jgate

      // compute sdev of phidp

      double phidpSdev = _computeSdevPhidp(phidpKernel);
      sdevPhidp2D[igate][isample] = phidpSdev;
      
    } // isample
  } // igate

  // compute CMD

  TaArray2D<double> cmd2D_;
  double **cmd2D = cmd2D_.alloc(_nGates, _nSamples);

  for (size_t igate = 0; igate < _nGates; igate++) {
    for (size_t isample = 0; isample < _nSamples; isample++) {

      double powerHc = powerHc2D[igate][isample];
      double sdevZdr = sdevZdr2D[igate][isample];
      double sdevPhidp = sdevPhidp2D[igate][isample];

      double powerHcDb = 10.0 * log10(powerHc);
      double snrDb = powerHcDb - calibNoiseHcDb;

      double snrInterest = 0.0;
      if (snrDb > 50) {
        snrInterest = 1.0;
      } else if (snrDb < 0) {
        snrInterest = 0.0;
      } else {
        snrInterest = (snrDb / 50);
      }

      double zdrInterest = 0.0;
      if (sdevZdr > 8.0) {
        zdrInterest = 1.0;
      } else {
        zdrInterest = sdevZdr / 8.0;
      }

      double phidpInterest = 0.0;
      if (sdevPhidp > 64.0) {
        phidpInterest = 1.0;
      } else {
        phidpInterest = sdevPhidp / 64.0;
      }

      double cmd = (snrInterest + zdrInterest + phidpInterest) / 3.0;
      cmd2D[igate][isample] = cmd;
      
      // double maxZdrPhidpInterest = max(zdrInterest, phidpInterest);
      // double cmd = (snrInterest + maxZdrPhidpInterest) / 2.0;


    } // isample
  } // igate

#endif
  
  // plot the data
  
  painter.save();

  for (size_t igate = 0; igate < _nGates; igate++) {
    double *cmd = _spectra.getSpecCmd2D()[igate];
    double yy = startRange + gateSpacing * (igate-0.5);
    for (size_t isample = 0; isample < _nSamples; isample++) {
      // get color
      int red, green, blue;
      _cmap.dataColor(cmd[isample], red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      // set x limits
      double xx = isample;
      // fill rectangle
      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);
    } // isample
  } // igate
  
  painter.restore();
  
}

/*************************************************************************
 * plot CMD fraction
 */

void WaterfallPlot::_plotCmdFraction(QPainter &painter,
                                     double selectedRangeKm)
  
{

  double startRange = _beam->getStartRangeKm();
  double gateSpacing = _beam->getGateSpacingKm();

  // draw the color scale
  
  if (_readColorMap(_params.waterfall_cmd_frac_color_scale_name) == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }
  
  painter.save();

  // loop through the gates
  
  for (size_t igate = 0; igate < _nGates; igate++) {

    // set limits for plotting this gate
    
    double yy = startRange + gateSpacing * (igate-0.5);

    // get field
    
    double frac = _spectra.getFractionCmd1D()[igate];

    // plot the samples
    
    for (size_t ii = 0; ii < _nSamples; ii++) {

      // get color

      int red, green, blue;
      _cmap.dataColor(frac, red, green, blue);
      QColor color(red, green, blue);
      QBrush brush(color);
      
      // set x limits

      double xx = ii;
      
      // fill rectangle

      double width = 1.0;
      double height = gateSpacing;
      _zoomWorld.fillRectangle(painter, brush, xx, yy, width * 2, height * 2);

    } // ii

  } // igate
  
  painter.restore();

}

/*************************************************************************
 * compute the spectrum
 */

void WaterfallPlot::_computePowerSpectrum(Beam *beam,
                                          size_t _nSamples,
                                          const RadarComplex_t *iq,
                                          double *power,
                                          double *dbm)
  
{

  // init
  
  for (size_t ii = 0; ii < _nSamples; ii++) {
    power[ii] = 1.0e-12;
    dbm[ii] = -120.0;
  }

  // apply window to time series
  
  TaArray<RadarComplex_t> iqWindowed_;
  RadarComplex_t *iqWindowed = iqWindowed_.alloc(_nSamples);
  _applyWindow(iq, iqWindowed, _nSamples);
  
  // compute power spectrum
  
  TaArray<RadarComplex_t> powerSpec_;
  RadarComplex_t *powerSpec = powerSpec_.alloc(_nSamples);
  RadarFft fft(_nSamples);
  fft.fwd(iqWindowed, powerSpec);
  fft.shift(powerSpec);
  
  // set up filtering
  
  const IwrfCalib &calib = _beam->getCalib();
  double calibNoise = 0.0;
  switch (_plotType) {
    case Params::WATERFALL_VC:
      calibNoise = pow(10.0, calib.getNoiseDbmVc() / 10.0);
      break;
    case Params::WATERFALL_HX:
      calibNoise = pow(10.0, calib.getNoiseDbmHx() / 10.0);
      break;
    case Params::WATERFALL_VX:
      calibNoise = pow(10.0, calib.getNoiseDbmVx() / 10.0);
      break;
    default:
      calibNoise = pow(10.0, calib.getNoiseDbmHc() / 10.0);
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

  TaArray<RadarComplex_t> iqFilt_, iqNotched_;
  RadarComplex_t *iqFilt = iqFilt_.alloc(_nSamples);
  RadarComplex_t *iqNotched = iqNotched_.alloc(_nSamples);
  double filterRatio, spectralNoise, spectralSnr;

  if (_clutterFilterType == RadarMoments::CLUTTER_FILTER_ADAPTIVE) {

    // adaptive spectral filter

    ClutFilter clutFilt;
    moments.applyAdaptiveFilter(_nSamples, _beam->getPrt(),
                                clutFilt, fft,
                                iqWindowed, calibNoise, _beam->getNyquist(),
                                iqFilt, iqNotched,
                                filterRatio, spectralNoise, spectralSnr);
    
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
    
    moments.applyRegressionFilter(_nSamples, _beam->getPrt(),
                                  fft, regrF,
                                  iqWindowed,
                                  calibNoise,
                                  iqFilt, iqNotched,
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
                             iqWindowed, calibNoise,
                             iqFilt,
                             filterRatio, spectralNoise, spectralSnr);
    
    TaArray<RadarComplex_t> filtNotchSpec_;
    RadarComplex_t *filtNotchSpec = filtNotchSpec_.alloc(_nSamples);
    fft.fwd(iqFilt, filtNotchSpec);
    fft.shift(filtNotchSpec);

    for (size_t ii = 0; ii < _nSamples; ii++) {
      powerFilt[ii] = RadarComplex::power(filtNotchSpec[ii]);
    }
    
  } else {

    // no filtering
    
    for (size_t ii = 0; ii < _nSamples; ii++) {
      powerFilt[ii] = RadarComplex::power(powerSpec[ii]);
    }
    
  }

  // compute dbm
  
  for (size_t ii = 0; ii < _nSamples; ii++) {
    power[ii] = powerFilt[ii];
    if (power[ii] <= 1.0e-12) {
      dbm[ii] = -120.0;
    } else {
      dbm[ii] = 10.0 * log10(power[ii]);
    }
  }

}

//////////////////////////////////
// get a string for the field name

string WaterfallPlot::getName(Params::waterfall_type_t wtype)
{
  switch (wtype) {
    case Params::WATERFALL_HC:
      return "HC";
    case Params::WATERFALL_VC:
      return "VC";
    case Params::WATERFALL_HX:
      return "HX";
    case Params::WATERFALL_VX:
      return "VX";
    case Params::WATERFALL_DBZ:
      return "DBZ";
    case Params::WATERFALL_SNR:
      return "SNR";
    case Params::WATERFALL_ZDR:
      return "ZDR";
    case Params::WATERFALL_PHIDP:
      return "PHIDP";
    case Params::WATERFALL_RHOHV:
      return "RHOHV";
    case Params::WATERFALL_TDBZ:
      return "TDBZ";
    case Params::WATERFALL_SDEV_ZDR:
      return "SDEV_ZDR";
    case Params::WATERFALL_SDEV_PHIDP:
      return "SDEV_PHIDP";
    case Params::WATERFALL_TDBZ_INT:
      return "TDBZ_INT";
    case Params::WATERFALL_SDEV_ZDR_INT:
      return "SDEV_ZDR_INT";
    case Params::WATERFALL_SDEV_PHIDP_INT:
      return "SDEV_PHIDP_INT";
    case Params::WATERFALL_CMD:
      return "CMD";
    case Params::WATERFALL_CMD_FRAC:
      return "CMD_FRAC";
    default:
      return "UNKNOWN";
  }
}

//////////////////////////////////
// get a string for the field units

string WaterfallPlot::getUnits(Params::waterfall_type_t wtype)
{
  switch (wtype) {
    case Params::WATERFALL_HC:
    case Params::WATERFALL_VC:
    case Params::WATERFALL_HX:
    case Params::WATERFALL_VX:
      return "dBm";
    case Params::WATERFALL_DBZ:
    case Params::WATERFALL_TDBZ:
      return "dBZ";
    case Params::WATERFALL_SNR:
    case Params::WATERFALL_ZDR:
    case Params::WATERFALL_SDEV_ZDR:
      return "dB";
    case Params::WATERFALL_PHIDP:
    case Params::WATERFALL_SDEV_PHIDP:
      return "deg";
    case Params::WATERFALL_CMD:
    default:
      return "";
  }
}

/*************************************************************************
 * set the geometry - unzooms
 */

void WaterfallPlot::setWindowGeom(int width, int height,
                                  int xOffset, int yOffset)
{
  _fullWorld.setWindowGeom(width, height, xOffset, yOffset);
  _fullWorld.setColorScaleWidth(_params.waterfall_color_scale_width);
  _zoomWorld = _fullWorld;
}

/*************************************************************************
 * set the world limits - unzooms
 */

void WaterfallPlot::setWorldLimits(double xMinWorld,
                                   double yMinWorld,
                                   double xMaxWorld,
                                   double yMaxWorld)
{
  _fullWorld.setWorldLimits(xMinWorld, yMinWorld,
                            xMaxWorld, yMaxWorld);
  _fullWorld.setColorScaleWidth(_params.waterfall_color_scale_width);
  _zoomWorld = _fullWorld;
}

/*************************************************************************
 * Protected methods
 *************************************************************************/

/*************************************************************************
 * Draw the overlays, axes, legends etc
 */

void WaterfallPlot::_drawOverlays(QPainter &painter, double selectedRangeKm)
{

  // save painter state
  
  painter.save();
  
  // store font
  
  QFont origFont = painter.font();
  
  painter.setPen(_params.waterfall_axis_label_color);

  _zoomWorld.drawAxisBottom(painter, "sample",
                            true, true, true, _xGridLinesOn);
  
  _zoomWorld.drawAxisLeft(painter, "km", 
                          true, true, true, _yGridLinesOn);

  _zoomWorld.drawYAxisLabelLeft(painter, "Range");

  // selected range line
  
  QPen pen(painter.pen());
  pen.setColor(_params.waterfall_selected_range_color);
  pen.setStyle(Qt::SolidLine);
  pen.setWidth(2);
  painter.setPen(pen);
  _zoomWorld.drawLine(painter,
                      _zoomWorld.getXMinWorld(), selectedRangeKm,
                      _zoomWorld.getXMaxWorld(), selectedRangeKm);

  painter.restore();

}

//////////////////////////////////////////////////
// read color map
// returns 0 on success, -1 on failure
  
int WaterfallPlot::_readColorMap(string colorScaleName)
{
  
  // check for color map location
  
  string colorMapDir = _params.color_scale_dir;
  Path mapDir(_params.color_scale_dir);
  if (!mapDir.dirExists()) {
    colorMapDir = Path::getPathRelToExec(_params.color_scale_dir);
    mapDir.setPath(colorMapDir);
    if (!mapDir.dirExists()) {
      cerr << "ERROR - WaterfallPlot::_readColorMap()" << endl;
      cerr << "  Cannot find color scale directory" << endl;
      cerr << "  Primary is: " << _params.color_scale_dir << endl;
      cerr << "  Secondary is relative to binary: " << colorMapDir << endl;
      return -1;
    }
    if (_params.debug) {
      cerr << "NOTE - using color scales relative to executable location" << endl;
      cerr << "  Exec path: " << Path::getExecPath() << endl;
      cerr << "  Color scale dir:: " << colorMapDir << endl;
    }
  }

  // create color map
  
  string colorMapPath = colorMapDir;
  colorMapPath += PATH_DELIM;
  colorMapPath += colorScaleName;
  _cmap.setName(getName(_plotType));
  _cmap.setUnits(getUnits(_plotType));

  if (_cmap.readMap(colorMapPath)) {
    cerr << "ERROR - WaterfallPlot::_readColorMap()" << endl;
    cerr << "  Cannot read color map, path: " << colorMapPath << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////////////
// Apply the window to the time series

void WaterfallPlot::_applyWindow(const RadarComplex_t *iq, 
                                 RadarComplex_t *iqWindowed,
                                 size_t _nSamples)
{
  
  // initialize the window
  
  _windowCoeff = _windowCoeff_.alloc(_nSamples);
  switch (_fftWindow) {
    case Params::FFT_WINDOW_RECT:
    default:
      RadarMoments::initWindowRect(_nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_VONHANN:
      RadarMoments::initWindowVonhann(_nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_BLACKMAN:
      RadarMoments::initWindowBlackman(_nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_BLACKMAN_NUTTALL:
      RadarMoments::initWindowBlackmanNuttall(_nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_TUKEY_10:
      RadarMoments::initWindowTukey(0.1, _nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_TUKEY_20:
      RadarMoments::initWindowTukey(0.2, _nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_TUKEY_30:
      RadarMoments::initWindowTukey(0.3, _nSamples, _windowCoeff);
      break;
    case Params::FFT_WINDOW_TUKEY_50:
      RadarMoments::initWindowTukey(0.5, _nSamples, _windowCoeff);
  }

  // compute power spectrum
  
  RadarMoments::applyWindow(iq, _windowCoeff, iqWindowed, _nSamples);

}
  
/////////////////////////////////////////////////
// compute for ZDR SDEV

double WaterfallPlot::_computeSdevZdr(const vector<double> &zdr)
  
{
  
  double nZdr = 0.0;
  double sumZdr = 0.0;
  double sumZdrSq = 0.0;
  
  for (size_t ii = 0; ii < zdr.size(); ii++) {
    double val = zdr[ii];
    sumZdr += val;
    sumZdrSq += (val * val);
    nZdr++;
  }
    
  double meanZdr = sumZdr / nZdr;
  double sdev = 0.001;
  if (nZdr > 2) {
    double term1 = sumZdrSq / nZdr;
    double term2 = meanZdr * meanZdr;
    if (term1 >= term2) {
      sdev = sqrt(term1 - term2);
    }
  }

  return sdev;

}

/////////////////////////////////////////////////
// compute SDEV for PHIDP
// takes account of folding

double WaterfallPlot::_computeSdevPhidp(const vector<double> &phidp)
   
{
  
  // compute mean phidp

  double count = 0.0;
  double sumxx = 0.0;
  double sumyy = 0.0;
  for (size_t ii = 0; ii < phidp.size(); ii++) {
    double xx, yy;
    ta_sincos(phidp[ii] * DEG_TO_RAD, &yy, &xx);
    sumxx += xx;
    sumyy += yy;
    count++;
  }
  double meanxx = sumxx / count;
  double meanyy = sumyy / count;
  double phidpMean = atan2(meanyy, meanxx) * RAD_TO_DEG;
  if (_phidpFoldsAt90) {
    phidpMean *= 0.5;
  }
  
  // compute standard deviation centered on the mean value
  
  count = 0.0;
  double sum = 0.0;
  double sumSq = 0.0;
  for (size_t ii = 0; ii < phidp.size(); ii++) {
    double diff = phidp[ii] - phidpMean;
    // constrain diff
    while (diff < -_phidpFoldVal) {
      diff += 2 * _phidpFoldVal;
    }
    while (diff > _phidpFoldVal) {
      diff -= 2 * _phidpFoldVal;
    }
    // sum up
    count++;
    sum += diff;
    sumSq += diff * diff;
  }

  double meanDiff = sum / count;
  double term1 = sumSq / count;
  double term2 = meanDiff * meanDiff;
  double sdev = 0.001;
  if (term1 >= term2) {
    sdev = sqrt(term1 - term2);
  }

  return sdev;
  
}

/////////////////////////////////////////////
// compute the folding values and range
// by inspecting the phidp values

void WaterfallPlot::_computePhidpFoldingRange(size_t _nGates, size_t _nSamples, 
                                              double **phidp2D)
  
{
  
  // check if fold is at 90 or 180
  
  double phidpMin = 9999;
  double phidpMax = -9999;

  for (size_t igate = 0; igate < _nGates; igate++) {
    for (size_t isample = 0; isample < _nSamples; isample++) {
      double phidp = phidp2D[igate][isample];
      phidpMin = min(phidpMin, phidp);
      phidpMax = max(phidpMax, phidp);
    } // isample
  } // igate
  
  _phidpFoldsAt90 = false;
  _phidpFoldVal = 180.0;
  if (phidpMin > -90 && phidpMax < 90) {
    _phidpFoldVal = 90.0;
    _phidpFoldsAt90 = true;
  }
  _phidpFoldRange = _phidpFoldVal * 2.0;
  
}

/////////////////////////////////////////////////////////
// create interest maps

int WaterfallPlot::_setInterestMaps()

{  

  // SNR

  vector<InterestMap::ImPoint> pts;
  if (_convertInterestMapToVector("SNR",
                                  _params._snr_interest_map,
                                  _params.snr_interest_map_n,
                                  pts)) {
    return -1;
  }
  _spectra.setInterestMapSnr(pts, _params.snr_interest_weight);

  // TDBZ

  if (_convertInterestMapToVector("TDBZ",
                                  _params._tdbz_interest_map,
                                  _params.tdbz_interest_map_n,
                                  pts)) {
    return -1;
  }
  _spectra.setInterestMapTdbz(pts, _params.tdbz_interest_weight);

  // sdev of zdr

  if (_convertInterestMapToVector("zdr sdev",
                                  _params._zdr_sdev_interest_map,
                                  _params.zdr_sdev_interest_map_n,
                                  pts)) {
    return -1;
  }
  _spectra.setInterestMapSdevZdr(pts, _params.zdr_sdev_interest_weight);

  // sdev of phidp
  
  if (_convertInterestMapToVector("phidp sdev",
                                  _params._phidp_sdev_interest_map,
                                  _params.phidp_sdev_interest_map_n,
                                  pts)) {
    return -1;
  }
  _spectra.setInterestMapSdevPhidp(pts, _params.phidp_sdev_interest_weight);

  // set threshold for identification of clutter
  
  _spectra.setCmdThresholdMoments(_params.cmd_threshold_for_moments);
  _spectra.setCmdThresholdDetect(_params.cmd_threshold_for_detection);
  
  return 0;
  
}

////////////////////////////////////////////////////////////////////////
// Convert interest map points to vector
//
// Returns 0 on success, -1 on failure

int WaterfallPlot::_convertInterestMapToVector
  (const string &label,
   const Params::interest_map_point_t *map,
   int nPoints,
   vector<InterestMap::ImPoint> &pts)

{

  pts.clear();

  double prevVal = -1.0e99;
  for (int ii = 0; ii < nPoints; ii++) {
    if (map[ii].value <= prevVal) {
      cerr << "ERROR - Cmd::_convertInterestMapToVector" << endl;
      cerr << "  Map label: " << label << endl;
      cerr << "  Map values must increase monotonically" << endl;
      return -1;
    }
    InterestMap::ImPoint pt(map[ii].value, map[ii].interest);
    pts.push_back(pt);
    prevVal = map[ii].value;
  } // ii
  
  return 0;

}

