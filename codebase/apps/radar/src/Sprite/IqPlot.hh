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
// IqPlot.hh
//
// Plotting of IQ data, as time series and spectra.
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////
#ifndef IqPlot_HH
#define IqPlot_HH

#include <string>
#include <vector>
#include <map>

#include <QDialog>
#include <QWidget>
#include <QResizeEvent>
#include <QImage>
#include <QTimer>
#include <QRubberBand>
#include <QPoint>
#include <QTransform>

#include <toolsa/TaArray.hh>
#include <radar/RadarComplex.hh>
#include "Params.hh"
#include "ScaledLabel.hh"
#include "WorldPlot.hh"

class Beam;
class MomentsFields;

class IqPlot
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * Constructor.
   */
  
  IqPlot(QWidget *parent,
         const Params &params,
         int id);
  
  /**
   * @brief Destructor.
   */

  virtual ~IqPlot();

  /**
   * Clear the plot
   */
  
  void clear();

  // set the window geometry
  
  void setWindowGeom(int width,
                     int height,
                     int xOffset,
                     int yOffset);

  // set the world limits

  void setWorldLimits(double xMinWorld,
                      double yMinWorld,
                      double xMaxWorld,
                      double yMaxWorld);

  void setWorldLimitsX(double xMinWorld,
                       double xMaxWorld);
  void setWorldLimitsY(double yMinWorld,
                       double yMaxWorld);

  // set the zoom limits, using pixel space

  void setZoomLimits(int xMin,
                     int yMin,
                     int xMax,
                     int yMax);
  
  void setZoomLimitsX(int xMin,
                      int xMax);

  void setZoomLimitsY(int yMin,
                      int yMax);

  // set the plot type and channel

  void setPlotType(Params::iq_plot_type_t val);
  void setRxChannel(Params::rx_channel_t val) { _rxChannel = val; }
  void setFftWindow(Params::fft_window_t val) { _fftWindow = val; }

  // set filtering
  
  void setMedianFiltLen(int val) { _medianFiltLen = val; }
  void setUseAdaptFilt(bool val) { _useAdaptFilt = val; }
  void setPlotClutModel(bool val) { _plotClutModel = val; }
  void setClutModelWidthMps(double val) { _clutModelWidthMps = val; }
  void setUseRegrFilt(bool val) { _useRegrFilt = val; }
  void setRegrOrder(int val) { _regrOrder = val; }
  void setRegrClutWidthFactor(double val) { _regrClutWidthFactor = val; }
  void setRegrFiltInterpAcrossNotch(bool val) {
    _regrFiltInterpAcrossNotch = val;
  }
  void setComputePlotRangeDynamically(bool val) {
    _computePlotRangeDynamically = val;
  }

  // zooming

  void zoom(int x1, int y1, int x2, int y2);
  void unzoom();

  // plot a beam
  
  void plotBeam(QPainter &painter,
                Beam *beam,
                size_t nSamples,
                double selectedRangeKm);

  // set grid lines on/off

  void setXGridLinesOn(bool val) { _xGridLinesOn = val; }
  void setYGridLinesOn(bool val) { _yGridLinesOn = val; }
  
  // legends
  
  void setLegendsOn(bool val) { _legendsOn = val; }

  // get the world plot objects
  
  WorldPlot &getFullWorld() { return _fullWorld; }
  WorldPlot &getZoomWorld() { return _zoomWorld; }
  bool getIsZoomed() const { return _isZoomed; }

  // get the window geom

  int getWidth() const { return _fullWorld.getWidthPixels(); }
  int getHeight() const { return _fullWorld.getHeightPixels(); }
  int getXOffset() const { return _fullWorld.getXPixOffset(); }
  int getYOffset() const { return _fullWorld.getYPixOffset(); }
  
  // get grid lines state

  bool getXGridLinesOn() const { return _xGridLinesOn; }
  bool getYGridLinesOn() const { return _yGridLinesOn; }
  
  // legends

  bool getLegendsOn() const { return _legendsOn; }
  
  // get the plot details

  const Params::iq_plot_type_t getPlotType() const { return _plotType; }
  const Params::rx_channel_t getRxChannelType() const { return _rxChannel; }
  const Params::fft_window_t getFftWindow() const { return _fftWindow; }

  // get the filter details
  
  int getMedianFiltLen() const { return _medianFiltLen; }
  bool getUseAdaptFilt() const { return _useAdaptFilt; }
  bool getPlotClutModel() const { return _plotClutModel; }
  double getClutModelWidthMps() const { return _clutModelWidthMps; }
  bool getUseRegrFilt() const { return _useRegrFilt; }
  int getRegrOrder() const { return _regrOrder; }
  double getRegrClutWidthFactor() const { return _regrClutWidthFactor; }
  bool getRegrFiltInterpAcrossNotch() const {
    return _regrFiltInterpAcrossNotch;
  }
  bool getComputePlotRangeDynamically() const {
    return _computePlotRangeDynamically;
  }

  // get strings

  string getName();
  string getXUnits();
  string getYUnits();
  string getFftWindowName();
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  QWidget *_parent;
  const Params &_params;
  int _id;

  // beam to plot
  
  Beam *_beam;
  size_t _nSamples;

  // plot type and channel

  Params::iq_plot_type_t _plotType;
  Params::rx_channel_t _rxChannel;
  Params::iq_plot_static_range_t _staticRange;

  // fft window
  
  Params::fft_window_t _fftWindow;
  TaArray<double> _windowCoeff_;
  double *_windowCoeff;

  // filtering

  int _medianFiltLen;
  bool _useAdaptFilt;
  bool _plotClutModel;
  double _clutModelWidthMps;
  bool _useRegrFilt;
  int _regrOrder;
  int _regrOrderInUse;
  double _regrClutWidthFactor;
  bool _regrFiltInterpAcrossNotch;
  bool _computePlotRangeDynamically;

  // unzoomed world

  WorldPlot _fullWorld;

  // zoomed world

  bool _isZoomed;
  WorldPlot _zoomWorld;

  // grid lines

  bool _xGridLinesOn;
  bool _yGridLinesOn;
  
  // legends

  bool _legendsOn;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  // draw the overlays
  
  void _drawOverlays(QPainter &painter, double selectedRangeKm);
  
  void _plotSpectralPower(QPainter &painter,
                          double selectedRangeKm,
                          int gateNum,
                          const RadarComplex_t *iq);

  void _plotSpectralPhase(QPainter &painter,
                          double selectedRangeKm,
                          int gateNum,
                          const RadarComplex_t *iq);

  void _plotSpectralZdr(QPainter &painter,
                        double selectedRangeKm,
                        int gateNum,
                        const RadarComplex_t *iqHc,
                        const RadarComplex_t *iqVc);

  void _plotSpectralPhidp(QPainter &painter,
                          double selectedRangeKm,
                          int gateNum,
                          const RadarComplex_t *iqHc,
                          const RadarComplex_t *iqVc);

  void _plotTsPower(QPainter &painter,
                    double selectedRangeKm,
                    int gateNum,
                    const RadarComplex_t *iq);
  
  void _plotTsPhase(QPainter &painter,
                    double selectedRangeKm,
                    int gateNum,
                    const RadarComplex_t *iq);

  void _plotIQVals(QPainter &painter,
                   double selectedRangeKm,
                   int gateNum,
                   const double *iVals, 
                   const double *qVals);
  
  void _plotIvsQ(QPainter &painter,
                 double selectedRangeKm,
                 int gateNum,
                 const RadarComplex_t *iq);

  void _plotPhasor(QPainter &painter,
                   double selectedRangeKm,
                   int gateNum,
                   const RadarComplex_t *iq);

  void _computePowerSpectrum(const RadarComplex_t *iq,
                             double *power,
                             double *dbm);
  
  void _applyWindow(const RadarComplex_t *iq, 
                    RadarComplex_t *iqWindowed,
                    int nSamples);
  
};

#endif
