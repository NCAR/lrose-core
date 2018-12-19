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
// RvSim.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2003
//
///////////////////////////////////////////////////////////////
//
// RvSim simulates overlaid trips in raw IQ data and writes
// to a netCDF file.
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
#include <didss/LdataInfo.hh>
#include <rapmath/stats.h>
#include "RvSim.hh"

using namespace std;

// Constructor

RvSim::RvSim(int argc, char **argv)

{

  isOK = true;
  _fft = NULL;
  _beamNum = 0;
  _gateNum = 0;
  
  // set programe name

  _progName = "RvSim";
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

  // set variables

  _nSamples = _params.nsamples;
  if (_params.data_mode == Params::RANDOM_DATA) {
    _nBeams = _params.nbeams;
    _nGates = _params.ngates;
  } else {
    _nBeams = 1;
    _nGates = _params.gate_data_n;
  }
  
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize random number generator

  // STATS_uniform_seed((int) time(NULL));
  STATS_uniform_seed(0);

  // set up FFT object

  _fft = new Fft(_nSamples, _params.debug,
		 _params.data_mode == Params::RANDOM_DATA);

  // initialize the phase codes

  _initPhaseCodes();

  return;

}

// destructor

RvSim::~RvSim()

{

  if (_fft) {
    delete _fft;
  }

  // unregister process

  PMU_auto_unregister();

}

/////////////////////////////////////////////////////
// initialize the phase codes

void RvSim::_initPhaseCodes()
  
{
  
  double ratio = (double) _phaseCodeN / (double) _phaseCodeM;
  double angle = 0.0;
  
  for (int ii = 0; ii < _phaseCodeM; ii++) {
    
    double code = (double) ii * (double) ii * ratio;
    double deltaAngle = code * M_PI;
    if (_params.negate_phase_codes) {
      deltaAngle *= -1.0;
    }
    angle += deltaAngle;

    _phaseCode[ii].re = cos(angle);
    _phaseCode[ii].im = sin(angle);
    
  }

}

//////////////////////////////////////////////////
// Run

int RvSim::Run ()
{

  PMU_auto_register("Run");
  
  // create the data
  // one time series will be added, a gate at a time

  for (int ifile = 0; ifile < _params.n_files; ifile++) {

    vector<iq_t> gateIQ;
    vector<moments_t> trip1Moments, trip2Moments;
    _createSimData(gateIQ, trip1Moments, trip2Moments);
    
    // reorder the data in the vector so that it is pulse-ordered
    
    vector<iq_t> pulseIQ;
    for (int ibeam = 0; ibeam < _nBeams; ibeam++) {
      int beamOffset = ibeam * _nGates * _nSamples;
      for (int isample = 0; isample < _nSamples; isample++) {
	for (int igate = 0; igate < _nGates; igate++) {
	  int offset = beamOffset + igate * _nSamples + isample;
	  pulseIQ.push_back(gateIQ[offset]);
	}
      }
    } // ibeam
    
    // compute output and tmp paths
    
    string outPath = _params.output_dir;
    outPath += PATH_DELIM;
    outPath += _params.output_file_name;
    if (_params.n_files > 1) {
      char numstr[32];
      sprintf(numstr, "_%.3d", ifile);
      outPath += numstr;
    }
    outPath += ".nc";
    string tmpPath = outPath;
    tmpPath += ".tmp";

    cout << "Writing output file: " << outPath << endl;
    
    // write out tmp file
    
    if (_writeTmpFile(tmpPath, pulseIQ, trip1Moments, trip2Moments)) {
      cerr << "ERROR - RvSim::_processFile" << endl;
      cerr << "  Cannot write tmp file: " << tmpPath << endl;
      return -1;
    }

    cout << "  Wrote tmp file: " << tmpPath << endl;
    
    // move the tmp file to final name
    
    if (rename(tmpPath.c_str(), outPath.c_str())) {
      int errNum = errno;
      cerr << "ERROR - RvSim::_processFile" << endl;
      cerr << "  Cannot rename file: " << tmpPath << endl;
      cerr << "             to file: " << outPath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    
    cout << "  Renamed file: " << tmpPath << endl;
    cout << "       to file: " << outPath << endl;
    cout.flush();
    
    // write latest data info
    
    if (_params.write_ldata_info_file) {
      LdataInfo ldata(_params.output_dir);
      ldata.setDataFileExt("nc");
      ldata.setWriter("RvSim");
      ldata.setRelDataPath(_params.output_file_name);
      int now = time(NULL);
      if (ldata.write(now)) {
	cerr << "ERROR - RvSim::_processFile" << endl;
	cerr << "  Cannot write ldata file to dir: "
	     << _params.output_dir << endl;
	return -1;
      }
    }

  } // ifile

  return 0;

}

////////////////////////////////////////
// create IQ data

void RvSim::_createSimData(vector<iq_t> &gateIQ,
			   vector<moments_t> &trip1Moments,
			   vector<moments_t> &trip2Moments)
  
{
  
  if (_params.data_mode == Params::SPECIFY_DATA) {
    
    for (int igate = 0; igate < _nGates; igate++) {
      
      _gateNum = igate;
      _beamNum = 0;

      moments_t trip1, trip2;
      trip1.dbm = _params._gate_data[igate].trip1_dbm;
      trip1.vel = _params._gate_data[igate].trip1_vel;
      trip1.width = _params._gate_data[igate].trip1_width;
      trip2.dbm = _params._gate_data[igate].trip2_dbm;
      trip2.vel = _params._gate_data[igate].trip2_vel;
      trip2.width = _params._gate_data[igate].trip2_width;

      trip1.power = pow(10.0, trip1.dbm / 10.0);
      trip2.power = pow(10.0, trip2.dbm / 10.0);
      
      _createGateData(trip1, trip2, gateIQ, trip1Moments, trip2Moments);

    } // igate

  } else {
    
    // random data

    double dbm1Low = _params.random_dbm1_low;
    double dbm1Range = _params.random_dbm1_high - _params.random_dbm1_low;

    double dbmDiffLow = _params.random_dbm_diff_low;
    double dbmDiffRange =
      _params.random_dbm_diff_high - _params.random_dbm_diff_low;

    double vel1Low = _params.random_vel1_low;
    double vel1Range = _params.random_vel1_high - _params.random_vel1_low;

    double vel2Low = _params.random_vel2_low;
    double vel2Range = _params.random_vel2_high - _params.random_vel2_low;

    double width1Low = _params.random_width1_low;
    double width1Range = _params.random_width1_high - _params.random_width1_low;

    double width2Low = _params.random_width2_low;
    double width2Range = _params.random_width2_high - _params.random_width2_low;

    for (int ibeam = 0; ibeam < _nBeams; ibeam++) {

      for (int igate = 0; igate < _nGates; igate++) {

	_gateNum = igate;
	_beamNum = ibeam;

	moments_t trip1, trip2;
	trip1.dbm = dbm1Low + STATS_uniform_gen() * dbm1Range;
	trip1.vel = vel1Low + STATS_uniform_gen() * vel1Range;
	trip1.width = width1Low + STATS_uniform_gen() * width1Range;

	double dbmDiff = dbmDiffLow + STATS_uniform_gen() * dbmDiffRange;
	trip2.dbm = trip1.dbm - dbmDiff;
	trip2.vel = vel2Low + STATS_uniform_gen() * vel2Range;
	trip2.width = width2Low + STATS_uniform_gen() * width2Range;

	trip1.power = pow(10.0, trip1.dbm / 10.0);
	trip2.power = pow(10.0, trip2.dbm / 10.0);

	_createGateData(trip1, trip2, gateIQ, trip1Moments, trip2Moments);

      } // igate

    } // ibeam

  }
    
}

////////////////////////////////////////
// create IQ data

void RvSim::_createGateData(const moments_t &trip1Mom,
			    const moments_t &trip2Mom,
			    vector<iq_t> &gateIQ,
			    vector<moments_t> &trip1Moments,
			    vector<moments_t> &trip2Moments)
  
{
  
  if (_params.debug) {
    cerr << "---> Simulated signal <---" << endl;
    cerr << "     trip1 dbm, vel, width: "
	 << trip1Mom.dbm << ", "
	 << trip1Mom.vel << ", " << trip1Mom.width << endl;
    cerr << "     trip2 dbm, vel, width: "
	 << trip2Mom.dbm << ", "
	 << trip2Mom.vel << ", " << trip2Mom.width << endl;
  }
  
  Complex_t trip1Td[_nSamples];
  Complex_t trip2Td[_nSamples];
  memset(trip1Td, 0, sizeof(trip1Td));
  memset(trip2Td, 0, sizeof(trip2Td));
  
  _createGaussian(trip1Mom.power, -trip1Mom.vel, trip1Mom.width, trip1Td);
  _createGaussian(trip2Mom.power, -trip2Mom.vel, trip2Mom.width, trip2Td);
  
  if (_params.truth_method == Params::THEORETICAL_TRUTH) {

    trip1Moments.push_back(trip1Mom);
    trip2Moments.push_back(trip2Mom);

  } else if (_params.truth_method == Params::FFT_TRUTH) {

    moments_t trip1Est, trip2Est;
    _momentsByFft(trip1Td, _params.prt / 1000000.0,
		  trip1Est.power, trip1Est.vel, trip1Est.width);
    _momentsByFft(trip2Td, _params.prt / 1000000.0,
		  trip2Est.power, trip2Est.vel, trip2Est.width);
    trip1Est.dbm = 10.0 * log10(trip1Est.power);
    trip2Est.dbm = 10.0 * log10(trip2Est.power);
    trip1Moments.push_back(trip1Est);
    trip2Moments.push_back(trip2Est);

    if (_params.debug) {
      cerr << "FFT truth" << endl;
      cerr << "     trip1 dbm, vel, width: "
	   << trip1Est.dbm << ", "
	   << trip1Est.vel << ", " << trip1Est.width << endl;
      cerr << "     trip2 dbm, vel, width: "
	   << trip2Est.dbm << ", "
	   << trip2Est.vel << ", " << trip2Est.width << endl;
    }
  
  } else if (_params.truth_method == Params::PP_TRUTH) {

    moments_t trip1Est, trip2Est;
    _momentsByPp(trip1Td, _params.prt / 1000000.0,
		 trip1Est.power, trip1Est.vel, trip1Est.width);
    _momentsByPp(trip2Td, _params.prt / 1000000.0,
		 trip2Est.power, trip2Est.vel, trip2Est.width);
    trip1Est.dbm = 10.0 * log10(trip1Est.power);
    trip2Est.dbm = 10.0 * log10(trip2Est.power);
    trip1Moments.push_back(trip1Est);
    trip2Moments.push_back(trip2Est);

    if (_params.debug) {
      cerr << "PP truth" << endl;
      cerr << "     trip1 dbm, vel, width: "
	   << trip1Est.dbm << ", "
	   << trip1Est.vel << ", " << trip1Est.width << endl;
      cerr << "     trip2 dbm, vel, width: "
	   << trip2Est.dbm << ", "
	   << trip2Est.vel << ", " << trip2Est.width << endl;
    }
  
  }

  // code up the trips
  
  Complex_t trip1Encoded[_nSamples];
  Complex_t trip2Encoded[_nSamples];
  _encodeTrip(1, trip1Td, trip1Encoded);
  _encodeTrip(2, trip2Td, trip2Encoded);
  
  // combine the trips
  
  Complex_t combined[_nSamples];
  Complex_t combinedCoded[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    combined[ii].re = trip1Td[ii].re + trip2Td[ii].re;
    combined[ii].im = trip1Td[ii].im + trip2Td[ii].im;
    combinedCoded[ii].re = trip1Encoded[ii].re + trip2Encoded[ii].re;
    combinedCoded[ii].im = trip1Encoded[ii].im + trip2Encoded[ii].im;
  }

  // cohere combined to trip1

  Complex_t coheredTrip1[_nSamples];
  _cohere2Trip1(combinedCoded, coheredTrip1);
  
  if (_params.write_spectra_files) {
    
    Complex_t specTrip1[_nSamples];
    _fft->fwd(trip1Td, specTrip1);
    _writeSpectraFile("sim_trip1", specTrip1);

    Complex_t specTrip2[_nSamples];
    _fft->fwd(trip2Td, specTrip2);
    _writeSpectraFile("sim_trip2", specTrip2);

    Complex_t specCombined[_nSamples];
    _fft->fwd(combined, specCombined);
    _writeSpectraFile("sim_combined", specCombined);

    Complex_t specCombinedCoded[_nSamples];
    _fft->fwd(combinedCoded, specCombinedCoded);
    _writeSpectraFile("sim_combined_coded", specCombinedCoded);
    
    Complex_t specCoheredTrip1[_nSamples];
    _fft->fwd(coheredTrip1, specCoheredTrip1);
    _writeSpectraFile("cohered_trip1", specCoheredTrip1);
    
  }
  
  // add combined coded IQ to vector
  
  iq_t iq;
  for (int ii = 0; ii < _nSamples; ii++) {
    iq.i = combinedCoded[ii].re;
    iq.q = combinedCoded[ii].im;
    gateIQ.push_back(iq);
  }
  
}

////////////////////////////////////////
// create gaussian spectrum

void RvSim::_createGaussian(double power,
			    double vel,
			    double width,
			    Complex_t *volts)

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

  Complex_t spec[_nSamples];

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


////////////////////////////////////////
// encode a trip

void RvSim::_encodeTrip(int tripNum,
			const Complex_t *trip,
			Complex_t *tripEncoded)

{

  if (_params.apply_phase_codes) {
    for (int ii = 0; ii < _nSamples; ii++) {
      int pIndex = (ii + _params.trip1_phase_index - (tripNum - 1) + _nSamples) % _nSamples;
      tripEncoded[ii].re = (trip[ii].re * _phaseCode[pIndex].re -
			    trip[ii].im * _phaseCode[pIndex].im);
      tripEncoded[ii].im = (trip[ii].re * _phaseCode[pIndex].im +
			    trip[ii].im * _phaseCode[pIndex].re);
    }
  } else {
    for (int ii = 0; ii < _nSamples; ii++) {
      tripEncoded[ii].re = trip[ii].re;
      tripEncoded[ii].im = trip[ii].im;
    }
  }

}
  
///////////////////////////////
// cohere to given trip
//
// beamCode runs from [-4 to 63].
// Therefore trip_num can vary from 1 to 4.

void RvSim::_cohere2Trip1(const Complex_t *IQ,
			  Complex_t *trip1)
  
{

  // to cohere to the given trip, we need to subtract the
  // transmit phase from the received i/q data. This is
  // done by multiplying the i/q value by the complex conjugate
  // of the phase code

  const Complex_t *code = _phaseCode;
  
  for (int ii = 0; ii < _nSamples; ii++, IQ++, trip1++, code++) {
    trip1->re = (IQ->re * code->re) + (IQ->im * code->im);
    trip1->im = (IQ->im * code->re) - (IQ->re * code->im);
  }

}

///////////////////////////////
// print complex compoments

void RvSim::_printComplex(ostream &out,
			  const string &heading,
			  const Complex_t *comp)
  
{
  
  out << "---->> " << heading << " <<----" << endl;
  out << setw(3) << "ii" << "  "
      << setw(10) << "re" << "  "
      << setw(10) << "im" << endl;
  for (int ii = 0; ii < _nSamples; ii++, comp++) {
    cout << setw(3) << ii << "  "
	 << setw(10) << comp->re << "  "
	 << setw(10) << comp->im << endl;
  }
  out.flush();

}

///////////////////////////////
// print complex vector

void RvSim::_printVector(ostream &out,
			 const string &heading,
			 const Complex_t *comp)
  
{
  
  out << "---->> " << heading << " <<----" << endl;
  out << setw(3) << "ii" << "  "
      << setw(10) << "magnitude" << "  "
      << setw(10) << "angle" << endl;
  for (int ii = 0; ii < _nSamples; ii++, comp++) {
    double mag = comp->re * comp->re + comp->im * comp->im;
    double angle = atan2(comp->im, comp->re) * RAD_TO_DEG;
    cout << setw(3) << ii << "  "
	 << setw(10) << mag << "  "
	 << setw(10) << angle << endl;
  }
  out.flush();

}

///////////////////////////////////////////////
// compute time-domain power

double RvSim::_computePower(const Complex_t *IQ)
  
{
  
  double p = 0.0;
  const Complex_t *iq0 = IQ;
  for (int i = 0; i < _nSamples; i++, iq0++) {
    p += ((iq0->re * iq0->re) + (iq0->im * iq0->im));
  }
  return p / _nSamples;
}

///////////////////////////////////////////////
// compute time-domain moments using pulse-pair

void RvSim::_momentsByPp(const Complex_t *IQ, double prtSecs,
			 double &power, double &vel, double &width)
  
{
  
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
  
  // compute c, d, r2
  
  double c = 0.0, d = 0.0;
  
  iq0 = IQ;
  const Complex_t *iq2 = IQ + 2;

  for (int i = 0; i < _nSamples - 2; i++, iq0++, iq2++) {
    c += ((iq0->re * iq2->re) + (iq0->im * iq2->im));
    d += ((iq0->re * iq2->im) - (iq2->re * iq0->im));
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

void RvSim::_momentsByFft(const Complex_t *IQ, double prtSecs,
			  double &power, double &vel, double &width)
  
{

  int kCent = _nSamples / 2;
  
  // compute fft
  
  Complex_t spectra[_nSamples];
  _fft->fwd(IQ, spectra);

  // compute magnitudes

  double magnitude[_nSamples];
  Complex_t *spp = spectra;
  double *mp = magnitude;
  for (int ii = 0; ii < _nSamples; ii++, spp++, mp++) {
    *mp = (spp->re * spp->re + spp->im * spp->im);
  }
  
  // find max magnitude

  double maxMag = 0.0;
  int kMax = 0;
  mp = magnitude;
  for (int ii = 0; ii < _nSamples; ii++, mp++) {
    if (*mp > maxMag) {
      kMax = ii;
      maxMag = *mp;
    }
  }
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

double RvSim::_computeSpectralNoise(const double *magCentered)
  
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

///////////////////////////////
// write spectra file

void RvSim::_writeSpectraFile(const string &heading,
			      const Complex_t *comp)
  
{
  
  char outPath[MAX_PATH_LEN];
  sprintf(outPath, "%s%sbeam%d_gate%d_%s",
	  _params.spectra_output_dir, PATH_DELIM,
	  _beamNum, _gateNum, heading.c_str());
  
  ta_makedir_recurse(_params.spectra_output_dir);
  
  ofstream out(outPath);
  if (!out.is_open()) {
    cerr << "ERROR opening file: " << outPath << endl;
    return;
  }
				
  for (int ii = 0; ii < _nSamples; ii++, comp++) {
    double mag = comp->re * comp->re + comp->im * comp->im;
    double angle = atan2(comp->im, comp->re) * RAD_TO_DEG;
    out << setw(3) << ii << "  "
	<< setw(10) << mag << "  "
	<< setw(10) << angle << endl;
  }

  out.close();
  
}

////////////////////////////////////////
// write out the netDCF file to tmp name
//
// Returns 0 on success, -1 on failure

int RvSim::_writeTmpFile(const string &tmpPath,
			 const vector<iq_t> &pulseIQ,
			 const vector<moments_t> &trip1Moments,
			 const vector<moments_t> &trip2Moments)

{

  // ensure directory exists

  if (ta_makedir_recurse(_params.output_dir)) {
    cerr << "ERROR - RvSim::_writeTmpFile" << endl;
    cerr << "  Cannot make output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errno) << endl;
    return -1;
  }

  ////////////////////////
  // create NcFile object
  
  NcError err(NcError::verbose_nonfatal);
  
  NcFile out(tmpPath.c_str(), NcFile::Replace);
  if (!out.is_valid()) {
    cerr << "ERROR - RvSim::_writeTmpFile" << endl;
    cerr << "  Cannot create file: " << tmpPath << endl;
    return -1;
  }
  int iret = 0;

  /////////////////////
  // global attributes
  
  int nTimes = pulseIQ.size() / _nGates;
  int startingSample = 0;
  int endingSample = startingSample + nTimes - 1;
  int startGate = _params.start_gate;
  int endGate = startGate + _nGates - 1;
  
  char desc[1024];
  sprintf(desc,
	  "Radar time series reformatted by RvSim\n"
	  "Starting Sample =%d, Ending Sample =%d, "
	  "Start Gate= %d, End Gate = %d\n"
	  "Azimuth = %.2f, Elevation = %.2f\n",
	  startingSample, endingSample, startGate, endGate,
	  _params.start_az, _params.elevation);
  out.add_att("Description", desc);
  out.add_att("FirstGate", startGate);
  out.add_att("LastGate", endGate);

  //////////////////
  // add dimensions
  
  NcDim *gatesDim = out.add_dim("gates", _nGates);
  //int gatesId = gatesDim->id();
  
  NcDim *beamDim = out.add_dim("beams", _nBeams);

  NcDim *frtimeDim = out.add_dim("frtime");
  //int frtimeId = frtimeDim->id();

  /////////////////////////////////
  // add vars and their attributes

  // I variable

  NcVar *iVar = out.add_var("I", ncFloat, frtimeDim, gatesDim);
  iVar->add_att("long_name", "In-phase time series variable");
  iVar->add_att("units", "scaled A/D counts");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    iVar->add_att("valid_range", 2, validRange);
  }
  iVar->add_att("_FillValue", (float) 0.0);
  {
    float *idata = (float *) umalloc(pulseIQ.size() * sizeof(float));
    for (size_t jj = 0; jj < pulseIQ.size(); jj++) {
      idata[jj] = pulseIQ[jj].i;
    }
    long edges[2];
    edges[0] = nTimes;
    edges[1] = _nGates;
    iVar->put(idata, edges);
    ufree(idata);
  }

  // Q variable

  NcVar *qVar = out.add_var("Q", ncFloat, frtimeDim, gatesDim);
  qVar->add_att("long_name", "Quadruture time series variable");
  qVar->add_att("units", "scaled A/D counts");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    qVar->add_att("valid_range", 2, validRange);
  }
  qVar->add_att("_FillValue", (float) 0.0);
  {
    float *qdata = (float *) umalloc(pulseIQ.size() * sizeof(float));
    for (size_t jj = 0; jj < pulseIQ.size(); jj++) {
      qdata[jj] = pulseIQ[jj].q;
    }
    long edges[2];
    edges[0] = nTimes;
    edges[1] = _nGates;
    qVar->put(qdata, edges);
    ufree(qdata);
  }

  // trip 1 dbm

  NcVar *dbm1Var = out.add_var("dbm1", ncFloat, beamDim, gatesDim);
  qVar->add_att("long_name", "Trip 1 dbm");
  qVar->add_att("units", "dbm");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    qVar->add_att("valid_range", 2, validRange);
  }
  qVar->add_att("_FillValue", (float) 0.0);
  {
    float vals[trip1Moments.size()];
    for (size_t jj = 0; jj < trip1Moments.size(); jj++) {
      vals[jj] = trip1Moments[jj].dbm;
    }
    long edges[2];
    edges[0] = _nBeams;
    edges[1] = _nGates;
    dbm1Var->put(vals, edges);
  }

  // trip 1 power

  NcVar *power1Var = out.add_var("power1", ncFloat, beamDim, gatesDim);
  qVar->add_att("long_name", "Trip 1 power");
  qVar->add_att("units", "volts");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    qVar->add_att("valid_range", 2, validRange);
  }
  qVar->add_att("_FillValue", (float) 0.0);
  {
    float vals[trip1Moments.size()];
    for (size_t jj = 0; jj < trip1Moments.size(); jj++) {
      vals[jj] = trip1Moments[jj].power;
    }
    long edges[2];
    edges[0] = _nBeams;
    edges[1] = _nGates;
    power1Var->put(vals, edges);
  }

  // trip 1 vel

  NcVar *vel1Var = out.add_var("vel1", ncFloat, beamDim, gatesDim);
  qVar->add_att("long_name", "Trip 1 velocity");
  qVar->add_att("units", "m/s");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    qVar->add_att("valid_range", 2, validRange);
  }
  qVar->add_att("_FillValue", (float) 0.0);
  {
    float vals[trip1Moments.size()];
    for (size_t jj = 0; jj < trip1Moments.size(); jj++) {
      vals[jj] = trip1Moments[jj].vel;
    }
    long edges[2];
    edges[0] = _nBeams;
    edges[1] = _nGates;
    vel1Var->put(vals, edges);
  }

  // trip 1 width

  NcVar *width1Var = out.add_var("width1", ncFloat, beamDim, gatesDim);
  qVar->add_att("long_name", "Trip 1 width");
  qVar->add_att("units", "m/s");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    qVar->add_att("valid_range", 2, validRange);
  }
  qVar->add_att("_FillValue", (float) 0.0);
  {
    float vals[trip1Moments.size()];
    for (size_t jj = 0; jj < trip1Moments.size(); jj++) {
      vals[jj] = trip1Moments[jj].width;
    }
    long edges[2];
    edges[0] = _nBeams;
    edges[1] = _nGates;
    width1Var->put(vals, edges);
  }

  // trip 2 dbm

  NcVar *dbm2Var = out.add_var("dbm2", ncFloat, beamDim, gatesDim);
  qVar->add_att("long_name", "Trip 2 dbm");
  qVar->add_att("units", "dbm");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    qVar->add_att("valid_range", 2, validRange);
  }
  qVar->add_att("_FillValue", (float) 0.0);
  {
    float vals[trip2Moments.size()];
    for (size_t jj = 0; jj < trip2Moments.size(); jj++) {
      vals[jj] = trip2Moments[jj].dbm;
    }
    long edges[2];
    edges[0] = _nBeams;
    edges[1] = _nGates;
    dbm2Var->put(vals, edges);
  }

  // trip 2 power

  NcVar *power2Var = out.add_var("power2", ncFloat, beamDim, gatesDim);
  qVar->add_att("long_name", "Trip 2 power");
  qVar->add_att("units", "volts");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    qVar->add_att("valid_range", 2, validRange);
  }
  qVar->add_att("_FillValue", (float) 0.0);
  {
    float vals[trip2Moments.size()];
    for (size_t jj = 0; jj < trip2Moments.size(); jj++) {
      vals[jj] = trip2Moments[jj].power;
    }
    long edges[2];
    edges[0] = _nBeams;
    edges[1] = _nGates;
    power2Var->put(vals, edges);
  }

  // trip 2 vel

  NcVar *vel2Var = out.add_var("vel2", ncFloat, beamDim, gatesDim);
  qVar->add_att("long_name", "Trip 2 velocity");
  qVar->add_att("units", "m/s");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    qVar->add_att("valid_range", 2, validRange);
  }
  qVar->add_att("_FillValue", (float) 0.0);
  {
    float vals[trip2Moments.size()];
    for (size_t jj = 0; jj < trip2Moments.size(); jj++) {
      vals[jj] = trip2Moments[jj].vel;
    }
    long edges[2];
    edges[0] = _nBeams;
    edges[1] = _nGates;
    vel2Var->put(vals, edges);
  }

  // trip 2 width

  NcVar *width2Var = out.add_var("width2", ncFloat, beamDim, gatesDim);
  qVar->add_att("long_name", "Trip 2 width");
  qVar->add_att("units", "m/s");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    qVar->add_att("valid_range", 2, validRange);
  }
  qVar->add_att("_FillValue", (float) 0.0);
  {
    float vals[trip2Moments.size()];
    for (size_t jj = 0; jj < trip2Moments.size(); jj++) {
      vals[jj] = trip2Moments[jj].width;
    }
    long edges[2];
    edges[0] = _nBeams;
    edges[1] = _nGates;
    width2Var->put(vals, edges);
  }

  // SampleNum variable

  NcVar *sampleNumVar = out.add_var("SampleNum", ncInt, frtimeDim);
  sampleNumVar->add_att("long_name", "Sample Number");
  sampleNumVar->add_att("units", "Counter");
  sampleNumVar->add_att("valid_range", 100000000);
  {
    int sampleNums[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      sampleNums[jj] = jj;
    }
    long edge = nTimes;
    sampleNumVar->put(sampleNums, &edge);
  }

  // Azimuth variable

  NcVar *azVar = out.add_var("Azimuth", ncFloat, frtimeDim);
  azVar->add_att("long_name", "Antenna Azimuth");
  azVar->add_att("units", "degrees");
  {
    float validRange[2] = {0.0f, 360.0f};
    azVar->add_att("valid_range", 2, validRange);
  }
  azVar->add_att("_FillValue", (float) 0.0);
  {
    float azimuths[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      azimuths[jj] = (float) (_params.start_az + jj * _params.delta_az);
    }
    long edge = nTimes;
    azVar->put(azimuths, &edge);
  }

  // Elevation variable
  
  NcVar *elVar = out.add_var("Elevation", ncFloat, frtimeDim);
  elVar->add_att("long_name", "Antenna Elevation");
  elVar->add_att("units", "degrees");
  {
    float validRange[2] = {0.0f, 360.0f};
    elVar->add_att("valid_range", 2, validRange);
  }
  elVar->add_att("_FillValue", (float) 0.0);
  {
    float elevations[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      elevations[jj] = (float) _params.elevation;
    }
    long edge = nTimes;
    elVar->put(elevations, &edge);
  }

  // PRT variable

  NcVar *prtVar = out.add_var("Prt", ncInt, frtimeDim);
  prtVar->add_att("long_name", "Pulse Repetition Time");
  prtVar->add_att("units", "microseconds");
  prtVar->add_att("valid_range", 1000000);
  {
    int prts[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      prts[jj] = _params.prt;
    }
    long edge = nTimes;
    prtVar->put(prts, &edge);
  }
  
  // Time variable

  NcVar *timeVar = out.add_var("Time", ncDouble, frtimeDim);
  timeVar->add_att("long_name", "Date/Time value");
  timeVar->add_att("units", "days since 0000-01-01");
  timeVar->add_att("_FillValue", 0.0);
  time_t now = time(NULL);
  {
    double times[nTimes];
    double delta_time = 1.0 / _params.prt;
    for (int jj = 0; jj < nTimes; jj++) {
      times[jj] = (double) (now + jj * delta_time) / 86400.0;
    }
    long edge = nTimes;
    timeVar->put(times, &edge);
  }

  return iret;

}


