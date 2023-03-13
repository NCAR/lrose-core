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
// ForsytheRegrFilter.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2023
//
///////////////////////////////////////////////////////////////
//
// Using forsyth polynomials, filter clutter by performing a
// polynomial regression fit to the time series and remove the
// smoothly-varing values, leaving the variation around
// the polynomial
//
// See: Torres S and D.S.Zrnic, 1999: Ground clutter filtering with
// a regression filter. Jtech, 16, 1364 - 1372.
//
// Hubbert, J. C., Meymaris, G., Romatschke, U., & Dixon, M. (2021).
// Improving signal statistics using a regression ground clutter filter.
// Part 1: Theory and simulations.
// Journal Of Atmospheric And Oceanic Technology, 38.
// doi:10.1175/JTECH-D-20-0026.1
//
// Estimation of the polynomial order is based on a simulation-based
// analysis by Greg Meymaris:
//
//    double clutterWidthFactor defaults to 1.0;
//    double cnrExponent defaults to 2/3;
//    double wc = clutterWidthFactor * (0.03 + 0.017 * antennaRateDegPerSec);
//    double nyquist = wavelengthM / (4.0 * prtSecs);
//    double wcNorm = wc / nyquist;
//    double orderNorm = -1.9791 * wcNorm * wcNorm + 0.6456 * wcNorm;
//    int order = ceil(orderNorm * pow(cnr3Db, cnrExponent) * nSamples);
//    if (order < 3) order = 3;
//
////////////////////////////////////////////////////////////////

#ifndef ForsytheRegrFilter_HH
#define ForsytheRegrFilter_HH

#include <radar/RadarComplex.hh>
#include <rapmath/ForsytheFit.hh>
#include <cstdio>
using namespace std;

////////////////////////
// This class

class ForsytheRegrFilter {
  
public:

  // constructor

  ForsytheRegrFilter();

  // copy constructor
  
  ForsytheRegrFilter(const ForsytheRegrFilter &rhs);

  // destructor
  
  ~ForsytheRegrFilter();

  // assignment
  
  ForsytheRegrFilter & operator=(const ForsytheRegrFilter &rhs);

  // set number of samples in IQ data and order of
  // polynomial to use
  //
  // nSamples: number of samples in IQ time series
  
  void setup(size_t nSamples);
  
  // set up regression parameters - staggered PRT
  //
  // nSamples: number of samples in IQ time series
  // staggeredM, staggeredN - stagger ratio = M/N
  //   time series starts with short PRT
  //
  // If successful, _setupDone will be set to true.
  // If not successful, _setupDone will be set to false.
  // Failure occurs if it is not possible to compute the
  // SVD of vvA.
  
  void setupStaggered(size_t nSamples,
                      int staggeredM,
                      int staggeredN);
  
  // Set the polynomial order.
  // If autoOrder is true, order will be computed automatically.
  // If autoOrder is false, specifiedOrder will be used.
  
  void setPolyOrder(bool autoOrder,
                    size_t specifiedOrder) {
    _orderAuto = autoOrder;
    _polyOrder = specifiedOrder;
  }
  
  // in automatic order computation, set the width factor
  
  void setClutterWidthFactor(double val) {
    _clutterWidthFactor = val;
  }

  // in automatic order computation, set the cnr exponent
  
  void setCnrExponent(double val) {
    _cnrExponent = val;
  }

  // Perform regression filtering on I,Q data
  // Note: assumes set() methods have been applied
  //
  // Inputs:
  //   rawIq: raw I,Q data
  //   cnr3Db: clutter-to-noise-ratio from center 3 spectral points
  //   antennaRateDegPerSec: antenna rate - higher rate widens clutter
  //   double prtSecs: PRT for the passed-in IQ values
  //   double wavelengthM: wanelength
  //
  // Outputs:
  //   filteredIq: filtered I,Q data
  //
  // Side effect:
  //   polyfitIq is computed
  //   retrieve with getPolyfitIq()
  
  void apply(const RadarComplex_t *rawIq,
             double cnr3Db,
             double antennaRateDegPerSec,
             double prtSecs,
             double wavelengthM,
             RadarComplex_t *filteredIq);
  
  // compute the power from the central 3 points in the FFT
  
  double compute3PtClutPower(const RadarComplex_t *rawIq);
  
  // Perform polynomial fit from observed data
  //
  // Input: yy - observed data
  //
  // Result:
  //   polynomial coefficients in _pp
  //
  // Note: assumes setup() has been successfully completed.

  void polyFit(const double *yy) const;

  // get methods

  inline size_t getNSamples() const { return _nSamples; }
  inline bool getOrderAuto() const { return _orderAuto; }
  inline size_t getPolyOrder() const { return _polyOrder; }

  inline bool isStaggered() const { return _isStaggered; }
  inline int getStaggeredM() const { return _staggeredM; }
  inline int getStaggeredN() const { return _staggeredN; }

  inline bool getSetupDone() const { return _setupDone; }
  
  inline double getX(size_t sampleNum) const {
    if (sampleNum < _nSamples) {
      return _xxVals[sampleNum];
    } else {
      return -9999;
    }
  }

  inline const double* getX() const { return _xxVals.data(); }
  inline const RadarComplex_t* getPolyfitIq() const {
    return _polyfitIqVals.data();
  }

protected:
private:

  static const size_t ORDER_ARRAY_MAX = 32;
  static const size_t NSAMPLES_ARRAY_MAX = 1024;
  
  // data
  
  bool _setupDone;

  size_t _nSamples;
  bool _orderAuto;     // determine the order from the clutter to signal ratio
  size_t _polyOrder;    // polynomial order

  bool _isStaggered; // staggered-PRT version
  int _staggeredM;
  int _staggeredN;

  double _clutterWidthFactor; // factor to allow us to increase order
  double _cnrExponent; // allows us to tune the width

  vector<double> _xxVals;
  vector<RadarComplex_t> _polyfitIqVals;

  // forsythe orthogonal polynomial object

  ForsytheFit _forsythe;

  // array of forsythe objects for efficiency
  // this allows us to re-use objects that have been
  // previously set up

  vector<vector<ForsytheFit *>> _forsytheArray;
  
  // methods

  ForsytheRegrFilter &_copy(const ForsytheRegrFilter &rhs);
  void _init();
  void _alloc();

};

#endif

