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
// TsRefract2Dsr.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////
//
// TsRefract2Dsr reads time-series data in Sigmet TsArchive
// format, computes the basic moments and refractivity quantities,
// and writes the results into a DsRadar FMQ.
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
#include "TsRefract2Dsr.hh"
using namespace std;

const double TsRefract2Dsr::_missingDbl = -9999.0;

////////////////////////////////////////////////////
// Constructor

TsRefract2Dsr::TsRefract2Dsr(int argc, char **argv)

{

  _input = NULL;
  _fmq = NULL;
  _momentsMgr = NULL;
  
  _nSamples = 64;
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
  
  _volMinEl = 180.0;
  _volMaxEl = -180.0;
  _nBeamsThisVol = 0;

  _prevPrfForParams = -1;
  _nGatesPulse = 0;
  _nGatesOut = 0;
  _prevNGatesForParams = -1;
  _paramsSentThisFile = false;
  
  _opsInfo = NULL;
  
  isOK = true;

  // set programe name

  _progName = "TsRefract2Dsr";
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
    cerr << "ERROR: TsRefract2Dsr::TsRefract2Dsr." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
    return;
  }
    
  if (_params.mode == Params::SIMULATE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: TsRefract2Dsr::TsRefract2Dsr." << endl;
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
  
  _momentsMgr = new MomentsMgr(_progName, _params, *_opsInfo);

  // compute pulse queue size, set to 2 * maxNsamples
  
  _nSamples = _params.n_samples;
  _maxPulseQueueSize = _nSamples * 2 + 2;
  
  // create the output queue
  
  _fmq = new OutputFmq(_progName, _params, *_opsInfo);
  if (!_fmq->isOK) {
    isOK = false;
    return;
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

//////////////////////////////////////////////////////////////////
// destructor

TsRefract2Dsr::~TsRefract2Dsr()

{

  if (_input) {
    delete _input;
  }

  if (_momentsMgr) {
    delete _momentsMgr;
  }
  
  for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
    if (_pulseQueue[ii]->removeClient("TsRefract2Dsr destructor") == 0) {
      delete _pulseQueue[ii];
    }
  } // ii
  _pulseQueue.clear();

  if (_fmq) {
    delete _fmq;
  }

  if (_opsInfo) {
    delete _opsInfo;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TsRefract2Dsr::Run ()
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
	  cerr << "ERROR - TsRefract2Dsr::Run" << endl;
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
	cerr << "ERROR - TsRefract2Dsr::Run" << endl;
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

int TsRefract2Dsr::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }
  _paramsSentThisFile = false;

  // open file
  
  FILE *in;
  if ((in = ta_fopen_uncompress(input_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsRefract2Dsr::_processFile" << endl;
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
    
    if (pulse->read(in)) {
      if (_params.debug && !feof(in)) {
	cerr << "ERROR - TsRefract2Dsr::_processFile" << endl;
	cerr << "  Cannot read in pulse headers and data" << endl;
	cerr << "  File: " << input_path << endl;
      }
      delete pulse;
      break;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      pulse->print(cerr);
    }
  
    // add pulse to queue, managing memory appropriately
    
    _addPulseToQueue(pulse);

    // prepare for moments computations
    
    _prepareForMoments(pulse);
  
    // is a beam ready?
    
    if (_beamReady()) {
      
      _countSinceBeam = 0;
      _nBeamsThisVol++;
      
      // create new beam
      
      Beam *beam = new Beam(_progName, _params,
                            _pulseQueue, _az, _momentsMgr);
      
      _nGatesOut = beam->getNGatesOut();
      
      // compute the basic beam
      
      _computeBeamMoments(beam);
      
      // write beam
      
      if (_fmq->writeBeam(beam, _volNum, _tiltNum)) {
        cerr << "ERROR - TsRefract2Dsr::_processFile" << endl;
        cerr << "  Cannot write the beam data to output FMQ" << endl;
        fclose(in);
        delete beam;
        return -1;
      }

      // clean up

      delete beam;

    } // if (_beamReady())
    
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
    
void TsRefract2Dsr::_prepareForMoments(Pulse *pulse)
  
{

  // compute phase differences between this pulse and previous ones
  // to prepare for cohering to multiple trips
  
  // pulse->computePhaseDiffs(_pulseQueue, 4);
  
  // set properties from pulse
  
  _nGatesPulse = pulse->getNGates();
  
}
    
/////////////////////////////////////////////////
// are we ready for a beam?
//
// Side effects: sets _az, _midIndex1, _midIndex2

bool TsRefract2Dsr::_beamReady()
  
{
  
  _countSinceBeam++;
  _az = 0.0;
  
  // enough data in the queue?

  int minPulses = _nSamples;
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
    
int TsRefract2Dsr::_computeBeamMoments(Beam *beam)
  
{

  // compute moments
  
  beam->computeMoments();
  
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
    _fmq->setNSamples(_nSamples);
    _fmq->setPulseWidth(_opsInfo->getPulseWidthUs());

    if (_fmq->writeParams(_opsInfo->getStartRange(),
			  _opsInfo->getGateSpacing())) {
      cerr << "ERROR - TsRefract2Dsr::_computeMoments" << endl;
      cerr << "  Cannot write the params to the queue" << endl;
      return -1;
    }
    
    _prevPrfForParams = beam->getPrf();
    _prevNGatesForParams = beam->getNGatesOut();
    _paramsSentThisFile = true;
    
  }
  
  _prevAz = beam->getAz();
  _prevEl = beam->getEl();
  
  return 0;

}
	
/////////////////////////////////////////////////
// add the pulse to the pulse queue
    
void TsRefract2Dsr::_addPulseToQueue(Pulse *pulse)
  
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

////////////////////////////////////////////////////////////////////////
// set sweep information
//
// side effects:
//  puts start/end of vol flags to FMQ
//  sets endOfVol

void TsRefract2Dsr::_setSweepInfo(Beam *beam, bool &endOfVol)
  
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
    
    if (_params.set_end_of_vol_from_elev_change) {
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

int TsRefract2Dsr::_readSweepInfo(Beam *beam, int &volNum, int &tiltNum)
  
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
    cerr << "Seaching for new sweep info" << endl;
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

    if (_guessingSweepInfo) {
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
// does a time fall within a sweep?

bool TsRefract2Dsr::_beamWithinSweep(const Beam *beam,
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
