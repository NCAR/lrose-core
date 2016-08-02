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
// PhaseCoding.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2014
//
///////////////////////////////////////////////////////////////
//
// PhaseCoding handles phase-coded radar data
//
////////////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <radar/PhaseCoding.hh>
using namespace std;

// initialize const doubles

const double PhaseCoding::_missingDbl = -9999.0;
const double PhaseCoding::_smallValue = 1.0e-9;
const double PhaseCoding::_fracPower75 = 0.25;
const double PhaseCoding::_fracPower50 = 0.5;

// Constructor

PhaseCoding::PhaseCoding(bool debug) :
        _debug(debug)
  
{

  _nSamples = 64;

  _wavelengthMeters = 0.1068;
  setNoiseValueDbm(-113.0);
  
  _snrThreshold = 3.0;
  _ncpThreshold = 0.5;

  _outOfTripPowerRatioThreshold = 6.0;
  _outOfTripPowerNReplicas = 3;

}

// destructor

PhaseCoding::~PhaseCoding()

{

}

/////////////////////////////////////
// set methods

void PhaseCoding::setWavelength(double wavelength) {
  _wavelengthMeters = wavelength;
}

void PhaseCoding::setNoiseValueDbm(double dbm) {
  _noiseValueDbm = dbm;
  _noiseValueMwatts = pow(10.0, dbm / 10.0);
}

void PhaseCoding::setSnrThreshold(double db) {
  _snrThreshold = db;
}

void PhaseCoding::setNcpThreshold(double val) {
  _snrThreshold = val;
}

void PhaseCoding::setOutOfTripPowerRatioThreshold(double db) {
  _outOfTripPowerRatioThreshold = db;
}

void PhaseCoding::setOutOfTripPowerNReplicas(int n) {
  _outOfTripPowerNReplicas = n;
}

///////////////////////////////////////////////
// compute total power from IQ

double PhaseCoding::computePower(const RadarComplex_t *IQ) const
  
{
  double p = 0.0;
  for (int i = 0; i < _nSamples; i++, IQ++) {
    p += ((IQ->re * IQ->re) + (IQ->im * IQ->im));
  }
  return (p / _nSamples);
}
  
///////////////////////////////////////////////
// compute total power from magnitudes

double PhaseCoding::computePower(const double *mag) const
  
{
  double p = 0.0;
  for (int i = 0; i < _nSamples; i++, mag++) {
    p += (*mag * *mag);
  }
  return (p / _nSamples);
}
  
///////////////////////////////////////////////
// adjust the power in IQ data to a given value

void PhaseCoding::adjustPower(RadarComplex_t *IQ,
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

bool PhaseCoding::checkSnrThreshold(const RadarComplex_t *IQ,
                                    double &totalPower) const
  
{
  double power = computePower(IQ);
  double dbm = 10.0 * log10(power);
  if (dbm < _noiseValueDbm + _snrThreshold) {
    return true;
  }
  totalPower = power;
  return false;
}
  
/////////////////////////////////////////////////////////////
// cohere from measued phase to given trip
// to cohere to the given trip, we need to subtract the
// burst phase from the received i/q data. This is
// done by multiplying the i/q value by the complex conjugate
// of the phase code

void PhaseCoding::cohereToTrip1(const RadarComplex_t *measured,
                                vector<IwrfTsPulse::burst_phase_t> &codes,
                                RadarComplex_t *trip1) const
  
{

  for (int ii = 0; ii < _nSamples; ii++, measured++, trip1++) {
    const RadarComplex_t &code = codes[ii].trip1;
    trip1->re = (measured->re * code.re) + (measured->im * code.im);
    trip1->im = (measured->im * code.re) - (measured->re * code.im);
  }

}

void PhaseCoding::cohereToTrip2(const RadarComplex_t *measured,
                                vector<IwrfTsPulse::burst_phase_t> &codes,
                                RadarComplex_t *trip2) const
  
{

  for (int ii = 0; ii < _nSamples; ii++, measured++, trip2++) {
    const RadarComplex_t &code = codes[ii].trip2;
    trip2->re = (measured->re * code.re) + (measured->im * code.im);
    trip2->im = (measured->im * code.re) - (measured->re * code.im);
  }

}

void PhaseCoding::cohereToTrip3(const RadarComplex_t *measured,
                                vector<IwrfTsPulse::burst_phase_t> &codes,
                                RadarComplex_t *trip3) const
  
{

  for (int ii = 0; ii < _nSamples; ii++, measured++, trip3++) {
    const RadarComplex_t &code = codes[ii].trip3;
    trip3->re = (measured->re * code.re) + (measured->im * code.im);
    trip3->im = (measured->im * code.re) - (measured->re * code.im);
  }

}

void PhaseCoding::cohereToTrip4(const RadarComplex_t *measured,
                                vector<IwrfTsPulse::burst_phase_t> &codes,
                                RadarComplex_t *trip4) const
  
{
  
  for (int ii = 0; ii < _nSamples; ii++, measured++, trip4++) {
    const RadarComplex_t &code = codes[ii].trip4;
    trip4->re = (measured->re * code.re) + (measured->im * code.im);
    trip4->im = (measured->im * code.re) - (measured->re * code.im);
  }

}

/////////////////////////////////////////////////////////////
// cohere from given trip back to measured phase
// To get back to the measured phase, we need to add back
// in the burst phase. This is done by a complex multiply.

void PhaseCoding::revertFromTrip1(const RadarComplex_t *trip1,
                                  vector<IwrfTsPulse::burst_phase_t> &codes,
                                  RadarComplex_t *measured) const
  
{
  
  for (int ii = 0; ii < _nSamples; ii++, measured++, trip1++) {
    const RadarComplex_t &code = codes[ii].trip1;
    measured->re = (trip1->re * code.re) - (trip1->im * code.im);
    measured->im = (trip1->im * code.re) + (trip1->re * code.im);
  }

}

void PhaseCoding::revertFromTrip2(const RadarComplex_t *trip2,
                                  vector<IwrfTsPulse::burst_phase_t> &codes,
                                  RadarComplex_t *measured) const
  
{
  
  for (int ii = 0; ii < _nSamples; ii++, measured++, trip2++) {
    const RadarComplex_t &code = codes[ii].trip2;
    measured->re = (trip2->re * code.re) - (trip2->im * code.im);
    measured->im = (trip2->im * code.re) + (trip2->re * code.im);
  }

}

void PhaseCoding::revertFromTrip3(const RadarComplex_t *trip3,
                                  vector<IwrfTsPulse::burst_phase_t> &codes,
                                  RadarComplex_t *measured) const
  
{
  
  for (int ii = 0; ii < _nSamples; ii++, measured++, trip3++) {
    const RadarComplex_t &code = codes[ii].trip3;
    measured->re = (trip3->re * code.re) - (trip3->im * code.im);
    measured->im = (trip3->im * code.re) + (trip3->re * code.im);
  }

}

void PhaseCoding::revertFromTrip4(const RadarComplex_t *trip4,
                                  vector<IwrfTsPulse::burst_phase_t> &codes,
                                  RadarComplex_t *measured) const
  
{
  
  for (int ii = 0; ii < _nSamples; ii++, measured++, trip4++) {
    const RadarComplex_t &code = codes[ii].trip4;
    measured->re = (trip4->re * code.re) - (trip4->im * code.im);
    measured->im = (trip4->im * code.re) + (trip4->re * code.im);
  }

}

/////////////////////////////////////////////////////////////
// Apply a notch to this time series

void PhaseCoding::applyNotch(const RadarFft &fft,
                             RadarComplex_t *iq,
                             double notchFraction)
  
{

  // compute spectrum

  TaArray<RadarComplex_t> spec_;
  RadarComplex_t *spec = spec_.alloc(_nSamples);
  fft.fwd(iq, spec);

  // compute power spectrum

  TaArray<double> powerSpec_;
  double *powerSpec = powerSpec_.alloc(_nSamples);
  RadarComplex::loadPower(spec, powerSpec, _nSamples);
  
  // find the location of the peak

  int ipeak = 0;
  double maxVal = -1.0e99;
  for (int ii = 0; ii < _nSamples; ii++) {
    if (powerSpec[ii] > maxVal) {
      ipeak = ii;
      maxVal = powerSpec[ii];
    }
  }

  // compute notch width and starting point

  int notchWidth = (int) (notchFraction * (double) _nSamples + 0.5);
  int notchHalf = (int) (notchFraction * (double) _nSamples * 0.5 + 0.5);
  int notchStart = (ipeak - notchHalf + _nSamples) % _nSamples;
  
  // apply the notch

  for (int ii = 0; ii <= notchWidth; ii++) {
    int jj = (ii + notchStart) % _nSamples;
    spec[jj].re = 0.0;
    spec[jj].im = 0.0;
  }
  
  // adjust for power missing in notch
  
  double fraction = 1.0 - ((double) notchWidth / (double) _nSamples);
  double pwrMult = sqrt(1.0 / fraction);

  for (int ii = 0; ii < _nSamples; ii++) {
    spec[ii].re *= pwrMult;
    spec[ii].im *= pwrMult;
  }
  
  // invert back into time domain
  
  fft.inv(spec, iq);

}

///////////////////////////////
// compute R1

double PhaseCoding::_computeR1(const RadarComplex_t *IQ) const

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
  
///////////////////////////////
// create conjugate
//

void PhaseCoding::_conjugate(const RadarComplex_t *in, RadarComplex_t *conj) const

{
  
  memcpy(conj, in, _nSamples * sizeof(RadarComplex_t));
  for (int ii = 0; ii < _nSamples; ii++, conj++) {
    conj->im *= -1.0;
  }
  
}

///////////////////////////////
// load magnitudes from IQ

void PhaseCoding::_loadMag(const RadarComplex_t *in, double *mag) const

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

void PhaseCoding::_loadPower(const RadarComplex_t *in, double *power) const

{
  
  for (int ii = 0; ii < _nSamples; ii++, in++, power++) {
    *power = in->re * in->re + in->im * in->im;
  }

}

///////////////////////////////
// normalize magnitudes
//

void PhaseCoding::_normalizeMag(const RadarComplex_t *in, double *norm_mag)

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

void PhaseCoding::_computeSpectralNoise(const double *powerCentered,
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

int PhaseCoding::_computeNotchStart(int notchWidth,
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

int PhaseCoding::_adjustNotchForClutter(int clutNotchStart, int clutNotchEnd,
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

void PhaseCoding::_applyNotch(int startIndex,
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

void PhaseCoding::_applyNotch75(int startIndex,
                                const RadarComplex_t *in,
                                RadarComplex_t *notched) const

{
  _applyNotch(startIndex, in,
	      _notchWidth75, _powerWidth75, _fracPower75,
	      notched);
}

void PhaseCoding::_applyNotch50(int startIndex,
                                const RadarComplex_t *in,
                                RadarComplex_t *notched) const

{
  _applyNotch(startIndex, in,
	      _notchWidth50, _powerWidth50, _fracPower50,
	      notched);
}

  
//////////////////////////////////////////////////////
// compute vel and width in time domain

void PhaseCoding::_velWidthFromTd(const RadarComplex_t *IQ,
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

void PhaseCoding::_velWidthFromFft(const double *magnitude,
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

bool PhaseCoding::_hasReplicas(double *magnitude,
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

  double ratio = pow(10.0, _outOfTripPowerRatioThreshold / 10.0);
  double testVal = peaks[7] / ratio;
  int count = 0;
  for (int i = 0; i < 6; i++) {
    if (peaks[i] > testVal) {
      count++;
    }
  }
  double sum = 0.0;
  for (int i = 0; i < _outOfTripPowerNReplicas; i++) {
    sum += peaks[i];
  }
  leakage = (sum / _outOfTripPowerNReplicas) / peaks[7];
  
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

}

///////////////////////////////////////////////////
// invert square matrix

void PhaseCoding::_invertMatrix(double *data, int nn) const
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

int PhaseCoding::_compareDoubles(const void *v1, const void *v2)

{
  double *d1 = (double *) v1;
  double *d2 = (double *) v2;
  if (*d1 > *d2) {
    return 1;
  } else {
    return -1;
  }
}

