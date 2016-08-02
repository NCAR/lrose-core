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
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <toolsa/TaArray.hh>
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
        _clutWxPeakSeparation(missingDbl),
        _innerIndex(1),
        _outerIndex(3)
  
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

  _powerRatio = (prob1._powerRatio + prob2._powerRatio) / 2.0;
  _powerNarrow = (prob1._powerNarrow + prob2._powerNarrow) / 2.0;
  _prNarrow = (prob1._prNarrow + prob2._prNarrow) / 2.0;
  _ratioNarrow = (prob1._ratioNarrow + prob2._ratioNarrow) / 2.0;
  _prWide = (prob1._prWide + prob2._prWide) / 2.0;
  _ratioWide = (prob1._ratioWide + prob2._ratioWide) / 2.0;
  _clutSpectrumWidth = (prob1._clutSpectrumWidth +
                        prob2._clutSpectrumWidth) / 2.0;

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
                       double wavelengthMeters,
                       double prtSecs,
                       double maxClutterVel,
                       double initNotchWidth)
  
{

  // compute power
  
  TaArray<double> power_;
  double *power = power_.alloc(nSamples);
  double *pp = power;
  const double *mag = magnitude;
  for (int ii = 0; ii < nSamples; ii++, mag++, pp++) {
    double mm = *mag;
    *pp = mm * mm;
  }
  
  // sum up powers in each of 3 regions:
  //  region 1: from ii of -1 to 1
  //  region 2: from ii of -3 to 3
  //  region 3: everywhere

  int limit1 = _innerIndex;
  int limit2 = _outerIndex;
  double pwrDc = 0.0;
  double sum1 = 0.0;
  double sum2 = 0.0;
  double sumTotal = 0.0;
  
  const double *pwrp = power;
  for (int ii = 0; ii < nSamples; ii++, pwrp++) {
    double pwr = *pwrp;
    if (ii == 0) {
      pwrDc = pwr;
    }
    int jj = nSamples - 1 - ii;
    if (ii <= limit1 || jj <= limit1) {
      sum1 += pwr;
    }
    if (ii <= limit2 || jj <= limit2) {
      sum2 += pwr;
    }
    sumTotal += pwr;
  } // ii
  
  double power1 = sum1;
  double power2 = sum2 - sum1;
  double power3 = sumTotal - sum1;

  double ratio12 = 10.0 * log10(power1 / power2);
  double ratio13 = 10.0 * log10(power1 / power3);
  
  _powerRatio = pwrDc / sumTotal;
  _powerNarrow = sum2 / nSamples;
  _ratioNarrow = ratio12;
  _prNarrow = sum1 / sum2;
  _ratioWide = ratio13;
  _prWide = sum1 / sumTotal;

  // find clutter peaks etc

  int notchWidth;
  double nyquist = wavelengthMeters / (4.0 * prtSecs);
  
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

    //    if (_clutWxPeakRatio < 20) {
      double separation =
        fabs((double) _clutterPos - _weatherPos) / ((double) nSamples / 2.0);
      if (separation > 1.0) {
        separation = 2.0 - separation;
      }
      _clutWxPeakSeparation = separation;
      // }
    
  }

  // compute clutter spectrum width

  _computeClutWidth(nSamples, power, wavelengthMeters, prtSecs);

  // compute weather to noise ratio

  _computeWx2NoiseRatio(nSamples, power);
  
}

//////////////////////////////////////////////////////
// compute width centered on clutter

void ClutProb::_computeClutWidth(int nSamples,
                                 const double *power,
                                 double wavelengthMeters,
                                 double prtSecs)
  
{
  
  // center power array on DC
  
  int kCent = nSamples / 2;
  TaArray<double> powerCentered_;
  double *powerCentered = powerCentered_.alloc(nSamples);
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = (ii + kCent) % nSamples;
    powerCentered[jj] = power[ii];
  }
  
  // sum up terms

  double sumPower = 0.0;
  double sumK = 0.0;
  double sumK2 = 0.0;
  double *pw = powerCentered;
  for (int ii = 0; ii < nSamples; ii++, pw++) {
    double phase = (double) ii;
    double pwr = *pw;
    sumPower += pwr;
    sumK += pwr * phase;
    sumK2 += pwr * phase * phase;
  }
  double meanK = 0.0;
  double sdevK = 0.0;
  if (sumPower > 0.0) {
    meanK = sumK / sumPower;
    double diff = (sumK2 / sumPower) - (meanK * meanK);
    if (diff > 0) {
      sdevK = sqrt(diff);
    }
  }

  double velFac = wavelengthMeters / (2.0 * nSamples * prtSecs);
  _clutSpectrumWidth = velFac * sdevK;

}

//////////////////////////////////////////////////////
// compute weather-to-noise ratio
//
// Notches out clutter power.
// Divides spectrum into 8 pieces.
// Computes peak power divided by mean of lowest 3 peaks.

void ClutProb::_computeWx2NoiseRatio(int nSamples,
                                     const double *power)
  
{

  // notch out the clutter, to limit 2

  TaArray<double> notchedPower_;
  double *notchedPower = notchedPower_.alloc(nSamples);
  memcpy(notchedPower, power, nSamples * sizeof(double));

  double *pwrp = notchedPower;
  for (int ii = 0; ii < nSamples; ii++, pwrp++) {
    int jj = nSamples - 1 - ii;
    if (ii <= _outerIndex || jj <= _outerIndex) {
      *pwrp = 1.0e-20;
    }
  } // ii

  // find max power
  
  double maxPwr = 0.0;
  int kMax = 0;
  const double *pp = notchedPower;
  for (int ii = 0; ii < nSamples; ii++, pp++) {
    if (*pp > maxPwr) {
      kMax = ii;
      maxPwr = *pp;
    }
  }

  // set peak 0 to the max pwr

  double peaks[8];
  peaks[0] = maxPwr;
  
  // find each of 7 other peaks
  
  int kStart = kMax + nSamples / 16;
  int binWidth = nSamples / 8;
  TaArray<double> pwr2_;
  double *pwr2 = pwr2_.alloc(nSamples * 2);
  memcpy(pwr2, notchedPower, nSamples * sizeof(double));
  memcpy(pwr2 + nSamples, notchedPower, nSamples * sizeof(double));

  for (int i = 1; i < 8; i++, kStart += binWidth) {
    double maxInBin = 0.0;
    double *pp2 = pwr2 + kStart;
    for (int k = 0; k < binWidth; k++, pp2++) {
      if (*pp2 > maxInBin) {
	maxInBin = *pp2;
      }
    } // k
    peaks[i] = maxInBin;
  } // i

  // sort the peaks

  qsort(peaks, 8, sizeof(double), _compareDoubles);

  // compute mean power from lowest 3 peaks

  double sumLowest = 0.0;
  for (int i = 0; i < 3; i++) {
    sumLowest += peaks[i];
  }
  double meanLowest = sumLowest / 3.0;

  // compute ratio

  double wx2NoiseRatio = maxPwr / meanLowest;
  _wx2NoiseRatio = 10.0 * log10(wx2NoiseRatio);

}

/*****************************************************************************
 * define function to be used for sorting (lowest to highest)
 */

int ClutProb::_compareDoubles(const void *v1, const void *v2)
  
{
  double *d1 = (double *) v1;
  double *d2 = (double *) v2;
  if (*d1 > *d2) {
    return 1;
  } else {
      return -1;
    }
}

