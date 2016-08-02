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
// ZdrCalSimHv.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2011
//
///////////////////////////////////////////////////////////////
//
// ZdrCalSimHv computes the cross-polar ratios between H and V
// returns. This is used for the cross-polar method of ZDR
// calibration, in conjunction with SunScan analysis.
// See SunCal for more info on the sun calibration aspects.
//
////////////////////////////////////////////////////////////////

#include "ZdrCalSimHv.hh"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <radar/RadarComplex.hh>
#include <radar/RadarMoments.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

using namespace std;

// Constructor

ZdrCalSimHv::ZdrCalSimHv(int argc, char **argv)
  
{

  isOK = true;
  _reader = NULL;
  _prevSeqNum = 0;
  _pulseCount = 0;
  _azIndex = -1;
  _nBeams = 0;
  _sumEl = 0.0;
  _nEl = 0.0;
  _startTime = 0;
  _endTime = 0;
  _beamTime = 0;
  _nBeamsSinceSwitch = 0;
  _endOfScanPair = false;
  _prevBeamTime = 0;
  _resultsFile = NULL;
  _pairsFile = NULL;
  _transmitH = false;
  _clearResults();
  
  // set programe name
  
  _progName = "ZdrCalSimHv";

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

 // calibration

  string errStr;
  if (_calib.readFromXmlFile(_params.cal_xml_file_path, errStr)) {
    cerr << "ERROR - ZdrCalSimHv" << endl;
    cerr << "  Cannot decode cal file: "
	 << _params.cal_xml_file_path << endl;
    isOK = false;
    return;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "From cal file: noiseHxDbm: " << _calib.getNoiseDbmHx() << endl;
    cerr << "From cal file: noiseVxDbm: " << _calib.getNoiseDbmVx() << endl;
  }

  _noiseHc = pow(10.0, _calib.getNoiseDbmHc() / 10.0);
  _noiseHx = pow(10.0, _calib.getNoiseDbmHx() / 10.0);
  _noiseVc = pow(10.0, _calib.getNoiseDbmVc() / 10.0);
  _noiseVx = pow(10.0, _calib.getNoiseDbmVx() / 10.0);
  
  _minValidRatio = pow(10.0, _params.min_valid_ratio_db / 10.0);
  _maxValidRatio = pow(10.0, _params.max_valid_ratio_db / 10.0);
  
  _alternating = _params.alternating_hv_mode;
  _switching = _params.switching_receiver;

   // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  return;
  
}

// destructor

ZdrCalSimHv::~ZdrCalSimHv()

{

  // close reader and files
  
  if (_reader) {
    delete _reader;
  }

  if (_pairsFile) {
    fclose(_pairsFile);
  }
  if (_resultsFile) {
    fclose(_resultsFile);
  }

  // clean up memory
  
  _clearPulseQueue();
  _freeGateData();
  
}

//////////////////////////////////////////////////
// Run

int ZdrCalSimHv::Run ()
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

  // initialize from params

  _startRangeAnalysis = _params.start_range;
  _endRangeAnalysis = _params.end_range;
  _gateSpacingAnalysis = _params.gate_spacing;
  _startGateAnalysis =
    (int) (_startRangeAnalysis / _gateSpacingAnalysis + 0.5);
  _endGateAnalysis =
    (int) (_endRangeAnalysis / _gateSpacingAnalysis + 0.5);
  _nGatesAnalysis = _endGateAnalysis - _startGateAnalysis + 1;

  if (_params.stationary_mode) {
    return _runStationaryMode();
  } else {
    return _runScanningMode();
  }

}

//////////////////////////////////////////////////
// Run in scanning mode - PPI

int ZdrCalSimHv::_runScanningMode()
{

  // initialize from params

  _azRes = _params.azimuth_resolution;
  _nAz = (size_t) (360.0 / _azRes + 0.5);
  
  for (size_t ii = 0; ii < _nAz; ii++) {
    double az = ii * _azRes;
    if (_params.start_az <= _params.end_az) {
      // sector does not cross north
      if (az >= _params.start_az && az <= _params.end_az) {
        _processThisAz.push_back(true);
      } else {
        _processThisAz.push_back(false);
      }
    } else {
      // sector does cross north
      if (az >= _params.start_az || az <= _params.end_az) {
        _processThisAz.push_back(true);
      } else {
        _processThisAz.push_back(false);
      }
    }
  }

  // set up power ppi array
  
  PStats pwr;
  vector<PStats> pwrAz;
  for (size_t irange = 0; irange < _nGatesAnalysis; irange++) {
    pwrAz.push_back(pwr);
  }
  for (size_t iaz = 0; iaz < _nAz; iaz++) {
    _ppiPStats.push_back(pwrAz);
  }

  // read in the pulses

  while (true) {
    
    const IwrfTsPulse *pulse = _reader->getNextPulse(true);
    if (pulse == NULL) {
      break;
    }
    
    if (_processPulseScanning(pulse)) {
      delete pulse;
      return -1;
    }
    
  }

  // write results to file

  if (_params.write_results) {
    _writeResults();
  }

  // write MDV if requested

  if (_params.write_mdv_files) {
    _writeMdv();
  }

  return 0;

}

//////////////////////////////////////////////////
// Run in stationary mode

int ZdrCalSimHv::_runStationaryMode()
{

  // set up power stats
  
  PStats pwr;
  for (size_t irange = 0; irange < _nGatesAnalysis; irange++) {
    _stationaryPStats.push_back(pwr);
  }
  
  // read in the pulses

  while (true) {
    
    const IwrfTsPulse *pulse = _reader->getNextPulse(true);
    if (pulse == NULL) {
      break;
    }
    
    if (_processPulseStationary(pulse)) {
      delete pulse;
      return -1;
    }
    
  }

  // write results to file

  if (_params.write_results) {
    _writeResults();
  }

  return 0;

}

////////////////////////////////////////////////////////////////////
// process a pulse in scanning mode
//
// returns -1 if pulse is not to be used

int ZdrCalSimHv::_processPulseScanning(const IwrfTsPulse *pulse)

{

  // compute beam index for this pulse

  double az = pulse->getAz();
  if (az < 0) {
    az += 360.0;
  } else if (az >= 360.0) {
    az -= 360.0;
  }
  size_t azIndex = (size_t) ((az / _azRes) + 0.5);
  if (azIndex >= _nAz) {
    azIndex = 0;
  }

  // if the az index has changed, process the beam because
  // we have moved out of the indexed beam limits
  
  if (azIndex != _azIndex &&
      (int) _pulseQueue.size() > _params.min_samples_per_dwell) {
    
    if (_checkNGates() && _checkDual()) {
      if (_processBeamScanning()) {
        return -1;
      }
    }

    // clear out the queue

    _clearPulseQueue();
    
  }

  // add the pulse to the queue
  
  _azIndex = azIndex;
  _addPulseToQueue(pulse);

  return 0;

}

////////////////////////////////////////////////////////////////////
// process a pulse in stationary mode
//
// returns -1 if pulse is not to be used

int ZdrCalSimHv::_processPulseStationary(const IwrfTsPulse *pulse)

{

  // add the pulse to the queue
  
  _addPulseToQueue(pulse);

  // do we have sufficient samples?

  if ((int) _pulseQueue.size() == _params.n_samples_stationary_mode) {
    if (_checkNGates() && _checkDual()) {
      if (_processBeamStationary()) {
        return -1;
      }
    }
    // clear out the queue
    _clearPulseQueue();
  }

  return 0;

}

/////////////////////////////////////////////////
// add the pulse to the pulse queue
    
void ZdrCalSimHv::_addPulseToQueue(const IwrfTsPulse *pulse)
  
{

  // check for missing pulses, and clear the queue if we miss
  // a pulse
  
  if ((_prevSeqNum != 0) &&
      (pulse->getSeqNum() != (_prevSeqNum + 1))) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "******** Missing seq num: Expected=" << _prevSeqNum + 1
	   << " Rx'd=" <<  pulse->getSeqNum() << " ********" << endl;
    }
    _clearPulseQueue();
  }

  // trim queue as needed to keep memory usage under control
  // push pulse onto back of queue

  if ((int) _pulseQueue.size() > _params.max_pulse_queue_size) {
    const IwrfTsPulse *lastPulse = _pulseQueue.front();
    delete lastPulse;
    _pulseQueue.pop_front();
  }
  _pulseQueue.push_back(pulse);
  
  _prevSeqNum = pulse->getSeqNum();
  _pulseCount++;
  
}

/////////////////////////////////////////////////
// clear the pulse queue
    
void ZdrCalSimHv::_clearPulseQueue()
  
{

  for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
    delete _pulseQueue[ii];
  }
  _pulseQueue.clear();

}

/////////////////////////////////////////////////
// check we have a constant number of gates

bool ZdrCalSimHv::_checkNGates()
  
{

  if (_pulseQueue.size() < 2) {
    return false;
  }

  // set prt and nGates, assuming single PRT for now

  _nGatesData = _pulseQueue[0]->getNGates();
  
  // check we have constant nGates

  for (size_t ii = 1; ii < _pulseQueue.size(); ii++) {
    size_t nGates = _pulseQueue[ii]->getNGates();
    if (nGates != _nGatesData) {
      if (_params.debug) {
        cerr << "nGates differ, this, prev: "
             << nGates << ", " << _nGatesData << endl;
      }
      return false;
    }
  }

  return true;

}

/////////////////////////////////////////////////
// check we have dual channels

bool ZdrCalSimHv::_checkDual()
  
{

  for (size_t ii = 1; ii < _pulseQueue.size(); ii++) {
    if (_pulseQueue[ii]->getIq1() == NULL) {
      return false;
    }
  }

  return true;

}

///////////////////////////////////
// process a beam in scanning mode

int ZdrCalSimHv::_processBeamScanning()

{

  _nBeams++;

  // set noise from time series if requested

  if (_params.get_noise_from_time_series) {
    _getNoiseFromTimeSeries();
  }

  // set number of samples to queue size

  _nSamples = _pulseQueue.size();

  // cerr << "_azIndex, _nSamples: " << _azIndex << ", " << _nSamples << endl;

  _beamTime = _pulseQueue[0]->getTime();
  if (_startTime == 0) {
    _startTime = _beamTime;
  }

  if (_prevBeamTime != 0) {
    int deltaTime = _beamTime - _prevBeamTime;
    if (abs(deltaTime) > _params.max_data_interval_secs) {
      if (_writeRunningSummary()) {
        return -1;
      }
      if (_writeResults()) {
        return -1;
      }
      if (_params.write_mdv_files) {
        _writeMdv();
      }
      _clearResults();
      if (_params.debug) {
        cerr << "====>> Break in data, deltaTime: " << deltaTime << endl;
        cerr << "====>> Setting stats to zero" << endl;
      }
      _setPpiPStatsToZero();
      _startTime = _beamTime;
    }
  }

  _endTime = _beamTime;
  _prevBeamTime = _beamTime;

  // set azimuth and elevation

  _beamAz = _azIndex * _azRes + _azRes / 2.0;

  double sumEl = 0.0;
  double count = 0.0;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    sumEl += _pulseQueue[ii]->getEl();
    count++;
  }
  _beamEl = sumEl / count;

  _sumEl += _beamEl;
  _nEl++;

  // load up the IQ data

  bool transmitH = true;
    
  if (_alternating) {
    // In alternating mode we are simulating H anv V transmit
    // a 360-PPI at a time
    int ppiNum = _nBeams / _nAz;
    if (ppiNum % 2 == 1) {
      transmitH = false;
    }
    _loadGateDataAlt(transmitH);
  } else {
    _loadGateDataSim();
  }

  // determine whether we are in H or V transmit mode

  if (_alternating) {

    if (transmitH != _transmitH) {
      if (_params.debug) {
        cerr << "Beam el, az, transmit mode changing to: "
             << _beamEl << ", " 
             << _beamAz << ", " 
             << (transmitH? "H" : "V") << endl;
      }
      _transmitH = transmitH;
      _nBeamsSinceSwitch = 0;
    } else {
      _nBeamsSinceSwitch++;
    }
  
  } else {

    // in simultaneous mode we comute the ratio of H and V powers to determine
    // whether we are in H or V transmit

    double hvRatio = _computeHvPowerRatio();
    double hvRatioDb = 10.0 * log10(hvRatio);
    if (hvRatioDb > _params.h_minus_v_threshold) {
      if (_params.debug) {
        if (!_transmitH) {
          cerr << "------->> changing to transmit H, el, az, n beams in V: "
               << _beamEl << ", " << _beamAz << ", " 
               << _nBeamsSinceSwitch << endl;
        }
      }
      if (_transmitH) {
        _nBeamsSinceSwitch++;
      } else {
        _transmitH = true;
        _nBeamsSinceSwitch = 0;
      }
    } else if (hvRatioDb < (_params.h_minus_v_threshold * -1.0)) {
      if (_params.debug) {
        if (_transmitH) {
          cerr << "------->> changing to transmit V, el, az, n beams in H: "
               << _beamEl << ", " << _beamAz << ", "
               << _nBeamsSinceSwitch << endl;
        }
      }
      if (!_transmitH) {
        _nBeamsSinceSwitch++;
      } else {
        _transmitH = false;
        _nBeamsSinceSwitch = 0;
      }
    } else {
      return 0; // ambiguous - do not use
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Beam el, az, hvRatioDb, transmit: "
           << _beamEl << ", " 
           << _beamAz << ", " 
           << hvRatioDb << ", "
           << (_transmitH? "H" : "V") << endl;
    }
  
  }

  if (_transmitH) {
    _nBeamsH++;
  } else {
    _nBeamsV++;
  }

  // save radar info

  const IwrfTsInfo &info = _reader->getOpsInfo();
  _radarLat = info.get_radar_latitude_deg();
  _radarLon = info.get_radar_longitude_deg();
  _radarAltKm = info.get_radar_altitude_m() / 1000.0;
  _radarName = info.get_radar_name();
  _siteName = info.get_radar_site_name();

  if (_params.override_radar_location) {
    _radarLat = _params.radar_location.latitude;
    _radarLon = _params.radar_location.longitude;
    _radarAltKm = _params.radar_location.altitude_km;
  }

  if (_radarLat < -90 || _radarLat > 90) {
    _radarLat = 0.0;
  }
  if (_radarLon < -180 || _radarLon > 180) {
    _radarLon = 0.0;
  }

  // loop through the gates

  double startRange = info.get_proc_start_range_km();
  double gateSpacing = info.get_proc_gate_spacing_km();

  for (size_t igate = 0; igate < _nGatesData; igate++) {
    
    double range = startRange + igate * gateSpacing;
    int jgate =
      (int) ((range - _startRangeAnalysis) / _gateSpacingAnalysis + 0.5);
    if (jgate < 0 || jgate > (int) _nGatesAnalysis - 1) {
      continue;
    }
    
    PStats &pstats = _ppiPStats[_azIndex][jgate];
    GateData *gate = _gateData[igate];
    
    // accumulate cross-polar powers if conditions are met
    
    if (_transmitH) {

      // horizontal transmit, vx receive

      double powerVx = RadarComplex::meanPower(gate->iqvx, _nSamplesInUse);
      double nsPowerVx = powerVx - _noiseVx;
      double snrVx = -9999;
      
      if (nsPowerVx > 0) {
        snrVx = 10.0 * log10(nsPowerVx / _noiseVx);
      } else {
        continue;
      }
      
      if (snrVx < _params.min_snr || snrVx > _params.max_snr) {
        continue;
      }
      
      double cpa = RadarMoments::computeCpaAlt(gate->iqvx, _nSamplesInUse);
      if (cpa < _params.min_cpa || cpa > _params.max_cpa) {
        continue;
      }
      
      pstats.powerVx = nsPowerVx;
      pstats.sumPowerVx += nsPowerVx;
      pstats.nSumVx++;

    } else {

      // vertical transmit, hx receive

      double powerHx = RadarComplex::meanPower(gate->iqhx, _nSamplesInUse);
      double nsPowerHx = powerHx - _noiseHx;
      double snrHx = -9999;
      
      if (nsPowerHx > 0) {
        snrHx = 10.0 * log10(nsPowerHx / _noiseHx);
      } else {
        continue;
      }
      
      if (snrHx < _params.min_snr || snrHx > _params.max_snr) {
        continue;
      }
      
      double cpa = RadarMoments::computeCpaAlt(gate->iqhx, _nSamplesInUse);
      if (cpa < _params.min_cpa || cpa > _params.max_cpa) {
        continue;
      }

      pstats.powerHx = nsPowerHx;
      pstats.sumPowerHx += nsPowerHx;
      pstats.nSumHx++;

    }
    
    if (_processThisAz[_azIndex] &&
        pstats.powerHx > 0 && pstats.powerVx > 0) {
      
      double ratio = pstats.powerHx / pstats.powerVx;
      
      if (ratio >= _minValidRatio && ratio <= _maxValidRatio) {
        
        _nPairs++;
        double meanPower = (pstats.powerHx + pstats.powerVx) / 2.0;
        double normPowerHx = pstats.powerHx / meanPower;
        double normPowerVx = pstats.powerVx / meanPower;
        _sumNormPowerHx += normPowerHx;
        _sumNormPowerVx += normPowerVx;
        
        _nPairsRunning++;
        _sumNormPowerHxRunning += normPowerHx;
        _sumNormPowerVxRunning += normPowerVx;
        
        if (_nPairs % 1000 == 0) {
          if (_writeRunningSummary()) {
            return -1;
          }
        }
        
        if (_params.write_data_pairs) {
          double dbHx = 10.0 * log10(pstats.powerHx);
          double dbVx = 10.0 * log10(pstats.powerVx);
          if (_writePairData(_beamEl, _beamAz, range, dbHx, dbVx)) {
            return -1;
          }
        }
        
      } // if (ratio >= _minValidRatio ...

      pstats.clearHx();
      pstats.clearVx();

    } // if (pstats.powerHx > 0 && pstats.powerVx > 0) {

  } // igate

  if (_alternating && _params.also_compute_results_by_gate) {
    _processBeamAltByGate();
  }

  return 0;
  
}

///////////////////////////////////
// process a beam in stationary mode

int ZdrCalSimHv::_processBeamStationary()

{

  _nBeams++;

  // set noise from time series if requested

  if (_params.get_noise_from_time_series) {
    _getNoiseFromTimeSeries();
  }

  // set number of samples to queue size

  _nSamples = _pulseQueue.size();
  _beamTime = _pulseQueue[0]->getTime();

  if (_startTime == 0) {
    _startTime = _beamTime;
  }

  if (_prevBeamTime != 0) {
    int deltaTime = _beamTime - _prevBeamTime;
    if (abs(deltaTime) > _params.max_data_interval_secs) {
      if (_writeRunningSummary()) {
        return -1;
      }
      if (_writeResults()) {
        return -1;
      }
      _clearResults();
      if (_params.debug) {
        cerr << "====>> Break in data, deltaTime: " << deltaTime << endl;
        cerr << "====>> Setting stats to zero" << endl;
      }
      _setStationaryPStatsToZero();
      _startTime = _beamTime;
    }
  }

  _endTime = _beamTime;
  _prevBeamTime = _beamTime;

  // set azimuth and elevation

  double sumAz = 0.0;
  double sumEl = 0.0;
  double count = 0.0;
  for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
    sumAz += _pulseQueue[ii]->getAz();
    sumEl += _pulseQueue[ii]->getEl();
    count++;
  }

  _beamAz = sumAz / count;
  _beamEl = sumEl / count;

  // load up the IQ data

  bool transmitH = true;

  if (_alternating) {
    
    // we are simulating H anv V transmit
    // for successive averaging periods
  
    double secsSinceStart = _beamTime - _startTime;
    int periodNum =
      (int) (secsSinceStart / _params.averaging_period_in_stationary_mode);
    
    if (periodNum % 2 == 1) {
      transmitH = false;
    }

    _loadGateDataAlt(transmitH);

  } else {

    _loadGateDataSim();

  }

  // determine whether we are in H or V transmit mode

  if (_alternating) {

    if (transmitH != _transmitH) {
      if (_params.debug) {
        cerr << "Beam el, az, transmit mode changing to: "
             << _beamEl << ", " 
             << _beamAz << ", " 
             << (transmitH? "H" : "V") << endl;
      }
      _transmitH = transmitH;
      _nBeamsSinceSwitch = 0;
    } else {
      _nBeamsSinceSwitch++;
    }
  
  } else {

    // in simultaneous mode we compute the ratio of H and V powers to determine
    // whether we are in H or V transmit

    double hvRatio = _computeHvPowerRatio();
    double hvRatioDb = 10.0 * log10(hvRatio);
    if (hvRatioDb > _params.h_minus_v_threshold) {
      if (_params.debug) {
        if (!_transmitH) {
          cerr << "------->> changing to transmit H, el, az, n beams in V: "
               << _beamEl << ", " << _beamAz << ", " 
               << _nBeamsSinceSwitch << endl;
        }
      }
      if (_transmitH) {
        _nBeamsSinceSwitch++;
      } else {
        _transmitH = true;
        _nBeamsSinceSwitch = 0;
      }
    } else if (hvRatioDb < (_params.h_minus_v_threshold * -1.0)) {
      if (_params.debug) {
        if (_transmitH) {
          cerr << "------->> changing to transmit V, el, az, n beams in H: "
               << _beamEl << ", " << _beamAz << ", "
               << _nBeamsSinceSwitch << endl;
        }
      }
      if (!_transmitH) {
        _nBeamsSinceSwitch++;
      } else {
        _transmitH = false;
        _nBeamsSinceSwitch = 0;
      }
    } else {
      return 0; // ambiguous - do not use
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Beam el, az, hvRatioDb, transmit: "
           << _beamEl << ", " 
           << _beamAz << ", " 
           << hvRatioDb << ", "
           << (_transmitH? "H" : "V") << endl;
    }
  
  }
  
  if (_nBeamsSinceSwitch == 0 && _nBeamsH > 0 && _nBeamsV > 0) {
    _endOfScanPair = true;
    if (_params.debug) {
      cerr << "Found end of scan pair, nBeamsH, nBeamsV: "
           << _nBeamsH << ", "
           << _nBeamsV << endl;
    }
    _nBeamsH = 0;
    _nBeamsV = 0;
  } else {
    _endOfScanPair = false;
  }

  if (_transmitH) {
    _nBeamsH++;
  } else {
    _nBeamsV++;
  }

  // save radar info

  const IwrfTsInfo &info = _reader->getOpsInfo();
  _radarLat = info.get_radar_latitude_deg();
  _radarLon = info.get_radar_longitude_deg();
  _radarAltKm = info.get_radar_altitude_m() / 1000.0;
  _radarName = info.get_radar_name();
  _siteName = info.get_radar_site_name();

  if (_params.override_radar_location) {
    _radarLat = _params.radar_location.latitude;
    _radarLon = _params.radar_location.longitude;
    _radarAltKm = _params.radar_location.altitude_km;
  }

  if (_radarLat < -90 || _radarLat > 90) {
    _radarLat = 0.0;
  }
  if (_radarLon < -180 || _radarLon > 180) {
    _radarLon = 0.0;
  }

  // loop through the gates

  double startRange = info.get_proc_start_range_km();
  double gateSpacing = info.get_proc_gate_spacing_km();
  
  for (size_t igate = 0; igate < _nGatesData; igate++) {
    
    double range = startRange + igate * gateSpacing;
    int jgate =
      (int) ((range - _startRangeAnalysis) / _gateSpacingAnalysis + 0.5);
    if (jgate < 0 || jgate > (int) _nGatesAnalysis - 1) {
      continue;
    }
    
    PStats &pstats = _stationaryPStats[jgate];
    GateData *gate = _gateData[igate];
    
    // accumulate cross-polar powers if conditions are met
    
    if (_transmitH) {

      // horizontal transmit, vx receive

      double powerVx = RadarComplex::meanPower(gate->iqvx, _nSamplesInUse);
      double nsPowerVx = powerVx - _noiseVx;
      double snrVx = -9999;
      
      if (nsPowerVx > 0) {
        snrVx = 10.0 * log10(nsPowerVx / _noiseVx);
      } else {
        continue;
      }
      
      if (snrVx < _params.min_snr || snrVx > _params.max_snr) {
        continue;
      }
      
      double cpa = RadarMoments::computeCpaAlt(gate->iqvx, _nSamplesInUse);
      if (cpa < _params.min_cpa || cpa > _params.max_cpa) {
        continue;
      }
      
      pstats.powerVx = nsPowerVx;
      pstats.sumPowerVx += nsPowerVx;
      pstats.nSumVx++;

    } else {

      // vertical transmit, hx receive

      double powerHx = RadarComplex::meanPower(gate->iqhx, _nSamplesInUse);
      double nsPowerHx = powerHx - _noiseHx;
      double snrHx = -9999;
      
      if (nsPowerHx > 0) {
        snrHx = 10.0 * log10(nsPowerHx / _noiseHx);
      } else {
        continue;
      }
      
      if (snrHx < _params.min_snr || snrHx > _params.max_snr) {
        continue;
      }
      
      double cpa = RadarMoments::computeCpaAlt(gate->iqhx, _nSamplesInUse);
      if (cpa < _params.min_cpa || cpa > _params.max_cpa) {
        continue;
      }

      pstats.powerHx = nsPowerHx;
      pstats.sumPowerHx += nsPowerHx;
      pstats.nSumHx++;

    }

    if (pstats.powerHx > 0 && pstats.powerVx > 0 && _endOfScanPair) {
      
      double ratio = pstats.powerHx / pstats.powerVx;
      
      if (ratio >= _minValidRatio && ratio <= _maxValidRatio) {
        
        _nPairs++;
        double meanPower = (pstats.powerHx + pstats.powerVx) / 2.0;
        double normPowerHx = pstats.powerHx / meanPower;
        double normPowerVx = pstats.powerVx / meanPower;
        _sumNormPowerHx += normPowerHx;
        _sumNormPowerVx += normPowerVx;
        
        _nPairsRunning++;
        _sumNormPowerHxRunning += normPowerHx;
        _sumNormPowerVxRunning += normPowerVx;
        
        if (_params.write_data_pairs) {
          double dbHx = 10.0 * log10(pstats.powerHx);
          double dbVx = 10.0 * log10(pstats.powerVx);
          if (_writePairData(_beamEl, _beamAz, range, dbHx, dbVx)) {
            return -1;
          }
        }
        
      } // if (ratio >= _minValidRatio ...

      pstats.clearHx();
      pstats.clearVx();

    } // if (pstats.powerHx > 0 && pstats.powerVx > 0) {

  } // igate
  
  if (_endOfScanPair) {
    _writeRunningSummary();
  }
        
  if (_alternating && _params.also_compute_results_by_gate) {
    _processBeamAltByGate();
  }

  return 0;
  
}

/////////////////////////////////////////////////
// get the noise power from time series
    
void ZdrCalSimHv::_getNoiseFromTimeSeries()
  
{

  const IwrfTsInfo &info = _reader->getOpsInfo();

  double noiseDbmHc = info.get_calib_noise_dbm_hc();
  double noiseDbmHx = info.get_calib_noise_dbm_hx();
  double noiseDbmVc = info.get_calib_noise_dbm_vc();
  double noiseDbmVx = info.get_calib_noise_dbm_vx();

  if (noiseDbmHx < -9990) {
    noiseDbmHx = noiseDbmHc;
  }
  if (noiseDbmVx < -9990) {
    noiseDbmVx = noiseDbmHc;
  }

  if (noiseDbmHc > -9990) {
    _noiseHc = pow(10.0, noiseDbmHc / 10.0);
  }
  
  if (noiseDbmHx > -9990) {
    _noiseHx = pow(10.0, noiseDbmHx / 10.0);
  }
  
  if (noiseDbmVc > -9990) {
    _noiseVc = pow(10.0, noiseDbmVc / 10.0);
  }
  
  if (noiseDbmVx > -9990) {
    _noiseVx = pow(10.0, noiseDbmVx / 10.0);
  }
  
}

/////////////////////////////////////////////////////////////////
// Allocate or re-allocate gate data

void ZdrCalSimHv::_allocGateData()

{
  int nNeeded = _nGatesData - (int) _gateData.size();
  if (nNeeded > 0) {
    for (int ii = 0; ii < nNeeded; ii++) {
      GateData *gate = new GateData();
      _gateData.push_back(gate);
    }
  }
  for (size_t ii = 0; ii < _gateData.size(); ii++) {
    _gateData[ii]->allocArrays(_nSamples, false, false, false);
  }
}

/////////////////////////////////////////////////////////////////
// Free gate data

void ZdrCalSimHv::_freeGateData()

{
  for (int ii = 0; ii < (int) _gateData.size(); ii++) {
    delete _gateData[ii];
  }
  _gateData.clear();
}

/////////////////////////////////////////////////////////////////
// Load gate IQ data.
//
// Assumptions:
// 1. Data is in simultaneouse mode.
// 2. Non-switching dual receivers,
//    H is channel 0 and V channel 1.
//
// We load up the hc and vc variables, even though one of these
// represents a cross-polar power in this analysis.

void ZdrCalSimHv::_loadGateDataSim()
  
{

  // alloc gate data

  _allocGateData();

  // set up data pointer arrays

  TaArray<const fl32 *> iqChan0_, iqChan1_;
  const fl32* *iqChan0 = iqChan0_.alloc(_nSamples);
  const fl32* *iqChan1 = iqChan1_.alloc(_nSamples);
  for (size_t ii = 0; ii < _nSamples; ii++) {
    iqChan0[ii] = _pulseQueue[ii]->getIq0();
    iqChan1[ii] = _pulseQueue[ii]->getIq1();
  }
  
  // load up IQ arrays
  
  for (size_t igate = 0, ipos = 0; 
       igate < _nGatesData; igate++, ipos += 2) {

    GateData *gate = _gateData[igate];

    RadarComplex_t *iqhc = gate->iqhc;
    RadarComplex_t *iqhx = gate->iqhx;
    RadarComplex_t *iqvc = gate->iqvc;
    RadarComplex_t *iqvx = gate->iqvx;

    for (size_t isamp = 0; isamp < _nSamples;
         isamp++, iqhc++, iqvc++, iqhx++, iqvx++) {

      // we set the cross and co polar values equal for H and V
      // because at this stage we do not yet know whether we
      // are in H or V transmit

      iqhc->re = iqChan0[isamp][ipos];
      iqhc->im = iqChan0[isamp][ipos + 1];
      iqhx->re = iqhc->re;
      iqhx->im = iqhc->im;
      
      iqvc->re = iqChan1[isamp][ipos];
      iqvc->im = iqChan1[isamp][ipos + 1];
      iqvx->re = iqvc->re;
      iqvx->im = iqvc->im;
      
    } // isamp
    
  } // igate

  // use all samples in sim mode

  _nSamplesInUse = _nSamples;

}

/////////////////////////////////////////////////////////////////
// Load gate IQ data, in alternating mode.
// This is used for testing only. If you do have alternating
// mode data, use AltCpCompute.
//
// Assumption:
//    Data is in alternating mode.
//
// We load up the hc and vc variables, even though one of these
// represents a cross-polar power in this analysis.

void ZdrCalSimHv::_loadGateDataAlt(bool transmitH)
  
{
  
  _allocGateData();

  if (transmitH) {
    _loadGateDataAltTransmitH();
  } else {
    _loadGateDataAltTransmitV();
  }

}

////////////////////////////////////////////////////////////////
// load up date data for alternating mode, in the transmit-H PPI

void ZdrCalSimHv::_loadGateDataAltTransmitH()
  
{
  
  // set up data pointer arrays
  
  TaArray<const fl32 *> iqChan0_, iqChan1_;
  const fl32* *iqChan0 = iqChan0_.alloc(_nSamples);
  const fl32* *iqChan1 = iqChan1_.alloc(_nSamples);
  _nSamplesInUse = 0;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    if (_pulseQueue[ii]->isHoriz()) {
      iqChan0[_nSamplesInUse] = _pulseQueue[ii]->getIq0();
      iqChan1[_nSamplesInUse] = _pulseQueue[ii]->getIq1();
      _nSamplesInUse++;
    }
  }

  // load up IQ arrays
  
  for (size_t igate = 0, ipos = 0; 
       igate < _nGatesData; igate++, ipos += 2) {
    
    GateData *gate = _gateData[igate];
    
    RadarComplex_t *iqhc = gate->iqhc;
    RadarComplex_t *iqhx = gate->iqhx;
    RadarComplex_t *iqvc = gate->iqvc;
    RadarComplex_t *iqvx = gate->iqvx;

    for (size_t isamp = 0; isamp < _nSamplesInUse; 
         isamp++, iqhc++, iqhx++, iqvc++, iqvx++) {
      
      if (_switching) {
        iqhc->re = iqChan0[isamp][ipos];
        iqhc->im = iqChan0[isamp][ipos + 1];
        iqvx->re = iqChan1[isamp][ipos];
        iqvx->im = iqChan1[isamp][ipos + 1];
      } else {
        iqhc->re = iqChan0[isamp][ipos];
        iqhc->im = iqChan0[isamp][ipos + 1];
        iqvx->re = iqChan1[isamp][ipos];
        iqvx->im = iqChan1[isamp][ipos + 1];
      }
      
    } // isamp
    
  } // igate

}

////////////////////////////////////////////////////////////////
// load up date data for alternating mode, in the transmit-V PPI

void ZdrCalSimHv::_loadGateDataAltTransmitV()
  
{
  
  // set up data pointer arrays
  
  TaArray<const fl32 *> iqChan0_, iqChan1_;
  const fl32* *iqChan0 = iqChan0_.alloc(_nSamples);
  const fl32* *iqChan1 = iqChan1_.alloc(_nSamples);
  _nSamplesInUse = 0;
  for (size_t ii = 0; ii < _nSamples; ii++) {
    if (!_pulseQueue[ii]->isHoriz()) {
      iqChan0[_nSamplesInUse] = _pulseQueue[ii]->getIq0();
      iqChan1[_nSamplesInUse] = _pulseQueue[ii]->getIq1();
      _nSamplesInUse++;
    }
  }

  // load up IQ arrays
  
  for (size_t igate = 0, ipos = 0; 
       igate < _nGatesData; igate++, ipos += 2) {
    
    GateData *gate = _gateData[igate];
    
    RadarComplex_t *iqhc = gate->iqhc;
    RadarComplex_t *iqhx = gate->iqhx;
    RadarComplex_t *iqvc = gate->iqvc;
    RadarComplex_t *iqvx = gate->iqvx;

    for (size_t isamp = 0; isamp < _nSamplesInUse; 
         isamp++, iqhc++, iqhx++, iqvc++, iqvx++) {
      
      if (_switching) {
        iqvc->re = iqChan0[isamp][ipos];
        iqvc->im = iqChan0[isamp][ipos + 1];
        iqhx->re = iqChan1[isamp][ipos];
        iqhx->im = iqChan1[isamp][ipos + 1];
      } else {
        iqvc->re = iqChan1[isamp][ipos];
        iqvc->im = iqChan1[isamp][ipos + 1];
        iqhx->re = iqChan0[isamp][ipos];
        iqhx->im = iqChan0[isamp][ipos + 1];
      }
      
    } // isamp
    
  } // igate

}

////////////////////////////////////////////////////////
// compute the ratio of H/V power
// to determine if we have H or V transmit

double ZdrCalSimHv::_computeHvPowerRatio()

{

  // sum up powers in H and V
  
  double sumPowerH = 0.0;
  double sumPowerV = 0.0;
  int count = 0;
  
  for (size_t igate = 0; igate < _nGatesData; igate++) {
    
    GateData *gate = _gateData[igate];
    
    double powerH = RadarComplex::meanPower(gate->iqhc, _nSamplesInUse);
    double powerV = RadarComplex::meanPower(gate->iqvc, _nSamplesInUse);

    double cpa;
    if (powerH > powerV) {
      cpa = RadarMoments::computeCpaAlt(gate->iqhc, _nSamplesInUse);
    } else {
      cpa = RadarMoments::computeCpaAlt(gate->iqvc, _nSamplesInUse);
    }
    if (cpa < _params.min_cpa || cpa > _params.max_cpa) {
      continue;
    }

    sumPowerH += powerH;
    sumPowerV += powerV;
    count++;

  } // igate

  if (count > 10) {
    double hvPowerRatio = sumPowerH / sumPowerV;
    return hvPowerRatio;
  } else {
    // don't know
    return 1.0;
  }
  
}

////////////////////////////
// clear the ppi power stats

void ZdrCalSimHv::_setPpiPStatsToZero()
  
{
  for (size_t ii = 0; ii < _ppiPStats.size(); ii++) {
    vector<PStats> &powers = _ppiPStats[ii];
    for (size_t jj = 0; jj < powers.size(); jj++) {
      powers[jj].clear();
    }
  }
}

///////////////////////////////////
// clear the stationary power stats

void ZdrCalSimHv::_setStationaryPStatsToZero()
  
{
  vector<PStats> &powers = _stationaryPStats;
  for (size_t jj = 0; jj < powers.size(); jj++) {
    powers[jj].clear();
  }
}

/////////////////////////
// clear the stats

void ZdrCalSimHv::_clearResults()
  
{

  _nPairs = 0;
  _sumNormPowerHx = 0;
  _sumNormPowerVx = 0;
  _meanNormPowerHxDb = -9999.0;
  _meanNormPowerVxDb = -9999.0;
  _hxVxRatioMean = -9999.0;
  _hxVxRatioMeanDb = -9999.0;

  _clearResultsRunning();

  _nPairsByGate = 0;
  _sumNormPowerHxByGate = 0;
  _sumNormPowerVxByGate = 0;

  _nPairsByGateTotal = 0;
  _sumNormPowerHxByGateTotal = 0;
  _sumNormPowerVxByGateTotal = 0;

}
  
void ZdrCalSimHv::_clearResultsRunning()
  
{

  _nPairsRunning = 0;
  _sumNormPowerHxRunning = 0;
  _sumNormPowerVxRunning = 0;
  _meanNormPowerHxDbRunning = -9999.0;
  _meanNormPowerVxDbRunning = -9999.0;
  _hxVxRatioMeanRunning = -9999.0;
  _hxVxRatioMeanDbRunning = -9999.0;

}
  
/////////////////////////
// compute the CP ratio

void ZdrCalSimHv::_computeResults()
  
{

  _meanNormPowerHxDbRunning =
    10.0  * log10(_sumNormPowerHxRunning / _nPairsRunning);
  _meanNormPowerVxDbRunning =
    10.0  * log10(_sumNormPowerVxRunning / _nPairsRunning);
  _hxVxRatioMeanRunning = _sumNormPowerHxRunning / _sumNormPowerVxRunning;
  _hxVxRatioMeanDbRunning = 10.0 * log10(_hxVxRatioMeanRunning);

  _meanNormPowerHxDb = 10.0  * log10(_sumNormPowerHx / _nPairs);
  _meanNormPowerVxDb = 10.0  * log10(_sumNormPowerVx / _nPairs);
  _hxVxRatioMean = _sumNormPowerHx / _sumNormPowerVx;
  _hxVxRatioMeanDb = 10.0 * log10(_hxVxRatioMean);

}

///////////////////////////////
// write out results to files

int ZdrCalSimHv::_writeResults()

{

  if (_resultsFile == NULL) {
    if (_openResultsFile()) {
      return -1;
    }
  }
  
  _computeResults();

  _writeResults(_resultsFile);

  if (_params.debug) {
    _writeResults(stderr);
  }

  return 0;

}

void ZdrCalSimHv::_writeResults(FILE *out)

{
  
  fprintf(out, "========================================\n");
  fprintf(out, "Sim-Xmit Cross-polar ZDR calibration\n");
  fprintf(out, "  start time    : %s\n", DateTime::strm(_startTime).c_str());
  fprintf(out, "  end time      : %s\n", DateTime::strm(_endTime).c_str());
  fprintf(out, "  min cpa       : %g\n", _params.min_cpa);
  fprintf(out, "  n data pairs  : %d\n", _nPairs);
  fprintf(out, "  Hx/Vx mean    : %g\n", _hxVxRatioMean);
  fprintf(out, "  Hx/Vx mean dB : %g\n", _hxVxRatioMeanDb);
  fprintf(out, "========================================\n");

}

/////////////////////////////////////////////////
// write running summary of results

int ZdrCalSimHv::_writeRunningSummary()
  
{
  
  _computeResults();

  char textRunning[1024];
  sprintf(textRunning,
          "Results running: %6d %10.3f %10.3f %10.3f %10.3f %10.3f",
          _nPairsRunning,
          _sumNormPowerHxRunning, _sumNormPowerVxRunning,
          _meanNormPowerHxDbRunning, _meanNormPowerVxDbRunning,
          _hxVxRatioMeanDbRunning);

  char textTotal[1024];
  sprintf(textTotal,
          "Results total  : %6d %10.3f %10.3f %10.3f %10.3f %10.3f",
          _nPairs,
          _sumNormPowerHx, _sumNormPowerVx,
          _meanNormPowerHxDb, _meanNormPowerVxDb,
          _hxVxRatioMeanDb);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "-->> %s\n", textRunning);
  }
  if (_params.debug) {
    fprintf(stderr, "-->> %s\n", textTotal);
  }
  
  if (_params.write_results) {
    
    if (_resultsFile == NULL) {
      if (_openResultsFile()) {
        return -1;
      }
    }
    
    fprintf(_resultsFile, "%s\n", textTotal);
    fflush(_resultsFile);

  }

  _clearResultsRunning();

  return 0;

}

/////////////////////////////////////////////////
// write pair data to file

int ZdrCalSimHv::_writePairData(double beamEl,
                                double beamAz,
                                double range,
                                double powerHxDb,
                                double powerVxDb)
  
{
  
  if (_pairsFile == NULL) {
    if (_openPairsFile()) {
      return -1;
    }
  }

  fprintf(_pairsFile,
          "%10.3f %10.3f %10.3f %10.3f %10.3f\n",
          beamEl, beamAz, range,
          powerHxDb, powerVxDb);

  fflush(_pairsFile);

  return 0;

}

///////////////////////////////
// open results file

int ZdrCalSimHv::_openResultsFile()

{
  
  if (_resultsFile != NULL) {
    return 0;
  }

  // create the directory for the output files, if needed
  
  if (ta_makedir_recurse(_params.output_dir)) {
    int errNum = errno;
    cerr << "ERROR - ZdrCalSimHv::_openResultsFile";
    cerr << "  Cannot create output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute results file path

  time_t startTime = (time_t) _startTime;
  DateTime ctime(startTime);
  char outPath[1024];
  sprintf(outPath, "%s/%s_cp_results_%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          _params.radar_name,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());
  
  if (_params.debug) {
    cerr << "-->> Opening results file: " << outPath << endl;
  }

  // open file
  
  if ((_resultsFile = fopen(outPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ZdrCalSimHv::_openResultsFile";
    cerr << "  Cannot create file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////
// open pairs file

int ZdrCalSimHv::_openPairsFile()

{
  
  if (_pairsFile != NULL) {
    return 0;
  }

  // create the directory for the output files, if needed
  
  if (ta_makedir_recurse(_params.output_dir)) {
    int errNum = errno;
    cerr << "ERROR - ZdrCalSimHv::_openPairsFile";
    cerr << "  Cannot create output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute pairs file path

  time_t startTime = (time_t) _startTime;
  DateTime ctime(startTime);
  char outPath[1024];
  sprintf(outPath, "%s/%s_cp_pairs_%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.output_dir,
          _params.radar_name,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());
  
  if (_params.debug) {
    cerr << "-->> Opening pairs file: " << outPath << endl;
  }

  // open file
  
  if ((_pairsFile = fopen(outPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ZdrCalSimHv::_openPairsFile";
    cerr << "  Cannot create file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////
// on a gate-by-gate basis, process a beam of data
// See also AltCpCompute app.

int ZdrCalSimHv::_processBeamAltByGate()

{

  // load up the IQ data for each gate

  _loadGateDataAltByGate();

  // loop through gates to compute moments
  
  const IwrfTsInfo &opsInfo = _reader->getOpsInfo();
  double startRangeKm = opsInfo.get_proc_start_range_km();
  double gateSpacingKm = opsInfo.get_proc_gate_spacing_km();
  double range = startRangeKm + gateSpacingKm / 2.0;
  
  for (size_t igate = 0; igate < _nGatesData;
       igate++, range += gateSpacingKm) {
    
    // check range is within limits

    if (range < _params.start_range ||
        range > _params.end_range) {
      continue;
    }
    
    GateData *gate = _gateDataByGate[igate];
    
    // power

    double power_hx = RadarComplex::meanPower(gate->iqhx, _nVByGate);
    double power_vx = RadarComplex::meanPower(gate->iqvx, _nHByGate);

    // noise subtracted power and SNR
    
    double ns_power_hx = power_hx - _noiseHx;
    double ns_power_vx = power_vx - _noiseVx;

    // check mean snr in cross-polar channels
    
    if (ns_power_hx <= 0 || ns_power_vx <= 0) {
      continue;
    }

    double snrhx = 10.0 * log10(ns_power_hx / _noiseHx);
    double snrvx = 10.0 * log10(ns_power_vx / _noiseVx);

    if (snrhx < _params.min_snr || snrhx > _params.max_snr) {
      continue;
    }
    if (snrvx < _params.min_snr || snrvx > _params.max_snr) {
      continue;
    }
    
    double snrx = (snrhx + snrvx) / 2.0;

    // check cpa
    
    double cpa =  RadarMoments::computeCpaAlt(gate->iqhc, _nSamples / 2);
    if (cpa < _params.min_cpa || cpa > _params.max_cpa) {
      continue;
    }
  
    // add to stats

    _addToResultsByGate(range, snrx, ns_power_hx, ns_power_vx);

  } // igate

  return 0;

}

/////////////////////////////////////////////////////////////////
// Load gate IQ data.
//
// Assumptions:
// 1. Data is in simultaneouse mode.
// 2. Non-switching dual receivers,
//    H is channel 0 and V channel 1.

void ZdrCalSimHv::_loadGateDataAltByGate()
  
{

  // alloc gate data

  _allocGateDataByGate();

  // set up data pointer arrays

  TaArray<const fl32 *> iqChan0_, iqChan1_;
  const fl32* *iqChan0 = iqChan0_.alloc(_nSamples);
  const fl32* *iqChan1 = iqChan1_.alloc(_nSamples);
  TaArray<bool> isHoriz_;
  bool *isHoriz = isHoriz_.alloc(_nSamples);
  _nHByGate = _nVByGate = 0;
  for (size_t isamp = 0; isamp < _nSamples; isamp++) {
    iqChan0[isamp] = _pulseQueue[isamp]->getIq0();
    iqChan1[isamp] = _pulseQueue[isamp]->getIq1();
    isHoriz[isamp] = _pulseQueue[isamp]->isHoriz();
    if (isHoriz[isamp]) {
      _nHByGate++;
    } else {
      _nVByGate++;
    }
  }
  
  // load up IQ arrays, gate by gate

  for (size_t igate = 0, ipos = 0; 
       igate < _nGatesData; igate++, ipos += 2) {
    
    GateData *gate = _gateDataByGate[igate];
    RadarComplex_t *iqhc = gate->iqhc;
    RadarComplex_t *iqvc = gate->iqvc;
    RadarComplex_t *iqhx = gate->iqhx;
    RadarComplex_t *iqvx = gate->iqvx;
    
    // samples start on horiz pulse

    for (size_t isamp = 0; isamp < _nSamples; isamp++) {
      
      if (_switching) {

        if (isHoriz[isamp]) {

          // H co-polar, V cross-polar
          
          iqhc->re = iqChan0[isamp][ipos];
          iqhc->im = iqChan0[isamp][ipos + 1];
          iqvx->re = iqChan1[isamp][ipos];
          iqvx->im = iqChan1[isamp][ipos + 1];
          iqhc++;
          iqvx++;

        } else {

          // V co-polar, H cross-polar
          
          iqvc->re = iqChan0[isamp][ipos];
          iqvc->im = iqChan0[isamp][ipos + 1];
          iqhx->re = iqChan1[isamp][ipos];
          iqhx->im = iqChan1[isamp][ipos + 1];
          iqvc++;
          iqhx++;
        
        }
        
      } else {

        // H from chan 0, V from chan 1

        if (isHoriz[isamp]) {

          iqhc->re = iqChan0[isamp][ipos];
          iqhc->im = iqChan0[isamp][ipos + 1];
          iqvx->re = iqChan1[isamp][ipos];
          iqvx->im = iqChan1[isamp][ipos + 1];
          iqhc++;
          iqvx++;

        } else {
        
          iqhx->re = iqChan0[isamp][ipos];
          iqhx->im = iqChan0[isamp][ipos + 1];
          iqvc->re = iqChan1[isamp][ipos];
          iqvc->im = iqChan1[isamp][ipos + 1];
          iqvc++;
          iqhx++;

        }

      }
      
    } // isamp

  } // igate

}

/////////////////////////////////////////////////////////////////
// Allocate or re-allocate gate data

void ZdrCalSimHv::_allocGateDataByGate()

{
  int nNeeded = _nGatesData - (int) _gateDataByGate.size();
  if (nNeeded > 0) {
    for (int ii = 0; ii < nNeeded; ii++) {
      GateData *gate = new GateData();
      _gateDataByGate.push_back(gate);
    }
  }
  for (size_t ii = 0; ii < _gateDataByGate.size(); ii++) {
    _gateDataByGate[ii]->allocArrays(_nSamples, false, false, false);
  }
}

////////////////////////////////////////////////
// add to running stats

void ZdrCalSimHv::_addToResultsByGate(double range,
                                      double snrX,
                                      double powerHx,
                                      double powerVx)
  
{
  
  double ratio = powerVx / powerHx;
  
  if (ratio >= _minValidRatio && ratio <= _maxValidRatio) {

    // accumulate sums of normalized power
    
    double meanPower = (powerHx + powerVx) / 2.0;
    
    double normPowerHx = powerHx / meanPower;
    double normPowerVx = powerVx / meanPower;
    
    _sumNormPowerHxByGate += normPowerHx;
    _sumNormPowerVxByGate += normPowerVx;

    _sumNormPowerHxByGateTotal += normPowerHx;
    _sumNormPowerVxByGateTotal += normPowerVx;

    _nPairsByGate++;
    _nPairsByGateTotal++;

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "-->> byGate: n, nSamples, range, meanPower, "
           << "sumH, sumV, ratio, running: "
           << _nPairsByGate << ", "
           << _nSamples << ", "
           << range << ", "
           << meanPower << ", "
           << _sumNormPowerHxByGate << ", "
           << _sumNormPowerVxByGate << ", "
           << 10 * log10(powerHx / powerVx) << ", "
           << 10 * log10(_sumNormPowerHxByGate / _sumNormPowerVxByGate)
           << endl;
    }
    
    if (_nPairsByGateTotal % 10000 == 0) {
      _writeRunningSummaryByGate();
    }
      
  }

}
  
/////////////////////////////////////////////////
// write running summary of results

void ZdrCalSimHv::_writeRunningSummaryByGate()
  
{
  
  double meanPowerHx = _sumNormPowerHxByGate / _nPairsByGate;
  double meanPowerVx = _sumNormPowerVxByGate / _nPairsByGate;
  double meanPowerHxTotal = _sumNormPowerHxByGateTotal / _nPairsByGateTotal;
  double meanPowerVxTotal = _sumNormPowerVxByGateTotal / _nPairsByGateTotal;

  double meanPowerHxDbm = 10.0 * log10(meanPowerHx);
  double meanPowerVxDbm = 10.0 * log10(meanPowerVx);
  double meanRatio = _sumNormPowerHxByGate / _sumNormPowerVxByGate;
  double meanRatioDb = 10.0 * log10(meanRatio);

  double meanPowerHxDbmTotal = 10.0 * log10(meanPowerHxTotal);
  double meanPowerVxDbmTotal = 10.0 * log10(meanPowerVxTotal);
  double meanRatioTotal =
    _sumNormPowerHxByGateTotal / _sumNormPowerVxByGateTotal;
  double meanRatioDbTotal = 10.0 * log10(meanRatioTotal);

  if (_params.debug >= Params::DEBUG_NORM) {

    fprintf(stderr,
            "By gate, running: %6d %10.3f %10.3f %10.3f %10.3f %10.3f\n",
            _nPairsByGate,
            meanPowerHx, meanPowerVx,
            meanPowerHxDbm, meanPowerVxDbm, meanRatioDb);
    
    fprintf(stderr,
            "By gate, total  : %6d %10.3f %10.3f %10.3f %10.3f %10.3f\n",
            _nPairsByGateTotal,
            meanPowerHxTotal, meanPowerVxTotal,
            meanPowerHxDbmTotal, meanPowerVxDbmTotal, meanRatioDbTotal);

  }
  
  _nPairsByGate = 0;
  _sumNormPowerHxByGate = 0;
  _sumNormPowerVxByGate = 0;

}

/////////////////////////////////////////////////
// write MDV output for debugging

void ZdrCalSimHv::_writeMdv()
  
{

  // create data arrays

  size_t nPts = _nAz * _nGatesAnalysis;
  TaArray<fl32> powerHxDbm_, powerVxDbm_, ratioHxVxDb_;
  fl32 *powerHxDbm = powerHxDbm_.alloc(nPts);
  fl32 *powerVxDbm = powerVxDbm_.alloc(nPts);
  fl32 *ratioHxVxDb = ratioHxVxDb_.alloc(nPts);
  fl32 missingVal = -9999;

  // fill out data arrays

  int ii = 0;
  for (size_t iaz = 0; iaz < _nAz; iaz++) {
    for (size_t igate = 0; igate < _nGatesAnalysis; igate++, ii++) {
      const PStats &pstats = _ppiPStats[iaz][igate];
      double meanPowerHxDbm = missingVal;
      double meanPowerVxDbm = missingVal;
      double hxVxDb = missingVal;
      double meanPowerHx = missingVal;
      double meanPowerVx = missingVal;
      if (pstats.nSumHx > 0 && pstats.nSumVx > 0) {
        meanPowerHx = pstats.sumPowerHx / pstats.nSumHx;
        meanPowerHxDbm = 10.0 * log10(meanPowerHx);
        meanPowerVx = pstats.sumPowerVx / pstats.nSumVx;
        meanPowerVxDbm = 10.0 * log10(meanPowerVx);
        double hxVx = meanPowerHx / meanPowerVx;
        hxVxDb = 10.0 * log10(hxVx);
      }
      powerHxDbm[ii] = meanPowerHxDbm - _calib.getReceiverGainDbHc();
      powerVxDbm[ii] = meanPowerVxDbm - _calib.getReceiverGainDbVc();
      ratioHxVxDb[ii] = hxVxDb;
    } // igate
  } // iaz

  // create Mdvx object

  DsMdvx mdvx;

  // set master header

  mdvx.clearMasterHeader();
  Mdvx::master_header_t mhdr = mdvx.getMasterHeader();
  
  mhdr.time_begin = _startTime;
  mhdr.time_end = _endTime;
  mhdr.time_centroid = _endTime;
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 2;
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  mhdr.vlevel_included = 1;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.n_fields = 3;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = 0;
  mhdr.sensor_lon = _radarLon;
  mhdr.sensor_lat = _radarLat;
  mhdr.sensor_alt = _radarAltKm;
  
  mdvx.setMasterHeader(mhdr);
  mdvx.setDataSetSource(_radarName.c_str());
  mdvx.setDataSetName(_siteName.c_str());
  mdvx.setDataSetInfo("Analysis by ZdrCalSimHv");

  // add the fields

  _addMdvField(mdvx,
               "DBMHX",
               "MeanCrossPolarPowerH",
               "dBm",
               powerHxDbm);
  
  _addMdvField(mdvx,
               "DBMVX",
               "MeanCrossPolarPowerV",
               "dBm",
               powerVxDbm);
  
  _addMdvField(mdvx,
               "RatioHxVx",
               "RatioOfHxToVx",
               "dB",
               ratioHxVxDb);

  // write

  if (mdvx.writeToDir(_params.mdv_output_dir)) {
    cerr << "ERROR -  ZdrCalSimHv::_writeMdv()" << endl;
    cerr << mdvx.getErrStr() << endl;
  } else {
    if (_params.debug) {
      cerr << "Wrote debug MDV file: " << mdvx.getPathInUse() << endl;
    }
  }

  
  
}
  
//////////////////////////////////////////
// add output field to output MDVX object

void ZdrCalSimHv::_addMdvField(DsMdvx &outMdvx,
                               const string &field_name,
                               const string &long_field_name,
                               const string &units,
                               const fl32 *data)
  
{
  
  if (_params.debug) {
    cerr << "  Adding field: " << field_name << endl;
  }

  // set field header, set members which change
  
  Mdvx::field_header_t fhdr;
  memset(&fhdr, 0, sizeof(fhdr));
  
  fhdr.nx = _nGatesAnalysis;
  fhdr.ny = _nAz;
  fhdr.nz = 1;
  size_t nPts = fhdr.nx * fhdr.ny * fhdr.nz;
  fhdr.volume_size = nPts * sizeof(fl32);

  fhdr.proj_type = Mdvx::PROJ_POLAR_RADAR;
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  fhdr.dz_constant = 1;
  fhdr.data_dimension = 2;

  fhdr.proj_origin_lat = _radarLat;
  fhdr.proj_origin_lon = _radarLon;

  fhdr.grid_dx = _gateSpacingAnalysis;
  fhdr.grid_dy = _azRes;
  fhdr.grid_dz = 1.0;
  
  fhdr.grid_minx = _startRangeAnalysis;
  fhdr.grid_miny = 0.0;
  double elevDeg = _sumEl / _nEl;
  fhdr.grid_minz = elevDeg;
  
  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fl32 missingVal = -9999;
  fhdr.bad_data_value = missingVal;
  fhdr.missing_data_value = missingVal;
  
  STRncopy(fhdr.field_name_long, long_field_name.c_str(), MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.units, units.c_str(), MDV_UNITS_LEN);
  STRncopy(fhdr.transform, "", MDV_TRANSFORM_LEN);
  
  // vlevel header
  
  Mdvx::vlevel_header_t vhdr;
  memset(&vhdr, 0, sizeof(vhdr));
  vhdr.level[0] = elevDeg;
  vhdr.type[0] = Mdvx::VERT_TYPE_ELEV;

  // create field
  
  MdvxField *fld = new MdvxField(fhdr, vhdr, data);

  // convert to output encoding type, and compress
  
  fld->convertDynamic(Mdvx::ENCODING_INT16,
                      Mdvx::COMPRESSION_GZIP);
  
  // add to object
  
  outMdvx.addField(fld);

}

  
