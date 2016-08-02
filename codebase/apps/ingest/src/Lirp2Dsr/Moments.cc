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
#include <toolsa/os_config.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/file_io.h>
#include "Moments.hh"
#include "ClutFilter.hh"
using namespace std;

// initialize const doubles

const double Moments::_missingDbl = -9999.0;
const double Moments::_smallValue = 1.0e-9;
const double Moments::_szFracPower75 = 0.25;
const double Moments::_szFracPower50 = 0.5;
const double Moments::_maxClutterVel = 1.0;
const double Moments::_initNotchWidth = 1.5;
const double Moments::_maxClutterVelSz = 2.0;
const double Moments::_initNotchWidthSz = 2.5;

// Constructor

Moments::Moments(int n_samples /* = 64 */)
  
{

  _nSamples = n_samples;

  _fft = NULL;
  
  _debugPrint = false;
  _debugWriteSpectraFiles = false;
  _debugEl = 0.0;
  _debugAz = 0.0;
  _debugRange = 0.0;
  _debugSpectraDir = "/tmp";

  _wavelengthMeters = 0.1068;
  setNoiseValueDbm(-113.0);
  
  _szNegatePhaseCodes = false;
  _szWindow = WINDOW_HANNING;

  _signalToNoiseRatioThreshold = 3.0;
  _szStrongToWeakPowerRatioThreshold = 45.0;
  _szOutOfTripPowerRatioThreshold = 6.0;
  _szOutOfTripPowerNReplicas = 3;

  _modCode12 = NULL;
  _modCode21 = NULL;
  _hanning = NULL;
  _blackman = NULL;
  _deconvMatrix75 = NULL;
  _deconvMatrix50 = NULL;

  // alloc arrays and matrices

  _modCode12 = new Complex_t[_nSamples];
  _modCode21 = new Complex_t[_nSamples];
  _hanning = new double[_nSamples];
  _blackman = new double[_nSamples];
  _deconvMatrix75 = new double[_nSamples * _nSamples];
  _deconvMatrix50 = new double[_nSamples * _nSamples];

  // set up FFT plan
  
  _fft = new Fft(_nSamples);
  
  // init phase codes

  _initPhaseCodes();

  // initialize deconvolution matrix for 3/4 notch

  _initDeconMatrix(_szNotchWidth75, _szPowerWidth75,
		   _szFracPower75, _deconvMatrix75);
  
  // initialize deconvolution matrix for 1/2 notch
  
  _initDeconMatrix(_szNotchWidth50, _szPowerWidth50,
		   _szFracPower50, _deconvMatrix50);

  // init windowing
  
  _initHanning(_hanning);
  _initBlackman(_blackman);

  return;

}

// destructor

Moments::~Moments()

{

  if (_modCode12) {
    delete[] _modCode12;
  }
  if (_modCode21) {
    delete[] _modCode21;
  }
  if (_hanning) {
    delete[] _hanning;
  }
  if (_blackman) {
    delete[] _blackman;
  }
  if (_deconvMatrix75) {
    delete[] _deconvMatrix75;
  }
  if (_deconvMatrix50) {
    delete[] _deconvMatrix50;
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

void Moments::setSzNegatePhaseCodes(bool status /* = true */) {
  _szNegatePhaseCodes = status;
}

void Moments::setSzWindow(window_t window) {
  _szWindow = window;
}

void Moments::setSignalToNoiseRatioThreshold(double db) {
  _signalToNoiseRatioThreshold = db;
}

void Moments::setSzStrongToWeakPowerRatioThreshold(double db) {
  _szStrongToWeakPowerRatioThreshold = db;
}

void Moments::setSzOutOfTripPowerRatioThreshold(double db) {
  _szOutOfTripPowerRatioThreshold = db;
}

void Moments::setSzOutOfTripPowerNReplicas(int n) {
  _szOutOfTripPowerNReplicas = n;
}

void Moments::setDebugPrint(bool status /* = true */) const {
  _debugPrint = status;
}

void Moments::setDebugWriteSpectra(bool status /* = true */,
				   const string &dir /* = "" */) const {
  _debugWriteSpectraFiles = status;
  _debugSpectraDir = dir;
}

void Moments::setDebugEl(double el) const {
  _debugEl = el;
}

void Moments::setDebugAz(double az) const {
  _debugAz = az;
}

void Moments::setDebugRange(double range) const {
  _debugRange = range;
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
			   int &flags,
			   ClutProb *clutProb /* = NULL */) const
  
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

  Complex_t windowedIq[_nSamples];
  if (windowType == WINDOW_HANNING) {
    applyHanningWindow(IQ, windowedIq);
  } else if (windowType == WINDOW_BLACKMAN) {
    applyBlackmanWindow(IQ, windowedIq);
  } else {
    memcpy(windowedIq, IQ, sizeof(windowedIq));
  }

  // compute fft
  
  Complex_t spectrum[_nSamples];
  _fft->fwd(windowedIq, spectrum);

  // compute vel and width

  double specMag[_nSamples];
  _loadMag(spectrum, specMag);
  double vv, ww, measuredNoise;
  _velWidthFromFft(specMag, prtSecs, vv, ww, measuredNoise);

  if (_debugWriteSpectraFiles) {
    _writeMag2File("specMag", specMag, true);
  }

  // compute clutter probability

  double nyquist = _wavelengthMeters / (4.0 * prtSecs);
  if (clutProb != NULL) {
    clutProb->compute(_nSamples, specMag, nyquist,
                      _maxClutterVel, _initNotchWidth);
    if (_debugPrint) {
      cerr << "============================" << endl;
      cerr << "     az: " << _debugAz << endl;
      cerr << "     el: " << _debugEl << endl;
      cerr << "     range: " << _debugRange << endl;
      cerr << "     clutterPos: " << clutProb->getClutterPos() << endl;
      cerr << "     wxPos: " << clutProb->getWeatherPos() << endl;
      cerr << "     clutterPeak: " << clutProb->getClutterPeak() << endl;
      cerr << "     wxPeak: " << clutProb->getWeatherPeak() << endl;
      cerr << "     clutWxPeakRatio: "
           << clutProb->getClutWxPeakRatio() << endl;
      cerr << "     clutWxPeakSep: "
           << clutProb->getClutWxPeakSeparation() << endl;
      printVector(cerr, "clutter gate", spectrum, true);
    }

  }

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

}

///////////////////////////////////////////////
// compute fft-based moments and filter clutter

void Moments::computeByFftFilterClutter(const Complex_t *IQ,
					window_t windowType,
					double prtSecs,
					double &power, double &snr,
					double &vel, double &width,
					double &clutterPower,
					double &noise) const
  
{

  // initialize return vals
  
  power = snr = vel = width = noise = _missingDbl;
  
   // apply window

  Complex_t windowedIq[_nSamples];
  if (windowType == WINDOW_HANNING) {
    applyHanningWindow(IQ, windowedIq);
  } else if (windowType == WINDOW_BLACKMAN) {
    applyBlackmanWindow(IQ, windowedIq);
  } else {
    memcpy(windowedIq, IQ, sizeof(windowedIq));
  }

 // compute forward fft
  
  Complex_t spectra[_nSamples];
  _fft->fwd(windowedIq, spectra);

  // compute magnitude
  
  double specMag[_nSamples];
  _loadMag(spectra, specMag);
  
  // apply filter
  
  ClutFilter filter;
  double filtered[_nSamples];
  double nyquist = _wavelengthMeters / (4.0 * prtSecs);
  bool clutterFound;
  int notchStart;
  int notchEnd;
  double powerRemoved = 0.0;
  double filteredVel = 0.0;
  double filteredWidth = 0.0;
  
  filter.run(specMag, _nSamples,
             _maxClutterVel, _initNotchWidth, nyquist,
    	     filtered, clutterFound, notchStart, notchEnd,
    	     powerRemoved, filteredVel, filteredWidth);
  
  // filter.notch(specMag, _nSamples, 2.5, nyquist, filtered,
  // clutterFound, notchStart, notchEnd, powerRemoved);
  
  // compute vel and width

  double vv, ww, measuredNoise;
  _velWidthFromFft(filtered, prtSecs, vv, ww, measuredNoise);
  double pwr = computePower(filtered);

  // compute clutter power correction, to reduce the effect of
  // the raised noise floor

  double powerRemovedCorrection = 1.0;
  if (powerRemoved > 0) {
    double powerTotalDb = 10.0 * log10(pwr + powerRemoved);
    double powerLeftDb = 10.0 * log10(pwr);
    double diffDb = powerTotalDb - powerLeftDb;
    if (diffDb > 10) {
      double powerRemovedDbCorrection = diffDb * 0.15;
      if (diffDb > 40) {
        powerRemovedDbCorrection += (diffDb - 40);
      }
      powerRemovedCorrection = pow(10.0, powerRemovedDbCorrection / 10.0);
    }
  }

  power = pwr / powerRemovedCorrection;
  vel = -1.0 * vv;
  width = ww;
  clutterPower = powerRemoved;
  noise = measuredNoise;
  if (pwr > measuredNoise) {
    snr = (pwr - measuredNoise) / measuredNoise;
  } else {
    snr = 1.0e-12;
  }

//      {
//        double noiseFixed = pow(10.0, -89.3 / 10.0);
//        double totalPower = computePower(specMag);
//        double dbRemoved = 10.0 * log10(powerRemoved / pwr);
//        double snrdb = 10.0 * log10((totalPower - noiseFixed) / noiseFixed);
//        if (powerRemoved > 0) {
//          cout << dbRemoved << " "
//  	     << snrdb << " "
//  	     << 10.0*log10(measuredNoise) << " "
//  	     << 10.0*log10(powerRemoved) << " "
//  	     << 10.0*log10(pwr) << endl;
//        }
//      }
  
}

/////////////////////////////////////////////////////////////
// compute moments using SZ 8/64 algorithm and pulse-pair
// for RV dealiasing

void Moments::computeBySzPp(const Complex_t *IQ,
			    const Complex_t *delta12,
			    double prtSecs,
			    double &totalPower,
			    double &power1, double &vel1,
			    double &width1, int &flags1,
			    double &power2, double &vel2,
			    double &width2, int &flags2,
			    int &strongTripFlag,
			    double &powerRatio) const
  
{

  // initialize return vals

  power1 = vel1 = width1 = _missingDbl;
  power2 = vel2 = width2 = _missingDbl;
  flags1 = flags2 = 0;
  strongTripFlag = 0;

  // check for 64 samples

  if (_nSamples != 64) {
    cerr << "ERROR - Moments::computeBySzPp" << endl;
    cerr << "  Can only be run with _nSamples = 64" << endl;
    cerr << "  _nSamples: " << _nSamples << endl;
    return;
  }

  // check thresholding on total power

  if (checkSnThreshold(IQ, totalPower)) {
    flags1 = _censorOnTotalPower;
    flags2 = _censorOnTotalPower;
    return;
  }

  // IQ data comes in cohered to trip 1

  const Complex_t *iqTrip1 = IQ;
      
  // cohere to trip 2
  
  Complex_t iqTrip2[_nSamples];
  _cohereTrip1_to_Trip2(iqTrip1, delta12, iqTrip2);
  
  // compute R1 for each trip
  
  double r1Trip1 = _computeR1(iqTrip1);
  double r1Trip2 = _computeR1(iqTrip2);

  // determine which trip is strongest by comparing R1 from 
  // each trip
  
  double r1Ratio;
  bool trip1IsStrong;
  const Complex_t *iqStrong;
  
  if (r1Trip1 > r1Trip2) {
    trip1IsStrong = true;
    iqStrong = iqTrip1;
    r1Ratio = r1Trip1 / r1Trip2;
    strongTripFlag = 1;
  } else {
    trip1IsStrong = false;
    iqStrong = iqTrip2;
    r1Ratio = r1Trip2 / r1Trip1;
    strongTripFlag = 2;
  }

  if (_debugPrint) {
    cerr << "  R1 trip1: " << r1Trip1 << endl;
    cerr << "  R1 trip2: " << r1Trip2 << endl;
    if (trip1IsStrong) {
      cerr << "  Strong trip is FIRST, ratio:" << r1Ratio << endl;
    } else {
      cerr << "  Strong trip is SECOND, ratio:" << r1Ratio << endl;
    }
  }
    
  // compute moments for strong trip

  double powerStrong = totalPower;
  double velStrong, widthStrong;
  _velWidthFromTd(iqStrong, prtSecs, velStrong, widthStrong);
  
  if (_debugPrint) {
    cerr << "  SZ-8-64 PP estimates for strong trip:" << endl;
    cerr << "    method        : PP" << endl;
    cerr << "    powerStrong  : " << powerStrong << endl;
    cerr << "    velStrong    : " << velStrong << endl;
    cerr << "    widthStrong  : " << widthStrong << endl;
  }
  
  // apply window to strong trip as appropriate
  
  Complex_t windowStrong[_nSamples];
  if (_szWindow == WINDOW_HANNING) {
    applyHanningWindow(iqStrong, windowStrong);
  } else if (_szWindow == WINDOW_BLACKMAN) {
    applyBlackmanWindow(iqStrong, windowStrong);
  } else {
    memcpy(windowStrong, iqStrong, sizeof(windowStrong));
  }
  
  // compute FFT for strong trip
  
  Complex_t windowStrongSpec[_nSamples];
  _fft->fwd(windowStrong, windowStrongSpec);
  
  // apply notch
  
  Complex_t notched[_nSamples];
  int notchStart = _computeNotchStart(_szNotchWidth75,
				      velStrong, prtSecs);
  _applyNotch75(notchStart, windowStrongSpec, notched);

  // compute weak trip power

  double powerWeak = computePower(notched);
  
  // adjust power if required
  
  double corrPowerStrong = powerStrong - powerWeak;
  double corrPowerRatioDb = 10.0 * log10(corrPowerStrong / powerWeak);
  powerRatio = corrPowerRatioDb;
  
  // invert the notched spectra into the time domain
  
  Complex_t notchedTd[_nSamples];
  _fft->inv(notched, notchedTd);
  
  // cohere to the weaker trip
  
  Complex_t weakTd[_nSamples];
  if (trip1IsStrong) {
    _cohereTrip1_to_Trip2(notchedTd, delta12, weakTd);
  } else {
    _cohereTrip2_to_Trip1(notchedTd, delta12, weakTd);
  }

  // take fft of cohered weaker trip
  
  Complex_t weakSpectra[_nSamples];
  _fft->fwd(weakTd, weakSpectra);
  
  // compute magnitude of cohered weak spectrum
  
  double weakMag[_nSamples];
  _loadMag(weakSpectra, weakMag);
  
  // deconvolve weak trip, by multiplying with the
  // deconvoltion matrix
  
  double weakMagDecon[_nSamples];
  memset(weakMagDecon, 0, sizeof(weakMagDecon));
  for (int irow = 0; irow < _nSamples; irow++) {
    double *decon = _deconvMatrix75 + irow * _nSamples;
    double *mag = weakMag;
    double sum = 0.0;
    for (int icol = 0; icol < _nSamples; icol++, decon++, mag++) {
      sum += *decon * *mag;
    }
    if (sum > 0) {
      weakMagDecon[irow] = sum;
    }
  }
  
  // check for replicas in the weak spectra, if they are there,
  // censor the weak trip
  
  double leakage;
  bool weakHasReplicas = _hasReplicas(weakMagDecon, leakage);
  if (_debugPrint) {
    cerr << "  weakHasReplicas: " << weakHasReplicas << endl;
  }
  
  if (weakHasReplicas) {
    if (trip1IsStrong) {
      power1 = powerStrong;
      vel1 = -1.0 * velStrong;
      width1 = widthStrong;
    } else {
      power2 = powerStrong;
      vel2 = -1.0 * velStrong;
      width2 = widthStrong;
    }
    return;
  }

  // compute deconvoluted weak trip complex spectrum, by scaling the
  // original spectrum with the ratio of the deconvoluted magnitudes
  // with the original magnitudes

  Complex_t weakSpecDecon[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    double ratio = weakMagDecon[ii] / weakMag[ii];
    weakSpecDecon[ii].re = weakSpectra[ii].re * ratio;
    weakSpecDecon[ii].im = weakSpectra[ii].im * ratio;
  }

  // invert back into time domain

  Complex_t weakTdDecon[_nSamples];
  _fft->inv(weakSpecDecon, weakTdDecon);
  
  // compute vel and width for weak trip in time domain
  
  double velWeak = 0.0, widthWeak = 0.0;
  _velWidthFromTd(weakTdDecon, prtSecs, velWeak, widthWeak);
  
  if (_debugPrint) {
    cerr << "========= power correction ============" << endl;
    cerr << "  powerStrong: " << powerStrong << endl;
    cerr << "  powerWeak: " << powerWeak << endl;
    cerr << "  corrPowerRatioDb: " << corrPowerRatioDb << endl;
    cerr << "  corrPowerStrong: " << corrPowerStrong << endl;
    cerr << "  velStrong: " << velStrong << endl;
    cerr << "  velWeak: " << velWeak << endl;
    cerr << "  widthStrong: " << widthStrong << endl;
    cerr << "  widthWeak: " << widthWeak << endl;
  }
  
  // set return vals, changing sign of velocity so that positive
  // is motion away from the radar
  
  if (trip1IsStrong) {
    power1 = corrPowerStrong;
    vel1 = -1.0 * velStrong;
    width1 = widthStrong;
    power2 = powerWeak;
    vel2 = -1.0 * velWeak;
    width2 = widthWeak;
  } else {
    power1 = powerWeak;
    vel1 = -1.0 * velWeak;
    width1 = widthWeak;
    power2 = corrPowerStrong;
    vel2 = -1.0 * velStrong;
    width2 = widthStrong;
  }
  
}

/////////////////////////////////////////////////////////////
// compute moments using SZ 8/64 algorithm and fft
// for RV dealiasing

void Moments::computeBySzFft(const Complex_t *IQ,
			     const Complex_t *delta12,
			     double prtSecs,
			     double &totalPower,
			     double &power1, double &vel1, 
			     double &width1, int &flags1,
			     double &power2, double &vel2,
			     double &width2, int &flags2,
			     double &leak1, double &leak2,
			     int &strongTripFlag,
			     double &powerRatio,
			     GateSpectra &gateSpec) const
  
{

  if (_debugPrint) {
    cerr << "==================================================" << endl;
    cerr << "  Az: " << _debugAz << endl;
    cerr << "  El: " << _debugEl << endl;
    cerr << "  Range: " << _debugRange << endl;
  }

  // initialize return vals

  power1 = vel1 = width1 = _missingDbl;
  power2 = vel2 = width2 = _missingDbl;
  flags1 = flags2 = 0;
  leak1 = leak2 = _missingDbl;
  strongTripFlag = 0;
  gateSpec.setPrtSecs(prtSecs);
 
  // check for 64 samples

  if (_nSamples != 64) {
    cerr << "ERROR - Moments::computeBySzFft" << endl;
    cerr << "  Can only be run with _nSamples = 64" << endl;
    cerr << "  _nSamples: " << _nSamples << endl;
    return;
  }

  // check thresholding on total power

  if (checkSnThreshold(IQ, totalPower)) {
    flags1 = _censorOnTotalPower;
    flags2 = _censorOnTotalPower;
    return;
  }

  // IQ data comes in cohered to trip 1

  const Complex_t *iqTrip1 = IQ;
      
  // cohere to trip 2
  
  Complex_t iqTrip2[_nSamples];
  _cohereTrip1_to_Trip2(iqTrip1, delta12, iqTrip2);
  
  // write FFTs for trip
  
  if (_debugWriteSpectraFiles) {
    Complex_t trip1Spec[_nSamples];
    Complex_t trip2Spec[_nSamples];
    _fft->fwd(iqTrip1, trip1Spec);
    _fft->fwd(iqTrip2, trip2Spec);
    _writeComplex2File("co_trip1", trip1Spec);
    _writeComplex2File("co_trip2", trip2Spec);
  }
  
  // compute R1 for each trip
  
  double r1Trip1 = _computeR1(iqTrip1);
  double r1Trip2 = _computeR1(iqTrip2);

  // determine which trip is strongest by comparing R1 from 
  // each trip
  
  double r1Ratio;
  bool trip1IsStrong;
  const Complex_t *iqStrong;
  
  if (r1Trip1 > r1Trip2) {
    trip1IsStrong = true;
    iqStrong = iqTrip1;
    r1Ratio =  r1Trip1 / r1Trip2;
    strongTripFlag = 1;
  } else {
    trip1IsStrong = false;
    iqStrong = iqTrip2;
    r1Ratio =  r1Trip2 / r1Trip1;
    strongTripFlag = 2;
  }

  if (_debugPrint) {
    cerr << "  R1 trip1: " << r1Trip1 << endl;
    cerr << "  R1 trip2: " << r1Trip2 << endl;
    if (trip1IsStrong) {
      cerr << "  Strong trip is FIRST, ratio:" << r1Ratio << endl;
    } else {
      cerr << "  Strong trip is SECOND, ratio:" << r1Ratio << endl;
    }
  }
  
  // apply window to strong trip as appropriate
  
  Complex_t windowStrong[_nSamples];
  if (_szWindow == WINDOW_HANNING) {
    applyHanningWindow(iqStrong, windowStrong);
  } else if (_szWindow == WINDOW_BLACKMAN) {
    applyBlackmanWindow(iqStrong, windowStrong);
  } else {
    memcpy(windowStrong, iqStrong, sizeof(windowStrong));
  }
  
  // compute FFT for strong trip
  
  Complex_t windowStrongSpec[_nSamples];
  _fft->fwd(windowStrong, windowStrongSpec);
  if (_debugWriteSpectraFiles) {
    _writeComplex2File("strong", windowStrongSpec);
  }
  gateSpec.setSpectrumStrongTrip(windowStrongSpec);
 
  // compute moments for strong trip
  
  double powerStrong = totalPower;
  double velStrong, widthStrong, noise;
  double strongMag[_nSamples];
  _loadMag(windowStrongSpec, strongMag);
  _velWidthFromFft(strongMag, prtSecs, velStrong, widthStrong, noise);

  gateSpec.setPowerStrong(powerStrong);
  gateSpec.setVelStrong(velStrong);
  gateSpec.setTrip1IsStrong(trip1IsStrong);
  
  if (_debugPrint) {
    cerr << "  SZ-8-64 FFT estimates for strong trip:" << endl;
    cerr << "    prtSecs      : " << prtSecs << endl;
    cerr << "    powerStrong  : " << powerStrong << endl;
    cerr << "    velStrong    : " << velStrong << endl;
    cerr << "    widthStrong  : " << widthStrong << endl;
  }

  // apply notch
  
  Complex_t notched[_nSamples];

  int notchStart = _computeNotchStart(_szNotchWidth75,
				      velStrong, prtSecs);
  if (_debugPrint) {
    cerr << "notchStart: " << notchStart << endl;
  }
  _applyNotch75(notchStart, windowStrongSpec, notched);
  if (_debugWriteSpectraFiles) {
    _writeComplex2File("notched", notched);
  }
   
  // compute weak trip power

  double powerWeak = computePower(notched);
  
  // adjust power if required
  
  double corrPowerStrong = powerStrong - powerWeak;
  double corrPowerRatioDb = 10.0 * log10(corrPowerStrong / powerWeak);
  powerRatio = corrPowerRatioDb;
  
  if (corrPowerRatioDb > _szStrongToWeakPowerRatioThreshold) {
    gateSpec.setCensorOnPowerRatio(true);
    // power diff too great - censor
    if (trip1IsStrong) {
      power1 = powerStrong;
      vel1 = -1.0 * velStrong;
      width1 = widthStrong;
      flags2 = _censorOnPowerRatio;
    } else {
      power2 = powerStrong;
      vel2 =  -1.0 * velStrong;
      width2 = widthStrong;
      flags1 = _censorOnPowerRatio;
    }
    return;
  }
  
  // invert the notched spectra into the time domain
  
  Complex_t notchedTd[_nSamples];
  _fft->inv(notched, notchedTd);
  if (_debugWriteSpectraFiles) {
    _writeComplex2File("notchedTd", notchedTd);
  }
 
  // cohere to the weaker trip
  
  Complex_t weakTd[_nSamples];
  if (trip1IsStrong) {
    _cohereTrip1_to_Trip2(notchedTd, delta12, weakTd);
  } else {
    _cohereTrip2_to_Trip1(notchedTd, delta12, weakTd);
  }
  if (_debugWriteSpectraFiles) {
    _writeComplex2File("weakTd", weakTd);
  }
  
  // take fft of cohered weaker trip
  
  Complex_t weakSpectrum[_nSamples];
  _fft->fwd(weakTd, weakSpectrum);
  if (_debugWriteSpectraFiles) {
    _writeComplex2File("weak", weakSpectrum);
  }

  // compute magnitude of cohered weak spectra
  
  double weakMag[_nSamples];
  _loadMag(weakSpectrum, weakMag);
  if (_debugWriteSpectraFiles) {
    _writeMag2File("weak_mag", weakMag);
  }
 
  // deconvolve weak trip, by multiplying with the
  // deconvolution matrix
  
  double weakMagDecon[_nSamples];
  memset(weakMagDecon, 0, sizeof(weakMagDecon));
  for (int irow = 0; irow < _nSamples; irow++) {
    double *decon = _deconvMatrix75 + irow * _nSamples;
    double *mag = weakMag;
    double sum = 0.0;
    for (int icol = 0; icol < _nSamples; icol++, decon++, mag++) {
      sum += *decon * *mag;
    }
    if (sum > 0) {
      weakMagDecon[irow] = sum;
    }
  }
  if (_debugWriteSpectraFiles) {
    _writeMag2File("weak_decon", weakMagDecon);
  }

  // check for replicas in the weak spectra, if they are there,
  // censor the weak trip

  double leakage;
  bool weakHasReplicas = _hasReplicas(weakMagDecon, leakage);
  if (_debugPrint) {
    cerr << "  weakHasReplicas: " << weakHasReplicas << endl;
  }
  
  if (trip1IsStrong) {
    leak1 = 0.01;
    leak2 = leakage;
  } else {
    leak1 = leakage;
    leak2 = 0.01;
  }

  if (weakHasReplicas) {
    if (trip1IsStrong) {
      if (flags2 == 0) {
	flags2 = _censorOnReplicas;
      }
    } else {
      if (flags1 == 0) {
	flags1 = _censorOnReplicas;
      }
    }
  }

  if (weakHasReplicas) {
    if (trip1IsStrong) {
      power1 = powerStrong;
      vel1 = -1.0 * velStrong;
      width1 = widthStrong;
      // flags2 = _censorOnReplicas;
    } else {
      power2 = powerStrong;
      vel2 = -1.0 * velStrong;
      width2 = widthStrong;
      // flags1 = _censorOnReplicas;
    }
    return;
  }
  
  // compute vel and width for weak trip
  
  double velWeak = 0.0, widthWeak = 0.0, noiseWeak;
  _velWidthFromFft(weakMagDecon, prtSecs, velWeak, widthWeak, noiseWeak);
  gateSpec.setMagWeakTrip(weakMagDecon);

  if (_debugPrint) {
    cerr << "========= power correction ============" << endl;
    cerr << "  powerStrong: " << powerStrong << endl;
    cerr << "  powerWeak: " << powerWeak << endl;
    cerr << "  corrPowerRatioDb: " << corrPowerRatioDb << endl;
    cerr << "  corrPowerStrong: " << corrPowerStrong << endl;
    cerr << "  velStrong: " << velStrong << endl;
    cerr << "  velWeak: " << velWeak << endl;
    cerr << "  widthStrong: " << widthStrong << endl;
    cerr << "  widthWeak: " << widthWeak << endl;
  }
  
  // set return vals
  
  if (trip1IsStrong) {
    power1 = corrPowerStrong;
    vel1 = -1.0 * velStrong;
    width1 = widthStrong;
    power2 = powerWeak;
    vel2 = -1.0 * velWeak;
    width2 = widthWeak;
  } else {
    power1 = powerWeak;
    vel1 = -1.0 * velWeak;
    width1 = widthWeak;
    power2 = corrPowerStrong;
    vel2 = -1.0 * velStrong;
    width2 = widthStrong;
  }
  
}

/////////////////////////////////////////////////////////////
// filter clutter for SZ

void Moments::filterClutterSz(const GateSpectra &gateSpec,
			      const Complex_t *delta12,
			      double prtSecs,
			      bool clutterInStrong,
			      bool clutterInWeak,
			      double &powerStrong, double &velStrong, 
			      double &widthStrong, double &clutStrong,
			      double &powerWeak, double &velWeak,
			      double &widthWeak, double &clutWeak) const

{
  
  double nyquist = _wavelengthMeters / (4.0 * prtSecs);

  // filter clutter in strong trip

  if (clutterInStrong) {

    // compute mag from strong trip spectrum

    const Complex_t *strongSpec = gateSpec.getSpectrumStrongTrip();
    double strongMag[_nSamples];
    _loadMag(strongSpec, strongMag);
    
    // apply filter to strong trip
    
    ClutFilter filter;
    double filtered[_nSamples];
    bool clutterFound;
    int clutNotchStart;
    int clutNotchEnd;
    double powerRemoved = 0.0;
    double filteredVel = 0.0;
    double filteredWidth = 0.0;
    
    filter.run(strongMag, _nSamples,
	       _maxClutterVelSz, _initNotchWidthSz, nyquist,
	       filtered, clutterFound, clutNotchStart, clutNotchEnd,
	       powerRemoved, filteredVel, filteredWidth);

//      cerr << "clutNotchStart, clutNotchEnd: "
//  	 << clutNotchStart << ", " << clutNotchEnd << endl;
    
    // compute vel and width
    
    double vv, ww, measuredNoise;
    _velWidthFromFft(filtered, prtSecs, vv, ww, measuredNoise);
    double pwr = computePower(filtered);
    
    // set strong moments

    powerStrong = pwr;
    velStrong = -1.0 * vv;
    widthStrong = ww;
    clutStrong = powerRemoved;

    if (clutterInWeak || gateSpec.getCensorOnPowerRatio() || true) {
      // not enough power in weak trip, return now
      return;
    }

    // weak trip moments

    // apply notch, modified for clutter
    
    //      bool debug = _debugPrint;
    //      _debugPrint = true;
    Complex_t notched[_nSamples];
    int notchStart = _computeNotchStart(_szNotchWidth75,
					velStrong, prtSecs);
    notchStart = _adjustNotchForClutter(clutNotchStart, clutNotchEnd,
					_szNotchWidth75, notchStart);
    _applyNotch75(notchStart, strongSpec, notched);
    // _debugPrint = debug;
    
    // compute weak trip power
    
    double pwrWeak = computePower(notched);
    
    // invert the notched spectra into the time domain
    
    Complex_t notchedTd[_nSamples];
    _fft->inv(notched, notchedTd);
    
    // cohere to the weaker trip
    
    Complex_t weakTd[_nSamples];
    if (gateSpec.getTrip1IsStrong()) {
      _cohereTrip1_to_Trip2(notchedTd, delta12, weakTd);
    } else {
      _cohereTrip2_to_Trip1(notchedTd, delta12, weakTd);
    }
  
    // take fft of cohered weaker trip
  
    Complex_t weakSpectrum[_nSamples];
    _fft->fwd(weakTd, weakSpectrum);
    
    // compute magnitude of cohered weak spectra
    
    double weakMag[_nSamples];
    _loadMag(weakSpectrum, weakMag);
    
    // deconvolve weak trip, by multiplying with the
    // deconvolution matrix
  
    double weakMagDecon[_nSamples];
    memset(weakMagDecon, 0, sizeof(weakMagDecon));
    for (int irow = 0; irow < _nSamples; irow++) {
      double *decon = _deconvMatrix75 + irow * _nSamples;
      double *mag = weakMag;
      double sum = 0.0;
      for (int icol = 0; icol < _nSamples; icol++, decon++, mag++) {
	sum += *decon * *mag;
      }
      if (sum > 0) {
	weakMagDecon[irow] = sum;
      }
    }
    
    // check for replicas in the weak spectra, if they are there,
    // censor the weak trip
    
    double leakage;
    bool weakHasReplicas = _hasReplicas(weakMagDecon, leakage);
    if (weakHasReplicas) {
      return;
    }
    
    // compute vel and width for weak trip
    
    double vvWeak, wwWeak, noiseWeak;
    _velWidthFromFft(weakMagDecon, prtSecs, vvWeak, wwWeak, noiseWeak);
    powerWeak = pwrWeak;
    velWeak = vvWeak;
    widthWeak = wwWeak;

    return;
    
  } else if (clutterInWeak) {
    
    // filter clutter in weak trip, no clutter in strong trip
    
    const double *weakMag = gateSpec.getMagWeakTrip();
    
    // apply filter
    
    ClutFilter filter;
    double filtered[_nSamples];
    bool clutterFound;
    int notchStart;
    int notchEnd;
    double powerRemoved = 0.0;
    double filteredVel = 0.0;
    double filteredWidth = 0.0;
    
    filter.run(weakMag, _nSamples,
	       _maxClutterVelSz, _initNotchWidthSz, nyquist,
	       filtered, clutterFound, notchStart, notchEnd,
	       powerRemoved, filteredVel, filteredWidth);
    
    // compute vel and width
    
    double vv, ww, measuredNoise;
    _velWidthFromFft(filtered, prtSecs, vv, ww, measuredNoise);
    double pwr = computePower(filtered);
    
    powerWeak = pwr;
    velWeak = -1.0 * vv;
    widthWeak = ww;
    clutWeak = powerRemoved;

  }

}
  
///////////////////////////////
// cohere to given trip
//
// beamCode runs from [-4 to 63].
// Therefore trip_num can vary from 1 to 4.

void Moments::cohere2Trip(const Complex_t *IQ,
			  const Complex_t *beamCode,
			  int trip_num,
			  Complex_t *trip) const
  
{

  assert(trip_num > 0);
  assert(trip_num <= 4);

  // to cohere to the given trip, we need to subtract the
  // transmit phase from the received i/q data. This is
  // done by multiplying the i/q value by the complex conjugate
  // of the phase code

  const Complex_t *code = beamCode - trip_num + 1;
  
  for (int ii = 0; ii < _nSamples; ii++, IQ++, trip++, code++) {
    trip->re = (IQ->re * code->re) + (IQ->im * code->im);
    trip->im = (IQ->im * code->re) - (IQ->re * code->im);
  }

}

/////////////////////////////////////////////////////
// initialize the phase codes

void Moments::_initPhaseCodes()
  
{
  
  double ratio = (double) _phaseCodeN / (double) _nSamples;
  double angle = 0.0;
  
  Complex_t switchCode[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    
    double code = (double) ii * (double) ii * ratio;
    double deltaAngle = code * M_PI;
    if (_szNegatePhaseCodes) {
      deltaAngle *= -1.0;
    }
    angle += deltaAngle;

    switchCode[ii].re = cos(angle);
    switchCode[ii].im = sin(angle);
    
  }

  // set the codes for trips 1 and 2
  
  Complex_t *trip1Code = switchCode;
  Complex_t trip2Code[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = (ii - 1 + _nSamples) % _nSamples;
    trip2Code[ii] = trip1Code[jj];
  }
  
  // compute modulation code from trip1 to trip2
  // and vice versa
  
  _subCode(trip1Code, trip2Code, _modCode12);
  _subCode(trip2Code, trip1Code, _modCode21);

}

/////////////////////////////////////////////////////
// initialize the deconvolution matrix

void Moments::_initDeconMatrix(int notchWidth,
			       int powerWidth,
			       double fracPower,
			       double *deconvMatrix)
  
{
  
  // compute the spectra

  Complex_t modSpec12[_nSamples];
  Complex_t modSpec21[_nSamples];
  _fft->fwd(_modCode12, modSpec12);
  _fft->fwd(_modCode21, modSpec21);
  
  // notch the spectra
  
  Complex_t notchedSpec12[_nSamples];
  Complex_t notchedSpec21[_nSamples];
  _applyNotch(0, modSpec12, notchWidth, powerWidth, fracPower, notchedSpec12);
  _applyNotch(0, modSpec21, notchWidth, powerWidth, fracPower, notchedSpec21);
  
  // invert the notched spectra into the time domain
  
  Complex_t notchedCode12[_nSamples];
  Complex_t notchedCode21[_nSamples];
  _fft->inv(notchedSpec12, notchedCode12);
  _fft->inv(notchedSpec21, notchedCode21);
  
  // cohere notchedCode12 to trip 1
  
  Complex_t cohered12[_nSamples];
  _subCode(notchedCode12, _modCode12, cohered12);
  
  // cohere notchedCode21 to trip 2
  
  Complex_t cohered21[_nSamples];
  _subCode(notchedCode21, _modCode21, cohered21);
  
  // compute cohered spectra
  
  Complex_t coheredSpec12[_nSamples];
  Complex_t coheredSpec21[_nSamples];
  _fft->fwd(cohered12, coheredSpec12);
  _fft->fwd(cohered21, coheredSpec21);
  
  // compute normalized magnitude2
  
  double normMag12[_nSamples];
  double normMag21[_nSamples];
  _normalizeMag(coheredSpec12, normMag12);
  _normalizeMag(coheredSpec21, normMag21);
  
  // Load up convolution matrix
  // First create mag array of double length so we can
  // use it as the source for each line without worrying about wrapping
  // Then we load up each row, moving to the right by one element
  // for each row.
  
  double mag2[_nSamples * 2];
  memcpy(mag2, normMag12, sizeof(normMag12));
  memcpy(mag2 + _nSamples, normMag12, sizeof(normMag12));
  
  double convMatrix[_nSamples * _nSamples];
  for (int irow = 0; irow < _nSamples; irow++) {
    memcpy(convMatrix + irow * _nSamples,
	   mag2 + _nSamples - irow, sizeof(normMag12));
  }
  
  // set small values to 0 to improve condition
  
  double *conv = convMatrix;
  for (int i=0; i < _nSamples * _nSamples; i++, conv++) {
    if (fabs(*conv) < _smallValue) {
      *conv = 0;
    }
  }

#ifdef NOTNOW  
  conv = convMatrix;
  cerr << "Convolution matrix" << endl;
  for (int irow = 0; irow < _nSamples; irow++) {
    cerr << "---> row: " << irow;
    for (int icol = 0; icol < _nSamples; icol++, conv++) {
      fprintf(stderr, " %g", *conv);
    }
    cerr << endl;
  }
#endif
  
  // invert the matrix
  
  memcpy(deconvMatrix, convMatrix, sizeof(convMatrix));
  _invertMatrix(deconvMatrix, _nSamples);
  
  // set small values to 0 to improve condition
  
  double *deconv = deconvMatrix;
  for (int i=0; i < _nSamples * _nSamples; i++, deconv++) {
    if (fabs(*deconv) < _smallValue) {
      *deconv = 0;
    }
  }

#ifdef NOTNOW

  deconv = deconvMatrix;
  cerr << "Deconvolution matrix" << endl;
  for (int irow = 0; irow < _nSamples; irow++) {
    cerr << "---> row: " << irow;
    for (int icol = 0; icol < _nSamples; icol++, deconv++) {
      fprintf(stderr, " %g", *deconv);
    }
    cerr << endl;
  }
  
  // check that we get the identity matrix when we multiply
  
  double identityMatrix[_nSamples * _nSamples];
  double *ident = identityMatrix;
  for (int irow = 0; irow < _nSamples; irow++) {
    for (int icol = 0; icol < _nSamples; icol++, ident++) {
      double sum = 0.0;
      for (int ii = 0; ii < _nSamples; ii++) {
	sum += (convMatrix[irow * _nSamples + ii] *
		deconvMatrix[ii * _nSamples + icol]);
      }
      *ident = sum;
    }
  }
  
  double *ident1 = identityMatrix;
  cerr << "Identity matrix" << endl;
  for (int irow = 0; irow < _nSamples; irow++) {
    cerr << "---> row: " << irow;
    for (int icol = 0; icol < _nSamples; icol++, ident1++) {
      if (fabs(*ident1) > 0.005) {
	cerr << "    posn, size: " << icol << ", "
	     << setprecision(4) << setw(10) << *ident1 << endl;
      }
    }
  }

#endif
  
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

#ifdef JUNK

  // get fft of the window

  Complex_t hann[_nSamples];
  Complex_t spec[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = (ii + _nSamples / 2) % _nSamples;
    hann[ii].re = window[jj];
    hann[ii].im = 0.0;
  }
  
  _fft->fwd(hann, spec);
  
  _printVector(cout, "hanning time domain", hann);
  _printVector(cout, "hanning spectrum", spec);

#endif
  
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
// compute R1

double Moments::_computeR1(const Complex_t *IQ) const

{

  if (_nSamples < 1) {
    return 0.0;
  }

  double a = 0.0, b = 0.0;
  
  const Complex_t *iq0 = IQ;
  const Complex_t *iq1 = IQ + 1;

  for (int i = 0; i < _nSamples - 1; i++, iq0++, iq1++) {
    a += ((iq0->re * iq1->re) + (iq0->im * iq1->im));
    b += ((iq0->re * iq1->im) - (iq1->re * iq0->im));
  }

  return (sqrt(a * a + b * b) / _nSamples);

}
  
///////////////////////////////////////
// change coherence from trip1 to trip2
//
// Subtract delta21, which is conjugate of delta12

void Moments::_cohereTrip1_to_Trip2(const Complex_t *trip1,
				    const Complex_t *delta12,
				    Complex_t *trip2) const
  
{
  
  const Complex_t *t1 = trip1;
  Complex_t *t2 = trip2;
  const Complex_t *dd = delta12;
  for (int ii = 0; ii < _nSamples; ii++, t1++, t2++, dd++) {
    t2->re = t1->re * dd->re + t1->im * (dd->im * -1.0);
    t2->im = t1->im * dd->re - t1->re * (dd->im * -1.0);
  }

}

///////////////////////////////////////
// change coherence from trip2 to trip1
//
// Subtract delta 12

void Moments::_cohereTrip2_to_Trip1(const Complex_t *trip2,
				    const Complex_t *delta12,
				    Complex_t *trip1) const
  
{
  
  Complex_t *t1 = trip1;
  const Complex_t *t2 = trip2;
  const Complex_t *dd = delta12;
  for (int ii = 0; ii < _nSamples; ii++, t1++, t2++, dd++) {
    t1->re = t2->re * dd->re + t2->im * dd->im;
    t1->im = t2->im * dd->re - t2->re * dd->im;
  }

}

///////////////////////////////
// add code in time domain

void Moments::_addCode(const Complex_t *in, const Complex_t *code,
		       Complex_t *sum) const

{
  
  for (int ii = 0; ii < _nSamples; ii++, in++, code++, sum++) {
    sum->re = (in->re * code->re - in->im * code->im);
    sum->im = (in->re * code->im + in->im * code->re);
  }
  
}

void Moments::_addCode(const Complex_t *in, const Complex_t *code, int trip,
		       Complex_t *sum) const

{

  const Complex_t *_codeUpper = code + _nSamples - 1;
  const Complex_t *_code = code + ((-(trip - 1) + _nSamples) % _nSamples);
  
  for (int ii = 0; ii < _nSamples; ii++, in++, _code++, sum++) {
    if (_code > _codeUpper) {
      _code -= _nSamples;
    }
    sum->re = (in->re * _code->re - in->im * _code->im);
    sum->im = (in->re * _code->im + in->im * _code->re);
  }

}

///////////////////////////////
// subtract code in time domain

void Moments::_subCode(const Complex_t *in, const Complex_t *code,
		       Complex_t *diff) const
  
{
  
  for (int ii = 0; ii < _nSamples; ii++, in++, code++, diff++) {
    diff->re = (in->re * code->re + in->im * code->im);
    diff->im = (in->im * code->re - in->re * code->im);
  }
  
}

void Moments::_subCode(const Complex_t *in, const Complex_t *code, int trip,
		       Complex_t *diff) const
  
{
  
  const Complex_t *codeUpperP = code + _nSamples - 1;
  const Complex_t *codeP = code + ((-(trip - 1) + _nSamples) % _nSamples);
  
  for (int ii = 0; ii < _nSamples; ii++, in++, codeP++, diff++) {
    if (codeP > codeUpperP) {
      codeP -= _nSamples;
    }
    diff->re = (in->re * codeP->re + in->im * codeP->im);
    diff->im = (in->im * codeP->re - in->re * codeP->im);
  }

}

///////////////////////////////
// create conjugate
//

void Moments::_conjugate(const Complex_t *in, Complex_t *conj) const

{
  
  memcpy(conj, in, _nSamples * sizeof(Complex_t));
  for (int ii = 0; ii < _nSamples; ii++, conj++) {
    conj->im *= -1.0;
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

///////////////////////////////
// normalize magnitudes
//

void Moments::_normalizeMag(const Complex_t *in, double *norm_mag)

{
  
  double sum = 0.0;
  double *norm = norm_mag;
  for (int ii = 0; ii < _nSamples; ii++, in++, norm++) {
    *norm = sqrt(in->re * in->re + in->im * in->im);
    sum += *norm;
  }
  norm = norm_mag;
  for (int ii = 0; ii < _nSamples; ii++, norm++) {
    (*norm) /= sum;
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

/////////////////////////////////////////////////
// compute notch start position around
// the velocity moment
//
// returns the index for the start of the notch

int Moments::_computeNotchStart(int notchWidth,
				double vel,
				double prtSecs) const
  
{
  
  // find the first moment index
  
  double nyquist = _wavelengthMeters / (4.0 * prtSecs);

  // compute the location of the spectral peack based on the
  // velocity
  
  double integ;
  double dPeak = modf(vel / (2.0 * nyquist) + 1.0, &integ);
  dPeak *= _nSamples;

  // if half width is even, add 0.5 to peak position to
  // evenly space the notch around the vel location
  
  int halfWidth = notchWidth / 2;
  int peakIndex;
  if (notchWidth == halfWidth * 2) {
    peakIndex = (int) (dPeak + 0.5);
  } else {
    peakIndex = (int) dPeak;
  }
  
  // compute the start position;
  
  int startIndex = peakIndex - halfWidth;

  if (_debugPrint) {
    cerr << "======= notch start pos =======" << endl;
    cerr << "vel: " << vel << endl;
    cerr << "nyquist: " << nyquist << endl;
    // cerr << "iVel: " << iVel << endl;
    cerr << "dPeak: " << dPeak << endl;
    cerr << "halfWidth: " << halfWidth << endl;
    cerr << "notchWidth: " << notchWidth << endl;
    cerr << "peakIndex: " << peakIndex << endl;
    cerr << "startIndex: " << startIndex << endl;
  }

  return startIndex;
  
}

/////////////////////////////////////////////////
// adjust notch for presence of clutter
//

int Moments::_adjustNotchForClutter(int clutNotchStart, int clutNotchEnd,
				    int notchWidth, int notchStart) const

{

  if (clutNotchEnd < clutNotchStart) {
    clutNotchEnd += _nSamples;
  }

  int adjustedNotchStart = notchStart;
  if (adjustedNotchStart < 0) {
    adjustedNotchStart += _nSamples;
  }

  if (notchStart > clutNotchStart) {
    adjustedNotchStart = clutNotchStart;
  }

  int notchEnd = notchStart + notchWidth - 1;
  int adjustedNotchEnd = notchEnd;

  if (notchEnd < clutNotchEnd) {
    int diff = clutNotchEnd - notchEnd;
    adjustedNotchStart += diff;
    adjustedNotchEnd += diff;
  }
  
  if (adjustedNotchEnd > _nSamples) {
    adjustedNotchStart -= _nSamples;
    adjustedNotchEnd -= _nSamples;
  }

  if (_debugPrint) {
    cerr << "======== Adjusting notch for clutter =========================" << endl;
    cerr << "clutNotchStart, clutNotchEnd: "
	 << clutNotchStart << ", " << clutNotchEnd << endl;
    cerr << "notchStart, notchEnd: " << notchStart << ", " << notchEnd << endl;
    if (notchStart != adjustedNotchStart) {
      cerr << "--->> adj notchStart, notchEnd: "
	   << adjustedNotchStart << ", " << adjustedNotchEnd << endl;
    }
  }
    
  return adjustedNotchStart;

}

/////////////////////////////////////////////////
// apply notch given the start point

void Moments::_applyNotch(int startIndex,
			  const Complex_t *in,
			  int notchWidth,
			  int powerWidth,
			  double fracPower,
			  Complex_t *notched) const
  
{
  
  // compute the end index for the notch
  
  int iStart = startIndex;
  int iEnd = iStart + notchWidth - 1;
  if (iStart < 0 && iEnd < 0) {
    iStart += _nSamples;
    iEnd += _nSamples;
  }

  if (_debugPrint) {
    cerr << "======= applying notch =======" << endl;
    cerr << "startIndex: " << startIndex << endl;
    cerr << "iStart: " << iStart << endl;
    cerr << "iEnd: " << iEnd << endl;
  }

  // apply the notch, taking into account the folding of the
  // notch around the ends of the array

  if (iStart >= 0 && iEnd < _nSamples) {

    // notch does not wrap, copy array then zero out the notch
    
    memcpy(notched, in, _nSamples * sizeof(Complex_t));
    memset(notched + iStart, 0, notchWidth * sizeof(Complex_t));
    
    if (_debugPrint) {
      cerr << "--> notch does not wrap, do memcpy" << endl;
    }

  } else {

    // notch wraps, non-notch region is centered
    // zero out array, copy non-notch region

    int copyStart;
    if (iStart < 0) {
      // notch wraps at lower end
      copyStart = iEnd + 1;
    } else {
      // notch wraps at upper end
      // copyStart = _nSamples - (iEnd + 1);
      copyStart = iEnd - _nSamples + 1;
    }
    
    memset(notched, 0, _nSamples * sizeof(Complex_t));
    memcpy(notched + copyStart, in + copyStart,
	   powerWidth * sizeof(Complex_t));

    if (_debugPrint) {
      cerr << "--> notch does wrap" << endl;
      cerr << "  copyStart: " << copyStart << endl;
    }

  }
  
  // modify power so that power in part left is equal to original power
  
  Complex_t *nn = notched;
  double mult = 1.0 / sqrt(fracPower);
  for (int ii = 0; ii < _nSamples; ii++, nn++) {
    nn->re *= mult;
    nn->im *= mult;
  }

}

void Moments::_applyNotch75(int startIndex,
			    const Complex_t *in,
			    Complex_t *notched) const

{
  _applyNotch(startIndex, in,
	      _szNotchWidth75, _szPowerWidth75, _szFracPower75,
	      notched);
}

void Moments::_applyNotch50(int startIndex,
			    const Complex_t *in,
			    Complex_t *notched) const

{
  _applyNotch(startIndex, in,
	      _szNotchWidth50, _szPowerWidth50, _szFracPower50,
	      notched);
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

  double powerCentered[_nSamples];
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

}

//////////////////////////////////////////////////////
// check if spectra has replicas

bool Moments::_hasReplicas(double *magnitude,
			   double &leakage) const
  
{
  
  // find max magnitude
  
  double maxMag = 0.0;
  int kMax = 0;
  double *mp = magnitude;
  for (int ii = 0; ii < _nSamples; ii++, mp++) {
    if (*mp > maxMag) {
      kMax = ii;
      maxMag = *mp;
    }
  }

  // set peak 0 to the max mag

  double peaks[8];
  peaks[0] = maxMag;

  // find each of 7 other peaks
  
  int kStart = kMax + _nSamples / 16;
  int binWidth = _nSamples / 8;
  double mag2[_nSamples * 2];
  memcpy(mag2, magnitude, _nSamples * sizeof(double));
  memcpy(mag2 + _nSamples, magnitude, _nSamples * sizeof(double));

  for (int i = 1; i < 8; i++, kStart += binWidth) {
    double maxInBin = 0.0;
    double *mp2 = mag2 + kStart;
    for (int k = 0; k < binWidth; k++, mp2++) {
      if (*mp2 > maxInBin) {
	maxInBin = *mp2;
      }
    } // k
    peaks[i] = maxInBin;
  } // i

  // sort the peaks

  qsort(peaks, 8, sizeof(double), _compareDoubles);

  // count the number of peaks for which the replica power ratio
  // exceeds the threshold

  double ratio = pow(10.0, _szOutOfTripPowerRatioThreshold / 10.0);
  double testVal = peaks[7] / ratio;
  int count = 0;
  for (int i = 0; i < 6; i++) {
    if (peaks[i] > testVal) {
      count++;
    }
  }
  double sum = 0.0;
  for (int i = 0; i < _szOutOfTripPowerNReplicas; i++) {
    sum += peaks[i];
  }
  leakage = (sum / _szOutOfTripPowerNReplicas) / peaks[7];
  
  if (_debugPrint) {
    cerr << "  Peaks:";
    for (int i = 0; i < 8; i++) {
      cerr << " " << peaks[i];
    }
    cerr << endl;
    cerr << "    ratio: " << ratio << endl;
    cerr << "    testVal: " << testVal << endl;
    cerr << "    peak count: " << count << endl;
    cerr << "    leakage: " << leakage << endl;
  }

  if (leakage > 0.17) {
    return true;
  } else {
    return false;
  }

  if (count >= _szOutOfTripPowerNReplicas) {
    return true;
  } else {
    return false;
  }

}

///////////////////////////////////////////////////
// invert square matrix

void Moments::_invertMatrix(double *data, int nn) const
{

  if (data[0] != 0.0) {
    for (int i=1; i < nn; i++) {
      data[i] /= data[0]; // normalize row 0
    }
  }

  for (int i=1; i < nn; i++)  { 

    for (int j=i; j < nn; j++)  { // do a column of L
      double sum = 0.0;
      for (int k = 0; k < i; k++) {
	sum += data[j*nn+k] * data[k*nn+i];
      }
      data[j*nn+i] -= sum;
    } // j

    if (i == nn-1) continue;

    for (int j=i+1; j < nn; j++)  {  // do a row of U
      double sum = 0.0;
      for (int k = 0; k < i; k++) {
	sum += data[i*nn+k]*data[k*nn+j];
      }
      data[i*nn+j] = (data[i*nn+j]-sum) / data[i*nn+i];
    }

  } // i

  // invert L
  
  for ( int i = 0; i < nn; i++ ) {
    
    for ( int j = i; j < nn; j++ )  {
      double x = 1.0;
      if ( i != j ) {
	x = 0.0;
	for ( int k = i; k < j; k++ ) {
	  x -= data[j*nn+k]*data[k*nn+i];
	}
      }
      data[j*nn+i] = x / data[j*nn+j];
    } // j

  }

  // invert U

  for ( int i = 0; i < nn; i++ ) {
    for ( int j = i; j < nn; j++ )  {
      if ( i == j ) continue;
      double sum = 0.0;
      for ( int k = i; k < j; k++ ) {
	sum += data[k*nn+j]*( (i==k) ? 1.0 : data[i*nn+k] );
      }
      data[i*nn+j] = -sum;
    }
  }
  
  // final inversion
  for ( int i = 0; i < nn; i++ ) {
    for ( int j = 0; j < nn; j++ )  {
      double sum = 0.0;
      for ( int k = ((i>j)?i:j); k < nn; k++ ) {
	sum += ((j==k)?1.0:data[j*nn+k])*data[k*nn+i];
      }
      data[j*nn+i] = sum;
    }
  }

}

///////////////////////////////
// print complex compoments

void Moments::printComplex(ostream &out,
                           const string &heading,
                           const Complex_t *comp,
                           bool reCenter /* = false */ ) const
  
{
  
  out << "---->> " << heading << " <<----" << endl;
  out << setw(3) << "ii" << "  "
      << setw(10) << "re" << "  "
      << setw(10) << "im" << endl;
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = ii;
    if (reCenter) {
      jj += _nSamples / 2;
      jj %= _nSamples;
    }
    out << setw(3) << jj << "  "
	<< setw(10) << comp[jj].re << "  "
	<< setw(10) << comp[jj].im << endl;
  }
  out.flush();

}

///////////////////////////////
// print complex vector

void Moments::printVector(ostream &out,
                          const string &heading,
                          const Complex_t *comp,
                          bool reCenter /* = false */ ) const
  
{
  
  out << "---->> " << heading << " <<----" << endl;
  out << setw(3) << "ii" << "  "
      << setw(10) << "magnitude" << "  "
      << setw(10) << "angle" << endl;
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = ii;
    if (reCenter) {
      jj += _nSamples / 2;
      jj %= _nSamples;
    }
    double mag = sqrt(comp[jj].re * comp[jj].re + comp[jj].im * comp[jj].im);
    double angle = atan2(comp[jj].im, comp[jj].re) * RAD_TO_DEG;
    out << setw(3) << ii << "  "
	<< setw(10) << mag << "  "
	<< setw(10) << angle << endl;
  }
  out.flush();

}

///////////////////////////////////
// write spectra file from complex

void Moments::_writeComplex2File(const string &heading,
				 const Complex_t *comp,
                                 bool reCenter /* = false */ ) const
  
{
  
  char outPath[MAX_PATH_LEN];
  sprintf(outPath, "%s%s%s_%05.1f_%05.1f_%07.3f.mag",
	  _debugSpectraDir.c_str(), PATH_DELIM,
	  heading.c_str(), _debugEl, _debugAz, _debugRange);
  
  ta_makedir_recurse(_debugSpectraDir.c_str());

  ofstream out(outPath);
  if (!out.is_open()) {
    cerr << "ERROR opening file: " << outPath << endl;
    return;
  }
  
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = ii;
    if (reCenter) {
      jj += _nSamples / 2;
      jj %= _nSamples;
    }
    double mag = sqrt(comp[jj].re * comp[jj].re + comp[jj].im * comp[jj].im);
    double angle = atan2(comp[jj].im, comp[jj].re) * RAD_TO_DEG;
    out << setw(3) << ii << "  "
	<< setw(10) << mag << "  "
	<< setw(10) << angle << endl;
  }

  out.close();

}

///////////////////////////////
// write spectra file from mag

void Moments::_writeMag2File(const string &heading,
			     const double *mag,
                             bool reCenter /* = false */ ) const
  
{
  
  char outPath[MAX_PATH_LEN];
  sprintf(outPath, "%s%s%s_%05.1f_%05.1f_%07.3f.mag",
	  _debugSpectraDir.c_str(), PATH_DELIM,
	  heading.c_str(), _debugEl, _debugAz, _debugRange);
  
  ta_makedir_recurse(_debugSpectraDir.c_str());
  
  ofstream out(outPath);
  if (!out.is_open()) {
    cerr << "ERROR opening file: " << outPath << endl;
    return;
  }
  
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = ii;
    if (reCenter) {
      jj += _nSamples / 2;
      jj %= _nSamples;
    }
    out << setw(3) << ii << "  "
	<< setw(10) << mag[jj] << endl;
  }
  
  out.close();

}

/*****************************************************************************
 * define function to be used for sorting (lowest to highest)
 */

int Moments::_compareDoubles(const void *v1, const void *v2)

{
  double *d1 = (double *) v1;
  double *d2 = (double *) v2;
  if (*d1 > *d2) {
    return 1;
  } else {
      return -1;
    }
}

