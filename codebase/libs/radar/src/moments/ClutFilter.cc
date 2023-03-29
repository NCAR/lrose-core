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
#include <algorithm>
#include <cstring>
#include <vector>
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

double ClutFilter::MissingPower = 1.0e-12;

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
//   clutterWidthMps: spectrum width for clutter model (m/s)
//   initNotchWidthMps: width of first guess notch (m/s)
//   nyquist: unambiguous vel (m/s)
//   calibratedNoise: noise power at digitizer from calibration (mW)
//   setNotchToNoise: if true, points within the notch will be
//                       set to the calibrated noise
//
// Outputs:
//
//   clutterFound: true if clutter is identified in signal
//   filteredPowerSpec: power spectrum after filtering
//   notchedPowerSpec: power spectrum after notching, no interp
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
                                 double clutterWidthMps,
                                 double initNotchWidthMps,
                                 double nyquist,
                                 double calibratedNoise,
                                 bool setNotchToNoise,
                                 bool &clutterFound,
                                 double *filteredPowerSpec,
                                 double *notchedPowerSpec,
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
  
  double weatherPeak, clutterPeak, clutNoise;
  int notchWidth;
  locateWxAndClutter(rawPowerSpec,
                     nSamples,
                     clutterWidthMps,
                     initNotchWidthMps,
                     nyquist,
                     notchWidth,
                     clutterFound,
                     clutterPos,
                     clutterPeak,
                     weatherPos,
                     weatherPeak,
                     clutNoise);

  // compute half notch width
  
  int halfNotchWidth = computeHalfNotchWidth(rawPowerSpec,
                                             nSamples,
                                             clutterWidthMps,
                                             initNotchWidthMps,
                                             nyquist);
  
  // notch out the clutter, using the initial notch width
  
  TaArray<double> notched_;
  double *notched = notched_.alloc(nSamples);
  memcpy(notched, rawPowerSpec, nSamples * sizeof(double));
  for (int ii = -halfNotchWidth; ii <= halfNotchWidth; ii++) {
    notched[(ii + nSamples) % nSamples] = 0.0;
  }
  memcpy(notchedPowerSpec, notched, nSamples * sizeof(double));

  // widen the notch by one point on either side,
  // copying in the value adjacent to the notch

  notched[(-halfNotchWidth - 1 + nSamples) % nSamples] =
    notched[(-halfNotchWidth - 2 + nSamples) % nSamples];
  notched[(-halfNotchWidth + 1 + nSamples) % nSamples] =
    notched[(-halfNotchWidth + 2 + nSamples) % nSamples];
  
  int maxSearchWidth = halfNotchWidth * 2;
  if (maxSearchWidth > nSamples / 4) {
    maxSearchWidth = nSamples / 4;
  }
  int clutterLowerBound = -maxSearchWidth;
  int clutterUpperBound = +maxSearchWidth;

  // iterate 3 times, refining the correcting further each time
  // by fitting a Gaussian to the spectrum

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
    
    prevPower = rawPowerSpec[0];
    for (int ii = -1; ii >= -maxSearchWidth; ii--) {
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
    
    prevPower = rawPowerSpec[0];
    for (int ii = 1; ii <= maxSearchWidth; ii++) {
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
  
  double spectralNoise = computeSpectralNoise(notchedPower, nSamples);
  
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

/////////////////////////////////////////////////////////////
// find weather and clutter
//
// Divide spectrum into 8 parts, compute peaks and means
// for each part. Check for bi-modal spectrum.

void ClutFilter::locateWxAndClutter(const double *power,
                                    int nSamples,
                                    double clutterWidthMps,
                                    double initNotchWidthMps,
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
  int nClutWidth =
    (int) ((clutterWidthMps / (nyquist * 2.0)) * nSamples + 0.5);
  nClutWidth = MAX(nClutWidth, nHalf - 1);
  nClutWidth = MIN(nClutWidth, 1);

  notchWidth =
    (int) ((initNotchWidthMps / (nyquist * 2.0)) * nSamples + 0.5);
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
  // if less than 30dB down, we have clutter
  
  double zeroMean = blockMeans[0];
  double maxOtherMean = 0.0;
  double minOtherMean = 1.0e100;
  for (int ii = 1; ii < 8; ii++) {
    maxOtherMean = MAX(maxOtherMean, blockMeans[ii]);
    minOtherMean = MIN(minOtherMean, blockMeans[ii]);
  }
  clutterFound = false;
  if ((zeroMean / maxOtherMean) > 0.001) {
    clutterFound = true;
  }
  
  // estimate the spectral noise as the mean of the power
  // in the lowest 1/8th

  spectralNoise = minOtherMean;

  if (!clutterFound) {
    return;
  }

  // find clutter peak within clutter width limits

  clutterPos = 0;
  clutterPeak = 0.0;
  for (int ii = -nClutWidth; ii <= nClutWidth; ii++) {
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
    
/////////////////////////////////////////////////////////////
// compute half notch using clutter model
// we find the spectral points at which the clutter model
// crosses the noise floor.
// returns notch width on success, -1 on failure

int ClutFilter::computeHalfNotchWidth(const double *power,
                                      int nSamples,
                                      double clutterWidthMps,
                                      double initNotchWidthMps,
                                      double nyquist)
  
{
  
  // initialize half notch width
  
  int nQuarter = nSamples / 4;
  int halfNotchWidth =
    (int) ((initNotchWidthMps / (nyquist * 2.0)) * nSamples + 0.5);
  if (halfNotchWidth > nQuarter) {
    halfNotchWidth = nQuarter;
  }

  // get the clutter model
  
  TaArray<double> powerShifted_;
  double *powerShifted = powerShifted_.alloc(nSamples);
  copy(powerShifted, power, nSamples);
  shift(powerShifted, nSamples);
  TaArray<double> clutModel_;
  double *clutModel = clutModel_.alloc(nSamples);
  if (computeGaussianClutterModel(powerShifted, nSamples, 
                                  clutterWidthMps, nyquist,
                                  clutModel)) {
    // failed - return default value
    return halfNotchWidth;
  }

  // clutter model succeeded
  // find limit points where the power exceeds the noise floor

  double noisePower = computeSpectralNoise(power, nSamples);

  int nHalf = nSamples / 2;
  int notchStart = nHalf;
  for (int ii = nHalf - 1; ii >= 0; ii--) {
    if (clutModel[ii] < noisePower) {
      notchStart = ii + 1;
      break;
    }
  }
  
  int notchEnd = nHalf;
  for (int ii = nHalf + 1; ii < nSamples - 1; ii++) {
    if (clutModel[ii] < noisePower) {
      notchEnd = ii - 1;
      break;
    }
  }

  int notchWidth = notchEnd - notchStart + 1;
  halfNotchWidth = notchWidth / 2;
  
  return halfNotchWidth;

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
// Compute noise from a power spectrum
//
// Divide spectrum into runs and compute the mean power
// for each run, incrementing by one index at a time.
//
// The noise power is estimated as the mimumum of the section
// powers.

double ClutFilter::computeSpectralNoise(const double *powerSpec,
                                        int nSamples)
  
{

  // for short spectra use the entire spectrum

  if (nSamples < 8) {
    double sum = 0.0;
    for (int ii = 0; ii < nSamples; ii++) {
      sum += powerSpec[ii];
    }
    double noisePower = sum / nSamples;
    return noisePower;
  }

  // compute the size of a section

  int nRuns = nSamples / 8;
  if (nRuns < 8) {
    nRuns = 8;
  }
  int nPtsRun = nSamples / nRuns;
  
  // compute the sum for the first run
  
  double sum = 0.0;
  for (int ii = 0; ii < nPtsRun; ii++) {
    sum += powerSpec[ii];
  }
  double minSum = sum;
  
  // not move through the spectrum one point at a time, computing
  // the sum for that run by removing the first point of the previous
  // run and adding the next point to the end.
  // keep track of the minimum sum

  for (int ii = nPtsRun; ii < nSamples; ii++) {
    sum -= powerSpec[ii - nPtsRun];
    sum += powerSpec[ii];
    if (sum < minSum) {
      minSum = sum;
    }
  }

  // compute noise from minimum sum

  double noisePower = minSum / nPtsRun;
  
  return noisePower;

}

//////////////////////////////////////////////
// Compute a gaussian clutter model, based
// on an observed power spectrum.
//
// Powers are linear - i.e. not dBm.
//
// Assume:
// (a) Spectrum is shifted so DC is centered.
// (b) Clutter is centered - i.e. 0 vel.
// (c) Clutter width is supplied.
//
// The model will match the peak of the spectrum at DC.
// If the peak is not at DC, the model will be set to missing.
// Missing power values are set to 1.0e-12 = -120 dBm. 
//
// The caller manages the memory for gaussianModel.
//
// Returns 0 if clutter is found, -1 otherwise.
// If force is true, the model is always computed.

int ClutFilter::computeGaussianClutterModel(const double *powerSpectrum,
                                            int nSamples, 
                                            double widthMps,
                                            double nyquistMps,
                                            double *gaussianModel,
                                            bool force /* = false*/)
  
{

  int nSamplesHalf = nSamples / 2;

  // find max power
  // if clutter this is in the 3 middle spectral points

  double sumClutPower = 0.0;
  double maxPower = 0.0;
  int maxIndex = 0;
  
  for (int ii = 0; ii < nSamples; ii++) {
    double power = powerSpectrum[ii];
    if (power > maxPower) {
      maxPower = power;
      maxIndex = ii;
    }
    if (ii > nSamplesHalf - 2 && ii < nSamplesHalf + 2) {
      sumClutPower += power;
    }
  }

  if (!force) {
    if (maxIndex < nSamplesHalf - 1 ||
        maxIndex > nSamplesHalf + 1) {
      // clutter does not dominate
      for (int ii = 0; ii < nSamples; ii++) {
        gaussianModel[ii] = MissingPower;
      }
      return -1;
    }
  }

  // set clutter properties

  double clutPower = sumClutPower / 3.0;
  double sampleMps = nyquistMps / (nSamples / 2.0);

  // compute model powers

  double sumModelPower = 0.0;
  vector<double> modelPowers;
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = (nSamples / 2) - ii;
    double xx = jj * sampleMps;
    double modelPower = 
      ((clutPower / (widthMps * sqrt(M_PI * 2.0))) * 
       exp(-0.5 * pow(xx / widthMps, 2.0)));
    modelPowers.push_back(modelPower);
    if (ii > nSamplesHalf - 2 && ii < nSamplesHalf + 2) {
      sumModelPower += modelPower;
    }
  }
  double maxModelPower = sumModelPower / 3.0;

  // adjust model power so that the peak observation
  // and peak of the model are the same

  double powerRatio = clutPower / maxModelPower;
  for (int ii = 0; ii < nSamples; ii++) {
    double power = modelPowers[ii] * powerRatio;
    if (power >= MissingPower) {
      gaussianModel[ii] = power;
    } else {
      gaussianModel[ii] = MissingPower;
    }
  }

  return 0;

}

/////////////////////////////////////////////////////////////////
// Shift a spectrum, in place, so that DC is in the center.
// Swaps left and right sides.
// DC location location starts at index 0.
// After the shift:
//   if n is odd,  the DC location is at the center index
//   if n is even, the DC location is at index n/2

void ClutFilter::shift(RadarComplex_t *spectrum, int nSamples)
  
{

  int nRight = nSamples / 2;
  int nLeft = nSamples - nRight;
  
  TaArray<RadarComplex_t> tmp_;
  RadarComplex_t *tmp = tmp_.alloc(nSamples);

  ClutFilter::copy(tmp, spectrum, nLeft);
  ClutFilter::copy(spectrum, spectrum + nLeft, nRight);
  ClutFilter::copy(spectrum + nRight, tmp, nLeft);
  
}

void ClutFilter::shift(double *spectrum, int nSamples)
  
{

  int nRight = nSamples / 2;
  int nLeft = nSamples - nRight;
  
  TaArray<double> tmp_;
  double *tmp = tmp_.alloc(nSamples);

  ClutFilter::copy(tmp, spectrum, nLeft);
  ClutFilter::copy(spectrum, spectrum + nLeft, nRight);
  ClutFilter::copy(spectrum + nRight, tmp, nLeft);
  
}

/////////////////////////////////////////////////////////////////
// Unshift a spectrum, in place, to undo a previous shift.
// Swaps left and right sides.
// After the shift, DC is at index 0.

void ClutFilter::unshift(RadarComplex_t *spectrum, int nSamples)
  
{

  int nRight = nSamples / 2;
  int nLeft = nSamples - nRight;
  
  TaArray<RadarComplex_t> tmp_;
  RadarComplex_t *tmp = tmp_.alloc(nSamples);
  
  ClutFilter::copy(tmp, spectrum, nRight);
  ClutFilter::copy(spectrum, spectrum + nRight, nLeft);
  ClutFilter::copy(spectrum + nLeft, tmp, nRight);
  
}

void ClutFilter::unshift(double *spectrum, int nSamples)
  
{

  int nRight = nSamples / 2;
  int nLeft = nSamples - nRight;
  
  TaArray<double> tmp_;
  double *tmp = tmp_.alloc(nSamples);
  
  ClutFilter::copy(tmp, spectrum, nRight);
  ClutFilter::copy(spectrum, spectrum + nRight, nLeft);
  ClutFilter::copy(spectrum + nLeft, tmp, nRight);
  
}

/////////////////////////////////////////////
// copy arrays

void ClutFilter::copy(RadarComplex_t *dest,
                      const RadarComplex_t *src,
                      size_t nSamples)
  
{
  memcpy(dest, src, nSamples * sizeof(RadarComplex_t));
}

void ClutFilter::copy(double *dest,
                      const double *src,
                      size_t nSamples)
  
{
  memcpy(dest, src, nSamples * sizeof(double));
}

