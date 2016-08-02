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
// ClutFilter.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2004
//
///////////////////////////////////////////////////////////////
//
// Perform clutter filtering
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <cmath>
#include "TaArray.hh"
#include "ClutFilter.hh"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

using namespace std;

// Constructor

ClutFilter::ClutFilter()

{

}

// destructor

ClutFilter::~ClutFilter()

{
  
}

/////////////////////////////////////////////////////
// perform filtering
//
// Returns power removed

void ClutFilter::run(const double *rawMag, 
		     int nSamples,
		     double max_clutter_vel,
		     double init_notch_width,
		     double nyquist,
		     double *filteredMag,
		     bool &clutterFound,
		     int &notchStart,
		     int &notchEnd,
		     double &powerRemoved,
		     double &vel,
		     double &width)
  
{

  // initialize

  clutterFound = false;
  notchStart = 0;
  notchEnd = 0;
  powerRemoved = 0.0;

  // compute power from magnitudes

  TaArray<double> rawPower_;
  double *rawPower = rawPower_.alloc(nSamples);
  const double *rm = rawMag;
  double *rp = rawPower;
  for (int ii = 0; ii < nSamples; ii++, rm++, rp++) {
    *rp = *rm * *rm;
  }
  
  // compute min power - for use as noise value
  
  double minPower = rawPower[0];
  for (int ii = 1; ii < nSamples; ii++) {
    if (minPower > rawPower[ii]) {
      minPower =  rawPower[ii];
    }
  }
  
  // locate the weather and clutter
  
  int weatherPos, clutterPos;
  double weatherPeak, clutterPeak;
  int notchWidth;

  locateWxAndClutter(rawPower,
                     nSamples,
                     max_clutter_vel,
                     init_notch_width,
                     nyquist,
                     notchWidth,
                     clutterFound,
                     clutterPos,
                     clutterPeak,
                     weatherPos,
                     weatherPeak);

  // notch out the clutter, using the initial notch width

  TaArray<double> notched_;
  double *notched = notched_.alloc(nSamples);
  memcpy(notched, rawPower, nSamples * sizeof(double));
  for (int ii = clutterPos - notchWidth;
       ii <= clutterPos + notchWidth; ii++) {
    notched[(ii + nSamples) % nSamples] = minPower;
  }
  
  // widen the notch by one point on either side,
  // copying in the value adjacent to the notch

  notched[(clutterPos - notchWidth - 1 + nSamples) % nSamples] =
    notched[(clutterPos - notchWidth - 2 + nSamples) % nSamples];
  notched[(clutterPos - notchWidth + 1 + nSamples) % nSamples] =
    notched[(clutterPos - notchWidth + 2 + nSamples) % nSamples];
  
  int maxSearchWidth = notchWidth * 2;
  if (maxSearchWidth > nSamples / 4) {
    maxSearchWidth = nSamples / 4;
  }
  int clutterLowerBound = clutterPos - maxSearchWidth;
  int clutterUpperBound = clutterPos + maxSearchWidth;

  // #ifdef NO_NOTCH

  TaArray<double> gaussian_;
  double *gaussian = gaussian_.alloc(nSamples);
  double matchRatio = 10.0;
  double prevPower = 0.0;

  // iterate 3 times, refining the correcting further each time

  for (int iter = 0; iter < 3; iter++) {
    
    // fit gaussian to notched spectrum
    
    fitGaussian(notched, nSamples, weatherPos, minPower, nyquist,
                vel, width, gaussian);
    
    // find where clutter peak drops below gaussian, and
    // create a notched spectrum using the gaussian where
    // the clutter peak was
    
    clutterLowerBound = clutterPos - maxSearchWidth;
    clutterUpperBound = clutterPos + maxSearchWidth;
    
    prevPower = rawPower[clutterPos];
    for (int ii = clutterPos - 1; ii >= clutterPos - maxSearchWidth; ii--) {
      int jj = (ii + nSamples) % nSamples;
      double power = rawPower[jj];
      double gauss = gaussian[jj];
      if (power <= gauss) {
	// power falls below gaussian fit
	clutterLowerBound = ii + 1;
	break;
      }
      if (power < gauss * matchRatio) {
	if (power > prevPower) {
	  // power came close to gaussian fit and is moving away
	  clutterLowerBound = ii + 1;
	  break;
	}
      }
      prevPower = power;
    }
    
    prevPower = rawPower[clutterPos];
    for (int ii = clutterPos + 1; ii <= clutterPos + maxSearchWidth; ii++) {
      int jj = (ii + nSamples) % nSamples;
      double power = rawPower[jj];
      double gauss = gaussian[jj];
      if (power < gauss) {
	// power falls below gaussian fit
	clutterUpperBound = ii - 1;
	break;
      }
      if (power < gauss * matchRatio) {
	if (power > prevPower) {
	  // power came close to gaussian fit and is moving away
	  clutterUpperBound = ii - 1;
	  break;
	}
      }
      prevPower = power;
    }
    
    // recompute notched spectrum, using gaussian to fill in notch
    
    memcpy(notched, rawPower, nSamples * sizeof(double));
    for (int ii = clutterLowerBound; ii <= clutterUpperBound; ii++) {
      int jj = (ii + nSamples) % nSamples;
      notched[jj] = gaussian[jj];
    }

  } // iter

  // #endif
  
  // compute the power associated with the peak at each point
  
  powerRemoved = 0.0;
  for (int ii = clutterLowerBound; ii <= clutterUpperBound; ii++) {
    int jj = (ii + nSamples) % nSamples;
    double diff = rawPower[jj] - notched[jj];
    powerRemoved += diff;
  }
  powerRemoved /= nSamples;
  
  notchStart = (clutterLowerBound + nSamples) % nSamples;
  notchEnd = (clutterUpperBound + nSamples) % nSamples;
  
  // set filtered mag array

  double *fm = filteredMag;
  double *no = notched;
  for (int ii = 0; ii < nSamples; ii++, fm++, no++) {
    *fm = sqrt(*no);
  }

  return;

}

///////////////////////////////
// find weather and clutter
//
// Divide spectrum into 8 parts, compute peaks and means
// for each part. Check for bi-modal spectrum.

void ClutFilter::locateWxAndClutter(const double *power,
                                    int nSamples,
                                    double max_clutter_vel,
                                    double init_notch_width,
                                    double nyquist,
                                    int &notchWidth,
                                    bool &clutterFound,
                                    int &clutterPos,
                                    double &clutterPeak,
                                    int &weatherPos,
                                    double &weatherPeak)
  
{

  // initialize

  clutterFound = false;
  weatherPos = 0;
  clutterPos = 0;

  int nHalf = nSamples / 2;
  int nClutVel =
    (int) ((max_clutter_vel / (nyquist * 2.0)) * nSamples + 0.5);
  nClutVel = MAX(nClutVel, nHalf - 1);
  nClutVel = MIN(nClutVel, 1);

  notchWidth =
    (int) ((init_notch_width / (nyquist * 2.0)) * nSamples + 0.5);
  notchWidth = MIN(notchWidth, nHalf - 1);
  notchWidth = MAX(notchWidth, 1);

  // divide spectrum into 8 parts, compute power in each part
  
  int nEighth = ((nSamples - 1) / 8) + 1;
  if (nEighth < 3) {
    nEighth = 3;
  }
  int nSixteenth = nEighth / 2;
  double blockMeans[8];
  for (int ii = 0; ii < 8; ii++) {
    int jjStart = ((ii * nSamples) / 8) - nSixteenth;
    blockMeans[ii] = 0.0;
    for (int jj = jjStart; jj < jjStart + nEighth; jj++) {
      int kk = (jj + nSamples) % nSamples;
      blockMeans[ii] += power[kk] / 8;
    }
  }

  // compare peak at 0 with max of other peaks
  // if less than 3dB down, we have clutter
  
  double zeroMean = blockMeans[0];
  double maxOtherMean = 0.0;
  for (int ii = 1; ii < 8; ii++) {
    maxOtherMean = MAX(maxOtherMean, blockMeans[ii]);
  }
  clutterFound = false;
  if ((zeroMean / maxOtherMean) > 0.5) {
    clutterFound = true;
  }
  
  if (!clutterFound) {
    return;
  }

  // find clutter peak within velocity limits
  
  clutterPos = 0;
  clutterPeak = 0.0;
  for (int ii = -nClutVel; ii <= nClutVel; ii++) {
    double val = power[(ii + nSamples) % nSamples];
    if (val > clutterPeak) {
      clutterPeak = val;
      clutterPos = ii;
    }
  }

  ///////////////////////////////////////////////////////
  // check for bimodal spectrum, assuming one peak at DC

  // find pos of peak away from DC

  double weatherMean = 0.0;
  int wxMeanPos = 0;
  for (int ii = 2; ii < 7; ii++) {
    if (blockMeans[ii] > weatherMean) {
      weatherMean = blockMeans[ii];
      wxMeanPos = ii;
    }
  }

  // check for 3dB valleys between DC and peak
  // if valleys exist on both sides, then we have a bimodal spectrum

  int biModal = 1;
  int vallyFound = 0;
  for (int ii = 1; ii < wxMeanPos; ii++) {
    if (weatherMean / blockMeans[ii] > 5.0) {
      vallyFound = 1;
      break;
    }
  }
  if (!vallyFound) {
    biModal = 0;
  }
  vallyFound = 0;
  for (int ii = wxMeanPos; ii < 8; ii++) {
    if (weatherMean / blockMeans[ii] > 5.0) {
      vallyFound = 1;
      break;
    }
  }
  if (!vallyFound) {
    biModal = 0;
  }

  // if bimodal, find weather peak away from clutter peak
  // else find weather peak outside the notch

  int iStart = 0, iEnd = 0;
  if (biModal) {
    iStart = ((wxMeanPos * nSamples) / 8) - nSixteenth;
    iEnd = iStart + nEighth;
  } else {
    iStart = clutterPos + 2 * notchWidth + 1;
    iEnd = iStart + nSamples - (4 * notchWidth) - 1;
  }

  weatherPeak = 0.0;
  for (int ii = iStart; ii < iEnd; ii++) {
    int kk = (ii + nSamples) % nSamples;
    if (weatherPeak < power[kk]) {
      weatherPeak = power[kk];
      weatherPos = kk;
    }
  }
    
}
    
///////////////////////////////
// fit gaussian to spectrum

void ClutFilter::fitGaussian(const double *power,
                             int nSamples, 
                             int weatherPos,
                             double minPower,
                             double nyquist,
                             double &vel,
                             double &width,
                             double *gaussian)
  
{

  // center power array on the max value
  
  TaArray<double> centered_;
  double *centered = centered_.alloc(nSamples);
  int kCent = nSamples / 2;
  int kOffset = kCent - weatherPos;
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = (nSamples + ii + kOffset) % nSamples;
    centered[jj] = power[ii];
  }

  // compute mean and sdev
  
  double sumPower = 0.0;
  double sumPhase = 0.0;
  double sumPhase2 = 0.0;
  double *ce = centered;
  for (int ii = 0; ii < nSamples; ii++, ce++) {
    double phase = (double) ii;
    double power = *ce;
    sumPower += power;
    sumPhase += power * phase;
    sumPhase2 += power * phase * phase;
  }
  double meanK = 0.0;
  double sdevK = 0.0;
  double varK = 0.0;
  if (sumPower > 0.0) {
    meanK = sumPhase / sumPower;
    varK = (sumPhase2 / sumPower) - (meanK * meanK);
    if (varK > 0) {
      sdevK = sqrt(varK);
    } else {
      varK = 0.0001;
    }
  }
  
  // compute curve
  
  double c1 = sumPower / (sqrt(2.0 * M_PI) * sdevK);
  double c2 = -1.0 / (2.0 * varK);
  int istart = (int) (meanK - nSamples / 2 + 0.5);
  for (int ii = istart; ii < istart + nSamples; ii++) {
    double xx = ((double) ii - meanK);
    double fit = c1 * exp((xx * xx) * c2);
    if (fit < minPower) {
      fit = minPower;
    }
    int jj = (nSamples + ii - kOffset) % nSamples;
    gaussian[jj] = fit;
  }

  double velFac = (nyquist * 2.0) / nSamples;
  vel = velFac * (meanK - (kOffset + nSamples) % nSamples);
  if (vel < -nyquist) {
    vel += 2.0 * nyquist;
  } else if (vel > nyquist) {
    vel -= 2.0 * nyquist;
  }
  width = (velFac * sdevK);

}

/////////////////////////////////////////////////////
// perform filtering
//
// Returns power removed

void ClutFilter::notch(const double *rawMag, 
		       int nSamples,
		       double max_clutter_vel,
		       double nyquist,
		       double *filteredMag,
		       bool &clutterFound,
		       int &notchStart,
		       int &notchEnd,
		       double &powerRemoved)
  
{

  // initialize

  clutterFound = true;

  // compute notch posn
  
  int halfNotch = int ((max_clutter_vel * nSamples) / (4.0 * nyquist) + 0.5);
  if (halfNotch < 1) {
    halfNotch = 1;
  }
  notchStart = nSamples - halfNotch;
  notchEnd = halfNotch;

  // compute power from magnitudes
  
  TaArray<double> rawPower_;
  double *rawPower = rawPower_.alloc(nSamples);
  const double *rm = rawMag;
  double *rp = rawPower;
  for (int ii = 0; ii < nSamples; ii++, rm++, rp++) {
    *rp = *rm * *rm;
  }

  // compute min power - for use as noise value
  
  double minPower = rawPower[0];
  for (int ii = 1; ii < nSamples; ii++) {
    if (minPower > rawPower[ii]) {
      minPower =  rawPower[ii];
    }
  }
  minPower = 1.0e-9; // XXXX

  // load up filtered magnitudes, summing up power removed

  memcpy(filteredMag, rawMag, nSamples * sizeof(double));
  double sum = 0.0;
  for (int ii = -halfNotch; ii <= halfNotch; ii++) {
    int jj = ii;
    if (jj < 0) {
      jj += nSamples;
    }
    double pwrRem = rawPower[jj] - minPower;
    sum += pwrRem;
    filteredMag[jj] -= sqrt(pwrRem);
  }
  powerRemoved = sum;

  return;

}

