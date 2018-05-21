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
// BeamHeight.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// Provide estimate of atmospheric attenuation for elev / range
//
////////////////////////////////////////////////////////////////

#include <toolsa/sincos.h>
#include <toolsa/toolsa_macros.h>
#include <radar/BeamHeight.hh>
#include <cmath>
#include <iostream>

using namespace std;

const double BeamHeight::_earthRadiusKm = 6375.636;
const double BeamHeight::_htMissing = -9999.0;

// Constructor

BeamHeight::BeamHeight()
{
    
  _instHtKm = 0.0;
  _htKm = 0.0;
  _gndRangeKm = 0.0;
  _slantRangeKm = 0.0;
  _htCache = NULL;
  setPseudoRadiusRatio(4.0 / 3.0); // standard ratio
  
}

// destructor

BeamHeight::~BeamHeight()
  
{

}

/////////////////////////////////////////////////////////////
// Set the pseudo radius ratio

void BeamHeight::setPseudoRadiusRatio(double ratio)

{
  
  _pseudoRadiusRatio = ratio;
  _pseudoRadiusKm = _earthRadiusKm * _pseudoRadiusRatio;
  _pseudoRadiusKmSq = _pseudoRadiusKm * _pseudoRadiusKm;
  _pseudoDiamKm = _pseudoRadiusKm * 2.0;
  if (_htCache != NULL) {
    setHtCacheToMissing();
  }

}

/////////////////////////////////////////////////////////////
// Compute elevation angle for given ground distance and ht.
// Side effect - sets slant range.
// Use getSlantRangeKm() to retrieve.

double BeamHeight::computeElevationDeg(double htKm, double gndRangeKm)

{

  _gndRangeKm = gndRangeKm;
  if (htKm == 0.0) {
    // prevent degenerative case
    _htKm = 0.00001;
  } else {
    _htKm = htKm;
  }
  
  // cannot solve directly, we use the secant gradient search
  // method

  double xx_n_2 = atan2(_htKm, _gndRangeKm);
  double xx_n_1 = xx_n_2 * 0.9;
  
  double fx_n_2 = _fx(xx_n_2);
  double fx_n_1 = _fx(xx_n_1);

  for (int ii = 0; ii < 1000; ii++) {
    
    if (fabs(fx_n_1 - fx_n_2) < 1.0e-9) {
      break;
    }
    
    double xx_n =
      xx_n_1 - (fx_n_1 * (xx_n_1 - xx_n_2) / (fx_n_1 - fx_n_2));

    xx_n_2 = xx_n_1;
    xx_n_1 = xx_n;
    
    fx_n_2 = fx_n_1;
    fx_n_1 = _fx(xx_n_1);
    
  } // ii

  return xx_n_1 * RAD_TO_DEG;

}

//////////////////////////////////////////////////////////
// compute ht from elevation angle and slant range
//
// Side effect: sets ground range
// Use getGndRangeKm() to retrieve.

double BeamHeight::computeHtKm(double elDeg, double slantRangeKm) const
{
  
  if (_htCache == NULL) {
    // no active cache, compute and return
    return _computeHtKm(elDeg, slantRangeKm);
  }
  
  // find location in cache

  int iel = (int) ((elDeg - _htCacheStartElevDeg) / _htCacheDeltaElevDeg + 0.5);
  if (iel < 0 || iel >= _htCache_.sizeMajor()) {
    // outside valid range of cache, compute on the fly
    return _computeHtKm(elDeg, slantRangeKm);
  }
  int irng = (int) ((elDeg - _htCacheStartElevDeg) / _htCacheDeltaElevDeg + 0.5);
  if (irng < 0 || irng >= _htCache_.sizeMinor()) {
    // outside valid range of cache, compute on the fly
    return _computeHtKm(elDeg, slantRangeKm);
  }

  // if cached value is no missing, use it

  double cachedHt = _htCache[iel][irng];
  if (cachedHt != _htMissing) {
    return cachedHt;
  }

  // cached value is missing, so compute it now

  double htKm = _computeHtKm(elDeg, slantRangeKm);
  _htCache[iel][irng] = htKm;

  // compute gnd range

  _gndRangeKm  = slantRangeKm * cos(elDeg * DEG_TO_RAD);

  // return computed value

  return htKm;
  
}

double BeamHeight::_computeHtKm(double elDeg, double slantRangeKm) const
{


  double elRad = elDeg * DEG_TO_RAD;
  
  double sinEl, cosEl;
  ta_sincos(elRad, &sinEl, &cosEl);
  
  double term1 = slantRangeKm * slantRangeKm + _pseudoRadiusKmSq;
  double term2 = slantRangeKm * _pseudoDiamKm * sinEl;
  
  double htKm = _instHtKm - _pseudoRadiusKm + sqrt(term1 + term2);
  _gndRangeKm  = slantRangeKm * cosEl;

  return htKm;

}

////////////////////////////////////////////////////////////////
// Compute function for solving using secant method.
// At the root this function has a value of 0.

double BeamHeight::_fx(double elRad)

{
  return (_htKm - _ht(elRad));
}

////////////////////////////////////////////////////////////////
// compute ht for given elevation in radians
// for the ground range previously set

double BeamHeight::_ht(double elRad)
  
{
  
  double sinEl, cosEl;
  ta_sincos(elRad, &sinEl, &cosEl);
  
  _slantRangeKm = _gndRangeKm / cosEl;
  double term1 = _slantRangeKm * _slantRangeKm + _pseudoRadiusKmSq;
  double term2 = _slantRangeKm * _pseudoDiamKm * sinEl;
  
  double htKm = _instHtKm - _pseudoRadiusKm + sqrt(term1 + term2);
  
  return htKm;

}

////////////////////////////////////////////////////////////////////////
// Initialize the cache, if desired.
//
// If the cache is in use, we store computed heights in the cache to
// improve performance. Subsequence requests for elev/range coords
// that have been used previously will return the previously computed
// heights.
// If using the cache, it is important to use a sufficiently fine
// granularity to provide results of the desired accuracy.

void BeamHeight::initHtCache(size_t nElev, 
                             double startElevDeg,
                             double deltaElevDeg,
                             size_t nRange, 
                             double startRangeKm,
                             double deltaRangeKm) const

{

  if (nElev == 0 || nRange == 0) {
    // zero size
    freeHtCache();
    return;
  }

  if (nElev == _htCacheNElev &&
      nRange == _htCacheNRange &&
      fabs(startElevDeg - _htCacheStartElevDeg) < 0.001 &&
      fabs(deltaElevDeg - _htCacheDeltaElevDeg) < 0.001 &&
      fabs(startRangeKm - _htCacheStartRangeKm) < 0.001 &&
      fabs(deltaRangeKm - _htCacheDeltaRangeKm) < 0.001) {
    // no change
    return;
  }

  // allocate space

  _htCache = _htCache_.alloc(nElev, nRange);

  // save details

  _htCacheNElev = nElev;
  _htCacheStartElevDeg = startElevDeg;
  _htCacheDeltaElevDeg = deltaElevDeg;
  _htCacheNRange = nRange;
  _htCacheStartRangeKm = startRangeKm;
  _htCacheDeltaRangeKm = deltaRangeKm;

  // initialze to missing

  setHtCacheToMissing();

}

//////////////////////////////////////////////////////
// initialize a previously allocated cache to missing

void BeamHeight::setHtCacheToMissing() const

{

  if (_htCache == NULL) {
    return;
  }

  // initialze to missing

  for (size_t ielev = 0; ielev < _htCacheNElev; ielev++) {
    for (size_t irange = 0; irange < _htCacheNRange; irange++) {
      _htCache[ielev][irange] = _htMissing;
    } // irange
  } // ielev

}

////////////////////////////////////////////////////////////////////////
// Free the cache, set cache pointer to NULL

void BeamHeight::freeHtCache() const
{
  if (_htCache == NULL) {
    return;
  }
  _htCache_.free();
  _htCache = NULL;
  _htCacheNElev = 0;
  _htCacheStartElevDeg = 0;
  _htCacheDeltaElevDeg = 0;
  _htCacheNRange = 0;
  _htCacheStartRangeKm = 0;
  _htCacheDeltaRangeKm = 0;
}

