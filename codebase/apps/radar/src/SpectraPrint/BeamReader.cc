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
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////
//
// Reads in pulses, creates beams.
//
////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include "BeamReader.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

BeamReader::BeamReader(const string &prog_name,
                       const Params &params,
                       const Args &args,
		       BeamMgr &mgr) :
        _progName(prog_name),
        _params(params),
        _args(args),
	_beamMgr(mgr)

{

  constructorOK = true;

  _pulseReader = NULL;

  _maxPulseQueueSize = 0;
  _pulseSeqNum = 0;
  
  _nSamples = _params.n_samples;
  
  _beamCount = 0;
  _midIndex1 = 0;
  _midIndex2 = 0;
  _pulseCountSinceBeam = 0;
  _isAlternating = false;
  _isStaggeredPrt = false;
  
  _time = 0;
  _az = 0.0;
  _el = 0.0;

  _nGates = 0;

  _prt = 0.0;
  _prtShort = 0.0;
  _prtLong = 0.0;

  // start in PPI mode

  _initPpiMode();
  
  // check that file list set in archive and simulate mode
  
  if (_params.mode == Params::ARCHIVE && args.inputFileList.size() == 0) {
    cerr << "ERROR: BeamReader::BeamReader." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    args.usage(_progName, cerr);
    constructorOK = false;
    return;
  }
    
  // create the pulse reader
  
  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug) {
    iwrfDebug = IWRF_DEBUG_NORM;
  } 
    
  if (_params.mode == Params::FMQ) {
    _pulseReader = new IwrfTsReaderFmq(_params.input_fmq,
                                        iwrfDebug,
                                        _params.position_fmq_at_start);
  } else {
    _pulseReader = new IwrfTsReaderFile(_args.inputFileList, iwrfDebug);
  }
  
  // compute pulse queue size, set to 5 * maxNsamples to allow plenty of
  // space

  _maxPulseQueueSize = _params.n_samples * 5;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "_maxPulseQueueSize: " << _maxPulseQueueSize << endl;
  }

}

//////////////////////////////////////////////////////////////////
// destructor

BeamReader::~BeamReader()

{

  if (_pulseReader) {
    delete _pulseReader;
  }

  for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
    // only delete the pulse object if it is no longer being used
    if (_pulseQueue[ii]->removeClient() == 0) {
      delete _pulseQueue[ii];
    }
  } // ii
  _pulseQueue.clear();

}

//////////////////////////////////////////////////
// get the next beam
// returns Beam object pointer on success, NULL on failure
// caller must free beam

Beam *BeamReader::getNextBeam()
  
{
  
  // read in pulse, load up pulse queue

  while (true) {

    PMU_auto_register("getNextBeam");
  
    IwrfTsPulse *pulse = _pulseReader->getNextPulse(true);
    
    if (pulse == NULL) {
      return NULL;
    }
    
    if (_params.invert_hv_flag) {
      pulse->setInvertHvFlag(true);
    }
    
    if (!_params.prt_is_for_previous_interval) {
      pulse->swapPrtValues();
    }

    // check scan mode and type

    scan_type_t scanType = SCAN_TYPE_PPI;
    int scanMode = pulse->getScanMode();
    if (scanMode == IWRF_SCAN_MODE_RHI) {
      scanType = SCAN_TYPE_RHI;
    }
    if (scanType != _scanType) {
      if (scanType == SCAN_TYPE_PPI) {
        _initPpiMode();
      } else {
        _initRhiMode();
      }
    }

    // Create a new pulse object and save a pointer to it in the
    // _pulseQueue.  _pulseQueue is a FIFO, with elements
    // added at the end and dropped off the beginning. So if we have a
    // full buffer delete the first element before shifting the
    // elements to the left.
    
    // add pulse to queue, managing memory appropriately

    _addPulseToQueue(pulse);
    
    // get number of gates
    
    _nGates = pulse->getNGates();
    
    // is a beam ready?
    
    if (_beamReady()) {
      
      _beamCount++;
      _pulseCountSinceBeam = 0;
      
      // load the pulse pointers into a deque for
      // just this beam
      
      deque<const IwrfTsPulse *> beamPulses;
      for (int ii = 0; ii < _nSamples; ii++) {
        int jj = ii + _offsetToBeamStart;
        beamPulses.push_front(_pulseQueue[jj]);
      }

     // create new beam
      
      Beam *beam = NULL;
      if (_scanType == SCAN_TYPE_PPI) {
        beam = new Beam(_progName, _params,
                        true, _az,
                        _isAlternating, _isStaggeredPrt,
                        _nGates, _nGatesPrtLong,
                        _prt, _prtLong,
                        _beamMgr, _pulseReader->getOpsInfo(),
                        beamPulses);
      } else {
        beam = new Beam(_progName, _params,
                        false, _el,
                        _isAlternating, _isStaggeredPrt,
                        _nGates, _nGatesPrtLong,
                        _prt, _prtLong,
                        _beamMgr, _pulseReader->getOpsInfo(),
                        beamPulses);
      }
      
      return beam;

    } // if (_beamReady())

  } // while ((pulse = _pulseReader->getNextPulse()) != NULL) {

  return NULL;

}

//////////////////////
// initialize ppi mode

void BeamReader::_initPpiMode()

{

  _azIndex = 0;
  _prevAzIndex = -999;
  _scanType = SCAN_TYPE_PPI;

}

//////////////////////
// initialize rhi mode

void BeamReader::_initRhiMode()

{

  _elIndex = 0;
  _prevElIndex = -999;
  _scanType = SCAN_TYPE_RHI;

}

/////////////////////////////////////////////////
// are we ready for a beam?
//
// Side effects: sets _az/_el, _midIndex1, _midIndex2

bool BeamReader::_beamReady()
  
{

  _pulseCountSinceBeam++;
  
  // enough data in the queue?
  // need one extra pulse because we sometimes need to search
  // backwards for alternating or staggered pulse properties
  
  int minPulses = _nSamples + 1;
  if ((int) _pulseQueue.size() < minPulses) {
    return false;
  }
  
  // check we have sequential pulses
  
  if (_params.check_for_missing_pulses) {
    long long prevSeqNum = _pulseQueue[0]->getSeqNum();
    for (int i = 1; i < _nSamples; i++) {
      long long seqNum = _pulseQueue[i]->getSeqNum();
      if (seqNum != 0 && prevSeqNum != 0 &&
	  abs(seqNum != prevSeqNum) > 1) {
	cerr << "WARNING - BeamReader::_beamReady" << endl;
	cerr << "  Sequence number out of order" << endl;
	cerr << "  Latest seq number: " << seqNum << endl;
	cerr << "  Prev   seq number: " << prevSeqNum << endl;
	cerr << "  n pulses missing: " << abs(seqNum - prevSeqNum) - 1 << endl;
	return false;
      }
    }
  }

  // set prt and nGates, assuming single PRT for now

  _prt = _pulseQueue[0]->getPrt();
  _nGates = _pulseQueue[0]->getNGates();

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
    int nGates0 = _pulseQueue[0]->getNGates();
    for (int i = 1; i < (int) _pulseQueue.size(); i += 2) {
      if (_pulseQueue[i]->getNGates() != nGates0) {
        return false;
      }
    }
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
    } else {
      cerr << "==>> Single PRT mode" << endl;
      cerr << "     prt: " << _prt << endl;
      cerr << "     ngates: " << _nGates << endl;
    }
  }

  // compute the indices at the middle of the beam.
  // index1 is just short of the midpt
  // index2 is just past the midpt
  
  _midIndex1 = _nSamples / 2;
  _midIndex2 = _midIndex1 - 1;

  bool isReady = false;
  if (_scanType == SCAN_TYPE_PPI) {

    if (_beamReadyPpi()) {
      _prevAzIndex = _azIndex;
      isReady = true;
    }
 
  } else {

    if (_beamReadyRhi()) {
      _prevElIndex = _elIndex;
      isReady = true;
    }

  }

  // if beam is ready, check that we start on the correct
  // pulse type for alternating or staggered PRT mode

  if (isReady) {
    if (_isAlternating && !_startsOnHoriz) {
      _offsetToBeamStart = 1;
    } else if (_isStaggeredPrt && !_startsOnPrtShort) {
      _offsetToBeamStart = 1;
    } else {
      _offsetToBeamStart = 0;
    }
  }

  return isReady;

}
      
/////////////////////////////////////////////////
// are we ready for a beam? PPI mode
//
// Side effects: sets _az, _midIndex1, _midIndex2

bool BeamReader::_beamReadyPpi()
  
{
  
  _az = 0.0;
  
  // compute azimuths which need to straddle the center of the beam
  
  double midAz1 = _pulseQueue[_midIndex1]->getAz();
  double midAz2 = _pulseQueue[_midIndex2]->getAz();

  bool indexTheBeam = _params.index_the_beams;

  // set resolution in azimuth

  double angularRes = _params.indexed_resolution;
  if (indexTheBeam) {
    int nazPer45 = (int) (45.0 / angularRes + 0.5);
    angularRes = 45.0 / nazPer45;
  }

  if (indexTheBeam) {

    // compute azimuth index
    
    _azIndex = (int) (midAz1 / angularRes + 0.5);
    int nAz = (int) (360.0 / angularRes + 0.5);
    if (_azIndex == nAz) {
      _azIndex = 0;
    }

    // check for duplicate index with previous one

    if (_azIndex == _prevAzIndex) {
      return false;
    }

    // compute target azimiuth by rounding the azimuth at the
    // center of the data to the closest suitable az
    
    _az = _azIndex * angularRes;
    
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
      
      if (midAz1 > 360.0 - angularRes &&
	  midAz2 < angularRes) {
	
	// az1 is below 0 and az2 above 0 - clockwise rotation
	return true;
	
      } else if (midAz2 > 360.0 - angularRes &&
		 midAz1 < angularRes) {
	
	// az1 is above 0 and az2 below 0 - counterclockwise rotation
	return true;
	
      }
      
    } else if (_pulseCountSinceBeam > (_nSamples * 16)) {
      
      // antenna moving very slowly, we have waited long enough
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Antenna moving very slowly - _pulseCountSinceBeam = "
	     << _pulseCountSinceBeam 
	     << " Returning beam anyway" << endl;
      }
      return true;
      
    }
    
  } else {
    
    // do not index - only check we have enough data
    
    if (_pulseCountSinceBeam >= _nSamples) {
      _az = _conditionAz(midAz1);
      return true;
    }
    
  }
  
  return false;

}

/////////////////////////////////////////////////
// are we ready for a beam? RHI mode
//
// Side effects: sets _el, _midIndex1, _midIndex2

bool BeamReader::_beamReadyRhi()
  
{
  
  _el = 0.0;
  
  // compute elevations which need to straddle the center of the beam
  
  double midEl1 = _conditionEl(_pulseQueue[_midIndex1]->getEl());
  double midEl2 = _conditionEl(_pulseQueue[_midIndex2]->getEl());

  // set resolution in elevation

  bool indexTheBeam = _params.index_the_beams;
  double angularRes = _params.indexed_resolution;

  if (indexTheBeam) {
    
    // compute elevation index
    
    _elIndex = (int) ((midEl1 + 180.0) / angularRes + 0.5);
    int nEl = (int) (360.0 / angularRes + 0.5);
    if (_elIndex == nEl) {
      _elIndex = 0;
    }
    
    // check for duplicate index with previous one
    
    if (_elIndex == _prevElIndex) {
      return false;
    }

    // compute target elevation by rounding the elevation at the
    // center of the data to the closest suitable el
    
    _el = -180.0 + (_elIndex * angularRes);

    // Check if the elevations at the center of the data straddle
    // the target elevation
    
    if (midEl1 <= _el && midEl2 >= _el) {
      
      // el1 is below and el2 above - el increasing
      return true;
      
    } else if (midEl1 >= _el && midEl2 <= _el) {
      
      // el1 is above and el2 below - el decreasing
      return true;
      
    } else if (_pulseCountSinceBeam > (_nSamples * 16)) {
      
      // antenna moving very slowly, we have waited long enough
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Antenna moving very slowly - _pulseCountSinceBeam = "
	     << _pulseCountSinceBeam 
	     << " Returning beam anyway" << endl;
      }
      return true;
      
    }
    
  } else {
    
    // do not index - only check we have enough data
    
    if (_pulseCountSinceBeam >= _nSamples) {
      _el = _conditionEl(midEl1);
      return true;
    }
    
  }
  
  return false;

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

  if (_pulseQueue[0]->isHoriz()) {
    _startsOnHoriz = false;
  } else {
    _startsOnHoriz = true;
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
  
  double prt0 = _pulseQueue[0]->getPrt(); // most recent pulse
  double prt1 = _pulseQueue[1]->getPrt(); // next most recent pulse

  int nGates0 = _pulseQueue[0]->getNGates();
  int nGates1 = _pulseQueue[1]->getNGates();

  if (fabs(prt0 - prt1) < 0.00001) {
    return;
  }

  for (int ii = 2; ii < _nSamples + 1; ii += 2) {
    if (fabs(_pulseQueue[ii]->getPrt() - prt0) > 0.00001) {
      return;
    }
    if (_pulseQueue[ii]->getNGates() != nGates0) {
      return;
    }
  }

  for (int ii = 1; ii < _nSamples + 1; ii += 2) {
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
  _nGates = _nGatesPrtShort;

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
  

/////////////////////////////////////////////////
// add the pulse to the pulse queue
    
void BeamReader::_addPulseToQueue(const IwrfTsPulse *pulse)
  
{

  // manage the size of the pulse queue, popping off the back
  
  if ((int) _pulseQueue.size() > _maxPulseQueueSize) {
    const IwrfTsPulse *oldest = _pulseQueue.back();
    // if (oldest->removeClient("Deleting from queue") == 0) {
      delete oldest;
      // }
    _pulseQueue.pop_back();
  }
  
  // push pulse onto front of queue
  
  pulse->addClient();
  _pulseQueue.push_front(pulse);

  // print missing pulses if requested
  
  if ((int) pulse->getSeqNum() != (int) (_pulseSeqNum + 1)) {
    if (_params.debug >= Params::DEBUG_VERBOSE && _pulseSeqNum != 0) {
      cerr << "**************** Missing seq num: Expected=" << _pulseSeqNum+1
	   << " Rx'd=" <<  pulse->getSeqNum() << " **************" << endl;
    }
  }
  _pulseSeqNum = pulse->getSeqNum();

}

