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
// Moments.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2003
//
///////////////////////////////////////////////////////////////
//
// Moments handles computation of the radar moments
//
////////////////////////////////////////////////////////////////

#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <cassert>
#include <cstring>
#include <toolsa/os_config.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/file_io.h>
#include "Moments.hh"
using namespace std;

// initialize const doubles

const double Moments::_missingDbl = -9999.0;
const double Moments::_smallValue = 1.0e-9;

// Constructor

Moments::Moments(int n_samples /* = 64 */)
  
{

  _debugPrint = false;
  _nSamples = n_samples;

  _fft = NULL;
  
  _wavelengthMeters = 0.1068;
  setNoiseValueDbm(-113.0);
  
  _signalToNoiseRatioThreshold = 3.0;
  _hanning = NULL;
  _blackman = NULL;

  // alloc arrays and matrices

  _hanning = new double[_nSamples];
  _blackman = new double[_nSamples];

  // set up FFT plan
  
  _fft = new Fft(_nSamples);
  
  // init windowing
  
  _initHanning(_hanning);
  _initBlackman(_blackman);

  return;

}

// destructor

Moments::~Moments()

{

  if (_hanning) {
    delete[] _hanning;
  }
  if (_blackman) {
    delete[] _blackman;
  }
  if (_fft) {
    delete _fft;
  }
  
}

/////////////////////////////////////
// set methods

void Moments::setWavelength(double wavelength) {
  _wavelengthMeters = wavelength;
}

void Moments::setNoiseValueDbm(double dbm) {
  _noiseValueDbm = dbm;
  _noiseValueMwatts = pow(10.0, dbm / 10.0);
}

void Moments::setSignalToNoiseRatioThreshold(double db) {
  _signalToNoiseRatioThreshold = db;
}

///////////////////////////////////
// apply hanning window

void Moments::applyHanningWindow(const Complex_t *in,
				 Complex_t *out) const
  
{
  _applyWindow(_hanning, in, out);
}
  
///////////////////////////////////
// apply modified hanning window

void Moments::applyBlackmanWindow(const Complex_t *in,
				  Complex_t *out) const
  
{
  _applyWindow(_blackman, in, out);
}
  
///////////////////////////////////////////////
// compute total power from IQ

double Moments::computePower(const Complex_t *IQ) const
  
{
  double p = 0.0;
  for (int i = 0; i < _nSamples; i++, IQ++) {
    p += ((IQ->re * IQ->re) + (IQ->im * IQ->im));
  }
  return (p / _nSamples);
}
  
///////////////////////////////////////////////
// compute total power from magnitudes

double Moments::computePower(const double *mag) const
  
{
  double p = 0.0;
  for (int i = 0; i < _nSamples; i++, mag++) {
    p += (*mag * *mag);
  }
  return (p / _nSamples);
}
  
///////////////////////////////////////////////
// check if we should threshold based on
// signal-to-noise of total power
//
// Side-effect: passes back total power

bool Moments::checkSnThreshold(const Complex_t *IQ,
			       double &totalPower) const
  
{
  totalPower = computePower(IQ);
  double dbm = 10.0 * log10(totalPower);
  if (dbm < _noiseValueDbm + _signalToNoiseRatioThreshold) {
    return true;
  }
  return false;
}
  
//////////////////////////////////////////////////////////
// compute time-domain moments using ABP pulse-pair method

void Moments::computeByAbp(const Complex_t *IQ,
			   double prtSecs,
			   double &power, double &vel,
			   double &width) const
  
{

  // initialize return vals
  
  power = vel = width = _missingDbl;
  
  // compute a, b, p, r1
  
  double a = 0.0, b = 0.0, p = 0.0;
  
  const Complex_t *iq0 = IQ;
  const Complex_t *iq1 = IQ + 1;
  
  p += ((iq0->re * iq0->re) + (iq0->im * iq0->im));
  
  for (int i = 0; i < _nSamples - 1; i++, iq0++, iq1++) {
    a += ((iq0->re * iq1->re) + (iq0->im * iq1->im));
    b += ((iq0->re * iq1->im) - (iq1->re * iq0->im));
    p += ((iq1->re * iq1->re) + (iq1->im * iq1->im));
  }
  double r1_val = sqrt(a * a + b * b) / _nSamples;
  
  // mom0

  double mom0 = p / _nSamples;

  // compute c, d, r2
  
  double c = 0.0, d = 0.0;
  
  iq0 = IQ;
  const Complex_t *iq2 = IQ + 2;

  for (int i = 0; i < _nSamples - 2; i++, iq0++, iq2++) {
    c += ((iq0->re * iq2->re) + (iq0->im * iq2->im));
    d += ((iq0->re * iq2->im) - (iq2->re * iq0->im));
  }
  double r2_val = sqrt(c * c + d * d) / _nSamples;
  
  // mom1 from pulse-pair
  
  double nyquist = _wavelengthMeters / (4.0 * prtSecs);
  double nyqFac = nyquist / M_PI;
  double mom1_pp;
  if (a == 0.0 && b == 0.0) {
    mom1_pp = 0.0;
  } else {
    mom1_pp = nyqFac * atan2(b, a);
  }
  
  // mom2 from pulse-pair

  double mom2_pp_fac = M_SQRT2 * nyquist / M_PI;
  double s_hat = mom0 - _noiseValueMwatts;
  if (s_hat < 1.0e-6) {
    s_hat = 1.0e-6;
  }
  double ln_ratio = log(s_hat / r1_val);
  double mom2_pp;
  if (ln_ratio > 0) {
    mom2_pp = mom2_pp_fac * sqrt(ln_ratio);
  } else {
    // mom2_pp = -1.0 * mom2_pp_fac * sqrt(fabs(ln_ratio));
    mom2_pp = 0;
  }
  
  // mom2 from R1R2

  double mom2_r1r2 = 0.0;
  double r1r2_fac = (2.0 * nyquist) / (M_PI * sqrt(6.0));
  double ln_r1r2 = log(r1_val/r2_val);
  if (ln_r1r2 > 0) {
    mom2_r1r2 = r1r2_fac * sqrt(ln_r1r2);
  } else {
    mom2_r1r2 = r1r2_fac * -1.0 * sqrt(fabs(ln_r1r2));
  }
  
  if (_debugPrint) {
    cerr << "  Pulse-pair estimates:" << endl;
    cerr << "    r1: " << r1_val << endl;
    cerr << "    r2: " << r2_val << endl;
    cerr << "    mom0: " << mom0 << endl;
    cerr << "    mom1_pp: " << mom1_pp << endl;
    cerr << "    mom2_pp: " << mom2_pp << endl;
    cerr << "    mom2_r1r2: " << mom2_r1r2 << endl;
  }

  power = mom0;
  vel = -1.0 * mom1_pp;
  width = mom2_r1r2;

}

///////////////////////////////////////////////
// compute time-domain moments using pulse-pair

void Moments::computeByPp(const Complex_t *IQ,
			  double prtSecs,
			  double &power, double &vel,
			  double &width, int &flags) const
  
{

  // initialize return vals

  power = vel = width = _missingDbl;
  flags = 0;
  
  // check thresholding on total power
  
  if (checkSnThreshold(IQ, power)) {
    flags = _censorOnTotalPower;
    return;
  }

  // compute velocity and width

  double vv, ww;
  _velWidthFromTd(IQ, prtSecs, vv, ww);
  
  // set return vals, changing the sign of the velocity so that
  // motion away from the radar is positive

  vel = -1.0 * vv;
  width = ww;

  if (_debugPrint) {
    cerr << "  Pulse pair estimates:" << endl;
    cerr << "    power: " << power << endl;
    cerr << "    vel: " << vel << endl;
    cerr << "    width: " << width << endl;
  }

}

////////////////////////////////////////////////////
// compute fft-based moments

void Moments::computeByFft(const Complex_t *IQ,
			   window_t windowType,
			   double prtSecs,
			   double &power, double &noise,
			   double &vel, double &width,
			   int &flags) const
  
{
  
 // initialize return vals

  power = vel = width = noise = _missingDbl;
  flags = 0;
  
  // check thresholding on power

  if (checkSnThreshold(IQ, power)) {
    flags = _censorOnTotalPower;
    return;
  }

  // apply window

  Complex_t *windowedIq = new Complex_t[_nSamples];
  if (windowType == WINDOW_HANNING) {
    applyHanningWindow(IQ, windowedIq);
  } else if (windowType == WINDOW_BLACKMAN) {
    applyBlackmanWindow(IQ, windowedIq);
  } else {
    memcpy(windowedIq, IQ, _nSamples * sizeof(Complex_t));
  }

  // compute fft
  
  Complex_t *spectrum = new Complex_t[_nSamples];
  _fft->fwd(windowedIq, spectrum);
  
  // compute vel and width

  double *specMag = new double[_nSamples];
  _loadMag(spectrum, specMag);
  double vv, ww, measuredNoise;
  _velWidthFromFft(specMag, prtSecs, vv, ww, measuredNoise);

  // set return vals, changing the sign of the velocity so that
  // motion away from the radar is positive

  vel = -1.0 * vv;
  width = ww;
  noise = measuredNoise;
  
  if (_debugPrint) {
    cerr << "  Spectral estimates:" << endl;
    cerr << "    power: " << power << endl;
    cerr << "    vel: " << vel << endl;
    cerr << "    width: " << width << endl;
    cerr << "    noise: " << noise << endl;
  }

  // free up
  
  delete[] windowedIq;
  delete[] spectrum;
  delete[] specMag;

}

///////////////////////////////////
// initialize windowing functions

void Moments::_initHanning(double *window)

{
  
  // compute hanning window
  
  for (int ii = 0; ii < _nSamples; ii++) {
    double ang = 2.0 * M_PI * ((ii + 0.5) / (double) _nSamples - 0.5);
    window[ii] = 0.5 * (1.0 + cos(ang));
  }

  // adjust window to keep power constant

  double sumsq = 0.0;
  for (int ii = 0; ii < _nSamples; ii++) {
    sumsq += window[ii] * window[ii];
  }
  double rms = sqrt(sumsq / _nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    window[ii] /= rms;
  }

}
  
void Moments::_initBlackman(double *window)

{
  
  // compute blackman window
  
  for (int ii = 0; ii < _nSamples; ii++) {
    double pos =
      ((_nSamples + 1.0) / 2.0 + (double) ii ) / _nSamples;
    window[ii] = (0.42
		  + 0.5 * cos(2.0 * M_PI * pos)
		  + 0.08 * cos(4.0 * M_PI * pos));
  }
  
  // adjust window to keep power constant

  double sumsq = 0.0;
  for (int ii = 0; ii < _nSamples; ii++) {
    sumsq += window[ii] * window[ii];
  }
  double rms = sqrt(sumsq / _nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    window[ii] /= rms;
  }
  
}
  
///////////////////////////////////
// apply window

void Moments::_applyWindow(const double *window,
			   const Complex_t *in,
			   Complex_t *out) const
  
{
  
  const double *ww = window;
  const Complex_t *inp = in;
  Complex_t *outp = out;
  
  for (int ii = 0; ii < _nSamples; ii++, ww++, inp++, outp++) {
    outp->re = inp->re * *ww;
    outp->im = inp->im * *ww;
  }

}
  
///////////////////////////////
// load magnitudes from IQ

void Moments::_loadMag(const Complex_t *in, double *mag) const

{
  
  for (int ii = 0; ii < _nSamples; ii++, in++, mag++) {
    // *mag = sqrt(in->re * in->re + in->im * in->im);
    double mm = sqrt(in->re * in->re + in->im * in->im);
    //      if (mm > 1.0e-8) {
    *mag = mm;
    //      } else {
    //        *mag = 0.0;
    //      }
  }

}

///////////////////////////////
// load power from IQ

void Moments::_loadPower(const Complex_t *in, double *power) const

{
  
  for (int ii = 0; ii < _nSamples; ii++, in++, power++) {
    *power = in->re * in->re + in->im * in->im;
  }

}

/////////////////////////////////////////////////////
// compute noise for the spectral power

void Moments::_computeSpectralNoise(const double *powerCentered,
				    double &noiseMean,
				    double &noiseSdev) const
  
{

  // We compute the mean power for 3 regions of the spectrum:
  //   1. 1/8 at lower end plus 1/8 at upper end
  //   2. 1/4 at lower end
  //   3. 1/4 at uppoer end
  // We estimate the noise to be the least of these 3 values
  // because if there is a weather echo it will not affect both ends
  // of the spectrum unless the width is very high, in which case we
  // probablyhave a bad signal/noise ratio anyway

  int nby4 = _nSamples / 4;
  int nby8 = _nSamples / 8;
  
  // combine 1/8 from each end

  double sumBoth = 0.0;
  double sumSqBoth = 0.0;
  const double *pw = powerCentered;
  for (int ii = 0; ii < nby8; ii++, pw++) {
    sumBoth += *pw;
    sumSqBoth += *pw * *pw;
  }
  pw = powerCentered + _nSamples - nby8 - 1;
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
  pw = powerCentered + _nSamples - nby4 - 1;
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

#ifdef NOTNOW
  {
    double sum = 0.0;
    const double *pw = powerCentered;
    for (int ii = 0; ii < _nSamples; ii++, pw++) {
      sum += 1.0 / (pow(*pw, 4.0));
    }
    double hnoise = (double) _nSamples / pow(sum, 0.25);
    cerr << "noiseMean, hnoise, ratio: " << noiseMean << ", " << hnoise << ", " << noiseMean/hnoise << endl;
  }
#endif

}

//////////////////////////////////////////////////////
// compute vel and width in time domain

void Moments::_velWidthFromTd(const Complex_t *IQ,
			      double prtSecs,
			      double &vel, double &width) const
  
{

  // compute a, b, r1
  
  double a = 0.0, b = 0.0;
  
  const Complex_t *iq0 = IQ;
  const Complex_t *iq1 = IQ + 1;
  
  for (int i = 0; i < _nSamples - 1; i++, iq0++, iq1++) {
    a += ((iq0->re * iq1->re) + (iq0->im * iq1->im));
    b += ((iq0->re * iq1->im) - (iq1->re * iq0->im));
  }
  double r1_val = sqrt(a * a + b * b) / _nSamples;
  
  // compute c, d, r2
  
  double c = 0.0, d = 0.0;
  
  iq0 = IQ;
  const Complex_t *iq2 = IQ + 2;

  for (int i = 0; i < _nSamples - 2; i++, iq0++, iq2++) {
    c += ((iq0->re * iq2->re) + (iq0->im * iq2->im));
    d += ((iq0->re * iq2->im) - (iq2->re * iq0->im));
  }
  double r2_val = sqrt(c * c + d * d) / _nSamples;
  
  // velocity
  
  double nyquist = _wavelengthMeters / (4.0 * prtSecs);
  double nyqFac = nyquist / M_PI;
  if (a == 0.0 && b == 0.0) {
    vel = 0.0;
  } else {
    vel = nyqFac * atan2(b, a);
  }
  
  // width from R1R2

  double r1r2_fac = (2.0 * nyquist) / (M_PI * sqrt(6.0));
  double ln_r1r2 = log(r1_val/r2_val);
  if (ln_r1r2 > 0) {
    width = r1r2_fac * sqrt(ln_r1r2);
  } else {
    width = r1r2_fac * -1.0 * sqrt(fabs(ln_r1r2));
  }
  
  if (_debugPrint) {
    cerr << "  Pulse-pair estimates:" << endl;
    cerr << "    r1: " << r1_val << endl;
    cerr << "    r2: " << r2_val << endl;
  }
  
}

//////////////////////////////////////////////////////
// compute vel and width from the spectrum magnitudes

void Moments::_velWidthFromFft(const double *magnitude,
			       double prtSecs,
			       double &vel, double &width,
			       double &measuredNoise) const
  
{

  int kCent = _nSamples / 2;
  
  // find max magnitude
  
  double maxMag = 0.0;
  int kMax = 0;
  const double *mp = magnitude;
  for (int ii = 0; ii < _nSamples; ii++, mp++) {
    if (*mp > maxMag) {
      kMax = ii;
      maxMag = *mp;
    }
  }
  if (kMax >= kCent) {
    kMax -= _nSamples;
  }

  // center power array on the max value

  double *powerCentered = new double[_nSamples];
  int kOffset = kCent - kMax;
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = (ii + kOffset) % _nSamples;
    double powr = magnitude[ii] * magnitude[ii];
    powerCentered[jj] = powr;
  }
  
  // compute noise floor

  double noiseMean, noiseSdev;
  _computeSpectralNoise(powerCentered, noiseMean, noiseSdev);
  double noiseThreshold = noiseMean + noiseSdev;
  
  // if the signal is noisy, we use the entire spectrum to compute
  // the width. Otherwise we trim down.

  int kStart = 0;
  int kEnd = _nSamples - 1;

  // compute mean power
  
  //    double sumP = 0.0;
  //    const double *mpp = magnitude;
  //    for (int ii = 0; ii < _nSamples; ii++, mpp++) {
  //      sumP += *mpp * *mpp;
  //    }
  // double meanPower = sumP / _nSamples;

  // moving away from the peak, find the points in the spectrum
  // where the signal drops below the noise threshold for at
  // least 3 points
  
  {
    
    int count = 0;
    int nTest = 3;
    kStart = kCent - 1;
    double *pw = powerCentered + kStart;
    for (int ii = kStart; ii >= 0; ii--, pw--) {
      if (*pw < noiseThreshold) {
	count ++;
	if (count >= nTest) {
	  break;
	}
      } else {
	count = 0;
      }
      kStart = ii;
    }
    
    count = 0;
    kEnd = kCent + 1;
    pw = powerCentered + kEnd;
    for (int ii = kEnd; ii < _nSamples; ii++, pw++) {
      if (*pw < noiseThreshold) {
	count ++;
	if (count >= nTest) {
	  break;
	}
      } else {
	count = 0;
      }
      kEnd = ii;
    }
    
  }

  // compute mom1 and mom2, using those points above the noise floor

  double sumPower = 0.0;
  double sumK = 0.0;
  double sumK2 = 0.0;
  double *pw = powerCentered + kStart;
  for (int ii = kStart; ii <= kEnd; ii++, pw++) {
    double phase = (double) ii;
    double pExcess = *pw - noiseMean;
    if (pExcess < 0.0) {
      pExcess = 0.0;
    }
    sumPower += pExcess;
    sumK += pExcess * phase;
    sumK2 += pExcess * phase * phase;
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

  double velFac = _wavelengthMeters / (2.0 * _nSamples * prtSecs);
  vel = velFac * (meanK - kOffset);
  width = velFac * sdevK;
  measuredNoise = noiseMean;

  if (_debugPrint) {
    cerr << "    kMax: " << kMax << endl;
    cerr << "    kOffset: " << kOffset << endl;
    cerr << "    noiseMean: " << noiseMean << endl;
    cerr << "    noiseSdev: " << noiseSdev << endl;
    cerr << "    kStart: " << kStart << endl;
    cerr << "    kEnd: " << kEnd << endl;
    cerr << "    meanK: " << meanK << endl;
    cerr << "    sdevK: " << sdevK << endl;
  }

  // free up

  delete[] powerCentered;

}

