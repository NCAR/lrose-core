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
// regression_test.cc
//
// testing the regression filter
//
////////////////////////////////////////////////////////////////////////
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2023
//
///////////////////////////////////////////////////////////////

#include <string>
#include <cmath>
#include <cstdio>
#include <cstddef>
#include <iostream>
#include "regression_test.hh"
using namespace std;

/////////////////////////////////////////////////////////////////////////
// compute the power from the central 3 points in the FFT

double compute3PtClutPower(size_t nSamples, const complex<double> *rawIq)
{
  double sumPower = 0.0; 
  for (size_t kk = 0; kk < 3; kk++) {
    if (kk == 2) {
      kk = nSamples - 1;
    }
    double sumReal = 0.0;
    double sumImag = 0.0;
    for (size_t tt = 0; tt < nSamples; tt++) {  // For each input element
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

double meanPower(size_t nSamples, const complex<double> *c1)
  
{
  if (nSamples < 1) {
    return 0.0;
  }
  double sum = 0.0;
  for (size_t ipos = 0; ipos < nSamples; ipos++, c1++) {
    sum += ((c1->real() * c1->real()) + (c1->imag() * c1->imag()));
  }
return sum / (double) nSamples;
}

/////////////////////////////////////////////////////////////////////////
// compute csr and cnr

void computePowerRatios(size_t nSamples, double calNoise, const complex<double> *iqRaw,
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

int computePolyOrder(size_t nSamples,
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
  } else if (order > (int) nSamples - 1) {
    order = nSamples - 1;
  }
  return order;
}

///////////////////////////////
// fit gaussian to spectrum

void fitGaussian(const double *power,
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

double computeSpectralNoise(const double *powerSpec,
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

void doInterpAcrossNotch(vector<double> &regrSpec,
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
  
  double regrNoise = computeSpectralNoise(startNonNotch, nUnfiltered);
  
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
    
