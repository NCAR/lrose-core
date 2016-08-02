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
// WxSpecSim.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2003
//
///////////////////////////////////////////////////////////////
//
// WxSpecSim simulates overlaid trips in raw IQ data and writes
// to a netCDF file.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <ctime>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <toolsa/os_config.h>
#include <rapmath/stats.h>
#include <toolsa/file_io.h>
#include "WxSpecSim.hh"

using namespace std;

// Constructor

WxSpecSim::WxSpecSim(int argc, char **argv)

{

  isOK = true;
  _fft = NULL;
  
  // set programe name

  _progName = "WxSpecSim";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // set variables

  _nSamples = _params.nsamples;
  if (_params.data_mode == Params::RANDOM_DATA) {
    _nGates = _params.ngates;
  } else {
    _nGates = _params.gate_data_n;
  }
  
  // initialize random number generator

  // STATS_uniform_seed((int) time(NULL));
  STATS_uniform_seed(0);

  // set up FFT object

  _fft = new Fft(_nSamples);

  return;

}

// destructor

WxSpecSim::~WxSpecSim()

{

  if (_fft) {
    delete _fft;
  }

}

//////////////////////////////////////////////////
// Run

int WxSpecSim::Run ()
{

  if (_params.data_mode == Params::SPECIFY_DATA) {
    
    for (int igate = 0; igate < _nGates; igate++) {
      
      _gateNum = igate;
      _createGateData(_params._gate_data[igate].dbm,
                      _params._gate_data[igate].vel,
                      _params._gate_data[igate].width);
      
    } // igate

  } else {
    
    // random data
    
    double dbmLow = _params.random_dbm_low;
    double dbmRange = _params.random_dbm_high - _params.random_dbm_low;

    double velLow = _params.random_vel_low;
    double velRange = _params.random_vel_high - _params.random_vel_low;

    double widthLow = _params.random_width_low;
    double widthRange = _params.random_width_high - _params.random_width_low;

    for (int igate = 0; igate < _nGates; igate++) {
      
      _gateNum = igate;

      double dbm = dbmLow + STATS_uniform_gen() * dbmRange;
      double vel = velLow + STATS_uniform_gen() * velRange;
      double width = widthLow + STATS_uniform_gen() * widthRange;
      
      _createGateData(dbm, vel, width);
      
    } // igate

  }

  return 0;

}

////////////////////////////////////////
// create data for a gate

void WxSpecSim::_createGateData(double dbm,
                                double vel,
                                double width)
  
{
  
  double power = pow(10.0, dbm / 10.0);
  
  if (_params.debug) {
    cerr << "---> Simulated signal <---" << endl;
    cerr << "     dbm, vel, width: "
	 << dbm << ", " << vel << ", " << width << endl;
  }
  
  vector<Complex_t> td;
  td.reserve(_nSamples);
  
  _createGaussian(power, -vel, width, td);

  double truth_power = power;
  double truth_vel = vel;
  double truth_width = width;
  
  if (_params.truth_method == Params::FFT_TRUTH) {

    moments_t est;
    _momentsByFft(td, _params.prt / 1000000.0,
		  est.power, est.vel, est.width);

    truth_power = est.power;
    truth_vel = est.vel;
    truth_width = est.width;

    est.dbm = 10.0 * log10(est.power);

    if (_params.debug) {
      cerr << "FFT truth" << endl;
      cerr << "     dbm, vel, width: "
	   << est.dbm << ", " << est.vel << ", " << est.width << endl;
    }
  
  } else if (_params.truth_method == Params::PP_TRUTH) {
    
    moments_t est;
    _momentsByPp(td, _params.prt / 1000000.0,
		 est.power, est.vel, est.width);
    est.dbm = 10.0 * log10(est.power);

    truth_power = est.power;
    truth_vel = est.vel;
    truth_width = est.width;

    if (_params.debug) {
      cerr << "PP truth" << endl;
      cerr << "     dbm, vel, width: "
	   << est.dbm << ", "
	   << est.vel << ", " << est.width << endl;
    }
  
  }

}

////////////////////////////////////////
// create gaussian spectrum

void WxSpecSim::_createGaussian(double power,
                                double vel,
                                double width,
                                vector<Complex_t> &volts)
  
{

  double lambda = _params.wavelength / 100.0;
  double Ts = _params.prt / 1.0e6;
  double nyquist = lambda / (4.0 * Ts);
  double RNoise = _params.receiver_noise;

  double fvmean = (2.0 * vel) / lambda;
  double fvsigma = (2.0 * width) / lambda;

  double fmin = -0.5 / Ts;
  double fmax = 0.5 / Ts;
  double fdelta = (fmax - fmin) / _nSamples;

  double C1 = power / Ts;
  double C2 = 1.0 / (sqrt(2.0 * M_PI) * fvsigma);

  vector<Complex_t> spec;
  spec.reserve(_nSamples);

  for (int k = 0; k < _nSamples; k++) {
    spec[k].re = 0.0;
    spec[k].im = 0.0;
  }

  double nn = 0.0;
  double sumPower = 0.0;
  double sumVel = 0.0;
  double sumsqVel = 0.0;

  for (int k = -_nSamples * 2; k < 3 * _nSamples; k++) {

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

    int kk = (k + _nSamples/2 + 2 * _nSamples) % _nSamples;
    
    spec[kk].re += mag * cos(phase);
    spec[kk].im += mag * sin(phase);

    // sum up for checking properties

    nn++;
    double power = mag * mag;
    sumPower += power;
    double vv = -nyquist + ((double) k / (double) _nSamples) * nyquist * 2.0;
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

  double genPower = sumPower / _nSamples;

  // scale to correct power

  sumPower = 0.0;
  double mult = sqrt(power / genPower);
  for (int k = 0; k < _nSamples; k++) {
    spec[k].re *= mult;
    spec[k].im *= mult;
    sumPower += spec[k].re * spec[k].re + spec[k].im  + spec[k].im;
  }

  double checkPower = sumPower / _nSamples;
  double checkDbm = 10.0 * log10(checkPower);
  double checkVel = meanVel;
  double checkWidth = sdevVel;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "     check dbm: " << checkDbm << endl;
    cerr << "     check vel: " << checkVel << endl;
    cerr << "     check width: " << checkWidth << endl;
    _printVector(cerr, "Single gaussian", spec);
  }
  
  // invert the spectra back into volts

  _fft->inv(spec, volts);

}


///////////////////////////////
// print complex compoments

void WxSpecSim::_printComplex(ostream &out,
			  const string &heading,
			  const vector<Complex_t> &comp)
  
{
  
  out << "---->> " << heading << " <<----" << endl;
  out << setw(3) << "ii" << "  "
      << setw(10) << "re" << "  "
      << setw(10) << "im" << endl;
  for (int ii = 0; ii < _nSamples; ii++) {
    cout << setw(3) << ii << "  "
	 << setw(10) << comp[ii].re << "  "
	 << setw(10) << comp[ii].im << endl;
  }
  out.flush();

}

///////////////////////////////
// print complex vector

void WxSpecSim::_printVector(ostream &out,
			 const string &heading,
			 const vector<Complex_t> &comp)
  
{
  
  out << "---->> " << heading << " <<----" << endl;
  out << setw(3) << "ii" << "  "
      << setw(10) << "magnitude" << "  "
      << setw(10) << "angle" << endl;
  for (int ii = 0; ii < _nSamples; ii++) {
    double mag = comp[ii].re * comp[ii].re + comp[ii].im * comp[ii].im;
    double angle = atan2(comp[ii].im, comp[ii].re) * RAD_TO_DEG;
    cout << setw(3) << ii << "  "
	 << setw(10) << mag << "  "
	 << setw(10) << angle << endl;
  }
  out.flush();

}

///////////////////////////////////////////////
// compute time-domain power

double WxSpecSim::_computePower(const vector<Complex_t> &IQ)
  
{
  
  double p = 0.0;
  for (int ii = 0; ii < _nSamples; ii++) {
    p += ((IQ[ii].re * IQ[ii].re) + (IQ[ii].im * IQ[ii].im));
  }
  return p / _nSamples;
}

///////////////////////////////////////////////
// compute time-domain moments using pulse-pair

void WxSpecSim::_momentsByPp(const vector<Complex_t> &IQ,
                             double prtSecs,
                             double &power,
                             double &vel,
                             double &width)
  
{
  
  // compute a, b, p, r1
  
  double a = 0.0, b = 0.0, p = 0.0;
  
  {
    double re = IQ[0].re;
    double im = IQ[0].im;
    p += re * re + im * im;
  }
    
  for (int ii = 0; ii < _nSamples - 1; ii++) {
    double re0 = IQ[ii].re;
    double im0 = IQ[ii].im;
    double re1 = IQ[ii+1].re;
    double im1 = IQ[ii+1].im;
    a += re0 * re1 + im0 * im1;
    b += re0 * im1 - re1 * im0;
    p += re1 * re1 + im1 * im1;
  }
  double r1_val = sqrt(a * a + b * b) / _nSamples;
  
  // compute c, d, r2
  
  double c = 0.0, d = 0.0;
  
  for (int ii = 0; ii < _nSamples - 2; ii++) {
    double re0 = IQ[ii].re;
    double im0 = IQ[ii].im;
    double re2 = IQ[ii+2].re;
    double im2 = IQ[ii+2].im;
    c += re0 * re2 + im0 * im2;
    d += re0 * im2 - re2 * im0;
  }
  double r2_val = sqrt(c * c + d * d) / _nSamples;
  
  // mom0

  double mom0 = p / _nSamples;

  // mom1 from pulse-pair
  
  double nyquist = (_params.wavelength / 100.0) / (4.0 * prtSecs);
  double mom1_fac = nyquist / M_PI;
  double mom1_pp;
  if (a == 0.0 && b == 0.0) {
    mom1_pp = 0.0;
  } else {
    mom1_pp = mom1_fac * atan2(b, a);
  }
  
  // mom2 from pulse-pair

  double mom2_pp_fac = sqrt(2.0) * nyquist / M_PI;
  double s_hat = mom0 - 5.5e-7;
  if (s_hat < 1.0e-6) {
    s_hat = 1.0e-6;
  }
  double ln_ratio = log(s_hat / r1_val);
  double mom2_pp;
  if (ln_ratio > 0) {
    mom2_pp = mom2_pp_fac * sqrt(ln_ratio);
  } else {
    mom2_pp = -1.0 * mom2_pp_fac * sqrt(fabs(ln_ratio));
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

  if (_params.debug >= Params::DEBUG_VERBOSE) {
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
  width = mom2_pp;

}

///////////////////////////////
// compute spectral moments

void WxSpecSim::_momentsByFft(const vector<Complex_t> &IQ, double prtSecs,
                              double &power, double &vel, double &width)
  
{

  int kCent = _nSamples / 2;
  
  // compute fft
  
  vector<Complex_t> spectra;
  spectra.reserve(_nSamples);
  _fft->fwd(IQ, spectra);

  // compute magnitudes

  vector<double> magnitude;
  magnitude.reserve(_nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    double re = spectra[ii].re;
    double im = spectra[ii].im;
    magnitude[ii] = (re * re + im * im);
  }
  
  // find max magnitude

  double maxMag = 0.0;
  int kMax = 0;
  for (int ii = 0; ii < _nSamples; ii++) {
    double mp = magnitude[ii];
    if (mp > maxMag) {
      kMax = ii;
      maxMag = mp;
    }
  }
  if (kMax >= kCent) {
    kMax -= _nSamples;
  }

  // center magnitude array on the max value

  vector<double> magCentered;
  magCentered.reserve(_nSamples);
  int kOffset = kCent - kMax;
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = (ii + kOffset) % _nSamples;
    magCentered[jj] = magnitude[ii];
  }

  // compute noise

  double noise = _computeSpectralNoise(magCentered);

  // moving away from the peak, find the points in the spectrum
  // where the signal drops below the noise

  int kStart = kCent - 1;
  for (int ii = kStart; ii >= 0; ii--) {
    double mp = magCentered[ii];
    if (mp < noise) {
      break;
    }
    kStart = ii;
  }
  
  int kEnd = kCent + 1;
  for (int ii = kEnd; ii < _nSamples; ii++) {
    double mp = magCentered[ii];
    if (mp < noise) {
      break;
    }
    kEnd = ii;
  }

  // compute mom1 and mom2, using those points above the noise

  double sumPower = 0.0;
  double sumK = 0.0;
  double sumK2 = 0.0;
  for (int ii = kStart; ii <= kEnd; ii++) {
    double mp = magCentered[ii];
    double phase = (double) ii;
    double pExcess = mp - noise;
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

  double velFac = (_params.wavelength / 100.0) / (2.0 * _nSamples * prtSecs);

  power = _computePower(IQ);
  vel = -1.0 * velFac * (meanK - kOffset);
  width = velFac * sdevK;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  Spectra estimates:" << endl;
    cerr << "    kMax: " << kMax << endl;
    cerr << "    kOffset: " << kOffset << endl;
    cerr << "    noise: " << noise << endl;
    cerr << "    kStart: " << kStart << endl;
    cerr << "    kEnd: " << kEnd << endl;
    cerr << "    meanK: " << meanK << endl;
    cerr << "    sdevK: " << sdevK << endl;
    cerr << "    power: " << power << endl;
    cerr << "    vel: " << vel << endl;
    cerr << "    width: " << width << endl;
  }

}

/////////////////////////////////////////////////////
// compute noise for the spectral power

double WxSpecSim::_computeSpectralNoise(const vector<double> &magCentered)
  
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
  const double *m;
  
  // combine 1/8 from each end

  double sumBoth = 0.0;
  for (int ii = 0; ii < nby8; ii++) {
    double m = magCentered[ii];
    sumBoth += m;
  }
  int jj = _nSamples - nby8 - 1;
  for (int ii = 0; ii < nby8; ii++, jj++) {
    double m = magCentered[jj];
    sumBoth += m;
  }
  double meanBoth = sumBoth / (2.0 * nby8);

  // 1/4 from lower end

  double sumLower = 0.0;
  for (int ii = 0; ii < nby4; ii++, m++) {
    double m = magCentered[ii];
    sumLower += m;
  }
  double meanLower = sumLower / (double) nby4;
  
  // 1/4 from upper end
  
  double sumUpper = 0.0;
  jj = _nSamples - nby4 - 1;
  for (int ii = 0; ii < nby4; ii++, jj++) {
    double m = magCentered[jj];
    sumUpper += m;
  }
  double meanUpper = sumUpper / (double) nby4;
  
  if (meanBoth < meanLower && meanBoth < meanUpper) {
    return meanBoth;
  } else if (meanLower < meanUpper) {
    return meanLower;
  } else {
    return meanUpper;
  }

}

///////////////////////////////
// write spectra file

void WxSpecSim::_writeSpectraFile(const string &heading,
                                  const vector<Complex_t> &comp)
  
{
  
  char outPath[MAX_PATH_LEN];
  sprintf(outPath, "%s%s_gate%d_%s",
	  _params.spectra_output_dir, PATH_DELIM,
	  _gateNum, heading.c_str());
  
  ta_makedir_recurse(_params.spectra_output_dir);
  
  ofstream out(outPath);
  if (!out.is_open()) {
    cerr << "ERROR opening file: " << outPath << endl;
    return;
  }
				
  for (int ii = 0; ii < _nSamples; ii++) {
    double mag = comp[ii].re * comp[ii].re + comp[ii].im * comp[ii].im;
    double angle = atan2(comp[ii].im, comp[ii].re) * RAD_TO_DEG;
    out << setw(3) << ii << "  "
	<< setw(10) << mag << "  "
	<< setw(10) << angle << endl;
  }

  out.close();
  
}

