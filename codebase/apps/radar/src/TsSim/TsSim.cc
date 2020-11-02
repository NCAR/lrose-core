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
// TsSim.cc
//
// TsSim object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 1999
//
///////////////////////////////////////////////////////////////
//
// TsSim is a test shell for C++
//
///////////////////////////////////////////////////////////////

#include "TsSim.hh"
#include <string>
#include <iostream>
#include <iomanip>
#include <toolsa/uusleep.h>
#include <toolsa/TaArray.hh>
#include <rapmath/stats.h>
using namespace std;

// Constructor

TsSim::TsSim(int argc, char **argv) :
  _args("TsSim")

{

  OK = TRUE;

  // set programe name

  _progName = strdup("TsSim");

  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = FALSE;
    return;
  }

  return;

}

// destructor

TsSim::~TsSim()

{

  
}

//////////////////////////////////////////////////
// Run

int TsSim::Run()

{

  if (_params.mode == Params::MODE_TEST) {

    return _runTest();

  } else if (_params.mode == Params::MODE_FFT) {

    return _runFft();

  } else if (_params.mode == Params::MODE_SPRT) {

    return _runSprt();

  }

  return -1;

}

//////////////////////////////////////////////////
// Run in test mode

int TsSim::_runTest()

{

  if (_params.debug) {
    cerr << "Running in TEST mode" << endl;
  }

  int nSamples = _params.n_samples;
  double lambda = _params.wavelength / 100.0;
  double Ts = _params.prt / 1.0e6;
  double nyquist = lambda / (4.0 * Ts);

  TaArray<double> corr_;
  double *corr = corr_.alloc(nSamples);

  double wx_power = pow(10.0, _params.wx_power_dbm / 10.0);
  _computeOneSidedAutoCorr(nSamples,
                           wx_power,
                           _params.wx_width,
                           nyquist,
                           corr);

  // return 0;

  // create clutter with long time series

  //   int nLong = 2048;
  //   TaArray<RadarComplex_t> _clutLong;
  //   RadarComplex_t *clutLong = _clutLong.alloc(nLong);
  //   _createTimeSeries(1.0, 0.0, 0.1, nLong, clutLong);
  
  //   _printVector(cerr, "Unwindowed long",
  //                nLong, clutLong);
  
  // copy a short section of the time series
  
  int nShort = 64;
  TaArray<RadarComplex_t> _clutShort;
  RadarComplex_t *clutShort = _clutShort.alloc(nShort);
  //   memcpy(clutShort, clutLong, nShort * sizeof(RadarComplex_t));
  
  for (int ii = 0; ii < nShort; ii++) {
    clutShort[ii].re = 1.0;
    clutShort[ii].im = 0.0;
  }
  
  _printVector(cerr, "Unwindowed clutter time series",
	       nShort, clutShort, false);
  
  // apply window to the short time series
  
  TaArray<double> _window;
  double *window = _window.alloc(nShort);
  // _computeWindowVonhann(nShort, window);
  // _computeWindowBlackman(nShort, window);
  _computeWindowHanning(nShort, window);
  // _computeWindowRect(nShort, window);
  _applyWindow(nShort, window, clutShort);
  
  //   _printArray(cerr, "Window", nShort, window);
  
  _printVector(cerr, "Windowed clutter time series",
               nShort, clutShort, false);
  
  // Take fft of the short clutter time series
  
  TaArray<RadarComplex_t> _clutSpec;
  RadarComplex_t *clutSpec = _clutSpec.alloc(nShort);
  
  Fft fft(nShort);
  fft.fwd(clutShort, clutSpec);
  
  // print out
  
  _printVector(cerr, "Windowed clutter spectrum",
               nShort, clutSpec, true);
  
  return 0;

}

//////////////////////////////////////////////////
// Run in fft mode

int TsSim::_runFft()

{

  if (_params.debug) {
    cerr << "Running in FFT mode" << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Run in staggered prt mode

int TsSim::_runSprt()

{

  if (_params.debug) {
    cerr << "Running in SPRT mode" << endl;
  }

  // create wx spectrum with long time series

  int nn = 2048;
  TaArray<RadarComplex_t> _wxTseries;
  RadarComplex_t *wxTseries = _wxTseries.alloc(nn);
  double wx_power = pow(10.0, _params.wx_power_dbm / 10.0);
  _createTimeSeries(wx_power, _params.wx_velocity, _params.wx_width,
		    nn, wxTseries);
  
  // compute spectrum of weather time series

  TaArray<RadarComplex_t> _wxSpectrum;
  RadarComplex_t *wxSpectrum = _wxSpectrum.alloc(nn);

  Fft fft(nn);
  fft.fwd(wxTseries, wxSpectrum);

  double power, vel, width;
  _computeMomentsFromSpec(wxSpectrum, nn, power, vel, width);
  cerr << "Orig power, vel, width: " << power << ", " << vel << ", " << width << endl;
  
  // _printVector(cout, "Wx spectrum", nn, wxSpectrum, true);

  // construct time series using every second point

  int mm = nn / 2;
  TaArray<RadarComplex_t> _modTseries;
  RadarComplex_t *modTseries = _modTseries.alloc(mm);
  modTseries[0] = wxTseries[0];
  RadarComplex_t cumDiff(1.0, 0.0);
  int nmod = 1;
  
  for (int ii = 1; ii < nn - 2; ii += 2) {
    RadarComplex_t normDiff =
      RadarComplex::normConjugateProduct(wxTseries[ii+1], wxTseries[ii]);
    cumDiff = RadarComplex::normComplexProduct(cumDiff, normDiff);
    modTseries[nmod] = RadarComplex::conjugateProduct(wxTseries[ii], cumDiff);
    nmod++;
  }
  
  // compute spectrum of modified time series

  TaArray<RadarComplex_t> _modSpectrum;
  RadarComplex_t *modSpectrum = _modSpectrum.alloc(nmod);

  Fft fft2(nmod);
  fft2.fwd(modTseries, modSpectrum);

  double modPower, modVel, modWidth;
  _computeMomentsFromSpec(modSpectrum, nmod, modPower, modVel, modWidth);
  cerr << "Mod power, vel, width: "
       << modPower << ", " << modVel << ", " << modWidth << endl;

  _printVector(cout, "Mod spectrum", nmod, modSpectrum, true);

  return 0;

}

////////////////////////////////////////
// create gaussian spectrum

void TsSim::_createTimeSeries(double power,
                              double vel,
                              double width,
                              int nSamples,
                              RadarComplex_t *iq)

{

  double lambda = _params.wavelength / 100.0;
  double Ts = _params.prt / 1.0e6;
  double nyquist = lambda / (4.0 * Ts);
  double RNoise = _params.receiver_noise;

  double fvmean = (2.0 * vel) / lambda;
  double fvsigma = (2.0 * width) / lambda;

  double fmin = -0.5 / Ts;
  double fmax = 0.5 / Ts;
  double fdelta = (fmax - fmin) / nSamples;

  double C1 = power / Ts;
  double C2 = 1.0 / (sqrt(2.0 * M_PI) * fvsigma);

  TaArray<RadarComplex_t> spec_;
  RadarComplex_t *spec = spec_.alloc(nSamples);

  for (int k = 0; k < nSamples; k++) {
    spec[k].re = 0.0;
    spec[k].im = 0.0;
  }

  double nn = 0.0;
  double sumPower = 0.0;
  double sumVel = 0.0;
  double sumsqVel = 0.0;

  for (int k = -nSamples * 2; k < 3 * nSamples; k++) {

    double f = fmin + k * fdelta;
    double dd = f - fvmean;
    double xx = (dd * dd) / (2.0 * fvsigma * fvsigma);

    double sig = C2 * exp(-xx) + RNoise * Ts;
    double mag;
    if (_params.use_exponential) {
      double xx = STATS_exponential_gen(_params.exponential_lambda);
      double scale = 1.0 + xx - _params.exponential_lambda;
      mag = sqrt(scale * C1 * sig);
    } else {
      mag = sqrt(C1 * sig);
    }
    
    double phase;
    if (_params.force_zero_phase) {
      phase = 0.0;
    } else {
      phase = STATS_uniform_gen() * 2.0 * M_PI;
    }

    int kk = (k + nSamples/2 + 2 * nSamples) % nSamples;
    
    spec[kk].re += mag * cos(phase);
    spec[kk].im += mag * sin(phase);

    // sum up for checking properties

    nn++;
    double power = mag * mag;
    sumPower += power;
    double vv = -nyquist + ((double) k / (double) nSamples) * nyquist * 2.0;
    sumVel += vv * power;
    sumsqVel += vv * vv * power;

  } // k
  
  // compute stats for checking
  
  double meanVel = sumVel / sumPower;
  double diff = (sumsqVel / sumPower) - (meanVel * meanVel);
  double sdevVel = 0.0;
  if (diff > 0) {
    sdevVel = sqrt(diff);
  }

  double genPower = sumPower / nSamples;

  // scale to correct power

  sumPower = 0.0;
  double mult = sqrt(power / genPower);
  for (int k = 0; k < nSamples; k++) {
    spec[k].re *= mult;
    spec[k].im *= mult;
    sumPower += spec[k].re * spec[k].re + spec[k].im  + spec[k].im;
  }

  double checkPower = sumPower / nSamples;
  double checkDbm = 10.0 * log10(checkPower);
  double checkVel = meanVel;
  double checkWidth = sdevVel;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printVector(cerr, "Generated time series", nSamples, spec, false);
  }
  if (_params.debug) {
    cerr << "  requested   dbm: " << 10.0 * log10(power) << endl;
    cerr << "  requested   vel: " << vel << endl;
    cerr << "  requested width: " << width << endl;
    cerr << "  check       dbm: " << checkDbm << endl;
    cerr << "  check       vel: " << checkVel << endl;
    cerr << "  check     width: " << checkWidth << endl;
  }

  // invert the spectra back into iq

  Fft fft(nSamples);
  fft.inv(spec, iq);

}


///////////////////////////////
// print complex compoments

void TsSim::_printComplex(ostream &out,
			  const string &heading,
                          int nSamples,
			  const RadarComplex_t *comp,
                          bool shift)
  
{
  
  out << "---->> " << heading << " <<----" << endl;
  out << setw(3) << "ii" << "  "
      << setw(10) << "re" << "  "
      << setw(10) << "im" << endl;
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = ii;
    if (shift) {
      jj = (ii + nSamples / 2) % nSamples;
    }
    const RadarComplex_t &val = comp[jj];
    out << setw(3) << ii << "  "
	<< setw(10) << val.re << "  "
	<< setw(10) << val.im << endl;
  }
  out.flush();

}

///////////////////////////////
// print complex vector

void TsSim::_printVector(ostream &out,
			 const string &heading,
                         int nSamples,
			 const RadarComplex_t *comp,
                         bool shift)
  
{
  
  cerr << "---->> " << heading << " <<----" << endl;
  cerr << setw(3) << "ii" << "  "
       << setw(10) << "magnitude" << "  "
       << setw(10) << "angle" << endl;
  cerr.flush();
  for (int ii = 0; ii < nSamples; ii++) {
    int jj = ii;
    double pos = (double) ii / (double) nSamples;
    if (shift) {
      jj = (ii + nSamples / 2) % nSamples;
      pos = (double) (ii - nSamples / 2.0) / (double) nSamples;
    }
    const RadarComplex_t &val = comp[jj];
    double mag = sqrt(val.re * val.re + val.im * val.im);
    double angle = atan2(val.im, val.re) * RAD_TO_DEG;
    // fprintf(stdout, "%d %.12e %.12e\n", ii, mag, angle);
    out << setw(3) << ii << "  "
	<< setw(10) << pos << "  "
	<< setw(10) << mag << "  "
	<< setw(10) << angle << endl;
  }
  out.flush();

}

///////////////////////////////
// print array

void TsSim::_printArray(ostream &out,
                        const string &heading,
                        int nSamples,
                        const double *array)
  
{
  
  out << "---->> " << heading << " <<----" << endl;
  out << setw(3) << "ii" << "  "
      << setw(10) << "value" << endl;
  for (int ii = 0; ii < nSamples; ii++, array++) {
    double val = *array;
    out << setw(3) << ii << "  "
	 << setw(10) << val << endl;
  }
  out.flush();

}

/////////////////////////////////////
// compute window
//
// Allocates memory and returns window

void TsSim::_computeWindowRect(int n, double *window)

{

  for (int ii = 0; ii < n; ii++) {
    window[ii] = 1.0;
  }

}
  
void TsSim::_computeWindowVonhann(int n, double *window)

{

  for (int ii = 0; ii < n; ii++) {
    double ang = 2.0 * M_PI * ((ii + 0.5) / (double) n - 0.5);
    window[ii] = 0.5 * (1.0 + cos(ang));
  }

  // adjust window to keep power constant

  double sumsq = 0.0;
  for (int ii = 0; ii < n; ii++) {
    sumsq += window[ii] * window[ii];
  }
  double rms = sqrt(sumsq / n);
  for (int ii = 0; ii < n; ii++) {
    window[ii] /= rms;
  }

}
  
void TsSim::_computeWindowHanning(int n, double *window)

{

  for (int ii = 0; ii < n; ii++) {
    double ang = 2.0 * M_PI * (ii / (double) n - 0.5);
    window[ii] = 0.5 * (1.0 + cos(ang));
  }

  // adjust window to keep power constant

  double sumsq = 0.0;
  for (int ii = 0; ii < n; ii++) {
    sumsq += window[ii] * window[ii];
  }
  double rms = sqrt(sumsq / n);
  for (int ii = 0; ii < n; ii++) {
    window[ii] /= rms;
  }

}
  
void TsSim::_computeWindowBlackman(int n, double *window)

{
  
  // compute blackman window
  
  for (int ii = 0; ii < n; ii++) {
    double pos =
      ((n + 1.0) / 2.0 + (double) ii ) / n;
    window[ii] = (0.42
                  + 0.5 * cos(2.0 * M_PI * pos)
                  + 0.08 * cos(4.0 * M_PI * pos));
  }
  
  // adjust window to keep power constant
  
  double sumsq = 0.0;
  for (int ii = 0; ii < n; ii++) {
    sumsq += window[ii] * window[ii];
  }
  double rms = sqrt(sumsq / n);
  for (int ii = 0; ii < n; ii++) {
    window[ii] /= rms;
  }

}
  
///////////////////////////////////
// apply window

void TsSim::_applyWindow(int nSamples,
                         const double *window,
                         RadarComplex_t *iq) const
  
{
  
  const double *ww = window;
  RadarComplex_t *iqp = iq;
  
  for (int ii = 0; ii < nSamples; ii++, ww++, iqp++) {
    iqp->re *= *ww;
    iqp->im *= *ww;
  }

}

///////////////////////////////////////
// compute one-sided auto-correlation

void TsSim::_computeOneSidedAutoCorr(int nSamples,
                                     double power,
                                     double width,
                                     double nyquist,
                                     double *corr) const
  
{
  

  for (int ii = 0; ii < nSamples; ii++) {
    
    double fac = (M_PI * width * ii) / nyquist;
    corr[ii] = power * exp(-0.5 * fac * fac);

    // cerr << "ii, corr: " << ii << ", " << corr[ii] << endl;
    
  }


}

///////////////////////////////////////
// compute real cross correlation

void TsSim::_computeRealCrossCorr(int nSamples,
                                  const double *aa,
                                  const double *bb,
                                  double *corr) const
  
{
  
  
  for (int mm = 0; mm < nSamples; mm++) {
    
    double sum = 0.0;
    double count = 0.0;

    for (int ii = 0; ii < nSamples - mm; ii++) {
      sum += aa[ii+mm] * bb[ii];
      count++;
    }

    corr[mm] = sum / count;
    cerr << "mm, corr: " << mm << ", " << corr[mm] << endl;
    
  }


}

////////////////////////////////////////
// create gaussian spectrum

void TsSim::_createTimeSeries2(double power,
                               double vel,
                               double width,
                               int nSamples,
                               RadarComplex_t *iq)

{

  double lambda = _params.wavelength / 100.0;
  double Ts = _params.prt / 1.0e6;
  double nyquist = lambda / (4.0 * Ts);
  double RNoise = _params.receiver_noise;

//   TaArray<double> B11_;
//   double *B11 = B11_.alloc(nSamples * 2);

  // loop until we have a good auto-correlation function

  bool done = false;
  while (!done) {

  } // while (!done)



  double fvmean = (2.0 * vel) / lambda;
  double fvsigma = (2.0 * width) / lambda;

  double fmin = -0.5 / Ts;
  double fmax = 0.5 / Ts;
  double fdelta = (fmax - fmin) / nSamples;

  double C1 = power / Ts;
  double C2 = 1.0 / (sqrt(2.0 * M_PI) * fvsigma);

  TaArray<RadarComplex_t> spec_;
  RadarComplex_t *spec = spec_.alloc(nSamples);

  for (int k = 0; k < nSamples; k++) {
    spec[k].re = 0.0;
    spec[k].im = 0.0;
  }

  double nn = 0.0;
  double sumPower = 0.0;
  double sumVel = 0.0;
  double sumsqVel = 0.0;

  for (int k = -nSamples * 2; k < 3 * nSamples; k++) {

    double f = fmin + k * fdelta;
    double dd = f - fvmean;
    double xx = (dd * dd) / (2.0 * fvsigma * fvsigma);

    double sig = C2 * exp(-xx) + RNoise * Ts;
    double mag;
    if (_params.use_exponential) {
      double xx = STATS_exponential_gen(_params.exponential_lambda);
      double scale = 1.0 + xx - _params.exponential_lambda;
      mag = sqrt(scale * C1 * sig);
    } else {
      mag = sqrt(C1 * sig);
    }
    
    double phase;
    if (_params.force_zero_phase) {
      phase = 0.0;
    } else {
      phase = STATS_uniform_gen() * 2.0 * M_PI;
    }

    int kk = (k + nSamples/2 + 2 * nSamples) % nSamples;
    
    spec[kk].re += mag * cos(phase);
    spec[kk].im += mag * sin(phase);

    // sum up for checking properties

    nn++;
    double power = mag * mag;
    sumPower += power;
    double vv = -nyquist + ((double) k / (double) nSamples) * nyquist * 2.0;
    sumVel += vv * power;
    sumsqVel += vv * vv * power;

  } // k
  
  // compute stats for checking
  
  double meanVel = sumVel / sumPower;
  double diff = (sumsqVel / sumPower) - (meanVel * meanVel);
  double sdevVel = 0.0;
  if (diff > 0) {
    sdevVel = sqrt(diff);
  }

  double genPower = sumPower / nSamples;

  // scale to correct power

  sumPower = 0.0;
  double mult = sqrt(power / genPower);
  for (int k = 0; k < nSamples; k++) {
    spec[k].re *= mult;
    spec[k].im *= mult;
    sumPower += spec[k].re * spec[k].re + spec[k].im  + spec[k].im;
  }

  double checkPower = sumPower / nSamples;
  double checkDbm = 10.0 * log10(checkPower);
  double checkVel = meanVel;
  double checkWidth = sdevVel;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printVector(cerr, "Generated time series", nSamples, spec, false);
  }
  if (_params.debug) {
    cerr << "  requested   dbm: " << 10.0 * log10(power) << endl;
    cerr << "  requested   vel: " << vel << endl;
    cerr << "  requested width: " << width << endl;
    cerr << "  check       dbm: " << checkDbm << endl;
    cerr << "  check       vel: " << checkVel << endl;
    cerr << "  check     width: " << checkWidth << endl;
  }

  // invert the spectra back into iq

  Fft fft(nSamples);
  fft.inv(spec, iq);

}


////////////////////////////////////////////////////
// compute fft-based moments

void TsSim::_computeMomentsFromSpec(const RadarComplex_t *spec,
				    int nn,
				    double &power,
				    double &vel,
				    double &width)
  
{
  
  // compute power
  
  power = _computePower(spec, nn);
  
  // compute power spectrum
  
  TaArray<double> powerSpec_;
  double *powerSpec = powerSpec_.alloc(nn);
  _loadPowerSpec(spec, nn, powerSpec);
  
  // compute vel and width from power spectrum
  
  double vv, ww;
  _velWidthFromFft(powerSpec, nn, vv, ww);
  
  // set return vals, changing the sign of the velocity so that
  // motion away from the radar is positive

  vel = -1.0 * vv;
  width = ww;
  
}

///////////////////////////////////////////////
// compute total power

double TsSim::_computePower(const RadarComplex_t *val,
			    int nn)
  
{
  double p = 0.0;
  for (int i = 0; i < nn; i++, val++) {
    double re = val->re;
    double im = val->im;
    p += (re * re + im * im);
  }
  return (p / nn);
}
  
///////////////////////////////////
// load power from complex spectrum

void TsSim::_loadPowerSpec(const RadarComplex_t *in,
			   int nn,
			   double *power)
  
{
  
  for (int ii = 0; ii < nn; ii++, in++, power++) {
    *power = in->re * in->re + in->im * in->im;
  }

}

//////////////////////////////////////////////////////
// compute vel and width from the power spectrum

void TsSim::_velWidthFromFft(const double *powerSpec,
			     int nn,
			     double &vel,
			     double &width)
  
{

  int kMid = nn / 2;
  
  // find max power
  
  double maxPower = 0.0;
  int kMax = 0;
  const double *ps = powerSpec;
  for (int ii = 0; ii < nn; ii++, ps++) {
    if (*ps > maxPower) {
      kMax = ii;
      maxPower = *ps;
    }
  }
  if (kMax >= kMid) {
    kMax -= nn;
  }
  
  // center power array on the max value
  
  TaArray<double> powerCentered_;
  double *powerCentered = powerCentered_.alloc(nn);
  int kOffset = kMid - kMax;
  for (int ii = 0; ii < nn; ii++) {
    int jj = (ii + kOffset) % nn;
    powerCentered[jj] = powerSpec[ii];
  }
  
  // compute mom1 and mom2, using those points above the noise floor

  double sumPower = 0.0;
  double sumK = 0.0;
  double sumK2 = 0.0;
  double *pw = powerCentered;
  for (int ii = 0; ii < nn; ii++, pw++) {
    double phase = (double) ii;
    double pExcess = *pw;
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

  double prtSecs = _params.prt / 1.0e6;
  double wavelengthMeters = _params.wavelength / 100;
  double velFac = wavelengthMeters / (2.0 * nn * prtSecs);
  vel = velFac * (meanK - kOffset);
  width = velFac * sdevK;

}

