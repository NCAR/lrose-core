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
// ClutFilter simulates raw IQ data with clutter, and tests
// a filter for removing the clutter
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <ctime>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <rapmath/stats.h>
#include "ClutFilter.hh"
#include "Filter.hh"

// #define USE_GMAP true
#ifdef USE_GMAP
#include "gmap.h"
#endif

using namespace std;

// Constructor

ClutFilter::ClutFilter(int argc, char **argv)

{

  isOK = true;
  _fft = NULL;
  _fftLong = NULL;
  _results = NULL;
  _hanning = NULL;
  _blackman = NULL;
  
  // set programe name
  
  _progName = "ClutFilter";
  ucopyright((char *) _progName.c_str());

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check params

  if (_params.nsamples > _nSamplesLong) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  nsamples too large: " << _params.nsamples << endl;
    cerr << "  max nsamples: " << _nSamplesLong << endl;
    isOK = false;
    return;
  }

  // set variables

  _nSamples = _params.nsamples;
  _lambda = _params.wavelength / 100.0;
  _prtSecs = _params.prt / 1.0e6;
  _nyquist = _lambda / (4.0 * _prtSecs);
  _rNoise = _params.receiver_noise;
  
  if (_params.mode == Params::MODE_SINGLE) {
    _params.write_results_file = pFALSE;
  } else {
    _params.write_spectra_files = pFALSE;
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize random number generator
  
  // STATS_uniform_seed((int) time(NULL));
  STATS_uniform_seed(0);

  // set up FFT objects
  
  _fft = new Fft(_nSamples);
  _fftLong = new Fft(_nSamplesLong);

  // windows

  _hanning = new double[_nSamples];
  _blackman = new double[_nSamples];
  _initHanning(_hanning);
  _initBlackman(_blackman);

  // open results output file if required
  
  if (_params.write_results_file) {

    _resultsFilePath = _params.output_dir;
    _resultsFilePath += PATH_DELIM;
    _resultsFilePath += _params.output_file_base;
    _resultsFilePath += "_results.txt";
    
    if ((_results = fopen(_resultsFilePath.c_str(), "w")) == NULL) {
      int errNum = errno;
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Cannot open results file: " << endl;
      cerr << "  " << strerror(errNum) << endl;
      isOK = false;
    }
    
  }

  // read in spectra if required

  if (_params.spectra_creation_method == Params::RECONSTRUCTED) {
    if (_readFileSpectra(_params.weather_spectra_input_path,
			 100.0, 100.0,
			 _weatherSpectra)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Cannot read weather spectra file: "
	   << _params.weather_spectra_input_path << endl;
      isOK = false;
    }
    if (_readFileSpectra(_params.clutter_spectra_input_path,
			 _params.reconstructed_max_clutter_abs_vel,
 			 _params.reconstructed_max_clutter_width,
			 _clutterSpectra)) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Cannot read clutter spectra file: "
	   << _params.clutter_spectra_input_path << endl;
      isOK = false;
    }
  }

  return;

}

// destructor

ClutFilter::~ClutFilter()

{

  if (_results != NULL) {
    fclose(_results);
  }

  if (_fft) {
    delete _fft;
  }
  if (_fftLong) {
    delete _fftLong;
  }

  if (_hanning) {
    delete[] _hanning;
  }
  if (_blackman) {
    delete[] _blackman;
  }

  for (size_t ii = 0; ii < _weatherSpectra.size(); ii++) {
    delete _weatherSpectra[ii];
  }
  for (size_t ii = 0; ii < _clutterSpectra.size(); ii++) {
    delete _clutterSpectra[ii];
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ClutFilter::Run ()
{

  PMU_auto_register("Run");
  int iret = 0;

  if (_params.spectra_creation_method == Params::MODELLED) {
    if (_processModelledSpectra()) {
      iret = -1;
    }
  } else {
    if (_processReconstructedSpectra()) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// process modelled spectra

int ClutFilter::_processModelledSpectra()
{

  Complex_t weatherSpec[_nSamples];
  Complex_t clutterSpec[_nSamples];
  int count = 0;
  
  if (_params.mode == Params::MODE_SINGLE) {
    
    _modelSpectra(_params.single_moments.weather_dbm,
		  _params.single_moments.weather_vel,
		  _params.single_moments.weather_width,
		  _params.single_moments.cwr_dbm,
		  _params.single_moments.clutter_vel,
		  _params.single_moments.clutter_width,
		  weatherSpec, clutterSpec);
    
    _processCase(count, count, weatherSpec, clutterSpec);
    
  } else {
    
    double weatherDbm;
    double weatherVel;
    double weatherWidth;
    double cwrDbm;
    double clutterVel;
    double clutterWidth;

    for (weatherDbm = _params.mult_moments.weather_dbm_min;
	 weatherDbm <= _params.mult_moments.weather_dbm_max;
	 weatherDbm += _params.mult_moments.weather_dbm_delta) {

      for (weatherVel = _params.mult_moments.weather_vel_min;
	   weatherVel <= _params.mult_moments.weather_vel_max;
	   weatherVel += _params.mult_moments.weather_vel_delta) {

	for (weatherWidth = _params.mult_moments.weather_width_min;
	     weatherWidth <= _params.mult_moments.weather_width_max;
	     weatherWidth += _params.mult_moments.weather_width_delta) {
	  
	  for (cwrDbm = _params.mult_moments.cwr_dbm_min;
	       cwrDbm <= _params.mult_moments.cwr_dbm_max;
	       cwrDbm += _params.mult_moments.cwr_dbm_delta) {
	    
 	    for (clutterVel = _params.mult_moments.clutter_vel_min;
		 clutterVel <= _params.mult_moments.clutter_vel_max;
		 clutterVel += _params.mult_moments.clutter_vel_delta) {
	      
	      for (clutterWidth = _params.mult_moments.clutter_width_min;
		   clutterWidth <= _params.mult_moments.clutter_width_max;
		   clutterWidth += _params.mult_moments.clutter_width_delta) {

		_modelSpectra(weatherDbm, weatherVel, weatherWidth,
			      cwrDbm, clutterVel, clutterWidth,
			      weatherSpec, clutterSpec);
		
		_processCase(count, count, weatherSpec, clutterSpec);
		
		count++;
    
	      } // clutterWidth

	    } // clutterVel
		
	  } // cwrDbm
	  
	} // weatherwidth
	
      } // weatherVel

    } // weatherDbm

  }

  return 0;

}

//////////////////////////////////////////////////
// process reconstructed spectra

int ClutFilter::_processReconstructedSpectra()
{

  Complex_t weatherSpec[_nSamples];
  Complex_t clutterSpec[_nSamples];
  
  if (_params.mode == Params::MODE_SINGLE) {

    if (_retrieveSpectra(_params.weather_spectrum_index,
			 _params.clutter_spectrum_index,
			 weatherSpec, clutterSpec)) {
      return -1;
    }
    
    _processCase(_params.weather_spectrum_index,
		 _params.clutter_spectrum_index,
		 weatherSpec, clutterSpec);
    
  } else {

    for (int ii = 0; ii < _params.n_reconstructed; ii++) {
      
      int weatherIndex = (int) (STATS_uniform_gen() * _weatherSpectra.size());
      weatherIndex = MIN(weatherIndex, (int) (_weatherSpectra.size() - 1));
      
      int clutterIndex = (int) (STATS_uniform_gen() * _clutterSpectra.size());
      clutterIndex = MIN(clutterIndex, (int) (_clutterSpectra.size() - 1));
      
      if (_retrieveSpectra(weatherIndex, clutterIndex,
			   weatherSpec, clutterSpec)) {
	return -1;
      }
    
      _processCase(weatherIndex, clutterIndex,
		   weatherSpec, clutterSpec);

    } // ii

  }

  return 0;

}

//////////////////////////////////////////////////
// Process one case

void ClutFilter::_processCase(int weatherIndex,
			      int clutterIndex,
			      const Complex_t *weatherSpec,
			      const Complex_t *clutterSpec)

{
  
  PMU_auto_register("processCase");
  
  // create composite spectrum 

  Complex_t unfilteredIQ[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    unfilteredIQ[ii].re = weatherSpec[ii].re + clutterSpec[ii].re;
    unfilteredIQ[ii].im = weatherSpec[ii].im + clutterSpec[ii].im;
  }
  
  // compute magnitudes
  
  double unfilteredMag[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    unfilteredMag[ii] =
      sqrt(weatherSpec[ii].re * weatherSpec[ii].re +
	   weatherSpec[ii].im * weatherSpec[ii].im) +
      sqrt(clutterSpec[ii].re * clutterSpec[ii].re +
	   clutterSpec[ii].im * clutterSpec[ii].im);
  }
  
  // run filter
  
  double filteredMag[_nSamples];
  double powerRemoved;
  int clutterFound = 1;
  double fitVel = 0.0, fitWidth = 0.0;
  int notchStart = 0, notchEnd = 0;

#ifdef USE_GMAP
  
  {
    
    double clutterPower, clutterVel, clutterWidth;
    _momentsByFft(clutterSpec, _prtSecs,
		  clutterPower, clutterVel, clutterWidth);

    double normClutWidth = clutterWidth / (2.0 * _nyquist);
    // double normClutWidth = 0.01;
    
    powerRemoved =
      gmap(unfilteredMag, _nSamples, normClutWidth, 0.0, filteredMag);

  }

#else

  Filter filter(*this, _params.debug >= Params::DEBUG_VERBOSE);
  // Filter filter(*this, _params.debug);
  
  filter.run(unfilteredMag, _nSamples,
	     _params.max_abs_clutter_vel,
	     _params.initial_notch_width,
	     _nyquist,
	     filteredMag,
	     clutterFound,
	     notchStart, notchEnd,
	     powerRemoved, fitVel, fitWidth);

#endif

  // create filtered IQ

  Complex_t filteredIQ[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    double ratio = filteredMag[ii] / unfilteredMag[ii];
    filteredIQ[ii].re = unfilteredIQ[ii].re * ratio;
    filteredIQ[ii].im = unfilteredIQ[ii].im * ratio;
  }
  
  if (_params.debug ||
      _params.write_results_file ||
      _params.write_spectra_files ) {

    writeRealSpectraSq("unfiltered", unfilteredMag, _nSamples);
    writeRealSpectraSq("filtered", filteredMag, _nSamples);
    
    double weatherTruthPower[_nSamples];
    double clutterTruthPower[_nSamples];
    for (int ii = 0; ii < _nSamples; ii++) {
      weatherTruthPower[ii] =
	(weatherSpec[ii].re * weatherSpec[ii].re +
	 weatherSpec[ii].im * weatherSpec[ii].im);
      clutterTruthPower[ii] =
	(clutterSpec[ii].re * clutterSpec[ii].re +
	 clutterSpec[ii].im * clutterSpec[ii].im);
    }

    writeRealSpectra("wx_truth", weatherTruthPower, _nSamples);
    writeRealSpectra("clutter_truth", clutterTruthPower, _nSamples);
    
    double truthPower, truthVel, truthWidth;
    _momentsByFft(weatherSpec, _prtSecs, truthPower, truthVel, truthWidth);
    
    double clutterPower, clutterVel, clutterWidth;
    _momentsByFft(clutterSpec, _prtSecs,
		  clutterPower, clutterVel, clutterWidth);
    
    double unfilteredPower, unfilteredVel, unfilteredWidth;
    _momentsByFft(unfilteredIQ, _prtSecs,
		  unfilteredPower, unfilteredVel, unfilteredWidth);
    
    double filteredPower, filteredVel, filteredWidth;
    _momentsByFft(filteredIQ, _prtSecs,
		  filteredPower, filteredVel, filteredWidth);
    
    if (_params.write_results_file && clutterFound) {
      fprintf(_results,
	      "%d %d %g %g %g %g %g %g %g %g %g %g %g %g %g\n",
	      weatherIndex,
	      clutterIndex,
	      10.0 * log10(truthPower),
	      truthVel,
	      truthWidth,
	      10.0 * log10(clutterPower),
	      clutterVel,
	      clutterWidth,
	      10.0 * log10(unfilteredPower),
	      unfilteredVel,
	      unfilteredWidth,
	      10.0 * log10(filteredPower),
	      filteredVel,
	      filteredWidth,
	      powerRemoved);
    }

    double powerRatio = 1.0;
    if (truthPower > filteredPower) {
      powerRatio = truthPower / filteredPower;
    } else {
      powerRatio = filteredPower / truthPower;
    }

    double velDiff = 0.0;
    if (truthVel >= filteredVel) {
      velDiff = truthVel - filteredVel;
    } else {
      velDiff = filteredVel - truthVel;
    }
    if (velDiff > _nyquist) {
      velDiff = (2 * _nyquist) - velDiff;
    }
    bool doPrint = false;
    if (velDiff > (_nyquist / 2) || _params.mode == Params::MODE_SINGLE) {
      doPrint = true;
    }
    if (powerRatio > (clutterPower / unfilteredPower) * 10) {
      doPrint = true;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      doPrint = true;
    }
    
    if (_params.debug && doPrint) {
      cerr << "=========================================" << endl;
      cerr << "weatherIndex   : " << weatherIndex << endl;
      cerr << "clutterIndex   : " << clutterIndex << endl;
      cerr << "truthDbm       : " << 10.0 * log10(truthPower) << endl;
      cerr << "truthVel       : " << truthVel << endl;
      cerr << "truthWidth     : " << truthWidth << endl;
      cerr << "clutterDbm     : " << 10.0 * log10(clutterPower) << endl;
      cerr << "clutterVel     : " << clutterVel << endl;
      cerr << "clutterWidth   : " << clutterWidth << endl;
      cerr << "unfilteredDbm  : " << 10.0 * log10(unfilteredPower) << endl;
      cerr << "unfilteredVel  : " << unfilteredVel << endl;
      cerr << "unfilteredWidth: " << unfilteredWidth << endl;
      cerr << "filteredDbm    : " << 10.0 * log10(filteredPower) << endl;
      cerr << "filteredVel    : " << filteredVel << endl;
      cerr << "filteredWidth  : " << filteredWidth << endl;
      cerr << "powerRemoved   : " << powerRemoved << endl;
      cerr << "fitVel         : " << fitVel << endl;
      cerr << "fitWidth       : " << fitWidth << endl;
      cerr << "clutterFound   : " << clutterFound << endl;
      cerr << "notchStart     : " << notchStart << endl;
      cerr << "notchEnd       : " << notchEnd << endl;
    }

  }
  
}

////////////////////////////////////////
// model spectra of IQ data

void ClutFilter::_modelSpectra(double weatherDbm,
			       double weatherVel,
			       double weatherWidth,
			       double cwrDbm,
			       double clutterVel,
			       double clutterWidth,
			       Complex_t *weatherSpec,
			       Complex_t *clutterSpec)
  
{
  
  memset(weatherSpec, 0, sizeof(weatherSpec));
  memset(clutterSpec, 0, sizeof(clutterSpec));
  
  double weatherPower = pow(10.0, weatherDbm / 10.0);
  double clutterDbm = weatherDbm + cwrDbm;
  double clutterPower = pow(10.0, clutterDbm / 10.0);
  
  _createGaussianWindowed(weatherPower, weatherVel, weatherWidth, weatherSpec);
  _createGaussianWindowed(clutterPower, clutterVel, clutterWidth, clutterSpec);

}

////////////////////////////////////////
// retreive spectra from file data
//
// 

int ClutFilter::_retrieveSpectra(int weatherIndex,
				 int clutterIndex,
				 Complex_t *weatherSpec,
				 Complex_t *clutterSpec)
  
{

  if (weatherIndex >= (int) _weatherSpectra.size()) {
    cerr << "ERROR - ClutFilter::_retrieveSpectra" << endl;
    cerr << "  weatherIndex out of range: " << weatherIndex << endl;
    cerr << "  max allowable: " << (int) _weatherSpectra.size() - 1 << endl;
    return -1;
  }
  if (clutterIndex >= (int) _clutterSpectra.size()) {
    cerr << "ERROR - ClutFilter::_retrieveSpectra" << endl;
    cerr << "  clutterIndex out of range: " << clutterIndex << endl;
    cerr << "  max allowable: " << (int) _clutterSpectra.size() - 1 << endl;
    return -1;
  }

  memset(weatherSpec, 0, sizeof(weatherSpec));
  memset(clutterSpec, 0, sizeof(clutterSpec));
  
  const MeasuredSpec *wxSpec = _weatherSpectra[weatherIndex];
  const Complex_t *wxIq = wxSpec->getIQ();

  const MeasuredSpec *clutSpec = _clutterSpectra[clutterIndex];
  const Complex_t *clutIq = clutSpec->getIQ();

  // apply the window

  Complex_t wxWindowed[_nSamples];
  Complex_t clutWindowed[_nSamples];
  if (_params.window == Params::WINDOW_NONE) {
    memcpy(wxWindowed, wxIq, sizeof(wxWindowed));
    memcpy(clutWindowed, clutIq, sizeof(clutWindowed));
  } else if (_params.window == Params::WINDOW_HANNING) {
    _applyWindow(_hanning, wxIq, wxWindowed);
    _applyWindow(_hanning, clutIq, clutWindowed);
  } else if (_params.window == Params::WINDOW_BLACKMAN) {
    _applyWindow(_blackman, wxIq, wxWindowed);
    _applyWindow(_blackman, clutIq, clutWindowed);
  }

  // compute ffts to get spectra

  Complex_t tmpWx[_nSamples];
  _fft->fwd(wxWindowed, tmpWx);
  _fft->fwd(clutWindowed, clutterSpec);

  // compute powers, adjust to get correct cwr

  double wxPower = _computePower(tmpWx);
  double clutPower = _computePower(clutterSpec);
  double cwr = clutPower / wxPower;
  double desiredCwr = pow(10.0, _params.reconstructed_cwr / 10.0);
  double correction = sqrt(desiredCwr / cwr);
  for (int ii = 0; ii < _nSamples; ii++) {
    clutterSpec[ii].re *= correction;
    clutterSpec[ii].im *= correction;
  }

  // randomly shift the weather velocity

  int shift = weatherIndex + clutterIndex;
  for (int ii = 0; ii < _nSamples; ii++) {
    int jj = (ii + shift + _nSamples) % _nSamples;
    weatherSpec[ii].re = tmpWx[jj].re;
    weatherSpec[ii].im = tmpWx[jj].im;
  }

  return 0;

}

////////////////////////////////////////
// create gaussian spectrum

void ClutFilter::_createGaussian(double power,
				 double vel,
				 double width,
				 Complex_t *spec)
  
{

  double fvmean = (2.0 * vel) / _lambda;
  double fvsigma = (2.0 * width) / _lambda;

  double fmin = -0.5 / _prtSecs;
  double fmax = 0.5 / _prtSecs;
  double fdelta = (fmax - fmin) / _nSamples;

  double C1 = power / _prtSecs;
  double C2 = 1.0 / (sqrt(2.0 * M_PI) * fvsigma);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "power: " << power << endl;
    cerr << "vel: " << vel << endl;
    cerr << "width: " << width << endl;
    cerr << "lambda: " << _lambda << endl;
    cerr << "prtSecs: " << _prtSecs << endl;
    cerr << "nyquist: " << _nyquist << endl;
    cerr << "_rNoise: " << _rNoise << endl;
    cerr << "fvmean: " << fvmean << endl;
    cerr << "fvsigma: " << fvsigma << endl;
    cerr << "fmin: " << fmin << endl;
    cerr << "fmax: " << fmax << endl;
    cerr << "fdelta: " << fdelta << endl;
    cerr << "C1: " << C1 << endl;
    cerr << "C2: " << C2 << endl;
  }

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

    double sig = C2 * exp(-xx) + _rNoise * _prtSecs;
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
    double pwr = mag * mag;
    sumPower += pwr;
    double vv = -_nyquist + ((double) k / (double) _nSamples) * _nyquist * 2.0;
    sumVel += vv * pwr;
    sumsqVel += vv * vv * pwr;

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
  }
  
}

////////////////////////////////////////
// create windowed gaussian spectrum
//
// First create a long spectrum, invert into time domain,
// sample a smaller part of the spectrum, window it
// and take fft.

void ClutFilter::_createGaussianWindowed(double power,
					 double vel,
					 double width,
					 Complex_t *spec)
  
{
  
  double fvmean = (2.0 * vel) / _lambda;
  double fvsigma = (2.0 * width) / _lambda;

  double fmin = -0.5 / _prtSecs;
  double fmax = 0.5 / _prtSecs;
  double fdelta = (fmax - fmin) / _nSamplesLong;
  
  double C1 = power / _prtSecs;
  double C2 = 1.0 / (sqrt(2.0 * M_PI) * fvsigma);

  // create a long spectrum
  
  Complex_t specLong[_nSamplesLong];
  for (int k = 0; k < _nSamplesLong; k++) {
    specLong[k].re = 0.0;
    specLong[k].im = 0.0;
  }

  double nn = 0.0;
  double sumPower = 0.0;
  double sumVel = 0.0;
  double sumsqVel = 0.0;

  for (int k = -_nSamplesLong * 2; k < 3 * _nSamplesLong; k++) {
    
    double f = fmin + k * fdelta;
    double dd = f - fvmean;
    double xx = (dd * dd) / (2.0 * fvsigma * fvsigma);
    
    double sig = C2 * exp(-xx) + _rNoise * _prtSecs;
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

    int kk = (k + _nSamplesLong/2 + 2 * _nSamplesLong) % _nSamplesLong;
    
    specLong[kk].re += mag * cos(phase);
    specLong[kk].im += mag * sin(phase);

    // sum up for checking properties

    nn++;
    double power = mag * mag;
    sumPower += power;
    double vv = -_nyquist +
      ((double) k / (double) _nSamplesLong) * _nyquist * 2.0;
    sumVel += vv * power;
    sumsqVel += vv * vv * power;
    
  } // k

  // scale to correct power

  double genPower = sumPower / _nSamplesLong;
  double mult = sqrt(power / genPower);
  for (int k = 0; k < _nSamplesLong; k++) {
    specLong[k].re *= mult;
    specLong[k].im *= mult;
  }

  // invert
  
  Complex_t iqLong[_nSamplesLong];
  _fftLong->inv(specLong, iqLong);

  // sample _nSamples from long iq into short iq

  Complex_t iq[_nSamples];
  int start = (_nSamplesLong / 2) - (_nSamples / 2);
  int end = start + _nSamples;
  if (end > _nSamplesLong) {
    start = _nSamplesLong - _nSamples;
  }
  if (start < 0) {
    start = 0;
  }
  memcpy(iq, iqLong + start, _nSamples * sizeof(Complex_t));

  // apply the window

  Complex_t windowed[_nSamples];
  if (_params.window == Params::WINDOW_NONE) {
    memcpy(windowed, iq, sizeof(windowed));
  } else if (_params.window == Params::WINDOW_HANNING) {
    _applyWindow(_hanning, iq, windowed);
  } else if (_params.window == Params::WINDOW_BLACKMAN) {
    _applyWindow(_blackman, iq, windowed);
  }

  // forward FFT into final spectrum
  
  _fft->fwd(windowed, spec);

}

///////////////////////////////////////////////
// compute time-domain power

double ClutFilter::_computePower(const Complex_t *IQ)
  
{
  
  double p = 0.0;
  const Complex_t *iq0 = IQ;
  for (int i = 0; i < _nSamples; i++, iq0++) {
    p += ((iq0->re * iq0->re) + (iq0->im * iq0->im));
  }
  return p / _nSamples;
}

///////////////////////////////
// compute spectral moments

void ClutFilter::_momentsByFft(const Complex_t *spec, double prtSecs,
			       double &power, double &vel, double &width)
  
{

  int kCent = _nSamples / 2;
  
  // compute magnitudes

  double magnitude[_nSamples];
  const Complex_t *spp = spec;
  double *mp = magnitude;
  for (int ii = 0; ii < _nSamples; ii++, spp++, mp++) {
    *mp = (spp->re * spp->re + spp->im * spp->im);
  }
  
  // divide spectrum into 8 parts, compute mean mag in each part
  
  int nEighth = ((_nSamples - 1) / 8) + 1;
  if (nEighth < 3) {
    nEighth = 3;
  }
  int nSixteenth = nEighth / 2;
  double blockMeans[8];
  for (int ii = 0; ii < 8; ii++) {
    int jjStart = ((ii * _nSamples) / 8) - nSixteenth;
    blockMeans[ii] = 0.0;
    for (int jj = jjStart; jj < jjStart + nEighth; jj++) {
      int kk = (jj + _nSamples) % _nSamples;
      blockMeans[ii] += magnitude[kk] / 8;
    }
  }
  
  // find block with the most power
  
  int maxBlock = 0;
  double maxMean = 0.0;
  for (int ii = 0; ii < 8; ii++) {
    if (maxMean < blockMeans[ii]) {
      maxMean = blockMeans[ii];
      maxBlock = ii;
    }
  }

  // get the max mag

  double maxMag = 0.0;
  int kMax = 0;
  int jjStart = ((maxBlock * _nSamples) / 8) - nSixteenth;
  for (int jj = jjStart; jj < jjStart + nEighth; jj++) {
    int kk = (jj + _nSamples) % _nSamples;
    if (maxMag < magnitude[kk]) {
      maxMag = magnitude[kk];
      kMax = kk;
    }
  } // jj

  if (kMax >= kCent) {
    kMax -= _nSamples;
  }

  // center magnitude array on the max value

  double magCentered[_nSamples];
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
  mp = magCentered + kStart;
  for (int ii = kStart; ii >= 0; ii--, mp--) {
    if (*mp < noise) {
      break;
    }
    kStart = ii;
  }

  int kEnd = kCent + 1;
  mp = magCentered + kEnd;
  for (int ii = kEnd; ii < _nSamples; ii++, mp++) {
    if (*mp < noise) {
      break;
    }
    kEnd = ii;
  }

  // compute mom1 and mom2, using those points above the noise

  double sumPower = 0.0;
  double sumK = 0.0;
  double sumK2 = 0.0;
  mp = magCentered + kStart;
  for (int ii = kStart; ii <= kEnd; ii++, mp++) {
    double phase = (double) ii;
    double pExcess = *mp - noise;
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

  power = _computePower(spec);
  vel = velFac * (meanK - kOffset);
  width = velFac * sdevK;

  // if (_params.debug >= Params::DEBUG_VERBOSE) {
  //   cerr << "  Spectra estimates:" << endl;
  //   cerr << "    kMax: " << kMax << endl;
  //   cerr << "    kOffset: " << kOffset << endl;
  //   cerr << "    noise: " << noise << endl;
  //   cerr << "    kStart: " << kStart << endl;
  //   cerr << "    kEnd: " << kEnd << endl;
  //   cerr << "    meanK: " << meanK << endl;
  //   cerr << "    sdevK: " << sdevK << endl;
  //   cerr << "    power: " << power << endl;
  //   cerr << "    vel: " << vel << endl;
  //   cerr << "    width: " << width << endl;
  // }
  
}

///////////////////////////////////
// initialize windowing functions

void ClutFilter::_initHanning(double *window)

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
  
void ClutFilter::_initBlackman(double *window)

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

void ClutFilter::_applyWindow(const double *window,
			      const Complex_t *in,
			      Complex_t *out)
  
{
  
  const double *ww = window;
  const Complex_t *inp = in;
  Complex_t *outp = out;
  
  for (int ii = 0; ii < _nSamples; ii++, ww++, inp++, outp++) {
    outp->re = inp->re * *ww;
    outp->im = inp->im * *ww;
  }

}
  
/////////////////////////////////////////////////////
// compute noise for the spectral power

double ClutFilter::_computeSpectralNoise(const double *magCentered)
  
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
  m = magCentered;
  for (int ii = 0; ii < nby8; ii++, m++) {
    sumBoth += *m;
  }
  m = magCentered + _nSamples - nby8 - 1;
  for (int ii = 0; ii < nby8; ii++, m++) {
    sumBoth += *m;
  }
  double meanBoth = sumBoth / (2.0 * nby8);

  // 1/4 from lower end

  double sumLower = 0.0;
  m = magCentered;
  for (int ii = 0; ii < nby4; ii++, m++) {
    sumLower += *m;
  }
  double meanLower = sumLower / (double) nby4;
  
  // 1/4 from upper end
  
  double sumUpper = 0.0;
  m = magCentered + _nSamples - nby4 - 1;
  for (int ii = 0; ii < nby4; ii++, m++) {
    sumUpper += *m;
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

/////////////////////////////////
// write complex spectra to file

int ClutFilter::writeComplexSpectra(const string &name,
				    const Complex_t *spec,
				    int nPoints,
                                    bool vel,
				    bool swap)
  
{
  
  if (!_params.write_spectra_files) {
    return 0;
  }

  // ensure directory exists

  if (ta_makedir_recurse(_params.output_dir)) {
    cerr << "ERROR - ClutFilter::Run" << endl;
    cerr << "  Cannot make output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errno) << endl;
    return -1;
  }

  // compute path

  string outBase = _params.output_dir;
  outBase += PATH_DELIM;
  outBase += _params.output_file_base;
  outBase += "_";
  string path = outBase + name + ".complex";

  // open file

  ofstream out(path.c_str());
  if (!out.is_open()) {
    cerr << "ERROR opening file: " << path << endl;
    return -1;
  }

  // write out

  for (int ii = 0; ii < nPoints; ii++) {
    int jj = ii;
    if (swap) {
      jj = (ii + nPoints / 2) % nPoints;
    }
    double re = spec[jj].re;
    double im = spec[jj].im;
    double mag = sqrt(re * re + im * im);
    double angle = atan2(im, re) * RAD_TO_DEG;
    double xx = ii;
    if (vel) {
      xx = (ii - nPoints / 2) * (_nyquist / (nPoints / 2.0));
    }
    out << setw(3) << xx << "  "
	<< setw(10) << mag << "  "
	<< setw(10) << angle << "  "
	<< setw(10) << re << "  "
	<< setw(10) << im << endl;
  }

  out.close();
  return 0;
  
}

///////////////////////////////
// write real spectra file

int ClutFilter::writeRealSpectra(const string &name,
				 const double *spec,
				 int nPoints,
                                 bool vel,
				 bool swap)
  
{

  if (!_params.write_spectra_files) {
    return 0;
  }

  // ensure directory exists

  if (ta_makedir_recurse(_params.output_dir)) {
    cerr << "ERROR - ClutFilter::Run" << endl;
    cerr << "  Cannot make output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errno) << endl;
    return -1;
  }

  // compute path

  string outBase = _params.output_dir;
  outBase += PATH_DELIM;
  outBase += _params.output_file_base;
  outBase += "_";
  string path = outBase + name + ".real";

  // open path

  ofstream out(path.c_str());
  if (!out.is_open()) {
    cerr << "ERROR opening file: " << path << endl;
    return -1;
  }

  // write out

  for (int ii = 0; ii < nPoints; ii++) {
    int jj = ii;
    if (swap) {
      jj = (ii + nPoints / 2) % nPoints;
    }
    double xx = ii;
    if (vel) {
      xx = (ii - nPoints / 2) * (_nyquist / (nPoints / 2.0));
    }
    out << setw(3) << xx << "  "
	<< setw(10) << spec[jj] << endl;
  }

  out.close();
  return 0;
  
}

////////////////////////////////////
// write real spectra squared to file

int ClutFilter::writeRealSpectraSq(const string &name,
				   const double *spec,
				   int nPoints,
                                   bool vel,
				   bool swap)
  
{

  double power[nPoints];
  for (int ii = 0; ii < nPoints; ii++) {
    power[ii] = spec[ii] * spec[ii];
  }

  return writeRealSpectra(name, power, nPoints, vel, swap);

}

///////////////////////////////
// read spectra

int ClutFilter::_readFileSpectra(const char *path,
				 double maxAbsVel,
				 double maxWidth,
				 vector<MeasuredSpec *> &spectra)
  
{
  
  // open file

  FILE *in;
  if ((in = fopen(path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ClutFilter::_readFileSpectra" << endl;
    cerr << "  Cannot open spectral file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  while (!feof(in)) {

    // create new spectrum

    MeasuredSpec *spec = new MeasuredSpec();
    
    // set it's state from the file

    if (spec->read(in)) {
      delete spec;
      continue;
    }

    // check the attributes

    if (fabs(spec->getVel()) > maxAbsVel ||
	spec->getWidth() > maxWidth) {
      delete spec;
      continue;
    }

    // add to vector

    spectra.push_back(spec);
    
  }

  fclose(in);

  // check size

  if (_params.debug) {
    cerr << "Spectra file: " << path << " has "
	 << spectra.size() << " suitable spectra" << endl;
  }
    
  if (spectra.size() < 1) {
    cerr << "ERROR - ClutFilter::_readFileSpectra" << endl;
    cerr << "  No spectra found" << endl;
    return -1;
  }
  
  return 0;

}

