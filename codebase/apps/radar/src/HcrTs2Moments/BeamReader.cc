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
// BeamReader.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2007
//
///////////////////////////////////////////////////////////////
//
// Reads in pulses, creates beams.
//
////////////////////////////////////////////////////////////////

#include <vector>
#include <algorithm>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <Radx/RadxTime.hh>
#include "BeamReader.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

BeamReader::BeamReader(const string &prog_name,
                       const Params &params,
                       const Args &args,
                       deque<Beam *> &beamRecyclePool,
                       pthread_mutex_t &beamRecyclePoolMutex) :
        constructorOK(true),
        _progName(prog_name),
        _params(params),
        _args(args),
        _beamRecyclePool(beamRecyclePool),
        _beamRecyclePoolMutex(beamRecyclePoolMutex),
        _pulseReader(NULL),
        _startTime(args.startTime),
        _endTime(args.endTime),

        // Create the pulse pool, starting with 0 pulses, and providing a custom
        // pulse factory function (since IwrfTsPulse has no default constructor).
        _pulsePool(0, std::function<IwrfTsPulse*()>([this]() {
          return new IwrfTsPulse(_pulseReader->getOpsInfo());
        })),

        _pulseSeqNum(0),
        _prevPulseSeqNum(0),
        _prevPulse(NULL),
        _latestPulse(NULL),
        _pulseQueue(),
        _pulseCache(),
        _pulseCount(0),
        _pulseCountSinceStatus(0),
        _beamCount(0),
        _midIndex(0),
        _startIndex(0),
        _endIndex(0),
        _prevBeamPulseSeqNum(0),
        _scanType(Beam::SCAN_TYPE_UNKNOWN),
        _nGates(0),
        _nSamples(100),
        _az(0.0),
        _el(0.0),
        _prt(0.001),
        _meanPrf(1000.0),
        _pulseWidthUs(1.0)

{

  // check that file list set in FILELIST mode
  
  vector<string> inputPathList;
  if (_params.mode == Params::FILELIST && args.inputFileList.size() == 0) {
    cerr << "ERROR: BeamReader::BeamReader." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    args.usage(_progName, cerr);
    constructorOK = false;
    return;
  } else {
    inputPathList = args.inputFileList;
  }
    
  // check that file list set in SIMULATE mode
  
  if (_params.mode == Params::SIMULATE && args.inputFileList.size() == 0) {
    cerr << "ERROR: BeamReader::BeamReader." << endl;
    cerr << "  Mode is SIMULATE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    args.usage(_progName, cerr);
    constructorOK = false;
    return;
  } else {
    inputPathList = args.inputFileList;
  }

  // in ARCHIVE mode, check we can get a file list
  
  if (_params.mode == Params::ARCHIVE) {
    DsInputPath dsInput(_progName, _params.debug >= Params::DEBUG_VERBOSE,
                        _params.input_dir,
                        _startTime, _endTime);
    inputPathList = dsInput.getPathList();
    if (inputPathList.size() < 0) {
      cerr << "ERROR: BeamReader::BeamReader." << endl;
      cerr << "  Mode is ARCHIVE."; 
      cerr << "  No files found in input_dir: " << _params.input_dir << endl;
      cerr << "    start_time: " << DateTime::strm(_args.startTime) << endl;
      cerr << "    end_time: " << DateTime::strm(_args.endTime) << endl;
      args.usage(_progName, cerr);
      constructorOK = false;
      return;
    }
  }
    
  // create the pulse reader
  
  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrfDebug = IWRF_DEBUG_NORM;
  } 
    
  if (_params.mode == Params::REALTIME) {
    _pulseReader = new IwrfTsReaderFmq(_params.input_fmq,
                                       iwrfDebug,
                                       _params.position_fmq_at_start);
  } else {
    _pulseReader = new IwrfTsReaderFile(inputPathList, iwrfDebug);
  }

  if (_params.use_secondary_georeference) {
    _pulseReader->setGeorefUseSecondary(true);
  }

  _pulseReader->setGeorefTimeMarginSecs(_params.georef_time_margin_secs);
  
}

//////////////////////////////////////////////////////////////////
// destructor

BeamReader::~BeamReader()

{
  
  if (_pulseReader) {
    delete _pulseReader;
  }

}

//////////////////////////////////////////////////
// get the next beam
// returns Beam object pointer on success, NULL on failure
// caller must free beam

Beam *BeamReader::getNextBeam()
  
{

  // if first time, initialize the queue

  if (_pulseQueue.size() == 0) {
    if (_initializeQueue()) {
      // no data
      return NULL;
    }
  }

  do { // loop here until the beam checks out as OK

    PMU_auto_register("Reading beam");
    
    // read a single pulse, so we can determine what mode is current
    // this pulse will be reused later
    // reading this pulse will set class members in terms of
    // indexing, dual-prt and other details, which we need to
    // decide how to assemble the beam
    
    if (_getNextPulse() == NULL) {
      // no data
      return NULL;
    }
    
    // save this latest pulse back to the cache
    // so it will be available to read again
    
    _cacheLatestPulse();
    
    // choose action depending on mode
    
    if (_params.dwell_method == Params::DWELL_PULSE_WIDTH_CHANGE) {
      
      if (_readPulseWidthChangeBeam()) {
        // end of data
        return NULL;
      }
      
    } else if (_params.dwell_method == Params::DWELL_SPECIFY_BLOCK) {
      
      // indexed beams
      
      if (_readBlockBeam()) {
        // end of data
        return NULL;
      }
      
    } else {
      
      // beam with n samples specified
      
      if (_readNSamplesBeam()) {
        // end of data
        return NULL;
      }
      
    } // if (_params.dwell_method ...

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      RadxTime ptime(_pulseQueue[0]->getTime(), _pulseQueue[0]->getNanoSecs() / 1.0e9);
      cerr << "  pulse0 time, el, az: "
           << ptime.asString(6) << ", "
           << _pulseQueue[0]->getEl() << ", " << _pulseQueue[0]->getAz() << endl;
    }

  } while (!_beamOk());

  // set nGates
  // in staggered mode, this is the short-prt nGates
  
  _nGates = _computeMinNGates();
  
  // pulse width
    
  _pulseWidthUs = _pulseQueue[_midIndex]->getPulseWidthUs();
  _beamCount++;
  
  // load the pulse shared_ptr-s into a vector for just this beam
  // reversing the order, so we start with oldest pulse
      
  vector<shared_ptr<IwrfTsPulse>> beamPulses;
  for (int ii = _startIndex; ii >= _endIndex; ii--) {
    beamPulses.push_back(_pulseQueue[ii]);
  }
  
  // set processing options
  
  iwrf_xmit_rcv_mode_t xmitRcvMode = IWRF_V_ONLY_FIXED_HV;
  if (_params.xmit_rcv_mode == Params::DP_H_ONLY_FIXED_HV) {
    xmitRcvMode = IWRF_H_ONLY_FIXED_HV;
  }
  
  // get a beam from the pool if available,
  // otherwise create a new one
      
  Beam *beam = NULL;
  pthread_mutex_lock(&_beamRecyclePoolMutex);
  if (_beamRecyclePool.size() > 0) {
    beam = _beamRecyclePool.back();
    _beamRecyclePool.pop_back();
  } else {
    beam = new Beam(_progName, _params);
  }
  pthread_mutex_unlock(&_beamRecyclePoolMutex);

  // set scan type
  
  _scanType = Beam::SCAN_TYPE_UNKNOWN;
  if (fabs(_progressiveElRate) < 0.25) {
    if (fabs(_el - 90.0) < 0.5 || fabs(_el - -90) < 0.5) {
      _scanType = Beam::SCAN_TYPE_VERT;
    } else {
      _scanType = Beam::SCAN_TYPE_POINT;
    }
  } else {
    _scanType = Beam::SCAN_TYPE_RHI;
  }
  
  // initialize the beam
  
  beam->init(_nSamples,
             _nGates,
             _pulseWidthUs,
             _prt,
             _el,
             _az,
             _progressiveElRate,
             _scanType,
             xmitRcvMode,
             _blockId,
             _blockName,
             _pulseReader->getOpsInfo(),
             beamPulses);

  double startAz = _conditionAz(_pulseQueue[_startIndex]->getAz());
  double startEl = _conditionEl(_pulseQueue[_startIndex]->getEl());

  double endAz = _conditionAz(_pulseQueue[_endIndex]->getAz());
  double endEl = _conditionEl(_pulseQueue[_endIndex]->getEl());
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, 
            "==>> New beam, el: mid, start, end, az: mid, start, end, nSamp: "
            "%7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %4d\n",
            _el, startEl, endEl,
            _az, startAz, endAz,
            _nSamples);
    cerr << "  xmitRcvMode: "
         << iwrf_xmit_rcv_mode_to_str(xmitRcvMode) << endl;
  }

  string statusXml = getOpsInfo().getStatusXmlStr();
  if (statusXml.size() > 0) {
    beam->setStatusXml(statusXml);
  }
  
  // move excess pulses to recycle pool
  
  _recyclePulses();
  
  return beam;

}

//////////////////////////////////////////////////////////
// initialize the queue with a few pulses to get going
// returns 0 on success, -1 on failure

int BeamReader::_initializeQueue()
  
{

  size_t nStart = 64;
  while (_pulseQueue.size() < nStart) {
    if (_getNextPulse() == NULL) {
      // end of data
      return -1;
    }
  } // while

  // we will start with the pulses at the end

  _prevBeamPulseSeqNum = _pulseQueue[0]->getPulseSeqNum();
  
  return 0;

}

//////////////////////////////////////////////////
// read in data for a beam of nsamples
// returns 0 on success, -1 on failure

int BeamReader::_readNSamplesBeam()
  
{

  // get _nSamples, ensure it is even

  _nSamples = (_params.dwell_n_samples / 2) * 2;

  // read in _nSamples pulses, adding to queue
  
  for (int ii = 0; ii < _nSamples; ii++) {
    if (_getNextPulse() == NULL) {
      // end of data
      _beamError = true;
      return -1;
    }
  }

  // finalize the beam for use

  if (_finalizeNSamplesBeam()) {
    _beamError = true;
    return 0;
  }

  _beamError = false;
  return 0;

}
    
//////////////////////////////////////////////////
// read in data for a beam with constant pulse
// width, for a case where the pulse widths
// are changing dwell to dwell.
// returns 0 on success, -1 on failure

int BeamReader::_readPulseWidthChangeBeam()
  
{

  // read in pulses until PRT changes
  
  int nPulsesInDwell = 0;
  int warningCount = 0;
  double pulseWidthUs = -9999;
  bool pulseWidthChange = false;
  
  while (nPulsesInDwell <= _params.max_n_samples) {
    
    // get a pulse
    shared_ptr<IwrfTsPulse> pulse = _getNextPulse();
    if (pulse == NULL) {
      // end of data
      _beamError = true;
      return -1;
    }
    warningCount++;
    
    // save pulse width from first pulse
    
    if (nPulsesInDwell == 0) {
      // check for valid pulse width
      if (fabs(_params.dwell_pulse_width_us - pulse->getPulseWidthUs()) > 2.0e-3) {
        // _getNextPulse() automatically inserted this pulse at the front of
        // _pulseQueue, but we don't want it! Remove it from the deque now.
        _pulseQueue.pop_front();
        if (warningCount == _params.max_n_samples * 10) {
          cerr << "WARNING - " << warningCount << " consecutive pulses with width != "
               << _params.dwell_pulse_width_us << " us" << endl;
          warningCount = 0;
        }
        // Go back to get a shared pointer to the next pulse
        continue;
      }
    }
    pulseWidthUs = pulse->getPulseWidthUs();
    
    // check if pulse width has changed
    
    if (fabs(pulseWidthUs - pulse->getPulseWidthUs()) > 2.0e-3 && nPulsesInDwell > 1) {
      pulseWidthChange = true;
      _cacheLatestPulse(); // save for start of next beam
      break;
    }
    
    nPulsesInDwell++;
    
  } // while

  if (!pulseWidthChange) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - BeamReader::_readPulseWidthChangeBeam" << endl;
      cerr << "  Did not find pulse width change" << endl;
    }
    _beamError = true;
    return 0;
  }
  
  // set PRT (and mean PRF)

  _setPrt();

  // set number of samples, ensuring an even number of pulses
  // by dropping one pulse if needed
  
  _nSamples = (nPulsesInDwell / 2) * 2;
  _midIndex = _nSamples / 2;

  _startIndex = _nSamples - 1;
  _endIndex = 0;

  _az = _conditionAz(_pulseQueue[_midIndex]->getAz());
  _el = _conditionEl(_pulseQueue[_midIndex]->getEl());

  _computeBeamElRate(0, _nSamples);

  // save sequence numbers in case we switch to indexed search
  
  _prevBeamPulseSeqNum = _pulseQueue[_midIndex]->getPulseSeqNum();
  
  _beamError = false;
  return 0;

}
    
//////////////////////////////////////////////////
// read in data for a beam with specified block
// returns 0 on success, -1 on failure

int BeamReader::_readBlockBeam()
  
{

  // read in pulses until PRT changes
  
  int nPulsesInDwell = 0;
  int blockId = 0;
  double pulseWidthUs = -9999.0;
  double prt = -9999.0;
  bool foundStartOfBlock = false;
  
  while (nPulsesInDwell <= _params.max_n_samples) {

    // get a pulse
    // this adds the pulse to the queue
    shared_ptr<IwrfTsPulse> pulse = _getNextPulse();
    if (pulse == NULL) {
      // end of data
      _beamError = true;
      return -1;
    }
    
    // read pulses until we get the start of a block
    
    if (!foundStartOfBlock) {
      if (pulse->get_start_of_block()) {
        // set details for this block
        foundStartOfBlock = true;
        blockId = pulse->getBlockId();
        pulseWidthUs = pulse->getPulseWidthUs();
        prt = pulse->getPrt();
      } else {
        // clear the queue
        _clearPulseQueue();
        continue;
      }
    }
    
    nPulsesInDwell++;

    // check for in-block consistency

    if (blockId != pulse->getBlockId()) {
      _beamError = true;
      return -1;
    }

    if (fabs(pulseWidthUs -pulse->getPulseWidthUs()) > 1.0e-6) {
      _beamError = true;
      return -1;
    }

    if (fabs(prt - pulse->getPrt()) > 1.0e-6) {
      _beamError = true;
      return -1;
    }

    // check for end of block
    
    if (pulse->get_end_of_block()) {
      break;
    }
    
  } // while

  _blockId = blockId;
  _blockName = "unknown";
  for (int ii = 0; ii < _params.block_defs_n; ii++) {
    if (_params._block_defs[ii].block_id == _blockId) {
      _blockName = _params._block_defs[ii].block_name;
      break;
    }
  }

  // set PRT (and mean PRF)
  
  _setPrt();

  // set number of samples, ensuring an even number of pulses
  // by dropping one pulse if needed
  
  _nSamples = (nPulsesInDwell / 2) * 2;
  _midIndex = _nSamples / 2;
  
  _startIndex = _nSamples - 1;
  _endIndex = 0;
  
  _az = _conditionAz(_pulseQueue[_midIndex]->getAz());
  _el = _conditionEl(_pulseQueue[_midIndex]->getEl());

  _computeBeamElRate(0, _nSamples);

  _beamError = false;

  return 0;

}
    
//////////////////////////////////////////////////
// prepare a nsamples beam for use
// returns 0 on success, -1 on failure

int BeamReader::_finalizeNSamplesBeam()
  
{

  // set PRT

  _setPrt();

  // check that we start on the correct
  // pulse type for alternating or staggered PRT mode

  // set indices, compute angles and rate

  _midIndex = _nSamples / 2;
  _endIndex = 0;
  _startIndex = _nSamples - 1;

  _az = _conditionAz(_pulseQueue[_midIndex]->getAz());
  _el = _conditionEl(_pulseQueue[_midIndex]->getEl());

  _computeBeamElRate(0, _nSamples);

  // save sequence numbers in case we switch to indexed search

  _prevBeamPulseSeqNum = _pulseQueue[_midIndex]->getPulseSeqNum();
  
  return 0;

}
    
/////////////////////////////////////////////////////////
/// @brief Find the next valid pulse, push it onto the front of _pulseQueue,
/// assign it as _latestPulse, and return the associated shared pointer.
///
/// A pulse is considered valid if it has a moments manager.
///
/// @return a shared pointer to the pulse which was read, or a NULL shared
/// pointer if the end of data is reached.

shared_ptr<IwrfTsPulse> BeamReader::_getNextPulse()

{

  // loop until a valid pulse is found - i.e. one which has
  // a moments manager

  while (true) {

    // get the next pulse

    shared_ptr<IwrfTsPulse> pulse = _readNextPulse();
    if (pulse == NULL && _params.mode == Params::SIMULATE) {
      // in simulate mode, reset the queue
      _pulseReader->reset();
      pulse = _readNextPulse();
    }
    
    if (pulse == NULL) {
      // end of data
      return NULL;
    }

    // check radar info and processing info are active

    if (!_pulseReader->getOpsInfo().isRadarInfoActive() ||
        !_pulseReader->getOpsInfo().isTsProcessingActive()) {
      continue;
    }

    // check pulse time in archive mode

    if (_params.mode == Params::ARCHIVE) {
      time_t pulseTime = pulse->getTime();
      if (pulseTime < _startTime || pulseTime > _endTime) {
        continue;
      }
    }

    // get scan name
    
    string scanName = _pulseReader->getOpsInfo().get_scan_segment_name();
    
    // compute the antenna rate in elevation
    
    _computeProgressiveElRate(pulse);
    
    // add to the queue

    _addPulseToQueue(pulse);

    // save the shared pointer as _latestPulse, then return it

    _latestPulse = pulse;
    return _latestPulse;
  
  } // while (true)

  // Nothing left. Return a NULL pulse
  return NULL;
    
}

/////////////////////////////////////////////////////////
// get the next pulse

shared_ptr<IwrfTsPulse> BeamReader::_readNextPulse()

{

  shared_ptr<IwrfTsPulse> pulse(NULL);

  if (_pulseCache.size() > 0) {

    // get from cache

    pulse = _pulseCache.front();
    _pulseCache.pop_front();

  } else {

    pulse = _doReadNextPulse();

  }

  return pulse;

}

/////////////////////////////////////////////////////////
// read the next pulse from the reader
// check prt is correct

shared_ptr<IwrfTsPulse> BeamReader::_doReadNextPulse()

{
  shared_ptr<IwrfTsPulse> sptr;

  // Fill _prevPulse if needed
  
  if (_prevPulse == NULL) {
    // Get a pulse shared_ptr from the pool
    sptr = _getPulseFromRecyclePool();
    if (sptr == NULL) return NULL;

    // Extract the next pulse, if any, into sptr's raw pointer.
    if (_pulseReader->getNextPulse(true, sptr.get()) == NULL) {
      // No pulse was obtained, so return NULL
      return NULL;
    }

    // The reader gave us something; save it as _prevPulse
    _prevPulse = sptr;
  }

  // Get a pulse shared_ptr from the pool
  sptr = _getPulseFromRecyclePool();
  if (sptr == NULL) return NULL;

  // Extract the next pulse, if any, into sptr's raw pointer.
  if (_pulseReader->getNextPulse(true, sptr.get()) == NULL) {
      // No pulse was obtained, so return NULL
      return NULL;
  }

  // The reader gave us something, so we have a real latest pulse
  shared_ptr<IwrfTsPulse> latest = sptr;

  // swap the PRT values if the current prt refers to the 
  // time to NEXT pulse instead of time since PREV pulse

  if (!_params.prt_is_for_previous_interval) {
    latest->swapPrtValues();
  }

  if (_params.compute_prt_from_interpulse_periods) {

    // compute time between pulses
    
    double dSecs = (double) latest->getTime() - (double) _prevPulse->getTime();
    double dNanoSecs =
      (double) latest->getNanoSecs() - (double) _prevPulse->getNanoSecs();
    dSecs += dNanoSecs / 1.0e9;
    
    // set the PRT on this latest pulse, and next PRT on prev pulse
    
    if (_params.prt_is_for_previous_interval) {
      _prevPulse->set_prt_next(dSecs);
      latest->set_prt(dSecs);
    } else {
      latest->set_prt_next(dSecs);
      _prevPulse->set_prt(dSecs);
    }

  }

  // return the previously-read pulse

  shared_ptr<IwrfTsPulse> prev = _prevPulse;
  _prevPulse = latest;

  return prev;

}
  
/////////////////////////////////////////////////
// Check that we have sequential pulse numbers
// returns true if beam is considered OK
//         false if not OK
// We check for missing pulses and optionally
// discard the queue if there are missing pulses
    
bool BeamReader::_beamOk()
 
{

  // check for beamError flag
  
  if (_beamError) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - BeamReader::_beamOk" << endl;
      cerr << "  Error forming beam" << endl;
    }
    _clearPulseQueue();
    return false;
  }
  
  // check for constant pulse width in dwell
  
  double prevPulseWidthUs = _pulseQueue[_startIndex]->get_pulse_width_us();
  for (int ii = _startIndex - 1; ii >= _endIndex; ii--) {
    double pulseWidthUs = _pulseQueue[ii]->get_pulse_width_us();
    if (fabs(pulseWidthUs - prevPulseWidthUs) > 1.0e-9) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - BeamReader::_beamOk" << endl;
        cerr << "  Pulse width changes in beam:" << endl;
        cerr << "    pulseWidthUs: " << pulseWidthUs << endl;
        cerr << "    prevPulseWidthUs: " << prevPulseWidthUs << endl;
      }
      _clearPulseQueue();
      return false;
    }
  }
  
  // check for missing pulses in the sequence numbers
  
  if (!_params.check_for_missing_pulses) {
    // no need to check further
    return true;
  }

  bool pulsesMissing = false;
  si64 prevSeqNum = _pulseQueue[_startIndex]->get_pulse_seq_num();
  for (int ii = _startIndex - 1; ii >= _endIndex; ii--) {
    si64 seqNum = _pulseQueue[ii]->get_pulse_seq_num();
    if ((seqNum - prevSeqNum) != 1) {
      cerr << "WARNING - BeamReader::_beamOk" << endl;
      cerr << "  Missing pulses in sequence" << endl;
      cerr << "  Latest seq number: " << seqNum << endl;
      cerr << "  Prev   seq number: " << prevSeqNum << endl;
      cerr << "  n pulses missing: " << (seqNum - prevSeqNum) - 1 << endl;
      pulsesMissing = true;
      break;
    }
    prevSeqNum = seqNum;
  }

 if (_params.discard_dwells_with_missing_pulses) {
   if (pulsesMissing) {
     cerr << "WARNING - BeamReader::_beamOk" << endl;
     cerr << "  n pulses in queue to be discarded: " << _pulseQueue.size() << endl;
     int midIndex = (_startIndex + _endIndex) / 2;
     shared_ptr<IwrfTsPulse> pulse = _pulseQueue[midIndex];
     RadxTime ptime(pulse->getTime(), pulse->getNanoSecs() / 1.0e9);
     cerr << "  time, el, az: "
          << ptime.asString(3) << ", "
          << _el << ", " << _az << endl;
     _clearPulseQueue();
      return false;
   }
 }

 return true;

}
  
/////////////////////////////////////////////////
// add the pulse to the pulse queue
// we use a deque and push onto the front
// so the youngest pulses are at the front and
// the oldest at the back
    
void BeamReader::_addPulseToQueue(shared_ptr<IwrfTsPulse> pulse)
  
{
  
  // push pulse onto front of queue

  _pulseQueue.push_front(pulse);
  _pulseCount++;
  
  if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
    cerr << "Pulse queue size, total count: "
	 << _pulseQueue.size() << ", "
	 << _pulseCount << endl;
  }
  
  // every so often, check the status of the queues

  _checkQueueStatus();

}

/////////////////////////////////////////////////
// save the latest pulse to the cache
// and remove from main queue
    
void BeamReader::_cacheLatestPulse()
  
{

  if (_pulseQueue.size() > 0) {

    // grab the latest pulse - from front of queue

    shared_ptr<IwrfTsPulse> pulse = _pulseQueue.front();
    _pulseQueue.pop_front();

    // save to front of cache

    _pulseCache.push_front(pulse);

    // decrease pulse count

    _pulseCount--;

  }

}

/////////////////////////////////////////////////
// Clear pulse queue and cache

void BeamReader::_clearPulseQueue()
  
{
  // Drop all of our shared pointers in _pulseQueue and _pulseCache
  _pulseQueue.clear();
  _pulseCache.clear();
}

/////////////////////////////////////////////////
// free excess pulses so they're returned to the pool
// we keep at least nSamples * 2 on the queue

void BeamReader::_recyclePulses()
  
{

  int nExcess = (int) _pulseQueue.size() - (_nSamples * 2) - 1;
  for (int ii = 0; ii < nExcess; ii++) {
    // Pop the back entry. This releases our hold on the shared_ptr.
    _pulseQueue.pop_back();
  }

}

/////////////////////////////////////////////////////////
// Get a pulse from the recycle pool.
// Returns NULL if the pool is empty, or if the pulse
// at the back of the queue still has clients

shared_ptr<IwrfTsPulse> BeamReader::_getPulseFromRecyclePool()

{
    return _pulsePool.alloc();
}


/////////////////////////////////////////////////
// set the PRT members - for non-dual PRT mode

void BeamReader::_setPrt()
  
{

  // set prt, assuming single PRT for now
  
  _prt = _pulseQueue[0]->getPrt();

  // compute mean PRF
  
  _meanPrf = 1.0 / _prt;

}

////////////////////////////////////////////////////////////////
// compute min number of gates in a beam
// for fixed prt

int BeamReader::_computeMinNGates()
  
{
  
  int minNGates = _pulseQueue[0]->getNGates();
  for (int ii = 1; ii < _nSamples; ii++) {
    int thisNGates = _pulseQueue[ii]->getNGates();
    if (thisNGates < minNGates) {
      minNGates = thisNGates;
    }
  }
  return minNGates;

}
      
////////////////////////////////
// condition azimuth angle

double BeamReader::_conditionAz(double az)
  
{
  if (az > 360.0) {
    return (az - 360);
  } else if (az < 0) {
    return (az + 360.0);
  }
  return az;
}
  
////////////////////////////////
// condition elevation angle

double BeamReader::_conditionEl(double el)
  
{
  if (el > 180.0) {
    return (el - 360);
  } else if (el < -180) {
    return (el + 360.0);
  }
  return el;
}
  

////////////////////////////////////////////////////////////////
// compute elevation rate progressively as pulses are added

void BeamReader::_computeProgressiveElRate(const shared_ptr<IwrfTsPulse> pulse)

{

  double el = pulse->getEl();
  double pulseTime = pulse->getFTime();
  
  // check we have initialized this routine

  if (!_elRateInitialized) {
    _progressiveElRate = 0;
    _prevTimeForElRate = pulseTime;
    _prevElForRate = el;
    _elRateInitialized = true;
    return;
  }

  // check we have waited enough time

  double deltaTime = pulseTime - _prevTimeForElRate;
  if (deltaTime < _params.nsecs_for_antenna_rate) {
    return;
  }

  // compute rate based on time and az travel

  double deltaEl = el - _prevElForRate;
  if (deltaEl > 180) {
    deltaEl -= 360;
  } else if (deltaEl < -180) {
    deltaEl += 360;
  }
  
  _progressiveElRate = deltaEl / deltaTime;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> Elevation rate: " << _progressiveElRate 
         << " az=" << pulse->getAz() << " el=" << el
         << endl;
  }

  _prevTimeForElRate = pulseTime;
  _prevElForRate = el;

  return;

}

////////////////////////////////////////////////////////////////
// compute elevation rate for a beam

void BeamReader::_computeBeamElRate(int endIndex, int nSamples)

{

  // compute start index

  if (endIndex < 0) {
    endIndex = 0;
  }

  int startIndex = endIndex + nSamples - 1;
  if (startIndex > (int) _pulseQueue.size() - 1) {
    startIndex = _pulseQueue.size() - 1;
  }

  shared_ptr<IwrfTsPulse> pulseStart = _pulseQueue[startIndex];
  shared_ptr<IwrfTsPulse> pulseEnd = _pulseQueue[endIndex];
  
  double elStart = pulseStart->getEl();
  double elEnd = pulseEnd->getEl();

  double deltaEl = elEnd - elStart;
  if (deltaEl > 180) {
    deltaEl -= 360;
  } else if (deltaEl < -180) {
    deltaEl += 360;
  }
  
  double timeStart = pulseStart->getFTime();
  double timeEnd = pulseEnd->getFTime();
  double deltaTime = timeEnd - timeStart;

  if (deltaTime <= 0) {
    _beamElRate = 0.0;
  } else {
    _beamElRate = deltaEl / deltaTime;
  }

}

////////////////////////////////////////////////////////////////
// check status of the queues

void BeamReader::_checkQueueStatus()

{
  
  // only check every so often

  if (_pulseCountSinceStatus < 10000) {
    _pulseCountSinceStatus++;
    return;
  }
  _pulseCountSinceStatus = 0;

  // trim resize pool as required

  size_t nInUse = _pulsePool.getInUseCount();
  size_t nTarget = (int) (nInUse * 1.5);
  size_t nStart = _pulsePool.getCount();
  int nExcess = nStart - nTarget;
  if (nExcess > 0) {
    _pulsePool.resize(nTarget);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "================= Recycle status ==================" << endl;
      cerr << "  trimmed pulse pool, nStart: " << nStart << endl;
      cerr << "                      nInUse: " << nInUse << endl;
      cerr << "                      nExcess: " << nExcess << endl;
      cerr << "                      nPool: " << _pulsePool.getCount() << endl;
      cerr << "===================================================" << endl;
    }
  }

  // how many pulses are now available from the recycle pool?

  int nAvailable = _pulsePool.getFreeCount();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "================= Queue status ==================" << endl;
    cerr << "  pulseCount: " << _pulseCount << endl;
    cerr << "  pulse queue size: " << _pulseQueue.size() << endl;
    cerr << "  pulse cache size: " << _pulseCache.size() << endl;
    cerr << "  pulse recycle pool size, n available: "
         << _pulsePool.getCount() << ", "
         << nAvailable << endl;
    cerr << "  pulse+recycle queue size: "
         << _pulseQueue.size() + _pulsePool.getCount() << endl;
    cerr << "  beam recycle pool size: " << _beamRecyclePool.size() << endl;
    cerr << "=================================================" << endl;
  }

}

