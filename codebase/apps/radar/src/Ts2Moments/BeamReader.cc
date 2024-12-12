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
                       pthread_mutex_t &beamRecyclePoolMutex,
                       vector<const MomentsMgr *> &momentsMgrArray) :
        constructorOK(true),
        _progName(prog_name),
        _params(params),
        _args(args),
        _beamRecyclePool(beamRecyclePool),
        _beamRecyclePoolMutex(beamRecyclePoolMutex),
        _momentsMgrArray(momentsMgrArray),
        _momentsMgr(NULL),
        _mgrIndex(-1),
        _missMgrCount(0),
        _pulseReader(NULL),
        _startTime(args.startTime),
        _endTime(args.endTime),
        _endOfSweepPending(false),
        _endOfVolPending(false),
        _endOfVolPulseSeqNum(0),
        _endOfSweepPulseSeqNum(0),
        _endOfSweepFlag(false),
        _endOfVolFlag(false),

        // Create the pulse pool, starting with 0 pulses, and providing a custom
        // pulse factory function (since IwrfTsPulse has no default constructor).
        _pulsePool(0, std::function<IwrfTsPulse*()>([this]() { return new IwrfTsPulse(_pulseReader->getOpsInfo()); })),

        _pulseSeqNum(0),
        _prevPulseSeqNum(0),
        _prevPulse(NULL),
        _latestPulse(NULL),
        _pulseQueue(),
        _pulseCache(),
        _pulseCount(0),
        _pulseCountSinceStatus(0),
        _interpQueue(),
        _interpReady(false),
        _interpOverflow(false),
        _prevAzInterp(-9999.0),
        _prevElInterp(-9999.0),
        _burstPhases(),
        _beamCount(0),
        _midIndex(0),
        _startIndex(0),
        _endIndex(0),
        _prevBeamPulseSeqNum(0),
        _scanType(Beam::SCAN_TYPE_UNKNOWN),
        _nGates(0),
        _nSamples(_params.min_n_samples),
        _az(0.0),
        _el(0.0),
        _prt(0.001),
        _meanPrf(1000.0),
        _pulseWidthUs(1.0),
        _isAlternating(false),
        _startsOnHoriz(true),
        _isStaggeredPrt(false),
        _startsOnPrtShort(true),
        _prtShort(0.001),
        _prtLong(0.001),
        _nGatesPrtShort(0),
        _nGatesPrtLong(0),
        _isDualPrt(false),
        _isDualReady(false),
        _dualPrtIndexStart(0),
        _dualPrtIndexEnd(0),
        _azIndex(0),
        _prevAzIndex(-999),
        _elIndex(0),
        _prevElIndex(-999),

        // initialize rate computations

        _azRateInitialized(false),
        _prevTimeForAzRate(0),
        _prevAzForRate(-999),
        _progressiveAzRate(0),
        _beamAzRate(0),
        _rotationClockwise(true),
        
        _elRateInitialized(false),
        _prevTimeForElRate(0),
        _prevElForRate(-999),
        _progressiveElRate(0),
        _beamElRate(0),
        _rotationUpwards(true),

        // beam indexing

        _indexTheBeams(false),
        _indexedResolution(1.0),
        _beamAngleDeg(1.0),

        // windowing

        _windowFactorRect(),
        _windowFactorVonhann(),
        _windowFactorBlackman(),
        _windowFactorBlackmanNuttall(),

        // atmospheric attenuation

        _atmosAtten()
{
  // compute the window factors
  
  _computeWindowFactors();

  // start in PPI mode

  _initPpiMode();
  
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

  if (_params.cohere_iq_to_burst_phase) {
    _pulseReader->setCohereIqToBurst(true);
  }

  if (_params.use_pulse_width_from_ts_proc) {
    _pulseReader->setCopyPulseWidthFromTsProc(true);
  }

  if (_params.check_radar_id) {
    _pulseReader->setRadarId(_params.radar_id);
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
    
    if (_momentsMgr->getBeamMethod() == Params::BEAM_CONSTANT_STEERING_ANGLE) {

      if (_readConstantSteeringAngleBeam()) {
        // end of data
        return NULL;
      }
      
    } else if (_momentsMgr->getBeamMethod() == Params::BEAM_PULSE_WIDTH_CHANGE) {

      if (_readPulseWidthChangeBeam()) {
        // end of data
        return NULL;
      }
      
    } else if (_indexTheBeams) {
      
      // indexed beams
      
      if (_readIndexedBeam()) {
        // end of data
        return NULL;
      }
      
    } else if (_isDualPrt) {
      
      // dual PRT - dwell determined by PRT changes
      
      if (_readDualPrtBeam()) {
        // end of data
        return NULL;
      }
      
    } else {
      
      // non-indexed beams
      
      if (_readNonIndexedBeam()) {
        // end of data
        return NULL;
      }
      
    } // if (_momentsMgr->getBeamMethod() == Params::BEAM_CONSTANT_STEERING_ANGLE)

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      RadxTime ptime(_pulseQueue[0]->getTime(), _pulseQueue[0]->getNanoSecs() / 1.0e9);
      cerr << "  pulse0 time, el, az: "
           << ptime.asString(6) << ", "
           << _pulseQueue[0]->getEl() << ", " << _pulseQueue[0]->getAz() << endl;
    }

  } while (!_beamOk());

  // set nGates
  // in staggered mode, this is the short-prt nGates
  
  if (_isStaggeredPrt) {
    _nGates = _nGatesPrtShort;
  } else {
    _nGates = _computeMinNGates();
  }
  
  // initialize attenuation correction
  
  if (_params.atmos_atten_method == Params::ATMOS_ATTEN_NONE) {
    _atmosAtten.setAttenNone();
  } else if (_params.atmos_atten_method == Params::ATMOS_ATTEN_CONSTANT) {
    _atmosAtten.setAttenConstant(_params.atmos_atten_db_per_km);
  } else if (_params.atmos_atten_method == Params::ATMOS_ATTEN_CRPL) {
    double wavelengthCm = _pulseReader->getOpsInfo().get_radar_wavelength_cm();
    _atmosAtten.setAttenCrpl(wavelengthCm);
  }

  // pulse width
    
  _pulseWidthUs = _pulseQueue[_midIndex]->getPulseWidthUs();
  _beamCount++;
  
  // load the pulse shared_ptr-s into a vector for just this beam
  // reversing the order, so we start with oldest pulse
      
  vector<shared_ptr<IwrfTsPulse>> beamPulses;
  for (int ii = _startIndex; ii >= _endIndex; ii--) {
    beamPulses.push_back(_pulseQueue[ii]);
  }
  
  // set pointing angle
  
  double pointingAngle = _az;
  if (_scanType == Beam::SCAN_TYPE_RHI ||
      _scanType == Beam::SCAN_TYPE_VERT) {
    pointingAngle = _el;
  }
  
  // set processing options
  
  iwrf_xmit_rcv_mode_t xmitRcvMode = _momentsMgr->getXmitRcvMode();
  if (_params.control_xmit_rcv_mode_from_time_series) {
    xmitRcvMode = (iwrf_xmit_rcv_mode_t)
      _pulseReader->getOpsInfo().get_proc_xmit_rcv_mode();
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (_isAlternating) {
      cerr << "==>> Fast alternating mode" << endl;
      cerr << "     prt: " << _prt << endl;
      cerr << "     ngates: " << _nGates << endl;
    } else if (_isStaggeredPrt) {
      cerr << "==>> Staggered PRT mode" << endl;
      cerr << "     prt short: " << _prtShort << endl;
      cerr << "     prt long: " << _prtLong << endl;
      cerr << "     ngates short PRT: " << _nGatesPrtShort << endl;
      cerr << "     ngates long PRT: " << _nGatesPrtLong << endl;
    } else if (_isDualPrt) {
      cerr << "==>> Dual PRT mode" << endl;
      cerr << "     prt this beam: " << _prt << endl;
      cerr << "     ngates: " << _nGates << endl;
    } else {
      cerr << "==>> Single PRT mode" << endl;
      cerr << "     prt: " << _prt << endl;
      cerr << "     ngates: " << _nGates << endl;
    }
    cerr << "    pulseWidthUs: " << _pulseWidthUs << endl;
    cerr << "    nSamples: " << beamPulses.size() << endl;
    cerr << "    beamCount: " << _beamCount << endl;
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

  // compute effective nsamples for rectangular window

  int nSamplesRect = _computeNSamplesRect(_nSamples);

  // check for end of vol and sweep

  _checkForEndFlags(beamPulses);

  // initialize the beam

  beam->init(*_momentsMgr,
             _nSamples,
             nSamplesRect,
             _nGates,
             _nGatesPrtLong,
             _indexTheBeams,
             _indexedResolution,
             pointingAngle,
             _scanType,
             getAntennaRate(),
             _isAlternating,
             _isStaggeredPrt,
             _prt,
             _prtLong,
             _momentsMgr->applyPhaseDecoding(),
             xmitRcvMode,
             _endOfSweepFlag,
             _endOfVolFlag,
             _atmosAtten,
             _pulseReader->getOpsInfo(),
             beamPulses);
  
  double startAz = _conditionAz(_pulseQueue[_startIndex]->getAz());
  double startEl = _conditionEl(_pulseQueue[_startIndex]->getEl());

  double endAz = _conditionAz(_pulseQueue[_endIndex]->getAz());
  double endEl = _conditionEl(_pulseQueue[_endIndex]->getEl());

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, 
            "==>> New beam, el: mid, start, end, rate, az: mid, start, end, rate, nSamp, nRect: "
            "%7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %6.2f %6.2f %4d %4d\n",
            _el, startEl, endEl, _beamElRate,
            _az, startAz, endAz, _beamAzRate,
            _nSamples, nSamplesRect);
    if (_scanType == Beam::SCAN_TYPE_PPI) {
      cerr << "  scanType, xmitRcvMode: PPI, "
           << iwrf_xmit_rcv_mode_to_str(xmitRcvMode) << endl;
    } else {
      cerr << "  scanType, xmitRcvMode: RHI, "
           << iwrf_xmit_rcv_mode_to_str(xmitRcvMode) << endl;
    }
  }

  // reset end of vol flag
  
  _endOfSweepFlag = false;
  _endOfVolFlag = false;
  
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

  size_t nStart = _params.min_n_samples;
  if (nStart < 64) {
    nStart = 64;
  }
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
// read in data for a dual-prt beam
// returns 0 on success, -1 on failure

int BeamReader::_readDualPrtBeam()
  
{

  _isAlternating = false;
  _isStaggeredPrt = false;

  // read in pulses until PRT changes

  int count = 0;
  double prt = -9999;
  
  while (count <= _params.max_n_samples) {
    
    // get a pulse

    shared_ptr<IwrfTsPulse> pulse = _getNextPulse();
    if (pulse == NULL) {
      // end of data
      _beamError = true;
      return -1;
    }

    // save prt from first pulse
    
    if (count == 0) {
      prt = pulse->getPrt();
    }

    // check if prt has changed

    if (fabs(prt - pulse->getPrt()) > 1.0e-5) {
      // save pulse for next beam
      _cacheLatestPulse();
      break;
    }
    
    count++;

  } // while

  // save prt

  _prt = prt;
  _meanPrf = 1.0 / _prt;
  
  // set number of samples, ensuring an even number of pulses
  // by dropping one pulse if needed
  
  _nSamples = (count / 2) * 2;
  _midIndex = _nSamples / 2;

  _startIndex = _nSamples - 1;
  _endIndex = 0;

  _az = _conditionAz(_pulseQueue[_midIndex]->getAz());
  _el = _conditionEl(_pulseQueue[_midIndex]->getEl());

  _computeBeamAzRate(0, _nSamples);
  _computeBeamElRate(0, _nSamples);

  // save sequence numbers in case we switch to indexed search
  
  _prevBeamPulseSeqNum = _pulseQueue[_midIndex]->getPulseSeqNum();
  
  _beamError = false;
  return 0;

}
    
//////////////////////////////////////////////////
// read in data for a non-indexed beam
// returns 0 on success, -1 on failure

int BeamReader::_readNonIndexedBeam()
  
{

  // get _nSamples, ensure it is even

  _nSamples = (_momentsMgr->getNSamples() / 2) * 2;

  // read in _nSamples pulses, adding to queue
  
  for (int ii = 0; ii < _nSamples; ii++) {
    if (_getNextPulse() == NULL) {
      // end of data
      _beamError = true;
      return -1;
    }
  }

  // finalize the beam for use

  if (_finalizeNonIndexedBeam()) {
    _beamError = true;
    return 0;
  }

  _beamError = false;
  return 0;

}
    
//////////////////////////////////////////////////
// read in data for a beam with constant steering
// angle, as in phased array radar.
// returns 0 on success, -1 on failure

int BeamReader::_readConstantSteeringAngleBeam()
  
{

  // read in pulses until PRT changes

  int count = 0;
  double az = -9999;
  double el = -9999;
  
  while (count <= _params.max_n_samples) {
    
    // get a pulse
    
    shared_ptr<IwrfTsPulse> pulse = _getNextPulse();
    if (pulse == NULL) {
      // end of data
      _beamError = true;
      return -1;
    }

    // save az/el from first pulse
    
    if (count == 0) {
      az = pulse->getAz();
      el = pulse->getEl();
    }

    // check if angles have changed

    if ((fabs(az - pulse->getAz()) > 1.0e-3) ||
        (fabs(el - pulse->getEl()) > 1.0e-3)) {
      if (count > 1) {
        // new angle
        // save pulse for next beam
        _cacheLatestPulse();
        break;
      }
    }
    
    count++;

  } // while

  // set PRT (and mean PRF)

  _setPrt();

  // set number of samples, ensuring an even number of pulses
  // by dropping one pulse if needed
  
  _nSamples = (count / 2) * 2;
  _midIndex = _nSamples / 2;

  _startIndex = _nSamples - 1;
  _endIndex = 0;

  _az = _conditionAz(_pulseQueue[_midIndex]->getAz());
  _el = _conditionEl(_pulseQueue[_midIndex]->getEl());

  _computeBeamAzRate(0, _nSamples);
  _computeBeamElRate(0, _nSamples);

  // save sequence numbers in case we switch to indexed search
  
  _prevBeamPulseSeqNum = _pulseQueue[_midIndex]->getPulseSeqNum();
  
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
      if (_params.specify_fixed_pulse_width) {
        // check for valid pulse width
        if (fabs(_params.fixed_pulse_width_us - pulse->getPulseWidthUs()) > 2.0e-3) {
          // _getNextPulse() automatically inserted this pulse at the front of
          // _pulseQueue, but we don't want it! Remove it from the deque now.
          _pulseQueue.pop_front();
          if (warningCount == _params.max_n_samples * 10) {
            cerr << "WARNING - " << warningCount << " consecutive pulses with width != "
		 << _params.fixed_pulse_width_us << " us" << endl;
            warningCount = 0;
          }
          // Go back to get a shared pointer to the next pulse
          continue;
        }
      }
      pulseWidthUs = pulse->getPulseWidthUs();
    }
    
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

  _computeBeamAzRate(0, _nSamples);
  _computeBeamElRate(0, _nSamples);

  // save sequence numbers in case we switch to indexed search
  
  _prevBeamPulseSeqNum = _pulseQueue[_midIndex]->getPulseSeqNum();
  
  _beamError = false;
  return 0;

}
    
//////////////////////////////////////////////////
// read in data for an indexed beam
// returns 0 on success, -1 on failure

int BeamReader::_readIndexedBeam()
  
{

  // find the indexed center of the next beam
  // side effect: sets _nSamples
  
  if (_findNextIndexedBeam()) {
    // end of data
    _beamError = true;
    return -1;
  }

  // check that indexing is still in force
  // it could have been turned off because the antenna is
  // not moving fast enough

  if (!_indexTheBeams) {
    // antenna too slow for indexing
    // finalize non-indexed beam for use
    if (_finalizeNonIndexedBeam()) {
      _beamError = true;
      return -1;
    }
    return 0;
  }

  // save sequence number as a starting point for next time

  _prevBeamPulseSeqNum = _pulseQueue[0]->getPulseSeqNum();
  
  // compute the current scan rate
  
  _computeBeamAzRate(0, -1);
  _computeBeamElRate(0, -1);

  // compute the number of samples

  _nSamples = _computeNSamplesIndexed();
  
  // read in second half of beam

  int nSamplesHalf = _nSamples / 2;
  int nNeeded = nSamplesHalf - 1;
  for (int ii = 0; ii < nNeeded; ii++) {
    if (_getNextPulse() == NULL) {
      // end of data
      _beamError = true;
      return -1;
    }
  }

  // check that we start on the correct
  // pulse type for alternating or staggered PRT mode
  
  if (_checkStartConditions()) {
    _beamError = true;
    return -1;
  }

  // set indices, compute angles and rate
  
  _endIndex = 0;
  _startIndex = _nSamples - 1;
  _midIndex = _nSamples / 2;

  // constrain pulses to be within the dwell
  // this step is needed to take care of overshoot when the
  // antenna rate is varying

  _constrainPulsesToWithinDwell();

  // set PRT (and mean PRF)

  _setPrt();

  _beamError = false;
  return 0;

}
    
//////////////////////////////////////////////////////////////////////
// set the search index for the pulse after the previous beam center
// returns 0 on success, -1 on failure

int BeamReader::_findPrevIndexedBeam()
  
{
  
  // make sure we have at least 3 pulses on the queue

  while (_pulseQueue.size() < 3) {
    if (_getNextPulse() == NULL) {
      // end of data
      return -1;
    }
  }

  // using the saved sequence number,
  // search for the previous beam location in the current queue

  int prevLoc = 0;
  for (int ii = 0; ii < (int) _pulseQueue.size() - 1; ii++) {
    shared_ptr<IwrfTsPulse> pulse = _pulseQueue[ii];
    if (pulse->getPulseSeqNum() == _prevBeamPulseSeqNum) {
      prevLoc = ii;
      break;
    }
  }

  // push all pulses newer than the previous location onto
  // the cache so they are available for reading again
  // starting from the center

  for (int ii = 0; ii < prevLoc; ii++) {
    _cacheLatestPulse();
  }

  return 0;
  
}

//////////////////////////////////////////////////
// find the center of the next beam
// returns 0 on success, -1 on failure
// side effect: sets _nSamples

int BeamReader::_findNextIndexedBeam()
  
{
  
  // set the search index at the previously found beam center
  // this sets _midIndex1 and _midIndex2
  
  if (_findPrevIndexedBeam()) {
    _nSamples = ((int) _pulseQueue.size() / 2) * 2;
    return -1;
  }

  int pulseCount = 0;
  while (true) {
    
    // read a new pulse onto the queue
    
    if (_getNextPulse() == NULL) {
      // end of data
      _nSamples = ((int) _pulseQueue.size() / 2) * 2;
      return -1;
    }
    
    if (_scanType == Beam::SCAN_TYPE_PPI) {
      if (_findBeamCenterPpi() == 0) {
	break;
      }
    } else {
      if (_findBeamCenterRhi() == 0) {
	break;
      }
    }

    if (!_params.discard_non_indexed_beams) {
      
      // check for slow movement
      // if too slow, use non-indexed method
      
      if (pulseCount > _params.max_n_samples) {
        _indexTheBeams = false;
        // set number of samples, ensuring an even number of pulses
        if (_params.max_n_samples <= (int) _pulseQueue.size()) {
          _nSamples = (_params.max_n_samples / 2) * 2;
        } else {
          _nSamples = ((int) _pulseQueue.size() / 2) * 2;
        }
        return 0;
      }
      
    }
    
    pulseCount++;

  } // while

  // set _nSamples, make it even
  _nSamples = ((int) _pulseQueue.size() / 2) * 2;
  
  return 0;

}

//////////////////////////////////////////////////
// prepare a non-indexed beam for use
// returns 0 on success, -1 on failure

int BeamReader::_finalizeNonIndexedBeam()
  
{

  // set PRT

  _setPrt();

  // check that we start on the correct
  // pulse type for alternating or staggered PRT mode

  if (_checkStartConditions()) {
    return -1;
  }
  
  // check for fixed pulse width
  if (_params.specify_fixed_pulse_width) {
    if (_checkFixedPulseWidth()) {
      return -1;
    }
  }

  // set indices, compute angles and rate

  _midIndex = _nSamples / 2;
  _endIndex = 0;
  _startIndex = _nSamples - 1;

  _az = _conditionAz(_pulseQueue[_midIndex]->getAz());
  _el = _conditionEl(_pulseQueue[_midIndex]->getEl());

  _computeBeamAzRate(0, _nSamples);
  _computeBeamElRate(0, _nSamples);

  // save sequence numbers in case we switch to indexed search

  _prevBeamPulseSeqNum = _pulseQueue[_midIndex]->getPulseSeqNum();
  
  return 0;

}
    
//////////////////////////////////////////////////
// check that we start on the correct
// pulse type for alternating or staggered PRT mode
// read in extra pulse as needed
// returns 0 on success, -1 on failure

int BeamReader::_checkStartConditions()
  
{
  
  if (_isAlternating) {

    // first check on starting condition

    _checkAlternatingStartsOnH();
    
    if (!_startsOnHoriz) {
      // need to start on H transmit
      if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
        cerr << "=======>> reading extra pulse so alternating starts on H xmit" << endl;
      }
      if (_getNextPulse() == NULL) {
        // end of data
        return -1;
      }
      _startsOnHoriz = true;
      return 0;
    }

  } else if (_isStaggeredPrt) {

    // first check on starting condition

    _checkStaggeredStartsOnShort();

    if (!_startsOnPrtShort) {
      // need to start on short PRT
      if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE) {
        cerr << "=======>> reading extra pulse so staggered starts on short PRT" << endl;
      }
      if (_getNextPulse() == NULL) {
        // end of data
        return -1;
      }
      _startsOnPrtShort = true;
      return 0;
    }

  }

  return 0;

}

//////////////////////////////////////////////////
// check that we have fixed pulse width operations
// we check nSamples * 4 pulses to make sure

int BeamReader::_checkFixedPulseWidth()
  
{

  // we accumulate 4 times the number of samples in a dwell
  // to make sure the pulse width is constant over multiple
  // dwells
  
  _fixedPulseWidthCheck.resize(_nSamples * 4, false);

  // for each pulse in the queue, check the width
  
  for (int ii = 0; ii < _nSamples; ii++) {
    shared_ptr<IwrfTsPulse> pulse = _pulseQueue[ii];
    _fixedPulseWidthCheck.pop_back();
    double pulseWidthUs = pulse->get_pulse_width_us();
    if (fabs(pulseWidthUs - _params.fixed_pulse_width_us) <= 2.0e-3) {
      // pulse matches the desired width
      _fixedPulseWidthCheck.push_front(true);
    } else {
      // pulse does not match the desired width
      _fixedPulseWidthCheck.push_front(false);
    }
  }

  // are there any bad pulses found?
  // if so return error
  
  for (size_t ii = 0; ii < _fixedPulseWidthCheck.size(); ii++) {
    if (!_fixedPulseWidthCheck[ii]) {
      // found a pulse that does not match desired width
      return -1;
    }
  }

  // all good
  
  return 0;

}

//////////////////////////////////////////////////////////
// constrain pulses to be within the dwell
// this step is needed to take care of overshoot when the
// antenna rate is varying

void BeamReader::_constrainPulsesToWithinDwell()

{

  // initialize local variables

  int nSamples = _nSamples;
  int startIndex = _startIndex;
  int endIndex = _endIndex;

  // get windowing factor - for widening the dwell to account for the window

  double windowFactor = 1.0;
  if (_momentsMgr->getWindowType() == Params::WINDOW_VONHANN) {
    windowFactor = _windowFactorVonhann;
  } else if (_momentsMgr->getWindowType() == Params::WINDOW_BLACKMAN) {
    windowFactor = _windowFactorBlackman;
  } else if (_momentsMgr->getWindowType() == Params::WINDOW_BLACKMAN_NUTTALL) {
    windowFactor = _windowFactorBlackmanNuttall;
  }
  double halfWideDwell = (_beamAngleDeg * windowFactor * 1.01) / 2.0;
  int nHalfMin = _params.min_n_samples / 2;

  // adjust start and end indexes to fall within dwell

  if (_scanType == Beam::SCAN_TYPE_PPI) {
    
    // SURVEILLANCE or SECTOR
    
    for (int ii = 0; ii < _midIndex - nHalfMin; ii++) {
      double pulseAz = _pulseQueue[ii]->getAz();
      double azDiff = fabs(RadarComplex::diffDeg(pulseAz, _az));
      if (azDiff <= halfWideDwell) {
        endIndex = ii;
        break;
      }
    } // ii
    
    for (int ii = nSamples - 1; ii >= _midIndex + nHalfMin; ii--) {
      double pulseAz = _pulseQueue[ii]->getAz();
      double azDiff = fabs(RadarComplex::diffDeg(pulseAz, _az));
      if (azDiff <= halfWideDwell) {
        startIndex = ii;
        break;
      }
    } // ii
    
  } else {

    // RHI
    
    for (int ii = 0; ii < _midIndex - nHalfMin; ii++) {
      double pulseEl = _pulseQueue[ii]->getEl();
      double elDiff = fabs(RadarComplex::diffDeg(pulseEl, _el));
      if (elDiff <= halfWideDwell) {
        endIndex = ii;
        break;
      }
    } // ii
    
    for (int ii = nSamples - 1; ii >= _midIndex + nHalfMin; ii--) {
      double pulseEl = _pulseQueue[ii]->getEl();
      double elDiff = fabs(RadarComplex::diffDeg(pulseEl, _el));
      if (elDiff <= halfWideDwell) {
        startIndex = ii;
        break;
      }
    } // ii
    
  }

  // ensure we change the start and end index values by an even number
  // that way we will not upset the starting conditions for
  // alternating mode or staggered mode

  int deltaStart = (startIndex - _startIndex);
  if (deltaStart % 2 != 0) {
    if (startIndex < (int) _pulseQueue.size() - 1) {
      startIndex++;
    } else {
      startIndex--;
    }
  }

  int deltaEnd = (endIndex - _endIndex);
  if (deltaEnd % 2 != 0) {
    if (endIndex > 0) {
      endIndex--;
    } else {
      endIndex++;
    }
  }

  // compute resulting dwell
  
  nSamples = startIndex - endIndex + 1;

  double startAz = _pulseQueue[endIndex]->getAz();
  double endAz = _pulseQueue[startIndex]->getAz();
  
  double startEl = _pulseQueue[endIndex]->getEl();
  double endEl = _pulseQueue[startIndex]->getEl();
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===>> constraining dwell: el, az, nSamples old/new, "
         << "startEl, endEl, startAz, endAz: "
         << _el << ", "
         << _az << ", "
         << _nSamples << "/"
         << nSamples << ", "
         << startEl << ", "
         << endEl << ", "
         << startAz << ", "
         << endAz << endl;
  }

  // set resulting values

  _startIndex = startIndex;
  _endIndex = endIndex;
  _nSamples = nSamples;
  
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

    // check for pending end of sweep

    if (pulse->get_end_of_sweep() && !_endOfSweepPending) {
      _endOfSweepPending = true;
      _endOfSweepPulseSeqNum = pulse->getPulseSeqNum();
    }
    
    // check for pending end of vol

    if (pulse->get_end_of_volume() && !_endOfVolPending) {
      _endOfVolPending = true;
      _endOfVolPulseSeqNum = pulse->getPulseSeqNum();
    }

    if (_params.invert_hv_flag) {
      pulse->setInvertHvFlag(true);
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

    // override scan mode if appropriate

    if (_params.override_scan_mode) {
      pulse->set_scan_mode(_params.scan_mode_for_override);
      IwrfTsInfo &info = _pulseReader->getOpsInfo();
      info.set_scan_mode(_params.scan_mode_for_override);
    }

    // get scan name
    
    string scanName = _pulseReader->getOpsInfo().get_scan_segment_name();
    
    // check scan mode and type
    
    Beam::scan_type_t scanType = Beam::SCAN_TYPE_UNKNOWN;
    int scanMode = pulse->getScanMode();
    if (scanMode == IWRF_SCAN_MODE_RHI || 
        scanMode == IWRF_SCAN_MODE_VERTICAL_POINTING ||
        scanMode == IWRF_SCAN_MODE_MANRHI) {
      scanType = Beam::SCAN_TYPE_RHI;
    } else if (scanMode == IWRF_SCAN_MODE_VERTICAL_POINTING) {
      scanType = Beam::SCAN_TYPE_VERT;
    } else {
      scanType = Beam::SCAN_TYPE_PPI;
    }
    
    if (scanType != _scanType) {
      if (scanType == Beam::SCAN_TYPE_PPI) {
        _initPpiMode();
      } else if (scanType == Beam::SCAN_TYPE_RHI) {
        _initRhiMode();
      } else if (scanType == Beam::SCAN_TYPE_VERT) {
        _initVertMode();
      }
      _prevTimeForElRate = 0;
      _prevTimeForAzRate = 0;
    }
    
    // compute the antenna rate
    
    double antennaRate = 0.0;
    if (_scanType == Beam::SCAN_TYPE_PPI) {
      _computeProgressiveAzRate(pulse);
      antennaRate = _progressiveAzRate;
    } else {
      _computeProgressiveElRate(pulse);
      antennaRate = _progressiveElRate;
    }
    
    // get the prf
    
    double prf = 1.0 / pulse->getPrt();
    
    // Find suitable moments manager, given the scan mode,
    // prf and antenna rate
    
    _momentsMgr = NULL;
    int mgrIndex = -1;
    for (int ii = 0; ii < (int) _momentsMgrArray.size(); ii++) {
      if (_momentsMgrArray[ii]->checkSuitable(scanMode,
                                              scanName,
                                              prf,
                                              antennaRate)) {
        _momentsMgr = _momentsMgrArray[ii];
        mgrIndex = ii;
        break;
      }
    }
    
    if (_momentsMgr == NULL) {
      // no suitable manager found, so ignore this pulse
      if (_params.debug >= Params::DEBUG_EXTRA_VERBOSE ||
          (_params.debug && _missMgrCount % 1000 == 0)) {
        cerr << "WARNING - BeamReader::getNextBeam" << endl;
        cerr << "  No suitable Moments Manager," << endl;
        cerr << "  scanMode: " << scanMode << endl;
        cerr << "  scanName: " << scanName << endl;
        cerr << "  prf: " << prf << endl;
        cerr << "  antennaRate: " << antennaRate << endl;
        cerr << "  Ignoring this pulse" << endl;
      }
      _missMgrCount++;

      // Drop the current shared pointer and go back for another.
      continue;
    }
    
    // debug print
    
    if (mgrIndex != _mgrIndex) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "-->> scanMode: " << scanMode << endl;
        cerr << "-->> new moments manager" << endl;
        _momentsMgr->print(cerr);
      }
      _mgrIndex = mgrIndex;
    }
    
    // set indexing
    
    _indexTheBeams = _momentsMgr->indexTheBeams();
    _beamAngleDeg = _momentsMgr->getBeamAngleDeg();
    _indexedResolution = _momentsMgr->getIndexedResolution();
    if (_params.control_beam_indexing_from_time_series) {
      const IwrfTsInfo &info = _pulseReader->getOpsInfo();
      _indexTheBeams = info.get_proc_beams_are_indexed();
      _indexedResolution = info.get_proc_indexed_beam_spacing_deg();
      _beamAngleDeg = info.get_proc_indexed_beam_width_deg();
    }
    if (_momentsMgr->isDualPrt()) {
      _indexTheBeams = false;
      _isDualPrt = true;
    } else {
      _isDualPrt = false;
    }

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

    if (_params.interpolate_antenna_angles) {
      pulse = _readNextPulseWithInterp();
    } else {
      pulse = _doReadNextPulse();
    }

  }

  if (pulse != NULL) {

    // save burst phases for trips 1 through 4

    _burstPhases.trip4 = _burstPhases.trip3;
    _burstPhases.trip3 = _burstPhases.trip2;
    _burstPhases.trip2 = _burstPhases.trip1;
    RadarComplex::setFromDegrees(pulse->get_burst_arg(0),
                                 _burstPhases.trip1);
    // set burst for pulse

    pulse->setBurstPhases(_burstPhases);
    
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
  
/////////////////////////////////////////////////////////
// get the next pulse, with interpolation

shared_ptr<IwrfTsPulse> BeamReader::_readNextPulseWithInterp()

{

  // if the interp queue has overflowed, empty it before
  // trying again

  if ((int) _interpQueue.size() > _params.angle_interp_max_queue_size) {
    _interpOverflow = true;
  } else if (_interpOverflow) {
    if (_interpQueue.size() < 2) {
      _interpOverflow = false;
    }
    return _popFromInterpQueue();
  }

  // if interp angles are ready, return the oldest one
  
  if (_interpReady && _interpQueue.size() > 1) {
    return _popFromInterpQueue();
  }

  // if the queue is down to 1 pulse, reset the interpReady flag
  // since we need to save the last item for interpolation
  
  if (_interpQueue.size() == 1) {
    _interpReady = false;
  }
  
  // for interpolation, we expect a series of pulses with constant
  // azimuth and/or elevation. When the angle changes, we interpolate
  // across the constant values

  while (true) {
    
    if ((int) _interpQueue.size() > _params.angle_interp_max_queue_size) {
      _interpOverflow = true;
      return _popFromInterpQueue();
    }
  
    // read pulses to make sure we have at least 2 pulses in queue
    
    while (_interpQueue.size() < 2) {
      if (_pushOntoInterpQueue() == NULL) {
        return NULL;
      }
    }
    
    // check that either all pulses are PPI
    // or all pulses are RHI
    // if not, do not interpolate and return the oldest one
    
    bool allPpi = true;
    bool allRhi = true;
    for (int ii = 0; ii < (int) _interpQueue.size(); ii++) {
      iwrf_scan_mode_t scanMode = _interpQueue[ii]->getScanMode();
      if (scanMode != IWRF_SCAN_MODE_SECTOR &&
          scanMode != IWRF_SCAN_MODE_AZ_SUR_360) {
        allPpi = false;
      }
      if (scanMode != IWRF_SCAN_MODE_RHI &&
          scanMode != IWRF_SCAN_MODE_MANRHI) {
        allRhi = false;
      } 
    }

    if (!allPpi && !allRhi) {
      // not consistent, do not interpolate
      return _popFromInterpQueue();
    }

    // check for single step in angle
    
    int nAngleChange = 0;
    for (int ii = 1; ii < (int) _interpQueue.size(); ii++) {
      double prevAng = 0;
      double thisAng = 0;
      if (allPpi) {
        prevAng = _interpQueue[ii-1]->getAz();
        thisAng = _interpQueue[ii]->getAz();
      } else {
        // allRhi
        prevAng = _interpQueue[ii-1]->getEl();
        thisAng = _interpQueue[ii]->getEl();
      }
      if (fabs(prevAng - thisAng) > 0.0001) {
        nAngleChange++;
      }
    }
    
    if (nAngleChange > 0) {
      
      // interpolate if we have a step change and more than 2 pulses
        
      _interpAzAngles();
      _interpElevAngles();
      _interpReady = true;
      
      // return oldest
      
      return _popFromInterpQueue();

    } else {

      // constant angles, read another pulse and repeat
      
      if (_pushOntoInterpQueue() == NULL) {
        return NULL;
      }

    }
      
  } // while
    
}

/////////////////////////////////////////////////////
// read another pulse and push onto the interp queue
// return pulse read, NULL on failure

shared_ptr<IwrfTsPulse> BeamReader::_pushOntoInterpQueue()

{
  
  shared_ptr<IwrfTsPulse> pulse = _doReadNextPulse();

  if (pulse != NULL) {
    _interpQueue.push_front(pulse);
    if (_params.angle_interp_debug) {
      fprintf(stderr, "=========>> reading pulse el, az: %7.3f %7.3f\n",
              pulse->getAz(), pulse->getEl());
    }
  }

  return pulse;

}

///////////////////////////////////////////////
// return the oldest pulse on the interp queue

shared_ptr<IwrfTsPulse> BeamReader::_popFromInterpQueue()

{

  shared_ptr<IwrfTsPulse> oldest = _interpQueue.back();
  _interpQueue.pop_back();
  if (_params.angle_interp_debug) {
    fprintf(stderr, "==========>> interpolated el, az: %7.3f %7.3f\n",
            oldest->getAz(), oldest->getEl());
  }
  return oldest;

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
  
  // check for constant pulse width in dwell, or queue, if pulse width
  // is used to find dwell, or is specified constant
  
  if ((_momentsMgr->getBeamMethod() == Params::BEAM_PULSE_WIDTH_CHANGE)) {
    
    // check for constant pulse width in beam only
    
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
    
  } else if (_params.specify_fixed_pulse_width) {
    
    // check for constant pulse width in full queue
    
    double prevPulseWidthUs = _pulseQueue[0]->get_pulse_width_us();
    for (int ii = 1; ii < (int) _pulseQueue.size(); ii++) {
      double pulseWidthUs = _pulseQueue[ii]->get_pulse_width_us();
      if (fabs(pulseWidthUs - prevPulseWidthUs) > 1.0e-9) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "WARNING - BeamReader::_beamOk" << endl;
          cerr << "  Pulse width changes in queue:" << endl;
          cerr << "    pulseWidthUs: " << pulseWidthUs << endl;
          cerr << "    prevPulseWidthUs: " << prevPulseWidthUs << endl;
        }
        _clearPulseQueue();
        return false;
      }
    }

    if (fabs(prevPulseWidthUs - _params.fixed_pulse_width_us) > 2.0e-3) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - BeamReader::_beamOk" << endl;
        cerr << "  Pulse width incorrect in dwell:" << endl;
        cerr << "    pulseWidthUs: " << prevPulseWidthUs << endl;
        cerr << "    should be: " << _params.fixed_pulse_width_us << endl;
      }
      _clearPulseQueue();
      return false;
    }
    
  } // if ((_momentsMgr->getBeamMethod() == Params::BEAM_PULSE_WIDTH_CHANGE)) {

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

  if (_params.discard_beams_with_missing_pulses) {
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

  int nExcess = (int) _pulseQueue.size() - (_nSamples * 2) - _maxTrips;
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
// interpolate azimuth angles as required
    
void BeamReader::_interpAzAngles()
  
{

  int nPulses = (int) _interpQueue.size();
  
  // compute difference between first and last
  
  double azLatest = _interpQueue[0]->getAz();
  double azFirst = _interpQueue[nPulses - 1]->getAz();
  double azChange = RadarComplex::diffDeg(azLatest, azFirst);

  // if too large, do not interpolate
  
  if (fabs(azChange) > _params.angle_interp_max_change) {
    return;
  }
  
  double dAz = azChange / (nPulses - 1.0);
  if (_params.angle_interp_debug) {
    fprintf(stderr,
            "====>> interp az, n, first, last, dAz:"
            "%3d %7.3f %7.3f %6.3f\n",
            nPulses, azFirst, azLatest, dAz);
  }

  double latencyCorr = 0.0;
  if (_params.angle_interp_adjust_for_latency && _progressiveAzRate > -990) {
    latencyCorr = _params.angle_interp_latency * fabs(_progressiveAzRate);
    if (_params.angle_interp_debug) {
      fprintf(stderr,
              "====>> latency correction az rate, delta: %7.3f %7.3f\n",
              _progressiveAzRate, latencyCorr);
    }
  }

  for (int ii = 0; ii < nPulses; ii++) {
    int jj = nPulses - ii - 1;
    double interpAz = azFirst + ii * dAz + latencyCorr;
    if (interpAz >= 360.0) {
      interpAz -= 360.0;
    } else if (interpAz < 0) {
      interpAz += 360.0;
    }
    _interpQueue[jj]->set_azimuth(interpAz);
  }

}

/////////////////////////////////////////////////
// interpolate elevation angles as required
    
void BeamReader::_interpElevAngles()
  
{

  int nPulses = (int) _interpQueue.size();
  
  // compute difference between first and last
  
  double elLatest = _interpQueue[0]->getEl();
  double elFirst = _interpQueue[nPulses - 1]->getEl();
  double elChange = RadarComplex::diffDeg(elLatest, elFirst);

  // if too large, do not interpolate
  
  if (fabs(elChange) > _params.angle_interp_max_change) {
    return;
  }
  
  double dEl = elChange / (nPulses - 1.0);
  if (_params.angle_interp_debug) {
    fprintf(stderr,
            "====>> interp el, n, first, last, dEl:"
            "%3d %7.3f %7.3f %6.3f\n",
            nPulses, elFirst, elLatest, dEl);
  }

  double latencyCorr = 0.0;
  if (_params.angle_interp_adjust_for_latency && _progressiveElRate > -990) {
    latencyCorr = _params.angle_interp_latency * fabs(_progressiveElRate);
    if (_params.angle_interp_debug) {
      fprintf(stderr,
              "====>> latency correction el rate, delta: %7.3f %7.3f\n",
              _progressiveElRate, latencyCorr);
    }
  }

  for (int ii = 0; ii < nPulses; ii++) {
    int jj = nPulses - ii - 1;
    double interpEl = elFirst + ii * dEl + latencyCorr;
    if (interpEl >= 180.0) {
      interpEl -= 360.0;
    } else if (interpEl < -180) {
      interpEl += 360.0;
    }
    _interpQueue[jj]->set_elevation(interpEl);
  }
    
}

//////////////////////
// initialize ppi mode

void BeamReader::_initPpiMode()

{

  _azIndex = 0;
  _prevAzIndex = -999;
  _scanType = Beam::SCAN_TYPE_PPI;

}

//////////////////////
// initialize rhi mode

void BeamReader::_initRhiMode()

{

  _elIndex = 0;
  _prevElIndex = -999;
  _scanType = Beam::SCAN_TYPE_RHI;

}

//////////////////////
// initialize vertical pointing mode

void BeamReader::_initVertMode()

{

  _elIndex = 0;
  _prevElIndex = -999;
  _scanType = Beam::SCAN_TYPE_VERT;

}

/////////////////////////////////////////////////
// set the PRT members - for non-dual PRT mode

void BeamReader::_setPrt()
  
{

  // set prt, assuming single PRT for now
  
  _prt = _pulseQueue[0]->getPrt();
  if (_params.override_primary_prt) {
    _prt = _params.primary_prt_secs;
  }

  // check for staggered mode first, and then alternating
  // if staggered, it cannot be alternating

  _checkIsStaggeredPrt();
  
  if (_isStaggeredPrt) {
    _isAlternating = false;
  } else {
    // check if we have alternating h/v pulses
    _checkIsAlternating();
  }
  
  // check if we have staggered PRT pulses - does not apply
  // to alternating mode
  
  // if (_isAlternating) {
  //   _isStaggeredPrt = false;
  // } else {
  //   _checkIsStaggeredPrt();
  // }
  
  // compute mean PRF
  
  if (_isStaggeredPrt) {
    _meanPrf = 1.0 / ((_prtShort + _prtLong) / 2.0);
  } else {
    _meanPrf = 1.0 / _prt;
  }

}

/////////////////////////////////////////////////
// Find center of beam, PPI mode
// Side effects: sets _az, _azIndex
// returns 0 on success, -1 on failure

int BeamReader::_findBeamCenterPpi()
  
{

  // set elevation

  _el = _conditionEl(_pulseQueue[0]->getEl());

  // initialize azimuth

  _az = 0.0;

  // compute azimuths which need to straddle the center of the beam
  
  double midAz1 = _pulseQueue[1]->getAz();
  double midAz2 = _pulseQueue[0]->getAz();

  // check that delta azimuth is reasonable

  double deltaAz = fabs(midAz2 - midAz1);
  if (deltaAz > 180.0) {
    deltaAz = fabs(deltaAz - 360.0);
  }

  if (deltaAz > _indexedResolution) {
    return -1;
  }
  
  // set resolution in azimuth, rounded suitably
  
  double angularRes = _indexedResolution;
  int nazPer45 = (int) (45.0 / angularRes + 0.5);
  angularRes = 45.0 / nazPer45;

  // compute azimuth index
  
  _azIndex = (int) (midAz1 / angularRes + 0.5);
  int nAz = (int) (360.0 / angularRes + 0.5);
  if (_azIndex == nAz) {
    _azIndex = 0;
  }
  
  // check for duplicate index with previous one
  
  if (_azIndex == _prevAzIndex) {
    return -1;
  }

  // compute target azimuth by rounding the azimuth at the
  // center of the data to the closest suitable az
  
  _az = _azIndex * angularRes;
  
  if (_az >= 360.0) {
    _az -= 360;
  } else if (_az < 0) {
    _az += 360.0;
  }

  // Check if the azimuths straddle the target azimuth
    
  if (midAz1 <= _az && midAz2 >= _az) {
    
    // az1 is below and az2 above - clockwise rotation
    _rotationClockwise = true;
    _prevAzIndex = _azIndex;
    return 0;
    
  } else if (midAz1 >= _az && midAz2 <= _az) {
    
    // az1 is above and az2 below - counterclockwise rotation
    _rotationClockwise = false;
    _prevAzIndex = _azIndex;
    return 0;
    
  } else if (_az == 0.0) {
    
    if (midAz1 > 360.0 - angularRes &&
        midAz2 < angularRes) {
      
      // az1 is below 0 and az2 above 0 - clockwise rotation
      _rotationClockwise = true;
      _prevAzIndex = _azIndex;
      return 0;
      
    } else if (midAz2 > 360.0 - angularRes &&
               midAz1 < angularRes) {
      
      // az1 is above 0 and az2 below 0 - counterclockwise rotation
      _rotationClockwise = false;
      _prevAzIndex = _azIndex;
      return 0;
      
    }
    
  }

  return -1;

}

/////////////////////////////////////////////////
// Find center of beam, RHI mode
// Side effects: sets _el, _elIndex
// returns 0 on success, -1 on failure

int BeamReader::_findBeamCenterRhi()
  
{

  // initialize azimuth

  _az = _conditionAz(_pulseQueue[0]->getAz());

  // initialize elevation

  _el = 0.0;
  
  // compute elevations which need to straddle the center of the beam
  
  double midEl1 = _conditionEl(_pulseQueue[1]->getEl());
  double midEl2 = _conditionEl(_pulseQueue[0]->getEl());

  // check that delta elevation is reasonable

  double deltaEl = fabs(midEl2 - midEl1);
  if (deltaEl > 180.0) {
    deltaEl = fabs(deltaEl - 360.0);
  }
  if (deltaEl > _indexedResolution) {
    return -1;
  }

  // set resolution in elevation
  
  double angularRes = _indexedResolution;
  
  // compute elevation index
  
  _elIndex = (int) ((midEl1 + 180.0) / angularRes + 0.5);
  int nEl = (int) (360.0 / angularRes + 0.5);
  if (_elIndex == nEl) {
    _elIndex = 0;
  }
    
  // check for duplicate index with previous one
  
  if (_elIndex == _prevElIndex) {
    return -1;
  }
  
  // compute target elevation by rounding the elevation at the
  // center of the data to the closest suitable el
  
  _el = -180.0 + (_elIndex * angularRes);
  
  // Check if the elevations at the center of the data straddle
  // the target elevation
  
  if (midEl1 <= _el && midEl2 >= _el) {
    
    // el1 is below and el2 above - el increasing
    _rotationUpwards = true;
    _prevElIndex = _elIndex;
    return 0;
      
  } else if (midEl1 >= _el && midEl2 <= _el) {
    
    // el1 is above and el2 below - el decreasing
    _rotationUpwards = false;
    _prevElIndex = _elIndex;
    return 0;
    
  }

  return -1;

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
      
////////////////////////////////////////////////////////////////
// compute nSamples, the number of samples in the beam dwell
// Sets _nSamples and _nSamplesRect

int BeamReader::_computeNSamplesIndexed()
  
{
  
  const IwrfTsInfo &info = _pulseReader->getOpsInfo();

  // did we specify the number of samples?
  
  bool specifyNSamples =
    (_momentsMgr->getBeamMethod() == Params::BEAM_SPECIFY_N_SAMPLES);
  if (_params.control_beam_indexing_from_time_series) {
    specifyNSamples = !info.get_proc_specify_dwell_width();
  }

  if (specifyNSamples) {
    
    // SZ864 is special - always 64 samples
    
    if (_momentsMgr->applyPhaseDecoding()) {
      return _nSamplesSz;
    }
    
    int nSamples = _momentsMgr->getNSamples();
    if (_params.control_n_samples_from_time_series) {
      nSamples = info.get_proc_integration_cycle_pulses();
    }
    if (nSamples > (int) _pulseQueue.size()) {
      nSamples = _pulseQueue.size();
    }
    
    // ensure even number of samples, rounding down
    nSamples = (nSamples / 2) * 2;
    return nSamples;
    
  }

  // compute antenna rate, force non-zero condition

  double antennaRate;
  if (_scanType == Beam::SCAN_TYPE_RHI) {
    antennaRate = fabs(_beamElRate);
  } else {
    antennaRate = fabs(_beamAzRate);
  }
  if (antennaRate < 0.01) {
    antennaRate = 0.01;
  }

  // compute nSamples from rate and dwell size

  int nSamplesRect = (int) ((_meanPrf * _beamAngleDeg / antennaRate) + 0.5);

  // sanity check
  
  if (nSamplesRect < _params.min_n_samples) {
    nSamplesRect = _params.min_n_samples;
  } else if (nSamplesRect > _params.max_n_samples) {
    nSamplesRect = _params.max_n_samples;
  }
  
  // adjust for the window
  
  int nSamples = nSamplesRect;
  if (_momentsMgr->getWindowType() == Params::WINDOW_VONHANN) {
    nSamples = (int) (nSamplesRect * _windowFactorVonhann + 0.5);
  } else if (_momentsMgr->getWindowType() == Params::WINDOW_BLACKMAN) {
    nSamples = (int) (nSamplesRect * _windowFactorBlackman + 0.5);
  } else if (_momentsMgr->getWindowType() == Params::WINDOW_BLACKMAN_NUTTALL) {
    nSamples = (int) (nSamplesRect * _windowFactorBlackmanNuttall + 0.5);
  }

  // adjust nSamples to queue size if needed
  
  if (nSamples > (int) _pulseQueue.size()) {
    nSamples = _pulseQueue.size();
  }
  
  // ensure even number of samples, rounding down
  
  nSamples = (nSamples / 2) * 2;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    shared_ptr<IwrfTsPulse> pulse = _pulseQueue[0];
    time_t psecs = pulse->getTime();
    int pnanosecs = pulse->getNanoSecs();
    DateTime ptime(psecs);
    ptime.setSubSec(pnanosecs * 1.0e-9);
    cerr << "-->> el, az, antennaRate, nPossiblyOdd, "
         << "nSamples, nSamplesRect, time: "
         << pulse->getEl() << ", "
         << pulse->getAz() << ", "
         << antennaRate << ", "
         << nSamples << ", "
         << nSamplesRect << ", "
         << ptime.getStr() << endl;
  }

  return nSamples;

}

////////////////////////////////////////////////////////////////
// compute nSamples adjusted for the window - i.e. what
// nSamples would be for a rectangular window

int BeamReader::_computeNSamplesRect(int nSamples)
  
{
  
  // phase coding?
  
  if (_momentsMgr->applyPhaseDecoding()) {
    return _nSamplesSz;
  }

  if (_momentsMgr->getWindowType() == Params::WINDOW_RECT) {
    return nSamples;
  }

  // adjust for the window
  
  int nSamplesRect = nSamples;
  if (_momentsMgr->getWindowType() == Params::WINDOW_VONHANN) {
    nSamplesRect = (int) ((double) nSamples / _windowFactorVonhann + 0.5);
  } else if (_momentsMgr->getWindowType() == Params::WINDOW_BLACKMAN) {
    nSamplesRect = (int) ((double) nSamples / _windowFactorBlackman + 0.5);
  } else if (_momentsMgr->getWindowType() == Params::WINDOW_BLACKMAN_NUTTALL) {
    nSamplesRect = (int) ((double) nSamples / _windowFactorBlackmanNuttall + 0.5);
  }
  
  // ensure even number of samples, rounding up
  
  nSamplesRect = ((nSamplesRect + 1) / 2) * 2;
  
  return nSamplesRect;

}

///////////////////////////////////////////      
// check if we have alternating h/v pulses
// 
// Also check that we start on a horizontal pulse
// If so, the queue is ready to make a beam

void BeamReader::_checkIsAlternating()
  
{
  
  _isAlternating = false;

  bool prevHoriz = _pulseQueue[0]->isHoriz();
  for (int i = 1; i < (int) _pulseQueue.size(); i++) {
    bool thisHoriz = _pulseQueue[i]->isHoriz();
    if (thisHoriz == prevHoriz) {
      return;
    }
    prevHoriz = thisHoriz;
  }
  _isAlternating = true;
  
  // we want to start on H, which means we must
  // end on V, since we always have an even number
  // of pulses

  _checkAlternatingStartsOnH();

}

///////////////////////////////////////////      
// check if dwell starts in H xmit
// 
// Also check that we start on a horizontal pulse
// If so, the queue is ready to make a beam

void BeamReader::_checkAlternatingStartsOnH()
  
{

  if (!_isAlternating) {
    return;
  }

  // we want to start on H
  // the starting pulse is at the end of the queue

  if (_pulseQueue[_nSamples-1]->isHoriz()) {
    _startsOnHoriz = true;
  } else {
    _startsOnHoriz = false;
  }

}

///////////////////////////////////////////      
// check if we have staggered PRT mode
//
// Also check that we start on a short prt
// If so, the queue is ready to make a beam

void BeamReader::_checkIsStaggeredPrt()

{

  _isStaggeredPrt = false;
  if ((int) _pulseQueue.size() < _nSamples) {
    return;
  }

  shared_ptr<IwrfTsPulse> pulse0 = _pulseQueue[_nSamples-1]; // first pulse in series
  shared_ptr<IwrfTsPulse> pulse1 = _pulseQueue[_nSamples-2]; // second pulse in series

  if (pulse0 == NULL || pulse1 == NULL) {
    return;
  }

  double prt0 = pulse0->getPrt();
  double prt1 = pulse1->getPrt();

  int nGates0 = pulse0->getNGates();
  int nGates1 = pulse1->getNGates();

  if (fabs(prt0 - prt1) < 0.00001) {
    return;
  }

  for (int ii = 1; ii < _nSamples - 1; ii += 2) {
    if (fabs(_pulseQueue[ii]->getPrt() - prt0) > 0.00001) {
      return;
    }
    if (_pulseQueue[ii]->getNGates() != nGates0) {
      return;
    }
  }
  
  for (int ii = 0; ii < _nSamples - 2; ii += 2) {
    if (fabs(_pulseQueue[ii]->getPrt() - prt1) > 0.00001) {
      return;
    }
    if (_pulseQueue[ii]->getNGates() != nGates1) {
      return;
    }
  }

  _isStaggeredPrt = true;

  // We want to start on short PRT, which means we must
  // end on long PRT, since we always have an even number
  // of pulses.
  // However, the prt in the headers refers to the
  // prt from the PREVIOUS pulse to THIS pulse, so the
  // short prt pulse is actually identified by the longer of
  // the prts, and has the smaller number of gates

  if (prt0 < prt1) {
    _prtShort = prt0;
    _prtLong = prt1;
    _nGatesPrtShort = nGates1;
    _nGatesPrtLong = nGates0;
    _startsOnPrtShort = true;
  } else {
    _prtShort = prt1;
    _prtLong = prt0;
    _nGatesPrtShort = nGates0;
    _nGatesPrtLong = nGates1;
    _startsOnPrtShort = false;
  }

  _prt = _prtShort;

  if (_params.override_primary_prt) {
    double ratio = _params.primary_prt_secs / _prt;
    _prt *= ratio;
    _prtShort *= ratio;
    _prtLong *= ratio;
  }

}

///////////////////////////////////////////      
// check if staggered PRT dwell starts on
// short PRT - this is required for the
// dealiasing code to work correctly.

void BeamReader::_checkStaggeredStartsOnShort()

{

  _startsOnPrtShort = false;

  if (!_isStaggeredPrt) {
    return;
  }
  
  // the pulses are stored in reverse order

  if ((int) _pulseQueue.size() < _nSamples) {
    return;
  }

  shared_ptr<IwrfTsPulse> pulse0 = _pulseQueue[_nSamples-1]; // first pulse in series
  shared_ptr<IwrfTsPulse> pulse1 = _pulseQueue[_nSamples-2]; // second pulse in series

  if (pulse0 == NULL || pulse1 == NULL) {
    return;
  }

  double prt0 = pulse0->getPrt();
  double prt1 = pulse1->getPrt();

  // NOTE:
  // We want to start on a short PRT pulse.
  // However, the prt in the headers refers to the
  // prt from the PREVIOUS pulse to THIS pulse.
  // This means that the short prt pulse is actually identified
  // with the longer of the prts.
  
  if (prt0 > prt1) {
    // prt0 is greater than prt1, which means
    // that the first pulse is a short PRT pulse
    // see note above
    _startsOnPrtShort = true;
  } else {
    // prt0 is less than prt1, which means
    // that the first pulse is a long PRT pulse
    // see note above
    _startsOnPrtShort = false;
  }

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
// compute azimuth rate progressively as pulses are added

void BeamReader::_computeProgressiveAzRate(const shared_ptr<IwrfTsPulse> pulse)

{

  double az = pulse->getAz();
  double pulseTime = pulse->getFTime();

  // check we have initialized this routine
  
  if (!_azRateInitialized) {
    _progressiveAzRate = 10;
    _prevTimeForAzRate = pulseTime;
    _prevAzForRate = az;
    _azRateInitialized = true;
    return;
  }

  // check we have waited enough time

  double deltaTime = pulseTime - _prevTimeForAzRate;
  if (deltaTime < _params.nsecs_for_antenna_rate) {
    return;
  }

  // compute rate based on time and az travel

  double deltaAz = az - _prevAzForRate;
  if (deltaAz > 180) {
    deltaAz -= 360;
  } else if (deltaAz < -180) {
    deltaAz += 360;
  }

  _progressiveAzRate = deltaAz / deltaTime;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> Azimuth rate: " << _progressiveAzRate 
         << " az=" << az << " el=" << pulse->getEl()
         << endl;
  }
  
  _prevTimeForAzRate = pulseTime;
  _prevAzForRate = az;

}

////////////////////////////////////////////////////////////////
// compute elevation rate progressively as pulses are added

void BeamReader::_computeProgressiveElRate(const shared_ptr<IwrfTsPulse> pulse)

{

  double el = pulse->getEl();
  double pulseTime = pulse->getFTime();
  
  // check we have initialized this routine

  if (!_elRateInitialized) {
    _progressiveElRate = 10;
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
// compute azimuth rate for a beam

void BeamReader::_computeBeamAzRate(int endIndex, int nSamples)

{

  // if nSamples is not specified, compute from the mean PRF

  if (nSamples <= 0) {
    nSamples = (int) (_meanPrf / _params.nsecs_for_antenna_rate + 0.5);
    if (nSamples < _params.min_n_samples) {
      nSamples = _params.min_n_samples;
    }
  }

  // check limit on end index

  if (endIndex > (int) _pulseQueue.size() - nSamples) {
    endIndex = (int) _pulseQueue.size() - nSamples;
  }
  if (endIndex < 0) {
    endIndex = 0;
  }

  // compute start index

  int startIndex = endIndex + nSamples - 1;
  if (startIndex > (int) _pulseQueue.size() - 1) {
    startIndex = (int) _pulseQueue.size() - 1;
  }

  shared_ptr<IwrfTsPulse> pulseStart = _pulseQueue[startIndex];
  shared_ptr<IwrfTsPulse> pulseEnd = _pulseQueue[endIndex];
  
  double azStart = pulseStart->getAz();
  double azEnd = pulseEnd->getAz();

  double deltaAz = azEnd - azStart;
  if (deltaAz > 180) {
    deltaAz -= 360;
  } else if (deltaAz < -180) {
    deltaAz += 360;
  }
  
  double timeStart = pulseStart->getFTime();
  double timeEnd = pulseEnd->getFTime();
  double deltaTime = timeEnd - timeStart;
  
  if (deltaTime <= 0) {
    _beamAzRate = 0.0;
  } else {
    _beamAzRate = deltaAz / deltaTime;
  }

}

////////////////////////////////////////////////////////////////
// compute elevation rate for a beam

void BeamReader::_computeBeamElRate(int endIndex, int nSamples)

{

  // if nSamples is not specified, compute from the mean PRF

  if (nSamples <= 0) {
    nSamples = (int) (_meanPrf / _params.nsecs_for_antenna_rate + 0.5);
    if (nSamples < _params.min_n_samples) {
      nSamples = _params.min_n_samples;
    }
  }

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

///////////////////////////////////////////
// get antenna rate for scan type

double BeamReader::getAntennaRate()

{
  
  double rate = 0.0;
  if (_scanType == Beam::SCAN_TYPE_RHI) {
    rate = _beamElRate;
  } else {
    rate = _beamAzRate;
  }
  return rate;
  
}


////////////////////////////////////////////////////////////////
// check for end of vol and sweep flags

void BeamReader::_checkForEndFlags(const vector<shared_ptr<IwrfTsPulse>> &beamPulses)
  
{
  
  si64 firstPulseIndex = beamPulses[0]->getPulseSeqNum();

  // check if the pulses have moved beyond the end of sweep flag

  if (_endOfSweepPending) {
    if (_endOfSweepPulseSeqNum < firstPulseIndex) {
      _endOfSweepFlag = true;
      _endOfSweepPending = false;
    }
  }
    
  // check if the pulses have moved beyond the end of vol flag

  if (_endOfVolPending) {
    if (_endOfVolPulseSeqNum < firstPulseIndex) {
      _endOfVolFlag = true;
      _endOfVolPending = false;
    }
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
    cerr << "  interp queue size: " << _interpQueue.size() << endl;
    cerr << "  pulse recycle pool size, n available: "
         << _pulsePool.getCount() << ", "
         << nAvailable << endl;
    cerr << "  pulse+recycle queue size: "
         << _pulseQueue.size() + _pulsePool.getCount() << endl;
    cerr << "  beam recycle pool size: " << _beamRecyclePool.size() << endl;
    cerr << "=================================================" << endl;
  }

}

////////////////////////////////////////////////////////////////
// compute the window factors
// this is the fraction of the window width, centered, which
// accounts for 90 percent of the power

void BeamReader::_computeWindowFactors()

{

  int nn = 1000;
  double dnn = (double) nn;
  double *window = NULL;

  // rectangular

  window = RadarMoments::createWindowRect(nn);
  double sum = 0.0;
  for (int ii = 0; ii < nn; ii++) {
    double wval = window[ii];
    sum += (wval * wval) / dnn;
    if (sum > 0.05) {
      _windowFactorRect = dnn / (nn - (ii * 2));
      break;
    }
  }
  delete[] window;

  // vonHann

  window = RadarMoments::createWindowVonhann(nn);
  sum = 0.0;
  for (int ii = 0; ii < nn; ii++) {
    double wval = window[ii];
    sum += (wval * wval) / dnn;
    if (sum > 0.05) {
      _windowFactorVonhann = dnn / (nn - (ii * 2));
      break;
    }
  }
  delete[] window;

  // blackman

  window = RadarMoments::createWindowBlackman(nn);
  sum = 0.0;
  for (int ii = 0; ii < nn; ii++) {
    double wval = window[ii];
    sum += (wval * wval) / dnn;
    if (sum > 0.05) {
      _windowFactorBlackman = dnn / (nn - (ii * 2));
      break;
    }
  }
  delete[] window;

  // blackman-nuttall

  window = RadarMoments::createWindowBlackmanNuttall(nn);
  sum = 0.0;
  for (int ii = 0; ii < nn; ii++) {
    double wval = window[ii];
    sum += (wval * wval) / dnn;
    if (sum > 0.05) {
      _windowFactorBlackmanNuttall = dnn / (nn - (ii * 2));
      break;
    }
  }
  delete[] window;

  if (_params.debug) {
    cerr << "NOTE: 90% window factors" << endl;
    cerr << "_windowFactorRect: " << _windowFactorRect << endl;
    cerr << "_windowFactorVonhann: " << _windowFactorVonhann << endl;
    cerr << "_windowFactorBlackman: " << _windowFactorBlackman << endl;
    cerr << "_windowFactorBlackmanNuttall: "
         << _windowFactorBlackmanNuttall << endl;
  }

}

