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
// Lirp2Dsr.cc
//
// Lirp2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2003
//
///////////////////////////////////////////////////////////////
//
// Lirp2Dsr reads raw Lirp IQ time-series data, computes the
// moments and writes the contents into a DsRadar FMQ.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <iomanip>
#include <toolsa/os_config.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <rapmath/stats.h>
#include "sz864.h"
#include "Lirp2Dsr.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

Lirp2Dsr::Lirp2Dsr(int argc, char **argv)

{

  _input = NULL;
  _fmq = NULL;
  _momentsMgr = NULL;

  _nSamples = _nSamplesSz;
  _maxPulseQueueSize = 0;

  _midIndex1 = 0;
  _midIndex2 = 0;
  _countSinceBeam = 0;

  _pulseSeqNum = 0;
  _volNum = 0;
  _az = 0.0;
  _prevAz = -1.0;
  _el = 0.0;
  _prevEl = -180;
  _time = 0;

  _prevPrfForParams = -1;
  _prevPrfForMoments = -1;
  _nGatesPulse = 0;
  _nGatesOut = 0;
  _prevNGatesForParams = -1;
  _paramsSentThisFile = false;
  
  _volMinEl = 180.0;
  _volMaxEl = -180.0;
  _nBeamsThisVol = 0;

  _specPrintCount = 0;
  _spectraFile = NULL;

  isOK = true;

  // set programe name

  _progName = "Lirp2Dsr";
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
    return;
  }
  
  // check that file list set in archive and simulate mode
  
  if (_params.mode == Params::ARCHIVE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: Lirp2Dsr::Lirp2Dsr." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
    return;
  }
    
  if (_params.mode == Params::SIMULATE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: Lirp2Dsr::Lirp2Dsr." << endl;
    cerr << "  Mode is SIMULATE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
    return;
  }

  // initialize the data input path object
  
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

  // set up moments objects
  // This initializes the FFT package to the set number of samples.

  for (int i = 0; i < _params.moments_params_n; i++) {
    Params::moments_params_t &mparams = _params._moments_params[i];
    int nSamples = mparams.n_samples;
    if (mparams.apply_sz) {
      nSamples = _nSamplesSz;
    }
    MomentsMgr *mgr = new MomentsMgr(_progName, _params,
				     _params._moments_params[i],
				     nSamples, _maxGates);
    _momentsMgrArray.push_back(mgr);
  }
  if (_momentsMgrArray.size() < 1) {
    cerr << "ERROR: Lirp2Dsr::Lirp2Dsr." << endl;
    cerr << "  No algorithm geometry specified."; 
    cerr << "  The param moments_menuetry must have at least 1 entry."
	 << endl;
    isOK = false;
    return;
  }

  if (_params.use_c_for_sz) {
    
    // initialize sz864 module
    
    szInit();
    szSetWavelength(_params.radar.wavelength / 100.0);
    szSetNoiseValueDbm(_params.radar.noise_value);
    szSetSignalToNoiseRatioThreshold
      (_params.signal_to_noise_ratio_threshold);
    szSetSzStrongToWeakPowerRatioThreshold
      (_params.sz_strong_to_weak_power_ratio_threshold);
    szSetSzOutOfTripPowerRatioThreshold
      (_params.sz_out_of_trip_power_ratio_threshold);
    szSetSzOutOfTripPowerNReplicas(_params.sz_out_of_trip_power_n_replicas);
    
  }
    
  // compute pulse queue size, set to 2 * maxNsamples

  int maxNsamples = _nSamplesSz;
  for (int i = 0; i < _params.moments_params_n; i++) {
    maxNsamples = MAX(maxNsamples, _params._moments_params[i].n_samples);
  }
  _maxPulseQueueSize = maxNsamples * 2 + 2;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "_maxPulseQueueSize: " << _maxPulseQueueSize << endl;
  }
  
  // create the output queue
  
  _fmq = new OutputFmq(_progName, _params);
  if (!_fmq->isOK) {
    isOK = false;
    return;
  }

  // open combined spectra file if required

  if (_params.write_combined_spectra_file) {
    if (ta_makedir_recurse(_params.spectra_dir)) {
      cerr << "ERROR - Lirp2Dsr" << endl;
      cerr << "  Cannot make spectra output dir: "
	   << _params.spectra_dir << endl;
      isOK = false;
      return;
    }
    _spectraFilePath = _params.spectra_dir;
    _spectraFilePath += PATH_DELIM;
    _spectraFilePath += "combined_spectra.out";
    if ((_spectraFile = fopen(_spectraFilePath.c_str(), "w")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - Lirp2Dsr" << endl;
      cerr << "  Cannot open combined spectra file: "
	   << _spectraFilePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      isOK = false;
      return;
    }
  }

  // REC initialization

  if (_params.index_beams_in_azimuth) {
    _applyRec = _params.apply_rec;
  } else {
    _applyRec = false;
  }

  if (_applyRec) {

    _recKernelWidth = (int) ((_params.rec_kernel_azimuth_width /
			      _params.azimuth_resolution) + 0.5);
    if (_recKernelWidth % 2 == 0) {
      // make sure we have odd number of beams
      _recKernelWidth++;
    }
    if (Beam::createInterestMaps(_params)) {
      cerr << "ERROR - Lirp2Dsr" << endl;
      cerr << "  Problems with params for REC interest maps" << endl;
      isOK = false;
      return;
    }
    _maxBeamQueueSize = _recKernelWidth;

  } else {

    _recKernelWidth = 1;
    _maxBeamQueueSize = 1;

    // make sure output parameters are consistent

    _params.filter_clutter_using_rec = pFALSE;
    _params.output_rec = pFALSE;
    _params.output_rec_debug = pFALSE;
    
  }

  if (!_params.filter_clutter_using_rec) {
    _params.output_clutter_fields = pFALSE;
  }

  _midBeamIndex = _maxBeamQueueSize / 2;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "_recKernelWidth: " << _recKernelWidth << endl;
    cerr << "_maxBeamQueueSize: " << _maxBeamQueueSize << endl;
    cerr << "_midBeamIndex: " << _midBeamIndex << endl;
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

//////////////////////////////////////////////////////////////////
// destructor

Lirp2Dsr::~Lirp2Dsr()

{

  if (_spectraFile != NULL) {
    fclose(_spectraFile);
  }

  if (_input) {
    delete _input;
  }

  for (size_t ii = 0; ii < _momentsMgrArray.size(); ii++) {
    delete _momentsMgrArray[ii];
  }
  _momentsMgrArray.clear();

  for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
    if (_pulseQueue[ii]->removeClient("Lirp2Dsr destructor") == 0) {
      delete _pulseQueue[ii];
    }
  } // ii
  _pulseQueue.clear();

  for (size_t ii = 0; ii < _beamQueue.size(); ii++) {
    delete _beamQueue[ii];
  } // ii
  _beamQueue.clear();

  if (_fmq) {
    delete _fmq;
  }

  if (_params.use_c_for_sz) {
    szCleanUp();
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Lirp2Dsr::Run ()
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
	  cerr << "ERROR - Lirp2Dsr::Run" << endl;
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
	cerr << "ERROR - Lirp2Dsr::Run" << endl;
	cerr << "  Processing file: " << inputPath << endl;
	iret = -1;
      }
      
    }

  } // if (_params.mode == Params::SIMULATE)

  // put final end of tilt and volume flags

  _fmq->putEndOfVolume(_volNum, _time);

  return iret;

}

///////////////////////////////
// process file

int Lirp2Dsr::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }
  _paramsSentThisFile = false;

  // is the data in the file in HIGHSNRPACK

  bool highSnrPack = false;
  if (strstr(input_path, "_hp_") != NULL) {
    highSnrPack = true;
  }

  // open file
  
  FILE *in;
  if ((in = fopen(input_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Lirp2Dsr::_processFile" << endl;
    cerr << "  Cannot open file: " << input_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  bool hasRocHdr = false;
  if (_checkForRocHeader(in, hasRocHdr)) {
    fclose(in);
    return -1;
  }

  // rewind

  rewind(in);
  
  // read in data, load up pulses

  while (!feof(in)) {
    
    // Create a new pulse object and save a pointer to it in the
    // _pulseBuffer array.  _pulseBuffer is a FIFO, with elements
    // added at the end and dropped off the beginning. So if we have a
    // full buffer delete the first element before shifting the
    // elements to the left.
    
    Pulse *pulse = new Pulse(_params, highSnrPack);
    
    // read in pulse headers and data
    
    if (pulse->read(in, hasRocHdr)) {
      if (_params.debug && !feof(in)) {
	cerr << "ERROR - Lirp2Dsr::_processFile" << endl;
	cerr << "  Cannot read in pulse headers and data" << endl;
	cerr << "  File: " << input_path << endl;
      }
      delete pulse;
      break;
    }

    // add pulse to queue, managing memory appropriately

    _addPulseToQueue(pulse);

    // prepare for moments computations
    
    _prepareForMoments(pulse);
  
    // is a beam ready?
    
    if (_momentsMgr != NULL) {

      if (_beamReady()) {
	
	_countSinceBeam = 0;
	_nBeamsThisVol++;

	// create new beam

	Beam *beam = new Beam(_progName, _params, _maxTrips,
			      _pulseQueue, _az, _momentsMgr);

	_nGatesOut = beam->getNGatesOut();
	
	// compute the basic beam
	
	_computeBeamMomentsBasic(beam);
	
	// add beam to the queue, managing the queue and memory
	
	_addBeamToQueue(beam);

	// process beam at middle of queue
	
	if ((int) _beamQueue.size() == _maxBeamQueueSize) {
	  
	  Beam *midBeam = _beamQueue[_midBeamIndex];
	  
	  // compute the REC for the beam
	  
	  if (_params.apply_rec) {
	    midBeam->computeRec(_beamQueue, _midBeamIndex);
	    if (_params.filter_clutter_using_rec) {
	      midBeam->filterClutterUsingRec();
	    }
	  }

	  if (_fmq->writeBeam(midBeam, _volNum)) {
	    cerr << "ERROR - Lirp2Dsr::_processFile" << endl;
	    cerr << "  Cannot write the beam data to output FMQ" << endl;
	    fclose(in);
	    return -1;
	  }

	}
	
      } // if (_beamReady())

    } // if (_momentsMgr != NULL)
    
  } // while (!feof(in))
  
  int endOfFile = feof(in);
  fclose(in);
  
  // print noise stats if required
  
  if (_params.compute_noise && _params.debug) {
    _printNoise();
  }
  
  if (!endOfFile) {
    return -1;
  } else {
    return 0;
  }

}

/////////////////////////////////
// check if we have a ROC header
//
// Returns 0 on success, -1 on failure

int Lirp2Dsr::_checkForRocHeader(FILE *in, bool &hasRocHdr)

{
  
  // read in first 4 bytes to see if we have a ROC hdr
  
  UINT4 rocId;
  if (fread(&rocId, sizeof(UINT4), 1, in) != 1) {
    int errNum = errno;
    cerr << "ERROR - Lirp2Dsr::_checkForRocHeader" << endl;
    cerr << "  Cannot read rocId" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  if (rocId == 0x53545652) {
    hasRocHdr = TRUE;
  } else {
    hasRocHdr = false;
  }

  return 0;

}
  
/////////////////////////////////////////////////
// prepare for moments computations
    
void Lirp2Dsr::_prepareForMoments(Pulse *pulse)
  
{

  // compute phase differences between this pulse and previous ones
  // to prepare for cohering to multiple trips
  
  pulse->computePhaseDiffs(_pulseQueue, _maxTrips);
  
  // set properties from pulse
  
  _nGatesPulse = pulse->getNGates();
  double prf = 1.0 / pulse->getPrt();

  // find moments manager, based on PRF
  
  if (fabs(prf - _prevPrfForMoments) > 0.1) {
    _momentsMgr = NULL;
    for (int i = 0; i < (int) _momentsMgrArray.size(); i++) {
      if (prf >= _momentsMgrArray[i]->getLowerPrf() &&
	  prf <= _momentsMgrArray[i]->getUpperPrf()) {
	_momentsMgr = _momentsMgrArray[i];
	_nSamples = _momentsMgr->getNSamples();
	break;
      }
    }
    _prevPrfForMoments = prf;
    if (_momentsMgr != NULL) {
      if (_params.compute_noise && _params.debug) {
	_printNoise();
      }
    }
  } // if (fabs(prf ...

}
    
/////////////////////////////////////////////////
// are we ready for a beam?
//
// Side effects: sets _az, _midIndex1, _midIndex2

bool Lirp2Dsr::_beamReady()
  
{
  
  _countSinceBeam++;
  _az = 0.0;
  
  // enough data in the queue?

  int minPulses = _nSamples;
  if (_momentsMgr->getDualPol()) {
    // need one extra pulse because we sometimes need to search
    // backwards for horizontal pulse
    minPulses++;
  }

  if ((int) _pulseQueue.size() < minPulses) {
    return false;
  }
  
  // check we have constant nGates
  
  int nGates = _pulseQueue[0]->getNGates();
  for (int i = 1; i < _nSamples; i++) {
    if (_pulseQueue[i]->getNGates() != nGates) {
      return false;
    }
  }
  
  // check we have constant prt
  
  double prt = _pulseQueue[0]->getPrt();
  for (int i = 1; i < _nSamples; i++) {
    if (fabs(_pulseQueue[i]->getPrt() - prt) > 0.001) {
      return false;
    }
  }

  // compute the indices at the middle of the beam.
  // index1 is just short of the midpt
  // index2 is just past the midpt
  
  _midIndex1 = _nSamples / 2;
  _midIndex2 = _midIndex1 - 1;

  // compute azimuths which need to straddle the center of the beam
  
  double midAz1 = _pulseQueue[_midIndex1]->getAz();
  double midAz2 = _pulseQueue[_midIndex2]->getAz();

  if (_params.index_beams_in_azimuth) {

    // compute target azimiuth by rounding the azimuth at the
    // center of the data to the closest suitable az
    
    _az = ((int) (midAz1 / _params.azimuth_resolution + 0.5)) *
      _params.azimuth_resolution;
    
    if (_az >= 360.0) {
      _az -= 360;
    } else if (_az < 0) {
      _az += 360.0;
    }
    
    // Check if the azimuths at the center of the data straddle
    // the target azimuth
    
    if (midAz1 <= _az && midAz2 >= _az) {
      
      // az1 is below and az2 above - clockwise rotation
      return true;
      
    } else if (midAz1 >= _az && midAz2 <= _az) {
      
      // az1 is above and az2 below - counterclockwise rotation
      return true;
      
    } else if (_az == 0.0) {
      
      if (midAz1 > 360.0 - _params.azimuth_resolution &&
	  midAz2 < _params.azimuth_resolution) {
	
	// az1 is below 0 and az2 above 0 - clockwise rotation
	return true;
	
      } else if (midAz2 > 360.0 - _params.azimuth_resolution &&
		 midAz1 < _params.azimuth_resolution) {
	
	// az1 is above 0 and az2 below 0 - counterclockwise rotation
	return true;
	
      }
      
    } else if (_countSinceBeam > (_nSamples * 16)) {
      
      // antenna moving very slowly, we have waited long enough
      return true;
      
    }
    
  } else {
    
    // do not index - only check we have enough data
    
    if (_countSinceBeam >= _nSamples) {
      _az = midAz1;
      if (_az >= 360.0) {
	_az -= 360;
      } else if (_az < 0) {
	_az += 360.0;
      }
      return true;
    }
    
  }
  
  return false;

}

/////////////////////////////////////////////////
// compute basic moments for the beam
    
int Lirp2Dsr::_computeBeamMomentsBasic(Beam *beam)
  
{

  // compute moments
  
  beam->computeMoments(_specPrintCount, _spectraFile);
  
  // set elev parameters
  
  if (beam->getEl() < _prevEl && beam->getEl() < _volMinEl) {
    _volMinEl = beam->getEl();
  }
  if (beam->getEl() > _prevEl && beam->getEl() > _volMaxEl) {
    _volMaxEl = beam->getEl();
  }
  
  // search for end of vol
  
  bool endOfVol = false;
  if (_params.set_end_of_vol_from_elev_angle) {
    if (_nBeamsThisVol >= _params.min_beams_per_vol) {
      if (_params.vol_starts_at_bottom) {
	double deltaEl = _volMaxEl - beam->getEl();
	if (deltaEl > _params.elev_change_for_end_of_vol) {
	  endOfVol = true;
	}
      } else {
	double deltaEl = beam->getEl() - _volMinEl;
	if (deltaEl > _params.elev_change_for_end_of_vol) {
	  endOfVol = true;
	}
      }
    }
  }
  if (_params.set_end_of_vol_on_prf_change) {
    if (fabs(beam->getPrf() - _prevPrfForParams) > 1.0) {
      endOfVol = true;
    }
  }
  
  if (endOfVol) {
    
    _fmq->putEndOfVolume(_volNum, beam->getTime());
    _fmq->putStartOfVolume(_volNum + 1, beam->getTime());
    
    _volNum++;
    _volMinEl = 180.0;
    _volMaxEl = -180.0;
    _nBeamsThisVol = 0;
    
  }
  
  // put the params if needed
  
  if (!_paramsSentThisFile ||
      endOfVol ||
      (fabs(beam->getPrf() - _prevPrfForParams) > 1.0) ||
      beam->getNGatesOut() != _prevNGatesForParams) {

    _fmq->setPrf(beam->getPrf());
    _fmq->setNGates(beam->getNGatesOut());
    _fmq->setNSamples(beam->getNSamples());
    _fmq->setPulseWidth(_momentsMgr->getPulseWidth());

    if (_fmq->writeParams(_momentsMgr->getStartRange(),
			  _momentsMgr->getGateSpacing())) {
      cerr << "ERROR - Lirp2Dsr::_computeMoments" << endl;
      cerr << "  Cannot write the params to the queue" << endl;
      return -1;
    }
    
    _prevPrfForParams = beam->getPrf();
    _prevNGatesForParams = beam->getNGatesOut();
    _paramsSentThisFile = true;
    
  }
  
  if (_params.print_summary) {
    fprintf(stdout,
	    "Vol,el,az,ngates,prf,time:");
    DateTime beamTime(beam->getTime());
    fprintf(stdout,
	    "%3d %6.2f %6.2f %4d %4g %s\n",
	    _volNum, beam->getEl(), beam->getAz(),
	    beam->getNGatesOut(), beam->getPrf(),
	    beamTime.getStr(_time).c_str());
    if (_prevAz >= 0) {
      double deltaAz = fabs(beam->getAz() - _prevAz);
      if (deltaAz > 180) {
	deltaAz = fabs(deltaAz - 360.0);
      }
      if (deltaAz > _params.azimuth_resolution) {
	fprintf(stdout, "\a-----> missing beams <-----\n");
      }
    }
  }
  
  _prevAz = beam->getAz();
  _prevEl = beam->getEl();
  
  return 0;

}
	
/////////////////////////////////////////////////
// add the pulse to the pulse queue
    
void Lirp2Dsr::_addPulseToQueue(Pulse *pulse)
  
{

  // manage the size of the pulse queue, popping off the back
  
  if ((int) _pulseQueue.size() > _maxPulseQueueSize) {
    Pulse *oldest = _pulseQueue.back();
    if (oldest->removeClient("Deleting from queue") == 0) {
      delete oldest;
    }
    _pulseQueue.pop_back();
  }
  
  // push pulse onto front of queue
  
  pulse->addClient("Adding to queue");
  _pulseQueue.push_front(pulse);

  // print missing pulses if requested
  
  if ((int) pulse->getSeqNum() != _pulseSeqNum + 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE && _pulseSeqNum != 0) {
      cout << "**************** Missing seq num: " << _pulseSeqNum
	   << " to " <<  pulse->getSeqNum() << " **************" << endl;
    }
  }
  _pulseSeqNum = pulse->getSeqNum();

}

/////////////////////////////////////////////////
// add the beam to the beam queue
    
void Lirp2Dsr::_addBeamToQueue(Beam *beam)
  
{

  // manage the size of the beam queue, popping off the back
  
  if ((int) _beamQueue.size() == _maxBeamQueueSize) {
    delete _beamQueue.back();
    _beamQueue.pop_back();
  }
  
  // push beam onto front of queue
  
  _beamQueue.push_front(beam);


}

////////////////////////////////////////////////////////////////////////
// Filter fields for spikes in dbz
//
// This routine filters the reflectivity data according to the
// NEXRAD specification DV1208621F, section 3.2.1.2.2, page 3-15.
//
// The algorithm is stated as follows:
//
// Clutter detection:
//
// The nth bin is declared to be a point clutter cell if its power value
// exceeds those of both its second nearest neighbors by a threshold
// value TCN. In other words:
//
//    if   P(n) exceeds TCN * P(n-2)
//    and  P(n) exceeds TCN * p(n+2)
//
//  where
//
//   TCN is the point clutter threshold factor, which is always
//       greater than 1, and typically has a value of 8 (9 dB)
//
//   P(n) if the poiwer sum value for the nth range cell
//
//   n is the range gate number
//
// Clutter censoring:
//
// The formulas for censoring detected strong point clutter in an
// arbitrary array A via data substitution are as follows. If the nth
// range cell is an isolated clutter cell (i.e., it si a clutter cell but
// neither of its immediate neighboring cells is a clutter cell) then the 
// replacement scheme is as follows:
//
//   Replace A(n-1) with  A(n-2)
//   Replace A(n)   with  0.5 * A(n-2) * A(n+2)
//   Replace A(n+1) with  A(n+2)
//
// If the nth and (n+1)th range bins constitute an isolated clutter pair,
// the bin replacement scheme is as follows:
//
//   Replace A(n-1) with  A(n-2)
//   Replace A(n)   with  A(n+2)
//   Replace A(n+1) with  A(n+3)
//   Replace A(n+2) with  A(n+3)
//
// Note that runs of more than 2 successive clutter cells cannot occur
// because of the nature of the algorithm.
 
void Lirp2Dsr::_filterSpikes(double *dbzf,
			     double *velf,
			     double *widthf)
  
{
  
  // set clutter threshold

  double tcn = 9.0;

  // loop through gates

  for (int ii = 2; ii < _nGatesOut - 3; ii++) {
    
    // check for clutter at ii and ii + 1

    bool this_gate = false, next_gate = false;
    
    if ((dbzf[ii] - dbzf[ii - 2]) > tcn &&
	(dbzf[ii] - dbzf[ii + 2]) > tcn) {
      this_gate = true;
    }
    if ((dbzf[ii + 1] - dbzf[ii - 1]) > tcn &&
	(dbzf[ii + 1] - dbzf[ii + 3]) > tcn) {
      next_gate = true;
    }

    if (this_gate) {

      if (!next_gate) {

	// only gate ii has clutter, substitute accordingly
	
	dbzf[ii - 1] = dbzf[ii - 2];
	dbzf[ii + 1] = dbzf[ii + 2];
	if (dbzf[ii - 2] == _missingDbl || dbzf[ii + 2] == _missingDbl) {
	  dbzf[ii] = _missingDbl;
	  velf[ii] = _missingDbl;
	  widthf[ii] = _missingDbl;
	} else {
	  // dbzf[ii] = (dbzf[ii - 2] + dbzf[ii + 2]) / 2;
	  dbzf[ii] = dbzf[ii - 2];
	  velf[ii] = velf[ii - 2];
	  widthf[ii] = widthf[ii - 2];
	}
	
      } else {

	// both gate ii and ii+1 has clutter, substitute accordingly

	dbzf[ii - 1] = dbzf[ii - 2];
	dbzf[ii]     = dbzf[ii - 2];
	dbzf[ii + 1] = dbzf[ii + 3];
	dbzf[ii + 2] = dbzf[ii + 3];

	velf[ii - 1] = velf[ii - 2];
	velf[ii]     = velf[ii - 2];
	velf[ii + 1] = velf[ii + 3];
	velf[ii + 2] = velf[ii + 3];

	widthf[ii - 1] = widthf[ii - 2];
	widthf[ii]     = widthf[ii - 2];
	widthf[ii + 1] = widthf[ii + 3];
	widthf[ii + 2] = widthf[ii + 3];

      }

    }
    
  } // ii

}


////////////////////////////////////////////////////////////////////////
// Filter dregs remaining after other filters
 
void Lirp2Dsr::_filterDregs(double nyquist,
			    double *dbzf,
			    double *velf,
			    double *widthf)
  
{
  
  // compute vel diff array
  
  double veld[_nGatesOut];
  veld[0] = 0.0;
  
  for (int ii = 1; ii < _nGatesOut; ii++) {
    double diff;
    if (velf[ii] == _missingDbl || velf[ii -1] == _missingDbl) {
      diff = nyquist / 2.0;
    } else {
      diff = velf[ii] - velf[ii -1];
      if (diff > nyquist) {
	diff -= 2.0 * nyquist;
      } else if (diff < -nyquist) {
	diff += 2.0 * nyquist;
      }
    }
    veld[ii] = diff;
  } // ii
  
  for (int len = 20; len >= 6; len--) {

    int nHalf = len / 2;

    for (int ii = nHalf; ii < _nGatesOut - nHalf; ii++) {
      
      int istart = ii - nHalf;
      int iend = istart + len;
      
      if (dbzf[istart] != _missingDbl || dbzf[iend] != _missingDbl) {
	continue;
      }
      
      double sum = 0.0;
      double count = 0.0;
      int ndata = 0;
      for (int jj = istart; jj <= iend; jj++) {
	sum += veld[jj] * veld[jj];
	count++;
	if (dbzf[jj] != _missingDbl) {
	  ndata++;
	}
      } // jj
      
      if (ndata == 0) {
	continue;
      }

      double texture = sqrt(sum / count) / nyquist;
      double interest = _computeInterest(texture, 0.0, 0.75);

      // cerr << len << ":" << interest << "  ";
      
      if (interest > 0.5) {
	// cerr << " " << istart << "--" << iend << " ";
	for (int jj = istart; jj <= iend; jj++) {
	  //  	  if (dbzf[jj] != _missingDbl) {
	  //  	    cerr << " **" << jj << "** ";
	  //  	  }
	  dbzf[jj] = _missingDbl;
	  velf[jj] = _missingDbl;
	  widthf[jj] = _missingDbl;
	} // jj
      }

    } // ii
    
  } // len
  
}


////////////////////////////////////////////////////////////////////////
// Compute vel censoring interest value

void Lirp2Dsr::_computeVelCensoring(const double *vel,
				    double nyquist,
				    double *vtexture,
				    double *vspin,
				    int *vcensor)
  
{
  
  // compute vel diff array
  
  double veld[_nGatesOut];
  veld[0] = 0.0;

  for (int ii = 1; ii < _nGatesOut; ii++) {
    double diff;
    if (vel[ii] == _missingDbl || vel[ii -1] == _missingDbl) {
      diff = nyquist / 2.0;
    } else {
      diff = vel[ii] - vel[ii -1];
      if (diff > nyquist) {
	diff -= 2.0 * nyquist;
      } else if (diff < -nyquist) {
	diff += 2.0 * nyquist;
      }
    }
    veld[ii] = diff;
  } // ii
  
  int nGates = 5;
  int nGatesHalf = nGates / 2;

  memset(vtexture, 0, _nGatesOut * sizeof(double));
  for (int ii = nGatesHalf; ii < _nGatesOut - nGatesHalf; ii++) {
    double sum = 0.0;
    double count = 0.0;
    for (int jj = ii - nGatesHalf; jj <= ii + nGatesHalf; jj++) {
      sum += veld[jj] * veld[jj];
      count++;
    } // jj
    double texture = sqrt(sum / count) / nyquist;
    // double interest = _computeInterest(texture, 40.0, 70.0);
    double interest = _computeInterest(texture, 0.0, 0.75);
    // double interest = texture;
    vtexture[ii] = interest;
  } // ii
  
  double dtest = nyquist / 4.0;
  memset(vspin, 0, _nGatesOut * sizeof(double));
  for (int ii = nGatesHalf; ii < _nGatesOut - nGatesHalf; ii++) {
    double sum = 0.0;
    double count = 0.0;
    for (int jj = ii - nGatesHalf + 1; jj <= ii + nGatesHalf; jj++) {
      if (vel[ii] == _missingDbl || vel[ii -1] == _missingDbl) {
	sum++;
      } else {
	double mult = veld[jj-1] * veld[jj];
	if (mult < 0 && (fabs(veld[jj-1]) > dtest || fabs(veld[jj]) > dtest)) {
	  sum++;
	}
      }
      count++;
    } // jj
    double spin = sum / count;
    double interest = _computeInterest(spin, 0.0, 1.0);
    // double interest = spin;
    vspin[ii] = interest;
  } // ii

  memset(vcensor, 0, _nGatesOut * sizeof(int));
  for (int ii = 0; ii < _nGatesOut; ii++) {
    if (vtexture[ii] * vspin[ii] > 0.5) {
      vcensor[ii] = 1;
    }
  } // ii

#ifdef JUNK  
  // remove runs which are less than half the nGates

  int nCensor = 0;
  for (int ii = 0; ii < _nGatesOut; ii++) {
    if (ii == _nGatesOut - 1 || vcensor[ii] < 0.5) {
      if (nCensor > 0 && nCensor < nGatesHalf) {
	int iStart = ii - nCensor;
	int iEnd = ii - 1;
	for (int jj = iStart; jj <= iEnd; jj++) {
	  vcensor[jj] = 0.01;
	}
      }
      nCensor = 0;
    } else {
      nCensor++;
    }
  }
#endif
  
}

////////////////////////////////////////////////////////////////////////
// Compute Z texture

void Lirp2Dsr::_computeZTexture(const double *dbz,
				double *ztexture)
  
{
  
  // compute dbz diff array
  
  double dbzd[_nGatesOut];
  dbzd[0] = 0.0;
  int dbzBase = -40;
  
  for (int ii = 1; ii < _nGatesOut; ii++) {
    if (dbz[ii - 1] != _missingDbl && dbz[ii] != _missingDbl) {
      double dbz0 = dbz[ii-1] - dbzBase;
      double dbz1 = dbz[ii] - dbzBase;
      double dbzMean = (dbz0 + dbz1) / 2.0;
      double diff = (dbz1 - dbz0) / dbzMean;
      dbzd[ii] = diff;
    } else {
      dbzd[ii] = _missingDbl;
    }
  } // ii
  
  // compute the z texture

  int nGates = 64;
  int nGatesHalf = nGates / 2;

  // int nGatesThreeQuarters = (nGates * 3) / 4;
  
  for (int ii = 0; ii < nGatesHalf - 1; ii++) {
    ztexture[ii] = _missingDbl;
  }
  for (int ii = _nGatesOut - nGatesHalf; ii < _nGatesOut; ii++) {
    ztexture[ii] = _missingDbl;
  }
  
  for (int ii = nGatesHalf; ii < _nGatesOut - nGatesHalf; ii++) {
    double sum = 0.0;
    double count = 0.0;
    for (int jj = ii - nGatesHalf; jj <= ii + nGatesHalf; jj++) {
      if (dbzd[jj] != _missingDbl) {
	sum += (dbzd[jj] * dbzd[jj]);
	count++;
      } // jj
    }
    double texture;
    if (count < nGatesHalf) {
      texture = _missingDbl;
    } else {
      texture = sqrt(sum / count);
    }
    // double interest = _computeInterest(texture, 40.0, 70.0);
    double interest = texture;
    ztexture[ii] = interest;
  } // ii

}

////////////////////////////////////////////////////////////////////////
// Perform infilling

void Lirp2Dsr::_performInfilling(const double *dbzf,
				 const double *velf,
				 const double *widthf,
				 const double *ztexture,
				 int *zinfill,
				 double *dbzi,
				 double *veli,
				 double *widthi)
  
{
  
  // set the infill flag

  for (int ii = 0; ii < _nGatesOut; ii++) {
    if (dbzf[ii] == _missingDbl &&
	ztexture[ii] != _missingDbl &&
	ztexture[ii] < 0.1) {
      zinfill[ii] = 1;
    } else {
      zinfill[ii] = 0;
    }
    // }
  }

  // perform infilling

  memcpy(dbzi, dbzf, _nGatesOut * sizeof(double));
  memcpy(veli, velf, _nGatesOut * sizeof(double));
  memcpy(widthi, widthf, _nGatesOut * sizeof(double));

  int nInfill = 0;
  for (int ii = 0; ii < _nGatesOut; ii++) {
    if (ii == _nGatesOut - 1 || zinfill[ii] == 0) {
      if (nInfill > 0) {
	int nHalf = nInfill / 2;
	int iLowStart = ii - nInfill;
	int iLowEnd = iLowStart + nHalf - 1;
	int iHighStart = iLowEnd + 1;
	int iHighEnd = ii - 1;
	for (int jj = iLowStart; jj <= iLowEnd; jj++) {
	  dbzi[jj] = dbzf[iLowStart - 1];
	  veli[jj] = velf[iLowStart - 1];
	  widthi[jj] = widthf[iLowStart - 1];
	}
	for (int jj = iHighStart; jj <= iHighEnd; jj++) {
	  dbzi[jj] = dbzf[ii];
	  veli[jj] = velf[ii];
	  widthi[jj] = widthf[ii];
	}
      }
      nInfill = 0;
    } else {
      nInfill++;
    }
  }  

}


////////////////////////////////////////////////////////////////////////
// Compute interest value

double Lirp2Dsr::_computeInterest(double xx,
				  double x0, double x1)
  
{

  if (xx <= x0) {
    return 0.01;
  }
  
  if (xx >= x1) {
    return 0.99;
  }
  
  double xbar = (x0 + x1) / 2.0;
  
  if (xx <= xbar) {
    double yy = (xx - x0) / (x1 - x0);
    double yy2 = yy * yy;
    double interest = 2.0 * yy2;
    return interest;
  } else {
    double yy = (x1 - xx) / (x1 - x0);
    double yy2 = yy * yy;
    double interest = 1.0 - 2.0 * yy2;
    return interest;
  }

}

////////////////////////////////////////////////////////////////////////
// print noise

void Lirp2Dsr::_printNoise()
  
{
  cerr << "Measured noise value (dBm): "
       << _momentsMgr->getMeasuredNoiseDbm() << endl;
}
  
