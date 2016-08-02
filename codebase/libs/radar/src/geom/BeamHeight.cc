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

// Constructor

BeamHeight::BeamHeight()
{
    
  _instHtKm = 0.0;
  _htKm = 0.0;
  _gndRangeKm = 0.0;
  _slantRangeKm = 0.0;
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

}

/////////////////////////////////////////////////////////////
// Compute elevation angle for given ground distance and ht.
// Side effect - sets slant range.
// Use getSlantRangeKm() to retrieve.

double BeamHeight::computeElevationDeg(double htKm, double gndRangeKm)

{

  _gndRangeKm = gndRangeKm;
  _htKm = htKm;
  
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


  double elRad = elDeg * DEG_TO_RAD;
  
  double sinEl, cosEl;
  ta_sincos(elRad, &sinEl, &cosEl);
  
  double term1 = slantRangeKm * slantRangeKm + _pseudoRadiusKmSq;
  double term2 = slantRangeKm * _pseudoDiamKm * sinEl;
  
  double htKm = _instHtKm - _pseudoRadiusKm + sqrt(term1 + term2);

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

