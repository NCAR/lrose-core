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
// SimCpCompute.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////
//
// SimCpCompute analyses time series data from vertical scans
//
////////////////////////////////////////////////////////////////

#include "SimCpCompute.hh"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <radar/RadarComplex.hh>

using namespace std;

// Constructor

SimCpCompute::SimCpCompute(int argc, char **argv)
  
{

  isOK = true;
  _pulseSeqNum = 0;
  _totalPulseCount = 0;
  _rangeCorr = NULL;

  _beamAz = 0.0;
  _beamEl = 0.0;
  _midIndex1 = 0;
  _midIndex2 = 0;
  _clockwise = false;
  _gateSpacingWarningPrinted = false;

  _currentPpi = NULL;
  _prevPpi = NULL;

  _countVcMinusHc = 0.0;
  _sumVcMinusHc = 0.0;
  _meanVcMinusHc = -9999;

  _countVxMinusHx = 0.0;
  _sumVxMinusHx = 0.0;
  _sum2VxMinusHx = 0.0;
  _meanVxMinusHx = -9999;

  _reader = NULL;
  _horizReader = NULL;
  _vertReader = NULL;

  // set programe name
  
  _progName = "SimCpCompute";

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

  // initialize variables which depend on params

  _nSamples = _params.n_samples;
  _nSamplesHalf = _nSamples / 2;
  _clockwise = _params.clockwise_rotation;
  
  _startRangeAnalysis = _params.start_range;
  _endRangeAnalysis = _params.end_range;
  _gateSpacingAnalysis = _params.expected_gate_spacing;

  // start at gate 1 since burst is in gate 0

  _startGateAnalysis =
    (int) (_startRangeAnalysis / _gateSpacingAnalysis + 0.5) + 1;
  _endGateAnalysis =
    (int) (_endRangeAnalysis / _gateSpacingAnalysis + 0.5) + 1;
  _nGatesAnalysis = _endGateAnalysis - _startGateAnalysis + 1;

  if (_params.debug) {
    cerr << "=======================================================" << endl;
    cerr << "Running: " << _progName << endl;
    cerr << "  nSamples: " << _nSamples << endl;
    cerr << "  clockwise: " << (_clockwise? "true" : "false") << endl;
    cerr << "  startRangeAnalysis: " << _startRangeAnalysis << endl;
    cerr << "  endRangeAnalysis: " << _endRangeAnalysis << endl;
    cerr << "  gateSpacingAnalysis: " << _gateSpacingAnalysis << endl;
    cerr << "  startGateAnalysis: " << _startGateAnalysis << endl;
    cerr << "  endGateAnalysis: " << _endGateAnalysis << endl;
    cerr << "  nGatesAnalysis: " << _nGatesAnalysis << endl;
    cerr << "  min_snr: " << _params.min_snr << endl;
    cerr << "  max_snr: " << _params.max_snr << endl;
    cerr << "  max_vel: " << _params.max_vel << endl;
    cerr << "  max_width: " << _params.max_width << endl;
  }

  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  return;
  
}

// destructor

SimCpCompute::~SimCpCompute()

{

  // close readers
  
  if (_reader) {
    delete _reader;
  }
  if (_horizReader) {
    delete _horizReader;
  }
  if (_vertReader) {
    delete _vertReader;
  }

  // clean up memory
  
  _clearPulseQueue();
  _clearHorizQueue();
  _clearVertQueue();
  
  if (_rangeCorr != NULL) {
    delete[] _rangeCorr;
    _rangeCorr = NULL;
  }

  if (_currentPpi) {
    delete _currentPpi;
    _currentPpi = NULL;
  }

  if (_prevPpi) {
    delete _prevPpi;
    _prevPpi = NULL;
  }

}

//////////////////////////////////////////////////
// Run

int SimCpCompute::Run ()
{

  int iret = 0;
  if (_params.pointing_mode) {
    iret = _runPointingMode();
  } else {
    iret = _runMovingMode();
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in moving mode

int SimCpCompute::_runMovingMode()
{

  // create reader

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug) {
    iwrfDebug = IWRF_DEBUG_NORM;
  }
  if (_params.input_mode == Params::TS_FILE_INPUT) {
    _reader = new IwrfTsReaderFile(_args.inputFileList, iwrfDebug);
  } else {
    _reader = new IwrfTsReaderFmq(_params.input_fmq_name, iwrfDebug);
  }

  while (true) {

    const IwrfTsPulse *pulse = _reader->getNextPulse(true);
    if (pulse == NULL) {
      break;
    }
    
    _processPulse(pulse);
    
  }

  return 0;

}

//////////////////////////////////////////////////
// Run in pointing mode

int SimCpCompute::_runPointingMode()
{

  // set up readers

  if (_params.debug) {
    _horizReader = new IwrfTsReaderFile(_args.horizFileList, IWRF_DEBUG_NORM);
    _vertReader = new IwrfTsReaderFile(_args.vertFileList, IWRF_DEBUG_NORM);
  } else {
    _horizReader = new IwrfTsReaderFile(_args.horizFileList);
    _vertReader = new IwrfTsReaderFile(_args.vertFileList);
  }

  // loop until data exhausted
  
  while (true) {

    // load horizontal queue

    if (_loadHorizQueue()) {
      return 0;
    }

    // load vertical queue

    if (_loadVertQueue()) {
      return 0;
    }

    // check number of gates is constant
    
    _nGates = _horizQueue[0]->getNGates();
    if (_nGates > _vertQueue[0]->getNGates()) {
      _nGates = _vertQueue[0]->getNGates();
    }
    
    if (_endGateAnalysis > _nGates - 1) {
      _endGateAnalysis = _nGates - 1;
      _nGatesAnalysis = _endGateAnalysis - _startGateAnalysis + 1;
    }

    // compute pointing results
    
    _computePointing();

  } // while (true)
  
  return 0;
  
}

/////////////////////
// process a pulse

void SimCpCompute::_processPulse(const IwrfTsPulse *pulse)

{

  // add the pulse to the queue

  _addPulseToQueue(pulse);
  _totalPulseCount++;

  // do we have a beam ready?

  if (_beamReady()) {

    // process the beam

    _processBeam();

    // clear the pulse queue

    _clearPulseQueue();
    
  }

}

/////////////////////
// process a beam

void SimCpCompute::_processBeam()

{

  // compute the moments
  
  const IwrfTsInfo &info = _reader->getOpsInfo();
  _initForMoments(info);
  _computeMomentsDualAlt(_pulseQueue, info, _momentData);
  _markValidMoments(_momentData);

  // check ppi exists
  
  if (_currentPpi == NULL) {
    _currentPpi = new Ppi(_params);
  }
  
  // can we add this beam to the current PPI?
  
  if (_currentPpi->azExists(_beamAz)) {
    // azimuth would overwrite, and yet the current PPI
    // is not full, so we have an error and must start over
    if (_params.debug) {
      cerr << "ERROR - SimCpCompute" << endl;
      cerr << "  Ppi not complete, but azimuth would overwrite" << endl;
      cerr << "  az: " << _beamAz << endl;
    }
    delete _currentPpi;
    _currentPpi = new Ppi(_params);
  }
  
  // create a new beam, add to the current ppi
  // the ppi object will be responsible for deleting it
  
  Beam *beam = new Beam(_params, _beamTime, _beamAz, _beamEl, _momentData);
  _currentPpi->addBeam(beam);
  
  // is the current ppi is complete?
  
  if (_currentPpi->complete()) {

    _currentPpi->computeProps();

    if (_prevPpi != NULL) {

      // analyze the differences between PPIs
      
      double vcMinusHc;
      double vxMinusHx;
      if (_currentPpi->computeCpDiff(*_prevPpi, vcMinusHc, vxMinusHx) == 0) {
        _countVcMinusHc++;
        _sumVcMinusHc += vcMinusHc;
        _meanVcMinusHc = _sumVcMinusHc / _countVcMinusHc;
        _countVxMinusHx++;
        _sumVxMinusHx += vxMinusHx;
        _sum2VxMinusHx += vxMinusHx * vxMinusHx;
        _meanVxMinusHx = _sumVxMinusHx / _countVxMinusHx;
        _sdevVxMinusHx = -9999;
        if (_countVxMinusHx > 2) {
          double term1 = _sum2VxMinusHx / _countVxMinusHx;
          double term2 = _meanVxMinusHx * _meanVxMinusHx;
          if (term1 >= term2) {
            _sdevVxMinusHx = sqrt(term1 - term2);
          }
        }
      }

      _currentPpi->print(cout);

      cout << "==================================================" << endl;
      cout << "From previous pair of PPIs:" << endl;
      cout << "  vc-hc: " << vcMinusHc << endl;
      cout << "  vx-hx: " << vxMinusHx << endl;
      cout << "From all pairs of PPIs so far:" << endl;
      cout << "  N ppi pairs: " << _countVxMinusHx << endl;
      cout << "  Mean vc-hc:  " << _meanVcMinusHc << endl;
      cout << "  Mean vx-hx:  " << _meanVxMinusHx << endl;
      cout << "  Sdev vx-hx:  " << _sdevVxMinusHx << endl;
      cout << "==================================================" << endl;

      // delete the previous ppi

      delete _prevPpi;

    } else {

      cout << "========= NO PREVIOUS PPI AVAILABLE ==============" << endl;
      _currentPpi->print(cout);

    }
    
    // move the current ppi to the previous one

    _prevPpi = _currentPpi;

    // start a new current ppi

    _currentPpi = new Ppi(_params);

  }
  
}

//////////////////////////
// compute pointing data

void SimCpCompute::_computePointing()

{

  // compute the moments
  
  vector<MomentData> horizMoments;
  vector<MomentData> vertMoments;

  const IwrfTsInfo &horizInfo = _horizReader->getOpsInfo();
  _initForMoments(horizInfo);
  _computeMomentsDualAlt(_horizQueue, horizInfo, horizMoments);
  _markValidMoments(horizMoments);
  
  const IwrfTsInfo &vertInfo = _horizReader->getOpsInfo();
  _initForMoments(vertInfo);
  _computeMomentsDualAlt(_vertQueue, vertInfo, vertMoments);
  _markValidMoments(vertMoments);
  
  // time

  time_t horizTime = _horizQueue[_nSamples / 2]->getTime();
  time_t vertTime = _vertQueue[_nSamples / 2]->getTime();
  
  // power differences
  
  double sum_hc_minus_vx = 0.0;
  double sum_vc_minus_hx = 0.0;
  double sum_vx_minus_hx = 0.0;
  double validCount = 0.0;
  
  for (int ii = 0; ii < (int) horizMoments.size(); ii++) {
    
    const MomentData &hmom = horizMoments[ii];
    if (!hmom.valid) {
      continue;
    }
    
    const MomentData &vmom = vertMoments[ii];
    if (!vmom.valid) {
      continue;
    }
      
    double hc_minus_vx = hmom.dbmhc - hmom.dbmvc;
    double vc_minus_hx = vmom.dbmvc - vmom.dbmhc;
    double vx_minus_hx = hmom.dbmvx - vmom.dbmhx;
    
    sum_hc_minus_vx += hc_minus_vx;
    sum_vc_minus_hx += vc_minus_hx;
    sum_vx_minus_hx += vx_minus_hx;

    validCount++;
    
  } // ii

  double mean_hc_minus_vx = -9999;
  double mean_vc_minus_hx = -9999;
  double mean_vx_minus_hx = -9999;

  if (validCount != 0) {
    mean_hc_minus_vx = sum_hc_minus_vx / validCount;
    mean_vc_minus_hx = sum_vc_minus_hx / validCount;
    mean_vx_minus_hx = sum_vx_minus_hx / validCount;
  }

  _sumVxMinusHx += mean_vx_minus_hx;
  _countVxMinusHx++;
  _meanVxMinusHx = _sumVxMinusHx / _countVxMinusHx;

  cout << "======= SimCpCompute in pointing mode ============" << endl;
  cout << "  htime: " << DateTime::strm(horizTime) << endl;
  cout << "  vtime: " << DateTime::strm(vertTime) << endl;
  cout << "  mean_hc_minus_vx:  " << mean_hc_minus_vx << endl;
  cout << "  mean_vc_minus_hx:  " << mean_vc_minus_hx << endl;
  cout << "  mean_vx_minus_hx:  " << mean_vx_minus_hx << endl;
  cout << "  Mean vx-hx so far: " << _meanVxMinusHx << endl;
  cout << "==================================================" << endl;
  
}

/////////////////////////////////////////////////
// add the pulse to the pulse queue
    
void SimCpCompute::_addPulseToQueue(const IwrfTsPulse *pulse)
  
{

  // manage the size of the pulse queue, popping off the back

  if ((int) _pulseQueue.size() >= _nSamples) {
    const IwrfTsPulse *oldest = _pulseQueue.front();
    if (oldest->removeClient() == 0) {
      delete oldest;
    }
    _pulseQueue.pop_front();
  }

  int qSize = (int) _pulseQueue.size();
  if (qSize > 1) {

    // check for big azimuth or elevation change
    // if so clear the queue
    
    double az = pulse->getAz();
    double el = pulse->getEl();
    double prevAz = _pulseQueue[qSize-1]->getAz();
    double prevEl = _pulseQueue[qSize-1]->getEl();
    
    double azDiff = RadarComplex::diffDeg(az, prevAz);
    double elDiff = RadarComplex::diffDeg(el, prevEl);
    if (fabs(azDiff) > 0.1 || fabs(elDiff) > 0.1) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "====>> Clearing pulse queue" << endl;
        cerr << "  az, prevAz: " << az << ", " << prevAz << endl;
        cerr << "  el, prevEl: " << el << ", " << prevEl << endl;
      }
      _clearPulseQueue();
      return;
    }

  }
    
  // push pulse onto front of queue
  
  pulse->addClient();
  _pulseQueue.push_back(pulse);

  // print missing pulses in verbose mode
  
  if ((int) pulse->getSeqNum() != _pulseSeqNum + 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE && _pulseSeqNum != 0) {
      cerr << "**** Missing seq num: " << _pulseSeqNum
	   << " to " <<  pulse->getSeqNum() << " ****" << endl;
    }
  }
  _pulseSeqNum = pulse->getSeqNum();

  // check number of gates is constant

  qSize = (int) _pulseQueue.size();
  _nGates = _pulseQueue[0]->getNGates();
  for (int ii = 1; ii < qSize; ii++) {
    if (_pulseQueue[ii]->getNGates() != _nGates) {
      _clearPulseQueue();
      return;
    }
  }

  if (_endGateAnalysis > _nGates - 1) {
    _endGateAnalysis = _nGates - 1;
    _nGatesAnalysis = _endGateAnalysis - _startGateAnalysis + 1;
  }

}

/////////////////////////////////////////////////
// clear the pulse queue
    
void SimCpCompute::_clearPulseQueue()
  
{

  for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
    if (_pulseQueue[ii]->removeClient() == 0) {
      delete _pulseQueue[ii];
    }
  }
  _pulseQueue.clear();

}

//////////////////////////////////////////////////
// load horiz queue

int SimCpCompute::_loadHorizQueue()
{

  _clearHorizQueue();

  while (true) {

    const IwrfTsPulse *pulse = _reader->getNextPulse(true);
    if (pulse == NULL) {
      break;
    }
    
    // check that we start with a horizontal pulse
      
    if (_horizQueue.size() == 0 && !pulse->isHoriz()) {
      continue;
    }
    
    // add pulse to queue
    
    _horizQueue.push_back(pulse);
    
    if ((int) _horizQueue.size() == _nSamples) {
      return 0;
    }

  } // while (true)

  return -1;

}

//////////////////////////////////////////////////
// load vert queue

int SimCpCompute::_loadVertQueue()
{

  _clearVertQueue();

  while (true) {

    const IwrfTsPulse *pulse = _reader->getNextPulse(true);
    if (pulse == NULL) {
      break;
    }
    
    // check that we start with a horizontal pulse
    
    if (_vertQueue.size() == 0 && !pulse->isHoriz()) {
      continue;
    }
    
    // add pulse to queue
    
    _vertQueue.push_back(pulse);
    
    if ((int) _vertQueue.size() == _nSamples) {
      return 0;
    }

  } // while (true)

  return -1;

}

/////////////////////////////////////////////////
// clear the horiz pulse queue
    
void SimCpCompute::_clearHorizQueue()
  
{

  for (size_t ii = 0; ii < _horizQueue.size(); ii++) {
    delete _horizQueue[ii];
  }
  _horizQueue.clear();

}

/////////////////////////////////////////////////
// clear the vert pulse queue
    
void SimCpCompute::_clearVertQueue()
  
{

  for (size_t ii = 0; ii < _vertQueue.size(); ii++) {
    delete _vertQueue[ii];
  }
  _vertQueue.clear();

}

/////////////////////////////////////////////////
// do we have a good beam?
//
// Side effects: sets _beamAz, _beamEl, _midIndex1, _midIndex2

bool SimCpCompute::_beamReady()
  
{
  
  // enough data in the queue?

  if ((int) _pulseQueue.size() < _nSamples) {
    return false;
  }

  // make sure we start on a pulse with HV flag set to H

  if (!_pulseQueue[0]->isHoriz()) {
    return false;
  }

  // check we have constant nGates
  
  int nGates = _pulseQueue[0]->getNGates();
  for (int i = 1; i < _nSamples; i++) {
    if (_pulseQueue[i]->getNGates() != nGates) {
      return false;
    }
  }
  
  // correct gate spacing?

  const IwrfTsInfo &info = _reader->getOpsInfo();
  double deltaSpacing =
    info.get_proc_gate_spacing_km() - _params.expected_gate_spacing;
  
  if (fabs(deltaSpacing) > 0.001) {
    if (!_gateSpacingWarningPrinted && _params.debug) {
      cerr << "WARNING - incorrect gate spacing" << endl;
      cerr << "  Expecting: " << _params.expected_gate_spacing << endl;
      cerr << "  Got: " << info.get_proc_gate_spacing_km() << endl;
      _gateSpacingWarningPrinted = true;
    }
    return false;
  }
  _gateSpacingWarningPrinted = false;

  // check we have constant prt
  
  double prt = _pulseQueue[0]->getPrt();
  for (int i = 1; i < _nSamples; i++) {
    if (fabs(_pulseQueue[i]->getPrt() - prt) > 0.001) {
      return false;
    }
  }
  
  // initialize

  _beamAz = 0.0;
  _beamEl = 0.0;
  _clockwise = _params.clockwise_rotation;
  
  // compute the indices at the middle of the beam.
  // index1 is just short of the midpt
  // index2 is just past the midpt
  
  _midIndex1 = _nSamples / 2 - 1;
  _midIndex2 = _midIndex1 + 2;

  // compute azimuths which need to straddle the center of the beam
  
  double midAz1 = _pulseQueue[_midIndex1]->getAz();
  double midAz2 = _pulseQueue[_midIndex2]->getAz();
  _beamEl = _pulseQueue[_midIndex1]->getEl();
  _beamTime = _pulseQueue[_midIndex1]->getFTime();

  // compute target azimiuth by rounding the azimuth at the
  // center of the data to the closest suitable az
  
  _beamAz = ((int) (midAz1 / _params.delta_az + 0.5)) * _params.delta_az;
    
  if (_beamAz >= 360.0) {
    _beamAz -= 360;
  } else if (_beamAz < 0) {
    _beamAz += 360.0;
  }
  
  // Check if the azimuths at the center of the data straddle
  // the target azimuth
  
  if (midAz1 <= _beamAz && midAz2 >= _beamAz) {
    
    // az1 is below and az2 above - clockwise rotation

    _clockwise = true;
    if (_params.clockwise_rotation) {
      return true;
    } else {
      return false;
    }
    
  } else if (midAz1 >= _beamAz && midAz2 <= _beamAz) {
    
    // az1 is above and az2 below - counterclockwise rotation

    _clockwise = true;
    if (!_params.clockwise_rotation) {
      return true;
    } else {
      return false;
    }
    
  } else if (_beamAz == 0.0) {
    
    if (midAz1 > 360.0 - _params.delta_az &&
        midAz2 < _params.delta_az) {
      
      // az1 is below 0 and az2 above 0 - clockwise rotation
      
      _clockwise = false;
      if (_params.clockwise_rotation) {
        return true;
      } else {
        return false;
      }
      
    } else if (midAz2 > 360.0 - _params.delta_az &&
               midAz1 < _params.delta_az) {
      
      // az1 is above 0 and az2 below 0 - counterclockwise rotation

      _clockwise = false;
      if (!_params.clockwise_rotation) {
        return true;
      } else {
        return false;
      }
      
    }

  }

  return false;

}

////////////////////////////////////////////
// compute properties using pulses in queue
//
// Loads up momentData vector

void SimCpCompute::_computeMomentsDualAlt
  (const deque<const IwrfTsPulse *> &pulseQueue,
   const IwrfTsInfo &opsInfo,
   vector<MomentData> &momentData)

{
  
  // set up data pointer array for each channel
  
  const fl32 **iqData0 = new const fl32*[_nSamples];
  const fl32 **iqData1 = new const fl32*[_nSamples];
  for (int ii = 0; ii < _nSamples; ii++) {
    iqData0[ii] = pulseQueue[ii]->getIq0();
    iqData1[ii] = pulseQueue[ii]->getIq1();
  }
  
  // load up IQ data array
  
  Complex_t **IQ0 = (Complex_t **)
    ucalloc2(_nGates, _nSamples, sizeof(Complex_t));
  Complex_t **IQ1 = (Complex_t **)
    ucalloc2(_nGates, _nSamples, sizeof(Complex_t));
  
  for (int igate = 0, posn = 0; igate < _nGates; igate++, posn += 2) {
    Complex_t *iq0 = IQ0[igate];
    Complex_t *iq1 = IQ1[igate];
    for (int isamp = 0; isamp < _nSamples; isamp++, iq0++, iq1++) {
      iq0->re = iqData0[isamp][posn];
      iq0->im = iqData0[isamp][posn + 1];
      iq1->re = iqData1[isamp][posn];
      iq1->im = iqData1[isamp][posn + 1];
    } // isamp
  } // igate
  
  // load up IQH and IQV data arrays
  
  Complex_t **IQHC =
    (Complex_t **) ucalloc2(_nGates, _nSamplesHalf, sizeof(Complex_t));
  Complex_t **IQHX =
    (Complex_t **) ucalloc2(_nGates, _nSamplesHalf, sizeof(Complex_t));
  Complex_t **IQVC =
    (Complex_t **) ucalloc2(_nGates, _nSamplesHalf, sizeof(Complex_t));
  Complex_t **IQVX =
    (Complex_t **) ucalloc2(_nGates, _nSamplesHalf, sizeof(Complex_t));
  
  for (int igate = 0, posn = 0; igate < _nGates; igate++, posn += 2) {
    Complex_t *iqhc = IQHC[igate];
    Complex_t *iqhx = IQHX[igate];
    Complex_t *iqvc = IQVC[igate];
    Complex_t *iqvx = IQVX[igate];
    for (int isamp = 0; isamp < _nSamples;
         isamp += 2, iqhc++, iqhx++, iqvc++, iqvx++) {
      iqhc->re = iqData0[isamp][posn];
      iqhc->im = iqData0[isamp][posn + 1];
      iqvx->re = iqData1[isamp][posn];
      iqvx->im = iqData1[isamp][posn + 1];
      iqvc->re = iqData0[isamp + 1][posn];
      iqvc->im = iqData0[isamp + 1][posn + 1];
      iqhx->re = iqData1[isamp + 1][posn];
      iqhx->im = iqData1[isamp + 1][posn + 1];
    } // isamp
  } // igate

  double prt = pulseQueue[0]->getPrt();
  momentData.clear();

  double sumHc = 0.0;
  double sumVx = 0.0;
  double nnn = 0.0;

  for (int igate = _startGateAnalysis; igate <= _endGateAnalysis; igate++) {
    
    MomentData mdata;

    Complex_t *iqhc = IQHC[igate];
    Complex_t *iqhx = IQHX[igate];
    Complex_t *iqvc = IQVC[igate];
    Complex_t *iqvx = IQVX[igate];
    
    // power
    
    double power_hc = _computePower(iqhc, _nSamplesHalf);
    double power_hx = _computePower(iqhx, _nSamplesHalf);
    double power_vc = _computePower(iqvc, _nSamplesHalf);
    double power_vx = _computePower(iqvx, _nSamplesHalf);

    mdata.phc = power_hc;
    mdata.phx = power_hx;
    mdata.pvc = power_vc;
    mdata.pvx = power_vx;

    sumHc += power_hc;
    sumVx += power_vx;
    nnn++;
    
    // compute snr
    
    mdata.dbmhc = 10.0 * log10(power_hc);

    if (power_hc > _noiseHc) {
      mdata.snrhc = 10.0 * log10((power_hc - _noiseHc) / _noiseHc);
    }
    
    mdata.dbmhx = 10.0 * log10(power_hx);
    if (power_hx > _noiseHx) {
      mdata.snrhx = 10.0 * log10((power_hx - _noiseHx) / _noiseHx);
    }
    
    mdata.dbmvc = 10.0 * log10(power_vc);
    if (power_vc > _noiseVc) {
      mdata.snrvc = 10.0 * log10((power_vc - _noiseVc) / _noiseVc);
    }
    
    mdata.dbmvx = 10.0 * log10(power_vx);
    if (power_vx > _noiseVx) {
      mdata.snrvx = 10.0 * log10((power_vx - _noiseVx) / _noiseVx);
    }
    
    // power, dbz, zdr, ldr
    
    mdata.dbm = (mdata.dbmhc + mdata.dbmvc) / 2.0;

    if (mdata.snrhc != MomentData::missingVal &&
        mdata.snrvc != MomentData::missingVal) {

      mdata.snr = (mdata.snrhc + mdata.snrvc) / 2.0;
      mdata.dbzhc = mdata.snrhc + _dbz0Hc + _rangeCorr[igate];
      mdata.dbzvc = mdata.snrvc + _dbz0Vc + _rangeCorr[igate];
      mdata.dbz = (mdata.dbzhc + mdata.dbzhc) / 2.0;

      if (mdata.snrhx != MomentData::missingVal &&
          mdata.snrvx != MomentData::missingVal) {
        mdata.dbzhx = mdata.snrhx + _dbz0Hx + _rangeCorr[igate];
        mdata.dbzvx = mdata.snrvx + _dbz0Vx + _rangeCorr[igate];
      }

    }

    // vel, width

    double vel_hc, width_hc;
    double vel_vc, width_vc;
    
    _velWidthFromTd(iqhc, _nSamplesHalf, prt, vel_hc, width_hc);
    _velWidthFromTd(iqvc, _nSamplesHalf, prt, vel_vc, width_vc);
    
    mdata.vel = -1.0 * (vel_hc + vel_vc) / 2.0;
    mdata.width = (width_hc + width_vc) / 2.0;
    
    momentData.push_back(mdata);

  } // igate

  // free up
  
  delete[] iqData0;
  delete[] iqData1;
  ufree2((void **) IQ0);
  ufree2((void **) IQ1);
  ufree2((void **) IQHC);
  ufree2((void **) IQHX);
  ufree2((void **) IQVC);
  ufree2((void **) IQVX);

}

///////////////////////////////////////
// initialize ready for moments comps

void SimCpCompute::_initForMoments(const IwrfTsInfo &opsInfo)
  
{

  // compute range correction table
  
  _startRange = opsInfo.get_proc_start_range_km();
  _gateSpacing = opsInfo.get_proc_gate_spacing_km();
  _computeRangeCorrection();

  // set noise value for each channel
  
  _noiseHc = pow(10.0, _params.hc_receiver.bsky_noise_at_ifd / 10.0);
  _noiseHx = pow(10.0, _params.hx_receiver.bsky_noise_at_ifd / 10.0);
  _noiseVc = pow(10.0, _params.vc_receiver.bsky_noise_at_ifd / 10.0);
  _noiseVx = pow(10.0, _params.vx_receiver.bsky_noise_at_ifd / 10.0);

  // set the dbz0 values for each channel
  
  _dbz0Hc = _params.hc_receiver.bsky_noise_at_ifd -
    _params.hc_receiver.gain - _params.hc_receiver.radar_constant;
  _dbz0Hx = _params.hx_receiver.bsky_noise_at_ifd -
    _params.hx_receiver.gain - _params.hx_receiver.radar_constant;
  _dbz0Vc = _params.vc_receiver.bsky_noise_at_ifd -
    _params.vc_receiver.gain - _params.vc_receiver.radar_constant;
  _dbz0Vx = _params.vx_receiver.bsky_noise_at_ifd -
    _params.vx_receiver.gain - _params.vx_receiver.radar_constant;

  // set up moments object
  
  _wavelengthMeters = opsInfo.get_radar_wavelength_cm() / 100.0;
  
}
     
///////////////////////////////////////
// compute range correction table

void SimCpCompute::_computeRangeCorrection()

{

  if (_rangeCorr != NULL) {
    delete[] _rangeCorr;
  }
  _rangeCorr = new double[_nGates];
  for (int i = 0; i < _nGates; i++) {
    double range_km = _startRange + i * _gateSpacing;
    _rangeCorr[i] =
      20.0 * log10(range_km) + range_km * _atmosAtten;
  }

}

/////////////////////////
// compute PPI differences

void SimCpCompute::_computePpiStats()
  
{

  
  
}

/////////////////////////
// compute global stats

void SimCpCompute::_computeGlobalStats()
  
{
  

}

///////////////////////////////
// write out results to files

int SimCpCompute::_writeResults()

{

#ifdef JUNK
  
  // create the directory for the output files, if needed

  if (ta_makedir_recurse(_params.output_dir)) {
    int errNum = errno;
    cerr << "ERROR - SimCpCompute::_writeResults";
    cerr << "  Cannot create output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute output file path

  time_t calTime = (time_t) _calTime;
  DateTime ctime(calTime);
  char outPath[1024];
  sprintf(outPath, "%s/vert_zdr_cal_%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());

  if (_params.debug) {
    cerr << "-->> Writing results file: " << outPath << endl;
  }

  // open file
  
  FILE *out;
  if ((out = fopen(outPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SimCpCompute::_writeFile";
    cerr << "  Cannot create file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  fprintf(out, "========================================\n");
  fprintf(out, "Vertical-pointing ZDR calibration\n");
  fprintf(out, "Time: %s\n", DateTime::strm(calTime).c_str());
  fprintf(out, "n samples   : %d\n", _params.n_samples);
  fprintf(out, "min snr (dB): %g\n", _params.min_snr);
  fprintf(out, "max snr (dB): %g\n", _params.max_snr);
  fprintf(out, "========================================\n");
  fprintf(out, "%10s%10s%10s%10s%10s%10s%10s%10s\n",
          "Range", "npts", "snr", "dBZ", "Hc", "Hx", "Vc", "Vx");
  for (int ii = 0; ii < (int) _intervals.size(); ii++) {
    const RangeStats &interval = *(_intervals[ii]);
    fprintf(out,
            "%10.2f%10d%10.3f%10.3f%10.1f%10.3f%10.3f%10.3f\n",
            interval.getMeanRange(),
            interval.getNValid(),
            interval.getMean().snr,
            interval.getMean().dbz,
            interval.getMean().dbmhc,
            interval.getMean().dbmhx,
            interval.getMean().dbmvc,
            interval.getMean().dbmvx);
  }
  
  // close file

  fclose(out);

#endif

  return 0;

}

///////////////////////////////
// write out results to files

int SimCpCompute::_writeGlobalResults()

{

#ifdef JUNK
  
  // compute output file path

  time_t startTime = (time_t) _startTime;
  DateTime ctime(startTime);
  char outPath[1024];
  sprintf(outPath, "%s/vert_zdr_global_cal_%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());
  
  if (_params.debug) {
    cerr << "-->> Writing global results file: " << outPath << endl;
  }

  // open file
  
  FILE *out;
  if ((out = fopen(outPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SimCpCompute::_writeFile";
    cerr << "  Cannot create file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  fprintf(out, "========================================\n");
  fprintf(out, "Vertical-pointing ZDR calibration - global\n");
  fprintf(out, "Time: %s\n", DateTime::strm(startTime).c_str());
  fprintf(out, "n samples   : %d\n", _params.n_samples);
  fprintf(out, "min snr (dB): %g\n", _params.min_snr);
  fprintf(out, "max snr (dB): %g\n", _params.max_snr);
  fprintf(out, "========================================\n");
  fprintf(out, "%10s%10s%10s%10s%10s%10s%10s%10s\n",
          "Range", "npts", "snr", "dBZ", "Hc", "Hx", "Vc", "Vx");
  for (int ii = 0; ii < (int) _intervals.size(); ii++) {
    const RangeStats &interval = *(_intervals[ii]);
    fprintf(out,
            "%10.2f%10d%10.3f%10.3f%10.1f%10.3f%10.3f%10.3f\n",
            interval.getMeanRange(),
            interval.getGlobalNValid(),
            interval.getGlobalMean().snr,
            interval.getGlobalMean().dbz,
            interval.getGlobalMean().dbmhc,
            interval.getGlobalMean().dbmhx,
            interval.getGlobalMean().dbmvc,
            interval.getGlobalMean().dbmvx);
  }
  
  // close file

  fclose(out);

#endif

  return 0;

}

///////////////////////////////////////////////
// compute total power from IQ

double SimCpCompute::_computePower(const Complex_t *IQ,
                                   int nSamples) const
  
{
  double p = 0.0;
  for (int i = 0; i < nSamples; i++, IQ++) {
    p += ((IQ->re * IQ->re) + (IQ->im * IQ->im));
  }
  return (p / nSamples);
}
  
//////////////////////////////////////////////////////
// compute vel and width in time domain

void SimCpCompute::_velWidthFromTd(const Complex_t *IQ,
                                   int nSamples,
                                   double prtSecs,
                                   double &vel, double &width) const
  
{
  
  // compute a, b, r1
  
  double a = 0.0, b = 0.0;
  
  const Complex_t *iq0 = IQ;
  const Complex_t *iq1 = IQ + 1;
  
  for (int i = 0; i < nSamples - 1; i++, iq0++, iq1++) {
    a += ((iq0->re * iq1->re) + (iq0->im * iq1->im));
    b += ((iq0->re * iq1->im) - (iq1->re * iq0->im));
  }
  double r1_val = sqrt(a * a + b * b) / nSamples;
  
  // compute c, d, r2
  
  double c = 0.0, d = 0.0;
  
  iq0 = IQ;
  const Complex_t *iq2 = IQ + 2;

  for (int i = 0; i < nSamples - 2; i++, iq0++, iq2++) {
    c += ((iq0->re * iq2->re) + (iq0->im * iq2->im));
    d += ((iq0->re * iq2->im) - (iq2->re * iq0->im));
  }
  double r2_val = sqrt(c * c + d * d) / nSamples;
  
  // velocity
  
  double nyquist = _wavelengthMeters / (4.0 * prtSecs);
  double nyqFac = nyquist / M_PI;
  if (a == 0.0 && b == 0.0) {
    vel = 0.0;
  } else {
    vel = nyqFac * atan2(b, a);
  }
  
  // width from R1R2

  double r1r2_fac = (2.0 * nyquist) / (M_PI * sqrt(6.0));
  double ln_r1r2 = log(r1_val/r2_val);
  if (ln_r1r2 > 0) {
    width = r1r2_fac * sqrt(ln_r1r2);
  } else {
    width = r1r2_fac * -1.0 * sqrt(fabs(ln_r1r2));
  }
  
}

/////////////////////////////////////////////////
// set validity of moment data
// mark data as invalid if values out of limits

void SimCpCompute::_markValidMoments(vector<MomentData> &momentData)

{

  for (int ii = 0; ii < (int) momentData.size(); ii++) {
    
    momentData[ii].valid = true;
    
    if (momentData[ii].snr < _params.min_snr ||
        momentData[ii].snr > _params.max_snr) {
      momentData[ii].valid = false;
    }

    if (fabs(momentData[ii].vel) > _params.max_vel) {
      momentData[ii].valid = false;
    }

    if (momentData[ii].width > _params.max_width) {
      momentData[ii].valid = false;
    }

  }

}

