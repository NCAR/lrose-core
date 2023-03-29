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
// IwrfTs2NetCDF.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2011
//
///////////////////////////////////////////////////////////////
//
// Ts2NetCDF reads time-series data and saves it out as netCDF
//
////////////////////////////////////////////////////////////////

#include "Ts2NetCDF.hh"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctime>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <radar/RadarComplex.hh>

using namespace std;

// Constructor

Ts2NetCDF::Ts2NetCDF(int argc, char **argv)
  
{

  isOK = true;
  _pulseReader = NULL;
  _nPulsesRead = 0;
  _nGatesPrev = -1;
  _nGatesPrev2 = -1;
  _nChannelsPrev = -1;
  MEM_zero(_radarPrev);

  MEM_zero(_scanPrev);
  _scanPrev.scan_mode = -1;
  _scanPrev.volume_num = -1;
  _scanPrev.sweep_num = -1;

  MEM_zero(_procPrev);
  _procPrev.xmit_rcv_mode = -1;
  _procPrev.xmit_phase_mode = -1;
  _procPrev.prf_mode = -1;

  MEM_zero(_calibPrev);

  // set programe name
  
  _progName = "Ts2NetCDF";

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

  // check params
  
  if (_params.specify_n_gates_save && _params.pad_n_gates_to_max) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with parameters or command line" << endl;
    cerr << "  You have specified both of:" << endl;
    cerr << "    specify_n_gates_save" << endl;
    cerr << "  and" << endl;
    cerr << "    pad_n_gates_to_max" << endl;
    cerr << "  These are not mutually compatible." << endl;
    cerr << "  Choose only one of these options." << endl;
    isOK = false;
    return;
  }
  
  // create the pulse reader
  
  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_NORM) {
    iwrfDebug = IWRF_DEBUG_NORM;
  } 
    
  if (_params.input_mode == Params::TS_FMQ_INPUT) {
    _pulseReader = new IwrfTsReaderFmq(_params.input_fmq_name,
				       iwrfDebug,
				       !_params.seek_to_end_of_input);
  } else if (_params.input_mode == Params::TS_TCP_INPUT) {
    _pulseReader = new IwrfTsReaderTcp(_params.tcp_server_host,
                                       _params.tcp_server_port,
				       iwrfDebug);
  } else {
    _pulseReader = new IwrfTsReaderFile(_args.inputFileList, iwrfDebug);
  }
  if (_params.rvp8_legacy_unpacking) {
    _pulseReader->setSigmetLegacyUnpacking(true);
  }

  _reset();

  // compute sector size etc for files
  
  _nPulsesFile = 0;

  int numSectors = (int) (359.9 / _params.max_sector_size) + 1;
  if (numSectors < 2) {
    numSectors = 2;
  }
  _sectorWidth = 360.0 / numSectors;
  _currentSector = -1;

  if (_params.debug && _params.save_scans_in_sectors) {
    cerr << "TsSmartSave: numSectors, sectorWidth: "
         << numSectors << ", "
         << _sectorWidth << endl;
  }

  // init process mapper registration
  
  if (_params.reg_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
}

// destructor

Ts2NetCDF::~Ts2NetCDF()

{

  if (_pulseReader) {
    delete _pulseReader;
  }

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Ts2NetCDF::Run ()
{

  PMU_auto_register("Run");
  
  // override cal if appropriate

  if (_params.override_radar_cal) {
    string errStr;
    if (_calOverride.readFromXmlFile
        (_params.radar_cal_xml_file_path, errStr)) {
      cerr << "ERROR - Ts2NetCDF::Run" << endl;
      cerr << "  Overriding radar cal" << endl;
      cerr << "  Cannot read in cal file: " 
           << _params.radar_cal_xml_file_path << endl;
      cerr << errStr << endl;
      return -1;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Running TsSmartSave - extra verbose debug mode" << endl;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running TsSmartSave - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running TsSmartSave - debug mode" << endl;
  }

  // set up ngates ray vector if needed
  // also compute maxNGates

  if (_params.pad_n_gates_to_max) {
    if (_params.debug) {
      cerr << "==>> checking for n gates max" << endl;
    }
    _setNGatesMax();
    if (_params.debug) {
      cerr << "DEBUG - padding ngates out to max" << endl;
      cerr << "  nGatesMax: " << _nGatesMax << endl;
    }
  }

  // check for alternating mode by inspecting the HV flag

  if (_params.debug) {
    cerr << "==>> checking for alternating mode" << endl;
  }
  _checkAltMode();
  if (_params.debug) {
    if (_alternatingMode) {
      cerr << "==>> alternating mode" << endl;
    } else {
      cerr << "==>> NOT alternating mode" << endl;
    }
  }

  if (_params.debug) {
    cerr << "==>> reading in data to convert" << endl;
  }

  while (true) {
    
    // read next pulse
    
    bool readyToWrite = false;
    IwrfTsPulse *pulse = _pulseReader->getNextPulse(true);
    if (pulse == NULL) {
      readyToWrite = true;
    }

    if (_pulseReader->endOfFile()) {
      readyToWrite = true;
      if (_params.debug) {
        cerr << "INFO - end of file found" << endl;
      }
    }

    if (_params.filter_antenna_transitions &&
        pulse != NULL &&
        pulse->get_antenna_transition()) {
      delete pulse;
      continue;
    }

    // check if we need a new file? If so open file
    
    if (pulse) {
      if (_checkReadyToWrite(*pulse)) {
        readyToWrite = true;
      }
    } else {
      if (_pulseReader->endOfFile()) {
        readyToWrite = true;
      }
    }

    if (_elArrayHc.size() < 1) {
      readyToWrite = false;
    }
    
    if (readyToWrite) {
      if (_writeFile()) {
        return -1;
      }
    }
    
    // handle the pulse
    
    if (pulse && _handlePulse(*pulse)) {
      return -1;
    }
    
    // delete pulse to clean up

    if (pulse) {
      delete pulse;
    } else {
      return 0;
    }
  
  } // while
  
  return 0;
  
}

//////////////////////////////////////////////////
// set the max number of gates

void Ts2NetCDF::_setNGatesMax()
{

  _nGatesMax = 0;

  while (true) {
    
    if (_pulseReader->endOfFile()) {
      _pulseReader->reset();
      return;
    }
    
    // read next pulse
    
    IwrfTsPulse *pulse = _pulseReader->getNextPulse(true);
    if (pulse == NULL) {
      _pulseReader->reset();
      return;
    }
    
    if (_params.filter_antenna_transitions &&
        pulse->get_antenna_transition()) {
      delete pulse;
      continue;
    }

    // record nGates for this ray

    int nGates = pulse->getNGates();
    delete pulse;
    
    if (nGates > _nGatesMax) {
      _nGatesMax = nGates;
    }

  } // while
  
}

////////////////////////////////////////
// check for alternating mode

void Ts2NetCDF::_checkAltMode()
  
{

  // initialize

  int count = 0;
  int prevHvFlag = 0;
  _alternatingMode = false;

  // read a few pulses from start of queue

  while (true) {
    
    if (_pulseReader->endOfFile()) {
      // end of data
      _pulseReader->reset();
      return;
    }
    
    // read next pulse
    
    IwrfTsPulse *pulse = _pulseReader->getNextPulse(true);
    if (pulse == NULL) {
      // end of data
      _pulseReader->reset();
      return;
    }
    int hvFlag = pulse->getHdr().hv_flag;
    delete pulse;

    // initialize isHoriz flag

    if (count == 0) {
      prevHvFlag = hvFlag;
      count++;
      continue;
    }
    
    if (count > 2 && (prevHvFlag == hvFlag)) {
      // not alternating
      _alternatingMode = false;
      _pulseReader->reset();
      return;
    }
    
    if (count > 4) {
      // alternating
      _alternatingMode = true;
      _pulseReader->reset();
      return;
    }

    prevHvFlag = hvFlag;
    count++;

  } // while

  _alternatingMode = false;
  _pulseReader->reset();
  return;

}

/////////////////////////////////////////
// Check if ops info has changed
// Returns true if changed, false if not

bool Ts2NetCDF::_checkInfoChanged(const IwrfTsPulse &pulse)

{
  
  const IwrfTsInfo &info = pulse.getTsInfo();
  const iwrf_radar_info_t &radar = info.getRadarInfo();
  const iwrf_scan_segment_t &scan = info.getScanSegment();
  const iwrf_ts_processing_t &proc = info.getTsProcessing();
  const iwrf_calibration_t &calib = info.getCalibration();

  bool changed = false;

  if (pulse.getNGates() != _nGatesPrev) {
    if (!_params.pad_n_gates_to_max) {
      if (pulse.getNGates() == _nGatesPrev2) {
        cerr << "WARNING - ngates changed, prev, now: "
             << _nGatesPrev << ", " << pulse.getNGates() << endl;
        cerr << "May be staggered mode, consider setting -pad_n_gates_to_max" 
             << endl;
      }
      changed = true;
    }
    _nGatesPrev2 = _nGatesPrev;
    _nGatesPrev = pulse.getNGates();
  }

  if (pulse.getNChannels() != _nChannelsPrev) {
    _nChannelsPrev = pulse.getNChannels();
    changed = true;
  }

  if (iwrf_compare(radar, _radarPrev)) {
    _radarPrev = radar;
    changed = true;
  }

  if (scan.scan_mode != _scanPrev.scan_mode ||
      scan.volume_num != _scanPrev.volume_num ||
      scan.sweep_num != _scanPrev.sweep_num) {
    if (_params.debug) {
      cerr << "==>> New scan info" << endl;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	iwrf_scan_segment_print(stderr, scan);
      }
    }
    _scanPrev = scan;
    changed = true;
  }
  
  if (proc.xmit_rcv_mode != _procPrev.xmit_rcv_mode ||
      proc.xmit_phase_mode != _procPrev.xmit_phase_mode ||
      proc.prf_mode != _procPrev.prf_mode) {
    if (_params.debug) {
      cerr << "==>> New ts processing" << endl;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	iwrf_ts_processing_print(stderr, proc);
      }
    }
    _procPrev = proc;
    changed = true;
  }

  
  if (iwrf_compare(calib, _calibPrev)) {
    changed = true;
    _calibPrev = calib;
  }

  return changed;

}
  
///////////////////////////////////////////
// Check if we are ready to write the file
//
// Returns 0 to continue, -1 to exit

bool Ts2NetCDF::_checkReadyToWrite(const IwrfTsPulse &pulse)

{

  bool infoChanged = _checkInfoChanged(pulse);

  _nPulsesFile++;
  if (_nPulsesFile < 2) {
    return false;
  }

  // base decision on end-of-file?
  
  if (_params.input_mode == Params::TS_FILE_INPUT &&
      _params.save_one_file_per_input_file) {
    if (_pulseReader->endOfFile()) {
      return true;
    } else {
      return false;
    }
  }
  
  // check for change in scan or proc params

  if (_params.input_mode == Params::TS_FILE_INPUT &&
      _params.save_file_when_processing_details_change) {
    if (infoChanged) {
      return true;
    } else {
      return false;
    }
  }
  
  // compute sector in surveillance mode

  if (_params.save_scans_in_sectors) {
    if (pulse.getScanMode() == IWRF_SCAN_MODE_AZ_SUR_360 ||
        pulse.getScanMode() == IWRF_SCAN_MODE_SECTOR ||
        pulse.getScanMode() == IWRF_SCAN_MODE_COPLANE ||
        pulse.getScanMode() == IWRF_SCAN_MODE_SECTOR ||
        pulse.getScanMode() == IWRF_SCAN_MODE_MANPPI ||
        pulse.getScanMode() == IWRF_SCAN_MODE_NOT_SET) {
      int sector = (int) (pulse.getAz() / _sectorWidth);
      if (sector != _currentSector && !_params.one_file_only) {
        _currentSector = sector;
        return true;
      }
    }
  }
  
  // do we have too many pulses in the file?
  
  if (_nPulsesFile > _params.max_pulses_per_file) {
    return true;
  }

  return false;

}

/////////////////////////////
// handle a pulse

int Ts2NetCDF::_handlePulse(IwrfTsPulse &pulse)

{

  // convert to floats
  
  _nPulsesRead++;
  pulse.convertToFL32();

  // set pulse properties

  _nGates = pulse.getNGates();
  _pulseTimeSecs = pulse.getTime();
  _pulseTime = pulse.getFTime();
  _prt = pulse.get_prt();
  _el = pulse.getEl();
  _az = pulse.getAz();
  _phaseDiff = pulse.getPhaseDiff0();
  _txPhaseDeg = pulse.getTxPhaseDeg();

  // set start time etc

  if (_startTime == 0) {
    _startTime = _pulseTimeSecs;
    _startEl = _el;
    _startAz = _az;
    _scanMode = pulse.getScanMode();
    _xmitRcvMode = (iwrf_xmit_rcv_mode)
      _pulseReader->getOpsInfo().get_proc_xmit_rcv_mode();
  }

  // set number of gates to be saved out
  
  if (_nGatesSave < 0) {
    if (_params.specify_n_gates_save) {
      _nGatesSave = _params.n_gates_save;
    } else {
      _nGatesSave = _nGates;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "--> DEBUG: only saving pulses with nGates: "
           << _nGatesSave << endl;
    }
  }

  // allocate tmp storage

  if (_params.pad_n_gates_to_max) {
    _iBuf0.prepare(_nGatesMax * sizeof(float));
    _qBuf0.prepare(_nGatesMax * sizeof(float));
    _iBuf1.prepare(_nGatesMax * sizeof(float));
    _qBuf1.prepare(_nGatesMax * sizeof(float));
  } else {
    _iBuf0.prepare(_nGates * sizeof(float));
    _qBuf0.prepare(_nGates * sizeof(float));
    _iBuf1.prepare(_nGates * sizeof(float));
    _qBuf1.prepare(_nGates * sizeof(float));
  }

  // load up data
  
  if ((_nGates == _nGatesSave) || _params.pad_n_gates_to_max) {

    if (_alternatingMode) {
      if (pulse.isHoriz()) {
        if (_savePulseDataAltH(pulse)) {
          return -1;
        }
      } else if (_elArrayHc.size() > 0) {
        // wait to make sure we start on H pulse
        if (_savePulseDataAltV(pulse)) {
          return -1;
        }
      }
    } else {
      if (_savePulseData(pulse)) {
        return -1;
      }
    }
    
  } else {
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Skipping pulse, time, _prt, _el, _az, _nGates: "
           << _pulseTime << " "
           << _prt << " "
           << _el << " "
           << _az << " "
           << _nGates << endl;
    }
    
  }
  
  si64 seqNum = pulse.getSeqNum();
  double thisEl = pulse.getEl();
  double thisAz = pulse.getAz();
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    if ((_nPulsesRead % 1000) == 0) {
      cerr << "El, az, npulses received: "
           << thisEl << " " << thisAz << " " << _nPulsesRead << endl; 
    }
    if (_prevSeqNum > 0 && seqNum != (_prevSeqNum + 1)) {
      cerr << "Missing sequence numbers, prev: " << _prevSeqNum
           << ", this: " << seqNum << endl;
    }
    if (_prevAz > -900) {
      double deltaAz = fabs(thisAz - _prevAz);
      if (deltaAz >= 180) {
        deltaAz -= 360.0;
      }
      fprintf(stderr,
              "Azimuth, prevAz, thisAz, deltaAz, prevNum, thisNum: "
              "%10.3f %10.3f %10.3f %10d %10d\n",
              _prevAz, thisAz, deltaAz, (int) _prevSeqNum, (int) seqNum);
      if (fabs(deltaAz) > 0.1) {
        cerr << "************************************************" << endl;
        fprintf(stderr,
                "Missing azimuth, prevAz, thisAz, deltaAz, prevNum, thisNum: "
                "%10.3f %10.3f %10.3f %10d %10d\n",
                _prevAz, thisAz, deltaAz, (int) _prevSeqNum, (int) seqNum);
        cerr << "************************************************" << endl;
      }
    }

  } // debug

  _prevSeqNum = seqNum;
  _prevAz = thisAz;

  return 0;

}

/////////////////////////////
// save pulse data

int Ts2NetCDF::_savePulseData(IwrfTsPulse &pulse)

{

  _nGatesRay.push_back(_nGates);

  _timeArrayHc.push_back(_pulseTime);
  _dtimeArrayHc.push_back((_pulseTimeSecs - _startTime) 
                          + pulse.getNanoSecs() * 1.0e-9);
  _prtArrayHc.push_back(_prt);
  _pulseWidthArrayHc.push_back(pulse.getPulseWidthUs());
  _elArrayHc.push_back(_el);
  _azArrayHc.push_back(_az);
  _fixedAngleArrayHc.push_back(pulse.getFixedAngle());
  _modCodeArrayHc.push_back(_phaseDiff);
  _txPhaseArrayHc.push_back(_txPhaseDeg);
  _transitionFlagArrayHc.push_back(pulse.antennaTransition());

  _burstMagArrayHc.push_back(pulse.get_burst_mag(0));
  _burstMagArrayVc.push_back(pulse.get_burst_mag(1));
  _burstArgArrayHc.push_back(pulse.get_burst_arg(0));
  _burstArgArrayVc.push_back(pulse.get_burst_arg(1));

  const fl32 *chan0 = pulse.getIq0();
  float *ivals0 = (float *) _iBuf0.getPtr();
  float *qvals0 = (float *) _qBuf0.getPtr();
  
  for (int igate = 0; igate < _nGates; igate++, ivals0++, qvals0++) {
    *ivals0 = *chan0;
    chan0++;
    *qvals0 = *chan0;
    chan0++;
  } // igate
  if (_params.pad_n_gates_to_max) {
    for (int igate = _nGates; igate < _nGatesMax; igate++, ivals0++, qvals0++) {
      *ivals0 = -9999.0;
      *qvals0 = -9999.0;
    } // igate
  }

  int nGatesStore = _nGates;
  if (_params.pad_n_gates_to_max) {
    nGatesStore = _nGatesMax;
  }

  if (_params.chan0_is_h_or_copolar || pulse.getIq1() == NULL) {
    _nPulsesHc++;
    _iBufHc.add(_iBuf0.getPtr(), nGatesStore * sizeof(float));
    _qBufHc.add(_qBuf0.getPtr(), nGatesStore * sizeof(float));
  } else {
    _nPulsesVc++;
    _iBufVc.add(_iBuf0.getPtr(), nGatesStore * sizeof(float));
    _qBufVc.add(_qBuf0.getPtr(), nGatesStore * sizeof(float));
  }
  
  if (pulse.getIq1() != NULL) {
    
    const fl32 *chan1 = pulse.getIq1();
    float *ivals1 = (float *) _iBuf1.getPtr();
    float *qvals1 = (float *) _qBuf1.getPtr();
    
    for (int igate = 0; igate < nGatesStore; igate++, ivals1++, qvals1++) {
      *ivals1 = *chan1;
      chan1++;
      *qvals1 = *chan1;
      chan1++;
    } // igate

    if (_params.chan0_is_h_or_copolar) {
      _nPulsesVc++;
      _iBufVc.add(_iBuf1.getPtr(), nGatesStore * sizeof(float));
      _qBufVc.add(_qBuf1.getPtr(), nGatesStore * sizeof(float));
    } else {
      _nPulsesHc++;
      _iBufHc.add(_iBuf1.getPtr(), nGatesStore * sizeof(float));
      _qBufHc.add(_qBuf1.getPtr(), nGatesStore * sizeof(float));
    }

  } // if (chan1 != NULL)
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Using pulse, time, _prt, _el, _az, _nGates: "
         << _pulseTime << " "
         << _prt << " "
         << _el << " "
         << _az << " "
         << _nGates << endl;
  }

  return 0;

}
    
//////////////////////////////////////
// save pulse data in alternating mode
// transmit H

int Ts2NetCDF::_savePulseDataAltH(IwrfTsPulse &pulse)

{

  _nGatesRay.push_back(_nGates);

  _timeArrayHc.push_back(_pulseTime);
  _dtimeArrayHc.push_back((_pulseTimeSecs - _startTime) 
                          + pulse.getNanoSecs() * 1.0e-9);
  _prtArrayHc.push_back(_prt);
  _pulseWidthArrayHc.push_back(pulse.getPulseWidthUs());
  _elArrayHc.push_back(_el);
  _azArrayHc.push_back(_az);
  _fixedAngleArrayHc.push_back(pulse.getFixedAngle());
  _modCodeArrayHc.push_back(_phaseDiff);
  _txPhaseArrayHc.push_back(_txPhaseDeg);
  _transitionFlagArrayHc.push_back(pulse.antennaTransition());

  _burstMagArrayHc.push_back(pulse.get_burst_mag(0));
  _burstMagArrayVc.push_back(pulse.get_burst_mag(1));
  _burstArgArrayHc.push_back(pulse.get_burst_arg(0));
  _burstArgArrayVc.push_back(pulse.get_burst_arg(1));
  
  const fl32 *chan0 = pulse.getIq0();
  float *ivals0 = (float *) _iBuf0.getPtr();
  float *qvals0 = (float *) _qBuf0.getPtr();
  
  for (int igate = 0; igate < _nGates; igate++, ivals0++, qvals0++) {
    *ivals0 = *chan0;
    chan0++;
    *qvals0 = *chan0;
    chan0++;
  } // igate
  if (_params.pad_n_gates_to_max) {
    for (int igate = _nGates; igate < _nGatesMax; igate++, ivals0++, qvals0++) {
      *ivals0 = -9999.0;
      *qvals0 = -9999.0;
    } // igate
  }

  const fl32 *chan1 = pulse.getIq1();
  float *ivals1 = (float *) _iBuf1.getPtr();
  float *qvals1 = (float *) _qBuf1.getPtr();
  
  for (int igate = 0; igate < _nGates; igate++, ivals1++, qvals1++) {
    *ivals1 = *chan1;
    chan1++;
    *qvals1 = *chan1;
    chan1++;
  } // igate
  if (_params.pad_n_gates_to_max) {
    for (int igate = _nGates; igate < _nGatesMax; igate++, ivals1++, qvals1++) {
      *ivals1 = -9999.0;
      *qvals1 = -9999.0;
    } // igate
  }
  
  _nPulsesHc++;
  _nPulsesVx++;

  int nGatesStore = _nGates;
  if (_params.pad_n_gates_to_max) {
    nGatesStore = _nGatesMax;
  }

  if (_params.chan0_is_h_or_copolar) {
    _iBufHc.add(_iBuf0.getPtr(), nGatesStore * sizeof(float));
    _qBufHc.add(_qBuf0.getPtr(), nGatesStore * sizeof(float));
    _iBufVx.add(_iBuf1.getPtr(), nGatesStore * sizeof(float));
    _qBufVx.add(_qBuf1.getPtr(), nGatesStore * sizeof(float));
  } else {
    _iBufHc.add(_iBuf1.getPtr(), nGatesStore * sizeof(float));
    _qBufHc.add(_qBuf1.getPtr(), nGatesStore * sizeof(float));
    _iBufVx.add(_iBuf0.getPtr(), nGatesStore * sizeof(float));
    _qBufVx.add(_qBuf0.getPtr(), nGatesStore * sizeof(float));
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Using H pulse, time, _prt, _el, _az, _nGates: "
         << _pulseTime << " "
         << _prt << " "
         << _el << " "
         << _az << " "
         << _nGates << endl;
  }

  return 0;

}
    
//////////////////////////////////////
// save pulse data in alternating mode
// transmit V

int Ts2NetCDF::_savePulseDataAltV(IwrfTsPulse &pulse)
  
{
  
  _nGatesRay.push_back(_nGates);

  _timeArrayVc.push_back(_pulseTime);
  _dtimeArrayVc.push_back((_pulseTimeSecs - _startTime) 
                          + pulse.getNanoSecs() * 1.0e-9);
  _prtArrayVc.push_back(_prt);
  _pulseWidthArrayVc.push_back(pulse.getPulseWidthUs());
  _elArrayVc.push_back(_el);
  _azArrayVc.push_back(_az);
  _fixedAngleArrayVc.push_back(pulse.getFixedAngle());
  _modCodeArrayVc.push_back(_phaseDiff);
  _txPhaseArrayVc.push_back(_txPhaseDeg);
  _transitionFlagArrayVc.push_back(pulse.antennaTransition());

  _burstMagArrayVc.push_back(pulse.get_burst_mag(1));
  _burstArgArrayVc.push_back(pulse.get_burst_arg(1));
  
  const fl32 *chan0 = pulse.getIq0();
  float *ivals0 = (float *) _iBuf0.getPtr();
  float *qvals0 = (float *) _qBuf0.getPtr();
  
  for (int igate = 0; igate < _nGates; igate++, ivals0++, qvals0++) {
    *ivals0 = *chan0;
    chan0++;
    *qvals0 = *chan0;
    chan0++;
  } // igate
  if (_params.pad_n_gates_to_max) {
    for (int igate = _nGates; igate < _nGatesMax; igate++, ivals0++, qvals0++) {
      *ivals0 = -9999.0;
      *qvals0 = -9999.0;
    } // igate
  }

  const fl32 *chan1 = pulse.getIq1();
  float *ivals1 = (float *) _iBuf1.getPtr();
  float *qvals1 = (float *) _qBuf1.getPtr();
  
  for (int igate = 0; igate < _nGates; igate++, ivals1++, qvals1++) {
    *ivals1 = *chan1;
    chan1++;
    *qvals1 = *chan1;
    chan1++;
  } // igate

  if (_params.pad_n_gates_to_max) {
    for (int igate = _nGates; igate < _nGatesMax; igate++, ivals1++, qvals1++) {
      *ivals1 = -9999.0;
      *qvals1 = -9999.0;
    } // igate
  }
  
  _nPulsesVc++;
  _nPulsesHx++;

  int nGatesStore = _nGates;
  if (_params.pad_n_gates_to_max) {
    nGatesStore = _nGatesMax;
  }

  if (_xmitRcvMode == IWRF_ALT_HV_FIXED_HV) {
    // fixed receiver
    if (_params.chan0_is_h_or_copolar) {
      _iBufHx.add(_iBuf0.getPtr(), nGatesStore * sizeof(float));
      _qBufHx.add(_qBuf0.getPtr(), nGatesStore * sizeof(float));
      _iBufVc.add(_iBuf1.getPtr(), nGatesStore * sizeof(float));
      _qBufVc.add(_qBuf1.getPtr(), nGatesStore * sizeof(float));
    } else {
      // channels in reverse order
      _iBufHx.add(_iBuf1.getPtr(), nGatesStore * sizeof(float));
      _qBufHx.add(_qBuf1.getPtr(), nGatesStore * sizeof(float));
      _iBufVc.add(_iBuf0.getPtr(), nGatesStore * sizeof(float));
      _qBufVc.add(_qBuf0.getPtr(), nGatesStore * sizeof(float));
    }
  } else {
    // switching co-polar receiver
    if (_params.chan0_is_h_or_copolar) {
      _iBufVc.add(_iBuf0.getPtr(), nGatesStore * sizeof(float));
      _qBufVc.add(_qBuf0.getPtr(), nGatesStore * sizeof(float));
      _iBufHx.add(_iBuf1.getPtr(), nGatesStore * sizeof(float));
      _qBufHx.add(_qBuf1.getPtr(), nGatesStore * sizeof(float));
    } else {
      // channels in reverse order
      _iBufVc.add(_iBuf1.getPtr(), nGatesStore * sizeof(float));
      _qBufVc.add(_qBuf1.getPtr(), nGatesStore * sizeof(float));
      _iBufHx.add(_iBuf0.getPtr(), nGatesStore * sizeof(float));
      _qBufHx.add(_qBuf0.getPtr(), nGatesStore * sizeof(float));
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Using V pulse, time, _prt, _el, _az, _nGates: "
         << _pulseTime << " "
         << _prt << " "
         << _el << " "
         << _az << " "
         << _nGates << endl;
  }

  return 0;

}
    
////////////////////////////////////////
// reset data variables

void Ts2NetCDF::_reset()

{

  _startTime = 0;
  _startAz = 0;
  _startEl = 0;

  _nPulsesFile = 0;
  _nGates = 0;
  _nGatesSave = -1;

  _alternatingMode = false;

  _nPulsesHc = 0;
  _nPulsesVc = 0;
  _nPulsesHx = 0;
  _nPulsesVx = 0;

  _iBufHc.reset();
  _qBufHc.reset();
  _iBufVc.reset();
  _qBufVc.reset();
  _iBufHx.reset();
  _qBufHx.reset();
  _iBufVx.reset();
  _qBufVx.reset();

  _nGatesRay.clear();

  _timeArrayHc.clear();
  _dtimeArrayHc.clear();

  _elArrayHc.clear();
  _azArrayHc.clear();
  _fixedAngleArrayHc.clear();
  _prtArrayHc.clear();
  _pulseWidthArrayHc.clear();
  _modCodeArrayHc.clear();
  _txPhaseArrayHc.clear();
  _transitionFlagArrayHc.clear();
  _burstMagArrayHc.clear();
  _burstArgArrayHc.clear();

  _timeArrayVc.clear();
  _dtimeArrayVc.clear();
  _elArrayVc.clear();
  _azArrayVc.clear();
  _fixedAngleArrayVc.clear();
  _prtArrayVc.clear();
  _pulseWidthArrayVc.clear();
  _modCodeArrayVc.clear();
  _txPhaseArrayVc.clear();
  _transitionFlagArrayVc.clear();
  _burstMagArrayVc.clear();
  _burstArgArrayVc.clear();

}

////////////////////////////////////////
// write out the netDCF file
//
// Returns 0 on success, -1 on failure

int Ts2NetCDF::_writeFile()
  
{

  // compute output and tmp paths

  if (_computeOutputFilePaths()) {
    return -1;
  }

  // compute number of times active
  
  _nTimes = _elArrayHc.size();
  if (_elArrayVc.size() > 0) {
    if (_elArrayVc.size() < _nTimes) {
      _nTimes = _elArrayVc.size();
    }
  }

  // write out tmp file

  if (_writeFileTmp()) {
    cerr << "ERROR - Ts2NetCDF::_writeFile" << endl;
    cerr << "  Cannot write netCDF tmp file: " << _tmpPath << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "  Wrote tmp file: " << _tmpPath << endl;
  }

  // move the tmp file to final name
  
  if (rename(_tmpPath.c_str(), _outputPath.c_str())) {
    int errNum = errno;
    cerr << "ERROR - Ts2NetCDF::_writeFile" << endl;
    cerr << "  Cannot rename file: " << _tmpPath << endl;
    cerr << "             to file: " << _outputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "  Renamed file: " << _tmpPath << endl;
    cerr << "       to file: " << _outputPath << endl;
    cerr << "  ================================================" << endl;
  }

  cerr << "  Wrote file: " << _outputPath << endl;

  // reset data variables

  _reset();

  return 0;

}

////////////////////////////////////////
// write out the netDCF file to tmp name
// in iwrf format
// Returns 0 on success, -1 on failure

int Ts2NetCDF::_writeFileTmp()
  
{

  ////////////////////////
  // create Nc3File object
  
  NcxxFile file;
  try {
    file.open(_tmpPath, NcxxFile::replace, NcxxFile::nc4);
  } catch (NcxxException& e) {
    cerr << "ERROR - Ts2NetCDF::_writeFileTmp" << endl;
    cerr << "  Cannot open tmp Ncxx file for writing: " << _tmpPath << endl;
    cerr << "  exception: " << e.what() << endl;
    return -1;
  }
  int iret = 0;

  // global attributes

  _addGlobAtt(file);
  
  //////////////////
  // add dimensions
  
  NcxxDim gatesDim = file.addDim("gates", _nGates);
  NcxxDim timeDim = file.addDim("time", _nTimes);

  /////////////////////////////////
  // add vars and their attributes
  
  if (_writeBaseTimeVars(file)) {
    cerr << "ERROR - Ts2NetCDF::_writeFileTmp" << endl;
    return -1;
  }

  if (_writeRangeVar(file, gatesDim)) {
    cerr << "ERROR - Ts2NetCDF::_writeFileTmp" << endl;
    return -1;
  }

  if (_alternatingMode) {
    
    // alternating mode
    
    if (_writeTimeDimVarsAlt(file, timeDim)) {
      cerr << "ERROR - Ts2NetCDF::_writeFileTmp" << endl;
      return -1;
    }

  } else {
    
    if (_writeTimeDimVars(file, timeDim)) {
      cerr << "ERROR - Ts2NetCDF::_writeFileTmp" << endl;
      return -1;
    }

  }

  if (_nPulsesHc > 0) {
    if (_writeIqVars(file, timeDim, gatesDim, "IHc", "QHc",
                     (float *) _iBufHc.getPtr(),
                     (float *) _qBufHc.getPtr())) {
      return -1;
    }
  }

  if (_nPulsesVc > 0) {
    if (_writeIqVars(file, timeDim, gatesDim, "IVc", "QVc",
                     (float *) _iBufVc.getPtr(),
                     (float *) _qBufVc.getPtr())) {
      return -1;
    }
  }

  if (_nPulsesHx > 0) {
    if (_writeIqVars(file, timeDim, gatesDim, "IHx", "QHx",
                     (float *) _iBufHx.getPtr(),
                     (float *) _qBufHx.getPtr())) {
      return -1;
    }
  }

  if (_nPulsesVx > 0) {
    if (_writeIqVars(file, timeDim, gatesDim, "IVx", "QVx",
                     (float *) _iBufVx.getPtr(),
                     (float *) _qBufVx.getPtr())) {
      return -1;
    }
  }

  return iret;

}

////////////////////////////////////////
// add global attributes

void Ts2NetCDF::_addGlobAtt(NcxxFile &file)
  
{

  int startingSample = 0;
  int endingSample = startingSample + _nTimes - 1;
  int startGate = 0;
  int endGate = startGate + _nGates - 1;
  
  char desc[1024];
  sprintf(desc,
	  "Radar time series reformatted by Ts2NetCDF\n"
	  "  This is the IWRF format\n"
	  "  Starting Sample =%d, Ending Sample =%d, "
	  "  Start Gate= %d, End Gate = %d\n"
	  "  Azimuth = %.2f, Elevation = %.2f\n",
	  startingSample, endingSample, startGate, endGate,
	  _startAz, _startEl);

  file.addGlobAttr("Description", desc);
  file.addGlobAttr("FirstGate", startGate);
  file.addGlobAttr("LastGate", endGate);

  if (_params.pad_n_gates_to_max) {
    file.addGlobAttr("n_gates_padded_to_max", _nGatesMax);
  }

  // radar info

  iwrf_radar_info_t radarInfo(_radarPrev);
  file.addGlobAttr("radar_latitude_deg", radarInfo.latitude_deg);
  file.addGlobAttr("radar_longitude_deg", radarInfo.longitude_deg);
  file.addGlobAttr("radar_altitude_m", radarInfo.altitude_m);
  file.addGlobAttr("radar_platform_type", radarInfo.platform_type);
  file.addGlobAttr("radar_beamwidth_deg_h", radarInfo.beamwidth_deg_h);
  file.addGlobAttr("radar_beamwidth_deg_v", radarInfo.beamwidth_deg_v);
  file.addGlobAttr("radar_wavelength_cm", radarInfo.wavelength_cm);
  file.addGlobAttr("radar_nominal_gain_ant_db_h", 
                   radarInfo.nominal_gain_ant_db_h);
  file.addGlobAttr("radar_name", radarInfo.radar_name);
  file.addGlobAttr("radar_site_name", radarInfo.site_name);
  
  // scan info

  iwrf_scan_segment_t scanSeg(_scanPrev);

  file.addGlobAttr("scan_scan_mode", 
                   iwrf_scan_mode_to_str(scanSeg.scan_mode).c_str());
  file.addGlobAttr("scan_follow_mode", 
                   iwrf_follow_mode_to_str(scanSeg.follow_mode).c_str());
  file.addGlobAttr("scan_volume_num", scanSeg.volume_num);
  file.addGlobAttr("scan_sweep_num", scanSeg.sweep_num);
  file.addGlobAttr("scan_time_limit", scanSeg.time_limit);
  file.addGlobAttr("scan_az_manual", scanSeg.az_manual);
  file.addGlobAttr("scan_el_manual", scanSeg.el_manual);
  file.addGlobAttr("scan_az_start", scanSeg.az_start);
  file.addGlobAttr("scan_el_start", scanSeg.el_start);
  file.addGlobAttr("scan_scan_rate", scanSeg.scan_rate);
  file.addGlobAttr("scan_left_limit", scanSeg.left_limit);
  file.addGlobAttr("scan_right_limit", scanSeg.right_limit);
  file.addGlobAttr("scan_up_limit", scanSeg.up_limit);
  file.addGlobAttr("scan_down_limit", scanSeg.down_limit);
  file.addGlobAttr("scan_step", scanSeg.step);
  file.addGlobAttr("scan_current_fixed_angle", scanSeg.current_fixed_angle);
  file.addGlobAttr("scan_init_direction_cw", scanSeg.init_direction_cw);
  file.addGlobAttr("scan_init_direction_up", scanSeg.init_direction_up);
  file.addGlobAttr("scan_n_sweeps", scanSeg.n_sweeps);
  file.addGlobAttr("scan_sun_scan_sector_width_az", 
                   scanSeg.sun_scan_sector_width_az);
  file.addGlobAttr("scan_sun_scan_sector_width_el", 
                   scanSeg.sun_scan_sector_width_el);
  file.addGlobAttr("scan_segment_name", scanSeg.segment_name);
  file.addGlobAttr("scan_project_name", scanSeg.project_name);

  // processor info

  iwrf_ts_processing_t proc(_procPrev);

  file.addGlobAttr("proc_xmit_rcv_mode",
                   iwrf_xmit_rcv_mode_to_str(proc.xmit_rcv_mode).c_str());
  file.addGlobAttr("proc_xmit_phase_mode",
                   iwrf_xmit_phase_mode_to_str(proc.xmit_phase_mode).c_str());
  file.addGlobAttr("proc_prf_mode", 
                   iwrf_prf_mode_to_str(proc.prf_mode).c_str());
  file.addGlobAttr("proc_pulse_type", 
                   iwrf_pulse_type_to_str(proc.pulse_type).c_str());
  file.addGlobAttr("proc_prt_usec", proc.prt_usec);
  file.addGlobAttr("proc_prt2_usec", proc.prt2_usec);
  file.addGlobAttr("proc_cal_type", 
                   iwrf_cal_type_to_str(proc.cal_type).c_str());
  file.addGlobAttr("proc_burst_range_offset_m", proc.burst_range_offset_m);
  file.addGlobAttr("proc_pulse_width_us", proc.pulse_width_us);
  file.addGlobAttr("proc_start_range_m", proc.start_range_m);
  file.addGlobAttr("proc_gate_spacing_m", proc.gate_spacing_m);
  file.addGlobAttr("proc_integration_cycle_pulses",
                   proc.integration_cycle_pulses);
  file.addGlobAttr("proc_clutter_filter_number", proc.clutter_filter_number);
  file.addGlobAttr("proc_range_gate_averaging", proc.range_gate_averaging);
  file.addGlobAttr("proc_max_gate", proc.max_gate);
  file.addGlobAttr("proc_test_power_dbm", proc.test_power_dbm);
  file.addGlobAttr("proc_test_pulse_range_km", proc.test_pulse_range_km);
  file.addGlobAttr("proc_test_pulse_length_usec", proc.test_pulse_length_usec);
  file.addGlobAttr("proc_pol_mode", proc.pol_mode);
  file.addGlobAttr("proc_xmit_flag[0]", proc.xmit_flag[0]);
  file.addGlobAttr("proc_xmit_flag[1]", proc.xmit_flag[1]);
  file.addGlobAttr("proc_beams_are_indexed", proc.beams_are_indexed);
  file.addGlobAttr("proc_specify_dwell_width", proc.specify_dwell_width);
  file.addGlobAttr("proc_indexed_beam_width_deg", proc.indexed_beam_width_deg);
  file.addGlobAttr("proc_indexed_beam_spacing_deg",
                   proc.indexed_beam_spacing_deg);
  file.addGlobAttr("proc_num_prts", proc.num_prts);
  file.addGlobAttr("proc_prt3_usec", proc.prt3_usec);
  file.addGlobAttr("proc_prt4_usec", proc.prt4_usec);

  // calibration

  if (_params.override_radar_cal) {

    const DsRadarCalib &cal(_calOverride);
    file.addGlobAttr("cal_wavelength_cm", (float) cal.getWavelengthCm());
    file.addGlobAttr("cal_beamwidth_deg_h", (float) cal.getBeamWidthDegH());
    file.addGlobAttr("cal_beamwidth_deg_v", (float) cal.getBeamWidthDegV());
    file.addGlobAttr("cal_gain_ant_db_h", (float) cal.getAntGainDbH());
    file.addGlobAttr("cal_gain_ant_db_v", (float) cal.getAntGainDbV());
    file.addGlobAttr("cal_pulse_width_us", (float) cal.getPulseWidthUs());
    file.addGlobAttr("cal_xmit_power_dbm_h", (float) cal.getXmitPowerDbmH());
    file.addGlobAttr("cal_xmit_power_dbm_v", (float) cal.getXmitPowerDbmV());
    file.addGlobAttr("cal_two_way_waveguide_loss_db_h",
                     (float) cal.getTwoWayWaveguideLossDbH());
    file.addGlobAttr("cal_two_way_waveguide_loss_db_v",
                     (float) cal.getTwoWayWaveguideLossDbV());
    file.addGlobAttr("cal_two_way_radome_loss_db_h",
                     (float) cal.getTwoWayRadomeLossDbH());
    file.addGlobAttr("cal_two_way_radome_loss_db_v",
                     (float) cal.getTwoWayRadomeLossDbV());
    file.addGlobAttr("cal_receiver_mismatch_loss_db",
                     (float) cal.getReceiverMismatchLossDb());
    file.addGlobAttr("cal_radar_constant_h", (float) cal.getRadarConstH());
    file.addGlobAttr("cal_radar_constant_v", (float) cal.getRadarConstV());
    file.addGlobAttr("cal_noise_dbm_hc", (float) cal.getNoiseDbmHc());
    file.addGlobAttr("cal_noise_dbm_hx", (float) cal.getNoiseDbmHx());
    file.addGlobAttr("cal_noise_dbm_vc", (float) cal.getNoiseDbmVc());
    file.addGlobAttr("cal_noise_dbm_vx", (float) cal.getNoiseDbmVx());
    file.addGlobAttr("cal_receiver_gain_db_hc", (float) cal.getReceiverGainDbHc());
    file.addGlobAttr("cal_receiver_gain_db_hx", (float) cal.getReceiverGainDbHx());
    file.addGlobAttr("cal_receiver_gain_db_vc", (float) cal.getReceiverGainDbVc());
    file.addGlobAttr("cal_receiver_gain_db_vx", (float) cal.getReceiverGainDbVx());
    file.addGlobAttr("cal_receiver_slope_hc", (float) cal.getReceiverSlopeDbHc());
    file.addGlobAttr("cal_receiver_slope_hx", (float) cal.getReceiverSlopeDbHx());
    file.addGlobAttr("cal_receiver_slope_vc", (float) cal.getReceiverSlopeDbVc());
    file.addGlobAttr("cal_receiver_slope_vx", (float) cal.getReceiverSlopeDbVx());
    file.addGlobAttr("cal_base_dbz_1km_hc", (float) cal.getBaseDbz1kmHc());
    file.addGlobAttr("cal_base_dbz_1km_hx", (float) cal.getBaseDbz1kmHx());
    file.addGlobAttr("cal_base_dbz_1km_vc", (float) cal.getBaseDbz1kmVc());
    file.addGlobAttr("cal_base_dbz_1km_vx", (float) cal.getBaseDbz1kmVx());
    file.addGlobAttr("cal_sun_power_dbm_hc", (float) cal.getSunPowerDbmHc());
    file.addGlobAttr("cal_sun_power_dbm_hx", (float) cal.getSunPowerDbmHx());
    file.addGlobAttr("cal_sun_power_dbm_vc", (float) cal.getSunPowerDbmVc());
    file.addGlobAttr("cal_sun_power_dbm_vx", (float) cal.getSunPowerDbmVx());
    file.addGlobAttr("cal_noise_source_power_dbm_h",
                     (float) cal.getNoiseSourcePowerDbmH());
    file.addGlobAttr("cal_noise_source_power_dbm_v",
                     (float) cal.getNoiseSourcePowerDbmV());
    file.addGlobAttr("cal_power_meas_loss_db_h", (float) cal.getPowerMeasLossDbH());
    file.addGlobAttr("cal_power_meas_loss_db_v", (float) cal.getPowerMeasLossDbV());
    file.addGlobAttr("cal_coupler_forward_loss_db_h",
                     (float) cal.getCouplerForwardLossDbH());
    file.addGlobAttr("cal_coupler_forward_loss_db_v",
                     (float) cal.getCouplerForwardLossDbV());
    file.addGlobAttr("cal_test_power_dbm_h", (float) cal.getTestPowerDbmH());
    file.addGlobAttr("cal_test_power_dbm_v", (float) cal.getTestPowerDbmV());
    file.addGlobAttr("cal_zdr_correction_db", (float) cal.getZdrCorrectionDb());
    file.addGlobAttr("cal_ldr_correction_db_h", (float) cal.getLdrCorrectionDbH());
    file.addGlobAttr("cal_ldr_correction_db_v", (float) cal.getLdrCorrectionDbV());
    file.addGlobAttr("cal_phidp_rot_deg", (float) cal.getSystemPhidpDeg());

  } else {

    iwrf_calibration_t cal(_calibPrev);
    file.addGlobAttr("cal_wavelength_cm", cal.wavelength_cm);
    file.addGlobAttr("cal_beamwidth_deg_h", cal.beamwidth_deg_h);
    file.addGlobAttr("cal_beamwidth_deg_v", cal.beamwidth_deg_v);
    file.addGlobAttr("cal_gain_ant_db_h", cal.gain_ant_db_h);
    file.addGlobAttr("cal_gain_ant_db_v", cal.gain_ant_db_v);
    file.addGlobAttr("cal_pulse_width_us", cal.pulse_width_us);
    file.addGlobAttr("cal_xmit_power_dbm_h", cal.xmit_power_dbm_h);
    file.addGlobAttr("cal_xmit_power_dbm_v", cal.xmit_power_dbm_v);
    file.addGlobAttr("cal_two_way_waveguide_loss_db_h",
                     cal.two_way_waveguide_loss_db_h);
    file.addGlobAttr("cal_two_way_waveguide_loss_db_v",
                     cal.two_way_waveguide_loss_db_v);
    file.addGlobAttr("cal_two_way_radome_loss_db_h",
                     cal.two_way_radome_loss_db_h);
    file.addGlobAttr("cal_two_way_radome_loss_db_v",
                     cal.two_way_radome_loss_db_v);
    file.addGlobAttr("cal_receiver_mismatch_loss_db",
                     cal.receiver_mismatch_loss_db);
    file.addGlobAttr("cal_radar_constant_h", cal.radar_constant_h);
    file.addGlobAttr("cal_radar_constant_v", cal.radar_constant_v);
    file.addGlobAttr("cal_noise_dbm_hc", cal.noise_dbm_hc);
    file.addGlobAttr("cal_noise_dbm_hx", cal.noise_dbm_hx);
    file.addGlobAttr("cal_noise_dbm_vc", cal.noise_dbm_vc);
    file.addGlobAttr("cal_noise_dbm_vx", cal.noise_dbm_vx);
    file.addGlobAttr("cal_receiver_gain_db_hc", cal.receiver_gain_db_hc);
    file.addGlobAttr("cal_receiver_gain_db_hx", cal.receiver_gain_db_hx);
    file.addGlobAttr("cal_receiver_gain_db_vc", cal.receiver_gain_db_vc);
    file.addGlobAttr("cal_receiver_gain_db_vx", cal.receiver_gain_db_vx);
    file.addGlobAttr("cal_receiver_slope_hc", cal.receiver_slope_hc);
    file.addGlobAttr("cal_receiver_slope_hx", cal.receiver_slope_hx);
    file.addGlobAttr("cal_receiver_slope_vc", cal.receiver_slope_vc);
    file.addGlobAttr("cal_receiver_slope_vx", cal.receiver_slope_vx);
    file.addGlobAttr("cal_base_dbz_1km_hc", cal.base_dbz_1km_hc);
    file.addGlobAttr("cal_base_dbz_1km_hx", cal.base_dbz_1km_hx);
    file.addGlobAttr("cal_base_dbz_1km_vc", cal.base_dbz_1km_vc);
    file.addGlobAttr("cal_base_dbz_1km_vx", cal.base_dbz_1km_vx);
    file.addGlobAttr("cal_sun_power_dbm_hc", cal.sun_power_dbm_hc);
    file.addGlobAttr("cal_sun_power_dbm_hx", cal.sun_power_dbm_hx);
    file.addGlobAttr("cal_sun_power_dbm_vc", cal.sun_power_dbm_vc);
    file.addGlobAttr("cal_sun_power_dbm_vx", cal.sun_power_dbm_vx);
    file.addGlobAttr("cal_noise_source_power_dbm_h",
                     cal.noise_source_power_dbm_h);
    file.addGlobAttr("cal_noise_source_power_dbm_v",
                     cal.noise_source_power_dbm_v);
    file.addGlobAttr("cal_power_meas_loss_db_h", cal.power_meas_loss_db_h);
    file.addGlobAttr("cal_power_meas_loss_db_v", cal.power_meas_loss_db_v);
    file.addGlobAttr("cal_coupler_forward_loss_db_h",
                     cal.coupler_forward_loss_db_h);
    file.addGlobAttr("cal_coupler_forward_loss_db_v",
                     cal.coupler_forward_loss_db_v);
    file.addGlobAttr("cal_test_power_dbm_h", cal.test_power_dbm_h);
    file.addGlobAttr("cal_test_power_dbm_v", cal.test_power_dbm_v);
    file.addGlobAttr("cal_zdr_correction_db", cal.zdr_correction_db);
    file.addGlobAttr("cal_ldr_correction_db_h", cal.ldr_correction_db_h);
    file.addGlobAttr("cal_ldr_correction_db_v", cal.ldr_correction_db_v);
    file.addGlobAttr("cal_phidp_rot_deg", cal.phidp_rot_deg);

  }

}

////////////////////////////////////////
// write out base time variables
// Returns 0 on success, -1 on failure

int Ts2NetCDF::_writeBaseTimeVars(NcxxFile &file)
  
{
  
  // Base time - start time of data in secs

  char timeUnitsStr[256];
  DateTime stime(_startTime);
  sprintf(timeUnitsStr, "seconds since %.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
          1970, 1, 1, 0, 0, 0);
  
  NcxxVar baseTimeVar;
  if (_addVar(file, baseTimeVar, ncxxDouble,
              "base_time", "time_since_Jan1_1970", timeUnitsStr)) {
    cerr << "ERROR - Ts2NetCDF::_writeBaseTimeVars" << endl;
    cerr << "  Cannot create base_time var" << endl;
    return -1;
  }
  double baseTime = _startTime;
  baseTimeVar.putVal(baseTime);

  int year = stime.getYear();
  int month = stime.getMonth();
  int day = stime.getDay();
  int hour = stime.getHour();
  int min = stime.getMin();
  int sec = stime.getSec();

  NcxxVar baseYearVar;
  if (_addVar(file, baseYearVar, ncxxInt,
              "base_year", "base_time_year", "")) {
    cerr << "ERROR - Ts2NetCDF::_writeBaseTimeVars" << endl;
    cerr << "  Cannot create base_year var" << endl;
    return -1;
  }
  baseYearVar.putVal(year);

  NcxxVar baseMonthVar;
  if (_addVar(file, baseMonthVar, ncxxInt,
              "base_month", "base_time_month", "")) {
    cerr << "ERROR - Ts2NetCDF::_writeBaseTimeVars" << endl;
    cerr << "  Cannot create base_month var" << endl;
    return -1;
  }
  baseMonthVar.putVal(&month);

  NcxxVar baseDayVar;
  if (_addVar(file, baseDayVar, ncxxInt,
              "base_day", "base_time_day", "")) {
    cerr << "ERROR - Ts2NetCDF::_writeBaseTimeVars" << endl;
    cerr << "  Cannot create base_day var" << endl;
    return -1;
  }
  baseDayVar.putVal(&day);

  NcxxVar baseHourVar;
  if (_addVar(file, baseHourVar, ncxxInt,
              "base_hour", "base_time_hour", "")) {
    cerr << "ERROR - Ts2NetCDF::_writeBaseTimeVars" << endl;
    cerr << "  Cannot create base_hour var" << endl;
    return -1;
  }
  baseHourVar.putVal(&hour);

  NcxxVar baseMinVar;
  if (_addVar(file, baseMinVar, ncxxInt,
              "base_min", "base_time_min", "")) {
    cerr << "ERROR - Ts2NetCDF::_writeBaseTimeVars" << endl;
    cerr << "  Cannot create base_min var" << endl;
    return -1;
  }
  baseMinVar.putVal(&min);

  NcxxVar baseSecVar;
  if (_addVar(file, baseSecVar, ncxxInt,
              "base_sec", "base_time_sec", "")) {
    cerr << "ERROR - Ts2NetCDF::_writeBaseTimeVars" << endl;
    cerr << "  Cannot create base_sec var" << endl;
    return -1;
  }
  baseSecVar.putVal(&sec);

  return 0;

}  

////////////////////////////////////////
// write out time variables
// in iwrf format
// Returns 0 on success, -1 on failure

int Ts2NetCDF::_writeTimeDimVars(NcxxFile &file,
                                 NcxxDim &timeDim)
  
{
  
  // Time variable - secs since start of file

  char timeUnitsStr[256];
  DateTime stime(_startTime);
  sprintf(timeUnitsStr, "seconds since %.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
          stime.getYear(), stime.getMonth(), stime.getDay(),
          stime.getHour(), stime.getMin(), stime.getSec());
  
  NcxxVar timeVarHc;
  if (_addVar(file, timeVarHc, ncxxDouble, timeDim,
              "time_offset", "time_offset_from_base_time", timeUnitsStr)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    cerr << "  Cannot create time var" << endl;
    return -1;
  }
  timeVarHc.addScalarAttr("_FillValue", -9999.0);
  TaArray<double> times_;
  double *times = times_.alloc(_nTimes);
  for (size_t jj = 0; jj < _nTimes; jj++) {
    times[jj] = _dtimeArrayHc[jj];
  }
  timeVarHc.putVal(times);

  // ngates per ray

  if (_params.pad_n_gates_to_max) {
    if (_writeVar(file, timeDim,
                  "n_gates_ray", "number_of_valid_gates_in_ray", "",
                  _nGatesRay)) {
      cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
      return -1;
    }
  }
  
  // Elevation variable

  if (_writeVar(file, timeDim,
                "elevation", "elevation_angle", "degrees",
                _elArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // Azimuth variable
  
  if (_writeVar(file, timeDim,
                "azimuth", "azimuth_angle", "degrees",
                _azArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // Fixed angle variable
  
  if (_writeVar(file, timeDim,
                "fixed_angle", "fixed_scan_angle", "degrees",
                _fixedAngleArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // modulation code variable

  if (_writeVar(file, timeDim,
                "mod_code", "modulation_code", "degrees",
                _modCodeArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  if (_writeVar(file, timeDim,
                "tx_phase_deg", "transmit_phase", "degrees",
                _txPhaseArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // PRT variable
  
  if (_writeVar(file, timeDim,
                "prt", "pulse_repetition_time", "seconds",
                _prtArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // Pulse width variable
  
  if (_writeVar(file, timeDim,
                "pulse_width", "pulse_width", "micro_seconds",
                _pulseWidthArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // Antenna transition variable
  
  if (_writeVar(file, timeDim,
                "antenna_transition", "antenna_is_in_transition", "",
                _transitionFlagArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // write burst data
  
  if (_writeVar(file, timeDim,
                "burst_mag_hc", "", "",
                _burstMagArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  if (_writeVar(file, timeDim,
                "burst_mag_vc", "", "",
                _burstMagArrayVc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  if (_writeVar(file, timeDim,
                "burst_arg_hc", "", "",
                _burstArgArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  if (_writeVar(file, timeDim,
                "burst_arg_vc", "", "",
                _burstArgArrayVc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////////
// write out time variables in alternating dual pol mode
// Returns 0 on success, -1 on failure

int Ts2NetCDF::_writeTimeDimVarsAlt(NcxxFile &file,
                                    NcxxDim &timeDim)
  
{

  // Time variable - secs since start of file

  char timeUnitsStr[256];
  DateTime stime(_startTime);
  sprintf(timeUnitsStr, "seconds since %.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
          stime.getYear(), stime.getMonth(), stime.getDay(),
          stime.getHour(), stime.getMin(), stime.getSec());
  
  // h copolar times

  NcxxVar timeVarHc;
  if (_addVar(file, timeVarHc, ncxxDouble, timeDim,
              "time_offset_hc", "time_offset_from_base_time_hc", 
              timeUnitsStr)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    cerr << "  Cannot create time_offset_hc var" << endl;
    return -1;
  }
  timeVarHc.addScalarAttr("_FillValue", -9999.0);
  TaArray<double> timesHc_;
  double *timesHc = timesHc_.alloc(_nTimes);
  for (size_t jj = 0; jj < _nTimes; jj++) {
    timesHc[jj] = _dtimeArrayHc[jj];
  }
  timeVarHc.putVal(timesHc);

  // v copolar times
  
  NcxxVar timeVarVc;
  if (_addVar(file, timeVarVc, ncxxDouble, timeDim,
              "time_offset_vc", "time_offset_from_base_time_vc", 
              timeUnitsStr)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    cerr << "  Cannot create time_offset_vc var" << endl;
    return -1;
  }
  timeVarVc.addScalarAttr("_FillValue", -9999.0);
  TaArray<double> timesVc_;
  double *timesVc = timesVc_.alloc(_nTimes);
  for (size_t jj = 0; jj < _nTimes; jj++) {
    timesVc[jj] = _dtimeArrayVc[jj];
  }
  timeVarVc.putVal(timesVc);

  // Elevation variable

  if (_writeVar(file, timeDim,
                "elevation_hc", "elevation_angle_h_copolar", "degrees",
                _elArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  if (_writeVar(file, timeDim,
                "elevation_vc", "elevation_angle_v_copolar", "degrees",
                _elArrayVc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // Azimuth variable
  
  if (_writeVar(file, timeDim,
                "azimuth_hc", "azimuth_angle_h_copolar", "degrees",
                _azArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  if (_writeVar(file, timeDim,
                "azimuth_vc", "azimuth_angle_v_copolar", "degrees",
                _azArrayVc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // Fixed angle variable
  
  if (_writeVar(file, timeDim,
                "fixed_angle_hc", "fixed_scan_angle_h_copolar", "degrees",
                _fixedAngleArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  if (_writeVar(file, timeDim,
                "fixed_angle_vc", "fixed_scan_angle_v_copolar", "degrees",
                _fixedAngleArrayVc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // modulation code variable

  if (_writeVar(file, timeDim,
                "mod_code_hc", "modulation_code_h_copolar", "degrees",
                _modCodeArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  if (_writeVar(file, timeDim,
                "mod_code_vc", "modulation_code_v_copolar", "degrees",
                _modCodeArrayVc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }

  if (_writeVar(file, timeDim,
                "tx_phase_deg_hc", "transmit_phase_h_copolar", "degrees",
                _txPhaseArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  if (_writeVar(file, timeDim,
                "tx_phase_deg_vc", "transmit_phase_v_copolar", "degrees",
                _txPhaseArrayVc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  
  // PRT variable
  
  if (_writeVar(file, timeDim,
                "prt_hc", "pulse_repetition_time_h_copolar", "seconds",
                _prtArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  if (_writeVar(file, timeDim,
                "prt_vc", "pulse_repetition_time_v_copolar", "seconds",
                _prtArrayVc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // Pulse width variable
  
  if (_writeVar(file, timeDim,
                "pulse_width_hc", "pulse_width_h_copolar", "micro_seconds",
                _pulseWidthArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  if (_writeVar(file, timeDim,
                "pulse_width_vc", "pulse_width_v_copolar", "micro_seconds",
                _pulseWidthArrayVc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // Antenna transition variable
  
  if (_writeVar(file, timeDim,
                "antenna_transition_hc", "antenna_is_in_transition_h_copolar", "",
                _transitionFlagArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }

  if (_writeVar(file, timeDim,
                "antenna_transition_vc", "antenna_is_in_transition_v_copolar", "",
                _transitionFlagArrayVc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  // write burst data
  
  if (_writeVar(file, timeDim,
                "burst_mag_hc", "burst_magnitude_H_copolar", "",
                _burstMagArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  if (_writeVar(file, timeDim,
                "burst_mag_vc", "burst_magnitude_V_copolar", "",
                _burstMagArrayVc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  if (_writeVar(file, timeDim,
                "burst_arg_hc", "burst_phase_H_copolar", "degrees",
                _burstArgArrayHc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  if (_writeVar(file, timeDim,
                "burst_arg_vc", "burst_phase_V_copolar", "",
                _burstArgArrayVc)) {
    cerr << "ERROR - Ts2NetCDF::_writeTimeDimVars" << endl;
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////
// write out range coordinate variable
// Returns 0 on success, -1 on failure

int Ts2NetCDF::_writeRangeVar(NcxxFile &file,
                              NcxxDim &gatesDim)
  
{
  
  NcxxVar rangeVar;
  if (_addVar(file, rangeVar, ncxxFloat, gatesDim,
              "range", "range_to_center_of_gate", "m")) {
    cerr << "ERROR - Ts2NetCDF::_writeRangeVar" << endl;
    cerr << "  Cannot create range var" << endl;
    return -1;
  }

  TaArray<float> range_;
  float *range = range_.alloc(_nGates);
  double thisRange = _procPrev.start_range_m;
  double gateSpacing = _procPrev.gate_spacing_m;
  for (int ii = 0; ii < _nGates; ii++, thisRange += gateSpacing) {
    range[ii] = thisRange;
  }
  rangeVar.putVal(range);

  return 0;

}

/////////////////////////////////
// compute output file path
//
// Returns 0 on success, -1 on failure

int Ts2NetCDF::_computeOutputFilePaths()

{

  DateTime stime(_startTime);
  
  char elevAngleStr[64];
  sprintf(elevAngleStr, "_%.2f", _startEl);

  char azAngleStr[64];
  sprintf(azAngleStr, "_%.2f", _startAz);
  
  string scanModeStr;
  switch (_scanMode) {
    case IWRF_SCAN_MODE_SECTOR:
      scanModeStr = ".sec";
      break;
    case IWRF_SCAN_MODE_COPLANE:
      scanModeStr = ".coplane";
      break;
    case IWRF_SCAN_MODE_RHI:
      scanModeStr = ".rhi";
      break;
    case IWRF_SCAN_MODE_VERTICAL_POINTING:
      scanModeStr = ".vert";
      break;
    case IWRF_SCAN_MODE_IDLE:
      scanModeStr = ".idle";
      break;
    case IWRF_SCAN_MODE_AZ_SUR_360:
      scanModeStr = ".sur";
      break;
    case IWRF_SCAN_MODE_EL_SUR_360:
      scanModeStr = ".elsur";
      break;
    case IWRF_SCAN_MODE_SUNSCAN:
      scanModeStr = ".sun";
      break;
    case IWRF_SCAN_MODE_POINTING:
      scanModeStr = ".point";
      break;
    case IWRF_SCAN_MODE_MANPPI:
      scanModeStr = ".manppi";
      break;
    case IWRF_SCAN_MODE_MANRHI:
      scanModeStr = ".manrhi";
      break;
    default: {}
  }
    
  // make the output dir

  char subDir[1024];
  sprintf(subDir, "%s/%.4d%.2d%.2d", _params.output_dir,
          stime.getYear(), stime.getMonth(), stime.getDay());
  
  if (ta_makedir_recurse(subDir)) {
    int errNum = errno;
    cerr << "ERROR - Ts2NetCDF" << endl;
    cerr << "  Cannot make output directory: " << subDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute output path

  char relPath[1024];

  if (_params.input_mode == Params::TS_FILE_INPUT &&
      _params.preserve_file_name) {
    Path path(_pulseReader->getPrevPathInUse());
    sprintf(relPath, "%s.nc", path.getFile().c_str());
  } else {
    sprintf(relPath, "%.4d%.2d%.2d_%.2d%.2d%.2d%s%s%s.nc",
            stime.getYear(), stime.getMonth(), stime.getDay(),
            stime.getHour(), stime.getMin(), stime.getSec(),
            elevAngleStr, azAngleStr, scanModeStr.c_str());
  }

  _outputName = relPath;

  _outputPath = subDir;
  _outputPath += PATH_DELIM;
  _outputPath += _outputName;

  _tmpPath = _outputPath;
  _tmpPath += ".tmp";
  
  return 0;

}

////////////////////////////////////////////////
// add string attribute to a variable

int Ts2NetCDF::_addAttr(NcxxVar &var,
                        const string &name,
                        const string &val)
  
{
  try {
    var.addScalarAttr(name , val);
  } catch (NcxxException& e) {
    cerr << "ERROR - Ts2NetCDF::_addAttr" << endl;
    cerr << "  Cannot add attr name: " << name << endl;
    cerr << "  val: " << val << endl;
    cerr << "  Variable name: " << var.getName() << endl;
    cerr << "  exception: " << e.what() << endl;
    return -1;
  }
  return 0;
}

////////////////////////////////////////////////
// add double attribute to a variable

int Ts2NetCDF::_addAttr(NcxxVar & var,
                        const string &name,
                        double val)

{
  try {
    var.addScalarAttr(name , val);
  } catch (NcxxException& e) {
    cerr << "ERROR - Ts2NetCDF::_addAttr" << endl;
    cerr << "  Cannot add attr name: " << name << endl;
    cerr << "  val: " << val << endl;
    cerr << "  Variable name: " << var.getName() << endl;
    cerr << "  exception: " << e.what() << endl;
    return -1;
  }
  return 0;
}

////////////////////////////////////////////////
// add int attribute to a variable

int Ts2NetCDF::_addAttr(NcxxVar &var,
                        const string &name,
                        int val)
  
{
  try {
    var.addScalarAttr(name , val);
  } catch (NcxxException& e) {
    cerr << "ERROR - Ts2NetCDF::_addAttr" << endl;
    cerr << "  Cannot add attr name: " << name << endl;
    cerr << "  val: " << val << endl;
    cerr << "  Variable name: " << var.getName() << endl;
    cerr << "  exception: " << e.what() << endl;
    return -1;
  }
  return 0;
}

//////////////////////////////////////////////
// add scalar variable

int Ts2NetCDF::_addVar(NcxxFile &file,
                       NcxxVar &var,
                       NcxxType ncType,
                       const string &name,
                       const string &standardName,
                       const string &units /* = "" */)

{

  try {
    var = file.addVar(name,
                      standardName,
                      standardName,
                      ncType,
                      units);
  } catch (NcxxException& e) {
    cerr << "ERROR - Ts2NetCDF::_addVar" << endl;
    cerr << "  Cannot add scalar var, name: " << name << endl;
    cerr << "  Type: " << _ncTypeToStr(ncType) << endl;
    cerr << "  exception: " << e.what() << endl;
    return -1;
  }
  
  return 0;
  
}

//////////////////////////////////////////////
// add a 1-D variable

int Ts2NetCDF::_addVar(NcxxFile &file,
                       NcxxVar &var,
                       NcxxType ncType,
                       NcxxDim &dim, 
                       const string &name,
                       const string &standardName,
                       const string &units /* = "" */)

{
  
  try {
    var = file.addVar(name,
                      standardName,
                      standardName,
                      ncType,
                      dim,
                      units);
  } catch (NcxxException& e) {
    cerr << "ERROR - Ts2NetCDF::_addVar" << endl;
    cerr << "  Cannot add 1-D var, name: " << name << endl;
    cerr << "  Type: " << _ncTypeToStr(ncType) << endl;
    cerr << "  Dim: " << dim.getName() << endl;
    cerr << "  exception: " << e.what() << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////
// add a 2-D variable

int Ts2NetCDF::_addVar(NcxxFile &file,
                       NcxxVar &var,
                       NcxxType ncType,
                       NcxxDim &dim0, 
                       NcxxDim &dim1, 
                       const string &name,
                       const string &standardName,
                       const string &units /* = "" */)

{
  
  try {
    var = file.addVar(name,
                      standardName,
                      standardName,
                      ncType,
                      dim0,
                      dim1,
                      units);
  } catch (NcxxException& e) {
    cerr << "ERROR - Ts2NetCDF::_addVar" << endl;
    cerr << "  Cannot add 2-D var, name: " << name << endl;
    cerr << "  Type: " << _ncTypeToStr(ncType) << endl;
    cerr << "  Dim0: " << dim0.getName() << endl;
    cerr << "  Dim1: " << dim1.getName() << endl;
    cerr << "  exception: " << e.what() << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////
// add and write float var
// Returns 0 on success, -1 on failure

int Ts2NetCDF::_writeVar(NcxxFile &file,
                         NcxxDim &timeDim,
                         const string &name,
                         const string &standardName,
                         const string &units,
                         const vector<float> vals)
  
{
  
  TaArray<float> floats_;
  float *floats = floats_.alloc(timeDim.getSize());
  
  NcxxVar var;
  if (_addVar(file, var, ncxxFloat, timeDim,
              name, standardName, units)) {
    cerr << "ERROR - Ts2NetCDF::_addFloatVar" << endl;
    cerr << "  Cannot create var, name: " << name << endl;
    return -1;
  }
  var.addScalarAttr("_FillValue", -9999.0f);
  for (size_t jj = 0; jj < timeDim.getSize(); jj++) {
    floats[jj] = vals[jj];
  }
  var.putVal(floats);
  
  return 0;

}

////////////////////////////////////////
// add and write int var
// Returns 0 on success, -1 on failure

int Ts2NetCDF::_writeVar(NcxxFile &file,
                         NcxxDim &timeDim,
                         const string &name,
                         const string &standardName,
                         const string &units,
                         const vector<int> vals)
  
{
  
  TaArray<int> ints_;
  int *ints = ints_.alloc(timeDim.getSize());
  
  NcxxVar var;
  if (_addVar(file, var, ncxxInt, timeDim,
              name, standardName, units)) {
    cerr << "ERROR - Ts2NetCDF::_addIntVar" << endl;
    cerr << "  Cannot create var, name: " << name << endl;
    return -1;
  }
  var.addScalarAttr("_FillValue", -9999);
  for (size_t jj = 0; jj < timeDim.getSize(); jj++) {
    ints[jj] = vals[jj];
  }
  var.putVal(ints);
  
  return 0;

}

////////////////////////////////////////
// add and write IQ vars
// Returns 0 on success, -1 on failure

int Ts2NetCDF::_writeIqVars(NcxxFile &file,
                            NcxxDim &timeDim,
                            NcxxDim &gatesDim,
                            const string &iName,
                            const string &qName,
                            const float *ivals,
                            const float *qvals)
  
{

  // I variable
  
  NcxxVar iVar = file.addVar(iName,
                             "time_series_in_phase",
                             "time_series_in_phase",
                             ncxxFloat,
                             timeDim,
                             gatesDim,
                             "scaled A/D counts");
  iVar.addScalarAttr("_FillValue", (float) -9999.0);
  iVar.putVal(ivals);

  // Q variable

  NcxxVar qVar = file.addVar(qName,
                             "time_series_quadrature",
                             "time_series_quadrature",
                             ncxxFloat,
                             timeDim,
                             gatesDim,
                             "scaled A/D counts");
  qVar.addScalarAttr("_FillValue", (float) -9999.0);
  qVar.putVal(qvals);

  return 0;

}

////////////////////////////////////////
// convert enums to strings

string Ts2NetCDF::_ncTypeToStr(NcxxType nctype)
  
{
  return nctype.getTypeClassName();
}

