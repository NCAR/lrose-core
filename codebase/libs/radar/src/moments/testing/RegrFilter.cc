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
////////////////////////////////////////////////////////////////////////
// RegrFilter.cc
//
// running the regression filter
//
////////////////////////////////////////////////////////////////////////
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2023
//
///////////////////////////////////////////////////////////////

#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstddef>
#include <iostream>
#include "RegrFilter.hh"
using namespace std;

////////////////////////////////////////////////////
// constructor

RegrFilter::RegrFilter()
        
{
  _cnrDb = 0.0;
  _csrDb = 0.0;
  _polyOrder = 0;

  _filterRatio = 1.0;
  _spectralNoise = 0.0;
  _spectralSnr = 0.0;

}

////////////////////////////////////////////////////
// destructorc

RegrFilter::~RegrFilter()
  
{
}

/////////////////////////////////////////////////////////////////
// apply polynomial regression clutter filter to IQ time series
// NOTE: input raw IQ data should not be windowed.
//
// Inputs:
//   nSamples
//   antennaRateDegPerSec: antenna rate in deg / sec
//   prtSecs: PRT in secs
//   wavelengthM: wavelength in meters
//   calNoise: calibration estimate of noise for this channel, linear units
//   applyDbForDbCorrection: apply nexrad legacy power correction
//   iqRaw: unfiltered time series, not windowed
//
//  Outputs (memory must be allocated by caller):
//    iqFiltered: filtered time series, gaussian interpolation
//    iqNotched: if non-NULL, notched time series, not interpolated
//    iqPolyFit: if non-NULL, polynomial fit
//
//  After calling this routine, you can call:
//    getCnrDb() to get the estimated clutter-to-noise ratio
//    getCsrDb() to get the estimated clutter-to-signal ratio
//    getPolyOrder() to get the polynomial order used

void RegrFilter::applyFilter(int nSamples,
                             double antennaRateDegPerSec,
                             double prtSecs,
                             double wavelengthM,
                             double calNoise,
                             bool applyDbForDbCorrection,
                             const complex<double> *iqRaw,
                             complex<double> *iqFiltered,
                             complex<double> *iqNotched /* = NULL */,
                             complex<double> *iqPolyFit /* = NULL */)
  
{

  // initialize the fft manager for number of samples
  // this is a no-op if nSamples has not changed
  
  _fft.init(nSamples);
  
  // take the forward fft to compute the complex spectrum of unfiltered series
  
  complex<double> empty(0.0, 0.0);
  vector<complex<double> > rawSpecC(nSamples, empty);
  _fft.fwd(iqRaw, rawSpecC.data());
  
  // compute the real unfiltered spectrum
  
  vector<double> unfiltSpec(nSamples, 0.0);
  loadPower(nSamples, rawSpecC.data(), unfiltSpec.data());

  // allocate space for regression power spectrum
  
  vector<double> regrSpec(nSamples, 0.0);

  // compute clutter to noise ratio, using the central 3 points in the FFT

  computePowerRatios(nSamples, calNoise, iqRaw, _cnrDb, _csrDb);

  // if no clutter, do not filter
  if (_csrDb < -5.0) {
    memcpy(iqFiltered, iqRaw, nSamples * sizeof(complex<double>));
    if (iqNotched) {
      memcpy(iqNotched, iqRaw, nSamples * sizeof(complex<double>));
    }
    regrSpec = unfiltSpec;
    _filterRatio = 1.0;
    _spectralNoise = computeSpectralNoise(nSamples, regrSpec.data());
    _spectralSnr = _spectralNoise / calNoise;
    return;
  }
  
  // compute the polynomial order to be used (from Meymaris 2021)
  
  if (_cnrDb < 1) {
    _cnrDb = 1.0;
  }
  _polyOrder = computePolyOrder(nSamples, _cnrDb,
                                antennaRateDegPerSec, prtSecs, wavelengthM);

  // apply regression filter, passing in CNR
  // results are in iqRegr
  
  // copy IQ data into double arrays

  vector<double> rawI, rawQ;
  for (int ii = 0; ii < nSamples; ii++) {
    rawI.push_back(iqRaw[ii].real());
    rawQ.push_back(iqRaw[ii].imag());
  }
  
  // load x (time scale) vector
  
  vector<double> xxVals;
  double xDelta = 1.0 / nSamples;
  double xx = -0.5;
  for (int ii = 0; ii < nSamples; ii++) {
    xxVals.push_back(xx);
    xx += xDelta;
  }

  // prepare for fitting
  
  _poly.prepareForFitFixedPrt(_polyOrder, nSamples);

  // fit I

  _poly.performFit(rawI);
  vector<double> iSmoothed = _poly.getYEstVector();

  // fit Q

  _poly.performFit(rawQ);
  vector<double> qSmoothed = _poly.getYEstVector();

  // save results as complex
  
  vector<complex<double> > iqRegr(nSamples, empty);
  for (int ii = 0; ii < nSamples; ii++) {
    double filteredI = rawI[ii] - iSmoothed[ii];
    double filteredQ = rawQ[ii] - qSmoothed[ii];
    iqRegr[ii] = complex<double>(filteredI, filteredQ);
    if (iqPolyFit) {
      iqPolyFit[ii] = complex<double>(iSmoothed[ii], qSmoothed[ii]);
    }
  }
  
  // if iqNotched is non-NULL,
  // save filtered data, without interp across the notch
  
  if (iqNotched != NULL) {
    memcpy(iqNotched, iqRegr.data(), nSamples * sizeof(complex<double>));
  }

  // take the forward fft to compute the complex spectrum
  // of regr-filtered series
  
  vector<complex<double> > regrSpecC(nSamples, empty);
  _fft.fwd(iqRegr.data(), regrSpecC.data());
  
  // compute the real regr-filtered spectrum
  
  loadPower(nSamples, regrSpecC.data(), regrSpec.data());
  
  // interpolate across the notch, computing the power before and after
  
  doInterpAcrossNotch(regrSpec, _notchStart, _notchEnd);

  // compute interp power ratio
  
  double powerAfterInterp = meanPower(nSamples, regrSpec.data());
  
  // compute spectral noise value
  
  _spectralNoise = computeSpectralNoise(nSamples, regrSpec.data());
  
  // compute SNR based on the spectral noise

  _spectralSnr = _spectralNoise / calNoise;

  // compute powers and filter ratio
  
  double rawPower = meanPower(nSamples, iqRaw);
  double filteredPower = powerAfterInterp;
  double powerRemoved = rawPower - filteredPower;
  _filterRatio = rawPower / filteredPower;
  
  // correct the filtered powers for clutter residue

  if (powerRemoved > 0 && applyDbForDbCorrection) {
    double correctionRatio =
      computeDbForDbCorrectionRatio(nSamples, _spectralSnr,
                                    rawPower, filteredPower,
                                    powerRemoved, calNoise);

    // correct the filtered powers for clutter residue
    for (int ii = 0; ii < nSamples; ii++) {
      regrSpec[ii] *= correctionRatio;
    }
  }
  
  // adjust the input spectrum by the filter ratio
  // constrain ratios to be 1 or less
  // this preserves the phases on the spectrum
  
  for (int ii = 0; ii < nSamples; ii++) {
    double magRatio = sqrt(regrSpec[ii] / unfiltSpec[ii]);
    if (magRatio > 1.0) {
      magRatio = 1.0;
    }
    rawSpecC[ii] = complex<double>(rawSpecC[ii].real() * magRatio,
                                   rawSpecC[ii].imag() * magRatio);
  }
  
  // invert the resulting fft
  // storing result in the filtered time series
  
  _fft.inv(rawSpecC.data(), iqFiltered);
  
}

/////////////////////////////////////////////////////////////////////////
// compute the power from the central 3 points in the FFT

double RegrFilter::compute3PtClutPower(int nSamples, const complex<double> *rawIq)
{
  double sumPower = 0.0; 
  for (int kk = 0; kk < 3; kk++) {
    if (kk == 2) {
      kk = nSamples - 1;
    }
    double sumReal = 0.0;
    double sumImag = 0.0;
    for (int tt = 0; tt < nSamples; tt++) {  // For each input element
      double angle = 2.0 * M_PI * tt * kk / nSamples;
      sumReal +=  rawIq[tt].real() * cos(angle) + rawIq[tt].imag() * sin(angle);
      sumImag += -rawIq[tt].real() * sin(angle) + rawIq[tt].imag() * cos(angle);
    }
    double power = (sumReal * sumReal + sumImag * sumImag) / nSamples;
    sumPower += power;
  }
  return sumPower / nSamples;
}

/////////////////////////////////////////////
// compute mean power of time series

double RegrFilter::meanPower(int nSamples, const complex<double> *c1)
{
  if (nSamples < 1) {
    return 0.0;
  }
  double sum = 0.0;
  for (int ipos = 0; ipos < nSamples; ipos++, c1++) {
    sum += ((c1->real() * c1->real()) + (c1->imag() * c1->imag()));
  }
  return sum / (double) nSamples;
}

double RegrFilter::meanPower(int nSamples, const double *pwr)
{
  if (nSamples < 1) {
    return 0.0;
  }
  double sum = 0.0;
  for (int ipos = 0; ipos < nSamples; ipos++, pwr++) {
    sum += (*pwr * *pwr);
  }
  return sum / (double) nSamples;
}

////////////////////////////////////////
// load power from complex

void RegrFilter::loadPower(int nSamples, const complex<double> *in, double *power)

{
  
  for (int ii = 0; ii < nSamples; ii++, in++, power++) {
    *power = in->real() * in->real() + in->imag() * in->imag();
  }

}

/////////////////////////////////////////////////////////////////////////
// compute csr and cnr

void RegrFilter::computePowerRatios(int nSamples, double calNoise, const complex<double> *iqRaw,
                                    double &cnrDb, double &csrDb)
{
  double clutPower = compute3PtClutPower(nSamples, iqRaw); // see above
  double cnr = clutPower / calNoise;
  cnrDb = 10.0 * log10(cnr);
  double totalPower = meanPower(nSamples, iqRaw);
  double signalPower = totalPower - clutPower;
  double csr = clutPower / signalPower;
  csrDb = 10.0 * log10(csr);
}

/////////////////////////////////////////////////////////////////////////
// compute polynomial order

int RegrFilter::computePolyOrder(int nSamples,
                                 double cnrDb,
                                 double antRateDegPerSec,
                                 double prtSecs,
                                 double wavelengthM)
{
  if (cnrDb < 1) {
    cnrDb = 1.0;
  }
  double ss = 1.0;
  double wc = ss * (0.03 + 0.017 * antRateDegPerSec);
  double nyquist = wavelengthM / (4.0 * prtSecs);
  double wcNorm = wc / nyquist;
  double orderNorm = -1.9791 * wcNorm * wcNorm + 0.6456 * wcNorm;
  int order = ceil(orderNorm * pow(cnrDb, 0.666667) * nSamples);
  if (order < 1) {
    order = 1;
  } else if (order > (int) nSamples - 2) {
    order = nSamples - 2;
  }
  return order;
}

///////////////////////////////
// fit gaussian to spectrum

void RegrFilter::fitGaussian(const double *power,
                             int nSamples, 
                             int weatherPos,
                             double spectralNoise,
                             double *gaussian)
  
{

  // center power array on the max value
  
  vector<double> centered_(nSamples, 0.0);
  double *centered = centered_.data();
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

double RegrFilter::computeSpectralNoise(int nSamples,
                                        const double *powerSpec)
  
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
  
  // now move through the spectrum one point at a time, computing
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

//////////////////////////////////////////////////////////////////////
// Interpolate across the regression filter notch

void RegrFilter::doInterpAcrossNotch(vector<double> &regrSpec,
                                     int &notchStart,
                                     int &notchEnd)

{

  // find start and end of notch
  // we start searching in the middle of the notch - spectral point 0
  // and then move away from the center, computing the power change
  // it increases as we move away from the center point
  // the notch limits are defined by where the power starts to decrease again
  // and we retain the inflection points

  int nSamples = regrSpec.size();
  int nSamplesHalf = nSamples / 2;

  // compute notch min power
  
  double notchMinPower = regrSpec[0];

  double prevPower = notchMinPower;
  int notchUpperBound = 1;
  for (int ii = 1; ii < nSamplesHalf - 2; ii++) {
    double power = regrSpec[ii];
    if (power < prevPower / 1.25) { // 1 dB down
      notchUpperBound = ii - 1;
      break;
    }
    prevPower = power;
  }
  
  prevPower = notchMinPower;
  int notchLowerBound = -1;
  for (int ii = 1; ii < nSamplesHalf - 2; ii++) {
    int jj = (nSamples - ii) % nSamples;
    double power = regrSpec[jj];
    if (power < prevPower / 1.25) { // 1 dB down
      notchLowerBound = jj + 1 - nSamples;
      break;
    }
    prevPower = power;
  }
  
  notchStart = notchLowerBound + nSamples;
  notchEnd = notchUpperBound;
  int nUnfiltered = notchStart - notchEnd + 1;
  // int nFiltered = nSamples - nUnfiltered;

  // gaussian interp
  
  double *startNonNotch = regrSpec.data() + notchUpperBound + 1;
  
  // compute the noise in the filtered spectrum, but not the notch
  
  double regrNoise = computeSpectralNoise(nUnfiltered, startNonNotch);
  
  // find the location of the max power in the filtered spectrum,
  // presumably the weather position
  
  int weatherPos = 0;
  double maxRegrPower = regrSpec[0];
  for (int ii = 1; ii < nSamples; ii++) {
    if (regrSpec[ii] > maxRegrPower) {
      weatherPos = ii;
      maxRegrPower = regrSpec[ii];
    }
  }
  
  // interpolate across the filtered notch
  // iterate 3 times, refining the correcting further each time
  // by fitting a Gaussian to the spectrum
  
  vector<double> gaussian;
  gaussian.resize(nSamples);
  for (int iter = 0; iter < 3; iter++) {
    // fit gaussian to notched spectrum
    fitGaussian(regrSpec.data(), nSamples,
                weatherPos, regrNoise,
                gaussian.data());
    for (int ii = notchLowerBound; ii <= notchUpperBound; ii++) {
      int jj = (ii + nSamples) % nSamples;
      regrSpec[jj] = gaussian[jj];
    }
  } // iter
  
}
    
/////////////////////////////////////////////////////////////////
// Compute correction ratio to be applied to filtered time series
// to account for noise added to the spectrum by the clutter peak
//
// The computations are carried out in dB space, because the
// corrections are normally discussed in this manner.

double RegrFilter::computeDbForDbCorrectionRatio(int nSamples,
                                                 double spectralSnr,
                                                 double rawPower,
                                                 double filteredPower,
                                                 double powerRemoved,
                                                 double calNoise)
  
{

  // check the SNR

  double snr = (rawPower - calNoise) / calNoise;
  if (snr <= 0) {
    return 1.0;
  }
  double snrDb = 10.0 * log10(snr);
  if (snrDb < 75.0) {
    return 1.0;
  }
  
  // apply the legacy NEXRAD clutter residue correction
    
  double dbForDbRatio = 0.2;
  double dbForDbThreshold = 40.0;
  
  double powerTotalDb = 10.0 * log10(rawPower);
  double powerLeft = rawPower - powerRemoved;
  if (powerLeft < 1.0e-12) {
    powerLeft = 1.0e-12;
  }
  double powerLeftDb = 10.0 * log10(powerLeft);
  double diffDb = powerTotalDb - powerLeftDb;
  double powerRemovedDbCorrection = diffDb * dbForDbRatio;
  if (diffDb > dbForDbThreshold) {
    powerRemovedDbCorrection += (diffDb - dbForDbThreshold);
  }
  double correctionRatio = 1.0 / pow(10.0, powerRemovedDbCorrection / 10.0);
  return correctionRatio;
  
}


