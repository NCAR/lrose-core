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
// XpolScanControl.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2012
//
///////////////////////////////////////////////////////////////
// XpolScanControl controls the XPOL antenna. It creates PAXI
// files and uploads them to the DRX. From one FMQ it reads
// the data coming from the DRX, to monitor the antenna behavior.
// It then adds the scan information as appropriate, and writes
// the modified data to an output FMQ.
////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <cmath>
#include <cerrno>
#include <signal.h>
#include <sys/wait.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include "XpolScanControl.hh"
#include "Field.hh"

using namespace std;

// Constructor

XpolScanControl::XpolScanControl(int argc, char **argv)
  
{

  isOK = true;
  _paxiFile = NULL;
  _lockFile = NULL;
  _volNum = -1;
  _sweepNum = -1;
  _scanMode = -1;
  _beamTimeSecs = 0;
  _beamTimeDsecs = 0;
  _prevBeamTimeDsecs = -1;
  _deltaTimeDsecs = 0;
  _transition = false;
  _az = 0;
  _el = 0;
  _prevAz = -1;
  _prevEl = -1;
  _azDist = 0;
  _elDist = 0;
  _azRate = 0;
  _elRate = 0;
  _prevAzRate = 0;
  _prevElRate = 0;
  _fixedEl = 0;
  _fixedAz = 0;
  _drxConfId = -1;
  
  // set programe name
  
  _progName = "XpolScanControl";
  
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
  if (_checkParams()) {
    isOK = false;
    return;
  }

  // check no other instance is running

  if (_createLockFile()) {
    isOK = false;
    return;
  }

  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }
  
}

// destructor

XpolScanControl::~XpolScanControl()

{

  // unregister process

  PMU_auto_unregister();

  if (_lockFile) {
    fclose(_lockFile);
  }

}

///////////////////////////////////////
// stop all activity - called on close

void XpolScanControl::stopActivity()

{

  _stopAntenna(0, false);
  _killRemainingChildren();

}

//////////////////////////////////////////////////
// check the parameters

int XpolScanControl::_checkParams()
{

  int iret = 0;

  // check the scan parameter - for sweep number

  for (int iscan = 0; iscan < _params.scan_sequence_n; iscan++) {

    const Params::scan_params_t &scan = _params._scan_sequence[iscan];
    
    switch (scan.scan_mode) {
      case Params::SCAN_MODE_SURVEILLANCE: {
        double elSpan = scan.end_el_deg - scan.start_el_deg;
        int nSweeps = (int) (elSpan / scan.delta_el_deg + 1.5);
        if (nSweeps != scan.n_sweeps) {
          cerr << "ERROR - XpolScanControl::_checkParams()" << endl;
          cerr << "  Surveillance scan, index: " << iscan << endl;
          cerr << "  start_el_deg: " << scan.start_el_deg << endl;
          cerr << "  end_el_deg: " << scan.end_el_deg << endl;
          cerr << "  delta_el_deg: " << scan.delta_el_deg << endl;
          cerr << "  Incorrect n_sweeps parameter: " << scan.n_sweeps << endl;
          cerr << "  Should be: " << nSweeps << endl;
          iret = -1;
        }
        break;
      }
      case Params::SCAN_MODE_SECTOR: {
        double elSpan = scan.end_el_deg - scan.start_el_deg;
        int nSweeps = (int) (elSpan / scan.delta_el_deg + 1.5);
        if (nSweeps != scan.n_sweeps) {
          cerr << "ERROR - XpolScanControl::_checkParams()" << endl;
          cerr << "  Sector scan, index: " << iscan << endl;
          cerr << "  start_el_deg: " << scan.start_el_deg << endl;
          cerr << "  end_el_deg: " << scan.end_el_deg << endl;
          cerr << "  delta_el_deg: " << scan.delta_el_deg << endl;
          cerr << "  Incorrect n_sweeps parameter: " << scan.n_sweeps << endl;
          cerr << "  Should be: " << nSweeps << endl;
          iret = -1;
        }
        break;
      }
      case Params::SCAN_MODE_RHI: {
        double azSpan = scan.end_az_deg - scan.start_az_deg;
        int nSweeps = (int) (azSpan / scan.delta_az_deg + 1.5);
        if (nSweeps != scan.n_sweeps) {
          cerr << "ERROR - XpolScanControl::_checkParams()" << endl;
          cerr << "  RHI scan, index: " << iscan << endl;
          cerr << "  start_az_deg: " << scan.start_az_deg << endl;
          cerr << "  end_az_deg: " << scan.end_az_deg << endl;
          cerr << "  delta_az_deg: " << scan.delta_az_deg << endl;
          cerr << "  Incorrect n_sweeps parameter: " << scan.n_sweeps << endl;
          cerr << "  Should be: " << nSweeps << endl;
          iret = -1;
        }
        break;
      }
      case Params::SCAN_MODE_SUNSCAN: {
        double elSpan = scan.end_el_deg - scan.start_el_deg;
        int nSweeps = (int) (elSpan / scan.delta_el_deg + 1.5);
        if (nSweeps != scan.n_sweeps) {
          cerr << "ERROR - XpolScanControl::_checkParams()" << endl;
          cerr << "  Sector scan, index: " << iscan << endl;
          cerr << "  start_el_deg: " << scan.start_el_deg << endl;
          cerr << "  end_el_deg: " << scan.end_el_deg << endl;
          cerr << "  delta_el_deg: " << scan.delta_el_deg << endl;
          cerr << "  Incorrect n_sweeps parameter: " << scan.n_sweeps << endl;
          cerr << "  Should be: " << nSweeps << endl;
          iret = -1;
        }
        break;
      }
      case Params::SCAN_MODE_SUNSCAN_RHI: {
        double azSpan = _getAbsAzDiff(scan.end_az_deg, scan.start_az_deg);
        int nSweeps = (int) (azSpan / scan.delta_az_deg + 1.5);
        if (nSweeps != scan.n_sweeps) {
          cerr << "ERROR - XpolScanControl::_checkParams()" << endl;
          cerr << "  Sector scan, index: " << iscan << endl;
          cerr << "  start_az_deg: " << scan.start_az_deg << endl;
          cerr << "  end_az_deg: " << scan.end_az_deg << endl;
          cerr << "  delta_az_deg: " << scan.delta_az_deg << endl;
          cerr << "  Incorrect n_sweeps parameter: " << scan.n_sweeps << endl;
          cerr << "  Should be: " << nSweeps << endl;
          iret = -1;
        }
        break;
      }
      default: {}
    }
    
  } // iscan

  // check the drx configurations

  if (_params.drx_conf_n < 1) {
    cerr << "ERROR - XpolScanControl::_checkParams()" << endl;
    cerr << "  drx_conf array has 0 length" << endl;
    cerr << "  You must have at least 1 entry" << endl;
    iret = -1;
  }

  for (int iscan = 0; iscan < _params.scan_sequence_n; iscan++) {
    const Params::scan_params_t &scan = _params._scan_sequence[iscan];
    int drxConfId = scan.drx_conf_id;
    bool confFound = false;
    for (int ii = 0; ii < _params.drx_conf_n; ii++) {
      const Params::drx_conf_t &conf = _params._drx_conf[ii];
      if (conf.conf_id == scan.drx_conf_id) {
        confFound = true;
        break;
      }
    }
    if (!confFound) {
      cerr << "ERROR - XpolScanControl::_checkParams()" << endl;
      cerr << "  Scan, index: " << iscan << endl;
      cerr << "  Cannot find drx_conf_id to match: " << drxConfId << endl;
      cerr << "  Edit the drx_conf section" << endl;
      iret = -1;
    }
  }

  return iret;

}

///////////////////////////////////////////////////////////
// Create a lock file to ensure only a single instance runs
// Returns 0 on success, -1 on failure

int XpolScanControl::_createLockFile()

{

  char lockFilePath[1024];
  sprintf(lockFilePath, "/tmp/%s.%s", _progName.c_str(), "lock");
  
  _lockFile = ta_create_lock_file(lockFilePath);
  
  if (_lockFile == NULL) {
    cerr << "ERROR - " << _progName << "::_createLockFile" << endl;
    cerr << "  Cannot create lock file '" << lockFilePath << "'" << endl;
    cerr << "  An instance of " << _progName
	 << "  is probably aleady running" << endl;
    return -1;
  }

  return 0;

}


//////////////////////////////////////////////////
// Outer run method

int XpolScanControl::Run()
{
  
  PMU_auto_register("Run");
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running XpolScanControl - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running XpolScanControl - debug mode" << endl;
  }

  int iret = 0;

  while (true) {
    
    PMU_auto_register("Run");

    if (_init()) {
      iret = -1;
      break;
    }

    if (_run()) {
      umsleep(1000);
      iret = -1;
    }
    
  } // while

  return iret;

}

//////////////////////////////////////////////////
// inner run method

int XpolScanControl::_run()
{

  // get a few beams to initialize rates etc

  for (int ii = 0; ii < 5; ii++) {
    if (_getNextBeam("Initializing")) {
      return -1;
    }
  }

  // loop through the scans, forever

  while (true) {

    for (int iscan = 0; iscan < _params.scan_sequence_n; iscan++) {
      
      const Params::scan_params_t &scan = _params._scan_sequence[iscan];

      // set the processor params if the request proc num has changed
      
      if (scan.drx_conf_id != _drxConfId) {
        _setDrxProc(scan.drx_conf_id);
        _drxConfId = scan.drx_conf_id;
      }

      switch (scan.scan_mode) {
        case Params::SCAN_MODE_POINTING:
          if(_runPointing(scan.start_az_deg,
                          scan.start_el_deg,
                          scan.az_rate_deg_per_sec,
                          scan.el_rate_deg_per_sec,
                          scan.nsecs_wait,
                          true)) {
            return -1;
          }
          break;
        case Params::SCAN_MODE_SURVEILLANCE:
          if(_runSurveillance(scan.start_az_deg,
                              scan.start_el_deg,
                              scan.end_el_deg,
                              scan.delta_el_deg,
                              scan.az_rate_deg_per_sec,
                              scan.el_rate_deg_per_sec,
                              scan.transition_tolerance_deg)) {
            return -1;
          }
          break;
        case Params::SCAN_MODE_SECTOR:
          if(_runSector(DS_RADAR_SECTOR_MODE,
                        "Sector",
                        scan.start_az_deg,
                        scan.end_az_deg,
                        scan.start_el_deg,
                        scan.end_el_deg,
                        scan.delta_el_deg,
                        scan.az_rate_deg_per_sec,
                        scan.el_rate_deg_per_sec,
                        scan.transition_tolerance_deg)) {
            return -1;
          }
          break;
        case Params::SCAN_MODE_RHI:
          if(_runRhi(scan.start_az_deg,
                     scan.end_az_deg,
                     scan.delta_az_deg,
                     scan.start_el_deg,
                     scan.end_el_deg,
                     scan.az_rate_deg_per_sec,
                     scan.el_rate_deg_per_sec,
                     scan.transition_tolerance_deg)) {
            return -1;
          }
          break;
        case Params::SCAN_MODE_VERTICAL_POINTING:
          if(_runVertPoint(scan.start_az_deg,
                           scan.start_el_deg,
                           scan.az_rate_deg_per_sec,
                           scan.el_rate_deg_per_sec,
                           scan.n_sweeps,
                           scan.transition_tolerance_deg)) {
            return -1;
          }
          break;
        case Params::SCAN_MODE_IDLE:
          if(_stopAntenna(scan.nsecs_wait, true)) {
            return -1;
          }
          break;
        case Params::SCAN_MODE_SUNSCAN:
          if(_runSunscanConstEl(scan.start_az_deg,
                                scan.end_az_deg,
                                scan.start_el_deg,
                                scan.end_el_deg,
                                scan.delta_el_deg,
                                scan.az_rate_deg_per_sec,
                                scan.el_rate_deg_per_sec)) {
            return -1;
          }
          break;
        case Params::SCAN_MODE_SUNSCAN_RHI:
          if(_runSunscanConstAz(scan.start_az_deg,
                                scan.end_az_deg,
                                scan.start_el_deg,
                                scan.end_el_deg,
                                scan.delta_az_deg,
                                scan.az_rate_deg_per_sec,
                                scan.el_rate_deg_per_sec)) {
            return -1;
          }
          break;
        default: {}
      }

    } // iscan

  } // while
  
  return 0;

}

//////////////////////////////////////////////////
// initialize
// returns 0 on success, -1 on failure

int XpolScanControl::_init()
{

  _inputContents = 0;
  
  // close as needed

  _inputQueue.closeMsgQueue();
  _outputQueue.closeMsgQueue();

  // create the input FMQs
  
  Fmq::openPosition startPos = Fmq::END;
  if (_params.start_reading_at_fmq_start) {
    startPos = Fmq::START;
  }

  if (_inputQueue.init(_params.input_fmq_path,
                        _progName.c_str(),
                        _params.debug,
                        DsFmq::BLOCKING_READ_WRITE, startPos)) {
    cerr << "ERROR - XpolScanControl::_init()" << endl;
    cerr << "  Cannot not initialize input queue: "
         << _params.input_fmq_path << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "-->> Opened input queue: "
         << _params.input_fmq_path << endl;
  }
  
  return 0;

}
  
///////////////////////////////////////////////////
// get next beam from input queue
// returns 0 on success, -1 on failure

int XpolScanControl::_getNextBeam(const string &modeStr)

{

  while (true) {

    PMU_auto_register("Reading input fmq");
  
    // get next message
    
    if (_inputQueue.getDsMsg(_inputMsg, &_inputContents)) {
      return -1;
    }
    
    // success on getting beam?
    
    if ((_inputContents & DsRadarMsg::RADAR_BEAM) &&
        _inputMsg.allParamsSet()) {

      DsRadarBeam &radarBeam = _inputMsg.getRadarBeam();
      radarBeam.azimuth = _getCorrectedAz(radarBeam.azimuth);
      
      _prevBeamTimeDsecs = _beamTimeDsecs;
      _beamTimeSecs = radarBeam.dataTime;
      _beamTimeDsecs = radarBeam.getDoubleTime();
      _deltaTimeDsecs = _beamTimeDsecs - _prevBeamTimeDsecs;

      _prevEl = _el;
      _prevAz = _az;
      _el = radarBeam.elevation;
      _az = radarBeam.azimuth;
      
      _azDist = _getAzDiff(_az, _prevAz);
      _elDist = _el - _prevEl;

      _prevAzRate = _azRate;
      _prevElRate = _elRate;
      _azRate = _azDist / _deltaTimeDsecs;
      _elRate = _elDist / _deltaTimeDsecs;

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "-->> got beam from input queue, mode: " << modeStr << endl;
        _printSummary(_inputMsg);
      }

      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "  _el: " << _el << endl;
        cerr << "  _az: " << _az << endl;
        cerr << "  _prevBeamTimeDsecs: " << _prevBeamTimeDsecs << endl;
        cerr << "  _beamTimeSecs: " << _beamTimeSecs << endl;
        cerr << "  _beamTimeDsecs: " << _beamTimeDsecs << endl;
        cerr << "  _deltaTimeDsecs: " << _deltaTimeDsecs << endl;
        cerr << "  _prevEl: " << _prevEl << endl;
        cerr << "  _prevAz: " << _prevAz << endl;
        cerr << "  _azDist: " << _azDist << endl;
        cerr << "  _elDist: " << _elDist << endl;
        cerr << "  _azRate: " << _azRate << endl;
        cerr << "  _elRate: " << _elRate << endl;
      }
      
      return 0;

    }
    
    // save radar params
    
    if (_inputMsg.allParamsSet()) {
      if ((_inputContents & DsRadarMsg::RADAR_PARAMS) ||
          (_inputContents & DsRadarMsg::FIELD_PARAMS)) {
        if (_writeParams()) {
          return -1;
        }
      }
    }

    // copy calibration and flags to output queue
    
    if (_inputContents & DsRadarMsg::RADAR_CALIB) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "-->> read calibration" << endl;
      }
      if (_writeCalib(_inputMsg.getRadarCalib())) {
        return -1;
      }
    }
    
    if (_inputContents & DsRadarMsg::RADAR_FLAGS) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "-->> got flags from queue 1" << endl;
      }
      if (_writeFlags(_inputMsg.getRadarFlags())) {
        return -1;
      }
    }

  } // while
    
  return -1;

}

/////////////////////////////////////////////////////
// set the drx processing parameters

int XpolScanControl::_setDrxProc(int drxConfId)

{

  // find the correct configuration

  int index = -1;
  for (int ii = 0; ii < _params.drx_conf_n; ii++) {
    if (_params._drx_conf[ii].conf_id == drxConfId) {
      index = ii;
      break;
    }
  }

  if (index < 0) {
    cerr << "ERROR - XpolScanControl::_setDrxProc()" << endl;
    cerr << "  Cannot find drx_conf_id to match: " << drxConfId << endl;
    cerr << "  Edit the drx_conf section" << endl;
    return -1;
  }

  const Params::drx_conf_t &conf = _params._drx_conf[index];

  // write paxi file

  // write paxi file
  
  // Name: AzRaster
  // Description: Execute azimuth raster scan from given position
  //   (or current position if omitted) at given speed,
  //   covering given region with given elevation increment.
  // Required params: azVel, elVel, azSpan, elSpan, elInc
  // Optional params: az, el
  // Example:
  //   To execute an azimuth raster scan covering the az, el region
  //   {azMin=20, azMax=60, elMin=10, elMax=30}
  //      in elevation increments of 2 degrees,
  //   at az, el speeds of 5 and 1, respectively:
  //     AzRaster az=20 el=10 azSpan=40 elSpan=20 elInc=2 azVel=5 elVel=1;
  //     NOTE: max ax, el of {60, 30} = {20, 10} (starting) + {40, 20} (span)

  if (_openPaxiFile()) {
    return -1;
  }

  fprintf(_paxiFile, "# DRX processor configuration\n");
  fprintf(_paxiFile, "SetConfig\n");
  fprintf(_paxiFile, "  server_state=1 # run \n");
  if (conf.proc_mode == Params::PROC_MODE_DUAL_PULSE_PAIR ||
      conf.proc_mode == Params::PROC_MODE_DUAL_PULSE_PAIR_SUM_POWERS) {
    fprintf(_paxiFile, "  server_mode=1 # dual pulse pair \n");
  } else {
    fprintf(_paxiFile, "  server_mode=0 # pulse pair \n");
  }
  if (conf.proc_mode == Params::PROC_MODE_PULSE_PAIR_SUM_POWERS ||
      conf.proc_mode == Params::PROC_MODE_DUAL_PULSE_PAIR_SUM_POWERS) {
    fprintf(_paxiFile, "  sum_powers=1\n");
  } else {
    fprintf(_paxiFile, "  sum_powers=0\n");
  }
  fprintf(_paxiFile, "  pri_1_usec=%g\n", conf.pri_1_usec);
  fprintf(_paxiFile, "  pri_2_usec=%g\n", conf.pri_2_usec);
  fprintf(_paxiFile, "  group_interval_usec=%g\n", conf.group_interval_usec);
  fprintf(_paxiFile, "  n_gates=%d\n", conf.n_gates);
  fprintf(_paxiFile, "  clutter_n_ave=%d\n", conf.clutter_n_ave);
  fprintf(_paxiFile, "  post_n_ave=%d\n", conf.post_n_ave);
  if (conf.use_clutter_filter) {
    fprintf(_paxiFile, "  use_clutter_filter=1\n");
  } else {
    fprintf(_paxiFile, "  use_clutter_filter=0\n");
  }

  switch (conf.range_res) {
    case Params::RANGE_RES_15_m:
      fprintf(_paxiFile, "  range_gate_spacing_m=15\n");
      break;
    case Params::RANGE_RES_30_m:
      fprintf(_paxiFile, "  range_gate_spacing_m=30\n");
      break;
    case Params::RANGE_RES_75_m:
      fprintf(_paxiFile, "  range_gate_spacing_m=75\n");
      break;
    case Params::RANGE_RES_105_m:
      fprintf(_paxiFile, "  range_gate_spacing_m=105\n");
      break;
    case Params::RANGE_RES_150_m:
      fprintf(_paxiFile, "  range_gate_spacing_m=150\n");
      break;
  }

  switch (conf.pulse_len) {
    case Params::PULSE_LEN_7pt5_m:
      fprintf(_paxiFile, "  pulse_len_m=7.5\n");
      break;
    case Params::PULSE_LEN_15_m:
      fprintf(_paxiFile, "  pulse_len_m=15\n");
      break;
    case Params::PULSE_LEN_30_m:
      fprintf(_paxiFile, "  pulse_len_m=30\n");
      break;
    case Params::PULSE_LEN_45_m:
      fprintf(_paxiFile, "  pulse_len_m=45\n");
      break;
    case Params::PULSE_LEN_75_m:
      fprintf(_paxiFile, "  pulse_len_m=75\n");
      break;
  }

  switch (conf.filter_bw) {
    case Params::FILTER_BW_1_mhz:
      fprintf(_paxiFile, "  filter_bw_mhz=1\n");
      break;
    case Params::FILTER_BW_1pt5_mhz:
      fprintf(_paxiFile, "  filter_bw_mhz=1.5\n");
      break;
    case Params::FILTER_BW_2_mhz:
      fprintf(_paxiFile, "  filter_bw_mhz=2\n");
      break;
    case Params::FILTER_BW_5_mhz:
      fprintf(_paxiFile, "  filter_bw_mhz=5\n");
      break;
    case Params::FILTER_BW_10_mhz:
      fprintf(_paxiFile, "  filter_bw_mhz=10\n");
      break;
    case Params::FILTER_BW_20_mhz:
      fprintf(_paxiFile, "  filter_bw_mhz=20\n");
      break;
  }

  fprintf(_paxiFile, ";\n");

  _closePaxiFile();

  // upload paxi file to server
  
  _uploadPaxiFile();
  
  return 0;

}

/////////////////////////////////////////////////////
// stop for a given time period, at current location

int XpolScanControl::_stopAntenna(double nsecsWait,
                                  bool dataActive)
{

  if (_params.debug) {
    cerr << "====>> _stopAntenna <<====" << endl;
    cerr << "  nsecsWait: " << nsecsWait << endl;
    cerr << "  dataActive: " << (char *) (dataActive?"Y":"N") << endl;
    cerr << "==========================" << endl;
  }

  _transition = false;

  // write paxi file

  // Name: PedStop
  // Description: Stop pedestal motion.
  // Required params: (none)
  // Optional params: az, el (boolean values)
  // Examples:
  //   To stop pedestal motion along the elevation axis only:
  //     PedStop az=false el=true;
  //   To stop pedestal motion along both axes:
  //     PedStop az=true el=true;
    
  if (_openPaxiFile()) {
    return -1;
  }
  fprintf(_paxiFile, "# Stop command\n");
  fprintf(_paxiFile, "PedStop az=true el=true;\n");
  _closePaxiFile();

  // upload paxi file to server
  
  _uploadPaxiFile();

  // set scan params

  if (dataActive) {
    _fixedAz = -99;
    _fixedEl = -99;
    _scanMode = DS_RADAR_IDLE_MODE;
    _volNum++;
    _sweepNum = 0;
    _writeParams();
    _writeStartOfVolume();
    _writeStartOfSweep();
  }
  
  // read in beam data, watch progress
  
  double startTime = _beamTimeDsecs;
  
  while (true) {
    
    PMU_auto_register("_stopAntenna");
    
    // read input queue
    
    if (_getNextBeam("Idle")) {
      return -1;
    }
    
    // check for wait time
    
    double now = _beamTimeDsecs;
    double elapsedTime = now - startTime;
    double nsecsLeft = nsecsWait - elapsedTime;
    if (elapsedTime > nsecsWait) {
      if (dataActive) {
        // end of volume
        _writeEndOfSweep();
        _writeEndOfVolume();
      }
      return 0;
    } else {
      if (dataActive) {
        // pass beam through
        if (_writeBeam()) {
          return -1;
        }
      }
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "------>> stopAntenna, nsecsLeft: " << nsecsLeft << endl;
      }
    }
    
  } // while
  
  return -1;

}

//////////////////////////////////////////////////
// run a pointing scan

int XpolScanControl::_runPointing(double azPoint,
                                  double elPoint,
                                  double azVel,
                                  double elVel,
                                  double nsecsWait,
                                  bool dataActive)
{

  if (_params.debug) {
    cerr << "====>> _runPointing <<====" << endl;
    cerr << "  az: " << _az << endl;
    cerr << "  el: " << _el << endl;
    cerr << "  azRate: " << _azRate << endl;
    cerr << "  elRate: " << _elRate << endl;
    cerr << "  azPoint: " << azPoint << endl;
    cerr << "  elPoint: " << elPoint << endl;
    cerr << "  requested azVel: " << azVel << endl;
    cerr << "  requested elVel: " << elVel << endl;
    cerr << "  nsecsWait: " << nsecsWait << endl;
    cerr << "  dataActive: " << (char *) (dataActive?"Y":"N") << endl;
  }

  _transition = false;
  if (dataActive) {
    _fixedAz = azPoint;
    _fixedEl = elPoint;
    _scanMode = DS_RADAR_POINTING_MODE;
    _writeParams();
  }

  // check angle errors, constrain velocity accordingly

  double azError = _getAbsAzDiff(_az, azPoint);
  double elError = fabs(_el - elPoint);
  double maxAzVel = azError / 2.0; // get there in 2 secs
  double maxElVel = elError / 2.0;
  azVel = MIN(azVel, maxAzVel);
  elVel = MIN(elVel, maxElVel);
  azVel = MAX(azVel, 5.0);
  elVel = MAX(elVel, 5.0);

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "  azError: " << azError << endl;
    cerr << "  elError: " << elError << endl;
    cerr << "  maxAzVel: " << maxAzVel << endl;
    cerr << "  maxElVel: " << maxElVel << endl;
    cerr << "  computed azVel: " << azVel << endl;
    cerr << "  computed elVel: " << elVel << endl;
    cerr << "==========================" << endl;
  }

  // write paxi file

  // Name: Point
  // Description: Move to given point at given velocity via shortest path. 
  //     Movement is delayed until the pedestal is stationary if the optional 
  //     "settle=true" parameter is specified.
  // Required params: (none - if either az or el is omitted,
  //                  no movement happens along that axis)
  // Optional params: az, el, azVel, elVel, settle
  // Example: 
  //   To move the pedestal to the az, el position of {90, 100} at a speed of 
  //   10 degrees/sec along azimuth, and the default pointing speed along 
  //   elevation (which is usually the pedestal's max speed):
  //     Point az=90 el=100 azVel=10;

  if (_openPaxiFile()) {
    return -1;
  }
  fprintf(_paxiFile, "# Pointing command\n");
  fprintf(_paxiFile, "Point az=%g el=%g azVel=%g elVel=%g;\n",
          _getRawAz(azPoint), elPoint, azVel, elVel);
  _closePaxiFile();

  // upload paxi file to server
  
  _uploadPaxiFile();
  
  // read in beam data, watch progress
  
  double startTime = 0;
  int stoppedCount = 0;

  while (true) {
    
    PMU_auto_register("_runPointing");
    
    // read input queue
    
    if (_getNextBeam("Pointing")) {
      return -1;
    }
    
    // how far to go?

    azError = _getAbsAzDiff(_az, azPoint);
    elError = fabs(_el - elPoint);
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "--->> _runPointing, az, el, azError, elError, stoppedCount: "
           << _az << ", "
           << _el << ", "
           << azError << ", "
           << elError << ", "
           << stoppedCount << endl;
    }

    // check we are close enough
    
    if (startTime == 0) {

      // check for stopped condition
      
      if (_isStopped()) {
        stoppedCount++;
      } else {
        stoppedCount = 0;
      }

      // are we close
      bool isClose = false;
      if (azError <= _params.requested_angle_tolerance && 
          elError <= _params.requested_angle_tolerance) {
        isClose = true;
      }
      bool isClosish = false;
      if (azError <= _params.requested_angle_tolerance * 10 && 
          elError <= _params.requested_angle_tolerance * 10) {
        isClosish = true;
      }
      if ((isClose && stoppedCount > 5) ||
          (isClosish && stoppedCount > 20)) {
        // pointing established
        // start of new volume
        startTime = _beamTimeDsecs;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "--->> vol start time: " << DateTime::strm(startTime) << endl;
        }
        if (dataActive) {
          _volNum++;
          _sweepNum = 0;
          _writeStartOfVolume();
          _writeStartOfSweep();
        }
      } else {
        // not there yet - continue to read
        continue;
      }
    }
    
    // check for wait time

    double now = _beamTimeDsecs;
    double elapsedTime = now - startTime;
    double nsecsLeft = nsecsWait - elapsedTime;
    if (elapsedTime > nsecsWait) {
      if (dataActive) {
        // end of volume
        _writeEndOfSweep();
        _writeEndOfVolume();
      }
      return 0;
    } else {
      if (dataActive) {
        // pass beam through
        if (_writeBeam()) {
          return -1;
        }
      }
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "------>> pointing, nsecsLeft: " << nsecsLeft << endl;
      }
    }
    
  } // while
  
  return -1;

}

//////////////////////////////////////////////////
// run a surveillance scan

int XpolScanControl::_runSurveillance(double startAz,
                                      double startEl,
                                      double endEl,
                                      double deltaEl,
                                      double azVel,
                                      double elVel,
                                      double transitionTolerance)
{

  if (_params.debug) {
    cerr << "====>> _runSurveillance <<====" << endl;
    cerr << "  startAz: " << startAz << endl;
    cerr << "  startEl: " << startEl << endl;
    cerr << "  endEl: " << endEl << endl;
    cerr << "  deltaEl: " << deltaEl << endl;
    cerr << "  azVel: " << azVel << endl;
    cerr << "  elVel: " << elVel << endl;
    cerr << "==============================" << endl;
  }

  // first point to startAz, startEl

  if (_runPointing(startAz, startEl,
                   _params.default_antenna_speed,
                   _params.default_antenna_speed,
                   0, false)) {
    cerr << "ERROR - XpolScanControl::_runSurveillance" << endl;
    return -1;
  }

  // set scan type

  _scanMode = DS_RADAR_SURVEILLANCE_MODE;
  _writeParams();

  // start of volume

  _transition = false;
  _volNum++;
  _sweepNum = 0;
  _writeStartOfVolume();
  _writeStartOfSweep();

  // write paxi file
  
  // Name: Volume
  // Description: Execute volume scan from given position (or current position
  //     if omitted) at given speed,
  //     covering given region with given elevation increment.
  // Required params: azVel, elVel, elSpan, elInc
  // Optional params: el
  // Example:
  //   To execute a volume scan covering the
  //   elevation range {elMin=10, elMax=90}
  //   in increments of 5 degrees, at az (continual spinning), el (stepping)
  //   speeds of 15 and 3, respectively:
  //     Volume el=10 elSpan=80 elInc=5 azVel=15 elVel=3;
  //     NOTE: max el of 90 = 10 (starting) + 80 (span)
  
  double elSpan = endEl - startEl;
  if (_openPaxiFile()) {
    return -1;
  }
  fprintf(_paxiFile, "# Surveillance command\n");
  fprintf(_paxiFile, "Loop count=2 name=Volume el=%g elSpan=%g elInc=%g azVel=%g elVel=%g;\n",
          startEl, elSpan, deltaEl, azVel, elVel);
  _closePaxiFile();

  // upload paxi file to server
  
  _uploadPaxiFile();
  
  // initialize

  double prevEl = startEl - deltaEl;
  double thisEl = startEl;
  double nextEl = thisEl + deltaEl;
  _fixedEl = thisEl;
  
  int nSweeps = (int) (elSpan / deltaEl + 1.5);
  if (_params.debug) {
    cerr << "Surveillance nSweeps: " << nSweeps << endl;
  }

  // read in beam data, watch progress

  while (true) {
    
    PMU_auto_register("_runSurveillance");
    
    // read input queue
    
    if (_getNextBeam("Surveillance")) {
      return -1;
    }
    
    double prevElError = fabs(_el - prevEl);
    double thisElError = fabs(_el - thisEl);
    double nextElError = fabs(_el - nextEl);

    // check for transition

    if (thisElError > transitionTolerance ||
        fabs(_azRate) < azVel * 0.25) {
      _transition = true;
    } else {
      _transition = false;
    }


    if (nextElError < thisElError) {

      //if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "--->> _runSurveillance, end of sweep" << endl;
        cerr << "  thisElError, nextElError: "
             << thisElError << ", " << nextElError << endl;
        //}
      
      // new sweep

      prevEl = thisEl;
      thisEl = nextEl;
      nextEl += deltaEl;

      _writeEndOfSweep();
      _sweepNum++;
      _fixedEl = thisEl;
      _writeStartOfSweep();

    } else if (prevElError < thisElError) {

      // if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "--->> _runSurveillance, end of volume" << endl;
        cerr << "  thisElError, prevElError: "
             << thisElError << ", " << prevElError << endl;
        // }
      
      // end of volume
      
      _writeEndOfSweep();
      _writeEndOfVolume();

      return 0;

    } else {
        cerr << "--->> _runSurveillance, dropped through sweep and volume test" << endl;
    }
    
    // pass beam through
    if (_writeBeam()) {
      return -1;
    }

  } // while
  
  return -1;

}

//////////////////////////////////////////////////
// run a sector scan

int XpolScanControl::_runSector(int mode,
                                const string &modeStr,
                                double startAz,
                                double endAz,
                                double startEl,
                                double endEl,
                                double deltaEl,
                                double azVel,
                                double elVel,
                                double transitionTolerance)
{

  if (_params.debug) {
    cerr << "====>> _runSector <<====" << endl;
    cerr << "  mode: " << modeStr << endl;
    cerr << "  startAz: " << startAz << endl;
    cerr << "  endAz: " << endAz << endl;
    cerr << "  startEl: " << startEl << endl;
    cerr << "  endEl: " << endEl << endl;
    cerr << "  deltaEl: " << deltaEl << endl;
    cerr << "  azVel: " << azVel << endl;
    cerr << "  elVel: " << elVel << endl;
    cerr << "==============================" << endl;
  }

  // widen the sector on each end

  double azWidenDeg = azVel * _params.sector_widen_secs;
  double scanStartAz = _getNormAz(startAz - azWidenDeg);
  double scanEndAz = _getNormAz(endAz + azWidenDeg);
  double azSpan = scanEndAz - scanStartAz;
  if (azSpan < 0) {
    azSpan += 360.0;
  }
  double elSpan = endEl - startEl;

  // first point to startAz, startEl
  
  if (_runPointing(scanStartAz, startEl, 
                   _params.default_antenna_speed,
                   _params.default_antenna_speed, 0, false)) {
    cerr << "ERROR - XpolScanControl::_runSector" << endl;
    return -1;
  }

  // set scan type

  _scanMode = mode;
  _writeParams();

  // start of volume

  _transition = false;
  _volNum++;
  _sweepNum = 0;
  _writeStartOfVolume();
  _writeStartOfSweep();

  // write paxi file
  
  // Name: AzRaster
  // Description: Execute azimuth raster scan from given position
  //   (or current position if omitted) at given speed,
  //   covering given region with given elevation increment.
  // Required params: azVel, elVel, azSpan, elSpan, elInc
  // Optional params: az, el
  // Example:
  //   To execute an azimuth raster scan covering the az, el region
  //   {azMin=20, azMax=60, elMin=10, elMax=30}
  //      in elevation increments of 2 degrees,
  //   at az, el speeds of 5 and 1, respectively:
  //     AzRaster az=20 el=10 azSpan=40 elSpan=20 elInc=2 azVel=5 elVel=1;
  //     NOTE: max ax, el of {60, 30} = {20, 10} (starting) + {40, 20} (span)

  if (_openPaxiFile()) {
    return -1;
  }
  fprintf(_paxiFile, "# Sector command\n");
  fprintf(_paxiFile,
          "AzRaster az=%g el=%g azSpan=%g elSpan=%g "
          "elInc=%g azVel=%g elVel=%g;\n",
          _getRawAz(scanStartAz), startEl,
          azSpan, elSpan, deltaEl, azVel, elVel);
  _closePaxiFile();

  // upload paxi file to server
  
  _uploadPaxiFile();
  
  // initialize

  double prevEl = startEl - deltaEl;
  double thisEl = startEl;
  double nextEl = thisEl + deltaEl;
  _fixedEl = thisEl;
  int stoppedCount = -1;

  int nSweeps = (int) (elSpan / deltaEl + 1.5);
  if (_params.debug) {
    cerr << "Sector nSweeps: " << nSweeps << endl;
  }

  // does the specified sector cross north?

  bool crossesNorth = false;
  if (endAz < startAz) {
    crossesNorth = true;
  }
  
  // read in beam data, watch progress

  while (true) {
    
    PMU_auto_register("_runSector");
    
    // read input queue
    
    if (_getNextBeam("Sector")) {
      return -1;
    }
    
    double prevElError = fabs(_el - prevEl);
    double thisElError = fabs(_el - thisEl);
    double nextElError = fabs(_el - nextEl);

    // check for transition

    _transition = false;
    if (thisElError > transitionTolerance) {
      _transition = true;
    }
    if (crossesNorth) {
      if (_az > endAz && _az < startAz) {
        _transition = true;
      }
    } else {
      if (_az > endAz || _az < startAz) {
        _transition = true;
      }
    }

    // check for stopped condition
    
    if (_isStopped()) {
      if (stoppedCount >= 0){
        stoppedCount++;
      }
    } else {
      stoppedCount = 0;
    }

    // check if we are done
    bool done = false;
    if (_sweepNum == nSweeps - 1) {
      double azError = 0;
      if (nSweeps % 2 == 0) {
        // even number of sweeps, end at scanStartAz
        azError = _getAbsAzDiff(_az, scanStartAz);
      } else {
        // odd number of sweeps, end at scanEndAz
        azError = _getAbsAzDiff(_az, scanEndAz);
      }
      if (azError < _params.requested_angle_tolerance) {
        done = true;
      }
    }
    if (prevElError < thisElError && _sweepNum > 0) {
      done = true;
    }
    if (stoppedCount > 100) {
      done = true;
    }

    if (done) {

      if (_params.debug) {
        cerr << "--->> _runSector, end of volume" << endl;
        cerr << "  thisElError, prevElError: "
             << thisElError << ", " << prevElError << endl;
        cerr << "  stoppedCount: " << stoppedCount << endl;
        cerr << "  done: " << (char *) (done?"Y":"N") << endl;
      }
      
      // end of volume
      
      _writeEndOfSweep();
      _writeEndOfVolume();

      return 0;
      
    } else if (nextElError < thisElError) {

      if (_params.debug) {
        cerr << "--->> _runSector, end of sweep" << endl;
        cerr << "  thisElError, nextElError: "
             << thisElError << ", " << nextElError << endl;
      }
      
      // new sweep

      prevEl = thisEl;
      thisEl = nextEl;
      nextEl += deltaEl;

      _writeEndOfSweep();
      _sweepNum++;
      _fixedEl = thisEl;
      _writeStartOfSweep();

    }
    
    // pass beam through
    if (_writeBeam()) {
      return -1;
    }

  } // while
  
  return -1;

}

//////////////////////////////////////////////////
// run an rhi scan

int XpolScanControl::_runRhi(double startAz,
                             double endAz,
                             double deltaAz,
                             double startEl,
                             double endEl,
                             double azVel,
                             double elVel,
                             double transitionTolerance)
{

  if (_params.debug) {
    cerr << "====>> _runRhi <<====" << endl;
    cerr << "  startAz: " << startAz << endl;
    cerr << "  endAz: " << endAz << endl;
    cerr << "  deltaAz: " << deltaAz << endl;
    cerr << "  startEl: " << startEl << endl;
    cerr << "  endEl: " << endEl << endl;
    cerr << "  azVel: " << azVel << endl;
    cerr << "  elVel: " << elVel << endl;
    cerr << "==============================" << endl;
  }

  // widen the vertical range on each end
  
  double elWidenDeg = elVel * _params.rhi_widen_secs;
  double scanStartEl = startEl - elWidenDeg;
  if (scanStartEl < -1.0) {
    scanStartEl = -1.0;
  }
  double scanEndEl = endEl + elWidenDeg;
  double elSpan = scanEndEl - scanStartEl;

  double azSpan = endAz - startAz;
  if (azSpan < 0) {
    azSpan += 360.0;
  }

  // first point to startAz, startEl
  
  if (_runPointing(startAz, scanStartEl, 
                   _params.default_antenna_speed,
                   _params.default_antenna_speed, 0, false)) {
    cerr << "ERROR - XpolScanControl::_runRhi" << endl;
    return -1;
  }

  // set scan type
  
  _scanMode = DS_RADAR_RHI_MODE;
  _writeParams();
  
  // start of volume

  _transition = false;
  _volNum++;
  _sweepNum = 0;
  _writeStartOfVolume();
  _writeStartOfSweep();

  // write paxi file
  
  // Name: ElRaster
  // Description: Execute elevation raster scan from given position
  //   (or current position if omitted) at given speed, covering given
  //   region with given azimuth increment.
  // Required params: azVel, elVel, azSpan, elSpan, azInc
  // Optional params: az, el
  // Example:
  //   To execute an elevation raster scan covering the az, el region
  //   {azMin=20, azMax=60, elMin=10, elMax=30}
  //     in azimuth increments of 2 degrees,
  //   at az, el speeds of 5 and 1, respectively:
  //     ElRaster az=20 el=10 azSpan=40 elSpan=20 azInc=2 azVel=5 elVel=1;
  //     NOTE: max ax, el of {60, 30} = {20, 10} (starting) + {40, 20} (span)

  if (_openPaxiFile()) {
    return -1;
  }

  fprintf(_paxiFile, "# Rhi command\n");
  fprintf(_paxiFile,
          "ElRaster az=%g el=%g azSpan=%g elSpan=%g "
          "azInc=%g azVel=%g elVel=%g;\n",
          _getRawAz(startAz), scanStartEl,
          azSpan, elSpan, deltaAz, azVel, elVel);
  _closePaxiFile();

  // upload paxi file to server
  
  _uploadPaxiFile();
  
  // initialize

  double prevAz = _getNormAz(startAz - deltaAz);
  double thisAz = startAz;
  double nextAz = _getNormAz(thisAz + deltaAz);
  _fixedAz = thisAz;
  int stoppedCount = -1;
  
  int nSweeps = (int) (azSpan / deltaAz + 1.5);
  if (_params.debug) {
    cerr << "Rhi nSweeps: " << nSweeps << endl;
  }

  // read in beam data, watch progress

  while (true) {
    
    PMU_auto_register("_runRhi");
    
    // read input queue
    
    if (_getNextBeam("Rhi")) {
      return -1;
    }
    
    double prevAzError = _getAbsAzDiff(_az, prevAz);
    double thisAzError = _getAbsAzDiff(_az, thisAz);
    double nextAzError = _getAbsAzDiff(_az, nextAz);

    // check for transition
    
    _transition = false;
    if (thisAzError > transitionTolerance) {
      _transition = true;
    }
    if (_el > endEl || _el < startEl) {
      _transition = true;
    }

    // check for stopped condition
    
    if (_isStopped()) {
      if (stoppedCount >= 0){
        stoppedCount++;
      }
    } else {
      stoppedCount = 0;
    }

    // check if we are done
    bool done = false;
    double elError = 0;
    if (_sweepNum == nSweeps - 1) {
      if (nSweeps % 2 == 0) {
        // even number of sweeps, end at startEl
        elError = fabs(_el - scanStartEl);
      } else {
        // odd number of sweeps, end at endEl
        elError = fabs(_el - scanEndEl);
      }
      if (elError < _params.requested_angle_tolerance) {
        done = true;
      }
    }

    if ((prevAzError < thisAzError) && _sweepNum > 0) {
      done = true;
    }
    if (stoppedCount > 100) {
      done = true;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "--->> _runRhi, az, el, elError, "
           << "prevAzError, thisAzError, stoppedCount: "
           << _az << ", "
           << _el << ", "
           << elError << ", "
           << prevAzError << ", "
           << thisAzError << ", "
           << stoppedCount << endl;
    }

    if (done) {
      
      if (_params.debug >= Params::DEBUG_NORM) {
        cerr << "--->> _runRhi, end of volume" << endl;
        cerr << "  thisAzError, prevAzError: "
             << thisAzError << ", " << prevAzError << endl;
        cerr << "  stoppedCount: " << stoppedCount << endl;
        cerr << "  done: " << (char *) (done?"Y":"N") << endl;
      }
      
      // end of volume
      
      _writeEndOfSweep();
      _writeEndOfVolume();

      return 0;

    } else if (nextAzError < thisAzError) {
      
      if (_params.debug >= Params::DEBUG_NORM) {
        cerr << "--->> _runRhi, end of sweep" << endl;
        cerr << "  thisAzError, nextAzError: "
             << thisAzError << ", " << nextAzError << endl;
      }
      
      // new sweep
      
      prevAz = thisAz;
      thisAz = nextAz;
      nextAz += deltaAz;
      
      _writeEndOfSweep();
      _sweepNum++;
      _fixedAz = thisAz;
      _writeStartOfSweep();

    } // if (done)
    
    // pass beam through
    if (_writeBeam()) {
      return -1;
    }

  } // while
  
  return -1;

}

//////////////////////////////////////////////////
// run a vertical pointing scan

int XpolScanControl::_runVertPoint(double startAz,
                                   double startEl,
                                   double azVel,
                                   double elVel,
                                   int nSweeps,
                                   double transitionTolerance)
{

  if (_params.debug) {
    cerr << "====>> _runVertPoint <<====" << endl;
    cerr << "  startAz: " << startAz << endl;
    cerr << "  startEl: " << startEl << endl;
    cerr << "  azVel: " << azVel << endl;
    cerr << "  elVel: " << elVel << endl;
    cerr << "  nSweeps: " << nSweeps << endl;
    cerr << "==============================" << endl;
  }

  // first point to startAz, startEl

  if (_runPointing(startAz, startEl, 
                   _params.default_antenna_speed,
                   _params.default_antenna_speed, 0, false)) {
    cerr << "ERROR - XpolScanControl::_runVertPoint" << endl;
    return -1;
  }

  // set scan type

  _scanMode = DS_RADAR_VERTICAL_POINTING_MODE;
  _writeParams();

  // start of volume

  _transition = false;
  _volNum++;
  _sweepNum = 0;
  _writeStartOfVolume();
  _writeStartOfSweep();

  // write paxi file
  
  // Name: PPI
  // Description: Execute PPI scan from given position (or current position if 
  //     omitted) at given speed.
  // Required params: azVel
  // Optional params: az, el
  // Example:
  //   To execute a PPI scan beginning from the current position, and
  //   spinning around in azimuth at a rate of 12 deg/sec:
  //     PPI azVel=12;
  
  if (_openPaxiFile()) {
    return -1;
  }
  double elInc = 0.01;
  double elSpan = nSweeps * elInc * 2.0;
  fprintf(_paxiFile, "# Volumr command for VertPoint\n");
  fprintf(_paxiFile, "Volume azVel=%g elVel=%g "
          "el=%g elSpan=%g elInc=%g;\n",
          azVel, elVel, startEl - elSpan, elSpan, elInc);
  _closePaxiFile();

  // upload paxi file to server
  
  _uploadPaxiFile();
  
  // initialize

  _fixedEl = startEl;
  if (_params.debug) {
    cerr << "VertPoint nSweeps: " << nSweeps << endl;
  }

  // read in beam data, watch progress

  double sumAzMoved = 0.0;
  
  while (true) {
    
    PMU_auto_register("_runVertPoint");
    
    // read input queue
    
    if (_getNextBeam("VertPoint")) {
      return -1;
    }
    
    // check for transition

    double thisElError = fabs(_el - startEl);
    if (thisElError > transitionTolerance) {
      _transition = true;
    } else {
      _transition = false;
    }

    // compute az distance moved

    sumAzMoved += fabs(_azDist);

    // allow for 10 degree overlap in data
    // i.e. 370 degrees in total movement
    if (sumAzMoved >= 370.0) {
      _writeEndOfSweep();
      _sweepNum++;
      if (_sweepNum == nSweeps) {
        _writeEndOfVolume();
        return 0;
      }
      _writeStartOfSweep();
      sumAzMoved = 0;
    }
    
    // pass beam through
    if (_writeBeam()) {
      return -1;
    }

  } // while
  
  return -1;

}

//////////////////////////////////////////////////
// run a sunscan scan as az raster

int XpolScanControl::_runSunscanRaster(double startAz,
                                       double endAz,
                                       double startEl,
                                       double endEl,
                                       double deltaEl,
                                       double azVel,
                                       double elVel,
                                       double transitionTolerance)
{

  if (_params.debug) {
    cerr << "====>> _runSunscanRaster <<====" << endl;
    cerr << "  startAz: " << startAz << endl;
    cerr << "  endAz: " << endAz << endl;
    cerr << "  startEl: " << startEl << endl;
    cerr << "  endEl: " << endEl << endl;
    cerr << "  deltaEl: " << deltaEl << endl;
    cerr << "  azVel: " << azVel << endl;
    cerr << "  elVel: " << elVel << endl;
    cerr << "================================" << endl;
  }

  // get a beam so we have the radar params
  
  if (_getNextBeam("Sunscan")) {
    return -1;
  }

  // initialize sun position object
  
  DsRadarParams rparams = _inputMsg.getRadarParams();
  _sunPosn.setLocation(rparams.latitude, rparams.longitude,
                       rparams.altitude / 1000.0);

  // compute estimated time to complete scan

  double azSpan = endAz - startAz;
  if (azSpan < 0) {
    azSpan += 360.0;
  }

  double elSpan = endEl - startEl;
  int nSweeps = (int) (elSpan / deltaEl + 1.5);
  if (_params.debug) {
    cerr << "Sunscan nSweeps: " << nSweeps << endl;
  }

  double azRange = fabs(endAz - startAz) * nSweeps;
  double elRange = fabs(endEl - startEl);
  double nsecsScan = ((azRange / azVel) + (elRange / elVel)) * 1.1;

  // get sun az and el at start and end time

  double startTime = time(NULL);
  double sunAzStart, sunElStart;
  _sunPosn.computePosnNova(startTime - 30000, sunElStart, sunAzStart);

  double endTime = startTime + nsecsScan;
  double sunAzEnd, sunElEnd;
  _sunPosn.computePosnNova(endTime - 30000, sunElEnd, sunAzEnd);

  // compute scanning limits

  double scanAzLeftStart = _getNormAz(sunAzStart + startAz);
  double scanAzLeftEnd = _getNormAz(sunAzEnd + startAz);
  double scanAzLeft = MIN(scanAzLeftStart, scanAzLeftEnd);

  double scanAzRightStart = _getNormAz(sunAzStart + endAz);
  double scanAzRightEnd = _getNormAz(sunAzEnd + endAz);
  double scanAzRight = MAX(scanAzRightStart, scanAzRightEnd);
  
  double scanElStart = sunElStart + startEl;
  double scanElEnd = sunElEnd + endEl;
  if (sunElEnd < sunElStart) {
    // sun elevation decreasing
    scanElStart = sunElStart - startEl;
    scanElEnd = sunElEnd - endEl;
  }
  double scanElRange = scanElEnd - scanElStart;
  double scanElInc = scanElRange / nSweeps;

  if (_params.debug) {
    cerr << "============ SunScan ============" << endl;
    cerr << "scanAzLeftStart: " << scanAzLeftStart << endl;
    cerr << "scanAzLeftEnd: " << scanAzLeftEnd << endl;
    cerr << "scanAzLeft: " << scanAzLeft << endl;
    cerr << "scanAzRightStart: " << scanAzRightStart << endl;
    cerr << "scanAzRightEnd: " << scanAzRightEnd << endl;
    cerr << "scanAzRight: " << scanAzRight << endl;
    cerr << "scanElStart: " << scanElStart << endl;
    cerr << "scanElEnd: " << scanElEnd << endl;
    cerr << "scanElRange: " << scanElRange << endl;
    cerr << "scanElInc: " << scanElInc << endl;
    cerr << "=================================" << endl;
  }

  // get close by pointing, moving fast

  if (_runPointing(scanAzLeft, scanElStart, 
                   _params.default_antenna_speed,
                   _params.default_antenna_speed, 0, false)) {
    cerr << "ERROR - XpolScanControl::_runSuncan" << endl;
    return -1;
  }

  // call sector mode

  if (_runSector(DS_RADAR_SUNSCAN_MODE,
                 "Sunscan",
                 scanAzLeft, scanAzRight,
                 scanElStart, scanElEnd, scanElInc,
                 azVel, elVel,
                 transitionTolerance)) {
    cerr << "ERROR - XpolScanControl::_runSunscan" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// run a sunscan scan as an elevation raster
//
// We perform a sector at a constant elevation,
// waiting for the sun to pass through the
// elevation angle

int XpolScanControl::_runSunscanConstEl(double startAz,
                                        double endAz,
                                        double startEl,
                                        double endEl,
                                        double deltaEl,
                                        double azVel,
                                        double elVel)
{
  
  if (_params.debug) {
    cerr << "====>> _runSunscanConstEl <<====" << endl;
    cerr << "  startAz: " << startAz << endl;
    cerr << "  endAz: " << endAz << endl;
    cerr << "  startEl: " << startEl << endl;
    cerr << "  endEl: " << endEl << endl;
    cerr << "  deltaEl: " << deltaEl << endl;
    cerr << "  azVel: " << azVel << endl;
    cerr << "  elVel: " << elVel << endl;
    cerr << "===================================" << endl;
  }

  // get a beam so we have the radar params
  
  if (_getNextBeam("Sunscan")) {
    return -1;
  }

  // initialize sun position object
  
  DsRadarParams rparams = _inputMsg.getRadarParams();
  _sunPosn.setLocation(rparams.latitude, rparams.longitude,
                       rparams.altitude / 1000.0);

  // compute estimated time to complete scan

  double azSpan = endAz - startAz;
  if (azSpan < 0) {
    azSpan += 360.0;
  }
  double elSpan = endEl - startEl;
  int nSweeps = (int) (elSpan / deltaEl + 1.5);
  if (_params.debug) {
    cerr << "Sunscan nSweeps: " << nSweeps << endl;
  }

  double azRange = fabs(endAz - startAz) * nSweeps;
  // double elRange = fabs(endEl - startEl);
  double nsecsScan = (azRange / azVel) * 1.1;

  // get sun az and el at start, end and mid time
  
  double startTime = time(NULL);
  double sunAzStart, sunElStart;
  _sunPosn.computePosnNova(startTime, sunElStart, sunAzStart);

  double endTime = startTime + nsecsScan;
  double sunAzEnd, sunElEnd;
  _sunPosn.computePosnNova(endTime, sunElEnd, sunAzEnd);

  double midTime = startTime + nsecsScan / 2.0;
  double sunAzMid, sunElMid;
  _sunPosn.computePosnNova(midTime, sunElMid, sunAzMid);

  // compute scanning limits

  double scanAzLeft = _getNormAz(sunAzMid + startAz);
  double scanAzRight = _getNormAz(sunAzMid + endAz);
  double scanAzSpan = azSpan;

  double scanElStart = sunElMid + 0.5;
  if (sunElEnd < sunElStart) {
    scanElStart = sunElMid - 0.5;
  }
  double scanElInc = 1.0e-6; // keep elevation constant
  double scanElSpan = 1.0; // dummy
  if (sunElEnd < sunElStart) {
    scanElInc *= -1.0;
    scanElSpan *= -1.0;
  }
  double scanElEnd = scanElStart + scanElSpan;

  if (_params.debug) {
    cerr << "============ SunScan ============" << endl;
    cerr << "nsecsScan: " << nsecsScan << endl;
    cerr << "sunAzStart: " << sunAzStart << endl;
    cerr << "sunElStart: " << sunElStart << endl;
    cerr << "sunAzEnd: " << sunAzEnd << endl;
    cerr << "sunElEnd: " << sunElEnd << endl;
    cerr << "sunAzMid: " << sunAzMid << endl;
    cerr << "sunElMid: " << sunElMid << endl;
    cerr << "scanAzLeft: " << scanAzLeft << endl;
    cerr << "scanAzRight: " << scanAzRight << endl;
    cerr << "scanElStart: " << scanElStart << endl;
    cerr << "scanElEnd: " << scanElEnd << endl;
    cerr << "scanElInc: " << scanElInc << endl;
    cerr << "scanElSpan: " << scanElSpan << endl;
    cerr << "=================================" << endl;
  }

  // get close by pointing, moving fast

  if (_runPointing(scanAzLeft, scanElStart,
                   _params.default_antenna_speed,
                   _params.default_antenna_speed, 0, false)) {
    cerr << "ERROR - XpolScanControl::_runSuncan" << endl;
    return -1;
  }

  // set scan type

  _scanMode = DS_RADAR_SUNSCAN_MODE;
  _writeParams();

  // start of volume

  _transition = false;
  _volNum++;
  _sweepNum = 0;
  _writeStartOfVolume();
  _writeStartOfSweep();

  // write paxi file
  
  // Name: AzRaster
  // Description: Execute azimuth raster scan from given position
  //   (or current position if omitted) at given speed,
  //   covering given region with given elevation increment.
  // Required params: azVel, elVel, azSpan, elSpan, elInc
  // Optional params: az, el
  // Example:
  //   To execute an azimuth raster scan covering the az, el region
  //   {azMin=20, azMax=60, elMin=10, elMax=30}
  //      in elevation increments of 2 degrees,
  //   at az, el speeds of 5 and 1, respectively:
  //     AzRaster az=20 el=10 azSpan=40 elSpan=20 elInc=2 azVel=5 elVel=1;
  //     NOTE: max ax, el of {60, 30} = {20, 10} (starting) + {40, 20} (span)

  if (_openPaxiFile()) {
    return -1;
  }
  fprintf(_paxiFile, "# Sector command for sunscan\n");
  fprintf(_paxiFile,
          "AzRaster az=%g el=%g azSpan=%g elSpan=%g "
          "elInc=%g azVel=%g elVel=%g;\n",
          _getRawAz(scanAzLeft), scanElStart, scanAzSpan, scanElSpan,
          scanElInc, azVel, elVel);
  _closePaxiFile();

  // upload paxi file to server
  
  _uploadPaxiFile();
  
  // initialize

  double thisEl = startEl;
  double nextEl = thisEl + deltaEl;
  _fixedEl = thisEl;
  double sumAzMoved = 0.0;

  // read in beam data, watch progress

  while (true) {
    
    PMU_auto_register("_runSector");
    
    // read input queue
    
    if (_getNextBeam("Sector")) {
      return -1;
    }
    
    // compute az distance moved

    sumAzMoved += fabs(_azDist);

    // check for end of sweep, looking for change in direction in azimuth

    if (sumAzMoved > scanAzSpan * 0.75) {
      if (_azRate * _prevAzRate < 0.0 || _azRate == 0.0) {
        // end of sweep
        if (_params.debug) {
          cerr << "==>> SunScan: end of sweep: " << _sweepNum << endl;
        }
        thisEl = nextEl;
        nextEl += deltaEl;
        _writeEndOfSweep();
        _sweepNum++;
        _fixedEl = thisEl;
        sumAzMoved = 0.0;
        if (_sweepNum == nSweeps) {
          // end of volume
          if (_params.debug) {
            cerr << "==>> SunScan: end of volume" << endl;
          }
          _writeEndOfVolume();
          return 0;
        } else {
          _writeStartOfSweep();
        }
      }
    }
    
    // pass beam through
    if (_writeBeam()) {
      return -1;
    }

  } // while
  
  return -1;

}

//////////////////////////////////////////////////
// run a sunscan scan as an azimuth raster
//
// We perform a sector at a constant azimuth,
// waiting for the sun to pass through the
// azimuth plane

int XpolScanControl::_runSunscanConstAz(double startAz,
                                        double endAz,
                                        double startEl,
                                        double endEl,
                                        double deltaAz,
                                        double azVel,
                                        double elVel)
{
  
  if (_params.debug) {
    cerr << "====>> _runSunscanConstAz <<====" << endl;
    cerr << "  startAz: " << startAz << endl;
    cerr << "  endAz: " << endAz << endl;
    cerr << "  startEl: " << startEl << endl;
    cerr << "  endEl: " << endEl << endl;
    cerr << "  deltaAz: " << deltaAz << endl;
    cerr << "  azVel: " << azVel << endl;
    cerr << "  elVel: " << elVel << endl;
    cerr << "===================================" << endl;
  }

  // get a beam so we have the radar params
  
  if (_getNextBeam("SunscanConstAz")) {
    return -1;
  }

  // initialize sun position object
  
  DsRadarParams rparams = _inputMsg.getRadarParams();
  _sunPosn.setLocation(rparams.latitude, rparams.longitude,
                       rparams.altitude / 1000.0);

  // compute estimated time to complete scan

  double azSpan = endAz - startAz;
  if (azSpan < 0) {
    azSpan += 360.0;
  }
  double elSpan = endEl - startEl;

  int nSweeps = (int) (azSpan / deltaAz + 1.5);
  if (_params.debug) {
    cerr << "SunscanConstAz nSweeps: " << nSweeps << endl;
  }

  double elRange = fabs(elSpan) * nSweeps;
  double nsecsScan = (elRange / elVel) * 2.0;

  // get sun el and az at start, end and mid time
  
  double startTime = time(NULL);
  double sunElStart, sunAzStart;
  _sunPosn.computePosnNova(startTime, sunElStart, sunAzStart);

  double endTime = startTime + nsecsScan;
  double sunElEnd, sunAzEnd;
  _sunPosn.computePosnNova(endTime, sunElEnd, sunAzEnd);

  double midTime = startTime + nsecsScan / 2.0;
  double sunElMid, sunAzMid;
  _sunPosn.computePosnNova(midTime, sunElMid, sunAzMid);

  // compute scanning limits

  double scanElLower = sunElMid + startEl;
  double scanElUpper = sunElMid + endEl;
  double scanElSpan = elSpan;
  
  double scanAzStart = sunAzEnd;
  double scanAzInc = 1.0e-6; // keep az constant
  double scanAzSpan = 1.0; // dummy
  if (sunAzEnd < sunAzStart) {
    scanAzInc *= -1.0;
    scanAzSpan *= -1.0;
  }
  double scanAzEnd = scanAzStart + scanAzSpan;
  
  if (_params.debug) {
    cerr << "============ SunScan ============" << endl;
    cerr << "nsecsScan: " << nsecsScan << endl;
    cerr << "sunElStart: " << sunElStart << endl;
    cerr << "sunAzStart: " << sunAzStart << endl;
    cerr << "sunElEnd: " << sunElEnd << endl;
    cerr << "sunAzEnd: " << sunAzEnd << endl;
    cerr << "sunElMid: " << sunElMid << endl;
    cerr << "sunAzMid: " << sunAzMid << endl;
    cerr << "scanElLower: " << scanElLower << endl;
    cerr << "scanElUpper: " << scanElUpper << endl;
    cerr << "scanAzStart: " << scanAzStart << endl;
    cerr << "scanAzEnd: " << scanAzEnd << endl;
    cerr << "scanAzInc: " << scanAzInc << endl;
    cerr << "scanAzSpan: " << scanAzSpan << endl;
    cerr << "=================================" << endl;
  }

  // get close by pointing, moving fast

  if (_runPointing(scanAzStart, scanElLower,
                   _params.default_antenna_speed,
                   _params.default_antenna_speed, 0, false)) {
    cerr << "ERROR - XpolScanControl::_runSuncan" << endl;
    return -1;
  }

  // set scan type

  _scanMode = DS_RADAR_SUNSCAN_RHI_MODE;
  _writeParams();

  // start of volume

  _transition = false;
  _volNum++;
  _sweepNum = 0;
  _writeStartOfVolume();
  _writeStartOfSweep();

  // write paxi file
  
  // Name: ElRaster
  // Description: Execute elimuth raster scan from given position
  //   (or current position if omitted) at given speed,
  //   covering given region with given azevation increment.
  // Required params: elVel, azVel, elSpan, azSpan, azInc
  // Optional params: el, az
  // Example:
  //   To execute an elimuth raster scan covering the el, az region
  //   {elMin=20, elMax=60, azMin=10, azMax=30}
  //      in azevation increments of 2 degrees,
  //   at el, az speeds of 5 and 1, respectivazy:
  //     ElRaster el=20 az=10 elSpan=40 azSpan=20 azInc=2 elVel=5 azVel=1;
  //     NOTE: max ax, az of {60, 30} = {20, 10} (starting) + {40, 20} (span)

  if (_openPaxiFile()) {
    return -1;
  }
  fprintf(_paxiFile, "# Sector command for sunscan\n");
  fprintf(_paxiFile,
          "ElRaster el=%g az=%g elSpan=%g azSpan=%g "
          "azInc=%g elVel=%g azVel=%g;\n",
          scanElLower, _getRawAz(scanAzStart), scanElSpan, scanAzSpan,
          scanAzInc, elVel, azVel);
  _closePaxiFile();

  // upload paxi file to server
  
  _uploadPaxiFile();
  
  // initialize

  double thisAz = startAz;
  double nextAz = thisAz + deltaAz;
  _fixedAz = thisAz;
  double sumElMoved = 0.0;

  // read in beam data, watch progress

  while (true) {
    
    PMU_auto_register("_runSector");
    
    // read input queue
    
    if (_getNextBeam("Sector")) {
      return -1;
    }
    
    // compute el distance moved

    sumElMoved += fabs(_elDist);
    // cerr << "11111111111 sumElMoved, elRate, prevElRate: "
    //      << sumElMoved << ", "
    //      << _elRate << ", "
    //      << _prevElRate << endl;

    // check for end of sweep, looking for change in direction in elimuth

    if (sumElMoved > scanElSpan * 0.75) {
      if (_elRate * _prevElRate < 0.0 || _elRate == 0.0) {
        // end of sweep
        if (_params.debug) {
          cerr << "==>> SunScan: end of sweep: " << _sweepNum << endl;
        }
        thisAz = nextAz;
        nextAz += deltaAz;
        _writeEndOfSweep();
        _sweepNum++;
        _fixedAz = thisAz;
        sumElMoved = 0.0;
        if (_sweepNum == nSweeps) {
          // end of volume
          if (_params.debug) {
            cerr << "==>> SunScan: end of volume" << endl;
          }
          _writeEndOfVolume();
          return 0;
        } else {
          _writeStartOfSweep();
        }
      }
    }
    
    // pass beam through
    if (_writeBeam()) {
      return -1;
    }

  } // while
  
  return -1;

}

///////////////////////////////////////
// initialize the output queue

int XpolScanControl::_openOutputFmq()
  
{

  if (_outputQueue.isOpen()) {
    // already open
    return 0;
  }

  if (!_inputQueue.isOpen()) {
    cerr << "ERROR - XpolScanControl::_openOutputFmq()" << endl;
    cerr << "  Input FMQ not yet open" << endl;
    return -1;
  }

  // compute output FMQ size from input size

  int numSlots = _params.output_fmq_nslots;
  int bufSize = _params.output_fmq_size;
  if (_params.set_output_size_equal_to_input_size) {
    numSlots = _inputQueue.getNumSlots();
    bufSize = _inputQueue.getBufSize();
    if (numSlots == 0 || bufSize == 0) {
      cerr << "ERROR - XpolScanControl::_openOutputFmq()" << endl;
      cerr << "  Input FMQ not yet initialized" << endl;
      return -1;
    }
  }
  
  if (_outputQueue.initReadWrite(_params.output_fmq_path,
                                 _progName.c_str(),
                                 _params.debug >= Params::DEBUG_EXTRA,
                                 Fmq::END, // start position
                                 false,    // compression
                                 numSlots, bufSize)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize FMQ: "
         << _params.output_fmq_path << endl;
    cerr << "  nSlots: " << numSlots << endl;
    cerr << "  nBytes: " << bufSize << endl;
    cerr << _outputQueue.getErrStr() << endl;
    return -1;
  }

  _outputQueue.setSingleWriter();
  if (_params.write_blocking) {
    _outputQueue.setBlockingWrite();
  }
  if (_params.data_mapper_report_interval > 0) {
    _outputQueue.setRegisterWithDmap
      (true, _params.data_mapper_report_interval);
  }
  _outputMsg.clearAll();
  _outputMsg.setType(0);

  if (_params.debug) {
    cerr << "-->> Opened output queue: " << _params.output_fmq_path << endl;
    cerr << "  nSlots: " << numSlots << endl;
    cerr << "  nBytes: " << bufSize << endl;
  }

  return 0;

}

///////////////////////////////////////
// write beam to output FMQ
// returns 0 on success, -1 on failure

int XpolScanControl::_writeBeam()
  
{

  // make sure it's open
  
  if (_openOutputFmq()) {
    return -1;
  }
  
  // copy input data to output beam
  
  const DsRadarBeam &beamIn = _inputMsg.getRadarBeam();
  DsRadarMsg msg;
  DsRadarBeam &beamOut = msg.getRadarBeam();
  beamOut.copy(beamIn);

  // modify scan parameters
  
  beamOut.scanMode = _scanMode;
  beamOut.volumeNum = _volNum;
  beamOut.tiltNum = _sweepNum;
  beamOut.targetElev = _fixedEl;
  beamOut.targetAz = _fixedAz;
  beamOut.antennaTransition = _transition;

  // write the beam message

  if (_outputQueue.putDsMsg(msg, DsRadarMsg::RADAR_BEAM)) {
    cerr << "ERROR - XpolScanControl::_writeBeam" << endl;
    cerr << "  Cannot put beam to queue" << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "-->> Wrote beam" << endl;
  }

  return 0;

}
    
////////////////////////////////////////////
// write params to output FMQ
// returns 0 on success, -1 on failure

int XpolScanControl::_writeParams()
  
{

  // make sure fmq is open
  
  if (_openOutputFmq()) {
    return -1;
  }

  if (!_inputMsg.allParamsSet()) {
    // params not yet set
    return -1;
  }

  // load message with radar and field params

  DsRadarMsg msg;
  DsRadarParams rparams = _inputMsg.getRadarParams();
  rparams.scanMode = _scanMode;
  msg.setRadarParams(rparams);
  msg.setFieldParams(_inputMsg.getFieldParams());
  
  // write the parameter
  
  if (_outputQueue.putDsMsg
      (msg, DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - XpolScanControl::_writeParams" << endl;
    cerr << "  Cannot put params to queue" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> Wrote params" << endl;
  }

  return 0;

}

////////////////////////////////////////////
// write calibration to output FMQ 
// returns 0 on success, -1 on failure

int XpolScanControl::_writeCalib(const DsRadarCalib &calib)
  
{

  // make sure fmq is open
  
  if (_openOutputFmq()) {
    return -1;
  }

  // load calibration into message

  DsRadarMsg msg;
  DsRadarCalib &outCalib = msg.getRadarCalib();
  outCalib = calib;

  // write the calibration message
  
  if (_outputQueue.putDsMsg(msg, DsRadarMsg::RADAR_CALIB)) {
    cerr << "ERROR - XpolScanControl::_writeCalib" << endl;
    cerr << "  Cannot put calib to queue" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> Wrote calib" << endl;
  }

  return 0;

}

////////////////////////////////////////////
// write flags to output FMQ
// returns 0 on success, -1 on failure

int XpolScanControl::_writeFlags(const DsRadarFlags &flags)
  
{

  // make sure fmq is open
  
  if (_openOutputFmq()) {
    return -1;
  }

  // load flags into message

  DsRadarMsg msg;
  DsRadarFlags &outFlags = msg.getRadarFlags();
  outFlags = flags;

  // write the flags message
  
  if (_outputQueue.putDsMsg(msg, DsRadarMsg::RADAR_FLAGS)) {
    cerr << "ERROR - XpolScanControl::_writeFlags" << endl;
    cerr << "  Cannot put flags to queue" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "-->> Wrote flags" << endl;
  }

  return 0;

}

////////////////////////////////////////
// write scan flags

void XpolScanControl::_writeStartOfVolume()
{
  if (_openOutputFmq()) {
    return;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "++++>> writing start of vol, volNum, time: "
         << _volNum << ", "
         << DateTime::strm(_beamTimeSecs) << endl;
  }
  _outputQueue.putStartOfVolume(_volNum, _beamTimeSecs);
}

void XpolScanControl::_writeEndOfVolume()
{
  if (_openOutputFmq()) {
    return;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "++++>> writing end of vol, volNum, time: "
         << _volNum << ", "
         << DateTime::strm(_beamTimeSecs) << endl;
  }
  _outputQueue.putEndOfVolume(_volNum, _beamTimeSecs);
}

void XpolScanControl::_writeStartOfSweep()
{
  if (_openOutputFmq()) {
    return;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "++++>> writing start of sweep, sweepNum, time: "
         << _sweepNum << ", "
         << DateTime::strm(_beamTimeSecs) << endl;
  }
  _outputQueue.putStartOfTilt(_sweepNum, _beamTimeSecs);
}

void XpolScanControl::_writeEndOfSweep()
{
  if (_openOutputFmq()) {
    return;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "++++>> writing end of sweep, sweepNum, time: "
         << _sweepNum << ", "
         << DateTime::strm(_beamTimeSecs) << endl;
  }
  _outputQueue.putEndOfTilt(_sweepNum, _beamTimeSecs);
}

//////////////////////////////////////////////////
// open paxi file

int XpolScanControl::_openPaxiFile()

{

  _closePaxiFile();
  if ((_paxiFile = fopen(_params.paxi_file_path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - XpolScanControl::_openPaxiFile" << endl;
    cerr << "  Cannot create file for writing: "
         << _params.paxi_file_path << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// close paxi file

void XpolScanControl::_closePaxiFile()

{

  if (_paxiFile == NULL) {
    return;
  }

  fclose(_paxiFile);
  _paxiFile = NULL;
  
  if (_params.debug >= Params::DEBUG_NORM) {
    // print the paxi file contents
    FILE *fp;
    if ((fp = fopen(_params.paxi_file_path, "r")) == NULL) {
      return;
    }
    cerr << "=======  PAXI file contents ===========" << endl;
    cerr << "File path: " << _params.paxi_file_path << endl;
    char line[1024];
    while (fgets(line, 1024, fp) != NULL) {
      cerr << line;
    }
    cerr << "=======================================" << endl;
    fclose(fp);
  }

}

////////////////////////////////////
// call script to upload PAXI file

void XpolScanControl::_uploadPaxiFile()
  
{
  char pmuStr[4096];
  sprintf(pmuStr, "Calling %s", _params.upload_paxi_script_name);
  PMU_force_register(pmuStr);
  
  // Fork a child to run the script
  
  time_t start_time = time(NULL);
  time_t terminate_time = start_time + _params.script_max_run_secs;
  pid_t childPid = fork();
  
  if (childPid == 0) {
    
    // this is the child process, so exec the script

    _execScript();

    // exit
    
    if (_params.debug) {
      cerr << "Child process exiting ..." << endl;
    }
    
    _exit(0);

  }

  // this is the parent

  if (_params.debug) {
    cerr << "Script started, child pid: " << childPid << endl;
  }

  if (_params.run_script_in_background) {
    
    // add the child to the map of active children
    
    activePair_t pp;
    pp.first = childPid;
    pp.second = terminate_time;
    _active.insert(_active.begin(), pp);

    if (_params.debug) {
      activeMap_t::iterator ii;
      for (ii = _active.begin(); ii != _active.end(); ii++) {
	cerr << "pid: " << ii->first << "  terminate_time: "
	     << ii->second << endl;
      }
    }
    
  } else {
    
    // script in the foreground - so we must wait for it
    // to complete
    
    while (true) {
      
      int status;
      if (waitpid(childPid, &status,
		  (int) (WNOHANG | WUNTRACED)) == childPid) {
	// child exited
	time_t end_time = time(NULL);
	int runtime = (int) (end_time - start_time);
	sprintf(pmuStr, "%s took %d secs",
		_params.upload_paxi_script_name, runtime);
	PMU_force_register(pmuStr);
	if (_params.debug) {
	  cerr << "Child exited, pid: " << childPid << endl;
	  cerr << "  Runtime in secs: " << runtime << endl;
	}
	return;
      }
      
      // script is still running
      
      sprintf(pmuStr, "%s running", _params.upload_paxi_script_name);
      PMU_auto_register(pmuStr);
      
      // child still running - kill it as required

      _killAsRequired(childPid, terminate_time);

      // sleep for a sec
      
      umsleep(1000);
      
    } // while
    
  } // if (run_script_in_background)

}

////////////////////////////////////
// execute the script

void XpolScanControl::_execScript()

{

  // load vector of strings for args
  
  vector<string> argVec;

  argVec.push_back(_params.upload_paxi_script_name);
  argVec.push_back(_params.paxi_file_path);
  
  // set up execvp args - this is a null-terminated array of strings
  
  TaArray<const char *> args_;
  const char **args = args_.alloc(argVec.size() + 1);
  for (size_t ii = 0; ii < argVec.size(); ii++) {
    args[ii] = argVec[ii].c_str();
  }
  args[argVec.size()] = NULL;

  if (_params.debug) {
    cerr << "Calling execvp with following args:" << endl;
    for (size_t ii = 0; ii < argVec.size(); ii++) {
      cerr << "  " << argVec[ii] << endl;
    }
  }
    
  // execute the command
  
  execvp(args[0], (char **) args);
  
}

//////////////////////////
// kill child as required
//

void XpolScanControl::_killAsRequired(pid_t pid,
				   time_t terminate_time)

{
  
  if (!_params.terminate_script_if_hung) {
    return;
  }

  time_t now = time(NULL);

  if (now < terminate_time) {
    return;
  }

  // Time to terminate script, will be reaped elsewhere
  
  if (_params.debug) {
    cerr << "Child has run too long, pid: " << pid << endl;
    cerr << "  Sending child kill signal" << endl;
  }
  
  char pmuStr[4096];
  sprintf(pmuStr, "Killing pid %d", pid);
  PMU_force_register(pmuStr);
  
  if(kill(pid,SIGKILL)) {
    perror("kill: ");
  }

}

////////////////////////////
// reap children as required
//

void XpolScanControl::_reapChildren()

{

  // reap any children which have died
  
  pid_t deadPid;
  int status;
  while ((deadPid = waitpid((pid_t) -1, &status,
			    (int) (WNOHANG | WUNTRACED))) > 0) {
    
    if (_params.debug) {
      cerr << "Reaped child: " << deadPid << endl;
    }
    
    // remove from active map
    activeMap_t::iterator ii = _active.find(deadPid);
    if (ii != _active.end()) {
      _active.erase(ii);
    }
    
  } // while
  
  // kill any children which have run too long
  
  activeMap_t::iterator ii;
  for (ii = _active.begin(); ii != _active.end(); ii++) {
    _killAsRequired(ii->first, ii->second);
  }
  
}

//////////////////////////
// kill remaining children
//

void XpolScanControl::_killRemainingChildren()

{
  
  // reap children which have died
  
  _reapChildren();

  // kill any child processes still running
  
  activeMap_t::iterator ii;
  for (ii = _active.begin(); ii != _active.end(); ii++) {
    pid_t pid = ii->first;
    if (_params.debug) {
      cerr << "  Program will exit, kill child, pid: " << pid << endl;
    }
    if(kill(pid,SIGKILL)) {
      perror("kill: ");
    }
  }

}

///////////////////////////////////////////////////
// print summary of beam headers

void XpolScanControl::_printSummary(const DsRadarMsg &msg)

{

  const DsRadarParams &radarParams = msg.getRadarParams();
  const DsRadarBeam &radarBeam = msg.getRadarBeam();
  
  // Parse the time of the beam

  DateTime dataTime(radarBeam.dataTime);

  char scanModeStr[32];
  bool isPpi = true;
  switch(radarParams.scanMode) {
    case DS_RADAR_SECTOR_MODE:
      sprintf(scanModeStr, "SECT");
      break;
    case DS_RADAR_COPLANE_MODE:
      sprintf(scanModeStr, "COPL");
      break;
    case DS_RADAR_RHI_MODE:
      sprintf(scanModeStr, " RHI");
      isPpi = false;
      break;
    case DS_RADAR_VERTICAL_POINTING_MODE:
      sprintf(scanModeStr, "VERT");
      break;
    case DS_RADAR_SURVEILLANCE_MODE:
      sprintf(scanModeStr, " SUR");
      break;
    case DS_RADAR_POINTING_MODE:
      sprintf(scanModeStr, " PNT");
      break;
    case DS_RADAR_SUNSCAN_MODE:
      sprintf(scanModeStr, " SUN");
      break;
    case DS_RADAR_IDLE_MODE:
      sprintf(scanModeStr, "IDLE");
      break;
    default:
      sprintf(scanModeStr, "%4d", radarParams.scanMode);
  }

  if (isPpi) {
    
    fprintf(stdout,
            "Mode   Vol Tilt El_tgt El_act     Az"
            " Ngat Gspac  PRF     "
            "  Date     Time\n");

    double targetAngle = radarBeam.targetElev;
    if (targetAngle < -360) {
      targetAngle = 0.0;
    }
    
    fprintf(stdout,
            "%4s %5d %4d %6.2f %6.2f %6.2f"
            " %4d %5.0f %4.0f "
            "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
            scanModeStr,
            radarBeam.volumeNum,
            radarBeam.tiltNum,
            targetAngle,
            radarBeam.elevation,
            radarBeam.azimuth,
            radarParams.numGates,
            (radarParams.gateSpacing * 1000),
            radarParams.pulseRepFreq,
            dataTime.getYear(),
            dataTime.getMonth(),
            dataTime.getDay(),
            dataTime.getHour(),
            dataTime.getMin(),
            dataTime.getSec());

  } else {

    fprintf(stdout,
            "Mode   Vol Tilt  Az_tgt  Az_act      El"
            " Ngat Gspac  PRF     "
            "  Date     Time\n");
    
    double targetAngle = radarBeam.targetAz;
    if (targetAngle < -360) {
      targetAngle = 0.0;
    }
    
    fprintf(stdout,
            "%4s %5d %4d %7.3f %7.3f %7.3f"
            " %4d %5.0f %4.0f "
            "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
            scanModeStr,
            radarBeam.volumeNum,
            radarBeam.tiltNum,
            targetAngle,
            radarBeam.azimuth,
            radarBeam.elevation,
            radarParams.numGates,
            (radarParams.gateSpacing * 1000),
            radarParams.pulseRepFreq,
            dataTime.getYear(),
            dataTime.getMonth(),
            dataTime.getDay(),
            dataTime.getHour(),
            dataTime.getMin(),
            dataTime.getSec());

  }

  int subsecsPrecision = 3;
  double divisor = 1.0e9 / pow(10.0, subsecsPrecision);
  int subsecs = (int) (radarBeam.nanoSecs / divisor + 0.5);
  char format[32];
  sprintf(format, ".%%.%dd", subsecsPrecision);
  fprintf(stdout, format, subsecs);

  if (radarBeam.antennaTransition) {
    fprintf(stdout, " *");
  }
  fprintf(stdout, "\n");

  fflush(stdout);

}

///////////////////////////////////////////////////
// Compute corrected or raw azimuth

double XpolScanControl::_getCorrectedAz(double rawAz)
{
  double correctedAz = rawAz + _params.azimuth_correction_deg;
  return _getNormAz(correctedAz);
}

double XpolScanControl::_getRawAz(double correctedAz)
{
  double rawAz = correctedAz - _params.azimuth_correction_deg;
  return _getNormAz(rawAz);
}

double XpolScanControl::_getNormAz(double az)
{
  double normAz = az;
  if (normAz > 360.0) {
    normAz -= 360.0;
  } else if (normAz < 0) {
    normAz += 360.0;
  }
  return normAz;
}

double XpolScanControl::_getAzDiff(double az1, double az2)
{
  double azError = az1 - az2;
  if (azError > 180) {
    azError -= 360;
  } else if (azError < -180) {
    azError += 360.0;
  }
  return azError;
}

double XpolScanControl::_getAbsAzDiff(double az1, double az2)
{
  double azError = fabs(az1 - az2);
  if (azError > 180) {
    azError = fabs(azError - 360);
  }
  return azError;
}

bool XpolScanControl::_isStopped()
{
  if (fabs(_azDist) < 0.005 && fabs(_elDist) < 0.005) {
    return true;
  } else {
    return false;
  }
}


