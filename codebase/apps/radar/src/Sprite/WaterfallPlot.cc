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

#include <assert.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/pjg.h>
#include <radar/FilterUtils.hh>

#include <QTimer>
#include <QBrush>
#include <QPalette>
#include <QPaintEngine>
#include <QPen>
#include <QResizeEvent>
#include <QStylePainter>

#include "WaterfallPlot.hh"
#include "SpectraMgr.hh"
#include "Beam.hh"

using namespace std;

WaterfallPlot::WaterfallPlot(QWidget* parent,
                             const Params &params,
                             int id) :
        _parent(parent),
        _params(params),
        _id(id)
        
{
  _isZoomed = false;
  _xGridLinesOn = _params.waterfall_x_grid_lines_on;
  _yGridLinesOn = _params.waterfall_y_grid_lines_on;
  _plotType = Params::WATERFALL_HC;
  _fftWindow = Params::FFT_WINDOW_VONHANN;
  _useAdaptiveFilt = false;
  _clutWidthMps = 0.75;
  _useRegrFilt = false;
  _regrOrder = 3;
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
 * perform zoom
 */

void WaterfallPlot::zoom(int x1, int y1, int x2, int y2)
{

  _zoomWorld.setZoomLimits(x1, y1, x2, y2);
  _isZoomed = true;

}

/*************************************************************************
 * unzoom the view
 */

void WaterfallPlot::unzoom()
{

  _zoomWorld = _fullWorld;
  _isZoomed = false;

}

/*************************************************************************
 * plot a beam
 */

void WaterfallPlot::plotBeam(QPainter &painter,
                             Beam *beam,
                             int nSamples,
                             double selectedRangeKm)
  
{

  if (beam == NULL) {
    cerr << "WARNING - WaterfallPlot::plotBeam() - got NULL beam, ignoring"
         << endl;
    return;
  }
  
  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "======== Waterfall - plotting beam data ================" << endl;
    DateTime beamTime(beam->getTimeSecs(), true, beam->getNanoSecs() * 1.0e-9);
    cerr << "  Beam time: " << beamTime.asString(3) << endl;
    cerr << "  Max range: " << beam->getMaxRange() << endl;
  }

  // draw the color scale
  
  if (_readColorMap() == 0) {
    _zoomWorld.drawColorScale(_cmap, painter,
                              _params.waterfall_color_scale_font_size);
  }
  
  // perform the relevant plot

  switch (_plotType) {

    case Params::WATERFALL_HC:
      _plotHc(painter, beam, nSamples, selectedRangeKm);
      break;
    case Params::WATERFALL_VC:
      _plotHc(painter, beam, nSamples, selectedRangeKm);
      break;
    case Params::WATERFALL_HX:
      _plotHc(painter, beam, nSamples, selectedRangeKm);
      break;
    case Params::WATERFALL_VX:
      _plotHc(painter, beam, nSamples, selectedRangeKm);
      break;
    case Params::WATERFALL_ZDR:
      _plotHc(painter, beam, nSamples, selectedRangeKm);
      break;
    case Params::WATERFALL_PHIDP:
      _plotHc(painter, beam, nSamples, selectedRangeKm);
      break;
      
  }

  // draw the overlays

  _drawOverlays(painter, selectedRangeKm);

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
                            Beam *beam,
                            int nSamples,
                            double selectedRangeKm)
  
{


  int nGates = beam->getNGates();
  double startRange = beam->getStartRangeKm();
  double gateSpacing = beam->getGateSpacingKm();

  painter.save();

  // loop through the gates
  
  for (int igate = 0; igate < nGates; igate++) {

    // set limits for plotting this gate
    
    double yy = startRange + gateSpacing * (igate-0.5);
    
    // get Iq data for this gate

    const GateData *gateData = beam->getGateData()[igate];
    TaArray<RadarComplex_t> iq_;
    RadarComplex_t *iq = iq_.alloc(nSamples);
    memcpy(iq, gateData->iqhcOrig, nSamples * sizeof(RadarComplex_t));
    
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

    // compute dbm
    
    TaArray<double> dbm_;
    double *dbm = dbm_.alloc(nSamples);

    for (int ii = 0; ii < nSamples; ii++) {
      double power = RadarComplex::power(powerSpec[ii]);
      dbm[ii] = 10.0 * log10(power);
      if (power <= 1.0e-12) {
        dbm[ii] = -120.0;
      }
    }

    // apply 3-pt median filter

    FilterUtils::applyMedianFilter(dbm, nSamples, 3);
      
    // plot the samples
    
    for (int ii = 0; ii < nSamples; ii++) {
      
      // get color

      int red, green, blue;
      _cmap.dataColor(dbm[ii], red, green, blue);
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
    case Params::WATERFALL_ZDR:
      return "ZDR";
    case Params::WATERFALL_PHIDP:
      return "PHIDP";
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
    case Params::WATERFALL_ZDR:
      return "dB";
    case Params::WATERFALL_PHIDP:
      return "deg";
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
 * set the zoom limits, from pixel space
 */

void WaterfallPlot::setZoomLimits(int xMin,
                                  int yMin,
                                  int xMax,
                                  int yMax)
{
  _zoomWorld.setZoomLimits(xMin, yMin, xMax, yMax);
  _isZoomed = true;
}

void WaterfallPlot::setZoomLimitsX(int xMin,
                                   int xMax)
{
  _zoomWorld.setZoomLimitsX(xMin, xMax);
  _isZoomed = true;
}

void WaterfallPlot::setZoomLimitsY(int yMin,
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
  
  painter.setPen(_params.waterfall_selected_range_color);
  _zoomWorld.drawLine(painter,
                      _zoomWorld.getXMinWorld(), selectedRangeKm,
                      _zoomWorld.getXMaxWorld(), selectedRangeKm);

  painter.restore();

}

//////////////////////////////////////////////////
// read color map
// returns 0 on success, -1 on failure
  
int WaterfallPlot::_readColorMap()
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

  // get color scale name

  string colorScaleName = _params._waterfall_plots[_id].color_scale;

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
  
