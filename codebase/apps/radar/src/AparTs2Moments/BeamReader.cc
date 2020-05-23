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
// Aug 2019
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
        _progName(prog_name),
        _params(params),
        _args(args),
        _beamRecyclePool(beamRecyclePool),
        _beamRecyclePoolMutex(beamRecyclePoolMutex)
        
{

  constructorOK = true;

  _cachedPulse = NULL;
  _dwellReady = false;

  _beamNumInDwell = -1;
  _dwellSeqNum = -1;

  _pulseReader = NULL;

  _startTime = args.startTime;
  _endTime = args.endTime;

  _endOfSweepFlag = false;
  _endOfVolFlag = false;
  
  _pulseSeqNum = 0;
  _pulseCount = 0;
  _beamCount = 0;

  _scanType = Beam::SCAN_TYPE_UNKNOWN;
  _nGates = 0;

  _az = 0.0;
  _el = 0.0;
  _prt = 0.001;
  _meanPrf = 2000.0;
  _pulseWidthUs = 1.0;
  
  _isAlternating = false;
  _isStaggeredPrt = false;

  _prtShort = 0.001;
  _prtLong = 0.001;

  _nGatesPrtShort = 0;
  _nGatesPrtLong = 0;

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
    
  // check that file list set in SIMLATE mode
  
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
  
  AparTsDebug_t aparDebug = AparTsDebug_t::OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    aparDebug = AparTsDebug_t::VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    aparDebug = AparTsDebug_t::NORM;
  } 
    
  if (_params.mode == Params::FMQ) {
    _pulseReader = new AparTsReaderFmq(_params.input_fmq,
                                       aparDebug,
                                       _params.position_fmq_at_start);
  } else {
    _pulseReader = new AparTsReaderFile(inputPathList, aparDebug);
  }

  _pulseReader->setCopyPulseWidthFromTsProc(true);

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
  
  if (_params.debug) {
    cerr << "Entering BeamReader destructor" << endl;
  }

  for (size_t ii = 0; ii < _dwellPulses.size(); ii++) {
    delete _dwellPulses[ii];
  } // ii
  _dwellPulses.clear();
  
  for (size_t ii = 0; ii < _pulseRecyclePool.size(); ii++) {
    delete _pulseRecyclePool[ii];
  } // ii
  _pulseRecyclePool.clear();
  
  if (_pulseReader) {
    delete _pulseReader;
  }

  if (_params.debug) {
    cerr << "Exiting BeamReader destructor" << endl;
  }

}

//////////////////////////////////////////////////
// get the next beam
// returns Beam object pointer on success, NULL on failure
// caller must free beam

Beam *BeamReader::getNextBeam()
  
{

  // check if any beam left to process in the dwell

  bool noBeamsLeft = true;
  for (size_t ii = 0; ii < _dwellPulses.size(); ii++) {
    AparTsPulse* pulse = _dwellPulses[ii];
    if (pulse->getBeamNumInDwell() == _beamNumInDwell+1) {
      noBeamsLeft = false;
      break;
    }
  }
  
  // if dwell is not yet available,
  // or we have handled all of the beams in the dwell,
  // read in a new dwell

  if (!_dwellReady || noBeamsLeft) {
    if (_readPulsesForDwell()) {
      return NULL;
    }
  }

  // dwell has been read in
  // go to next beam in dwell
  // this was initialized to -1 when dwell is read in

  _beamNumInDwell++;

  // create vector with pulses for this beam num

  _beamPulses.clear();
  for (size_t ii = 0; ii < _dwellPulses.size(); ii++) {
    AparTsPulse* pulse = _dwellPulses[ii];
    if (pulse->getBeamNumInDwell() == _beamNumInDwell) {
      _beamPulses.push_back(pulse);
    }
  }

  // set beam params
  // angles, prt, flags for alternating and staggered modes, etc

  _setBeamParams();

  // set nGates
  // in staggered mode, this is the short-prt nGates
  // otherwise use min number of gates in beam
  
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
    double wavelengthCm = _pulseReader->getOpsInfo().getRadarWavelengthCm();
    _atmosAtten.setAttenCrpl(wavelengthCm);
  }

  // pulse width
    
  _pulseWidthUs = _beamPulses[0]->getPulseWidthUs();
  _beamCount++;
  
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
    } else {
      cerr << "==>> Single PRT mode" << endl;
      cerr << "     prt: " << _prt << endl;
      cerr << "     ngates: " << _nGates << endl;
    }
    cerr << "    pulseWidthUs: " << _pulseWidthUs << endl;
    cerr << "    beamCount: " << _beamCount << endl;
  }

  // set processing options
  
  // get a beam from the pool if available,
  // otherwise create a new one
  
  Beam *beam = NULL;
  {
    pthread_mutex_lock(&_beamRecyclePoolMutex);
    if (_beamRecyclePool.size() > 0) {
      beam = _beamRecyclePool.back();
      _beamRecyclePool.pop_back();
    } else {
      beam = new Beam(_progName, _params);
    }
    pthread_mutex_unlock(&_beamRecyclePoolMutex);
  }

  // initialize the beam

  beam->init(_beamPulses.size(),
             _nGates,
             _nGatesPrtLong,
             _el,
             _az,
             _scanType,
             _isAlternating,
             _isStaggeredPrt,
             _prt,
             _prtLong,
             _endOfSweepFlag,
             _endOfVolFlag,
             _atmosAtten,
             _pulseReader->getOpsInfo(),
             _beamPulses);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, 
            "==>> New beam, el, az, nSamples: "
            "%7.2f %7.2f %4d\n",
            _el, _az, (int) _beamPulses.size());
  }

  // reset end of vol flag
  
  _endOfSweepFlag = false;
  _endOfVolFlag = false;
  
  string statusXml = getOpsInfo().getStatusXmlStr();
  if (statusXml.size() > 0) {
    beam->setStatusXml(statusXml);
  }
  
  return beam;

}

//////////////////////////////////////////////////
// read in a dwell
// returns 0 on success, -1 on failure

int BeamReader::_readPulsesForDwell()
  
{

  // clear the queue

  _clearDwellPulses();

  // read in pulses until we have a full dwell
  
  while (true) {
    
    // read a single pulse
    
    AparTsPulse *pulse = _getNextPulse();
    if (pulse == NULL) {
      // end of data
      return -1;
    }
    
    // ignore pulses with 0 gates
    if (pulse->getNGates() == 0) {
      delete pulse;
      _clearDwellPulses();
      continue;
    }

    // has dwell seq num changed?
    // if so process dwell

    if (_dwellSeqNum < 0) {
      // initialization
      _dwellSeqNum = pulse->getDwellSeqNum();
    } else if (pulse->getDwellSeqNum() != _dwellSeqNum) {
      // dwell num has changed, cache the latest pulse
      _cachedPulse = pulse;
      // dwell ready, return
      _dwellReady = true;
      _beamNumInDwell = -1;
      return 0;
    }

    // add to the queue
    
    _addPulseToDwell(pulse);
    
  } // while

  // should not reach here

  return -1;

}


/////////////////////////////////////////////////////////
// get the next pulse
// check to see if it has a moments manager
// loops until one is found, or end of data is reached

AparTsPulse *BeamReader::_getNextPulse()

{

  // loop until a valid pulse is found - i.e. one which has
  // a moments manager

  while (true) {

    // get the next pulse

    AparTsPulse *pulse = NULL;
    if (_cachedPulse != NULL) {
      pulse = _cachedPulse;
      _cachedPulse = NULL;
    } else {
      pulse = _readNextPulse();
    }
    
    if (pulse == NULL && _params.mode == Params::SIMULATE) {
      // in simulate mode, reset the queue
      _pulseReader->reset();
      pulse = _readNextPulse();
    }
    
    if (pulse == NULL) {
      // end of data
      return NULL;
    }

    if (pulse->getEndOfSweep()) {
      // set end of sweep flag, used for next beam
      _endOfSweepFlag = true;
    }
    
    if (pulse->getEndOfVolume()) {
      // set end of vol flag, used for next beam
      _endOfVolFlag = true;
    }
    
    if (_params.invert_hv_flag) {
      pulse->setInvertHvFlag(true);
    }
    
    // check radar info and processing info are active

    if (!_pulseReader->getOpsInfo().isRadarInfoActive() ||
        !_pulseReader->getOpsInfo().isTsProcessingActive()) {
      delete pulse;
      continue;
    }

    // check pulse time in archive mode

    if (_params.mode == Params::ARCHIVE) {
      time_t pulseTime = pulse->getTime();
      if (pulseTime < _startTime || pulseTime > _endTime) {
        delete pulse;
        continue;
      }
    }

    // get scan name
    
    string scanName = _pulseReader->getOpsInfo().getScanSegmentName();

    // return the pulse

    return pulse;
  
  } // while (true)

  return NULL;
    
}

/////////////////////////////////////////////////////////
// get the next pulse

AparTsPulse *BeamReader::_readNextPulse()
  
{

  // do we have a pulse object available from the recycle pool
  // if so, his avoids having to reconstruct a pulse object every time

  AparTsPulse *pulse = _getPulseFromRecyclePool();

  if (pulse == NULL) {
    // create a new pulse
    pulse = _pulseReader->getNextPulse(true);
  } else {
    // recycle previously used pulse
    pulse = _pulseReader->getNextPulse(true, pulse);
  }

  if (pulse == NULL) {
    // end of data
    return NULL;
  }

  return pulse;

}
  
/////////////////////////////////////////////////
// add pulse to the dwell
// we use a vector and push onto the back
// to keep the order as they are read in
    
void BeamReader::_addPulseToDwell(AparTsPulse *pulse)
  
{

  // push pulse onto queue
  // increase client count by 1 so we know pulse is being used
  // by the _dwellPulses
  
  pulse->addClient();
  _dwellPulses.push_back(pulse);
  _pulseCount++;
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Pulse queue size, total count: "
	 << _dwellPulses.size() << ", "
	 << _pulseCount << endl;
  }

  // check that dwell has not grown too large

  if ((int) _dwellPulses.size() > _params.max_pulses_per_dwell) {
    cerr << "Too many pulses in dwell: " << _dwellPulses.size() << endl;
    cerr << "Clearing dwell pulses and starting again" << endl;
    _clearDwellPulses();
  }
  
}

/////////////////////////////////////////////////
// Clear pulses in dwell
// move them to the recycle pool

void BeamReader::_clearDwellPulses()
  
{

  for (size_t ii = 0; ii < _dwellPulses.size(); ii++) {
    AparTsPulse *pulse = _dwellPulses[ii];
    pulse->removeClient();
    _addPulseToRecyclePool(pulse);
  }

  _dwellPulses.clear();
  _dwellReady = false;
  _dwellSeqNum = -1;

}

/////////////////////////////////////////////////////////
// add pulse to recycle pool

void BeamReader::_addPulseToRecyclePool(AparTsPulse *pulse)
  
{
  _pulseRecyclePool.push_front(pulse);
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Pulse recycle pool size: "
         << _pulseRecyclePool.size() << endl;
  }

}

/////////////////////////////////////////////////////////
// Get a pulse from the recycle pool.
// Returns NULL if the pool is empty, or if the pulse
// at the back of the queue still has clients

AparTsPulse *BeamReader::_getPulseFromRecyclePool()

{
  if (_pulseRecyclePool.size() < 1) {
    return NULL;
  }

  // find a pulse with no clients - i.e. all threads have released it

  deque<AparTsPulse *>::iterator ii;
  for (ii = _pulseRecyclePool.begin();
       ii != _pulseRecyclePool.end(); ii++) {
    AparTsPulse *pulse = *ii;
    if (pulse->getNClients() == 0) {
      _pulseRecyclePool.erase(ii);
      return pulse;
    }
  }
  
  // none available

  return NULL;

}

/////////////////////////////////////////////////
// set the beam parameters
// angles, PRT, flags, etc

void BeamReader::_setBeamParams()
  
{

  // angles

  _az = _beamPulses[0]->getAzimuth();
  _el = _beamPulses[0]->getElevation();

  // set prt, assuming single PRT for now
  
  _prt = _beamPulses[0]->getPrt();
  
  // check if we have alternating h/v pulses
  
  _checkIsAlternating();
  
  // check if we have staggered PRT pulses - does not apply
  // to alternating mode
  
  if (_isAlternating) {
    _isStaggeredPrt = false;
  } else {
    _checkIsStaggeredPrt();
  }
  
  // compute mean PRF
  
  if (_isStaggeredPrt) {
    _meanPrf = 1.0 / ((_prtShort + _prtLong) / 2.0);
  } else {
    _meanPrf = 1.0 / _prt;
  }

}

////////////////////////////////////////////////////////////////
// compute min number of gates in a beam
// for fixed prt

int BeamReader::_computeMinNGates()
  
{
  
  if (_beamPulses.size() == 0) {
    return 0;
  }

  int minNGates = _beamPulses[0]->getNGates();
  for (size_t ii = 1; ii < _beamPulses.size(); ii++) {
    int thisNGates = _beamPulses[ii]->getNGates();
    if (thisNGates < minNGates) {
      minNGates = thisNGates;
    }
  }
  return minNGates;

}
      
///////////////////////////////////////////      
// check if we have alternating h/v pulses
// 
// Also check that we start on a horizontal pulse
// If so, the queue is ready to make a beam

void BeamReader::_checkIsAlternating()
  
{

  _isAlternating = false;

  bool prevHoriz = _beamPulses[0]->isHoriz();
  for (int i = 1; i < (int) _beamPulses.size(); i++) {
    bool thisHoriz = _beamPulses[i]->isHoriz();
    if (thisHoriz != prevHoriz) {
      _isAlternating = true;
      return;
    }
    prevHoriz = thisHoriz;
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

  const AparTsPulse *pulse0 = _beamPulses[0]; // first pulse in series
  const AparTsPulse *pulse1 = _beamPulses[1]; // second pulse in series

  double prt0 = pulse0->getPrt();
  double prt1 = pulse1->getPrt();

  int nGates0 = pulse0->getNGates();
  int nGates1 = pulse1->getNGates();

  if (fabs(prt0 - prt1) < 0.00001) {
    return;
  }

  for (size_t ii = 1; ii < _beamPulses.size() - 1; ii += 2) {
    if (fabs(_beamPulses[ii]->getPrt() - prt0) > 0.00001) {
      return;
    }
    if (_beamPulses[ii]->getNGates() != nGates0) {
      return;
    }
  }
  
  for (size_t ii = 0; ii < _beamPulses.size() - 2; ii += 2) {
    if (fabs(_beamPulses[ii]->getPrt() - prt1) > 0.00001) {
      return;
    }
    if (_beamPulses[ii]->getNGates() != nGates1) {
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
    // _startsOnPrtShort = true;
  } else {
    _prtShort = prt1;
    _prtLong = prt0;
    _nGatesPrtShort = nGates0;
    _nGatesPrtLong = nGates1;
    // _startsOnPrtShort = false;
  }

  _prt = _prtShort;

}

