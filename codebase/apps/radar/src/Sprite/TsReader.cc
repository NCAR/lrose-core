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
// TsReader.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////
//
// Reads in pulses, creates beams.
//
////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/ReadDir.hh>
#include <toolsa/Path.hh>
#include "TsReader.hh"
using namespace std;

/////////////////////////
// TimePath constructor

TsReader::TimePath::TimePath(time_t valid_time,
                             time_t start_time,
                             time_t end_time,
                             const string &name,
                             const string &path) :
        validTime(valid_time), 
        startTime(start_time), 
        endTime(end_time), 
        fileName(name),
        filePath(path)
  
{

  // scan name for fixed angle

  fixedAngle = 0.0;
  int iang;
  int year, month, day, hour, min, sec;
  
  if (sscanf(fileName.c_str(),
             "%4d%2d%2d_%2d%2d%2d_%d",
             &year, &month, &day, 
             &hour, &min, &sec, 
             &iang) == 7) {
    fixedAngle = iang / 10.0;
  }

}

////////////////////////////////////////////////////
// TsReader Constructor

TsReader::TsReader(const string &prog_name,
                   const Params &params,
                   const Args &args) :
        _progName(prog_name),
        _params(params),
        _args(args)
  
{

  OK = true;

  _pulseReader = NULL;
  _pulseGetter = NULL;
  _scanType = SCAN_TYPE_UNKNOWN;
  
  _nPulsesRead = 0;
  
  _nSamples = _params.n_samples;
  
  _isAlternating = false;
  _isStaggeredPrt = false;
  
  _time = 0;
  _az = 0.0;
  _el = 0.0;
  _searchEl = 0.0;
  _searchAz = 0.0;

  _nGates = 0;

  _prt = 0.0;
  _prtShort = 0.0;
  _prtLong = 0.0;

  if (_params.debug >= Params::DEBUG_EXTRA) {
    _iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    _iwrfDebug = IWRF_DEBUG_NORM;
  } else {
    _iwrfDebug = IWRF_DEBUG_OFF;
  }

  _isRhi = false;
  _scanModeStr = "PPI";
  _scanType = SCAN_TYPE_PPI;
  _indexedRes = _params.indexed_resolution_ppi;

  switch (_params.input_mode) {
    case Params::REALTIME_FMQ_MODE:
    default: {
      _pulseReader = new IwrfTsReaderFmq(_params.input_fmq_url,
                                         _iwrfDebug,
                                         _params.seek_to_start_of_fmq);
      break;
    }
    case Params::REALTIME_TCP_MODE: {
      _pulseReader = new IwrfTsReaderTcp(_params.input_tcp_address,
                                         _params.input_tcp_port,
                                         _iwrfDebug);
      break;
    }
    case Params::ARCHIVE_TIME_MODE: {
      _timeSpanSecs = _params.archive_time_span_secs;
      _archiveStartTime.set(_params.archive_start_time);
      _archiveEndTime = _archiveStartTime + _timeSpanSecs;
      _beamTime = _archiveStartTime;
      _beamIntervalSecs = 0.0;
      _pulseGetter = new IwrfTsGet(_iwrfDebug);
      _pulseGetter->setTopDir(_params.archive_data_dir);
      if (_params.invert_hv_flag) {
        _pulseGetter->setInvertHvFlag(true);
      }
      if (!_params.prt_is_for_previous_interval) {
        _pulseGetter->setPrtIsForNextInterval(true);
      }
      break;
    }
    case Params::FILE_LIST_MODE: {
      _pulseReader = new IwrfTsReaderFile(_args.inputFileList,
                                          _iwrfDebug);
      break;
    }
    case Params::FOLLOW_DISPLAY_MODE: {
      _timeSpanSecs = _params.archive_time_span_secs;
      _archiveStartTime.set(_params.archive_start_time);
      _archiveEndTime = _archiveStartTime + _timeSpanSecs;
      _beamTime = _archiveStartTime;
      _beamIntervalSecs = 0.0;
      _pulseGetter = new IwrfTsGet(_iwrfDebug);
      _pulseGetter->setTopDir(_params.archive_data_dir);
      if (_params.invert_hv_flag) {
        _pulseGetter->setInvertHvFlag(true);
      }
      if (!_params.prt_is_for_previous_interval) {
        _pulseGetter->setPrtIsForNextInterval(true);
      }
      break;
    }

  } // switch

}

//////////////////////////////////////////////////////////////////
// destructor

TsReader::~TsReader()

{

  if (_pulseReader) {
    delete _pulseReader;
  }

  if (_pulseGetter) {
    delete _pulseGetter;
  }

  _clearPulseGetterQueue();
  _clearPulseReaderQueue();

}

/////////////////////////////////////////////////////////
// get the next beam in realtime or archive sequence
// returns Beam object pointer on success, NULL on failure
// caller must free beam

Beam *TsReader::getNextBeam()
  
{
  
  if (_pulseGetter != NULL) {
    
    // advance beam time

    if (_beamIntervalSecs != 0) {
      _prevBeamTime = _beamTime;
    }
    _beamTime += _beamIntervalSecs;
    
    // get IwrfTsGet
    // pass in missing values for el and az

    return _getBeamViaGetter(_beamTime, -9999.0, -9999.0);

  } else {

    // get IwrfTsReader
    
    return _getBeamViaReader();

  }

}

/////////////////////////////////////////////////////////
// get the previous beam in realtime or archive sequence
// we need to reset the queue and position to read the 
// previous beam
//
// returns Beam object pointer on success, NULL on failure
// caller must free beam

Beam *TsReader::getPreviousBeam()
  
{
  
  if (_pulseGetter != NULL) {
    
    // go back in time
    
    if (_beamIntervalSecs != 0) {
      _prevBeamTime = _beamTime;
    }
    _beamTime -= _beamIntervalSecs;
    
    // use IwrfTsGet
    // pass in missing values for el and az
    
    return _getBeamViaGetter(_beamTime, -9999.0, -9999.0);

  } else {

    // position for previous beam
    
    if (_positionReaderForPreviousBeam()) {
      cerr << "ERROR - TsReader::getPreviousBeam()" << endl;
      return NULL;
    }
    
    // use IwrfTsReader
    
    return _getBeamViaReader();

  }
}

/////////////////////////////////////////////////////////
// get a beam from the getter, based on the time and
// location from the display

Beam *TsReader::getBeamFollowDisplay(const DateTime &searchTime,
                                     double searchEl, 
                                     double searchAz)
  
{
  
  if (_pulseGetter == NULL) {
    cerr << "ERROR - TsReader::getBeamFollowDisplay()" << endl;
    cerr << "  No IwrfTsGet opened" << endl;
    return NULL;
  }
    
  // set beam time
  
  _prevBeamTime = _beamTime;
  _beamTime = searchTime;
  _searchEl = searchEl;
  _searchAz = searchAz;
    
  return _getBeamViaGetter(_beamTime, _searchEl, _searchAz);

}

/////////////////////////////////////////////////////////
// using IwrfTsGet
// get the next beam in realtime or archive sequence
// returns Beam object pointer on success, NULL on failure
// caller must free beam

Beam *TsReader::_getBeamViaGetter(const DateTime &searchTime, 
                                  double searchEl,
                                  double searchAz)
  
{

  if (_params.debug) {
    cerr << "Getting beam at time, el, az: "
         << searchTime.asString(3) << ", "
         << searchEl << ", " << searchAz << endl;
  }
  
  // retrieve pulses for beam

  vector<IwrfTsPulse *> beamPulses;
  if (_pulseGetter->retrieveBeam(searchTime,
                                 searchEl,
                                 searchAz,
                                 _nSamples,
                                 beamPulses)) {
    // on failure, try previous time
    if (_prevBeamTime.isValid()) {
      _beamTime = _prevBeamTime;
      if (_params.debug) {
        cerr << "====>> Retry, beam at time: " << _beamTime.asString(3) << endl;
      }
      if (_pulseGetter->retrieveBeam(_beamTime,
                                     searchEl,
                                     searchAz,
                                     _nSamples,
                                     beamPulses)) {
        cerr << "ERROR - TsReader::_getBeamViaGetter()" << endl;
        cerr << "  cannot retrieve beam" << endl;
        return NULL;
      }
    }
  }

  // compute beam interval

  IwrfTsPulse *firstPulse = beamPulses[0];
  IwrfTsPulse *lastPulse = beamPulses[beamPulses.size() - 1];
  DateTime beamStartTime(firstPulse->getTime(), true,
                         (double) firstPulse->getNanoSecs() / 1.0e9);
  DateTime beamEndTime(lastPulse->getTime(), true,
                       (double) lastPulse->getNanoSecs() / 1.0e9);
  _beamIntervalSecs =
    ((beamEndTime - beamStartTime) *
     (((double) _nSamples + 1.0) / (double) _nSamples));
  
  // set the pulse queue
  
  _clearPulseGetterQueue();
  for (size_t ii = 0; ii < beamPulses.size(); ii++) {
    _addPulseToGetterQueue(beamPulses[ii]);
  }

  // set prt and nGates, assuming single PRT for now
  
  _prt = _pulseGetter->getPrt();
  _nGates = _pulseGetter->getNGates();
  
  // check if we have alternating h/v pulses
  
  _isAlternating = _pulseGetter->getIsAlternating();

  // check if we have staggered PRT pulses - does not apply
  // to alternating mode

  _isStaggeredPrt = _pulseGetter->getIsStaggeredPrt();
  _prtShort = _pulseGetter->getPrtShort();
  _prtLong = _pulseGetter->getPrtLong();
  _nGatesPrtShort = _pulseGetter->getNGatesPrtShort();
  _nGatesPrtLong = _pulseGetter->getNGatesPrtLong();
  
  // in non-staggered mode check we have constant nGates
  
  if (!_isStaggeredPrt) {
    int nGates0 = _getterQueue[0]->getNGates();
    for (int i = 1; i < (int) _getterQueue.size(); i += 2) {
      if (_getterQueue[i]->getNGates() != nGates0) {
        cerr << "ERROR - TsReader::getNextBeam()" << endl;
        cerr << "  nGates varies in non-staggered mode" << endl;
        return NULL;
      }
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
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
  }

  _filePath = _pulseGetter->getPathInUse();

  size_t midIndex = _getterQueue.size() / 2;
  _az = _conditionAz(_getterQueue[midIndex]->getAz());
  _el = _conditionEl(_getterQueue[midIndex]->getEl());

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "got beam, _nPulsesRead: " << _nPulsesRead << endl;
  }

  // create new beam
  
  Beam *beam = new Beam(_progName, _params);
  beam->setPulses(_scanType == SCAN_TYPE_RHI,
                  _nSamples,
                  _nGates,
                  _nGatesPrtLong,
                  _indexedBeams,
                  _indexedRes,
                  _isAlternating,
                  _isStaggeredPrt,
                  _prt,
                  _prtLong,
                  _pulseGetter->getTsInfo(),
                  _getterQueue);
  
  return beam;

}

/////////////////////////////////////////////////////////
// using the IwrfTsReader
// get the next beam in realtime or archive sequence
// returns Beam object pointer on success, NULL on failure
// caller must free beam

Beam *TsReader::_getBeamViaReader()
  
{

  // clear queue and free up pulses
  
  _clearPulseReaderQueue();
  
  // read in pulses, load up pulse queue
  
  while (true) {
    
    PMU_auto_register("getNextBeam");
    
    IwrfTsPulse *pulse = _pulseReader->getNextPulse(true);
    if (pulse == NULL) {
      cerr << "ERROR - TsReader::getNextBeam()" << endl;
      cerr << "  end of data" << endl;
      _clearPulseReaderQueue();
      return NULL;
    }
    _nPulsesRead++;

    if (_params.invert_hv_flag) {
      pulse->setInvertHvFlag(true);
    }
    
    if (!_params.prt_is_for_previous_interval) {
      pulse->swapPrtValues();
    }
    
    // check scan mode and type
    
    int scanMode = pulse->getScanMode();
    if (scanMode == IWRF_SCAN_MODE_RHI) {
      if (!_isRhi) {
        // change in scan mode, clear pulse queue
        _clearPulseReaderQueue();
      }
      _isRhi = true;
      _scanModeStr = "RHI";
      _scanType = SCAN_TYPE_RHI;
      _indexedRes = _params.indexed_resolution_rhi;
    } else {
      if (_isRhi) {
        // change in scan mode, clear pulse queue
        _clearPulseReaderQueue();
      }
      _isRhi = false;
      _scanModeStr = "PPI";
      _scanType = SCAN_TYPE_PPI;
      _indexedRes = _params.indexed_resolution_ppi;
    }
    
    // check for missing pulses
    if (_readerQueue.size() == 0) {
      _prevPulseSeqNum = pulse->get_pulse_seq_num();
    } else {
      if (pulse->get_pulse_seq_num() != _prevPulseSeqNum + 1) {
        cerr << "ERROR - TsReader::getNextBeam()" << endl;
        cerr << "  missing pulses, clearing queue" << endl;
        _clearPulseReaderQueue();
      }
      _prevPulseSeqNum = pulse->get_pulse_seq_num();
    }
    
    // Create a new pulse object and save a pointer to it in the
    // _readerQueue.  _readerQueue is a FIFO, with elements
    // added at the end and dropped off the beginning. So if we have a
    // full buffer delete the first element before shifting the
    // elements to the left.
    
    // add pulse to queue, managing memory appropriately
    
    _addPulseToReaderQueue(pulse);

    if ((int) _readerQueue.size() == (_nSamples + 4)) {
      // got the pulses we need
      break;
    }
    
  } // while

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    const IwrfTsPulse *firstPulse = _readerQueue[0];
    DateTime beamStartTime(firstPulse->getTime(), true,
                           (double) firstPulse->getNanoSecs() / 1.0e9);
    cerr << "Read pulse at time: " << beamStartTime.asString(3) << endl;
  }
    
  // set prt and nGates, assuming single PRT for now
  
  _prt = _readerQueue[0]->getPrt();
  _nGates = _readerQueue[0]->getNGates();
  
  // check if we have alternating h/v pulses
  
  _checkIsAlternating();

  // check if we have staggered PRT pulses - does not apply
  // to alternating mode

  if (_isAlternating) {
    _isStaggeredPrt = false;
  } else {
    _checkIsStaggeredPrt();
  }

  // in non-staggered mode check we have constant nGates
  
  if (!_isStaggeredPrt) {
    int nGates0 = _readerQueue[0]->getNGates();
    for (int i = 1; i < (int) _readerQueue.size(); i += 2) {
      if (_readerQueue[i]->getNGates() != nGates0) {
        cerr << "ERROR - TsReader::getNextBeam()" << endl;
        cerr << "  nGates varies in non-staggered mode" << endl;
        return NULL;
      }
    }
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
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
  }

  _filePath = _pulseReader->getPathInUse();

  size_t midIndex = _readerQueue.size() / 2;
  _az = _conditionAz(_readerQueue[midIndex]->getAz());
  _el = _conditionEl(_readerQueue[midIndex]->getEl());

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "got beam, _nPulsesRead: " << _nPulsesRead << endl;
  }

  // set index limits
  
  int startIndex = midIndex - _nSamples / 2;
  if (startIndex < 0) {
    startIndex = 0;
  }
  
  int endIndex = startIndex + _nSamples - 1;
  int qSize = (int) _readerQueue.size();
  if (endIndex > qSize - 4) {
    endIndex = qSize - 4;
    startIndex = endIndex - _nSamples + 1;
  }

  // adjust to make sure we start on correct pulse
  
  if (_isAlternating) {
    bool startsOnHoriz = _readerQueue[startIndex]->isHoriz();
    if (!startsOnHoriz) {
      startIndex++;
      endIndex++;
    }
  } else if (_isStaggeredPrt) {
    double prt0 = _readerQueue[startIndex]->getPrt();
    double prt1 = _readerQueue[startIndex+1]->getPrt();
    if (prt0 > prt1) {
      startIndex++;
      endIndex++;
    }
  }

  // load the pulse pointers into a deque for
  // just this beam
  
  deque<const IwrfTsPulse *> beamPulses;
  for (int ii = startIndex; ii <= endIndex; ii++) {
    beamPulses.push_back(_readerQueue[ii]);
  }
  
  // create new beam
  
  Beam *beam = new Beam(_progName, _params);
  beam->setPulses(_scanType == SCAN_TYPE_RHI,
                  _nSamples,
                  _nGates,
                  _nGatesPrtLong,
                  _indexedBeams,
                  _indexedRes,
                  _isAlternating,
                  _isStaggeredPrt,
                  _prt,
                  _prtLong,
                  _pulseReader->getOpsInfo(),
                  beamPulses);
  
  return beam;
  
}

////////////////////////////////////////////////////////////////////
// position to get the previous beam in realtime or archive sequence
// we need to reset the queue and position to read the previous beam
// returns 0 on success, -1 on error

int TsReader::_positionReaderForPreviousBeam()
  
{

  _clearPulseReaderQueue();
    
  // save the location to which we want to return

  int64_t seekLoc = _nPulsesRead - (_nSamples + 4) * 2;
  if (seekLoc < 0) {
    seekLoc = 0;
  }

  // reset the pulse queue

  _pulseReader->reset();

  // read pulses to move to seek location

  _nPulsesRead = 0;
  while (_nPulsesRead < seekLoc) {
    IwrfTsPulse *pulse = _pulseReader->getNextPulse(true);
    if (pulse == NULL) {
      cerr << "ERROR - TsReader::_positionReaderForPreviousBeam()" << endl;
      cerr << "  Cannot reposition to seekLoc: " << seekLoc << endl;
      _clearPulseReaderQueue();
      return -1;
    }
    _prevPulseSeqNum = pulse->get_pulse_seq_num();
    _nPulsesRead++;
    delete pulse;
  }

  return 0;

}

////////////////////////////////////////////////////////////////////
// position at end of queue

void TsReader::seekToEndOfQueue()
  
{
  if (_pulseReader) {
    _pulseReader->seekToEnd();
  }
}

/////////////////////////////////////////////////
// add the pulse to the pulse queue for the getter
    
void TsReader::_addPulseToGetterQueue(const IwrfTsPulse *pulse)
  
{
  _getterQueue.push_back(pulse);
}

//////////////////////////////////////////////////////////////////
// clear pulse queue for the getter
// does not free pulses

void TsReader::_clearPulseGetterQueue()

{
  _getterQueue.clear();
}

/////////////////////////////////////////////////
// add the pulse to the pulse queue for the reader
// increments client count
    
void TsReader::_addPulseToReaderQueue(const IwrfTsPulse *pulse)
  
{
  // push pulse onto back of queue
  pulse->addClient();
  _readerQueue.push_back(pulse);
}

//////////////////////////////////////////////////////////////////
// clear pulse queue for the reader
// frees pulses

void TsReader::_clearPulseReaderQueue()

{
  for (size_t ii = 0; ii < _readerQueue.size(); ii++) {
    // only delete the pulse object if it is no longer being used
    if (_readerQueue[ii]->removeClient() == 0) {
      delete _readerQueue[ii];
    }
  } // ii
  _readerQueue.clear();
}

///////////////////////////////////////////      
// check if we have alternating h/v pulses
// 
// Also check that we start on a horizontal pulse
// If so, the queue is ready to make a beam

void TsReader::_checkIsAlternating()
  
{
  
  _isAlternating = false;

  bool prevHoriz = _readerQueue[0]->isHoriz();
  for (int i = 1; i < (int) _readerQueue.size(); i++) {
    bool thisHoriz = _readerQueue[i]->isHoriz();
    if (thisHoriz == prevHoriz) {
      return;
    }
    prevHoriz = thisHoriz;
  }
  _isAlternating = true;
  
}

///////////////////////////////////////////      
// check if we have staggered PRT mode
//
// Also check that we start on a short prt
// If so, the queue is ready to make a beam

void TsReader::_checkIsStaggeredPrt()

{

  _isStaggeredPrt = false;
  
  double prt0 = _readerQueue[0]->getPrt(); // most recent pulse
  double prt1 = _readerQueue[1]->getPrt(); // next most recent pulse

  int nGates0 = _readerQueue[0]->getNGates();
  int nGates1 = _readerQueue[1]->getNGates();

  if (fabs(prt0 - prt1) < 0.00001) {
    return;
  }

  for (int ii = 2; ii < _nSamples + 1; ii += 2) {
    if (fabs(_readerQueue[ii]->getPrt() - prt0) > 0.00001) {
      return;
    }
    if (_readerQueue[ii]->getNGates() != nGates0) {
      return;
    }
  }

  for (int ii = 1; ii < _nSamples + 1; ii += 2) {
    if (fabs(_readerQueue[ii]->getPrt() - prt1) > 0.00001) {
      return;
    }
    if (_readerQueue[ii]->getNGates() != nGates1) {
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
  } else {
    _prtShort = prt1;
    _prtLong = prt0;
    _nGatesPrtShort = nGates0;
    _nGatesPrtLong = nGates1;
  }

  _prt = _prtShort;
  _nGates = _nGatesPrtShort;

}

////////////////////////////////
// condition azimuth angle

double TsReader::_conditionAz(double az)
  
{
  if (az > 360.0) {
    return (az - 360);
  } else if (az < 0) {
    return (az + 360.0);
  }
  return az;
}
  
double TsReader::_conditionAz(double az, double refAz)
  
{
  double diff = az - refAz;
  if (diff > 180.0) {
    return (az - 360);
  } else if (diff < -180) {
    return (az + 360.0);
  }
  return az;
}
  
////////////////////////////////
// condition elevation angle

double TsReader::_conditionEl(double el)
  
{
  if (el > 180.0) {
    return (el - 360);
  } else if (el < -180) {
    return (el + 360.0);
  }
  return el;
}
  
