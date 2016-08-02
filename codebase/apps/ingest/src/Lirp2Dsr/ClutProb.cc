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
// ClutProb.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2005
//
////////////////////////////////////////////////////////////////

#include "ClutProb.hh"
#include "ClutFilter.hh"
#include <cmath>
#include <iostream>
using namespace std;

// initialize const doubles

const double ClutProb::missingDbl = -9999.0;

// Constructor

ClutProb::ClutProb() :
        _powerNarrow(missingDbl),
        _ratioNarrow(missingDbl),
        _ratioWide(missingDbl),
        _clutterFound(false),
        _clutterPos(0),
        _weatherPos(0),
        _clutterPeak(0),
        _weatherPeak(0),
        _clutWxPeakRatio(missingDbl),
        _clutWxPeakSeparation(missingDbl)
  
{
  
}

// destructor

ClutProb::~ClutProb()

{
}

// combine two probability objects

void ClutProb::combine(const ClutProb &prob1,
                       const ClutProb &prob2)
                                 
{

  _powerNarrow = (prob1._powerNarrow + prob2._powerNarrow) / 2.0;
  _ratioNarrow = (prob1._ratioNarrow + prob2._ratioNarrow) / 2.0;
  _ratioWide = (prob1._ratioWide + prob2._ratioWide) / 2.0;

  if (prob1._clutterFound && prob2._clutterFound) {
    _clutterFound = true;
    _clutterPos =
      (prob1._clutterPos + prob2._clutterPos) / 2;
    _clutterPeak =
      (prob1._clutterPeak + prob2._clutterPeak) / 2.0;
    _weatherPos =
      (prob1._weatherPos + prob2._weatherPos) / 2;
    _weatherPeak =
      (prob1._weatherPeak + prob2._weatherPeak) / 2.0;
    _clutWxPeakRatio =
      (prob1._clutWxPeakRatio + prob2._clutWxPeakRatio) / 2.0;
    _clutWxPeakSeparation =
      (prob1._clutWxPeakSeparation + prob2._clutWxPeakSeparation) / 2.0;
  } else if (prob1._clutterFound) {
    _clutterFound = true;
    _clutterPos = prob1._clutterPos;
    _clutterPeak = prob1._clutterPeak;
    _weatherPos = prob1._weatherPos;
    _weatherPeak = prob1._weatherPeak;
    _clutWxPeakRatio = prob1._clutWxPeakRatio;
    _clutWxPeakSeparation = prob1._clutWxPeakSeparation;
  } else if (prob2._clutterFound) {
    _clutterFound = true;
    _clutterPos = prob2._clutterPos;
    _clutterPeak = prob2._clutterPeak;
    _weatherPos = prob2._weatherPos;
    _weatherPeak = prob2._weatherPeak;
    _clutWxPeakRatio = prob2._clutWxPeakRatio;
    _clutWxPeakSeparation = prob2._clutWxPeakSeparation;
  } else {
    _clutterFound = false;
    _clutterPos = 0;
    _clutterPeak = missingDbl;
    _weatherPos = 0;
    _weatherPeak = missingDbl;
    _clutWxPeakRatio = missingDbl;
    _clutWxPeakSeparation = missingDbl;
  }

}

////////////////////////////////////////////////////////
// compute probability of clutter, based on the ratio of
// power near 0 to power away from 0.

void ClutProb::compute(int nSamples,
                       const double *magnitude,
                       double nyquist,
                       double maxClutterVel,
                       double initNotchWidth)
  
{

  // compute power
  
  double power[nSamples];
  const double *mag = magnitude;
  double *pp = power;
  for (int ii = 0; ii < nSamples; ii++, mag++, pp++) {
    *pp = *mag * *mag;
  }
  
  // sum up powers in each of 3 regions:
  //  region 1: from ii of -1 to 1
  //  region 2: from ii of -3 to 3
  //  region 3: everywhere

  int limit1 = 1;
  int limit2 = 3;
  double sum1 = 0.0;
  double sum2 = 0.0;
  double sum3 = 0.0;
  
  pp = power;
  for (int ii = 0; ii < nSamples; ii++, pp++) {
    double pwr = *pp;
    int jj = nSamples - 1 - ii;
    if (ii <= limit1 || jj <= limit1) {
      sum1 += pwr;
    }
    if (ii <= limit2 || jj <= limit2) {
      sum2 += pwr;
    }
    sum3 += pwr;
  } // ii
  
  double power1 = sum1;
  double power2 = sum2 - sum1;
  double power3 = sum3 - sum1;

  double ratio12 = 10.0 * log10(power1 / power2);
  double ratio13 = 10.0 * log10(power1 / power3);
  
  _powerNarrow = sum2 / nSamples;
  _ratioNarrow = ratio12;
  _ratioWide = ratio13;
  
  // find clutter peaks etc

  int notchWidth;
  
  ClutFilter::locateWxAndClutter(power,
                                 nSamples,
                                 maxClutterVel,
                                 initNotchWidth,
                                 nyquist,
                                 notchWidth,
                                 _clutterFound,
                                 _clutterPos,
                                 _clutterPeak,
                                 _weatherPos,
                                 _weatherPeak);

  if (_clutterFound) {

    _clutWxPeakRatio = 10.0 * log10((_clutterPeak / _weatherPeak));

    if (_clutWxPeakRatio < 20) {
      double separation =
        fabs((double) _clutterPos - _weatherPos) / ((double) nSamples / 2.0);
      if (separation > 1.0) {
        separation = 2.0 - separation;
      }
      _clutWxPeakSeparation = separation;
    }
    
  }

  
}

