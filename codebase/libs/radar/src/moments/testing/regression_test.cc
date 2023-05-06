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

/********************************************************************
 * Approximation of a discrete real function F(x) by least squares
 * ---------------------------------------------------------------
 * Refs:
 *
 * (a) Méthodes de calcul numérique, Tome 2 by Claude
 *     Nowakowski, PSI Edition, 1984" [BIBLI 04].
 *
 * (b) Generation and use of orthogonal polynomials for data-fitting
 *     with a digital computer.
 *     George E. Forsythe.
 *     J.Soc.Indust.Appl.Math, Vol 5, No 2, June 1957.
 *
 * (c) Basic Scientific Subroutines, Vol II.
 *     Fred Ruckdeschel. McGraw Hill, 1981.
 *
 ********************************************************/
/////////////////////////////////////////////////////////////
// ForsytheFit.hh
//
// Regression fit to (x, y) data set using Forsythe polynomials
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2020
//
///////////////////////////////////////////////////////////////

class ForsytheFit {
  
public:
  
  // constructor

  ForsytheFit();
  
  // destructor
  
  virtual ~ForsytheFit();

  // clear the data values

  void clear();

  // perform a fit, given (x, y) vals
  
  int performFit(size_t order,
                 const vector<double> &xVals,
                 const vector<double> &yVals);

  // get order
  
  int getOrder() const { return _order; }

  // get number of values
  
  size_t getNVals() const { return _nObs; }
  
  // get coefficients after fit
  
  const vector<double> getCoeffs() const { return _coeffs; }
  
  // get single y value, given the x value
  
  double getYEst(double xx);

  // get the full vector of estimated Y values
  
  const vector<double> &getYEstVector();

  // compute standard error of estimate for the fit
  
  double computeStdErrEst(double &rSquared);

protected:
private:

  size_t _order;        // polynomial order

  vector<double> _coeffs; // polynomial coefficients (0-based)
  
  vector<double> _xObs, _yObs; // observations
  size_t _nObs; // number of obs
  
  vector<double> _yEst; // regression estimate of y, size _nObs

  // arrays for the fitting procedure
  
  vector<double> _aa, _bb, _ff, _cc; // size _order + 2 (1-based)
  vector<double> _dd, _ee; // size _nObs (0-based)
  vector<vector<double> > _bbSave, _eeSave; // if prepareForFit is used
  vector<vector<double> > _xPowers; // polynomial powers of x [_nObs][_order+1]

  // private methods

  void _allocDataArrays();
  void _allocPolyArrays();

};

///////////////////////////////////////////////////////////////
// ForsytheFit.cc
//
// Regression fit to (x, y) data set using Forsythe polynomials
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2020
//
///////////////////////////////////////////////////////////////
// NOTE on array indices
//   The obs arrays (_xObs, _yObs) and associated 
//     arrays (dd, ee) are 0-based.
//   The order arrays (aa, bb, ff, c2) are 1-based.
///////////////////////////////////////////////////////////////

////////////////////////////////////////////////////
// constructor

ForsytheFit::ForsytheFit()
        
{
  clear();
}

////////////////////////////////////////////////////
// destructor

ForsytheFit::~ForsytheFit()
  
{
  clear();
}

//////////////////////////////////////////////////////////////////
// clear the data values
  
void ForsytheFit::clear()
{
  _order = 0;
  _xObs.clear();
  _yObs.clear();
  _yEst.clear();
  _nObs = 0;
}

////////////////////////////////////////////////////
// perform a fit

int ForsytheFit::performFit(size_t order,
                            const vector<double> &xVals,
                            const vector<double> &yVals)
  
{

  _order = order;
  _allocPolyArrays();
  _xPowers.clear();
  
  _xObs = xVals;
  _yObs = yVals;
  _nObs = _xObs.size();
  if (_xObs.size() != _yObs.size()) {
    cerr << "ERROR - ForsytheFit::performFit" << endl;
    cerr << "  number of X and Y vals differ, they should be the same" << endl;
    cerr << "  n X vals: " << _xObs.size() << endl;
    cerr << "  n Y vals: " << _yObs.size() << endl;
    return -1;
  }

  // allocate arrays
  
  _allocDataArrays();

  // check conditions

  if (_nObs < _order + 2) {
    cerr << "ERROR - ForsytheFit::performFit()" << endl;
    cerr << "  Not enough observations to  fit order: " << _order << endl;
    cerr << "  Min n obs: " << _order << endl;
    return -1;
  }

  size_t order1 = _order + 1;
  
  // Initialize the order arrays - 1-based

  for (size_t ii = 1; ii <= order1; ++ii) {
    _aa[ii] = 0.0;
    _bb[ii] = 0.0;
    _cc[ii] = 0.0;
    _ff[ii] = 0.0;
  }
  
  double rootN = sqrt((double) _nObs);
  _bb[1] = 1.0 / rootN;
  
  for (size_t ii = 0; ii < _nObs; ++ii) {
    _ee[ii] = 1.0 / rootN;
  }
  
  double a1 = 0.0;
  for (size_t ii = 0; ii < _nObs; ++ii) {
    a1 += (_xObs[ii] * _ee[ii] * _ee[ii]);
  }

  double c1 = 0.0;
  for (size_t ii = 0; ii < _nObs; ++ii) {
    c1 += (_yObs[ii] * _ee[ii]);
  }
  _ff[1] = _bb[1] * c1;

  // Initialize the sample arrays - 0-based

  for (size_t ii = 0; ii < _nObs; ++ii) {
    _dd[ii] = 0.0;
  }

  // Main loop, increasing the order as we go

  double f1 = rootN;
  for (size_t iorder = 2; iorder <= order1; iorder++) {
    
    double f1Prev = f1;
    double a1Prev = a1;
    
    double f1Sq = 0.0;
    for (size_t ii = 0; ii < _nObs; ++ii) {
      double ddPrev = _dd[ii];
      _dd[ii] = _ee[ii];
      _ee[ii] = (_xObs[ii] - a1Prev) * _ee[ii] - f1Prev * ddPrev;
      f1Sq += _ee[ii] * _ee[ii];
    }
    f1 = sqrt(f1Sq);

    for (size_t ii = 0; ii < _nObs; ++ii) {
      _ee[ii] /= f1;
    }
    a1 = 0.0;
    for (size_t ii = 0; ii < _nObs; ++ii) {
      a1 += (_xObs[ii] * _ee[ii] * _ee[ii]);
    }

    c1 = 0.0;
    for (size_t ii = 0; ii < _nObs; ++ii) {
      c1 += _ee[ii] * _yObs[ii];
    }

    for (size_t jj = 0; jj < iorder; jj++) {
      size_t ll = iorder - jj;
      double bbSave = _bb[ll];
      double bbNew = 0.0;
      if (ll > 1) {
        bbNew = _bb[ll - 1];
      }
      bbNew = bbNew - a1Prev * _bb[ll] - f1Prev * _aa[ll];
      _bb[ll] = bbNew / f1;
      _aa[ll] = bbSave;
    } // jj

    for (size_t ii = 1; ii <= order1; ++ii) {
      _ff[ii] += _bb[ii] * c1;
      _cc[ii] = _ff[ii];
    }

  } // iorder

  // Load _coeffs arrays - this is 0 based instead of 1 based
  // so shift down by 1

  for (size_t ii = 1; ii <= order1; ++ii) {
    _coeffs[ii - 1] = _cc[ii];
  }

  return 0;

}

////////////////////////////////////////////////////
// get single y value, given the x value

double ForsytheFit::getYEst(double xx)
{
  if (_coeffs.size() < 1) {
    return 0.0;
  }
  double yy = _coeffs[0];
  double fac = xx;
  for (size_t ii = 1; ii < _coeffs.size(); ii++) {
    yy += _coeffs[ii] * fac;
    fac *= xx;
  }
  return yy;
}

////////////////////////////////////////////////////
// get full vector of estimated Y value

const vector<double> &ForsytheFit::getYEstVector()
{ 
  _yEst.clear();
  for (size_t ii = 0; ii < _nObs; ii++) {
    _yEst.push_back(getYEst(_xObs[ii]));
  }
  return _yEst;
}

////////////////////////////////////////////////////
// compute standard error of estimate for the fit

double ForsytheFit::computeStdErrEst(double &rSquared)
{

  // compute mean

  double sumy = 0.0;
  for (size_t ii = 0; ii < _nObs; ii++) {
    sumy += _yObs[ii];
  }
  double meany = sumy / _nObs;

  // compute sum of residuals
  
  double sum_dy_squared = 0.0;
  double sum_of_residuals = 0.0;
  for (size_t ii = 0; ii < _nObs; ii++) {
    double xx = _xObs[ii];
    double yy = _yObs[ii];
    double dy = yy - meany;
    double yyEst = getYEst(xx);
    double error = yyEst - yy;
    sum_of_residuals += error * error;
    sum_dy_squared += dy * dy;
  }

  // compute standard error of estimate and r-squared
    
  double stdErrEst = sqrt(sum_of_residuals / (_nObs - 3.0));
  rSquared = (sum_dy_squared - sum_of_residuals) / sum_dy_squared;
  return stdErrEst;

}

/////////////////////////////////////////////////////
// allocate space

void ForsytheFit::_allocDataArrays()

{
  
  _dd.resize(_nObs);
  _ee.resize(_nObs);
  _yEst.clear();
  
}

void ForsytheFit::_allocPolyArrays()

{

  _aa.resize(_order + 2);
  _bb.resize(_order + 2);
  _cc.resize(_order + 2);
  _ff.resize(_order + 2);
  _coeffs.resize(_order + 1);
  
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
    
