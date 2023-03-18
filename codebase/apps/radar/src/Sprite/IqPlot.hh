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

// #include <QDialog>
// #include <QWidget>
// #include <QResizeEvent>
// #include <QImage>
// #include <QTimer>
// #include <QRubberBand>
// #include <QPoint>
// #include <QTransform>

#include <toolsa/TaArray.hh>
#include <radar/RadarComplex.hh>
#include <radar/RadarMoments.hh>

#include "Params.hh"
#include "SpritePlot.hh"

class Beam;

class IqPlot : public SpritePlot
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
  
  virtual void clear();

  // set the window geometry
  
  virtual void setWindowGeom(int width,
                             int height,
                             int xOffset,
                             int yOffset);

  // set the world limits

  virtual void setWorldLimits(double xMinWorld,
                              double yMinWorld,
                              double xMaxWorld,
                              double yMaxWorld);

  void setWorldLimitsX(double xMinWorld,
                       double xMaxWorld);
  void setWorldLimitsY(double yMinWorld,
                       double yMaxWorld);

  // set the plot type and channel

  void setPlotType(Params::iq_plot_type_t val);
  void setRxChannel(Params::rx_channel_t val) { _rxChannel = val; }
  void setFftWindow(Params::fft_window_t val) { _fftWindow = val; }

  // set filtering
  
  void setMedianFiltLen(int val) { _medianFiltLen = val; }
  void setClutterFilterType(RadarMoments::clutter_filter_type_t val) {
    _clutterFilterType = val;
  }
  void setPlotClutModel(bool val) { _plotClutModel = val; }
  void setClutModelWidthMps(double val) { _clutModelWidthMps = val; }
  void setRegrOrder(int val) { _regrOrder = val; }
  void setRegrClutWidthFactor(double val) { _regrClutWidthFactor = val; }
  void setRegrCnrExponent(double val) { _regrCnrExponent = val; }
  void setRegrFiltNotchInterpMethod(RadarMoments::notch_interp_method_t val) {
    _regrNotchInterpMethod = val;
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

  // get the plot details

  const Params::iq_plot_type_t getPlotType() const { return _plotType; }
  const Params::rx_channel_t getRxChannelType() const { return _rxChannel; }
  const Params::fft_window_t getFftWindow() const { return _fftWindow; }

  // get the filter details
  
  int getMedianFiltLen() const { return _medianFiltLen; }
  RadarMoments::clutter_filter_type_t getClutterFilterType() const {
    return _clutterFilterType;
  }
  bool getPlotClutModel() const { return _plotClutModel; }
  double getClutModelWidthMps() const { return _clutModelWidthMps; }
  int getRegrOrder() const { return _regrOrder; }
  double getRegrClutWidthFactor() const { return _regrClutWidthFactor; }
  double getRegrCnrExponent() const { return _regrCnrExponent; }
  RadarMoments::notch_interp_method_t getRegrFiltNotchInterpMethod() const {
    return _regrNotchInterpMethod;
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
  RadarMoments::clutter_filter_type_t _clutterFilterType;
  bool _plotClutModel;
  double _clutModelWidthMps;
  int _regrOrder;
  int _regrOrderInUse;
  double _regrClutWidthFactor;
  double _regrCnrExponent;
  RadarMoments::notch_interp_method_t _regrNotchInterpMethod;
  bool _computePlotRangeDynamically;

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
                             RadarComplex_t *iqWindowed,
                             RadarComplex_t *iqFilt,
                             RadarComplex_t *iqNotched,
                             double *dbm,
                             double *dbmFilt,
                             double &filterRatio,
                             double &spectralNoise,
                             double &spectralSnr);
  
  void _applyWindow(const RadarComplex_t *iq, 
                    RadarComplex_t *iqWindowed,
                    int nSamples);
  
};

#endif
