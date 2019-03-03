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
// RvDealias.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2003
//
///////////////////////////////////////////////////////////////
//
// RvDealias reads netCDF radar IQ data and performs range-velocity
// dealiasing
//
////////////////////////////////////////////////////////////////

#include <iomanip>
#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include "RvDealias.hh"
#include "Moments.hh"
#include "Verify.hh"
using namespace std;

// Constructor

RvDealias::RvDealias(int argc, char **argv)

{

  _input = NULL;
  _moments = NULL;
  _verify = NULL;
  isOK = true;
  _nTimes = 0;
  _nBeams = 0;
  _nGates = 0;
  _nBeamGates = 0;
  _nTimeGates = 0;
  _firstGate = 0;
  _I = NULL;
  _Q = NULL;
  _dbm1 = _power1 = _vel1 = _width1 = NULL;
  _dbm2 = _power2 = _vel2 = _width2 = NULL;
  _Azimuth = NULL;
  _Elevation = NULL;
  _Prt = NULL;
  _SampleNum = NULL;
  _Time = NULL;

  // set programe name
  
  _progName = "RvDealias";
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

  // check that file list set in archive and simulate mode
  
  if (_params.mode == Params::ARCHIVE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: RvDealias::RvDealias." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  if (_params.mode == Params::SIMULATE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: RvDealias::RvDealias." << endl;
    cerr << "  Mode is SIMULATE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.use_ldata_info_file);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  // initialize phase codes

  _initPhaseCodes();

  // set up moments object
  // This initializes the FFT package to the set number of samples.

  _moments = new Moments();
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _moments->setDebug();
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _moments->setVerbose();
  }
  _moments->setWavelength(_params.wavelength_cm / 100.0);
  if (_params.negate_phase_codes) {
    _moments->setSzNegatePhaseCodes();
  }
  if (_params.sz_window == Params::HANNING) {
    _moments->setSzWindow(Moments::WINDOW_HANNING);
  } else if (_params.sz_window == Params::MOD_HANNING) {
    _moments->setSzWindow(Moments::WINDOW_MOD_HANNING);
  }
  
  _moments->setNoiseValueDbm(_params.noise_value);
  _moments->setSignalToNoiseRatioThreshold
    (_params.signal_to_noise_ratio_threshold);
  _moments->setSzStrongToWeakPowerRatioThreshold
    (_params.sz_strong_to_weak_power_ratio_threshold);
  _moments->setSzOutOfTripPowerRatioThreshold
    (_params.sz_out_of_trip_power_ratio_threshold);
  _moments->setSzOutOfTripPowerNReplicas
    (_params.sz_out_of_trip_power_n_replicas);
  
  if (_params.write_spectra_files) {
    _moments->setWriteSpectra(true, _params.spectra_dir);
  }

  _nSamples = _moments->getNSamples();
  
  // set up verification object

  if (_params.write_verification_data && 
      (_params.algorithm == Params::SZ864_PP ||
       _params.algorithm == Params::SZ864_FFT)) {
    _verify = new Verify(_params);
  }
  
  return;

}

// destructor

RvDealias::~RvDealias()

{

  if (_input) {
    delete _input;
  }

  if (_moments) {
    delete _moments;
  }

  if (_verify) {
    delete _verify;
  }

  _freeArrays();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RvDealias::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");

  if (_params.mode == Params::SIMULATE) {

    // simulate mode - go through the file list repeatedly
    
    while (true) {
      
      char *inputPath;
      _input->reset();
      while ((inputPath = _input->next()) != NULL) {
	PMU_auto_register("Simulate mode");
	if (_processFile(inputPath)) {
	  cerr << "ERROR = RvDealias::Run" << endl;
	  cerr << "  Processing file: " << inputPath << endl;
	  iret = -1;
	}
      } // while

    }

  } else {
    
    // loop until end of data
    
    char *inputPath;
    while ((inputPath = _input->next()) != NULL) {
      
      PMU_auto_register("Non-simulate mode");
      
      if (_processFile(inputPath)) {
	cerr << "ERROR = RvDealias::Run" << endl;
	cerr << "  Processing file: " << inputPath << endl;
	iret = -1;
      }
      
    }

  } // if (_params.mode == Params::SIMULATE)
  
  // write verification data if needed

  if (_verify) {
    _verify->computeStatsAndWrite();
  }
    
  return iret;

}

/////////////////////////////////////////////////////
// initialize the phase codes

void RvDealias::_initPhaseCodes()
  
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

///////////////////////////////
// process file

int RvDealias::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // open file

  Nc3File ncf(input_path);
  if (!ncf.is_valid()) {
    cerr << "ERROR - RvDealias::_processFile" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }

  // declare an error object

  Nc3Error err(Nc3Error::silent_nonfatal);

  // if (_params.debug >= Params::DEBUG_VERBOSE) {
  // _printFile(ncf);
  // }

  // check that this is a valid file

  if (_loadFromFile(ncf)) {
    cerr << "ERROR - RvDealias::_processFile" << endl;
    cerr << "  Not a valid IQ file" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }

  // if (_params.debug >= Params::DEBUG_VERBOSE) {
  // _printData();
  // }

  clock_t start = clock();
  for (int ibeam = 0; ibeam < _nBeams; ibeam++) {
    _processBeam(ibeam);
  }
  clock_t end = clock();
  if (_params.debug) {
    double cpuSecs = ((double) end - start) / CLOCKS_PER_SEC;
    double totalGates = _nBeams * _nGates;
    double gatesPerSec = totalGates / cpuSecs;
    cerr << "  CPU time used: " << cpuSecs << endl;
    cerr << "  Nbeams: " << _nBeams << endl;
    cerr << "  Ngates: " << _nGates << endl;
    cerr << "  Gates per sec: " << gatesPerSec << endl;
  }
  
  return 0;

}

///////////////////////////////
// process a beam

void RvDealias::_processBeam(int beam_num)

{

  int istart = beam_num * _nSamples;
  int iend = istart + _nSamples;
  
  double meanAz = 0.0;
  double meanEl = 0.0;
  double meanDay = 0.0;
  double meanPrt = 0.0;
  
  for (int ii = istart; ii < iend; ii++) {
    meanAz += _Azimuth[ii];
    meanEl += _Elevation[ii];
    meanDay += _Time[ii];
    meanPrt += _Prt[ii];
  }
  if (_nSamples > 0) {
    meanAz /= _nSamples;
    meanEl /= _nSamples;
    meanDay /= _nSamples;
    meanPrt /= (_nSamples * 1000000);
  }
  double meanSecs = meanDay * 86.400;
  time_t meanUtime = (time_t) meanSecs;
  int msecs = (int) ((meanSecs - meanUtime) * 1000.0 + 0.5);

  // Set up beam phase codes.
  // Allow for up to 4 trips, so we index into the array by 4
  //   and wrap the data around.
  // Also compute trip1 to trip 2 difference.
  
  RadarComplex_t beamCodeArray[4 + _nSamples];
  RadarComplex_t *beamCode = beamCodeArray + 4;
  RadarComplex_t delta12[_nSamples];
  for (int ii = -4; ii < _nSamples; ii++) {
    int index = (ii + _params.trip1_phase_index + _nSamples) % _nSamples;
    beamCode[ii] = _phaseCode[index];
  }
  for (int ii = 0; ii < _nSamples; ii++) {
    delta12[ii].re =
      beamCode[ii-1].re * beamCode[ii].re +
      beamCode[ii-1].im * beamCode[ii].im;
    delta12[ii].im =
      beamCode[ii-1].re * beamCode[ii].im -
      beamCode[ii-1].im * beamCode[ii].re;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      double ang = 0.0;
      if (delta12[ii].re != 0.0 || delta12[ii].im != 0.0) {
	ang = atan2(delta12[ii].im, delta12[ii].re) * RAD_TO_DEG;
	cerr << "delta12[" << ii << "]: " << ang << endl;
      }
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Beam: " << beam_num << endl;
    cerr << "  Time: " << DateTime::str(meanUtime, false)
	 << "." << msecs << endl;
    cerr << "  El (deg): " << meanEl << endl;
    cerr << "  Az (deg): " << meanAz << endl;
    cerr << "  Prt (usecs) : " << meanPrt << endl;
  }

  for (int ii = 0; ii < _nGates; ii++) {
    
    int jj = beam_num * _nGates + ii;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "---------> gate: " << setw(4) << ii << " " << endl;
    }

    // load up IQ data

    RadarComplex_t IQ[_nSamples];
    RadarComplex_t *iq = IQ;
    
    float *II = _I + istart * _nGates + ii;
    float *QQ = _Q + istart * _nGates + ii;
    
    for (int isamp = 0; isamp < _nSamples;
	 isamp++, iq++, II += _nGates, QQ += _nGates) {
      iq->re = *II;
      iq->im = *QQ;
    }

    _moments->setAz(beam_num);
    _moments->setRange(ii);

    double power1, vel1, width1;
    double power2, vel2, width2;
    int flags1, flags2;
    double r1Ratio = 0;

    switch (_params.algorithm) {
      
    case Params::PP: {
      _moments->computeByPp(IQ, meanPrt,
			    power1, vel1, width1, flags1);
      break;
    }
    
    case Params::FFT: {
      _moments->computeByFft(IQ, meanPrt,
			     power1, vel1, width1, flags1);
      break;
    }
      
    case Params::FFT_HANNING: {
      RadarComplex_t IQ_hanning[_nSamples];
      _moments->applyHanningWindow(IQ, IQ_hanning);
      _moments->computeByFft(IQ_hanning, meanPrt,
			     power1, vel1, width1, flags1);
      break;
    }
    
    case Params::FFT_MOD_HANNING: {
      RadarComplex_t IQ_modHanning[_nSamples];
      _moments->applyModHanningWindow(IQ, IQ_modHanning);
      _moments->computeByFft(IQ_modHanning, meanPrt,
			     power1, vel1, width1, flags1);
      break;
    }
    
    case Params::SZ864_PP: {

      // cohere to trip 1
      
      RadarComplex_t trip1[_nSamples];
      _moments->cohere2Trip(IQ, beamCode, 1, trip1);
      
      _moments->computeBySzPp(trip1, delta12, meanPrt,
			      power1, vel1, width1, flags1,
			      power2, vel2, width2, flags2);
      break;

    }
    
    case Params::SZ864_FFT: {

      // cohere to trip 1
      
      RadarComplex_t trip1[_nSamples];
      _moments->cohere2Trip(IQ, beamCode, 1, trip1);
      
      _moments->computeBySzFft(trip1, delta12, meanPrt,
			       power1, vel1, width1, flags1,
			       power2, vel2, width2, flags2);
      break;

    }
    
    case Params::TEST: {
      //        _moments->testDecon(IQ, beamCode, delta12, meanPrt,
      //    			  power1, vel1, width1,
      //  			  power2, vel2, width2);
      break;
    }
    
    default: {}
      
    } // switch (_params.algorithm)

    double dbm1 = -200.0, dbm2 = -200.0;
    if (power1 > 0) {
      dbm1 = 10.0 * log10(power1);
    }
    if (power2 > 0) {
      dbm2 = 10.0 * log10(power2);
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << setprecision(4);
      cerr << "  trip1 retrv db,v,w,f: "
	   << setw(10) << dbm1 << setw(10) << vel1
	   << setw(10) << width1 << setw(10) << flags1 << endl;
      if (_dbm1) {
	cerr << "  trip1 truth db,v,w: "
	     << setw(10) << _dbm1[jj] << setw(10) << _vel1[jj]
	     << setw(10) << _width1[jj] << endl;
      }
      if (_params.algorithm == Params::SZ864_PP ||
	  _params.algorithm == Params::SZ864_FFT) {
	cerr << "  trip2 retrv db,v,w: "
	     << setw(10) << dbm2 << setw(10) << vel2
	     << setw(10) << width2 << setw(10) << flags2 << endl;
      }
      if (_dbm2) {
	cerr << "  trip2 truth db,v,w: "
	     << setw(10) << _dbm2[jj] << setw(10) << _vel2[jj]
	     << setw(10) << _width2[jj] << endl;
      }
      if (_dbm1 && _dbm2) {
	cerr << "  power diff (db): " << _dbm1[jj] - _dbm2[jj] << endl;
      }
    }

    if (_verify && _dbm1 && _dbm2 && _vel1 && _vel2 && _width1 && _width2) {
      _verify->addToStats(dbm1, _dbm1[jj],
			  vel1, _vel1[jj],
			  width1, _width1[jj],
			  dbm2, _dbm2[jj],
			  vel2, _vel2[jj],
			  width2, _width2[jj],
			  r1Ratio);
    }
    
  } // ii

}

//////////////////////////////////
// Check that this is a valid file
//
// Returns 0 on success, -1 on failure

int RvDealias::_loadFromFile(Nc3File &ncf)

{

  _freeArrays();
  Nc3Values *vals;

  if (ncf.rec_dim() == NULL) {
    cerr << "ERROR - RvDealias::_loadFromFile" << endl;
    cerr << "  Time dimension missing" << endl;
    return -1;
  }
  _nTimes = ncf.rec_dim()->size();
  
  if (ncf.get_dim("beams") != NULL) {
    _nBeams = ncf.get_dim("beams")->size();
  } else {
    _nBeams = _nTimes / _nSamples;
  }
  
  if (ncf.get_dim("gates") == NULL) {
    if (ncf.get_dim("Gates") == NULL) {
      cerr << "ERROR - RvDealias::_loadFromFile" << endl;
      cerr << "  gates dimension missing" << endl;
      return -1;
    }
    _nGates = ncf.get_dim("Gates")->size();
  } else {
    _nGates = ncf.get_dim("gates")->size();
  }
  _nBeamGates = _nBeams * _nGates;
  
  if (ncf.get_var("I") == NULL) {
    cerr << "ERROR - RvDealias::_loadFromFile" << endl;
    cerr << "  I variable missing" << endl;
    return -1;
  }
  _nTimeGates = _nTimes * _nGates;
  vals = ncf.get_var("I")->values();
  _I = new float[_nTimeGates];
  memcpy(_I, vals->base(), _nTimeGates * sizeof(float));
  delete vals;

  if (ncf.get_var("Q") == NULL) {
    cerr << "ERROR - RvDealias::_loadFromFile" << endl;
    cerr << "  Q variable missing" << endl;
    return -1;
  }
  vals = ncf.get_var("Q")->values();
  _Q = new float[_nTimeGates];
  memcpy(_Q, vals->base(), _nTimeGates * sizeof(float));
  delete vals;

  if (ncf.get_var("dbm1") != NULL) {
    vals = ncf.get_var("dbm1")->values();
    _dbm1 = new float[_nBeamGates];
    memcpy(_dbm1, vals->base(), _nBeamGates * sizeof(float));
    delete vals;
  }

  if (ncf.get_var("power1") != NULL) {
    vals = ncf.get_var("power1")->values();
    _power1 = new float[_nBeamGates];
    memcpy(_power1, vals->base(), _nBeamGates * sizeof(float));
    delete vals;
  }

  if (ncf.get_var("vel1") != NULL) {
    vals = ncf.get_var("vel1")->values();
    _vel1 = new float[_nBeamGates];
    memcpy(_vel1, vals->base(), _nBeamGates * sizeof(float));
    delete vals;
  }

  if (ncf.get_var("width1") != NULL) {
    vals = ncf.get_var("width1")->values();
    _width1 = new float[_nBeamGates];
    memcpy(_width1, vals->base(), _nBeamGates * sizeof(float));
    delete vals;
  }

  if (ncf.get_var("dbm2") != NULL) {
    vals = ncf.get_var("dbm2")->values();
    _dbm2 = new float[_nBeamGates];
    memcpy(_dbm2, vals->base(), _nBeamGates * sizeof(float));
    delete vals;
  }

  if (ncf.get_var("power2") != NULL) {
    vals = ncf.get_var("power2")->values();
    _power2 = new float[_nBeamGates];
    memcpy(_power2, vals->base(), _nBeamGates * sizeof(float));
    delete vals;
  }

  if (ncf.get_var("vel2") != NULL) {
    vals = ncf.get_var("vel2")->values();
    _vel2 = new float[_nBeamGates];
    memcpy(_vel2, vals->base(), _nBeamGates * sizeof(float));
    delete vals;
  }

  if (ncf.get_var("width2") != NULL) {
    vals = ncf.get_var("width2")->values();
    _width2 = new float[_nBeamGates];
    memcpy(_width2, vals->base(), _nBeamGates * sizeof(float));
    delete vals;
  }

  if (ncf.get_var("SampleNum") == NULL) {
    cerr << "ERROR - RvDealias::_loadFromFile" << endl;
    cerr << "  SampleNum variable missing" << endl;
    return -1;
  }
  vals = ncf.get_var("SampleNum")->values();
  _SampleNum = new int[_nTimes];
  memcpy(_SampleNum, vals->base(), _nTimes * sizeof(int));
  delete vals;

  if (ncf.get_var("Azimuth") == NULL) {
    cerr << "ERROR - RvDealias::_loadFromFile" << endl;
    cerr << "  Azimuth variable missing" << endl;
    return -1;
  }
  vals = ncf.get_var("Azimuth")->values();
  _Azimuth = new float[_nTimes];
  memcpy(_Azimuth, vals->base(), _nTimes * sizeof(float));
  delete vals;

  if (ncf.get_var("Elevation") == NULL) {
    cerr << "ERROR - RvDealias::_loadFromFile" << endl;
    cerr << "  Elevation variable missing" << endl;
    return -1;
  }
  vals = ncf.get_var("Elevation")->values();
  _Elevation = new float[_nTimes];
  memcpy(_Elevation, vals->base(), _nTimes * sizeof(float));
  delete vals;

  if (ncf.get_var("Prt") == NULL) {
    cerr << "ERROR - RvDealias::_loadFromFile" << endl;
    cerr << "  Prt variable missing" << endl;
    return -1;
  }
  vals = ncf.get_var("Prt")->values();
  _Prt = new int[_nTimes];
  memcpy(_Prt, vals->base(), _nTimes * sizeof(int));
  delete vals;

  Nc3Var *timeVar = ncf.get_var("Time");
  if (timeVar == NULL) {
    cerr << "ERROR - RvDealias::_loadFromFile" << endl;
    cerr << "  Time variable missing" << endl;
    return -1;
  }
  vals = timeVar->values();
  _Time = new double[_nTimes];
  memcpy(_Time, vals->base(), _nTimes * sizeof(double));
  delete vals;

  Nc3Att *firstGateAtt = ncf.get_att("FirstGate");
  if (firstGateAtt == NULL) {
    cerr << "ERROR - RvDealias::_loadFromFile" << endl;
    cerr << "  FirstGate attribute missing" << endl;
    return -1;
  }
  _firstGate = firstGateAtt->as_int(0);
  delete firstGateAtt;

  return 0;

}

////////////////////
// free all arrays

void RvDealias::_freeArrays()
{

  if (_I) {
    delete _I;
  }

  if (_Q) {
    delete _Q;
  }

  if (_Azimuth) {
    delete _Azimuth;
  }

  if (_Elevation) {
    delete _Elevation;
  }

  if (_Prt) {
    delete _Prt;
  }

  if (_SampleNum) {
    delete _SampleNum;
  }

  if (_Time) {
    delete _Time;
  }

}

///////////////////////////////
// print data

void RvDealias::_printData()

{

  cout << "==================================================" << endl;
  cout << "  nTimes: " << _nTimes << endl;
  cout << "  nBeams: " << _nBeams << endl;
  cout << "  nGates: " << _nGates << endl;
  cout << "  firstGate: " << _firstGate << endl;
  
  cout << "  I:";
  for (int ii = 0; ii < _nTimeGates; ii++) {
    cout << " " << _I[ii];
  }
  cout << endl;
  
  cout << "  Q:";
  for (int ii = 0; ii < _nTimeGates; ii++) {
    cout << " " << _Q[ii];
  }
  cout << endl;
  
  cout << "  SampleNum:";
  for (int ii = 0; ii < _nTimes; ii++) {
    cout << " " << _SampleNum[ii];
  }
  cout << endl;
  
  cout << "  Azimuth:";
  for (int ii = 0; ii < _nTimes; ii++) {
    cout << " " << _Azimuth[ii];
  }
  cout << endl;
  
  cout << "  Elevation:";
  for (int ii = 0; ii < _nTimes; ii++) {
    cout << " " << _Elevation[ii];
  }
  cout << endl;
  
  cout << "  Prt:";
  for (int ii = 0; ii < _nTimes; ii++) {
    cout << " " << _Prt[ii];
  }
  cout << endl;
  
  cout << "  Times:";
  for (int ii = 0; ii < _nTimes; ii++) {
    cout << " " << _Time[ii];
  }
  cout << endl;

}

///////////////////////////////
// print data in file

void RvDealias::_printFile(Nc3File &ncf)

{

  cout << "ndims: " << ncf.num_dims() << endl;
  cout << "nvars: " << ncf.num_vars() << endl;
  cout << "nattributes: " << ncf.num_atts() << endl;
  Nc3Dim *unlimd = ncf.rec_dim();
  cout << "unlimdimid: " << unlimd->size() << endl;
  
  // dimensions

  Nc3Dim *dims[ncf.num_dims()];
  for (int idim = 0; idim < ncf.num_dims(); idim++) {
    dims[idim] = ncf.get_dim(idim);

    cout << endl;
    cout << "Dim #: " << idim << endl;
    cout << "  Name: " << dims[idim]->name() << endl;
    cout << "  Length: " << dims[idim]->size() << endl;
    cout << "  Is valid: " << dims[idim]->is_valid() << endl;
    cout << "  Is unlimited: " << dims[idim]->is_unlimited() << endl;
    
  } // idim
  
  cout << endl;

  // global attributes

  cout << "Global attributes:" << endl;

  for (int iatt = 0; iatt < ncf.num_atts(); iatt++) {
    cout << "  Att num: " << iatt << endl;
    Nc3Att *att = ncf.get_att(iatt);
    _printAtt(att);
    delete att;
  }

  // loop through variables

  Nc3Var *vars[ncf.num_vars()];
  for (int ivar = 0; ivar < ncf.num_vars(); ivar++) {

    vars[ivar] = ncf.get_var(ivar);
    cout << endl;
    cout << "Var #: " << ivar << endl;
    cout << "  Name: " << vars[ivar]->name() << endl;
    cout << "  Is valid: " << vars[ivar]->is_valid() << endl;
    cout << "  N dims: " << vars[ivar]->num_dims();
    Nc3Dim *vdims[vars[ivar]->num_dims()];
    if (vars[ivar]->num_dims() > 0) {
      cout << ": (";
      for (int ii = 0; ii < vars[ivar]->num_dims(); ii++) {
	vdims[ii] = vars[ivar]->get_dim(ii);
	cout << " " << vdims[ii]->name();
	if (ii != vars[ivar]->num_dims() - 1) {
	  cout << ", ";
	}
      }
      cout << " )";
    }
    cout << endl;
    cout << "  N atts: " << vars[ivar]->num_atts() << endl;
    
    for (int iatt = 0; iatt < vars[ivar]->num_atts(); iatt++) {

      cout << "  Att num: " << iatt << endl;
      Nc3Att *att = vars[ivar]->get_att(iatt);
      _printAtt(att);
      delete att;

    } // iatt

    cout << endl;
    _printVarVals(vars[ivar]);
    
  } // ivar
  
}

/////////////////////
// print an attribute

void RvDealias::_printAtt(Nc3Att *att)

{

  cout << "    Name: " << att->name() << endl;
  cout << "    Num vals: " << att->num_vals() << endl;
  cout << "    Type: ";
  
  Nc3Values *values = att->values();

  switch(att->type()) {
    
  case nc3NoType: {
    cout << "No type: ";
  }
  break;
  
  case nc3Byte: {
    cout << "BYTE: ";
    unsigned char *vals = (unsigned char *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Char: {
    cout << "CHAR: ";
    char vals[att->num_vals() + 1];
    MEM_zero(vals);
    memcpy(vals, values->base(), att->num_vals());
    cout << vals;
  }
  break;
  
  case nc3Short: {
    cout << "SHORT: ";
    short *vals = (short *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Int: {
    cout << "INT: ";
    int *vals = (int *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Int64: {
    cout << "INT: ";
    int64_t *vals = (int64_t *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Float: {
    cout << "FLOAT: ";
    float *vals = (float *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Double: {
    cout << "DOUBLE: ";
    double *vals = (double *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  }
  
  cout << endl;

  delete values;

}

    
void RvDealias::_printVarVals(Nc3Var *var)

{

  int nprint = var->num_vals();
  if (nprint > 100) {
    nprint = 100;
    cout << "  NOTE - only printing first 100 vals" << endl;
  }

  Nc3Values *values = var->values();

  cout << "  Variable vals:";
  
  switch(var->type()) {
    
  case nc3NoType: {
  }
  break;
  
  case nc3Byte: {
    cout << "(byte)";
    unsigned char *vals = (unsigned char *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Char: {
    cout << "(char)";
    char str[nprint + 1];
    MEM_zero(str);
    memcpy(str, values->base(), nprint);
    cout << " " << str;
  }
  break;
  
  case nc3Short: {
    cout << "(short)";
    short *vals = (short *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Int: {
    cout << "(int)";
    int *vals = (int *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Int64: {
    cout << "(int64_t)";
    int64_t *vals = (int64_t *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Float: {
    cout << "(float)";
    float *vals = (float *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case nc3Double: {
    cout << "(double)";
    double *vals = (double *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      fprintf(stdout, " %.10f", vals[ii]);
      // cout << " " << vals[ii];
    }
  }
  break;
  
  }
  
  cout << endl;
  delete values;

}

