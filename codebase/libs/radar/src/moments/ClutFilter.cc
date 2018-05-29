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
#include <cstring>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <radar/ClutFilter.hh>
#include <radar/RadarComplex.hh>
using namespace std;

// #define DEBUG_PRINT_SPECTRA
#ifdef DEBUG_PRINT_SPECTRA
int GlobGateNum = 0;
double GlobAz = 0.0;
double GlobElev = 0.0;
#endif

// Constructor

ClutFilter::ClutFilter()

{

}

// destructor

ClutFilter::~ClutFilter()

{
  
}

/////////////////////////////////////////////////////
// Perform adaptive filtering on a power spectrum
//
// Inputs:
//   rawPowerSpec: unfiltered power spectrum
//   nSamples: number of samples
//   maxClutterVel: max velocity of the clutter component
//                  of the signal (m/s)
//   initNotchWidth: width of first guess notch (m/s)
//   nyquist: unambiguous vel (m/s)
//   calibratedNoise: noise power at digitizer from calibration (mW)
//   setNotchToNoise: if true, points within the notch will be
//                       set to the calibrated noise
//
// Outputs:
//
//   clutterFound: true if clutter is identified in signal
//   filteredPowerSpec: power spectrum after filtering
//   notchStart: spectral position of start of final filtering notch
//   notchEnd: spectral position of end of final filtering notch
//   rawPower: mean power in unfiltered spectrum
//   filteredPower: mean power in filtered spectrum
//   powerRemoved: mean power removed by the filter (mW)
//   spectralNoise: noise determined from the spectrum (mW)
//   weatherPos: spectral location of weather peak
//   clutterPos: spectral location of clutter peak

void ClutFilter::performAdaptive(const double *rawPowerSpec, 
                                 int nSamples,
                                 double maxClutterVel,
                                 double initNotchWidth,
                                 double nyquist,
                                 double calibratedNoise,
                                 bool setNotchToNoise,
                                 bool &clutterFound,
                                 double *filteredPowerSpec,
                                 int &notchStart,
                                 int &notchEnd,
				 double &rawPower,
				 double &filteredPower,
                                 double &powerRemoved,
                                 double &spectralNoise,
                                 int &weatherPos,
                                 int &clutterPos)
  
{

  // initialize

  clutterFound = false;
  notchStart = 0;
  notchEnd = 0;
  powerRemoved = 0.0;

  // compute raw power
  
  rawPower = RadarComplex::meanPower(rawPowerSpec, nSamples);

  // locate the weather and clutter
  
  double weatherPeak, clutterPeak;
  int notchWidth;

  double clutNoise;
  locateWxAndClutter(rawPowerSpec,
                     nSamples,
                     maxClutterVel,
                     initNotchWidth,
                     nyquist,
                     notchWidth,
                     clutterFound,
                     clutterPos,
                     clutterPeak,
                     weatherPos,
                     weatherPeak,
                     clutNoise);

  // notch out the clutter, using the initial notch width
  
  TaArray<double> notched_;
  double *notched = notched_.alloc(nSamples);
  memcpy(notched, rawPowerSpec, nSamples * sizeof(double));
  for (int ii = clutterPos - notchWidth;
       ii <= clutterPos + notchWidth; ii++) {
    notched[(ii + nSamples) % nSamples] = clutNoise;
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

  // iterate 3 times, refining the correcting further each time

  TaArray<double> gaussian_;
  double *gaussian = gaussian_.alloc(nSamples);
  double matchRatio = 10.0;
  double prevPower = 0.0;

  for (int iter = 0; iter < 3; iter++) {
    
    // fit gaussian to notched spectrum
    
    fitGaussian(notched, nSamples, weatherPos, clutNoise, gaussian);
    
    // find where clutter peak drops below gaussian, and
    // create a notched spectrum using the gaussian where
    // the clutter peak was
    
    clutterLowerBound = clutterPos - maxSearchWidth;
    clutterUpperBound = clutterPos + maxSearchWidth;
    
    prevPower = rawPowerSpec[(clutterPos + nSamples) % nSamples];
    for (int ii = clutterPos - 1; ii >= clutterPos - maxSearchWidth; ii--) {
      int jj = (ii + nSamples) % nSamples;
      double power = rawPowerSpec[jj];
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
    
    prevPower = rawPowerSpec[(clutterPos + nSamples) % nSamples];
    for (int ii = clutterPos + 1; ii <= clutterPos + maxSearchWidth; ii++) {
      int jj = (ii + nSamples) % nSamples;
      double power = rawPowerSpec[jj];
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
    
    memcpy(notched, rawPowerSpec, nSamples * sizeof(double));
    for (int ii = clutterLowerBound; ii <= clutterUpperBound; ii++) {
      int jj = (ii + nSamples) % nSamples;
      notched[jj] = gaussian[jj];
    }

  } // iter

  // set notch limits used

  notchStart = (clutterLowerBound + nSamples) % nSamples;
  notchEnd = (clutterUpperBound + nSamples) % nSamples;
  
  // set filtered power array
  
  for (int ii = 0; ii < nSamples; ii++) {
    filteredPowerSpec[ii] = notched[ii];
  }

  // if requested, set power to the noise in the notch

  if (setNotchToNoise) {
    for (int ii = clutterLowerBound; ii <= clutterUpperBound; ii++) {
      int jj = (ii + nSamples) % nSamples;
      filteredPowerSpec[jj] = calibratedNoise;
    }
  }
  
  // compute filtered power
  
  filteredPower = RadarComplex::meanPower(filteredPowerSpec, nSamples);
  
  // compute power removed
  
  powerRemoved = rawPower - filteredPower;

  // compute spectral noise by stand-alone method

  spectralNoise = computeSpectralNoise(rawPowerSpec, nSamples);
  
}

/////////////////////////////////////////////////////
// Given a spectrum which has been filtered,
// fill in the notch using a gaussian fit.
// The phase information is preserved.
//
// Inputs:
//   filtSpec: filtered spectrum, which has power reduced
//             in the notch
//   nSamples: number of samples
//   maxNotchWidth: max width of filter notch in spectrum
//
// Outputs:
//
//   filledSpec: result of filling in spectrum
//               with same phases as filtSpec

void ClutFilter::fillNotchUsingGfit(const RadarComplex_t *notchedSpec,
                                    int nSamples,
                                    int maxNotchWidth,
                                    RadarComplex_t *filledSpec)
  
{

  // compute notch half width

  int nSamplesHalf = nSamples / 2;
  int notchWidthHalf = maxNotchWidth / 2;
  if (notchWidthHalf > nSamplesHalf) notchWidthHalf = nSamplesHalf;
  
  // compute the filtered power, rotating the spectrum so that
  // DC is in the center
  // copy filtered power to filled power

  TaArray<double> notchedPower_, filledPower_;
  double *notchedPower = notchedPower_.alloc(nSamples);
  double *filledPower = filledPower_.alloc(nSamples);
  for (int ii = 0; ii < nSamples; ii++) {
    notchedPower[ii] = RadarComplex::power(notchedSpec[ii]);
    filledPower[ii] = notchedPower[ii];
  }
  
  // estimate the spectral noise
  
  double spectralNoise = estimateSpectralNoise(notchedPower, nSamples);
  
  // find the location of the max power in the filtered spectrum,
  // presumably the weather position
  
  int weatherPos = 0;
  double maxPower = notchedPower[0];
  for (int ii = 1; ii < nSamples; ii++) {
    if (notchedPower[ii] > maxPower) {
      maxPower = notchedPower[ii];
      weatherPos = ii;
    }
  }
  
  // iterate 3 times, refining the correcting further each time

  TaArray<double> gaussian_;
  double *gaussian = gaussian_.alloc(nSamples);
  
  for (int iter = 0; iter < 3; iter++) {
    
    // fit gaussian to notched spectrum
    
    fitGaussian(filledPower, nSamples, weatherPos, spectralNoise, gaussian);
    
    // within the notch, if the gaussian exceeds the filtered power,
    // fill in that point using the gaussian value
    
    for (int ii = 0; ii <= notchWidthHalf; ii++) {
      if (gaussian[ii] > notchedPower[ii]) {
        filledPower[ii] = gaussian[ii];
      }
    }
    
    for (int ii = nSamples - notchWidthHalf - 1; ii < nSamples; ii++) {
      if (gaussian[ii] > notchedPower[ii]) {
        filledPower[ii] = gaussian[ii];
      }
    }
    
  } // iter
  
  // compute filled spec, using filled power
  // keep phase unchanged by using same ratio for re and im
  
  for (int ii = 0; ii < nSamples; ii++) {
    double powerRatio = filledPower[ii] / notchedPower[ii];
    double magRatio = sqrt(powerRatio);
    filledSpec[ii].re = notchedSpec[ii].re * magRatio;
    filledSpec[ii].im = notchedSpec[ii].im * magRatio;
  }

}

/////////////////////////////////////////////////////
// perform notch filtering on a power spectrum
//
// Inputs:
//   rawPowerSpec: unfiltered power spectrum
//   nSamples: number of samples
//   notchWidthMps: notch width (m/s)
//   nyquist: unambiguous vel (m/s)
//   notchPower: power top be set within the notch
//
// Outputs:
//
//   filteredPowerSpec: power spectrum after filtering
//   notchStart: spectral position of start of final filtering notch
//   notchEnd: spectral position of end of final filtering notch
//   rawPower: mean power in unfiltered spectrum
//   filteredPower: mean power in filtered spectrum
//   powerRemoved: mean power removed by the filter (mW)

void ClutFilter::performNotch(const double *rawPowerSpec, 
                              int nSamples,
                              double notchWidthMps,
                              double nyquist,
                              double notchPower,
                              double *filteredPowerSpec,
                              int &notchStart,
                              int &notchEnd,
			      double &rawPower,
			      double &filteredPower,
                              double &powerRemoved)
  
{

  // compute raw power
  
  rawPower = RadarComplex::meanPower(rawPowerSpec, nSamples);

  // compute notch posn
  
  int halfNotch = int ((notchWidthMps * nSamples) / (4.0 * nyquist) + 0.5);
  if (halfNotch < 1) {
    halfNotch = 1;
  }
  notchStart = nSamples - halfNotch;
  notchEnd = halfNotch;

  // load up filtered power, summing up power removed

  memcpy(filteredPowerSpec, rawPowerSpec, nSamples * sizeof(double));
  for (int ii = -halfNotch; ii <= halfNotch; ii++) {
    int jj = ii;
    if (jj < 0) {
      jj += nSamples;
    }
    filteredPowerSpec[jj] = notchPower;
  }

  // compute filtered power
  
  filteredPower = RadarComplex::meanPower(filteredPowerSpec, nSamples);
  
  // compute power removed
  
  powerRemoved = rawPower - filteredPower;

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
                                    double &weatherPeak,
                                    double &spectralNoise)
  
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
  double minOtherMean = 1.0e100;
  for (int ii = 1; ii < 8; ii++) {
    maxOtherMean = MAX(maxOtherMean, blockMeans[ii]);
    minOtherMean = MIN(minOtherMean, blockMeans[ii]);
  }
  clutterFound = false;
  if ((zeroMean / maxOtherMean) > 0.5) {
    clutterFound = true;
  }
  
  // estimate the spectral noise as the mean of the power
  // in the lowest 1/8th

  spectralNoise = minOtherMean;

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
    
///////////////////////////////////////////////////////////////
// estimate the spectral noise
//
// Divide spectrum into 8 parts, compute power in each part
// estimate the spectral noise as the mean of the power
// in the lowest 1/8th.

double ClutFilter::estimateSpectralNoise(const double *powerSpec,
                                         int nSamples)
  
{

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
      blockMeans[ii] += powerSpec[kk] / 8;
    }
  }

  double maxOtherMean = 0.0;
  double minOtherMean = 1.0e100;
  for (int ii = 1; ii < 8; ii++) {
    maxOtherMean = MAX(maxOtherMean, blockMeans[ii]);
    minOtherMean = MIN(minOtherMean, blockMeans[ii]);
  }
  
  return minOtherMean;

}
    
///////////////////////////////
// fit gaussian to spectrum

void ClutFilter::fitGaussian(const double *power,
                             int nSamples, 
                             int weatherPos,
                             double spectralNoise,
                             double *gaussian)
  
{

  // center power array on the max value
  
  TaArray<double> centered_;
  double *centered = centered_.alloc(nSamples);
  int nSamplesHalf = nSamples / 2;
  int kOffset = nSamplesHalf - weatherPos;
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
  
  // load up gaussian with computed curve
  
  for (int ii = 0; ii < nSamples; ii++) {
    gaussian[ii] = spectralNoise;
  }

  double c1 = sumPower / (sqrt(2.0 * M_PI) * sdevK);
  double c2 = -1.0 / (2.0 * varK);

  int iMeanK = (int) (meanK + 0.5);
  for (int ii = iMeanK; ii < iMeanK + nSamplesHalf; ii++) {
    double xx = ((double) ii - meanK);
    double fit = c1 * exp((xx * xx) * c2);
    if (fit < spectralNoise) {
      break;
    }
    int jj = (nSamples + ii - kOffset) % nSamples;
    if (jj < 0) {
      jj += nSamples;
    }
    gaussian[jj] = fit;
  }
  for (int ii = iMeanK - 1; ii >= iMeanK - nSamplesHalf; ii--) {
    double xx = ((double) ii - meanK);
    double fit = c1 * exp((xx * xx) * c2);
    if (fit < spectralNoise) {
      break;
    }
    int jj = (nSamples + ii - kOffset) % nSamples;
    if (jj < 0) {
      jj += nSamples;
    }
    gaussian[jj] = fit;
  }

}

/////////////////////////////////////////////////////
// compute noise of a power spectrum
// 
// We compute the mean power for 3 regions of the spectrum:
//   1. 1/8 at lower end plus 1/8 at upper end
//   2. 1/4 at lower end
//   3. 1/4 at upper end
// We estimate the noise to be the least of these 3 values
// because if there is a weather echo it will not affect both ends
// of the spectrum unless the width is very high, in which case we
// probably have a bad signal/noise ratio anyway.
//
// Inputs:
//   powerSpec: power spectrum
//   nSamples
//
// Outputs:
//   noiseMean: mean of the noise in the relevant 1/4 of the spectrum
//   noiseSdev: standard deviation of the same

void ClutFilter::computeSpectralNoise(const double *powerSpec,
                                      int nSamples,
                                      double &noiseMean,
                                      double &noiseSdev)
  
{

  // first, compute a spectrum centered on the max value
  
  int kCent = nSamples / 2;
  
  // find max power
  
  double maxPwr = 0.0;
  int kMax = 0;
  const double *pwr = powerSpec;
  for (int ii = 0; ii < nSamples; ii++, pwr++) {
    if (*pwr > maxPwr) {
      kMax = ii;
      maxPwr = *pwr;
    }
  }
  if (kMax >= kCent) {
    kMax -= nSamples;
  }

  // center power array on the max value

  TaArray<double> powerCentered_;
  double *powerCentered = powerCentered_.alloc(nSamples);
  pwr = powerSpec;
  int kOffset = kCent - kMax;
  for (int ii = 0; ii < nSamples; ii++, pwr++) {
    int jj = (ii + kOffset) % nSamples;
    powerCentered[jj] = *pwr;
  }
  
  // We compute the mean power for 3 regions of the spectrum:
  //   1. 1/8 at lower end plus 1/8 at upper end
  //   2. 1/4 at lower end
  //   3. 1/4 at uppoer end
  // We estimate the noise to be the least of these 3 values
  // because if there is a weather echo it will not affect both ends
  // of the spectrum unless the width is very high, in which case we
  // probablyhave a bad signal/noise ratio anyway

  int nby4 = nSamples / 4;
  int nby8 = nSamples / 8;
  
  // combine 1/8 from each end

  double sumBoth = 0.0;
  double sumSqBoth = 0.0;
  const double *pw = powerCentered;
  for (int ii = 0; ii < nby8; ii++, pw++) {
    sumBoth += *pw;
    sumSqBoth += *pw * *pw;
  }
  pw = powerCentered + nSamples - nby8 - 1;
  for (int ii = 0; ii < nby8; ii++, pw++) {
    sumBoth += *pw;
    sumSqBoth += *pw * *pw;
  }
  double meanBoth = sumBoth / (2.0 * nby8);

  // 1/4 from lower end

  double sumLower = 0.0;
  double sumSqLower = 0.0;
  pw = powerCentered;
  for (int ii = 0; ii < nby4; ii++, pw++) {
    sumLower += *pw;
    sumSqLower += *pw * *pw;
  }
  double meanLower = sumLower / (double) nby4;
  
  // 1/4 from upper end
  
  double sumUpper = 0.0;
  double sumSqUpper = 0.0;
  pw = powerCentered + nSamples - nby4 - 1;
  for (int ii = 0; ii < nby4; ii++, pw++) {
    sumUpper += *pw;
    sumSqUpper += *pw * *pw;
  }
  double meanUpper = sumUpper / (double) nby4;

  if (meanBoth < meanLower && meanBoth < meanUpper) {

    double diff = (sumSqBoth / (2.0 * nby8)) - (meanBoth * meanBoth);
    double sdev = 0.0;
    if (diff > 0) {
      sdev = sqrt(diff);
    }
    noiseMean = meanBoth;
    noiseSdev = sdev;

  } else if (meanLower < meanUpper) {
    
    double diff = (sumSqLower / nby4) - (meanLower * meanLower);
    double sdev = 0.0;
    if (diff > 0) {
      sdev = sqrt(diff);
    }
    noiseMean = meanLower;
    noiseSdev = sdev;

  } else {

    double diff = (sumSqUpper / nby4) - (meanUpper * meanUpper);
    double sdev = 0.0;
    if (diff > 0) {
      sdev = sqrt(diff);
    }
    noiseMean = meanUpper;
    noiseSdev = sdev;

  }

#ifdef DEBUG_PRINT2
  {
    double sum = 0.0;
    const double *pw = powerCentered;
    for (int ii = 0; ii < nSamples; ii++, pw++) {
      sum += 1.0 / (pow(*pw, 4.0));
    }
    double hnoise = (double) nSamples / pow(sum, 0.25);
    cerr << "noiseMean, hnoise, ratio: " << noiseMean
         << ", " << hnoise << ", " << noiseMean/hnoise << endl;
  }
#endif

}

/////////////////////////////////////////////////////
// Compute noise from a power spectrum
//
// Divide spectrum into sections and compute the mean power
// for each section.
//
// The noise power is estimated as the mimumum of the section
// powers.

double ClutFilter::computeSpectralNoise(const double *powerSpec,
					int nSamples)
  
{

  // divide the spectrum into sections, of at least 8 points
  // must have minimum of 8 points

  int nSections = nSamples / 8;
  if (nSections < 8) {
    nSections = 8;
  }
  int nPtsPerSect = nSamples / nSections;
  
  double noisePower = 1.0e99;
  
  for (int isect = 0; isect < nSections; isect++) {

    int istart = isect * nPtsPerSect;
    int iend = istart + nPtsPerSect - 1;
    if (iend > nSamples - 1) {
      iend = nSamples - 1;
    }
    
    double sum = 0;
    double count = 0;
    for (int ii = istart; ii <= iend; ii++) {
      sum += powerSpec[ii];
      count++;
    }
    double mean = sum / count;
    
    if (mean < noisePower) {
      noisePower = mean;
    }
    
  } // isect

  return noisePower;

}
