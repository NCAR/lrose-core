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
// TsSmartSave.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// TsSmartSave reads data from TsTcpServer in TS TimeSeries format
// and writes it to files in TsArchive format
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <ctime>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <dsserver/DsLdataInfo.hh>
#include "TsSmartSave.hh"

using namespace std;

// Constructor

TsSmartSave::TsSmartSave(int argc, char **argv)
  
{

  isOK = true;
  MEM_zero(_scanPrev);
  MEM_zero(_procPrev);
  _transPrev = false;
  _nPulses = 0;
  _prevSeqNum = 0;
  _prevAz = -999;
  
  // set programe name
  
  _progName = "TsSmartSave";

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
  
  if (_params.debug) {
    cerr << "Running TsSmartSave" << endl;
    if (_params.save_normal_scan_data) {
      cerr << "Saving normal scans, dir: "
           << _params.normal_output_dir << endl;
      if (_params.normal_ignore_idle_mode) {
        cerr << "  NOTE: ignoring IDLE mode scans" << endl;
      }
      if (_params.normal_ignore_stationary_antenna) {
        cerr << "  NOTE: ignoring stationary antenna" << endl;
      }
    }
    if (_params.save_vert_pointing_data) {
      cerr << "Saving vert pointing, dir: "
           << _params.vert_pointing_output_dir << endl;
    }
    if (_params.save_sun_scan_data) {
      cerr << "Saving sun scans, dir: "
           << _params.sun_scan_output_dir << endl;
    }
    if (_params.save_when_flag_file_exists) {
      cerr << "Saving data when flag file exists, dir: "
           << _params.flag_file_output_dir << endl;
      cerr << "  Flag file path: "
           << _params.flag_file_path << endl;
    }
  }

  // create the reader from FMQ
  
  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrfDebug = IWRF_DEBUG_NORM;
    // iwrfDebug = IWRF_DEBUG_VERBOSE;
    // } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    //   iwrfDebug = IWRF_DEBUG_NORM;
  } 
  _pulseReader = new IwrfTsReaderFmq(_params.fmq_name, iwrfDebug);
  
  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  // movement check - initialization
  
  _isMoving = true;
  _moveCheckTime = -1;
  _moveCheckAz = -999;
  _moveCheckEl = -999;

  // compute sector size etc for files

  _out = NULL;
  _needNewFile = true;
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

  // pointing mode

  _currentPointingMode = MODE_NORMAL;
  _isStationary = true;
  _timeEnterVertPoint = 0.0;
  _timeEnterSunScan = 0.0;
  _timePrintSunPosn = 0;
 
  _sunPosn.setLocation(_params.radar_latitude, _params.radar_longitude,
                       _params.radar_altitude_km * 1000);

  // run period and exit time

  _startTime = time(NULL);
  _exitTime = -1;
  if (_params.exit_after_specified_period) {
    _exitTime = _startTime + _params.run_period_secs;
    if (_params.debug) {
      cerr << "NOTE: run period (secs): " << _params.run_period_secs << endl;
      cerr << "      will exit at time: " << DateTime::strm(_exitTime) << endl;
    }
  }

  return;
  
}

// destructor

TsSmartSave::~TsSmartSave()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TsSmartSave::Run ()
{
  
  PMU_auto_register("Run");
  
  while (true) {

    time_t now = time(NULL);
    if (_exitTime > 0 && now > _exitTime) {
      if (_params.debug) {
        cerr << "Exiting at time: " << DateTime::strm(now) << endl;
      }
      return 0;
    }
    
    // read next pulse
    
    IwrfTsPulse *pulse = _pulseReader->getNextPulse();
    if (pulse == NULL) {
      return 0;
    }
    _currentScanMode = (iwrf_scan_mode_t) pulse->get_scan_mode();

    if (_params.filter_antenna_transitions && pulse->get_antenna_transition()) {
      delete pulse;
      continue;
    }

    // check if we need a new file? If so open file
    
    if (_checkNeedNewFile(*pulse)) {
      return -1;
    }
    
    // write pulse info to file, if info has changed since last write

    if (_out != NULL) {
      if (_handlePulse(*pulse)) {
	return -1;
      }
    }

    // delete pulse to clean up

    delete pulse;
  
  } // while
  
  return 0;
  
}

/////////////////////////////////////////
// Check if we need a new file
// Opens new file as needed.
//
// Returns 0 to continue, -1 to exit

int TsSmartSave::_checkNeedNewFile(const IwrfTsPulse &pulse)

{

  bool needNewFile = false;

  const IwrfTsInfo &info = pulse.getTsInfo();
  const iwrf_scan_segment_t &scan = info.getScanSegment();
  const iwrf_ts_processing_t &proc = info.getTsProcessing();

  bool trans = pulse.antennaTransition();
  if (trans != _transPrev) {
    if (_params.debug) {
      if (trans) {
	cerr << "==>> Starting transition" << endl;
      } else {
	cerr << "==>> Ending transition" << endl;
      }
    }
    needNewFile = true;
    _transPrev = trans;
  }

  if (scan.scan_mode != _scanPrev.scan_mode ||
      scan.volume_num != _scanPrev.volume_num ||
      scan.sweep_num != _scanPrev.sweep_num) {
    if (_params.debug) {
      cerr << "==>> New scan info" << endl;
      if (_params.debug >= Params::DEBUG_EXTRA) {
	iwrf_scan_segment_print(stderr, scan);
      }
    }
    if (!_transPrev) {
      needNewFile = true;
    }
    _scanPrev = scan;
  }
  
  if (proc.xmit_rcv_mode != _procPrev.xmit_rcv_mode ||
      proc.xmit_phase_mode != _procPrev.xmit_phase_mode ||
      proc.prf_mode != _procPrev.prf_mode) {
    if (_params.debug) {
      cerr << "==>> New ts processing" << endl;
      if (_params.debug >= Params::DEBUG_EXTRA) {
	iwrf_ts_processing_print(stderr, proc);
      }
    }
    if (!_transPrev) {
      needNewFile = true;
    }
    _procPrev = proc;
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
        needNewFile = true;
        _currentSector = sector;
      }
    }
  }

  // do we have too many pulses in the file?
  
  _nPulsesFile++;
  if (_nPulsesFile > _params.max_pulses_per_file) {
    needNewFile = true;
  }

  // new pointing mode

  pointing_mode_t mode =
    _getPointingMode(pulse.getTime(), pulse.getEl(), pulse.getAz());
  if (mode != _currentPointingMode) {
    _currentPointingMode = mode;
    needNewFile = true;
    if (_params.debug) {
      if (_currentPointingMode == MODE_NORMAL) {
        cerr << "Changing pointing mode to MODE_NORMAL" << endl;
      } else if (_currentPointingMode == MODE_VERT_POINT) {
        cerr << "Changing pointing mode to MODE_VERT_POINT" << endl;
      } else if (_currentPointingMode == MODE_SUN_SCAN) {
        cerr << "Changing pointing mode to MODE_SUN_SCAN" << endl;
      } else if (_currentPointingMode == MODE_FLAG_FILE) {
        cerr << "Changing pointing mode to MODE_FLAG_FILE" << endl;
      } else if (_currentPointingMode == MODE_STATIONARY) {
        cerr << "Changing pointing mode to MODE_STATIONARY" << endl;
      }
    }
  }

  // open a new file if needed
    
  if (needNewFile) {

    // check for special case of writing one file only

    if (_out != NULL && _params.one_file_only) {
      _closeFile();
      return -1;
    }

    _nPulsesFile = 0;
    
    if (_openNewFile(pulse) == 0) {

      if (_params.debug >= Params::DEBUG_EXTRA) {
	info.print(stderr);
      }

      // write ops info to the start of the file
      
      if (_params.output_format == Params::FORMAT_TSARCHIVE) {
	info.writeToTsarchiveFile(_out);
      } else {
	info.writeMetaToFile(_out, 0);
      }
  
    }

  }

  return 0;

}

/////////////////////////////
// handle a pulse

int TsSmartSave::_handlePulse(IwrfTsPulse &pulse)

{

  // write ops info to file, if info has changed since last write
  
  const IwrfTsInfo &info = pulse.getTsInfo();

  if (_params.output_format == Params::FORMAT_TSARCHIVE) {
    info.writeToTsarchiveFile(_out);
  } else {
    info.writeMetaQueueToFile(_out, true);
  }

  // reformat pulse as needed

  if (_params.output_format == Params::FORMAT_TSARCHIVE) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_FL32);
  } else if (_params.output_packing == Params::PACKING_FL32) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_FL32);
  } else if (_params.output_packing == Params::PACKING_SCALED_SI16) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_SCALED_SI16);
  } else if (_params.output_packing == Params::PACKING_DBM_PHASE_SI16) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_DBM_PHASE_SI16);
  } else if (_params.output_packing == Params::PACKING_SIGMET_FL16) {
    pulse.convertToPacked(IWRF_IQ_ENCODING_SIGMET_FL16);
  }

  // write burst if applicable

  if (_pulseReader->isBurstForLatestPulse()) {
    if (_params.output_format == Params::FORMAT_IWRF) {
      _pulseReader->getBurst().writeToFile(_out);
    }
  }

  // write pulse to file
  
  if (_params.output_format == Params::FORMAT_TSARCHIVE) {
    pulse.writeToTsarchiveFile(_out);
  } else {
    pulse.writeToFile(_out);
  }

  _nPulses++;
  si64 seqNum = pulse.getSeqNum();
  double thisEl = pulse.getEl();
  double thisAz = pulse.getAz();
  
  if (_prevSeqNum > 0 && seqNum > (_prevSeqNum + 1)) {
    cerr << "WARNING - missing sequence numbers, prev: " << _prevSeqNum
         << ", latest: " << seqNum << endl;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    if ((_nPulses % 1000) == 0) {
      cerr << "El, az, npulses received: "
           << thisEl << " " << thisAz << " " << _nPulses << endl; 
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

/////////////////////////////////////////
// Check for pointing mode

TsSmartSave::pointing_mode_t
  TsSmartSave::_getPointingMode(time_t ptime, double el, double az)

{

  pointing_mode_t pointingMode = MODE_NORMAL;

  // get time and antenna position
  
  if (az < 0) {
    az += 360.0;
  }
  if (el > 180) {
    el -= 360.0;
  }
  
  // first check for vertical pointing mode

  if (_params.save_vert_pointing_data &&
      (_params.check_for_vert_mode == false || 
       _currentScanMode == IWRF_SCAN_MODE_VERTICAL_POINTING)) {
    
    if (el >= _params.min_elev_for_vert_pointing &&
        el <= _params.max_elev_for_vert_pointing) {
      
      if (_timeEnterVertPoint > 0) {
        double periodSinceEnter = ptime - _timeEnterVertPoint;
        if (periodSinceEnter >= _params.vert_pointing_lockon_period) {
          pointingMode = MODE_VERT_POINT;
        }
      } else {
        if (_params.debug) {
          cerr << "--->> Entering possible VERT_POINT mode" << endl;
        }
        _timeEnterVertPoint = ptime;
      }

    } else {
      
      if (_timeEnterVertPoint != 0) {
        if (_params.debug) {
          cerr << "Exiting VERT_POINT mode" << endl;
        }
        _timeEnterVertPoint = 0;
      }

    } // if (_timeEnterVertPoint > 0)
    
  } // if (_params.save_vert_pointing_data)
  
  // if not in vert mode, check if we are pointing close to the sun

  if (pointingMode == MODE_NORMAL &&
      _params.save_sun_scan_data &&
      (!_params.check_for_sun_scan_sector_mode || 
       (_currentScanMode == IWRF_SCAN_MODE_SECTOR))) {
    
    double sunEl, sunAz;
    _sunPosn.computePosnNova(ptime, sunEl, sunAz);
    double elDiff = fabs(el - sunEl);
    double azDiff = fabs(az - sunAz);
    if (azDiff > 180) {
      azDiff = fabs(azDiff - 360.0);
    }

    // cerr << "sunEl, sunAz, elDiff, azDiff: "
    //      << sunEl << ", " << sunAz << ", " << elDiff << ", " << azDiff << endl;
    
    if (elDiff < _params.sun_scan_pointing_margin &&
        azDiff < _params.sun_scan_pointing_margin) {

      // check time since entering sun pointing

      if (_timeEnterSunScan > 0) {
        double periodSinceEnter = ptime - _timeEnterSunScan;
        if (periodSinceEnter >= _params.sun_scan_lockon_period) {
          pointingMode = MODE_SUN_SCAN;
	  if (_params.debug) {
	    time_t now = time(NULL);
	    if (now - _timePrintSunPosn >= 5) {
	      cerr << "Sun posn, el, az: " << sunEl << ", " << sunAz << endl;
	      _timePrintSunPosn = now;
	    }
	  }
        }
      } else {
        _timeEnterSunScan = ptime;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Entering possible SUN_SCAN mode" << endl;
        }
      }

    } else {
      
      if (_timeEnterSunScan != 0) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Exiting SUN_SCAN mode" << endl;
        }
        _timeEnterSunScan = 0;
      }

    } // if (elDiff ...
    
  } // if (pointingMode == MODE_NORMAL ...

  // check for flag file

  if (pointingMode == MODE_NORMAL && _params.save_when_flag_file_exists) {
    if (ta_stat_exists(_params.flag_file_path)) {
      pointingMode = MODE_FLAG_FILE;
    }
  }

  // check for stationary antenna

  _isStationary = !_moving(ptime, el, az);
  if (pointingMode == MODE_FLAG_FILE) {
    if (_isStationary) {
      pointingMode = MODE_STATIONARY;
    }
  }

  return pointingMode;

}
  
/////////////////////////////////
// open a new file

int TsSmartSave::_openNewFile(const IwrfTsPulse &pulse)

{

  // close out old file

  _closeFile();

  // should we save data?

  bool doSave = false;

  if (_currentPointingMode == MODE_SUN_SCAN) {
    if (_params.save_sun_scan_data) {
      doSave = true;
    }
  } else if (_currentPointingMode == MODE_VERT_POINT) {
    if (_params.save_vert_pointing_data) {
      doSave = true;
    }
  } else if (_currentPointingMode == MODE_FLAG_FILE) {
    if (_params.save_when_flag_file_exists) {
      doSave = true;
    }
  } else if (_params.save_normal_scan_data) {
    if (_currentPointingMode == MODE_NORMAL) {
      doSave = true;
    }
    if (_params.normal_ignore_idle_mode) {
      if (_currentScanMode == IWRF_SCAN_MODE_IDLE) {
        if (_params.debug) {
          cerr << "  NOTE: ignoring IDLE scan" << endl;
        }
        doSave = false;
      }
    }
    if (_params.normal_ignore_stationary_antenna) {
      if (_isStationary) {
        if (_params.debug) {
          cerr << "  NOTE: ignoring stationary scan" << endl;
        }
        doSave = false;
      }
    }
  } else if (_currentPointingMode == MODE_STATIONARY) {
    if (_params.save_stationary_data) {
      doSave = true;
    }
  }

  if (!doSave) {
    return -1;
  }

  // get time

  time_t ptime = pulse.getTime();
  int nanoSecs = pulse.getNanoSecs();
  int milliSecs = nanoSecs / 1000000;
  if (milliSecs > 999) {
    milliSecs = 999;
  }

  date_time_t ttime;
  ttime.unix_time = ptime;
  _outputTime = ptime;
  uconvert_from_utime(&ttime);

  // compute antenna pos strings
  
  iwrf_scan_mode_t scanMode = pulse.getScanMode();

  char fixedAngleStr[64];
  char movingAngleStr[64];
  
  if (scanMode == IWRF_SCAN_MODE_RHI ||
      scanMode == IWRF_SCAN_MODE_MANRHI) {
    double el = pulse.getEl();
    if (el < 0) {
      el += 360;
    }
    double az = pulse.getAz();
    if (_params.use_fixed_angle_for_file_name) {
      az = pulse.getFixedAz();
      if (az < -9990) {
        az = 999; // missing
      }
    }
    sprintf(fixedAngleStr, "_%.3d", (int) (az * 10.0 + 0.5));
    sprintf(movingAngleStr, "_%.3d", (int) (el + 0.5));
  } else {
    double az = pulse.getAz();
    if (az < 0) {
      az += 360;
    }
    double el = pulse.getEl();
    if (_params.use_fixed_angle_for_file_name) {
      el = pulse.getFixedEl();
      if (el < -9990) {
        el = 99.9; // missing
      }
    }
    sprintf(fixedAngleStr, "_%.3d", (int) (el * 10.0 + 0.5));
    sprintf(movingAngleStr, "_%.3d", (int) (az + 0.5));
  }

  // compute xmit mode string

  string xmitModeStr;
  if (_params.add_xmit_mode_to_file_name) {
    const IwrfTsInfo &info = pulse.getTsInfo();
    const iwrf_ts_processing_t &tsProc = info.getTsProcessing();
    if (tsProc.prf_mode == IWRF_PRF_MODE_STAGGERED_2_3) {
      xmitModeStr = ".stag23";
    } else if (tsProc.prf_mode == IWRF_PRF_MODE_STAGGERED_3_4) {
      xmitModeStr = ".stag34";
    } else if (tsProc.prf_mode == IWRF_PRF_MODE_STAGGERED_4_5) {
      xmitModeStr = ".stag45";
    } else if (tsProc.xmit_rcv_mode == IWRF_ALT_HV_CO_CROSS ||
               tsProc.xmit_rcv_mode == IWRF_ALT_HV_CO_ONLY ||
               tsProc.xmit_rcv_mode == IWRF_ALT_HV_FIXED_HV) {
      xmitModeStr = ".alt";
    } else if (tsProc.xmit_rcv_mode == IWRF_SIM_HV_FIXED_HV ||
               tsProc.xmit_rcv_mode == IWRF_SIM_HV_SWITCHED_HV) {
      xmitModeStr = ".sim";
    } else if (tsProc.xmit_rcv_mode == IWRF_SINGLE_POL) {
      xmitModeStr = ".single";
    } else if (tsProc.xmit_rcv_mode == IWRF_H_ONLY_FIXED_HV) {
      xmitModeStr = ".honly";
    } else if (tsProc.xmit_rcv_mode == IWRF_V_ONLY_FIXED_HV ||
               tsProc.xmit_rcv_mode == IWRF_SINGLE_POL_V) {
      xmitModeStr = ".vonly";
    }
  }

  // compute scan mode string
  
  string scanModeStr;
  if (_params.add_scan_mode_to_file_name) {
    switch (scanMode) {
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
  }

  if (pulse.antennaTransition()) {
    scanModeStr += "_trans";
  }
    
  // make the output dir

  char subdir[1024];
  _outputDir = _params.normal_output_dir;
  if (_currentPointingMode == MODE_VERT_POINT) {
    _outputDir = _params.vert_pointing_output_dir;
  } else if (_currentPointingMode == MODE_SUN_SCAN) {
    _outputDir = _params.sun_scan_output_dir;
  } else if (_currentPointingMode == MODE_FLAG_FILE) {
    _outputDir = _params.flag_file_output_dir;
  } else if (_currentPointingMode == MODE_STATIONARY) {
    _outputDir = _params.stationary_output_dir;
  }
  
  if (_params.debug) {
    cerr << "Using outputDir: " << _outputDir << endl;
  }
  
  sprintf(subdir, "%s/%.4d%.2d%.2d", _outputDir.c_str(),
          ttime.year, ttime.month, ttime.day);

  if (ta_makedir_recurse(subdir)) {
    int errNum = errno;
    cerr << "ERROR - TsSmartSave" << endl;
    cerr << "  Cannot make output directory: " << subdir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute output path

  string format;
  if (_params.output_format == Params::FORMAT_TSARCHIVE) {
    format = ".tsarchive";
  } else {
    format = ".iwrf_ts";
  }

  string packing;
  if (_params.output_format == Params::FORMAT_TSARCHIVE) {
    packing = "";
  } else if (_params.output_packing == Params::PACKING_FL32) {
    packing = ".fl32";
  } else if (_params.output_packing == Params::PACKING_SCALED_SI16) {
    packing = ".scaled";
  } else if (_params.output_packing == Params::PACKING_DBM_PHASE_SI16) {
    packing = ".dbm_phase";
  } else if (_params.output_packing == Params::PACKING_SIGMET_FL16) {
    packing = ".sigmet";
  }

  char name[1024];
  sprintf(name, "%.4d%.2d%.2d_%.2d%.2d%.2d.%.3d%s%s%s%s%s%s",
          ttime.year, ttime.month, ttime.day,
          ttime.hour, ttime.min, ttime.sec, milliSecs,
	  fixedAngleStr, movingAngleStr,
          scanModeStr.c_str(), xmitModeStr.c_str(),
          packing.c_str(), format.c_str());
  _outputName = name;

  char relPath[1024];
  sprintf(relPath, "%.4d%.2d%.2d/%s",
          ttime.year, ttime.month, ttime.day, name);
  _relPath = relPath;

  char path[1024];
  sprintf(path, "%s/%s", _outputDir.c_str(), relPath);
  
  // open file

  if ((_out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsSmartSave" << endl;
    cerr << "  Cannot open output file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Opened new file: " << _relPath << endl;
  }

  return 0;

}

///////////////////////////////////////////
// close file
//
// Returns 0 if file already open, -1 if not

int TsSmartSave::_closeFile()

{

  // close out old file
  
  if (_out != NULL) {

    fclose(_out);
    _out = NULL;

    if (_params.debug) {
      cerr << "Done with file: " << _relPath << endl;
    }

    // write latest data info file
    
    DsLdataInfo ldata(_outputDir,
		      _params.debug >= Params::DEBUG_VERBOSE);
    ldata.setLatestTime(_outputTime);
    ldata.setRelDataPath(_relPath);
    ldata.setWriter("TsSmartSave");
    ldata.setDataType("iwrf_ts");
    if (ldata.write(_outputTime)) {
      cerr << "ERROR - cannot write LdataInfo" << endl;
      cerr << " outputDir: " << _outputDir << endl;
    }

    if (_params.write_ldata_info_to_proxy_path) {
      DsLdataInfo proxy(_params.ldata_info_proxy_path,
                        _params.debug >= Params::DEBUG_VERBOSE);
      proxy.setLatestTime(_outputTime);
      string relDir;
      Path::stripDir(_params.ldata_info_proxy_path, _outputDir, relDir);
      string relPath(relDir);
      relPath += PATH_DELIM;
      relPath += _relPath;
      proxy.setRelDataPath(relPath);
      proxy.setWriter("TsSmartSave");
      proxy.setDataType("iwrf_ts");
      if (proxy.write(_outputTime)) {
        cerr << "ERROR - cannot write LdataInfo" << endl;
        cerr << " outputDir: " << _params.ldata_info_proxy_path << endl;
      }
    }
   
    return 0;

  }

  return -1;

}
  
//////////////////////////
// is the antenna moving?

bool TsSmartSave::_moving(time_t ptime, double el, double az)
  
{

  int elapsedTime = ptime - _moveCheckTime;
  if (elapsedTime < _params.stationary_lockon_period) {
    return _isMoving;
  }
  
  double maxAngleChange = _params.stationary_max_angle_change;
  if (fabs(az - _moveCheckAz) > maxAngleChange ||
      fabs(el - _moveCheckEl) > maxAngleChange) {
    _isMoving = true;
  } else {
    _isMoving = false;
  }
  
  _moveCheckAz = az;
  _moveCheckEl = el;
  _moveCheckTime = ptime;
  
  return _isMoving;

}

