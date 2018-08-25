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
// TsArchive2Dsr.cc
//
// TsArchive2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// TsArchive2Dsr reads raw Lirp IQ time-series data, computes the
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
#include <Spdb/DsSpdb.hh>
#define in_TsArchive2Dsr_cc
#include "TsArchive2Dsr.hh"
#undef in_TsArchive2Dsr_cc
using namespace std;

////////////////////////////////////////////////////
// Constructor

TsArchive2Dsr::TsArchive2Dsr(int argc, char **argv)

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
  _volNum = -1;
  _tiltNum = -1;
  _currentSweepValid = false;
  _guessingSweepInfo = false;
  _prevSweepEndTime = 0;

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

  _opsInfo = NULL;
  
  isOK = true;

  // set programe name

  _progName = "TsArchive2Dsr";
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
    cerr << "ERROR: TsArchive2Dsr::TsArchive2Dsr." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
    return;
  }
    
  if (_params.mode == Params::SIMULATE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: TsArchive2Dsr::TsArchive2Dsr." << endl;
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

  // create OpsInfo object

  _opsInfo = new OpsInfo(_params);

  // set up moments objects
  // This initializes the FFT package to the set number of samples.

  for (int i = 0; i < _params.moments_params_n; i++) {
    Params::moments_params_t &mparams = _params._moments_params[i];
    int nSamples = mparams.n_samples;
    if (mparams.apply_sz) {
      nSamples = _nSamplesSz;
    }
    MomentsMgr *mgr = new MomentsMgr(_progName, _params, *_opsInfo,
				     _params._moments_params[i],
				     nSamples, _maxGates);
    _momentsMgrArray.push_back(mgr);
  }
  if (_momentsMgrArray.size() < 1) {
    cerr << "ERROR: TsArchive2Dsr::TsArchive2Dsr." << endl;
    cerr << "  No algorithm geometry specified."; 
    cerr << "  The param moments_menuetry must have at least 1 entry."
	 << endl;
    isOK = false;
    return;
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
  
  _fmq = new OutputFmq(_progName, _params, *_opsInfo);
  if (!_fmq->isOK) {
    isOK = false;
    return;
  }

  // open combined spectra file if required

  if (_params.write_combined_spectra_file) {
    if (ta_makedir_recurse(_params.spectra_dir)) {
      cerr << "ERROR - TsArchive2Dsr" << endl;
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
      cerr << "ERROR - TsArchive2Dsr" << endl;
      cerr << "  Cannot open combined spectra file: "
	   << _spectraFilePath << endl;
      cerr << "  " << strerror(errNum) << endl;
      isOK = false;
      return;
    }
  }

  // REC initialization

  if (_params.index_beams_in_azimuth) {
    _applyCmd = _params.apply_cmd;
  } else {
    _applyCmd = false;
  }

  if (_applyCmd) {

    _cmdKernelWidth = _params.cmd_kernel_nbeams;
    if (_cmdKernelWidth % 2 == 0) {
      // make sure we have odd number of beams
      _cmdKernelWidth++;
    }
    if (Beam::createInterestMaps(_params)) {
      cerr << "ERROR - TsArchive2Dsr" << endl;
      cerr << "  Problems with params for CMD interest maps" << endl;
      isOK = false;
      return;
    }
    _maxBeamQueueSize = _cmdKernelWidth;

  } else {

    _cmdKernelWidth = 1;
    _maxBeamQueueSize = 1;

    // make sure output parameters are consistent

    _params.apply_cmd = pFALSE;
    _params.output_fields.cmd = pFALSE;
    
  }

  _midBeamIndex = _maxBeamQueueSize / 2;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "_cmdKernelWidth: " << _cmdKernelWidth << endl;
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

TsArchive2Dsr::~TsArchive2Dsr()

{

  if (_input) {
    delete _input;
  }

  if (_opsInfo) {
    delete _opsInfo;
  }

  if (_spectraFile != NULL) {
    fclose(_spectraFile);
  }

  for (size_t ii = 0; ii < _momentsMgrArray.size(); ii++) {
    delete _momentsMgrArray[ii];
  }
  _momentsMgrArray.clear();

  for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
    if (_pulseQueue[ii]->removeClient("TsArchive2Dsr destructor") == 0) {
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

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TsArchive2Dsr::Run ()
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
	  cerr << "ERROR - TsArchive2Dsr::Run" << endl;
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
	cerr << "ERROR - TsArchive2Dsr::Run" << endl;
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

int TsArchive2Dsr::_processFile(const char *input_path)

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
    cerr << "ERROR - TsArchive2Dsr::_processFile" << endl;
    cerr << "  Cannot open file: " << input_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read in ops info

  if (_opsInfo->read(in)) {
    cerr << "ERROR - TsRefract2Dsr::_processFile" << endl;
    cerr << "  Cannot read pulse info" << endl;
    cerr << "  File: " << input_path << endl;
    fclose(in);
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _opsInfo->print(cerr);
  }
  
  // read in data, load up pulses

  while (!feof(in)) {
    
    // Create a new pulse object and save a pointer to it in the
    // _pulseBuffer array.  _pulseBuffer is a FIFO, with elements
    // added at the end and dropped off the beginning. So if we have a
    // full buffer delete the first element before shifting the
    // elements to the left.
    
    Pulse *pulse = new Pulse(_params, _opsInfo->getClockMhz());
    
    // read in pulse headers and data
    
    if (pulse->read(in, *_opsInfo)) {
      if (_params.debug && !feof(in)) {
	cerr << "ERROR - TsArchive2Dsr::_processFile" << endl;
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
	  
	  // compute the CMD for the beam
	  
	  if (_params.apply_cmd) {
	    midBeam->computeCmd(_beamQueue, _midBeamIndex);
            midBeam->filterClutterUsingCmd();
	  }

	  if (_fmq->writeBeam(midBeam, _volNum, _tiltNum)) {
	    cerr << "ERROR - TsArchive2Dsr::_processFile" << endl;
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
  
  if (!endOfFile) {
    return -1;
  } else {
    return 0;
  }

}

/////////////////////////////////////////////////
// prepare for moments computations
    
void TsArchive2Dsr::_prepareForMoments(Pulse *pulse)
  
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
  } // if (fabs(prf ...

}
    
/////////////////////////////////////////////////
// are we ready for a beam?
//
// Side effects: sets _az, _midIndex1, _midIndex2

bool TsArchive2Dsr::_beamReady()
  
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
    
int TsArchive2Dsr::_computeBeamMomentsBasic(Beam *beam)
  
{
  
  // compute moments
  
  beam->computeMoments(_specPrintCount, _spectraFile);

  // set the sweep information
  // side effect: puts start/end of vol flags to FMQ

  bool endOfVol;
  _setSweepInfo(beam, endOfVol);

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
      cerr << "ERROR - TsArchive2Dsr::_computeMoments" << endl;
      cerr << "  Cannot write the params to the queue" << endl;
      return -1;
    }
    
    _prevPrfForParams = beam->getPrf();
    _prevNGatesForParams = beam->getNGatesOut();
    _paramsSentThisFile = true;
    
  }

  bool azMissing = false;
  if (_prevAz >= 0) {
    double deltaAz = fabs(beam->getAz() - _prevAz);
    if (deltaAz > 180) {
      deltaAz = fabs(deltaAz - 360.0);
    }
    if (deltaAz > _params.azimuth_resolution) {
      azMissing = true;
    }
  }

  if (_params.debug) {
    if (azMissing) {
      cerr << "Azimuths missing, prev: " << _prevAz
           << ", this: " << beam->getAz() << endl;
    }
  }
  
  _prevAz = beam->getAz();
  _prevEl = beam->getEl();
  
  return 0;

}
	
/////////////////////////////////////////////////
// add the pulse to the pulse queue
    
void TsArchive2Dsr::_addPulseToQueue(Pulse *pulse)
  
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
    
void TsArchive2Dsr::_addBeamToQueue(Beam *beam)
  
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
// set sweep information
//
// side effects:
//  puts start/end of vol flags to FMQ
//  sets endOfVol

void TsArchive2Dsr::_setSweepInfo(Beam *beam, bool &endOfVol)
  
{

  endOfVol = false;

  // if requested, read sweep info from SPDB
  // we will wait until we have sweep info, or 
  // we time out and use the guesses for vol and tilt number
  
  bool gotCurrentSweep = false;
  int volNum, tiltNum;
  if (_params.read_sweep_info_from_spdb) {
    if (_readSweepInfo(beam, volNum, tiltNum) == 0) {
      gotCurrentSweep = true;
    }
  }

  if (gotCurrentSweep) {
    
    if (volNum != _volNum) {
      
      endOfVol = true;
      _fmq->putEndOfTilt(_tiltNum, beam->getTime());
      _fmq->putEndOfVolume(_volNum, beam->getTime());
      _volNum = volNum;
      _tiltNum = tiltNum;
      _fmq->putStartOfVolume(_volNum, beam->getTime());
      _fmq->putStartOfTilt(_tiltNum, beam->getTime());

      _volMinEl = 180.0;
      _volMaxEl = -180.0;
      _nBeamsThisVol = 0;

    } else if (tiltNum != _tiltNum) {
      
      _fmq->putEndOfTilt(_tiltNum, beam->getTime());
      _tiltNum = tiltNum;
      _fmq->putStartOfTilt(_tiltNum, beam->getTime());

    }

  } else { // if (gotCurrentSweep)

    // set tilt number to missing

    _tiltNum = -1;
    
    // set elev stats
    
    if (beam->getEl() < _prevEl && beam->getEl() < _volMinEl) {
      _volMinEl = beam->getEl();
    }
    if (beam->getEl() > _prevEl && beam->getEl() > _volMaxEl) {
      _volMaxEl = beam->getEl();
    }
    
    // guess at end of vol condition
    
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
      _volNum++;
      _fmq->putStartOfVolume(_volNum, beam->getTime());
      
      _volMinEl = 180.0;
      _volMaxEl = -180.0;
      _nBeamsThisVol = 0;
      
    }

  } // if (gotCurrentSweep ...

}

////////////////////////////////////////////////////////////////////////
// read sweep information
//
// returns 0 on success, -1 on failure

int TsArchive2Dsr::_readSweepInfo(Beam *beam, int &volNum, int &tiltNum)
  
{

  // is the beam still within the current sweep?

  if (_currentSweepValid) {
    if (_beamWithinSweep(beam, _currentSweep)) {
      volNum = _currentSweep.getVolNum();
      tiltNum = _currentSweep.getTiltNum();
      beam->setTargetEl(_currentSweep.getFixedEl());
      return 0;
    }
  }

  if (_params.debug && !_guessingSweepInfo) {
    cerr << "Searching for new sweep info" << endl;
  }

  time_t startWait = time(NULL);

  while (true) {

    // read sweep info from SPDB

    DsSpdb spdb;
    time_t searchStart = beam->getTime() - _params.sweep_search_margin;
    time_t searchEnd = beam->getTime() + _params.sweep_search_margin;

    if (spdb.getInterval(_params.sweep_url,
                         searchStart, searchEnd) == 0) {

      // look through available chunks
      
      const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
      for (int ii = 0; ii < (int) chunks.size(); ii++) {
        DsRadarSweep sweep;
        if (sweep.disassemble(chunks[ii].data, chunks[ii].len) == 0) {
          // good sweep info
          if (_beamWithinSweep(beam, sweep)) {
            // beam is within sweep time, so accept it and return
            volNum = sweep.getVolNum();
            tiltNum = sweep.getTiltNum();
            if (_currentSweepValid) {
              _prevSweepEndTime = _currentSweep.getEndTime();
            }
            _currentSweep = sweep;
            _currentSweepValid = true;
            _guessingSweepInfo = false;
            beam->setTargetEl(_currentSweep.getFixedEl());
            if (_params.debug) {
              cerr << "Found a new sweep" << endl;
              cerr << "  Setting volNum, tiltNum: "
                   << volNum << ", " << tiltNum << endl;
              _currentSweep.print(cerr, "  ");
            }
            return 0;
          }
        }
      } // ii - chunks

    } // getInterval()

    // if guessing, continue guessing

    if (_params.mode == Params::ARCHIVE || _guessingSweepInfo) {
      return -1;
    }

    // no sweep info yet available, sleep a bit

    umsleep(1000);
    PMU_auto_register("Waiting for sweep info ...");
    time_t now = time(NULL);
    if ((now - startWait) > _params.sweep_info_wait_secs) {
      // if (_params.debug >= Params::DEBUG_VERBOSE) {
      if (_params.debug) {
        cerr << "Failed to find sweep info" << endl;
        cerr << "  Will guess on volume number" << endl;
      }
      _guessingSweepInfo = true;
      _currentSweepValid = false;
      return -1;
    }

  } // while

  return -1;

}

////////////////////////////////////////////////////////////////////////
// does beam fall within a sweep?

bool TsArchive2Dsr::_beamWithinSweep(const Beam *beam,
                                     const DsRadarSweep &sweep)
  
{
  double beamTime = beam->getDoubleTime();
  if (_params.strict_sweep_times) {
    if (beamTime > sweep.getStartTime() &&
        beamTime <= sweep.getEndTime()) {
      return true;
    }
  } else {
    if (beamTime > _prevSweepEndTime &&
        beamTime <= sweep.getEndTime()) {
      return true;
    }
  }
  return false;
}

