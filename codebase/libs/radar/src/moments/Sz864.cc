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
// Sz864.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2006
//
///////////////////////////////////////////////////////////////
//
// Sz864 handles computation of the radar moments for SZ864
//
////////////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <radar/Sz864.hh>
using namespace std;

// initialize const doubles

const double Sz864::_missingDbl = -9999.0;
const double Sz864::_smallValue = 1.0e-9;
const double Sz864::_szFracPower75 = 0.25;
const double Sz864::_szFracPower50 = 0.5;
const double Sz864::_maxClutterVel = 1.0;
const double Sz864::_initNotchWidth = 1.5;
const double Sz864::_maxClutterVelSz = 2.0;
const double Sz864::_initNotchWidthSz = 2.5;

// Constructor

Sz864::Sz864(bool debug) :
        _debug(debug)
  
{

  _nSamples = 64;

  _wavelengthMeters = 0.1068;
  setNoiseValueDbm(-113.0);
  
  _szNegatePhaseCodes = false;
  _szWindow = WINDOW_VONHANN;

  _signalToNoiseRatioThreshold = 3.0;
  _szStrongToWeakPowerRatioThreshold = 45.0;
  _szOutOfTripPowerRatioThreshold = 6.0;
  _szOutOfTripPowerNReplicas = 3;

  _modCode12 = NULL;
  _modCode21 = NULL;
  _vonhann = NULL;
  _blackman = NULL;
  _deconvMatrix75 = NULL;
  _deconvMatrix50 = NULL;

  // alloc arrays and matrices

  _modCode12 = new RadarComplex_t[_nSamples];
  _modCode21 = new RadarComplex_t[_nSamples];
  _vonhann = new double[_nSamples];
  _blackman = new double[_nSamples];
  _deconvMatrix75 = new double[_nSamples * _nSamples];
  _deconvMatrix50 = new double[_nSamples * _nSamples];

  // init phase codes

  if (_nSamples >= 64) {

    _initPhaseCodes();
    
    // initialize deconvolution matrix for 3/4 notch
    
    _initDeconMatrix(_szNotchWidth75, _szPowerWidth75,
                     _szFracPower75, _deconvMatrix75);
    
    // initialize deconvolution matrix for 1/2 notch
    
    _initDeconMatrix(_szNotchWidth50, _szPowerWidth50,
                     _szFracPower50, _deconvMatrix50);

  }
    
  // init windowing
  
  _initVonhann(_vonhann);
  _initBlackman(_blackman);

  return;

}

// destructor

Sz864::~Sz864()

{

  if (_modCode12) {
    delete[] _modCode12;
  }
  if (_modCode21) {
    delete[] _modCode21;
  }
  if (_vonhann) {
    delete[] _vonhann;
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

}

/////////////////////////////////////
// set methods

void Sz864::setWavelength(double wavelength) {
  _wavelengthMeters = wavelength;
}

void Sz864::setNoiseValueDbm(double dbm) {
  _noiseValueDbm = dbm;
  _noiseValueMwatts = pow(10.0, dbm / 10.0);
}

void Sz864::setSzNegatePhaseCodes(bool status /* = true */) {
  _szNegatePhaseCodes = status;
}

void Sz864::setSzWindow(window_t window) {
  _szWindow = window;
}

void Sz864::setSignalToNoiseRatioThreshold(double db) {
  _signalToNoiseRatioThreshold = db;
}

void Sz864::setSzStrongToWeakPowerRatioThreshold(double db) {
  _szStrongToWeakPowerRatioThreshold = db;
}

void Sz864::setSzOutOfTripPowerRatioThreshold(double db) {
  _szOutOfTripPowerRatioThreshold = db;
}

void Sz864::setSzOutOfTripPowerNReplicas(int n) {
  _szOutOfTripPowerNReplicas = n;
}

/////////////////////////////////////////////////////////////
// compute moments using SZ 8/64 algorithm and pulse-pair
// for RV dealiasing

void Sz864::separateTrips(GateData &gateData,
                          const RadarComplex_t *delta12,
                          double prtSecs,
                          const RadarFft &fft)

{

  // initialize

  gateData.trip1IsStrong = true;
  gateData.censorStrong = false;
  gateData.censorWeak = false;

  // check for 64 samples
  
  if (_nSamples != 64) {
    cerr << "ERROR - Sz864::separateTrips" << endl;
    cerr << "  Can only be run with _nSamples = 64" << endl;
    cerr << "  _nSamples: " << _nSamples << endl;
    gateData.censorStrong = true;
    gateData.censorWeak = true;
    return;
  }

  // check thresholding on total power

  double totalPower;
  if (checkSnThreshold(gateData.iqhc, totalPower)) {
    gateData.censorStrong = true;
    gateData.censorWeak = true;
    return;
  }
  
  // IQ data comes in cohered to trip 1
  
  const RadarComplex_t *iqTrip1 = gateData.iqhc;
  
  // cohere to trip 2
  
  TaArray<RadarComplex_t> iqTrip2_;
  RadarComplex_t *iqTrip2 = iqTrip2_.alloc(_nSamples);
  _cohereTrip1_to_Trip2(iqTrip1, delta12, iqTrip2);
  
  // compute R1 for each trip
  
  double r1Trip1 = _computeR1(iqTrip1);
  double r1Trip2 = _computeR1(iqTrip2);

  // determine which trip is strongest by comparing R1 from 
  // each trip
  
  double r1Ratio;
  const RadarComplex_t *iqStrong;
  
  if (r1Trip1 > r1Trip2) {
    iqStrong = iqTrip1;
    r1Ratio = r1Trip1 / r1Trip2;
    gateData.trip1IsStrong = true;
    memcpy(gateData.iqStrong, iqTrip1, _nSamples * sizeof(RadarComplex_t));
  } else {
    iqStrong = iqTrip2;
    r1Ratio = r1Trip2 / r1Trip1;
    gateData.trip1IsStrong = false;
    memcpy(gateData.iqStrong, iqTrip2, _nSamples * sizeof(RadarComplex_t));
  }

  // compute moments for strong trip

  double powerStrong = totalPower;
  double velStrong, widthStrong;
  _velWidthFromTd(iqStrong, prtSecs, velStrong, widthStrong);
  
  if (_debug) {
    cerr << "  R1 trip1: " << r1Trip1 << endl;
    cerr << "  R1 trip2: " << r1Trip2 << endl;
    if (gateData.trip1IsStrong) {
      cerr << "  Strong trip is FIRST, ratio:" << r1Ratio << endl;
    } else {
      cerr << "  Strong trip is SECOND, ratio:" << r1Ratio << endl;
    }
  }
  
  // compute FFT for strong trip
  // IQ is already windowed
  
  TaArray<RadarComplex_t> strongSpec_;
  RadarComplex_t *strongSpec = strongSpec_.alloc(_nSamples);
  fft.fwd(iqStrong, strongSpec);

  // apply notch
  
  TaArray<RadarComplex_t> notched_;
  RadarComplex_t *notched = notched_.alloc(_nSamples);
  int notchStart =
    _computeNotchStart(_szNotchWidth75, velStrong, prtSecs);
  _applyNotch75(notchStart, strongSpec, notched);
  
  // compute weak trip power
  
  double powerWeak = computePower(notched);
  
  // adjust power
  
  double corrPowerStrong = powerStrong - powerWeak;
  adjustPower(gateData.iqStrong, corrPowerStrong);
  
  // invert the notched spectra into the time domain
  
  TaArray<RadarComplex_t> notchedTd_;
  RadarComplex_t *notchedTd = notchedTd_.alloc(_nSamples);
  fft.inv(notched, notchedTd);
  
  // cohere to the weaker trip
  
  TaArray<RadarComplex_t> weakTd_;
  RadarComplex_t *weakTd = weakTd_.alloc(_nSamples);
  if (gateData.trip1IsStrong) {
    _cohereTrip1_to_Trip2(notchedTd, delta12, weakTd);
  } else {
    _cohereTrip2_to_Trip1(notchedTd, delta12, weakTd);
  }

  // take fft of cohered weaker trip
  
  TaArray<RadarComplex_t> weakSpectra_;
  RadarComplex_t *weakSpectra = weakSpectra_.alloc(_nSamples);
  fft.fwd(weakTd, weakSpectra);
  
  // compute magnitude of cohered weak spectrum
  
  TaArray<double> weakMag_;
  double *weakMag = weakMag_.alloc(_nSamples);
  _loadMag(weakSpectra, weakMag);
  
  // deconvolve weak trip, by multiplying with the
  // deconvoltion matrix
  
  TaArray<double> weakMagDecon_;
  double *weakMagDecon = weakMagDecon_.alloc(_nSamples);
  memset(weakMagDecon, 0, _nSamples * sizeof(double));
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
  if (_debug) {
    cerr << "  weakHasReplicas: " << weakHasReplicas << endl;
  }
  gateData.szLeakage = leakage;
  if (weakHasReplicas) {
    gateData.censorWeak = true;
    return;
  }

  // compute deconvoluted weak trip complex spectrum, by scaling the
  // original spectrum with the ratio of the deconvoluted magnitudes
  // with the original magnitudes
  
  TaArray<RadarComplex_t> weakSpecDecon_;
  RadarComplex_t *weakSpecDecon = weakSpecDecon_.alloc(_nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    double ratio = weakMagDecon[ii] / weakMag[ii];
    weakSpecDecon[ii].re = weakSpectra[ii].re * ratio;
    weakSpecDecon[ii].im = weakSpectra[ii].im * ratio;
  }

  // invert back into time domain
  
  TaArray<RadarComplex_t> weakTdDecon_;
  RadarComplex_t *weakTdDecon = weakTdDecon_.alloc(_nSamples);
  fft.inv(weakSpecDecon, weakTdDecon);

  // adjust weak time series for power

  memcpy(gateData.iqWeak, weakTdDecon, _nSamples * sizeof(RadarComplex_t));
  adjustPower(gateData.iqWeak, powerWeak);
  
  return;
  
}

///////////////////////////////////////////////
// compute total power from IQ

double Sz864::computePower(const RadarComplex_t *IQ) const
  
{
  double p = 0.0;
  for (int i = 0; i < _nSamples; i++, IQ++) {
    p += ((IQ->re * IQ->re) + (IQ->im * IQ->im));
  }
  return (p / _nSamples);
}
  
///////////////////////////////////////////////
// compute total power from magnitudes

double Sz864::computePower(const double *mag) const
  
{
  double p = 0.0;
  for (int i = 0; i < _nSamples; i++, mag++) {
    p += (*mag * *mag);
  }
  return (p / _nSamples);
}
  
///////////////////////////////////////////////
// adjust the power in IQ data to a given value

void Sz864::adjustPower(RadarComplex_t *IQ,
                        double adjustedPower) const
  
{
  double power = computePower(IQ);
  double ratio = sqrt(adjustedPower / power);
  for (int i = 0; i < _nSamples; i++, IQ++) {
    IQ->re *= ratio;
    IQ->im *= ratio;
  }
}
  
///////////////////////////////////////////////
// check if we should threshold based on
// signal-to-noise of total power
//
// Side-effect: passes back total power

bool Sz864::checkSnThreshold(const RadarComplex_t *IQ,
                             double &totalPower) const
  
{
  double power = computePower(IQ);
  double dbm = 10.0 * log10(power);
  if (dbm < _noiseValueDbm + _signalToNoiseRatioThreshold) {
    return true;
  }
  totalPower = power;
  return false;
}
  
///////////////////////////////
// cohere to given trip
//
// beamCode runs from [-4 to 63].
// Therefore trip_num can vary from 1 to 4.

void Sz864::cohere2Trip(const RadarComplex_t *IQ,
                        const RadarComplex_t *beamCode,
                        int trip_num,
                        RadarComplex_t *trip) const
  
{

  assert(trip_num > 0);
  assert(trip_num <= 4);

  // to cohere to the given trip, we need to subtract the
  // transmit phase from the received i/q data. This is
  // done by multiplying the i/q value by the complex conjugate
  // of the phase code

  const RadarComplex_t *code = beamCode - trip_num + 1;
  
  for (int ii = 0; ii < _nSamples; ii++, IQ++, trip++, code++) {
    trip->re = (IQ->re * code->re) + (IQ->im * code->im);
    trip->im = (IQ->im * code->re) - (IQ->re * code->im);
  }

}

/////////////////////////////////////////////////////
// initialize the phase codes

void Sz864::_initPhaseCodes()
  
{
  
  double ratio = (double) _phaseCodeN / (double) _nSamples;
  double angle = 0.0;
  
  TaArray<RadarComplex_t> switchCode_;
  RadarComplex_t *switchCode = switchCode_.alloc(_nSamples);
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
  
  RadarComplex_t *trip1Code = switchCode;
  TaArray<RadarComplex_t> trip2Code_;
  RadarComplex_t *trip2Code = trip2Code_.alloc(_nSamples);
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

void Sz864::_initDeconMatrix(int notchWidth,
                             int powerWidth,
                             double fracPower,
                             double *deconvMatrix)
  
{
  
  // set up FFT plan
  
  RadarFft fft(_nSamples);
  
  // compute the spectra

  TaArray<RadarComplex_t> modSpec12_;
  RadarComplex_t *modSpec12 = modSpec12_.alloc(_nSamples);
  TaArray<RadarComplex_t> modSpec21_;
  RadarComplex_t *modSpec21 = modSpec21_.alloc(_nSamples);
  fft.fwd(_modCode12, modSpec12);
  fft.fwd(_modCode21, modSpec21);
  
  // notch the spectra
  
  TaArray<RadarComplex_t> notchedSpec12_;
  RadarComplex_t *notchedSpec12 = notchedSpec12_.alloc(_nSamples);
  TaArray<RadarComplex_t> notchedSpec21_;
  RadarComplex_t *notchedSpec21 = notchedSpec21_.alloc(_nSamples);
  _applyNotch(0, modSpec12, notchWidth, powerWidth, fracPower, notchedSpec12);
  _applyNotch(0, modSpec21, notchWidth, powerWidth, fracPower, notchedSpec21);
  
  // invert the notched spectra into the time domain
  
  TaArray<RadarComplex_t> notchedCode12_;
  RadarComplex_t *notchedCode12 = notchedCode12_.alloc(_nSamples);
  TaArray<RadarComplex_t> notchedCode21_;
  RadarComplex_t *notchedCode21 = notchedCode21_.alloc(_nSamples);
  fft.inv(notchedSpec12, notchedCode12);
  fft.inv(notchedSpec21, notchedCode21);
  
  // cohere notchedCode12 to trip 1
  
  TaArray<RadarComplex_t> cohered12_;
  RadarComplex_t *cohered12 = cohered12_.alloc(_nSamples);
  _subCode(notchedCode12, _modCode12, cohered12);
  
  // cohere notchedCode21 to trip 2
  
  TaArray<RadarComplex_t> cohered21_;
  RadarComplex_t *cohered21 = cohered21_.alloc(_nSamples);
  _subCode(notchedCode21, _modCode21, cohered21);
  
  // compute cohered spectra
  
  TaArray<RadarComplex_t> coheredSpec12_;
  RadarComplex_t *coheredSpec12 = coheredSpec12_.alloc(_nSamples);
  memset(coheredSpec12, 0, _nSamples * sizeof(RadarComplex_t));
  TaArray<RadarComplex_t> coheredSpec21_;
  RadarComplex_t *coheredSpec21 = coheredSpec21_.alloc(_nSamples);
  memset(coheredSpec21, 0, _nSamples * sizeof(RadarComplex_t));
  fft.fwd(cohered12, coheredSpec12);
  fft.fwd(cohered21, coheredSpec21);
  
  // compute normalized magnitude2
  
  TaArray<double> normMag12_;
  double *normMag12 = normMag12_.alloc(_nSamples);
  TaArray<double> normMag21_;
  double *normMag21 = normMag21_.alloc(_nSamples);
  _normalizeMag(coheredSpec12, normMag12);
  _normalizeMag(coheredSpec21, normMag21);
  
  // Load up convolution matrix
  // First create mag array of double length so we can
  // use it as the source for each line without worrying about wrapping
  // Then we load up each row, moving to the right by one element
  // for each row.
  
  TaArray<double> mag2_;
  double *mag2 = mag2_.alloc(_nSamples * 2);
  memcpy(mag2, normMag12, _nSamples * sizeof(double));
  memcpy(mag2 + _nSamples, normMag12, _nSamples * sizeof(double));
  
  TaArray<double> convMatrix_;
  double *convMatrix = convMatrix_.alloc(_nSamples * _nSamples);
  for (int irow = 0; irow < _nSamples; irow++) {
    memcpy(convMatrix + irow * _nSamples,
	   mag2 + _nSamples - irow, _nSamples * sizeof(double));
  }
  
  // set small values to 0 to improve condition
  
  double *conv = convMatrix;
  for (int i=0; i < _nSamples * _nSamples; i++, conv++) {
    if (fabs(*conv) < _smallValue) {
      *conv = 0;
    }
  }

  // invert the matrix
  
  memcpy(deconvMatrix, convMatrix, _nSamples * _nSamples * sizeof(double));
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

void Sz864::_initVonhann(double *window)

{
  
  // compute vonhann window
  
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

  RadarComplex_t hann[_nSamples];
  RadarComplex_t spec[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = (ii + _nSamples / 2) % _nSamples;
    hann[ii].re = window[jj];
    hann[ii].im = 0.0;
  }
  
  _fft->fwd(hann, spec);
  
  _printVector(cout, "vonhann time domain", hann);
  _printVector(cout, "vonhann spectrum", spec);

#endif
  
}
  
void Sz864::_initBlackman(double *window)

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

void Sz864::_applyWindow(const double *window,
                         const RadarComplex_t *in,
                         RadarComplex_t *out) const
  
{
  
  const double *ww = window;
  const RadarComplex_t *inp = in;
  RadarComplex_t *outp = out;
  
  for (int ii = 0; ii < _nSamples; ii++, ww++, inp++, outp++) {
    outp->re = inp->re * *ww;
    outp->im = inp->im * *ww;
  }

}
  
///////////////////////////////
// compute R1

double Sz864::_computeR1(const RadarComplex_t *IQ) const

{

  if (_nSamples < 1) {
    return 0.0;
  }

  double a = 0.0, b = 0.0;
  
  const RadarComplex_t *iq0 = IQ;
  const RadarComplex_t *iq1 = IQ + 1;

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

void Sz864::_cohereTrip1_to_Trip2(const RadarComplex_t *trip1,
                                  const RadarComplex_t *delta12,
                                  RadarComplex_t *trip2) const
  
{
  
  const RadarComplex_t *t1 = trip1;
  RadarComplex_t *t2 = trip2;
  const RadarComplex_t *dd = delta12;
  for (int ii = 0; ii < _nSamples; ii++, t1++, t2++, dd++) {
    t2->re = t1->re * dd->re + t1->im * (dd->im * -1.0);
    t2->im = t1->im * dd->re - t1->re * (dd->im * -1.0);
  }

}

///////////////////////////////////////
// change coherence from trip2 to trip1
//
// Subtract delta 12

void Sz864::_cohereTrip2_to_Trip1(const RadarComplex_t *trip2,
                                  const RadarComplex_t *delta12,
                                  RadarComplex_t *trip1) const
  
{
  
  RadarComplex_t *t1 = trip1;
  const RadarComplex_t *t2 = trip2;
  const RadarComplex_t *dd = delta12;
  for (int ii = 0; ii < _nSamples; ii++, t1++, t2++, dd++) {
    t1->re = t2->re * dd->re + t2->im * dd->im;
    t1->im = t2->im * dd->re - t2->re * dd->im;
  }

}

///////////////////////////////
// add code in time domain

void Sz864::_addCode(const RadarComplex_t *in, const RadarComplex_t *code,
                     RadarComplex_t *sum) const

{
  
  for (int ii = 0; ii < _nSamples; ii++, in++, code++, sum++) {
    sum->re = (in->re * code->re - in->im * code->im);
    sum->im = (in->re * code->im + in->im * code->re);
  }
  
}

void Sz864::_addCode(const RadarComplex_t *in,
                     const RadarComplex_t *code, int trip,
                     RadarComplex_t *sum) const

{

  const RadarComplex_t *_codeUpper = code + _nSamples - 1;
  const RadarComplex_t *_code = code + ((-(trip - 1) + _nSamples) % _nSamples);
  
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

void Sz864::_subCode(const RadarComplex_t *in, const RadarComplex_t *code,
                     RadarComplex_t *diff) const
  
{
  
  for (int ii = 0; ii < _nSamples; ii++, in++, code++, diff++) {
    diff->re = (in->re * code->re + in->im * code->im);
    diff->im = (in->im * code->re - in->re * code->im);
  }
  
}

void Sz864::_subCode(const RadarComplex_t *in,
                     const RadarComplex_t *code, int trip,
                     RadarComplex_t *diff) const
  
{
  
  const RadarComplex_t *codeUpperP = code + _nSamples - 1;
  const RadarComplex_t *codeP = code + ((-(trip - 1) + _nSamples) % _nSamples);
  
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

void Sz864::_conjugate(const RadarComplex_t *in, RadarComplex_t *conj) const

{
  
  memcpy(conj, in, _nSamples * sizeof(RadarComplex_t));
  for (int ii = 0; ii < _nSamples; ii++, conj++) {
    conj->im *= -1.0;
  }
  
}

///////////////////////////////
// load magnitudes from IQ

void Sz864::_loadMag(const RadarComplex_t *in, double *mag) const

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

void Sz864::_loadPower(const RadarComplex_t *in, double *power) const

{
  
  for (int ii = 0; ii < _nSamples; ii++, in++, power++) {
    *power = in->re * in->re + in->im * in->im;
  }

}

///////////////////////////////
// normalize magnitudes
//

void Sz864::_normalizeMag(const RadarComplex_t *in, double *norm_mag)

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

void Sz864::_computeSpectralNoise(const double *powerCentered,
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
    cerr << "noiseMean, hnoise, ratio: "
         << noiseMean << ", " << hnoise << ", " << noiseMean/hnoise << endl;
  }
#endif

}

/////////////////////////////////////////////////
// compute notch start position around
// the velocity moment
//
// returns the index for the start of the notch

int Sz864::_computeNotchStart(int notchWidth,
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

  if (_debug) {
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

int Sz864::_adjustNotchForClutter(int clutNotchStart, int clutNotchEnd,
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

  if (_debug) {
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

void Sz864::_applyNotch(int startIndex,
                        const RadarComplex_t *in,
                        int notchWidth,
                        int powerWidth,
                        double fracPower,
                        RadarComplex_t *notched) const
  
{
  
  // compute the end index for the notch
  
  int iStart = startIndex;
  int iEnd = iStart + notchWidth - 1;
  if (iStart < 0 && iEnd < 0) {
    iStart += _nSamples;
    iEnd += _nSamples;
  }

  if (_debug) {
    cerr << "======= applying notch =======" << endl;
    cerr << "startIndex: " << startIndex << endl;
    cerr << "iStart: " << iStart << endl;
    cerr << "iEnd: " << iEnd << endl;
  }

  // apply the notch, taking into account the folding of the
  // notch around the ends of the array

  if (iStart >= 0 && iEnd < _nSamples) {

    // notch does not wrap, copy array then zero out the notch
    
    memcpy(notched, in, _nSamples * sizeof(RadarComplex_t));
    memset(notched + iStart, 0, notchWidth * sizeof(RadarComplex_t));
    
    if (_debug) {
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
    
    memset(notched, 0, _nSamples * sizeof(RadarComplex_t));
    memcpy(notched + copyStart, in + copyStart,
	   powerWidth * sizeof(RadarComplex_t));

    if (_debug) {
      cerr << "--> notch does wrap" << endl;
      cerr << "  copyStart: " << copyStart << endl;
    }

  }
  
  // modify power so that power in part left is equal to original power
  
  RadarComplex_t *nn = notched;
  double mult = 1.0 / sqrt(fracPower);
  for (int ii = 0; ii < _nSamples; ii++, nn++) {
    nn->re *= mult;
    nn->im *= mult;
  }

}

void Sz864::_applyNotch75(int startIndex,
                          const RadarComplex_t *in,
                          RadarComplex_t *notched) const

{
  _applyNotch(startIndex, in,
	      _szNotchWidth75, _szPowerWidth75, _szFracPower75,
	      notched);
}

void Sz864::_applyNotch50(int startIndex,
                          const RadarComplex_t *in,
                          RadarComplex_t *notched) const

{
  _applyNotch(startIndex, in,
	      _szNotchWidth50, _szPowerWidth50, _szFracPower50,
	      notched);
}

  
//////////////////////////////////////////////////////
// compute vel and width in time domain

void Sz864::_velWidthFromTd(const RadarComplex_t *IQ,
                            double prtSecs,
                            double &vel, double &width) const
  
{

  // compute a, b, r1
  
  double a = 0.0, b = 0.0;
  
  const RadarComplex_t *iq0 = IQ;
  const RadarComplex_t *iq1 = IQ + 1;
  
  for (int i = 0; i < _nSamples - 1; i++, iq0++, iq1++) {
    a += ((iq0->re * iq1->re) + (iq0->im * iq1->im));
    b += ((iq0->re * iq1->im) - (iq1->re * iq0->im));
  }
  double r1_val = sqrt(a * a + b * b) / _nSamples;
  
  // compute c, d, r2
  
  double c = 0.0, d = 0.0;
  
  iq0 = IQ;
  const RadarComplex_t *iq2 = IQ + 2;

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
    // width = r1r2_fac * -1.0 * sqrt(fabs(ln_r1r2));
    width = r1r2_fac * sqrt(fabs(ln_r1r2));
  }
  
  if (_debug) {
    cerr << "  Pulse-pair estimates:" << endl;
    cerr << "    r1: " << r1_val << endl;
    cerr << "    r2: " << r2_val << endl;
  }
  
}

//////////////////////////////////////////////////////
// compute vel and width from the spectrum magnitudes

void Sz864::_velWidthFromFft(const double *magnitude,
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

  TaArray<double> powerCentered_;
  double *powerCentered = powerCentered_.alloc(_nSamples);
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

  if (_debug) {
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

bool Sz864::_hasReplicas(double *magnitude,
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
  TaArray<double> mag2_;
  double *mag2 = mag2_.alloc(_nSamples * 2);
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
  
  if (_debug) {
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

  //   if (count >= _szOutOfTripPowerNReplicas) {
  //     return true;
  //   } else {
  //     return false;
  //   }

}

///////////////////////////////////////////////////
// invert square matrix

void Sz864::_invertMatrix(double *data, int nn) const
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

/*****************************************************************************
 * define function to be used for sorting (lowest to highest)
 */

int Sz864::_compareDoubles(const void *v1, const void *v2)

{
  double *d1 = (double *) v1;
  double *d2 = (double *) v2;
  if (*d1 > *d2) {
    return 1;
  } else {
    return -1;
  }
}

