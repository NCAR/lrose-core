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
// WaterfallPlot.hh
//
// Plotting of spectra vs range
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2021
//
///////////////////////////////////////////////////////////////
#ifndef WaterfallPlot_HH
#define WaterfallPlot_HH

#include <string>
#include <vector>

#include <QWidget>
#include <toolsa/TaArray.hh>
#include <radar/RadarComplex.hh>

#include "Params.hh"
#include "SpritePlot.hh"
#include "ColorMap.hh"

class Beam;
class MomentsFields;

/// Waterfall plotting

class WaterfallPlot : public SpritePlot
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * Constructor.
   */
  
  WaterfallPlot(QWidget *parent,
                const Params &params,
                int id);
  
  /**
   * @brief Destructor.
   */

  virtual ~WaterfallPlot();

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

  // set the plot details

  void setPlotType(Params::waterfall_type_t val) { _plotType = val; }

  // set filtering
  
  void setFftWindow(Params::fft_window_t val) { _fftWindow = val; }
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

  // plot a beam
  
  void plotBeam(QPainter &painter,
                Beam *beam,
                size_t nSamples,
                double selectedRangeKm);

  // get the plot details
  
  const Params::waterfall_type_t getPlotType() const { return _plotType; }
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
  
  // get strings

  static string getName(Params::waterfall_type_t wtype);
  static string getUnits(Params::waterfall_type_t wtype);

  // missing value

  static double missingVal;

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  QWidget *_parent;
  const Params &_params;
  int _id;

  // moment type active for plotting

  Params::waterfall_type_t _plotType;
  
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

  // color map

  ColorMap _cmap;

  // phidp sdev

  bool _phidpFoldsAt90;
  double _phidpFoldVal, _phidpFoldRange;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  void _plotHc(QPainter &painter,
               Beam *beam,
               size_t nSamples,
               size_t nGates,
               double selectedRangeKm);
  
  void _plotVc(QPainter &painter,
               Beam *beam,
               size_t nSamples,
               size_t nGates,
               double selectedRangeKm);
  
  void _plotHx(QPainter &painter,
               Beam *beam,
               size_t nSamples,
               size_t nGates,
               double selectedRangeKm);
  
  void _plotVx(QPainter &painter,
               Beam *beam,
               size_t nSamples,
               size_t nGates,
               double selectedRangeKm);
  
  void _plotDbz(QPainter &painter,
                Beam *beam,
                size_t nSamples,
                size_t nGates,
                double selectedRangeKm);
  
  void _plotZdr(QPainter &painter,
                Beam *beam,
                size_t nSamples,
                size_t nGates,
                double selectedRangeKm);
  
  void _plotPhidp(QPainter &painter,
                  Beam *beam,
                  size_t nSamples,
                  size_t nGates,
                  double selectedRangeKm);
  
  void _plotRhohv(QPainter &painter,
                  Beam *beam,
                  size_t nSamples,
                  size_t nGates,
                  double selectedRangeKm);
  
  void _plotTdbz(QPainter &painter,
                 Beam *beam,
                 size_t nSamples,
                 size_t nGates,
                 double selectedRangeKm);
  
  void _plotSdevZdr(QPainter &painter,
                    Beam *beam,
                    size_t nSamples,
                    size_t nGates,
                    double selectedRangeKm);
  
  void _plotSdevPhidp(QPainter &painter,
                      Beam *beam,
                      size_t nSamples,
                      size_t nGates,
                      double selectedRangeKm);
  
  void _plotCmd(QPainter &painter,
                Beam *beam,
                size_t nSamples,
                size_t nGates,
                double selectedRangeKm);
  
  void _computePowerSpectrum(Beam *beam,
                             size_t nSamples,
                             const RadarComplex_t *iq,
                             double *power,
                             double *dbm);
  
  void _drawOverlays(QPainter &painter, double selectedRangeKm);

  int _readColorMap(string colorScaleName);

  void _applyWindow(const RadarComplex_t *iq, 
                    RadarComplex_t *iqWindowed,
                    size_t nSamples);
  
  double _computeSdevZdr(const vector<double> &zdr);
  double _computeSdevPhidp(const vector<double> &phidp);
  void _computePhidpFoldingRange(size_t nGates, size_t nSamples, 
                                 double **phidp2D);

};

#endif
