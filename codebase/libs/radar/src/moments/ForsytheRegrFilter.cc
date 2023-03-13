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
///////////////////////////////////////////////////////////////
// ForsytheRegrFilter.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000,
// Boulder, CO, 80307-3000, USA
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

#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <toolsa/toolsa_macros.h>
#include <toolsa/mem.h>
#include <toolsa/TaArray.hh>
#include <radar/ForsytheRegrFilter.hh>
#include <rapmath/usvd.h>
using namespace std;

// Constructor

ForsytheRegrFilter::ForsytheRegrFilter()

{
  _init();
}

/////////////////////////////
// Copy constructor
//

ForsytheRegrFilter::ForsytheRegrFilter(const ForsytheRegrFilter &rhs)

{
  if (this != &rhs) {
    _copy(rhs);
  }
}

// destructor

ForsytheRegrFilter::~ForsytheRegrFilter()

{

  for (size_t ii = 0; ii < ORDER_ARRAY_MAX; ii++) {
    for (size_t jj = 0; jj < NSAMPLES_ARRAY_MAX; jj++) {
      if (_forsytheArray[ii][jj] != NULL) {
        delete _forsytheArray[ii][jj];
      }
    } // jj
  } // ii

}

/////////////////////////////
// Assignment
//

ForsytheRegrFilter &ForsytheRegrFilter::operator=(const ForsytheRegrFilter &rhs)

{
  return _copy(rhs);
}

/////////////////////////////
// initialization

void ForsytheRegrFilter::_init()

{

  _setupDone = false;

  _nSamples = 0;
  _orderAuto = true;
  _polyOrder = 5;
  
  _isStaggered = false;
  _staggeredM = 0;
  _staggeredN = 0;
  
  _clutterWidthFactor = 1.0;
  _cnrExponent = 2.0 / 3.0;
  
  // prepare the array of forsythe fit objects,
  // for supported orders and nsamples
  // this is done for efficiency

  for (size_t ii = 0; ii < ORDER_ARRAY_MAX; ii++) {
    vector<ForsytheFit *> row;
    for (size_t jj = 0; jj < NSAMPLES_ARRAY_MAX; jj++) {
      row.push_back(NULL);
    }
    _forsytheArray.push_back(row);
  }

}

/////////////////////////////
// copy
//

ForsytheRegrFilter &ForsytheRegrFilter::_copy(const ForsytheRegrFilter &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  _init();

  // copy simple members
  
  _nSamples = rhs._nSamples;
  _orderAuto = rhs._orderAuto;
  _polyOrder = rhs._polyOrder;
  
  _isStaggered = rhs._isStaggered;
  _staggeredM = rhs._staggeredM;
  _staggeredN = rhs._staggeredN;

  _setupDone = rhs._setupDone;

  _clutterWidthFactor = rhs._clutterWidthFactor;
  _cnrExponent = rhs._cnrExponent;

  // copy the arrays
  
  _xxVals = rhs._xxVals;
  _polyfitIqVals = rhs._polyfitIqVals;
  
  return *this;

}

/////////////////////////////////////////////////////
// set up regression parameters - fixed PRT
//
// nSamples: number of samples in IQ time series
// polyOrder: order of polynomial for regression
//
// If successful, _setupDone will be set to true.
// If not successful, _setupDone will be set to false.
// Failure occurs if it is not possible to compute the
// SVD of vvA.

void ForsytheRegrFilter::setup(size_t nSamples)
  
{

  if (_setupDone && _nSamples == nSamples) {
    return;
  }

  _nSamples = nSamples;

  _isStaggered = false;
  _staggeredM = 0;
  _staggeredN = 0;

  // allocate arrays

  _alloc();
  
  // load x vector

  double xDelta = 1.0 / _nSamples;
  double xx = -0.5;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    _xxVals[ii] = xx;
    xx += xDelta;
  }
  
  // done

  _setupDone = true;

}

/////////////////////////////////////////////////////
// set up regression parameters - staggered PRT
//
// nSamples: number of samples in IQ time series
// staggeredM, staggeredN - stagger ratio = M/N
//   time series starts with short PRT
// polyOrder: order of polynomial for regression
//
// If successful, _setupDone will be set to true.
// If not successful, _setupDone will be set to false.
// Failure occurs if it is not possible to compute the
// SVD of vvA.

void ForsytheRegrFilter::setupStaggered(size_t nSamples,
                                        int staggeredM,
                                        int staggeredN)
  
{
  
  if (_setupDone &&
      _isStaggered &&
      _nSamples == nSamples &&
      _staggeredM == staggeredM &&
      _staggeredN == staggeredN) {
    return;
  }

  _nSamples = nSamples;
  _isStaggered = true;
  _staggeredM = staggeredM;
  _staggeredN = staggeredN;
  
  // allocate arrays

  _alloc();

  // load x vector

  int nStaggered = (_nSamples / 2) * (_staggeredM + _staggeredN);
  double xDelta = 1.0 / nStaggered;
  double xx = -0.5;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    _xxVals[ii] = xx;
    if (ii % 2 == 0) {
      xx += xDelta * _staggeredM;
    } else {
      xx += xDelta * _staggeredN;
    }
  }

  // done

  _setupDone = true;

}

/////////////////////////////////////////////////////
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

void ForsytheRegrFilter::apply(const RadarComplex_t *rawIq,
                               double cnr3Db,
                               double antennaRateDegPerSec,
                               double prtSecs,
                               double wavelengthM,
                               RadarComplex_t *filteredIq)
  
{

  assert(_nSamples != 0);
  assert(_setupDone);

  // copy IQ data

  vector<double> rawI, rawQ;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    rawI.push_back(rawIq[ii].re);
    rawQ.push_back(rawIq[ii].im);
  }

  if (_orderAuto) {

    // automatically compute the order to be used (from Meymaris 2021)
    
    if (cnr3Db < 1) {
      cnr3Db = 1.0;
    }
    double ss = _clutterWidthFactor;
    double wc = ss * (0.03 + 0.017 * antennaRateDegPerSec);
    double nyquist = wavelengthM / (4.0 * prtSecs);
    double wcNorm = wc / nyquist;
    double orderNorm = -1.9791 * wcNorm * wcNorm + 0.6456 * wcNorm;
    int order = ceil(orderNorm * pow(cnr3Db, _cnrExponent) * _nSamples);
    if (order < 3) {
      order = 3;
    }
  
    // cerr << "rate, cnr, wc, wcNorm, orderNorm, order: "
    //      << setw(6) << antennaRateDegPerSec << ", "
    //      << setw(6) << cnr3Db << ", "
    //      << setw(6) << wc << ", "
    //      << setw(6) << wcNorm << ", "
    //      << setw(6) << orderNorm << ", "
    //      << setw(3) << order << endl;
  
    _polyOrder = order;

  }

  // find the entry in the forsythe array, if possible
  
  ForsytheFit *fit = &_forsythe;
  
  if (_polyOrder < ORDER_ARRAY_MAX &&
      _nSamples < NSAMPLES_ARRAY_MAX) {

    // re-use objects
    // check for existing entry in array of fit objects
    
    fit = _forsytheArray[_polyOrder][_nSamples];
    if (fit == NULL) {
      // create new object for (order, nsamples)
      fit = new ForsytheFit;
      fit->prepareForFit(_polyOrder, _xxVals);
      _forsytheArray[_polyOrder][_nSamples] = fit;
    }
    fit->performFit(rawI);
    
  } else {

    // use single object

    fit->prepareForFit(_polyOrder, _xxVals);
    
  }

  // perform the fit on I and Q and
  // compute the estimated I/Q polynomial values
  // load residuals into filtered Iq
  
  fit->performFit(rawI);
  vector<double> iSmoothed = fit->getYEstVector();
  for (size_t ii = 0; ii < _nSamples; ii++) {
    filteredIq[ii].re = rawI[ii] - iSmoothed[ii];
    _polyfitIqVals[ii].re = iSmoothed[ii];
  }
  
  fit->performFit(rawQ);
  vector<double> qSmoothed = fit->getYEstVector();
  for (size_t ii = 0; ii < _nSamples; ii++) {
    filteredIq[ii].im = rawQ[ii] - qSmoothed[ii];
    _polyfitIqVals[ii].im = qSmoothed[ii];
  }

}

/////////////////////////////////////////////////////
// allocate space

void ForsytheRegrFilter::_alloc()

{
  
  _xxVals.resize(_nSamples);
  _polyfitIqVals.resize(_nSamples);
  
}

/////////////////////////////////////////////////////////////////////////
// compute the power from the central 3 points in the FFT

double ForsytheRegrFilter::compute3PtClutPower(const RadarComplex_t *rawIq)
  
{
  
  double sumPower = 0.0;
  
  for (size_t kk = 0; kk < 3; kk++) {
    if (kk == 2) {
      kk = _nSamples - 1;
    }
    double sumReal = 0.0;
    double sumImag = 0.0;
    for (size_t tt = 0; tt < _nSamples; tt++) {  // For each input element
      double angle = 2.0 * M_PI * tt * kk / _nSamples;
      sumReal +=  rawIq[tt].re * cos(angle) + rawIq[tt].im * sin(angle);
      sumImag += -rawIq[tt].re * sin(angle) + rawIq[tt].im * cos(angle);
    }
    double power = (sumReal * sumReal + sumImag * sumImag) / _nSamples;
    sumPower += power;
  }

  return sumPower / 3.0;

}

