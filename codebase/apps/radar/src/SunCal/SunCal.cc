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
// SunCal.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////
//
// SunCal analyses time series data from sun scans
//
////////////////////////////////////////////////////////////////

#include "SunCal.hh"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <algorithm>
#include <functional>
#include <toolsa/uusleep.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/sincos.h>
#include <radar/RadarComplex.hh>
#include <rapformats/WxObs.hh>
#include <Mdv/MdvxField.hh>
#include <Spdb/DsSpdb.hh>
#include <didss/DsInputPath.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include "sunscan_nexrad.h"

using namespace std;

// Constructor

SunCal::SunCal(int argc, char **argv)
  
{

  _initMembers();
  isOK = true;

  // set programe name
  
  _progName = "SunCal";

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

  if (_params.n_gates < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  n_gates must be greater than 0" << endl;
    isOK = false;
    return;
  }
  
  if (_params.alternating_mode && _params.switching_receiver) {
    _switching = true;
  } else {
    _switching = false;
  }

  if (_params.scan_mode_rhi) {
    _isRhi = true;
  }

  if (_params.n_samples % 2 != 0) {
    _params.n_samples++;
  }
    
  // compute max valid sun power
  
  _maxValidSunPowerDbm = 200;
  if (_params.set_max_sun_power) {
    _maxValidSunPowerDbm = _params.max_valid_sun_power_dbm;
  }

  // compute grid props

  _gridMinAz = _params.grid_min_az;
  _gridMinEl = _params.grid_min_el;
  _gridDeltaAz = _params.grid_delta_az;
  _gridDeltaEl = _params.grid_delta_el;
  _gridNAz =
    (int) ((_params.grid_max_az - _params.grid_min_az) / _gridDeltaAz + 1.5);
  _gridNEl =
    (int) ((_params.grid_max_el - _params.grid_min_el) / _gridDeltaEl + 1.5);
  _gridMaxAz = _gridMinAz + _gridNAz * _gridDeltaAz;
  _gridMaxEl = _gridMinEl + _gridNEl * _gridDeltaEl;

  // set up vectors for raw moments and interp moments

  _createRawMomentsArray();
  _createInterpMomentsArray();

  // calibration for noise

  _noiseDbmHc = -9999.0;
  _noiseDbmVc = -9999.0;
  _noiseDbmHx = -9999.0;
  _noiseDbmVx = -9999.0;
  
  // initialize nexrad C processing

  if (_params.test_nexrad_processing) {
    nexradSolarInit(_params.n_samples);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      nexradSolarSetDebug(2);
    } else if (_params.debug) {
      nexradSolarSetDebug(1);
    }
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

SunCal::~SunCal()

{

  if (_params.debug) {
    cerr << "SunCal done ..." << endl;
  }

  if (_tsReader) {
    delete _tsReader;
  }

  if (_covarReader) {
    delete _covarReader;
  }

  // clean up memory

  _deletePulseQueue();
  _deleteRawMomentsArray();

  _deleteInterpMomentsArray();
  _deleteXpolMomentsArray();
  _deleteTestPulseMomentsArray();

  if (_params.test_nexrad_processing) {
    nexradSolarFree();
  }

  _freeGateData();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// init members

void SunCal::_initMembers()
{

  _paramsPath = NULL;
  _tsReader = NULL;
  _covarReader = NULL;

  _radarLat = 0;
  _radarLon = 0;
  _radarAltKm = 0;
  
  _isRhi = false;

  _switching = false;
  _dualPol = false;
  _alternating = false;

  _maxPulseQueueSize = 0;
  _pulseSeqNum = 0;
  _totalPulseCount = 0;
  _prevAngleOffset = -999;

  _nGates = 0;
  _startRangeKm = 0;
  _gateSpacingKm = 0;
  _nSamples = 0;
  _nSamplesHalf = 0;
  _startGateSun = 0;
  _endGateSun = 0;

  _volNum = -1;
  _prevVolNum = -1;

  _startTime = 0;
  _endTime = 0;
  _calTime = 0;

  _gridNAz = 0;
  _gridNEl = 0;
  _gridMinAz = 0;
  _gridMinEl = 0;
  _gridMaxAz = 0;
  _gridMaxEl = 0;
  _gridDeltaAz = 0;
  _gridDeltaEl = 0;

  _midTime = 0;
  _midPrt = 0;
  _midAz = 0;
  _midEl = 0;
  _targetEl = 0;
  _targetAz = 0;
  _offsetAz = 0;
  _offsetEl = 0;

  _prevOffsetEl = 0;
  _prevOffsetAz = 0;

  _nBeamsNoise = 0;
  _noiseDbmHc = 0;
  _noiseDbmHx = 0;
  _noiseDbmVc = 0;
  _noiseDbmVx = 0;

  _maxValidSunPowerDbm = 0;

  _maxPowerDbm = 0;
  _quadPowerDbm = 0;
  
  _maxPowerDbmHc = 0;
  _quadPowerDbmHc = 0;
  
  _maxPowerDbmVc = 0;
  _quadPowerDbmVc = 0;
  
  _validCentroid = false;
  _meanSunEl = 0;
  _meanSunAz = 0;
  _sunCentroidAzOffset = 0;
  _sunCentroidElOffset = 0;
  _sunCentroidAzOffsetHc = 0;
  _sunCentroidElOffsetHc = 0;
  _sunCentroidAzOffsetVc = 0;
  _sunCentroidElOffsetVc = 0;

  _ccAz = 0;
  _bbAz = 0;
  _aaAz = 0;
  _errEstAz = 0;
  _rSqAz = 0;
  _ccEl = 0;
  _bbEl = 0;
  _aaEl = 0;
  _errEstEl = 0;
  _rSqEl = 0;

  _widthRatioElAzHc = 0;
  _widthRatioElAzVc = 0;
  _widthRatioElAzDiffHV = 0;
  _zdrDiffElAz = -9999.0;

  _meanCorrSun = 0;
  _meanCorr00H = 0;
  _meanCorr00V = 0;
  _meanCorr00 = 0;
  
  _nBeamsThisVol = 0;
  _volMinEl = 0;
  _volMaxEl = 0;
  _volCount = 0;
  _endOfVol = false;
  _volInProgress = true;
  _prevEl = 0;

  _S1S2Sdev = 0;
  _SSSdev = 0;

  _nXpolPoints = 0;
  _meanXpolRatioDb = 0;
  _zdrCorr = 0;

  _testPulseDbmHc = MomentsSun::missing;
  _testPulseDbmVc = MomentsSun::missing;
  _testPulseDbmHx = MomentsSun::missing;
  _testPulseDbmVx = MomentsSun::missing;

  _sumXmitPowerHDbm = 0;
  _sumXmitPowerVDbm = 0;
  _countXmitPowerH = 0;
  _countXmitPowerV = 0;
  _meanXmitPowerHDbm = 0;
  _meanXmitPowerVDbm = 0;

  _globalPrintCount = 0;

  _timeForXpolRatio = 0;
  _xpolRatioDbFromSpdb = 0;

  _timeForSiteTemp = 0;
  _siteTempC = -9999;

}

//////////////////////////////////////////////////
// Run

int SunCal::Run ()
{

  // read in calibration

  string errStr;
  if (_calib.readFromXmlFile(_params.cal_xml_file_path, errStr)) {
    cerr << "ERROR - SunCal" << endl;
    cerr << "  Cannot decode cal file: "
         << _params.cal_xml_file_path << endl;
    return -1;
  }
  
  // create readers

  if (_createReaders()) {
    return -1;
  }

  if (_tsReader != NULL) {
    return _runForTimeSeries();
  } else if (_covarReader != NULL) {
    return _runForCovar();
  } else {
    cerr << "ERROR - SunCal::Run()" << endl;
    cerr << "  No reader object created" << endl;
    return -1;
  }

}

//////////////////////////////////////////////////
// Run with time series

int SunCal::_runForTimeSeries()
{

  PMU_auto_register("_runForTimeSeries");
  if (_params.debug) {
    cerr << "_runForTimeSeries ..." << endl;
  }

  // initialize
  
  _initForAnalysis();
  _clearPulseQueue();

  // initialize sun posn object

  _radarLat = _params.radar_lat;
  _radarLon = _params.radar_lon;
  _radarAltKm = _params.radar_alt_km;
  _sunPosn.setLocation(_radarLat, _radarLon, _radarAltKm / 1000.0);
  if (_params.test_nexrad_processing) {
    nexradSolarSetLocation(_radarLat, _radarLon, _radarAltKm / 1000.0);
  }

  while (true) {

    const IwrfTsPulse *pulse = _tsReader->getNextPulse(true);
    if (pulse == NULL) {
      _endOfVol = true;
      _volCount++;
      break;
    }
    const IwrfTsInfo &info = _tsReader->getOpsInfo();

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      if (pulse->getPulseSeqNum() % 10000 == 0) {
        cerr << "==>> Reading pulses, latest seq num, el, az: " 
             << pulse->getPulseSeqNum() << ", "
             << pulse->getEl() << ", "
             << pulse->getAz() << ", "
             << endl;
      }
    }


    if (_params.invert_hv_flag) {
      pulse->setInvertHvFlag();
    }

    _volNum = pulse->getVolNum();

    if (_params.get_location_from_data) {
      _radarLat = info.get_radar_latitude_deg();
      _radarLon = info.get_radar_longitude_deg();
      _radarAltKm = info.get_radar_altitude_m() / 1000.0;
      _sunPosn.setLocation(_radarLat, _radarLon, _radarAltKm / 1000.0);
    }

    bool useThisPulse = true;
    if (_params.check_scan_segment_name) {
      const iwrf_scan_segment_t &scanSeg = info.getScanSegment();
      string segName = scanSeg.segment_name;
      string requestedSegName = _params.scan_segment_name;
      if (segName != requestedSegName) {
        useThisPulse = false;
      }
    }

    if (useThisPulse) {
      _processPulse(pulse);
    } else {
      // delete pulse by adding to queue and then
      // clearing the queue
      _addPulseToQueue(pulse);
      _clearPulseQueue();
    }

    // process data if end of vol
    
    if (_endOfVol) {
      if (_performAnalysis(false)) {
        return -1;
      }
      _endOfVol = false;
    }

  } // while

  // process any remaining data

  if (_performAnalysis(true)) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// Run for covariances analysis

int SunCal::_runForCovar()
{

  PMU_auto_register("_runForMoments");
  if (_params.debug) {
    cerr << "_runForMoments ..." << endl;
  }

  // read through the files

  int iret = 0;
  const char *filePath;
  while ((filePath = _covarReader->next()) != NULL) {

    if (_processCovarFile(filePath)) {
      iret = -1;
    }

  }

  return iret;

}

/////////////////////////
// create data readers

int SunCal::_createReaders()

{

  int iret = 0;

  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug) {
    iwrfDebug = IWRF_DEBUG_NORM;
  }
    
  if (_params.input_mode == Params::TS_REALTIME_DIR_INPUT) {

    _tsReader = new IwrfTsReaderFile(_params.input_dir, 300,
                                     PMU_auto_register, true, iwrfDebug);

  } else if (_params.input_mode == Params::TS_FILELIST_INPUT) {
    
    _tsReader = new IwrfTsReaderFile(_args.inputFileList, iwrfDebug);

  } else if (_params.input_mode == Params::TS_FMQ_INPUT) {

    _tsReader = new IwrfTsReaderFmq(_params.input_fmq_name, iwrfDebug);
    
  } else if (_params.input_mode == Params::COVAR_REALTIME_INPUT) {
    
    _covarReader = new DsInputPath(_progName,
                                   _params.debug >= Params::DEBUG_VERBOSE,
                                   _params.input_dir,
                                   600, PMU_auto_register, true);

  } else if (_params.input_mode == Params::COVAR_ARCHIVE_INPUT) {

    date_time_t start;
    if (sscanf(_params.archive_start_time, "%d %d %d %d %d %d",
               &start.year, &start.month, &start.day,
               &start.hour, &start.min, &start.sec) != 6) {
      cerr << "ERROR - SunCal" << endl;
      cerr << "  Bad date/time for parameter archive_start_time" << endl;
      cerr << "  Set to: " << _params.archive_start_time << endl;
      cerr << "  Format is \"yyyy mm dd hh mm ss\"" << endl;
      iret = -1;
    }
    date_time_t end;
    if (sscanf(_params.archive_end_time, "%d %d %d %d %d %d",
               &end.year, &end.month, &end.day,
               &end.hour, &end.min, &end.sec) != 6) {
      cerr << "ERROR - SunCal" << endl;
      cerr << "  Bad date/time for parameter archive_end_time" << endl;
      cerr << "  Set to: " << _params.archive_end_time << endl;
      cerr << "  Format is \"yyyy mm dd hh mm ss\"" << endl;
      iret = -1;
    }
    _covarReader = new DsInputPath(_progName,
                                   _params.debug >= Params::DEBUG_VERBOSE,
                                   _params.input_dir,
                                   start.unix_time, end.unix_time);
    
  } else if (_params.input_mode == Params::COVAR_FILELIST_INPUT) {

    if (_args.inputFileList.size() > 0) {
      _covarReader = new DsInputPath(_progName,
                                     _params.debug >= Params::DEBUG_VERBOSE,
                                     _args.inputFileList);
    } else {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In FILELIST mode you must specify the files using -f arg." << endl;
      _args.usage(_progName, cerr);
      iret = -1;
    }

  }

  return iret;

}

/////////////////////
// process a pulse

int SunCal::_processPulse(const IwrfTsPulse *pulse)

{

  PMU_auto_register("processPulse");

  // at start, print headers
  
  if (_totalPulseCount == 0) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      _printRunDetails(cerr);
      _printOpsInfo(cerr);
      _printMomentsLabels(cerr);
    }
    _startTime = pulse->getFTime();
  }

  double pulseTime = pulse->getFTime();
  if (_endTime != 0) {
    double timeGap = pulseTime - _endTime;
    if (timeGap > _params.max_time_gap_within_volume) {
      if (_params.debug) {
        cerr << "Interpulse time exceeds max time gap" << endl;
        cerr << "  Gap found (secs)  : " << timeGap << endl;
        cerr << "  Max allowed (secs): "
             << _params.max_time_gap_within_volume << endl;
      }
      _volCount++;
      _endTime = pulseTime;
      _performAnalysis(true);
      if (_params.debug) {
        cerr << "Starting new analysis" << endl;
      }
      // add to queue for next analysis
      _addPulseToQueue(pulse);
      return 0;
    }
  }

  _endTime = pulseTime;

  // add the pulse to the queue
  
  _addPulseToQueue(pulse);
  _totalPulseCount++;
  
  // do we have a full pulse queue?

  if ((int) _pulseQueue.size() < _params.n_samples) {
    if (_totalPulseCount % 1000 == 0 && 
        _totalPulseCount > _params.n_samples) {
      cerr << "WARNING - pulses are not being processed" << endl;
      cerr << "  Check for fixed angle error in antenna pointing" << endl;
    }
    return 0;
  }

  // find pulses around mid point of queue

  int midIndex0 = _params.n_samples / 2;
  int midIndex1 = midIndex0 + 1;
  const IwrfTsPulse *pulse0 = _pulseQueue[midIndex0];
  const IwrfTsPulse *pulse1 = _pulseQueue[midIndex1];
  
  // compute angles at mid pulse
  
  double az0 = pulse0->getAz();
  double az1 = pulse1->getAz();

  double el0 = pulse0->getEl();
  double el1 = pulse1->getEl();

  // adjust angle if they cross north

  _checkForNorthCrossing(az0, az1);
  
  // order the azimuths

  if (az0 > az1) {
    double tmp = az0;
    az0 = az1; 
    az1 = tmp;
  }
  
  double az = RadarComplex::meanDeg(az0, az1);
  if (az < 0) {
    az += 360.0;
  }
  double el = RadarComplex::meanDeg(pulse0->getEl(), pulse1->getEl());
  double cosel = cos(el * DEG_TO_RAD);

  // compute angles relative to sun
  
  _midTime = (pulse0->getFTime() + pulse1->getFTime()) / 2.0;
  if (_params.specify_fixed_target_location) {
    _targetEl = _params.target_elevation;
    _targetAz = _params.target_azimuth;
  } else {
    _sunPosn.computePosnNova(_midTime, _targetEl, _targetAz);
  }
  _offsetEl = RadarComplex::diffDeg(el, _targetEl);
  _offsetAz = RadarComplex::diffDeg(az, _targetAz) * cosel;

  // set other properties at mid pulse

  _midPrt = pulse0->getPrt();
  _midEl = el;
  _midAz = az;

  int angleIndex = 0;

  if (_isRhi) {

    // compute offset el for mid pulses
    
    double offsetEl0 = RadarComplex::diffDeg(el0, _targetEl);
    double offsetEl1 = RadarComplex::diffDeg(el1, _targetEl);
    
    // compute grid el closest to the offset el
    
    double roundedOffsetEl =
      (floor (offsetEl0 / _gridDeltaEl + 0.5)) * _gridDeltaEl;
    
    // have we moved far enough?
    
    if (fabs(offsetEl0 - _prevAngleOffset) < _gridDeltaEl / 2) {
      return 0;
    }
    
    // is the elevation correct?
    
    if (offsetEl0 > roundedOffsetEl || offsetEl1 < roundedOffsetEl) {
      return 0;
    }
    
    // is this elevation contained in the grid?
    
    _offsetEl = roundedOffsetEl;
    _prevAngleOffset = _offsetEl;
    
    angleIndex = (int) ((_offsetEl - _gridMinEl) / _gridDeltaEl + 0.5);
    if (angleIndex < 0 || angleIndex > _gridNEl - 1) {
      return 0;
    }

  } else {

    // compute offset az for mid pulses
    
    double offsetAz0 = RadarComplex::diffDeg(az0, _targetAz) * cosel;
    double offsetAz1 = RadarComplex::diffDeg(az1, _targetAz) * cosel;
    
    // compute grid az closest to the offset az
    
    double roundedOffsetAz =
      (floor (offsetAz0 / _gridDeltaAz + 0.5)) * _gridDeltaAz;
    
    // have we moved far enough?
    
    if (fabs(offsetAz0 - _prevAngleOffset) < _gridDeltaAz / 2) {
      return 0;
    }
    
    // is the azimuth correct?
    
    if (offsetAz0 > roundedOffsetAz || offsetAz1 < roundedOffsetAz) {
      return 0;
    }
    
    // is this azimuth contained in the grid?
    
    _offsetAz = roundedOffsetAz;
    _prevAngleOffset = _offsetAz;
    
    angleIndex = (int) ((_offsetAz - _gridMinAz) / _gridDeltaAz + 0.5);
    if (angleIndex < 0 || angleIndex > _gridNAz - 1) {
      return 0;
    }

  } // if (_isRhi) 

  // ensure queue starts on H transmit if alternating mode

  if (_checkAlternatingStartsOnH()) {
    cerr << "ERROR - _processPulse()" << endl;
    cerr << "  Cannot get pulse queue to start on H" << endl;
    return -1;
  }

  // load IQ data into gates
  
  if (_loadGateData()) {
    cerr << "ERROR - _processPulse()" << endl;
    cerr << "  Bad IQ data" << endl;
    return -1;
  }

  // compute moments at all gates

  _computeMomentsAllGates();

  // compute moments, add to raw moments array
  
  MomentsSun *moments = new MomentsSun();
  
  if (_computeMomentsSun(moments, _startGateSun, _endGateSun)) {
    delete moments;
  } else {
    _rawMoments[angleIndex].push_back(moments);
  }

  // add to xpol data

  if (_params.compute_cross_polar_power_ratio) {
    
    double elOffset = _midEl - _targetEl;
    double azOffset = _midAz - _targetAz;
    double offset = sqrt(elOffset * elOffset + azOffset * azOffset);
    
    if (offset > _params.min_angle_offset_for_cross_pol_ratio) {
      int startGateXpol = _params.cross_polar_start_gate;
      int endGateXpol = startGateXpol + _params.cross_polar_n_gates - 1;
      if (endGateXpol > _nGates - 1) {
        endGateXpol = _nGates - 1;
      }
      _accumForXpol(startGateXpol, endGateXpol);
    }

  }

  // add to test pulse data

  if (_params.compute_test_pulse_powers) {
    _accumForTestPulse();
  }

  // add to xmit power data

  if (_params.compute_mean_transmit_powers) {
    _accumForXmitPowers(pulse);
  }

  // check for end of volume

  _checkEndOfVol(_midEl);
  if (_endOfVol) {
    _volCount++;
    if (_params.debug) {
      cerr << "==>> volCount so far: " << _volCount << endl;
    }
  }
  
  _nBeamsThisVol++;

  return 0;
  
}

/////////////////////////////////////////////////
// add the pulse to the pulse queue
    
void SunCal::_addPulseToQueue(const IwrfTsPulse *pulse)
  
{

  // check pointing error

  double pointingError = 0.0;
  if (_isRhi) {
    if (pulse->getFixedAz() > -9000) {
      pointingError = pulse->getAz() - pulse->getFixedAz();
    }
  } else {
    if (pulse->getFixedEl() > -9000) {
      pointingError = pulse->getEl() - pulse->getFixedEl();
    }
  }
  if (fabs(pointingError) > _params.max_pointing_angle_error_deg) {
    // discard pulse because not part of main scan
    delete pulse;
    return;
  }

  // manage the size of the pulse queue, popping off the back

  if ((int) _pulseQueue.size() >= _params.n_samples) {
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
    if (fabs(azDiff) > 2.0 || fabs(elDiff) > 0.25) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "====>> Rapid change in angle  - addPulseToQueue() ====<<" << endl;
        cerr << "  az, prevAz: " << az << ", " << prevAz << endl;
        cerr << "  el, prevEl: " << el << ", " << prevEl << endl;
        cerr << "  azDiff, elDiff: " << azDiff << ", " << elDiff << endl;
        cerr << "====>> Clearing pulse queue" << endl;
      }
      _clearPulseQueue();
      // free NEXRAD queue
      if (_params.test_nexrad_processing) {
        nexradSolarClearPulseQueue();
      }
      return;
    }

  }
    
  // push pulse onto front of queue
  
  pulse->addClient();
  _pulseQueue.push_back(pulse);
  if (_params.test_nexrad_processing) {
    _addPulseToNexradQueue(pulse);
  }
  
  // print missing pulses in verbose mode
  
  if ((pulse->getSeqNum() - _pulseSeqNum) != 1) {
    if (_params.print_missing_pulses && _pulseSeqNum != 0) {
      cerr << "**** Missing seq num: " << _pulseSeqNum
	   << " to " <<  pulse->getSeqNum() << " ****" << endl;
    }
  }
  _pulseSeqNum = pulse->getSeqNum();

}

/////////////////////////////////////////////////
// add pulse to the nexrad pulse queue
    
int SunCal::_addPulseToNexradQueue(const IwrfTsPulse *iwrfPulse)
  
{

  if (_alternating) {
    bool isHoriz = iwrfPulse->isHoriz();
    // only use H transmit pulses in alternating mode
    if (!isHoriz) {
      return -1;
    }
  }

  // load up IQ data for sun gates

  int nGatesSun = _endGateSun - _startGateSun + 1;
  if (nGatesSun < 3) {
    return -1;
  }

  TaArray<RadarComplex_t> iqh_, iqv_;
  RadarComplex_t *iqh = iqh_.alloc(nGatesSun);
  RadarComplex_t *iqv = iqv_.alloc(nGatesSun);

  const fl32 *iq0 = iwrfPulse->getIq0();
  const fl32 *iq1 = iwrfPulse->getIq1();

  // load up IQ data along the pulse
  
  int count = 0;
  int ii2 = _startGateSun * 2;
  for (int igate = _startGateSun; igate <= _endGateSun; igate++, ii2 += 2) {
    
    iqh[count].re = iq0[ii2];
    iqh[count].im = iq0[ii2 + 1];
    
    iqv[count].re = iq1[ii2];
    iqv[count].im = iq1[ii2 + 1];

    // check power for interference
    
    double power =
      (RadarComplex::power(iqh[count]) + RadarComplex::power(iqv[count])) / 2.0;
    double dbm = 10.0 * log10(power);
    if (dbm <= _maxValidSunPowerDbm) {
      // probably not interference so use this gate
      // so increment count
      count++;
    }

  } // igate

  if (count < 3) {
    return -1;
  }

  // compute moments
  
  double meanPowerH = RadarComplex::meanPower(iqh, count);
  double meanPowerV = RadarComplex::meanPower(iqv, count);
  RadarComplex_t Rvvhh0 =
    RadarComplex::meanConjugateProduct(iqh, iqv, count);
  
  solar_pulse_t solarPulse;
  solarPulse.time = iwrfPulse->getFTime();
  solarPulse.el = iwrfPulse->getEl();
  solarPulse.az = iwrfPulse->getAz();
  solarPulse.powerH = meanPowerH;
  solarPulse.powerV = meanPowerV;
  solarPulse.rvvhh0.re = Rvvhh0.re;
  solarPulse.rvvhh0.im = Rvvhh0.im;
  solarPulse.nGatesUsed = count;

  nexradSolarAddPulseToQueue(&solarPulse);

  return 0;

}


/////////////////////////////////////////////////
// clear the pulse queue
    
void SunCal::_clearPulseQueue()
  
{

  for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
    if (_pulseQueue[ii]->removeClient() == 0) {
      delete _pulseQueue[ii];
    }
  }
  _pulseQueue.clear();

}

/////////////////////////////////////////////////
// delete the pulse queue, force memory freeing
    
void SunCal::_deletePulseQueue()
  
{

  for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
    delete _pulseQueue[ii];
  }
  _pulseQueue.clear();

}

//////////////////////////////////////////////////
// Process a covariances file

int SunCal::_processCovarFile(const char *filePath)
{

  if (_params.debug) {
    cerr << "Processing moments file: " << filePath << endl;
  }

  // initialize for analysis

  _initForAnalysis();

  // read in file

  RadxFile inFile;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
   inFile.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    inFile.setVerbose(true);
  }
  RadxVol vol;
  if (inFile.readFromPath(filePath, vol)) {
    cerr << "ERROR - SunCal::_processCovarFile" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  // increment beam count

  _nBeamsThisVol = vol.getNRays();
  if (_nBeamsThisVol < _params.min_beams_per_volume) {
    cerr << "ERROR - SunCal::_processCovarFile" << endl;
    cerr << "  too few rays in file:: " << filePath << endl;
    cerr << "  min nRays required: " << _params.min_beams_per_volume << endl;
    cerr << "      nRays found   : " << _nBeamsThisVol << endl;
    return -1;
  }

  _volNum = vol.getVolumeNumber();
  _isRhi = vol.checkIsRhi();

  _startTime = vol.getStartTimeSecs();
  _endTime = vol.getEndTimeSecs();

  // initialize sun posn object

  _radarLat = vol.getLatitudeDeg();
  _radarLon = vol.getLongitudeDeg();
  _radarAltKm = vol.getAltitudeKm();
  _sunPosn.setLocation(_radarLat, _radarLon, _radarAltKm / 1000.0);

  vector<RadxRay *> &rays = vol.getRays();
  RadxRay *ray0 = rays[0];
  _volCount = 1;
  _nGates = ray0->getNGates();
  _startRangeKm = ray0->getStartRangeKm();
  _gateSpacingKm = ray0->getGateSpacingKm();
  _startGateSun = _params.start_gate;
  _endGateSun = _startGateSun + _params.n_gates - 1;
  if (_endGateSun > _nGates - 1) {
    _endGateSun = _nGates - 1;
  }
  if (ray0->getPolarizationMode() == Radx::POL_MODE_HV_ALT) {
    _alternating = TRUE;
  } else {
    _alternating = FALSE;
  }

  // loop through rays

  for (size_t iray = 0; iray < rays.size(); iray++) {
    RadxRay *ray = rays[iray];
    ray->convertToFl32();
    if (_processCovarRay(iray, ray)) {
      cerr << "ERROR - SunCal::_processCovarFile" << endl;
      cerr << "  Cannot process ray, el, az:"
           << ray->getElevationDeg() << ", "
           << ray->getAzimuthDeg() << endl;
      return -1;
    }
  }
  
  // perform analysis

  if (_performAnalysis(false)) {
    return -1;
  }
    
  return 0;

}

//////////////////////////////////////////////////
// Process a covariance ray

int SunCal::_processCovarRay(size_t rayIndex, RadxRay *ray)
{

  // init moments object

  MomentsSun mom;
  mom.time = ray->getTimeSecs();
  mom.prt = ray->getPrtSec();
  mom.az = ray->getAzimuthDeg();
  mom.el = ray->getElevationDeg();

  // compute mean moments along ray

  if (_computeCovarMoments(ray, mom)) {
    return -1;
  }

  // compute the offset relative to the sun

  if (_params.specify_fixed_target_location) {
    _targetEl = _params.target_elevation;
    _targetAz = _params.target_azimuth;
  } else {
    _sunPosn.computePosnNova(ray->getTimeDouble(), _targetEl, _targetAz);
  }
  double cosElTarget = cos(_targetEl * DEG_TO_RAD);
  _offsetEl = RadarComplex::diffDeg(mom.el, _targetEl);
  _offsetAz = RadarComplex::diffDeg(mom.az, _targetAz) * cosElTarget;
  mom.offsetEl = _offsetEl;
  mom.offsetAz = _offsetAz;
  
  // if first ray, save moments and return now

  if (rayIndex == 0) {
    _prevRawMoments = mom;
    _prevOffsetEl = _offsetEl;
    _prevOffsetAz = _offsetAz;
    return 0;
  }

  if (_isRhi) {

    // RHI mode

    // interp onto the grid in elevation

    // order the consecutive pair of moments
    
    MomentsSun *momLower, *momUpper;
    double offsetElLower, offsetElUpper;
    if (_offsetEl > _prevOffsetEl) {
      momLower = &_prevRawMoments;
      momUpper = &mom;
      offsetElLower = _prevOffsetEl;
      offsetElUpper = _offsetEl;
    } else {
      momUpper = &_prevRawMoments;
      momLower = &mom;
      offsetElUpper = _prevOffsetEl;
      offsetElLower = _offsetEl;
    }
    
    // compute grid el closest to the offset el

    int elIndexLower = (int) ((offsetElLower - _gridMinEl) / _gridDeltaEl + 0.5);
    int elIndexUpper = (int) ((offsetElUpper - _gridMinEl) / _gridDeltaEl + 0.5);
    
    // have we moved across a grid cell boundary?
    // if not, return now.

    if (elIndexLower == elIndexUpper) {
      _prevRawMoments = mom;
      _prevOffsetEl = _offsetEl;
      _prevOffsetAz = _offsetAz;
      return 0;
    }

    // interp to grid in el

    for (int elIndex = elIndexLower; elIndex < elIndexUpper; elIndex++) {

      double gridEl = _gridMinEl + elIndex * _gridDeltaEl;
      double wtUpper = (gridEl - momLower->el) / (momUpper->el - momLower->el);
      double wtLower = 1.0 - wtUpper;
      
      MomentsSun *elMom = new MomentsSun;
      elMom->interp(*momLower, *momUpper, wtLower, wtUpper);
      int azIndex = (int) ((elMom->az - _gridMinAz) / _gridDeltaAz + 0.5);
      if (azIndex >= 0 && azIndex < _gridNAz) {
        _rawMoments[azIndex].push_back(elMom);
      } else {
        delete elMom;
      }
      
      int elIndex2 = (int) ((elMom->el - _gridMinEl) / _gridDeltaEl + 0.5);

      if (elIndex2 >= 0 && elIndex2 < _gridNEl) {
        _rawMoments[elIndex].push_back(elMom);
      } else {
        delete elMom;
      }
      

    } // elIndex

  } else {

    // PPI mode

    // check for error relative to fixed angle

    double pointingError = fabs(ray->getElevationDeg() - ray->getFixedAngleDeg());
    if (pointingError > _params.max_pointing_angle_error_deg) {
      _prevRawMoments = mom;
      _prevOffsetEl = _offsetEl;
      _prevOffsetAz = _offsetAz;
      return 0;
    }
  
    // interp onto the grid in azimuth

    // order the consecutive pair of moments
    
    MomentsSun momLeft, momRight;
    double offsetAzLeft, offsetAzRight;
    if (_offsetAz > _prevOffsetAz) {
      momLeft = _prevRawMoments;
      momRight = mom;
      offsetAzLeft = _prevOffsetAz;
      offsetAzRight = _offsetAz;
    } else {
      momRight = _prevRawMoments;
      momLeft = mom;
      offsetAzRight = _prevOffsetAz;
      offsetAzLeft = _offsetAz;
    }
    
    // compute grid az closest to the offset az

    int azIndexLeft = (int) ((offsetAzLeft - _gridMinAz) / _gridDeltaAz + 0.5);
    int azIndexRight = (int) ((offsetAzRight - _gridMinAz) / _gridDeltaAz + 0.5);

    // have we moved across a grid cell boundary?
    // if not, return now.

    if (azIndexLeft == azIndexRight) {
      _prevRawMoments = mom;
      _prevOffsetEl = _offsetEl;
      _prevOffsetAz = _offsetAz;
      return 0;
    }

    // interp to grid in az

    for (int azIndex = azIndexLeft; azIndex < azIndexRight; azIndex++) {

      double gridAz = _gridMinAz + azIndex * _gridDeltaAz;
      double wtRight = 
        (gridAz - momLeft.offsetAz) / (momRight.offsetAz - momLeft.offsetAz);
      double wtLeft = 1.0 - wtRight;
      
      MomentsSun *azMom = new MomentsSun;
      azMom->interp(momLeft, momRight, wtLeft, wtRight);
      
      int azIndex2 = (int) ((azMom->offsetAz - _gridMinAz) / _gridDeltaAz + 0.5);

      if (azIndex2 >= 0 && azIndex2 < _gridNAz) {
        _rawMoments[azIndex2].push_back(azMom);
      } else {
        delete azMom;
      }

    } // azIndex

  } // if (_isRhi) 

  // save prev values for next time

  _prevRawMoments = mom;
  _prevOffsetEl = _offsetEl;
  _prevOffsetAz = _offsetAz;

  return 0;

}

//////////////////////////////////////////////////
// Compute moments from covariances

int SunCal::_computeCovarMoments(RadxRay *ray,
                                 MomentsSun &mom)
{
  
  mom.time = ray->getTimeSecs();
  mom.prt = ray->getPrtSec();
  mom.az = ray->getAzimuthDeg();
  mom.el = ray->getElevationDeg();

  // compute the offset relative to the sun
  
  if (_params.specify_fixed_target_location) {
    _targetEl = _params.target_elevation;
    _targetAz = _params.target_azimuth;
  } else {
    _sunPosn.computePosnNova(ray->getTimeDouble(), _targetEl, _targetAz);
  }
  double cosElTarget = cos(_targetEl * DEG_TO_RAD);
  _offsetEl = RadarComplex::diffDeg(mom.el, _targetEl);
  _offsetAz = RadarComplex::diffDeg(mom.az, _targetAz) * cosElTarget;
  mom.offsetEl = _offsetEl;
  mom.offsetAz = _offsetAz;
  
  RadxField *lag0_hc_db_fld = ray->getField(_params.covar_field_names.LAG0_HC_DB);
  RadxField *lag0_vc_db_fld = ray->getField(_params.covar_field_names.LAG0_VC_DB);
  RadxField *lag0_hx_db_fld = ray->getField(_params.covar_field_names.LAG0_HX_DB);
  RadxField *lag0_vx_db_fld = ray->getField(_params.covar_field_names.LAG0_VX_DB);
  
  RadxField *lag0_hcvx_db_fld =
    ray->getField(_params.covar_field_names.LAG0_HCVX_DB);
  RadxField *lag0_hcvx_phase_fld =
    ray->getField(_params.covar_field_names.LAG0_HCVX_PHASE);

  RadxField *lag0_vchx_db_fld =
    ray->getField(_params.covar_field_names.LAG0_VCHX_DB);
  RadxField *lag0_vchx_phase_fld =
    ray->getField(_params.covar_field_names.LAG0_VCHX_PHASE);

  RadxField *lag1_vxhx_db_fld =
    ray->getField(_params.covar_field_names.LAG1_VXHX_DB);
  RadxField *lag1_vxhx_phase_fld =
    ray->getField(_params.covar_field_names.LAG1_VXHX_PHASE);

  RadxField *rvvhh0_db_fld =
    ray->getField(_params.covar_field_names.RVVHH0_DB);
  RadxField *rvvhh0_phase_fld =
    ray->getField(_params.covar_field_names.RVVHH0_PHASE);

  Radx::fl32 *lag0_hc_db = NULL;
  Radx::fl32 *lag0_vc_db = NULL;
  Radx::fl32 *lag0_hx_db = NULL;
  Radx::fl32 *lag0_vx_db = NULL;
  Radx::fl32 *lag0_hcvx_db = NULL;
  Radx::fl32 *lag0_hcvx_phase = NULL;
  Radx::fl32 *lag0_vchx_db = NULL;
  Radx::fl32 *lag0_vchx_phase = NULL;
  Radx::fl32 *lag1_vxhx_db = NULL;
  Radx::fl32 *lag1_vxhx_phase = NULL;
  Radx::fl32 *rvvhh0_db = NULL;
  Radx::fl32 *rvvhh0_phase = NULL;

  _dualPol = false;

  if (lag0_hc_db_fld) {
    lag0_hc_db = lag0_hc_db_fld->getDataFl32();
  }
  if (lag0_vc_db_fld) {
    lag0_vc_db = lag0_vc_db_fld->getDataFl32();
    _dualPol = true;
  }
  if (lag0_hx_db_fld) {
    lag0_hx_db = lag0_hx_db_fld->getDataFl32();
    _dualPol = true;
  }
  if (lag0_vx_db_fld) {
    lag0_vx_db = lag0_vx_db_fld->getDataFl32();
    _dualPol = true;
  }

  if (lag0_hcvx_db_fld) {
    lag0_hcvx_db = lag0_hcvx_db_fld->getDataFl32();
    _dualPol = true;
  }
  if (lag0_hcvx_phase_fld) {
    lag0_hcvx_phase = lag0_hcvx_phase_fld->getDataFl32();
    _dualPol = true;
  }

  if (lag0_vchx_db_fld) {
    lag0_vchx_db = lag0_vchx_db_fld->getDataFl32();
    _dualPol = true;
  }
  if (lag0_vchx_phase_fld) {
    lag0_vchx_phase = lag0_vchx_phase_fld->getDataFl32();
    _dualPol = true;
  }

  bool haveVxHx = true;
  if (lag1_vxhx_db_fld) {
    lag1_vxhx_db = lag1_vxhx_db_fld->getDataFl32();
    _dualPol = true;
  } else {
    haveVxHx = false;
  }
  if (lag1_vxhx_phase_fld) {
    lag1_vxhx_phase = lag1_vxhx_phase_fld->getDataFl32();
    _dualPol = true;
  } else {
    haveVxHx = false;
  }

  if (rvvhh0_db_fld) {
    rvvhh0_db = rvvhh0_db_fld->getDataFl32();
    _dualPol = true;
  }
  if (rvvhh0_phase_fld) {
    rvvhh0_phase = rvvhh0_phase_fld->getDataFl32();
    _dualPol = true;
  }
  
  // initialize summation quantities
  
  double sumPowerHc = 0.0;
  double sumPowerHx = 0.0;
  double sumPowerVx = 0.0;
  double sumPowerVc = 0.0;
  RadarComplex_t sumRvvhh0Hc(0.0, 0.0);
  RadarComplex_t sumRvvhh0Vc(0.0, 0.0);
  double nn = 0.0;
  
  // loop through gates to be used for sun computations

  for (int igate = _startGateSun; igate <= _endGateSun; igate++, nn++) {
    
    // check power for interference
    double dbmHc = lag0_hc_db[igate];
    if (dbmHc > _maxValidSunPowerDbm) {
      // don't use this gate - probably interference
      continue;
    }
    
    double lag0_hc = 0;
    double lag0_vc = 0;
    double lag0_hx = 0;
    double lag0_vx = 0;

    if (lag0_hc_db) {
      lag0_hc = pow(10.0, lag0_hc_db[igate] / 10.0);
    }
    if (lag0_vc_db) {
      lag0_vc = pow(10.0, lag0_vc_db[igate] / 10.0);
    }
    if (lag0_hx_db) {
      lag0_hx = pow(10.0, lag0_hx_db[igate] / 10.0);
    }
    if (lag0_vx_db) {
      lag0_vx = pow(10.0, lag0_vx_db[igate] / 10.0);
    }

    double lag0_hcvxMag = 0;
    double lag0_hcvxPhase = 0;
    if (lag0_hcvx_db) {
      lag0_hcvxMag = pow(10.0, lag0_hcvx_db[igate] / 20.0);
    }
    if (lag0_hcvx_phase) {
      lag0_hcvxPhase = lag0_hcvx_phase[igate] * DEG_TO_RAD;
    }
    double sinval, cosval;
    ta_sincos(lag0_hcvxPhase, &sinval, &cosval);
    RadarComplex_t lag0_hcvx;
    lag0_hcvx.set(lag0_hcvxMag * cosval, lag0_hcvxMag * sinval);

    double lag0_vchxMag = 0;
    double lag0_vchxPhase = 0;
    if (lag0_vchx_db) {
      lag0_vchxMag = pow(10.0, lag0_vchx_db[igate] / 20.0);
    }
    if (lag0_vchx_phase) {
      lag0_vchxPhase = lag0_vchx_phase[igate] * DEG_TO_RAD;
    }
    ta_sincos(lag0_vchxPhase, &sinval, &cosval);
    RadarComplex_t lag0_vchx;
    lag0_vchx.set(lag0_vchxMag * cosval, lag0_vchxMag * sinval);

    double rvvhh0Mag = 0;
    double rvvhh0Phase = 0;
    if (rvvhh0_db) {
      rvvhh0Mag = pow(10.0, rvvhh0_db[igate] / 20.0);
    }
    if (rvvhh0_phase) {
      rvvhh0Phase = rvvhh0_phase[igate] * DEG_TO_RAD;
    }
    ta_sincos(rvvhh0Phase, &sinval, &cosval);
    RadarComplex_t rvvhh0;
    rvvhh0.set(rvvhh0Mag * cosval, rvvhh0Mag * sinval);

    sumPowerHc += lag0_hc;
    sumPowerVc += lag0_vc;
    sumPowerHx += lag0_hx;
    sumPowerVx += lag0_vx;
    
    if (_alternating) {
      sumRvvhh0Hc = RadarComplex::complexSum(sumRvvhh0Hc, lag0_vchx);
      sumRvvhh0Vc = RadarComplex::complexSum(sumRvvhh0Vc, lag0_hcvx);
    } else {
      sumRvvhh0Hc = RadarComplex::complexSum(sumRvvhh0Hc, rvvhh0);
      sumRvvhh0Vc = RadarComplex::complexSum(sumRvvhh0Vc, rvvhh0);
    }

  } // igate
  
  if (nn < 3) {
    cerr << "Warning - processCovarMoments()" << endl;
    cerr << "  Insufficient good data found" << endl;
    cerr << "  az, el: " << _midAz << ", " << _midEl << endl;
    cerr << "  nn: " << nn << endl;
    return -1;
  }
  
  // compute moments

  mom.nn = nn;

  double meanPowerHc = sumPowerHc / nn;
  double meanPowerVc = sumPowerVc / nn;
  double meanPowerHx = sumPowerHx / nn;
  double meanPowerVx = sumPowerVx / nn;

  mom.powerHc = meanPowerHc;
  mom.powerVc = meanPowerVc;
  mom.powerHx = meanPowerHx;
  mom.powerVx = meanPowerVx;

  mom.dbmHc = 10.0 * log10(meanPowerHc);
  mom.dbmVc = 10.0 * log10(meanPowerVc);
  mom.dbmHx = 10.0 * log10(meanPowerHx);
  mom.dbmVx = 10.0 * log10(meanPowerVx);
  
  mom.dbm = (mom.dbmHc + mom.dbmVc +
             mom.dbmHx + mom.dbmVx) / 4.0;

  mom.sumRvvhh0Hc = sumRvvhh0Hc;
  mom.sumRvvhh0Vc = sumRvvhh0Vc;

  RadarComplex_t sumRvvhh0 =
    RadarComplex::complexMean(sumRvvhh0Hc, sumRvvhh0Vc);
  mom.sumRvvhh0 = sumRvvhh0;
  
  mom.Rvvhh0Hc = RadarComplex::mean(sumRvvhh0Hc, nn);
  mom.Rvvhh0Vc = RadarComplex::mean(sumRvvhh0Vc, nn);
  mom.Rvvhh0 = RadarComplex::mean(sumRvvhh0, nn);
  
  double corrMagHc = RadarComplex::mag(sumRvvhh0Hc) / nn;
  mom.corrMagHc = corrMagHc;
  double corrMagVc = RadarComplex::mag(sumRvvhh0Vc) / nn;
  mom.corrMagVc = corrMagVc;
  mom.corrMag = (corrMagHc + corrMagVc) / 2.0;
  
  double corr00Hc = corrMagHc / sqrt(meanPowerHc * meanPowerVx);
  mom.corr00Hc = corr00Hc;
  double corr00Vc = corrMagVc / sqrt(meanPowerVc * meanPowerHx);
  mom.corr00Vc = corr00Vc;
  mom.corr00 = (corr00Hc + corr00Vc) / 2.0;

  mom.arg00Hc = RadarComplex::argDeg(sumRvvhh0Hc);
  mom.arg00Vc = RadarComplex::argDeg(sumRvvhh0Vc);
  mom.arg00 = RadarComplex::argDeg(sumRvvhh0);

  // can we compute cross-polar power ratio?

  double offset = sqrt(_offsetEl * _offsetEl + _offsetAz * _offsetAz);
  if (_params.compute_cross_polar_power_ratio && _alternating &&
      offset > _params.min_angle_offset_for_cross_pol_ratio) {
    
    // Compute xpol values, add to array
    
    int startGateXpol = _params.cross_polar_start_gate;
    int endGateXpol = startGateXpol + _params.cross_polar_n_gates - 1;
    if (endGateXpol > _nGates - 1) {
      endGateXpol = _nGates - 1;
    }
    
    double count = 0;
    double sumPowerHxXpol = 0.0;
    double sumPowerVxXpol = 0.0;
    RadarComplex_t sumRvxhx(0.0, 0.0);

    for (int igate = startGateXpol; igate <= endGateXpol; igate++, count++) {
      
      double lag0_hx_xpol = 0;
      double lag0_vx_xpol = 0;
      
      if (lag0_hx_db) {
        lag0_hx_xpol = pow(10.0, lag0_hx_db[igate] / 10.0);
      }
      if (lag0_vx_db) {
        lag0_vx_xpol = pow(10.0, lag0_vx_db[igate] / 10.0);
      }
      
      sumPowerHxXpol += lag0_hx_xpol;
      sumPowerVxXpol += lag0_vx_xpol;
      
      double lag1_vxhxMag = 0;
      double lag1_vxhxPhase = 0;
      if (lag1_vxhx_db) {
        lag1_vxhxMag = pow(10.0, lag1_vxhx_db[igate] / 20.0);
      }
      if (lag1_vxhx_phase) {
        lag1_vxhxPhase = lag1_vxhx_phase[igate] * DEG_TO_RAD;
      }
      double sinval, cosval;
      ta_sincos(lag1_vxhxPhase, &sinval, &cosval);
      RadarComplex_t lag1_vxhx;
      lag1_vxhx.set(lag1_vxhxMag * cosval, lag1_vxhxMag * sinval);
      sumRvxhx = RadarComplex::complexSum(sumRvxhx, lag1_vxhx);
      
    } // igate
    
    double meanPowerHxXpol = sumPowerHxXpol / count;
    double meanPowerVxXpol = sumPowerVxXpol / count;
    RadarComplex_t Rvxhx = RadarComplex::mean(sumRvxhx, count);

    double rhoVxHx = 1.0;
    if (haveVxHx) {
      rhoVxHx = RadarComplex::mag(Rvxhx) / sqrt(meanPowerVxXpol * meanPowerHxXpol);
    }
    Xpol xpol(meanPowerHxXpol, meanPowerVxXpol, rhoVxHx);
    _xpolMoments.push_back(xpol);
    
  } // if (_params.compute_cross_polar_power_ratio ...

  return 0;

}

///////////////////////////////////////////////////////////////////
// compute the mean for a given field, in the gate range specified
// in the params file

double SunCal::_computeFieldMean(const RadxField *field)

{

  if (field == NULL) {
    return MomentsSun::missing;
  }

  const Radx::fl32 *data = field->getDataFl32();
  if (data == NULL) {
    return MomentsSun::missing;
  }

  double sum = 0.0;
  double count = 0.0;
  int startGate = _params.start_gate;
  int endGate = startGate + _params.n_gates;
  if (endGate > (int) field->getNPoints() - 1) {
    endGate = field->getNPoints() - 1;
  }
  for (int igate = startGate; igate <= endGate; igate++) {
    if (data[igate] != Radx::missingFl32) {
      sum += data[igate];
      count++;
    }
  }

  if (count == 0) {
    return MomentsSun::missing;
  }

  double mean = sum / count;
  return mean;

}

///////////////////////////////////////////////////////////////////
// compute the mean of rvvhh0, in the gate range specified
// in the params file

RadarComplex_t SunCal::_computeRvvhh0Mean(const RadxField *rvvhh0_db,
                                          const RadxField *rvvhh0_phase,
                                          double &corrMag)

{

  RadarComplex_t mean(0.0, 0.0);

  if (rvvhh0_db == NULL || rvvhh0_phase == NULL) {
    return mean;
  }

  const Radx::fl32 *rvvhh0_db_vals = rvvhh0_db->getDataFl32();
  const Radx::fl32 *rvvhh0_phase_vals = rvvhh0_phase->getDataFl32();
  
  RadarComplex_t sum(0.0, 0.0);
  double count = 0.0;
  int startGate = _params.start_gate;
  int endGate = startGate + _params.n_gates;
  if (endGate > (int) rvvhh0_db->getNPoints() - 1) {
    endGate = rvvhh0_db->getNPoints() - 1;
  }
  for (int igate = startGate; igate <= endGate; igate++) {
    double db = rvvhh0_db_vals[igate];
    double mag = pow(10.0, db / 20.0);
    double phase = rvvhh0_phase_vals[igate] * DEG_TO_RAD;
    double sinPhase, cosPhase;
    ta_sincos(phase, &sinPhase, &cosPhase);
    sum.re += mag * cosPhase;
    sum.im += mag * sinPhase;
    count++;
  }

  if (count > 0) {
    mean.re = sum.re / count;
    mean.im = sum.im / count;
  }

  corrMag = RadarComplex::mag(sum) / count;

  return mean;

}

//////////////////////////////////////////////////
// initialize for analysis

void SunCal::_initForAnalysis()
{

  _totalPulseCount = 0;
  _volCount = 0;
  _startTime = 0;
  _endTime = 0;
  _sunCentroidAzOffset = 0;
  _sunCentroidElOffset = 0;
  _prevAngleOffset = -999;
  _validCentroid = false;

  _maxPowerDbm = -120;
  _quadPowerDbm = -120;

  _maxPowerDbmHc = -120;
  _quadPowerDbmHc = -120;

  _maxPowerDbmVc = -120;
  _quadPowerDbmVc = -120;

  _nBeamsThisVol = 0;
  
  // _clearPulseQueue();
  _clearRawMomentsArray();
  _clearInterpMomentsArray();
  _deleteXpolMomentsArray();
  _deleteTestPulseMomentsArray();
  _stats.clear();

  _testPulseDbmHc = MomentsSun::missing;
  _testPulseDbmVc = MomentsSun::missing;
  _testPulseDbmHx = MomentsSun::missing;
  _testPulseDbmVx = MomentsSun::missing;

  _sumXmitPowerHDbm = 0.0;
  _sumXmitPowerVDbm = 0.0;
  _countXmitPowerH = 0.0;
  _countXmitPowerV = 0.0;
  _meanXmitPowerHDbm = MomentsSun::missing;
  _meanXmitPowerVDbm = MomentsSun::missing;

}

//////////////////////////////////////////////////
// perform analysis on scans which have been saved
//
// Returns 0 on success, -1 on failure

int SunCal::_performAnalysis(bool force)
{

  PMU_auto_register("performAnalysis");
  int iret = 0;

  // check if we are ready

  if (_volCount < 1) {
    return 0;
  }
  
  if (!_params.analyze_individual_volumes) {
    if (_volCount < _params.min_n_volumes_for_analysis) {
      return 0;
    }
    if (!force && _volCount < _params.n_volumes_for_analysis) {
      return 0;
    }
  }

  double startRangeSun = _startRangeKm + _gateSpacingKm * _startGateSun;
  double endRangeSun = _startRangeKm + _gateSpacingKm * _endGateSun;

  if (_params.debug) {
    cerr << "----------------------------------------------------" << endl;
    cerr << "Performing analysis, n volumes: " << _volCount << endl;
    cerr << "  nGates        : " << _nGates << endl;
    cerr << "  startGateSun  : " << _startGateSun << endl;
    cerr << "  endGateSun    : " << _endGateSun << endl;
    cerr << "  startRangeSun : " << startRangeSun << endl;
    cerr << "  endRangeSun   : " << endRangeSun << endl;
    cerr << "----------------------------------------------------" << endl;
  }

  // set the sun location

  _sunPosn.setLocation(_radarLat, _radarLon, _radarAltKm * 1000.0);

  // compute cal time as mean of start and end time
  
  _calTime = (_startTime + _endTime) / 2.0;

  if (_params.debug) {
    cerr << "  startTime: " << DateTime::strm((time_t) _startTime) << endl;
    cerr << "  endTime: " << DateTime::strm((time_t) _endTime) << endl;
    cerr << "  calTime: " << DateTime::strm((time_t) _calTime) << endl;
  }
  
  if (_tsReader != NULL) {

    // time series processing,
    // sort the raw moments and interp onto a regular grid
    
    if (_isRhi) {
      _sortRawMomentsByAz();
      _interpMomentsRhi();
    } else {
      _sortRawMomentsByEl();
      _interpMomentsPpi();
    }
    
    // print raw moments
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      _printRawMomentsArray(cerr);
    }

  } else {

    // covariance processing
    // interp onto 2-D grid using quadrants

    if (_isRhi) {
      _sortRawMomentsByAz();
      _interpMomentsRhi();
    } else {
      _sortRawMomentsByEl();
      _interpMomentsPpi();
    }
    // _interpMomentsUsingQuadrants();

  }

  // compute the min power for each channel

  if (_params.noise_method == Params::GET_NOISE_FROM_CAL_FILE) {
    _getNoiseFromCalFile();
  } else if (_params.noise_method == Params::GET_NOISE_FROM_TIME_SERIES) {
    _getNoiseFromTimeSeries();
  } else if (_params.noise_method == Params::COMPUTE_MIN_NOISE) {
    _computeNoiseFromMinPower();
  } else {
    _computeMeanNoise();
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "_nBeamsNoise: " << _nBeamsNoise << endl;
    cerr << "_noiseDbmHc: " << _noiseDbmHc << endl;
    cerr << "_noiseDbmHx: " << _noiseDbmHx << endl;
    cerr << "_noiseDbmVc: " << _noiseDbmVc << endl;
    cerr << "_noiseDbmVx: " << _noiseDbmVx << endl;
  }

  // adjust powers for noise

  if (_params.correct_powers_for_noise) {
    _correctPowersForNoise();
  }

  // compute power ratios

  _computePowerRatios();

  // compute the max power

  _computeMaxPower();

  // compute mean sun location
  
  _sunPosn.computePosnNova(_calTime, _meanSunEl, _meanSunAz);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "========== mean sun location ==========" << endl;
    cerr << "  calTime: " << DateTime::strm((time_t) _calTime) << endl;
    cerr << "  _meanSunEl: " << _meanSunEl << endl;
    cerr << "  _meanSunAz: " << _meanSunAz << endl;
  }

  // compute sun centroid

  _widthRatioElAzHc = MomentsSun::missing;
  _widthRatioElAzVc = MomentsSun::missing;
  _widthRatioElAzDiffHV = MomentsSun::missing;

  if (!_params.specify_fixed_target_location) {
    _computeSunCentroid(channelHc);
    if (_dualPol) {
      _computeSunCentroid(channelVc);
      _computeSunCentroid(channelMeanc);
      if (_widthRatioElAzHc != MomentsSun::missing &&
          _widthRatioElAzVc != MomentsSun::missing) {
        _widthRatioElAzDiffHV = _widthRatioElAzHc - _widthRatioElAzVc;
      }
    }
  }

  if (_params.debug) {
    cerr << "============================" << endl;
    if (_dualPol) {
      cerr << "_sunCentroidAzOffsetHc: " << _sunCentroidAzOffsetHc << endl;
      cerr << "_sunCentroidElOffsetHc: " << _sunCentroidElOffsetHc << endl;
      cerr << "_sunCentroidAzOffsetVc: " << _sunCentroidAzOffsetVc << endl;
      cerr << "_sunCentroidElOffsetVc: " << _sunCentroidElOffsetVc << endl;
      cerr << "Stats for ellipses at -3dB: "
           << "widthRatioElAzHc, widthRatioElAzVc, widthRatioElAzDiffHV: "
           << _widthRatioElAzHc << ", "
           << _widthRatioElAzVc << ", "
           << _widthRatioElAzDiffHV << endl;

    }
    cerr << "_sunCentroidAzOffset: " << _sunCentroidAzOffset << endl;
    cerr << "_sunCentroidElOffset: " << _sunCentroidElOffset << endl;
    cerr << "============================" << endl;
  }
  
  // compute the mean correlation for sun disk

  if (_dualPol) {
    _computeSunCorr();
  }

  // compute results from moments

  if (_dualPol) {
    _computeSSUsingSolidAngle();
    if (_params.compute_ellipse_hv_power_diffs) {
      _computeEllipsePowerDiffsUsingSolidAngle();
    }
  }

  // get the xpol ratio and temp data from SPDB as approptiate

  if (_dualPol) {
    _zdrCorr = MomentsSun::missing;
    _retrieveXpolRatioFromSpdb(_startTime, _xpolRatioDbFromSpdb, _timeForXpolRatio);
    if (_params.read_site_temp_from_time_series_xml) {
      _retrieveSiteTempFromXml(_startTime, _siteTempC, _timeForSiteTemp);
    } else {
      _retrieveSiteTempFromSpdb(_startTime, _siteTempC, _timeForSiteTemp);
    }
    if (_params.use_xpol_ratio_from_spdb) {
      if (_statsForZdrBias.meanS1S2 > -99 && _xpolRatioDbFromSpdb > -99) {
        _zdrCorr = _statsForZdrBias.meanS1S2 + _xpolRatioDbFromSpdb;
      }
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "========== cross polar ratio from spdb ==========" << endl;
        cerr << "  xpolRatioDbFromSpdb: " << _xpolRatioDbFromSpdb << endl;
        cerr << "  _meanS1S2: " << _statsForZdrBias.meanS1S2 << endl;
        cerr << "  _zdrCorr: " << _zdrCorr << endl;
        cerr << "=================================================" << endl;
      }
    }
  }

  // compute cross-polar ratio
  
  if (_dualPol && _params.compute_cross_polar_power_ratio) {
    _computeXpolRatio(_xpolMoments);
  }
  
  if (_params.compute_test_pulse_powers) {
    _computeTestPulse();
  }

  if (_params.compute_mean_transmit_powers) {
    if (_countXmitPowerH > 0) {
      _meanXmitPowerHDbm = _sumXmitPowerHDbm / _countXmitPowerH;
      _meanXmitPowerVDbm = _sumXmitPowerVDbm / _countXmitPowerV;
    }
  }

  // perform nexrad analysis

  if (_params.test_nexrad_processing) {
    nexradSolarPerformAnalysis();
  }

  // write out results

  PMU_auto_register("writing results");

  if (_validCentroid || !_params.only_write_for_valid_centroid) {

    if (_params.write_text_files) {
      if (_writeSummaryText()) {
        iret = -1;
      }
    }
    
    if (_params.write_gridded_files) {
      if (_writeGriddedTextFiles()) {
        iret = -1;
      }
      if (_params.test_nexrad_processing) {
        nexradSolarWriteGriddedTextFiles(_params.nexrad_text_output_dir);
      }
    }
    
    if (_params.append_to_global_results_file) {
      _appendToGlobalResults();
    }
    
    if (_params.write_mdv_files) {
      if (_writeToMdv()) {
        iret = -1;
      }
      if (_params.test_nexrad_processing) {
        _writeNexradToMdv();
      }
    }
    
    if (_params.write_summary_to_spdb) {
      if (_writeSummaryToSpdb()) {
        iret = -1;
      }
      if (_params.test_nexrad_processing) {
        _writeNexradSummaryToSpdb();
      }
    }
    
  }

  // initialize for next analysis

  _initForAnalysis();
  _clearPulseQueue();

  return iret;

}

////////////////////////////////////////////
// compute moments at all gates

void SunCal::_computeMomentsAllGates()

{

  // alloc fields for each gate

  _fields.reserve(_nGates);
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii].init();
  }
  
  // initialize moments generation object
  
  RadarMoments rmom(_nGates,
                    _params.debug,
                    _params.debug >= Params::DEBUG_VERBOSE);
  
  const IwrfTsPulse *pulse0 = _pulseQueue[0];
  const IwrfTsInfo &info = _tsReader->getOpsInfo();
  rmom.init(pulse0->get_prt(),
            info.get_radar_wavelength_cm() / 100.0,
            pulse0->get_start_range_km(),
            pulse0->get_gate_spacing_km());

  _getNoiseFromCalFile();
  rmom.setCalib(_calib);
  rmom.setEstimatedNoiseDbmHc(_noiseDbmHc);
  rmom.setEstimatedNoiseDbmVc(_noiseDbmVc);
  rmom.setEstimatedNoiseDbmHx(_noiseDbmHx);
  rmom.setEstimatedNoiseDbmVx(_noiseDbmVx);
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    GateData *gate = _gateData[igate];
    
    if (_dualPol) {
      if (_alternating) {
        rmom.dpAltHvCoCross(gate->iqhc, gate->iqvc,
                            gate->iqhx, gate->iqvx,
                            igate, false, _fields[igate]);
      } else {
        rmom.dpSimHv(gate->iqhc, gate->iqvc,
                     igate, false, _fields[igate]);
      }
    } else {
      rmom.singlePolH(gate->iqhc, igate, false, _fields[igate]);
    }

  } // igate

}

////////////////////////////////////////////
// compute sun moments

int SunCal::_computeMomentsSun(MomentsSun *moments,
                               int startGate,
                               int endGate)

{
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Sun time, sunEl, sunAz, el, az, deltaEl, deltaAz: "
         << DateTime::strm((time_t) _midTime) << ", "
         << _midEl << ", "
         << _midAz << ", "
         << _targetEl << ", "
         << _midEl - _targetEl << ", "
         << _midAz - _targetAz << endl;
  }
  
  moments->time = _midTime;
  moments->prt =  _midPrt;
  moments->az = _midAz;
  moments->el = _midEl;
  moments->offsetAz = _offsetAz;
  moments->offsetEl = _offsetEl;

  if (_dualPol) {
    if (_alternating) {
      return _computeMomentsSunDualAlt(moments, startGate, endGate);
    } else {
      return _computeMomentsSunDualSim(moments, startGate, endGate);
    }
  } else {
    return _computeMomentsSunSinglePol(moments, startGate, endGate);
  }

  moments->ratioDbmHcHx = moments->dbmHc - moments->dbmHx;
  moments->ratioDbmVcHx = moments->dbmVc - moments->dbmHx;

  moments->ratioDbmVxHc = moments->dbmVx - moments->dbmHc;
  moments->ratioDbmVcHx = moments->dbmVc - moments->dbmHx;
  
  moments->ratioDbmVcHc = moments->dbmVc - moments->dbmHc;
  moments->ratioDbmVxHx = moments->dbmVx - moments->dbmHx;

}

///////////////////////////////////////////////////
// compute sun moments in dual-pol alternating mode

int SunCal::_computeMomentsSunDualAlt(MomentsSun *moments,
                                      int startGate,
                                      int endGate)

{
  
  // initialize summation quantities
  
  double sumPowerHc = 0.0;
  double sumPowerHx = 0.0;
  double sumPowerVx = 0.0;
  double sumPowerVc = 0.0;
  RadarComplex_t sumRvvhh0Hc(0.0, 0.0);
  RadarComplex_t sumRvvhh0Vc(0.0, 0.0);
  double nn = 0.0;

  // loop through gates to be used for sun computations

  for (int igate = startGate; igate <= endGate; igate++, nn++) {
    
    GateData *gate = _gateData[igate];
    
    const RadarComplex_t *iqhc = gate->iqhc;
    const RadarComplex_t *iqvc = gate->iqvc;
    const RadarComplex_t *iqhx = gate->iqhx;
    const RadarComplex_t *iqvx = gate->iqvx;
    
    double lag0_hc = RadarComplex::meanPower(iqhc, _nSamplesHalf - 1);
    double lag0_hx = RadarComplex::meanPower(iqhx, _nSamplesHalf - 1);
    double lag0_vc = RadarComplex::meanPower(iqvc, _nSamplesHalf - 1);
    double lag0_vx = RadarComplex::meanPower(iqvx, _nSamplesHalf - 1);
    
    // check power for interference
    double dbmHc = 10.0 * log10(lag0_hc);
    if (dbmHc > _maxValidSunPowerDbm) {
      // don't use this gate - probably interference
      continue;
    }
    
    RadarComplex_t lag0_hcvx =
      RadarComplex::meanConjugateProduct(iqhc, iqvx, _nSamplesHalf - 1);
    
    RadarComplex_t lag0_vchx =
      RadarComplex::meanConjugateProduct(iqvc, iqhx, _nSamplesHalf - 1);

    sumPowerHc += lag0_hc;
    sumPowerVc += lag0_vc;
    sumPowerHx += lag0_hx;
    sumPowerVx += lag0_vx;
    
    sumRvvhh0Hc = RadarComplex::complexSum(sumRvvhh0Hc, lag0_hcvx);
    sumRvvhh0Vc = RadarComplex::complexSum(sumRvvhh0Vc, lag0_vchx);

  } // igate

  if (nn < 3) {
    cerr << "Warning - computeMomentsSunDualAlt()" << endl;
    cerr << "  Insufficient good data found" << endl;
    cerr << "  az, el: " << _midAz << ", " << _midEl << endl;
    cerr << "  nn: " << nn << endl;
    return -1;
  }
  
  // compute moments

  moments->nn = nn;

  double meanPowerHc = sumPowerHc / nn;
  double meanPowerVc = sumPowerVc / nn;
  double meanPowerHx = sumPowerHx / nn;
  double meanPowerVx = sumPowerVx / nn;
  
  moments->powerHc = meanPowerHc;
  moments->powerVc = meanPowerVc;
  moments->powerHx = meanPowerHx;
  moments->powerVx = meanPowerVx;

  moments->dbmHc = 10.0 * log10(meanPowerHc);
  moments->dbmVc = 10.0 * log10(meanPowerVc);
  moments->dbmHx = 10.0 * log10(meanPowerHx);
  moments->dbmVx = 10.0 * log10(meanPowerVx);

  moments->dbm = (moments->dbmHc + moments->dbmVc +
                  moments->dbmHx + moments->dbmVx) / 4.0;

  moments->sumRvvhh0Hc = sumRvvhh0Hc;
  moments->sumRvvhh0Vc = sumRvvhh0Vc;

  RadarComplex_t sumRvvhh0 =
    RadarComplex::complexMean(sumRvvhh0Hc, sumRvvhh0Vc);
  moments->sumRvvhh0 = sumRvvhh0;
  
  moments->Rvvhh0Hc = RadarComplex::mean(sumRvvhh0Hc, nn);
  moments->Rvvhh0Vc = RadarComplex::mean(sumRvvhh0Vc, nn);
  moments->Rvvhh0 = RadarComplex::mean(sumRvvhh0, nn);
  
  double corrMagHc = RadarComplex::mag(sumRvvhh0Hc) / nn;
  moments->corrMagHc = corrMagHc;
  double corrMagVc = RadarComplex::mag(sumRvvhh0Vc) / nn;
  moments->corrMagVc = corrMagVc;
  moments->corrMag = (corrMagHc + corrMagVc) / 2.0;
  
  double corr00Hc = corrMagHc / sqrt(meanPowerHc * meanPowerVx);
  moments->corr00Hc = corr00Hc;
  double corr00Vc = corrMagVc / sqrt(meanPowerVc * meanPowerHx);
  moments->corr00Vc = corr00Vc;
  moments->corr00 = (corr00Hc + corr00Vc) / 2.0;

  moments->arg00Hc = RadarComplex::argDeg(sumRvvhh0Hc);
  moments->arg00Vc = RadarComplex::argDeg(sumRvvhh0Vc);
  moments->arg00 = RadarComplex::argDeg(sumRvvhh0);
  
  return 0;

}

///////////////////////////////////////////////////
// compute sun moments in dual-pol simultaneous mode

int SunCal::_computeMomentsSunDualSim(MomentsSun *moments,
                                      int startGate,
                                      int endGate)

{
  
  // initialize summation quantities
  
  double sumPowerHc = 0.0;
  double sumPowerVc = 0.0;
  RadarComplex_t sumRvvhh0(0.0, 0.0);
  double nn = 0.0;

  // loop through gates to be used for sun computations
  
  for (int igate = startGate; igate <= endGate; igate++, nn++) {
    
    GateData *gate = _gateData[igate];
    
    const RadarComplex_t *iqhc = gate->iqhc;
    const RadarComplex_t *iqvc = gate->iqvc;
    
    double lag0_hc = RadarComplex::meanPower(iqhc, _nSamples - 1);
    double lag0_vc = RadarComplex::meanPower(iqvc, _nSamples - 1);
    
    // check power for interference
    double dbmHc = 10.0 * log10(lag0_hc);
    if (dbmHc > _maxValidSunPowerDbm) {
      // don't use this gate - probably interference
      continue;
    }
    
    RadarComplex_t lag0_hcvc =
      RadarComplex::meanConjugateProduct(iqhc, iqvc, _nSamples - 1);
    
    sumPowerHc += lag0_hc;
    sumPowerVc += lag0_vc;
    
    sumRvvhh0 = RadarComplex::complexSum(sumRvvhh0, lag0_hcvc);
    
  } // igate

  if (nn < 3) {
    cerr << "Warning - computeMomentsSunDualSim()" << endl;
    cerr << "  Insufficient good data found" << endl;
    cerr << "  az, el: " << _midAz << ", " << _midEl << endl;
    cerr << "  nn: " << nn << endl;
    return -1;
  }
  
  // compute moments

  moments->nn = nn;

  double meanPowerHc = sumPowerHc / nn;
  double meanPowerVc = sumPowerVc / nn;
  
  moments->powerHc = meanPowerHc;
  moments->powerVc = meanPowerVc;
  
  moments->dbmHc = 10.0 * log10(meanPowerHc);
  moments->dbmVc = 10.0 * log10(meanPowerVc);
  moments->dbm = (moments->dbmHc + moments->dbmVc) / 2.0;

  moments->sumRvvhh0 = sumRvvhh0;
  moments->Rvvhh0 = RadarComplex::mean(sumRvvhh0, nn);

  double corrMag = RadarComplex::mag(sumRvvhh0) / nn;
  moments->corrMag = corrMag;

  double corr00 = corrMag / sqrt(meanPowerHc * meanPowerVc);
  moments->corr00 = corr00;

  moments->arg00 = RadarComplex::argDeg(sumRvvhh0);
  
  return 0;

}

///////////////////////////////////////////////////
// compute sun moments in single-pol mode

int SunCal::_computeMomentsSunSinglePol(MomentsSun *moments,
                                        int startGate,
                                        int endGate)

{
  
  // initialize summation quantities
  
  double sumPowerHc = 0.0;
  double nn = 0.0;
  
  // loop through gates to be used for sun computations
  
  for (int igate = startGate; igate <= endGate; igate++, nn++) {
    
    GateData *gate = _gateData[igate];
    
    const RadarComplex_t *iqhc = gate->iqhc;
    double lag0_hc = RadarComplex::meanPower(iqhc, _nSamples - 1);
    
    // check power for interference
    double dbmHc = 10.0 * log10(lag0_hc);
    if (dbmHc > _maxValidSunPowerDbm) {
      // don't use this gate - probably interference
      continue;
    }
    
    sumPowerHc += lag0_hc;
    
  } // igate

  if (nn < 3) {
    cerr << "Warning - computeMomentsSunSinglePol()" << endl;
    cerr << "  Insufficient good data found" << endl;
    cerr << "  az, el: " << _midAz << ", " << _midEl << endl;
    cerr << "  nn: " << nn << endl;
    return -1;
  }
  
  // compute moments

  moments->nn = nn;
  double meanPowerHc = sumPowerHc / nn;
  moments->powerHc = meanPowerHc;
  moments->dbmHc = 10.0 * log10(meanPowerHc);
  moments->dbm = moments->dbmHc;

  return 0;

}

////////////////////////////////////////////
// Compute xpol values, add to array

void SunCal::_accumForXpol(int startGate, int endGate)

{
  
  if (!_alternating) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - SunCal::_accumForXpol" << endl;
      cerr << "  Cross polar method only applicable in alternating mode" << endl;
    }
    return;
  }
  
  // loop through gates to be used for xpolar ratio computations
  
  for (int igate = startGate; igate <= endGate; igate++) {
    
    GateData *gate = _gateData[igate];
    
    const RadarComplex_t *iqhx = gate->iqhx;
    const RadarComplex_t *iqvx = gate->iqvx;
    
    double powerHx = RadarComplex::meanPower(iqhx, _nSamplesHalf - 1);
    double powerVx = RadarComplex::meanPower(iqvx, _nSamplesHalf - 1);
    RadarComplex_t lag1VxHx =
      RadarComplex::meanConjugateProduct(iqvx, iqhx, _nSamplesHalf - 1);
    
    double rhoVxHx =
      RadarComplex::mag(lag1VxHx) / sqrt(powerVx * powerHx);
    
    Xpol xpol(powerHx, powerVx, rhoVxHx);
    _xpolMoments.push_back(xpol);
    
  } // igate
  
}

////////////////////////////////////////////
// Compute test pulse powers, add to array

void SunCal::_accumForTestPulse()

{
  
  // compute test pulse gate number for each channel

  const IwrfTsPulse &pulse0 = *_pulseQueue[0];
  int nGates = pulse0.getNGates();
  double startRange = pulse0.get_start_range_km();
  double gateSpacing = pulse0.get_gate_spacing_km();

  int gateNumHc = (int)
    ((_params.test_pulse_range_km_hc - startRange) / gateSpacing + 0.5);
  if (gateNumHc < 0) gateNumHc = 0;
  if (gateNumHc > nGates - 1) gateNumHc = nGates - 1;
  
  int gateNumHx = (int)
    ((_params.test_pulse_range_km_hx - startRange) / gateSpacing + 0.5);
  if (gateNumHx < 0) gateNumHx = 0;
  if (gateNumHx > nGates - 1) gateNumHx = nGates - 1;
  
  int gateNumVc = (int)
    ((_params.test_pulse_range_km_vc - startRange) / gateSpacing + 0.5);
  if (gateNumVc < 0) gateNumVc = 0;
  if (gateNumVc > nGates - 1) gateNumVc = nGates - 1;
  
  int gateNumVx = (int)
    ((_params.test_pulse_range_km_vx - startRange) / gateSpacing + 0.5);
  if (gateNumVx < 0) gateNumVx = 0;
  if (gateNumVx > nGates - 1) gateNumVx = nGates - 1;

  // compute gate range

  int minGate = gateNumHc;
  minGate = MIN(minGate, gateNumVc);
  minGate = MIN(minGate, gateNumHx);
  minGate = MIN(minGate, gateNumVx);

  int maxGate = gateNumHc;
  maxGate = MAX(maxGate, gateNumVc);
  maxGate = MAX(maxGate, gateNumHx);
  maxGate = MAX(maxGate, gateNumVx);
  
  // compute mean power for test pulse in each channel
  
  double testPulsePowerHc = MomentsSun::missing;
  double testPulsePowerVc = MomentsSun::missing;
  double testPulsePowerHx = MomentsSun::missing;
  double testPulsePowerVx = MomentsSun::missing;
  
  for (int igate = minGate; igate <= maxGate; igate++) {

    double nnH = 0;
    double nnV = 0;
    double sumPowerHc = 0.0;
    double sumPowerVc = 0.0;
    double sumPowerHx = 0.0;
    double sumPowerVx = 0.0;
    
    for (size_t ipulse = 0; ipulse < _pulseQueue.size(); ipulse++) {
      
      const IwrfTsPulse &pulse = *_pulseQueue[ipulse];
      if (igate >= pulse.getNGates()) {
        continue;
      }
      const fl32 *iqChan0 = pulse.getIq0();
      const fl32 *iqChan1 = pulse.getIq1();
      if (iqChan0 == NULL || iqChan1 == NULL) {
        continue;
      }
      
      bool isHoriz = pulse.isHoriz();

      int index = igate * 2;

      double ii0 = iqChan0[index];
      double qq0 = iqChan0[index + 1];
      double power0 = ii0 * ii0 + qq0 * qq0;

      double ii1 = iqChan1[index];
      double qq1 = iqChan1[index + 1];
      double power1 = ii1 * ii1 + qq1 * qq1;
      
      if (_alternating) {
        
        // alternating
        
        if (isHoriz) {
          nnH++;
          sumPowerHc += power0;
          sumPowerVx += power1;
        } else {
          nnV++;
          sumPowerVc += power0;
          sumPowerHx += power1;
        } // if (isHoriz)
        
      } else {
        
        // simultaneous or not switching
        
        nnH++;
        nnV++;
        sumPowerHc += power0;
        sumPowerVc += power1;
        
      }
      
    } // ipulse

    double powerHc = 0.0;
    double powerVc = 0.0;
    double powerHx = 0.0;
    double powerVx = 0.0;
    
    if (nnH > 0 && nnV > 0) {
      powerHc = sumPowerHc / nnH;
      powerVc = sumPowerVc / nnV;
      powerVx = sumPowerVx / nnH;
      powerHx = sumPowerHx / nnV;
    }

    if (igate == gateNumHc) {
      testPulsePowerHc = powerHc;
    }
    if (igate == gateNumVc) {
      testPulsePowerVc = powerVc;
    }
    if (igate == gateNumHx) {
      testPulsePowerHx = powerHx;
    }
    if (igate == gateNumVx) {
      testPulsePowerVx = powerVx;
    }
    
  } // igate

  // store for later
  
  TestPulse tpm(testPulsePowerHc,
                testPulsePowerVc,
                testPulsePowerHx,
                testPulsePowerVx);

  _testPulseMoments.push_back(tpm);

}

/////////////////////
// add to xmit powers

void SunCal::_accumForXmitPowers(const IwrfTsPulse *pulse)

{

  double powerH = pulse->getMeasXmitPowerDbmH();
  double powerV = pulse->getMeasXmitPowerDbmV();

  if (powerH > 60.0) {
    _sumXmitPowerHDbm += powerH;
    _countXmitPowerH++;
  }

  if (powerV > 60.0) {
    _sumXmitPowerVDbm += powerV;
    _countXmitPowerV++;
  }

}

///////////////////////////
// check for north crossing
// and adjust accordingly

void SunCal::_checkForNorthCrossing(double &az0, double &az1)

{

  if (az0 - az1 > 180) {
    az0 -= 360.0;
  } else if (az0 - az1 < -180) {
    az1 -= 360.0;
  }

}

/////////////////////////////////////////////////
// create the raw moments array
    
void SunCal::_createRawMomentsArray()
  
{

  int nn = _gridNAz;
  if (_gridNEl > nn) {
    nn = _gridNEl;
  }

  for (int ii = 0; ii < nn; ii++) {
    vector<MomentsSun *> momentsVec;
    // momentsVec will be filled out later
    _rawMoments.push_back(momentsVec);
  }

}
  
/////////////////////////////////////////////////
// clear the raw moments array
    
void SunCal::_clearRawMomentsArray()
  
{
  
  for (size_t ii = 0; ii < _rawMoments.size(); ii++) {
    vector<MomentsSun *> &moments = _rawMoments[ii];
    for (size_t jj = 0; jj < moments.size(); jj++) {
      delete moments[jj];
    }
    moments.clear();
  }

  _prevRawMoments.init();

}

/////////////////////////////////////////////////
// delete the raw moments array
    
void SunCal::_deleteRawMomentsArray()
  
{

  _clearRawMomentsArray();
  _rawMoments.clear();

}

/////////////////////////////////////////////////
// create the interp moments array
// ready for interp
    
void SunCal::_createInterpMomentsArray()
  
{

  for (int iaz = 0; iaz < _gridNAz; iaz++) {
    vector<MomentsSun *> momentsVec;
    for (int iel = 0; iel < _gridNEl; iel++) {
      MomentsSun *moments = new MomentsSun();
      momentsVec.push_back(moments);
    }
    _interpMoments.push_back(momentsVec);
  }
  
}

/////////////////////////////////////////////////
// clear the interp moments array
    
void SunCal::_clearInterpMomentsArray()
  
{
  
  for (size_t iaz = 0; iaz < _interpMoments.size(); iaz++) {
    vector<MomentsSun *> &moments = _interpMoments[iaz];
    for (size_t iel = 0; iel < moments.size(); iel++) {
      moments[iel]->init();
    }
  }

}

/////////////////////////////////////////////////
// delete the interp moments array
    
void SunCal::_deleteInterpMomentsArray()
  
{
  
  for (size_t iaz = 0; iaz < _interpMoments.size(); iaz++) {
    vector<MomentsSun *> &moments = _interpMoments[iaz];
    for (size_t iel = 0; iel < moments.size(); iel++) {
      delete moments[iel];
    }
    moments.clear();
  }

  _interpMoments.clear();

}

//////////////////////////////////////////////////////////
// compute distance from moments point to specified point
    
double SunCal::_computeDist(MomentsSun *moments, double el, double az)
  
{
  double del = el - moments->offsetEl;
  double daz = az - moments->offsetAz;
  double dist = sqrt(del * del + daz * daz);
  return dist;
}

/////////////////////////////////////////////////
// delete xpol moments array
    
void SunCal::_deleteXpolMomentsArray()
  
{
  _xpolMoments.clear();
}

/////////////////////////////////////////////////
// delete test pulse moments array
    
void SunCal::_deleteTestPulseMomentsArray()
  
{
  _testPulseMoments.clear();
}

/////////////////////////////////////////////////
// sort the raw moments data by elevation
    
void SunCal::_sortRawMomentsByEl()
  
{
  
  for (size_t iaz = 0; iaz < _rawMoments.size(); iaz++) {
    vector<MomentsSun *> &moments = _rawMoments[iaz];
    sort (moments.begin(), moments.end(), _compareMomentsEl);
  }

}

/////////////////////////////////////////////////
// sort the raw moments data by azimuth
    
void SunCal::_sortRawMomentsByAz()
  
{
  
  for (size_t iel = 0; iel < _rawMoments.size(); iel++) {
    vector<MomentsSun *> &moments = _rawMoments[iel];
    sort (moments.begin(), moments.end(), _compareMomentsAz);
  }

}

/////////////////////////////////////////////////
// interp ppi moments onto regular grid
    
void SunCal::_interpMomentsPpi()
  
{

  // loop through azimuths

  for (int iaz = 0; iaz < _gridNAz; iaz++) {
    
    vector<MomentsSun *> &interp = _interpMoments[iaz];
    
    // find straddle if available
    
    for (int iel = 0; iel < _gridNEl; iel++) {

      double el = _gridMinEl + iel * _gridDeltaEl;

      // find the raw moments which straddle this elevation
      
      vector<MomentsSun *> &raw = _rawMoments[iaz];
      if (raw.size() < 2) {
        continue;
      }

      for (size_t ii = 0; ii < raw.size() - 1; ii++) {
        
        if (raw[ii] == NULL || raw[ii+1] == NULL) {
          continue;
        }

        double el0 = raw[ii]->offsetEl;
        double el1 = raw[ii+1]->offsetEl;

        // is the elevation between these two values

        if (el0 > el || el1 < el) {
          continue;
        }
        
        // compute weights
        
        double weight0 = 1.0;
        double weight1 = 0.0;
        
        if (el0 != el1) {
          weight1 = (el - el0) / (el1 - el0);
          weight0 = 1.0 - weight1;
        }

        // compute interpolated values
        
        interp[iel]->interp(*raw[ii], *raw[ii+1], weight0, weight1);

      } // ii

    } // iel

  } // iaz

}

/////////////////////////////////////////////////
// interp moments onto regular grid in RHI mode
    
void SunCal::_interpMomentsRhi()
  
{

  // loop through azimuths

  for (int iaz = 0; iaz < _gridNAz; iaz++) {
    
    vector<MomentsSun *> &interp = _interpMoments[iaz];
    
    // find straddle if available
    
    for (int iel = 0; iel < _gridNEl; iel++) {

      double az = _gridMinAz + iaz * _gridDeltaAz;

      // find the raw moments which straddle this azevation

      vector<MomentsSun *> &raw = _rawMoments[iel];
      for (size_t ii = 0; ii < raw.size() - 1; ii++) {
        
        double az0 = raw[ii]->offsetAz;
        double az1 = raw[ii+1]->offsetAz;

        // is the azevation between these two values

        if (az0 > az || az1 < az) {
          continue;
        }
        
        // compute weights
        
        double weight0 = 1.0;
        double weight1 = 0.0;
        
        if (az0 != az1) {
          weight1 = (az - az0) / (az1 - az0);
          weight0 = 1.0 - weight1;
        }

        // compute interpolated values
        
        interp[iel]->interp(*raw[ii], *raw[ii+1], weight0, weight1);

      } // ii

    } // iaz

  } // iel

}

/////////////////////////////////////////////////
// get noise power from cal fuke

void SunCal::_getNoiseFromCalFile()
  
{

  if (_params.input_mode == Params::TS_REALTIME_DIR_INPUT ||
      _params.input_mode == Params::TS_FILELIST_INPUT ||
      _params.input_mode == Params::TS_FMQ_INPUT) {
    _noiseDbmHc = _calib.getNoiseDbmHc();
    _noiseDbmVc = _calib.getNoiseDbmVc();
    _noiseDbmHx = _calib.getNoiseDbmHx();
    _noiseDbmVx = _calib.getNoiseDbmVx();
  } else {
    _noiseDbmHc = _calib.getNoiseDbmHc() - _calib.getReceiverGainDbHc();
    _noiseDbmVc = _calib.getNoiseDbmVc() - _calib.getReceiverGainDbVc();
    _noiseDbmHx = _calib.getNoiseDbmHx() - _calib.getReceiverGainDbHx();
    _noiseDbmVx = _calib.getNoiseDbmVx() - _calib.getReceiverGainDbVx();
  }
    
}


/////////////////////////////////////////////////
// get the noise power from time series
    
void SunCal::_getNoiseFromTimeSeries()
  
{

  const IwrfTsInfo &info = _tsReader->getOpsInfo();

  _noiseDbmHc = info.get_calib_noise_dbm_hc();
  _noiseDbmHx = info.get_calib_noise_dbm_hx();
  _noiseDbmVc = info.get_calib_noise_dbm_vc();
  _noiseDbmVx = info.get_calib_noise_dbm_vx();

}

/////////////////////////////////////////////////
// compute the mean noise power for each channel
    
void SunCal::_computeMeanNoise()
  
{

  double sumNoiseDbmHc = 0.0;
  double sumNoiseDbmHx = 0.0;
  double sumNoiseDbmVc = 0.0;
  double sumNoiseDbmVx = 0.0;
  _nBeamsNoise = 0.0;

  double minOffset = _params.min_angle_offset_for_noise_power;

  for (int iel = 0; iel < _gridNEl; iel++) {

    double elOffset = _gridMinEl + iel * _gridDeltaEl;

    for (int iaz = 0; iaz < _gridNAz; iaz++) {

      double offsetAz = _gridMinAz + iaz * _gridDeltaAz;

      double offset = sqrt(elOffset * elOffset + offsetAz * offsetAz);

      if (offset < minOffset) {
        continue;
      }
      
      MomentsSun *moments = _interpMoments[iaz][iel];

      if (moments->dbmHc != MomentsSun::missing &&
          moments->dbmHx != MomentsSun::missing &&
          moments->dbmVc != MomentsSun::missing &&
          moments->dbmVx != MomentsSun::missing &&
          moments->dbmHc < _maxValidSunPowerDbm) {
        
        sumNoiseDbmHc += moments->dbmHc;
        sumNoiseDbmHx += moments->dbmHx;
        sumNoiseDbmVc += moments->dbmVc;
        sumNoiseDbmVx += moments->dbmVx;
        _nBeamsNoise++;
      }
      
    }

  }

  if (_nBeamsNoise == 0) {
    _nBeamsNoise = 1;
  }

  _noiseDbmHc = sumNoiseDbmHc / _nBeamsNoise;
  _noiseDbmHx = sumNoiseDbmHx / _nBeamsNoise;
  _noiseDbmVc = sumNoiseDbmVc / _nBeamsNoise;
  _noiseDbmVx = sumNoiseDbmVx / _nBeamsNoise;

}

/////////////////////////////////////////////////
// compute the noise power as the 
// min power for each channel
    
void SunCal::_computeNoiseFromMinPower()
  
{

  _noiseDbmHc = 120.0;
  _noiseDbmHx = 120.0;
  _noiseDbmVc = 120.0;
  _noiseDbmVx = 120.0;

  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++) {
      
      MomentsSun *moments = _interpMoments[iaz][iel];
      
      if (moments->dbmHc != MomentsSun::missing &&
          moments->dbmHc < _noiseDbmHc) {
        _noiseDbmHc = moments->dbmHc;
      }
      if (moments->dbmHx != MomentsSun::missing &&
          moments->dbmHx < _noiseDbmHx) {
        _noiseDbmHx = moments->dbmHx;
      }
      if (moments->dbmVc != MomentsSun::missing &&
          moments->dbmVc < _noiseDbmVc) {
        _noiseDbmVc = moments->dbmVc;
      }
      if (moments->dbmVx != MomentsSun::missing &&
          moments->dbmVx < _noiseDbmVx) {
        _noiseDbmVx = moments->dbmVx;
      }

    }
  }

  _noiseDbmHc -= -0.001;
  _noiseDbmHx -= -0.001;
  _noiseDbmVc -= -0.001;
  _noiseDbmVx -= -0.001;

}

/////////////////////////////////////////////////
// crrect powers by subtracting the noise
    
void SunCal::_correctPowersForNoise()
  
{

  double noisePowerHc = 0.0;
  double noisePowerVc = 0.0;
  double noisePowerHx = 0.0;
  double noisePowerVx = 0.0;
  
  if (_noiseDbmHc > -9990) {
    noisePowerHc = pow(10.0, _noiseDbmHc / 10.0);
  }
  
  if (_noiseDbmHx > -9990) {
    noisePowerHx = pow(10.0, _noiseDbmHx / 10.0);
  }
  
  if (_noiseDbmVc > -9990) {
    noisePowerVc = pow(10.0, _noiseDbmVc / 10.0);
  }
  
  if (_noiseDbmVx > -9990) {
    noisePowerVx = pow(10.0, _noiseDbmVx / 10.0);
  }

  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++) {
      MomentsSun *moments = _interpMoments[iaz][iel];
      moments->adjustForNoise(noisePowerHc,
			      noisePowerHx,
			      noisePowerVc,
			      noisePowerVx);
    }
  }
  
}
  
/////////////////////////////////////////////////
// compute the power ratios
    
void SunCal::_computePowerRatios()
  
{

  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++) {
      MomentsSun *moments = _interpMoments[iaz][iel];
      moments->computeRatios();
    }
  }

}

/////////////////////////////////////////////////
// compute the maximum power
    
void SunCal::_computeMaxPower()
  
{

  _maxPowerDbmHc = -120.0;
  _maxPowerDbmVc = -120.0;

  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++) {
      MomentsSun *moments = _interpMoments[iaz][iel];
      if (moments->dbmHc <= _maxValidSunPowerDbm) {
        _maxPowerDbmHc = MAX(_maxPowerDbmHc, moments->dbmHc);
      }
      if (moments->dbmVc <= _maxValidSunPowerDbm) {
        _maxPowerDbmVc = MAX(_maxPowerDbmVc, moments->dbmVc);
      }
    }
  }
   
  // compute dbm below peak

  _maxPowerDbm = _maxPowerDbmHc;
  if (_dualPol) {
    _maxPowerDbm = (_maxPowerDbmHc + _maxPowerDbmVc) / 2.0;
  }
  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++) {
      MomentsSun *moments = _interpMoments[iaz][iel];
      moments->dbm = moments->dbmHc;
      if (_dualPol) {
        moments->dbm = (moments->dbmHc + moments->dbmVc) / 2.0;
      }
      moments->dbBelowPeak = moments->dbm - _maxPowerDbm;
    }
  }

}

/////////////////////////////////////////////////
// compute mean correlation for sun disk
    
void SunCal::_computeSunCorr()
  
{

  if (_alternating) {

    double sumPowerHc = 0.0;
    double sumPowerHx = 0.0;
    double sumPowerVc = 0.0;
    double sumPowerVx = 0.0;
    
    RadarComplex_t sumRvvhh0Hc(0.0, 0.0);
    RadarComplex_t sumRvvhh0Vc(0.0, 0.0);
    
    double nn = 0.0;
    
    // sun_solid_angle_for_correlation;
    
    for (int iel = 0; iel < _gridNEl; iel++) {
      double dEl = _gridMinEl + iel * _gridDeltaEl;
      for (int iaz = 0; iaz < _gridNAz; iaz++) {
        double dAz = _gridMinAz + iaz * _gridDeltaAz;
        double angDist = sqrt(dEl * dEl + dAz * dAz);
        if (angDist > _params.max_solid_angle_for_mean_correlation / 2.0) {
          continue;
        }
        const MomentsSun &moments = *_interpMoments[iaz][iel];
        sumPowerHc += moments.powerHc;
        sumPowerVc += moments.powerVc;
        sumPowerHx += moments.powerHx;
        sumPowerVx += moments.powerVx;
        sumRvvhh0Hc = RadarComplex::complexSum(sumRvvhh0Hc, moments.Rvvhh0Hc);
        sumRvvhh0Vc = RadarComplex::complexSum(sumRvvhh0Vc, moments.Rvvhh0Vc);
        nn++;
      }
    }
    
    if (nn < 2) {
      return;
    }
    
    double meanPowerHc = sumPowerHc / nn;
    double meanPowerVc = sumPowerVc / nn;
    double meanPowerHx = sumPowerHc / nn;
    double meanPowerVx = sumPowerHc / nn;
    
    RadarComplex_t meanRvvhh0Hc;
    RadarComplex_t meanRvvhh0Vc;
    meanRvvhh0Hc = RadarComplex::mean(sumRvvhh0Hc, nn);
    meanRvvhh0Vc = RadarComplex::mean(sumRvvhh0Vc, nn);
    
    if (meanPowerHc > 0 && meanPowerVx > 0) {
      _meanCorr00H =
        RadarComplex::mag(meanRvvhh0Hc) / sqrt(meanPowerHc * meanPowerVx);
    }
    if (meanPowerVc > 0 && meanPowerHx > 0) {
      _meanCorr00V =
        RadarComplex::mag(meanRvvhh0Vc) / sqrt(meanPowerVc * meanPowerHx);
    }

  } else {
    
    double sumPowerHc = 0.0;
    double sumPowerVc = 0.0;
    RadarComplex_t sumRvvhh0(0.0, 0.0);
    double nn = 0.0;
    
    for (int iel = 0; iel < _gridNEl; iel++) {
      double dEl = _gridMinEl + iel * _gridDeltaEl;
      for (int iaz = 0; iaz < _gridNAz; iaz++) {
        double dAz = _gridMinAz + iaz * _gridDeltaAz;
        double angDist = sqrt(dEl * dEl + dAz * dAz);
        if (angDist > _params.max_solid_angle_for_mean_correlation / 2.0) {
          continue;
        }
        const MomentsSun &moments = *_interpMoments[iaz][iel];
        sumPowerHc += moments.powerHc;
        sumPowerVc += moments.powerVc;
        sumRvvhh0 = RadarComplex::complexSum(sumRvvhh0, moments.Rvvhh0);
        nn++;
      }
    }
    
    if (nn < 2) {
      return;
    }

    double meanPowerHc = sumPowerHc / nn;
    double meanPowerVc = sumPowerVc / nn;
    RadarComplex_t meanRvvhh0;
    meanRvvhh0 = RadarComplex::mean(sumRvvhh0, nn);
    
    if (meanPowerHc > 0 && meanPowerVc > 0) {
      _meanCorr00 =
        RadarComplex::mag(meanRvvhh0) / sqrt(meanPowerHc * meanPowerVc);
    }

  }

}

/////////////////////////////////////////////////
// compute the sun centroid
// specify channel Hc, Vc or Meanc
    
void SunCal::_computeSunCentroid(power_channel_t channel)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    switch (channel) {
    case channelHc:
      cerr << "_computeSunCentroid: computing for Hc" << endl;
      break;
    case channelVc:
      cerr << "_computeSunCentroid: computing for Vc" << endl;
      break;
    default:
      cerr << "_computeSunCentroid: computing for Meanc" << endl;
      break;
    }
  }

  double maxPowerDbm = _maxPowerDbm;
  if (channel == channelHc) {
    maxPowerDbm = _maxPowerDbmHc;
  } else if (channel == channelVc) {
    maxPowerDbm = _maxPowerDbmVc;
  }

  double sumWtAz = 0.0;
  double sumWtEl = 0.0;
  double sumPower = 0.0;
  double count = 0.0;

  double edgePowerDbm = maxPowerDbm - _params.sun_edge_below_peak_db;

  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++) {
      MomentsSun *moments = _interpMoments[iaz][iel];
      double dbm = moments->dbm;
      if (channel == channelHc) {
	dbm = moments->dbmHc;
      } else if (channel == channelVc) {
	dbm = moments->dbmVc;
      }
      double power = pow(10.0, dbm / 10.0);
      if (dbm >= edgePowerDbm && dbm <= _maxValidSunPowerDbm) {
        double az = moments->offsetAz;
        double el = moments->offsetEl;
        sumPower += power;
        sumWtAz += az * power;
        sumWtEl += el * power;
        count++;
      }
    }
  }

  if (count == 0) {
    // no valid data
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Cannot estimate solar centroid:" << endl;
      cerr << "  no measured power" << endl;
    }
    return;
  }

  double sunCentroidAzOffset = sumWtAz / sumPower;
  double sunCentroidElOffset = sumWtEl / sumPower;

  if (sunCentroidAzOffset < _gridMinAz ||
      sunCentroidAzOffset > _gridMaxAz ||
      sunCentroidElOffset < _gridMinEl ||
      sunCentroidElOffset > _gridMaxEl) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Cannot estimate solar centroid:" << endl;
      cerr << "  sunCentroidAzOffset: " << sunCentroidAzOffset << endl;
      cerr << "  sunCentroidElOffset: " << sunCentroidElOffset << endl;
      cerr << "  Setting offsets to 0" << endl;
    }
    return;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Initial estimates for solar centroid:" << endl;
    cerr << "  sunCentroidAzOffset: " << sunCentroidAzOffset << endl;
    cerr << "  sunCentroidElOffset: " << sunCentroidElOffset << endl;
  }

  // if possible, fit parabola to elevation at solar peak
  // to refine the azimuth centroid

  _validCentroid = false;
  
  vector<double> azArray;
  vector<double> azDbm;

  int elCentroidIndex =
    (int) ((sunCentroidElOffset - _gridMinEl) /  _gridDeltaEl);

  double dbmAdd = 200.0;
  
  for (int iaz = 0; iaz < _gridNAz; iaz++) {
    MomentsSun *moments = _interpMoments[iaz][elCentroidIndex];
    double dbm = moments->dbm;
    if (channel == channelHc) {
      dbm = moments->dbmHc;
    } else if (channel == channelVc) {
      dbm = moments->dbmVc;
    }
    if (dbm >= edgePowerDbm) {
      double az = _gridMinAz + iaz * _gridDeltaAz;
      azArray.push_back(az);
      // add 200 to dbm to ensure real roots
      azDbm.push_back(dbm + dbmAdd);
    }
  }
  
  double widthAz3Db = MomentsSun::missing;
  if (_quadFit((int) azArray.size(),
               azArray, azDbm,
               _ccAz, _bbAz, _aaAz,
               _errEstAz, _rSqAz) == 0) {
    double rootTerm = _bbAz * _bbAz - 4.0 * _aaAz * _ccAz;
    double rootTerm2 = _bbAz * _bbAz - 2.0 * _aaAz * _ccAz;
    if (_rSqAz > 0.9 && rootTerm >= 0) {
      // good fit, real roots, so override centroid
      double root1 = (-_bbAz - sqrt(rootTerm)) / (2.0 * _aaAz); 
      double root2 = (-_bbAz + sqrt(rootTerm)) / (2.0 * _aaAz);
      sunCentroidAzOffset = (root1 + root2) / 2.0;
      if (rootTerm2 >= 0) {
        widthAz3Db = -(sqrt(rootTerm2) / _aaAz);
        _validCentroid = true;
      }
    }
  }
  
  // if possible, fit parabola to azimuth at solar peak
  // to refine the azimuth centroid

  vector<double> elArray;
  vector<double> elDbm;
  
  int azCentroidIndex =
    (int) ((sunCentroidAzOffset - _gridMinAz) /  _gridDeltaAz);
  if (azCentroidIndex < 0 || azCentroidIndex > (_gridNAz - 1)) {
    return;
  }
  for (int iel = 0; iel < _gridNEl; iel++) {
    MomentsSun *moments = _interpMoments[azCentroidIndex][iel];
    double dbm = moments->dbm;
    if (channel == channelHc) {
      dbm = moments->dbmHc;
    } else if (channel == channelVc) {
      dbm = moments->dbmVc;
    }
    if (dbm >= edgePowerDbm) {
      double el = _gridMinEl + iel * _gridDeltaEl;
      elArray.push_back(el);
      // add 200 to dbm to ensure real roots
      elDbm.push_back(dbm + dbmAdd);
    }
  }
  
  double widthEl3Db = MomentsSun::missing;
  if (_quadFit((int) elArray.size(),
               elArray, elDbm,
               _ccEl, _bbEl, _aaEl,
               _errEstEl, _rSqEl) == 0) {
    double rootTerm = _bbEl * _bbEl - 4.0 * _aaEl * _ccEl;
    double rootTerm2 = _bbEl * _bbEl - 2.0 * _aaEl * _ccEl;
    if (_rSqEl > 0.9 && rootTerm >= 0) {
      // good fit, real roots, so override centroid
      double root1 = (-_bbEl - sqrt(rootTerm)) / (2.0 * _aaEl); 
      double root2 = (-_bbEl + sqrt(rootTerm)) / (2.0 * _aaEl);
      sunCentroidElOffset = (root1 + root2) / 2.0;
      if (rootTerm2 >= 0) {
        widthEl3Db = -(sqrt(rootTerm2) / _aaEl);
        _validCentroid = true;
      }
    }
  }

  double widthRatio = MomentsSun::missing;
  if (widthEl3Db != MomentsSun::missing && widthAz3Db != MomentsSun::missing) {
    widthRatio = widthEl3Db / widthAz3Db;
  }
  
  if (_params.debug) {
    cerr << "Final estimates for solar centroid:" << endl;
    cerr << "  sunCentroidAzOffset: " << sunCentroidAzOffset << endl;
    cerr << "  sunCentroidElOffset: " << sunCentroidElOffset << endl;
    cerr << "  parabolaWidthAz: " << widthAz3Db << endl;
    cerr << "  parabolaWidthEl: " << widthEl3Db << endl;
    cerr << "  parabolaWidthRatio: " << widthRatio << endl;
  }

  // add in check for parabola fit
  // typical parabola width is 4.75

  if (widthAz3Db < 2.5 || widthAz3Db > 7.5) {
    _validCentroid = false;
    if (_params.debug) {
      cerr << "ERROR - SunCal::_computeSunCentroid" << endl;
      cerr << "  Parabola width Az out of limits: "
           << widthAz3Db << endl;
      cerr << "  Should be within 2.5 to 7.5 deg" << endl;
      cerr << "  Setting centroid invalid" << endl;
    }
  }
  if (widthEl3Db < 2.5 || widthEl3Db > 7.5) {
    _validCentroid = false;
    if (_params.debug) {
      cerr << "ERROR - SunCal::_computeSunCentroid" << endl;
      cerr << "  Parabola width El out of limits: "
           << widthEl3Db << endl;
      cerr << "  Should be within 2.5 to 7.5 deg" << endl;
      cerr << "  Setting centroid invalid" << endl;
    }
  }

  double quadPowerDbm = (_ccAz + _ccEl) / 2.0 - dbmAdd;

  switch (channel) {
  case channelHc:
    _quadPowerDbmHc = quadPowerDbm;
    _sunCentroidAzOffsetHc = sunCentroidAzOffset;
    _sunCentroidElOffsetHc = sunCentroidElOffset;
    _widthRatioElAzHc = widthRatio;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "===>>> CHANNEL HC widthEl3Db, widthAz3Db, ratio: "
           << widthEl3Db << ", " << widthAz3Db << ", " << widthRatio << endl;
    }
    break;
  case channelVc:
    _quadPowerDbmVc = quadPowerDbm;
    _sunCentroidAzOffsetVc = sunCentroidAzOffset;
    _sunCentroidElOffsetVc = sunCentroidElOffset;
    _widthRatioElAzVc = widthRatio;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "===>>> CHANNEL VC widthEl3Db, widthAz3Db, ratio: "
           << widthEl3Db << ", " << widthAz3Db << ", " << widthRatio << endl;
    }
    break;
  default:
    _quadPowerDbm = quadPowerDbm;
    _sunCentroidAzOffset = sunCentroidAzOffset;
    _sunCentroidElOffset = sunCentroidElOffset;
    break;
  }
  
}

////////////////////////////////////////////
// class for sorting moments based on power

class MomentsPowerCompare {
public:
  bool operator() (const MomentsSun &a, const MomentsSun &b) const;
};

bool MomentsPowerCompare::operator()
  (const MomentsSun &a, const MomentsSun &b) const
{
  if (a.dbm < b.dbm) {
    return true;
  } else {
    return false;
  }
}

/////////////////////////////////////////////////
// compute ratios using solid angle
    
void SunCal::_computeSSUsingSolidAngle()
  
{

  int nAngles = _params.n_s1s2_ratios_computed;
  double minAngle = _params.min_solid_angle_for_s1s2;
  double deltaAngle = _params.delta_solid_angle_for_s1s2;
  
  int indexForZdrBias = 0;
  double angleForZdrBias = 1.0;
  double minDiff = 9999.0;
  for (int ii = 0; ii < nAngles; ii++) {
    double angle = minAngle + ii * deltaAngle;
    double diff = fabs(angle - _params.solid_angle_for_zdr_bias);
    if (diff < minDiff) {
      indexForZdrBias = ii;
      angleForZdrBias = angle;
      minDiff = diff;
    }
  }

  for (int ii = 0; ii < nAngles; ii++) {

    double angle = minAngle + ii * deltaAngle;
    double searchRadius = angle / 2.0;

    // find points within the required solid angle
    
    vector<MomentsSun> selected;
    
    for (int iel = 0; iel < _gridNEl; iel++) {
      double el = _gridMinEl + iel * _gridDeltaEl;
      double elOffset = el - _sunCentroidElOffset;
      for (int iaz = 0; iaz < _gridNAz; iaz++) {
        double az = _gridMinAz + iaz * _gridDeltaAz;
        double azOffset = az - _sunCentroidAzOffset;
        double offset = sqrt(elOffset * elOffset + azOffset * azOffset);
        if (offset <= searchRadius) {
          MomentsSun *moments = _interpMoments[iaz][iel];
          selected.push_back(*moments);
        }
      } // iaz
    } // iel
    
    // compute ratios using selected points
    
    Stats stats;
    _computeSS(selected, angle, stats);
    _stats.push_back(stats);

    if (ii == indexForZdrBias) {
      _statsForZdrBias = stats;
      if (_params.debug) {
        cerr << "Using solid angle for ZDR bias calcs: " << angleForZdrBias << endl;
      }
    }

  }

  // set global results

  _S1S2Sdev = 0;
  _S1S2Results.clear();
  _SSResults.clear();

  for (int ii = 0; ii < (int) _stats.size(); ii++) {
    Stats &stats = _stats[ii];
    if (ii == 0) {
      _S1S2Sdev = stats.sdevS1S2;
      _SSSdev = stats.sdevSS;
    }
    _S1S2Results.push_back(stats.ratioMeanS1S2Db);
    _SSResults.push_back(stats.ratioMeanSSDb);
  }
  
}

/////////////////////////////////////////////////
// compute S1S2 ratio
    
void SunCal::_computeSS(const vector<MomentsSun> &selected,
                        double solidAngle,
                        Stats &stats)
  
{

  double sumPowerHc = 0.0;
  double sumPowerVc = 0.0;
  double sumPowerHx = 0.0;
  double sumPowerVx = 0.0;
  double minPower = 9999;

  double sumRatioDbmVcHc = 0.0;
  double sumRatioDbmVxHx = 0.0;
  double sumRatioDbmVcHx = 0.0;
  double sumRatioDbmVxHc = 0.0;

  RadarComplex_t sumRvvhh0Hc(0.0, 0.0);
  RadarComplex_t sumRvvhh0Vc(0.0, 0.0);
  RadarComplex_t sumRvvhh0(0.0, 0.0);

  double sumS1S2 = 0.0;
  double sumSqS1S2 = 0.0;
  double sumSS = 0.0;
  double prodSS = 1.0;
  double sumSqSS = 0.0;
  double nn = 0.0;

  for (int ii = 0; ii < (int) selected.size(); ii++) {
    
    const MomentsSun &moments = selected[ii];

    if (moments.ratioDbmVcHc < -9990) {
      continue;
    }

    if (moments.powerHc < minPower) {
      minPower = moments.powerHc;
    }

    if (_alternating) {
      sumRvvhh0Hc = RadarComplex::complexSum(sumRvvhh0Hc, moments.Rvvhh0Hc);
      sumRvvhh0Vc = RadarComplex::complexSum(sumRvvhh0Vc, moments.Rvvhh0Vc);
    } else {
      sumRvvhh0 = RadarComplex::complexSum(sumRvvhh0, moments.Rvvhh0);
    }
    
    sumPowerHc += moments.powerHc;
    sumPowerVc += moments.powerVc;
    sumRatioDbmVcHc += moments.ratioDbmVcHc;
    if (_alternating) {
      sumPowerHx += moments.powerHx;
      sumPowerVx += moments.powerVx;
      sumRatioDbmVxHx += moments.ratioDbmVxHx;
      sumRatioDbmVcHx += moments.ratioDbmVcHx;
      sumRatioDbmVxHc += moments.ratioDbmVxHc;
      sumS1S2 += moments.S1S2;
      sumSqS1S2 += moments.S1S2 * moments.S1S2;
    } else {
      sumSS += moments.SS;
      prodSS *= moments.SS;
      sumSqSS += moments.SS * moments.SS;
    }
    
    nn++;

  }

  if (nn < 2) {
    return;
  }

  stats.solidAngle = solidAngle;
  stats.nBeamsUsed = nn;

  double meanPowerHc = sumPowerHc / nn;
  double meanPowerVc = sumPowerVc / nn;
  double meanPowerHx = sumPowerHc / nn;
  double meanPowerVx = sumPowerHc / nn;

  RadarComplex_t meanRvvhh0Hc;
  RadarComplex_t meanRvvhh0Vc;
  RadarComplex_t meanRvvhh0;

  meanRvvhh0Hc.re = sumRvvhh0Hc.re / nn;
  meanRvvhh0Hc.im = sumRvvhh0Hc.im / nn;
  meanRvvhh0Vc.re = sumRvvhh0Vc.re / nn;
  meanRvvhh0Vc.im = sumRvvhh0Vc.im / nn;

  meanRvvhh0.re = sumRvvhh0.re / nn;
  meanRvvhh0.im = sumRvvhh0.im / nn;
  
  stats.meanDbm = 10.0 * log10(meanPowerHc);
  stats.minDbm = 10.0 * log10(minPower);
  
  if (_alternating) {
    if (meanPowerHc > 0 && meanPowerVx > 0) {
      stats.meanCorr00Hc =
        RadarComplex::mag(meanRvvhh0Hc) / sqrt(meanPowerHc * meanPowerVx);
      stats.sumRvvhh0Hc = sumRvvhh0Hc;
    }
    if (meanPowerVc > 0 && meanPowerHx > 0) {
      stats.meanCorr00Vc =
        RadarComplex::mag(meanRvvhh0Vc) / sqrt(meanPowerVc * meanPowerHx);
      stats.sumRvvhh0Vc = sumRvvhh0Vc;
    }
  } else {
    if (meanPowerHc > 0 && meanPowerVc > 0) {
      stats.meanCorr00 =
        RadarComplex::mag(meanRvvhh0) / sqrt(meanPowerHc * meanPowerVc);
      stats.sumRvvhh0 = sumRvvhh0;
    }
  }

  stats.meanRatioDbmVcHc = sumRatioDbmVcHc / (double) nn;
  if (_alternating) {
    stats.meanRatioDbmVxHx = sumRatioDbmVxHx / (double) nn;
    stats.meanRatioDbmVcHx = sumRatioDbmVcHx / (double) nn;
    stats.meanRatioDbmVxHc = sumRatioDbmVxHc / (double) nn;
  }
  if (_alternating) {
    stats.meanS1S2 = sumS1S2 / (double) nn;
  } else {
    // stats.meanSS = pow(prodSS, 1.0 / (double) nn);
    stats.meanSS = sumSS / (double) nn;
  }
  
  double ratioMeanVcHc = sumPowerVc / sumPowerHc;
  stats.ratioMeanVcHcDb = 10.0 * log10(ratioMeanVcHc);

  if (_alternating) {
    double ratioMeanVxHx = sumPowerVx / sumPowerHx;
    stats.ratioMeanVxHxDb = 10.0 * log10(ratioMeanVxHx);
    stats.ratioMeanS1S2Db = stats.ratioMeanVcHcDb + stats.ratioMeanVxHxDb;
  } else {
    stats.ratioMeanSSDb = stats.ratioMeanVcHcDb * 2;
  }
  
  if (_alternating) {
    double variance =
      (sumSqS1S2 - (sumS1S2 * sumS1S2) / (double) nn) /
      ((double) nn - 1.0);
    stats.sdevS1S2 = 0.000001;
    if (variance >= 0.0) {
      stats.sdevS1S2 = sqrt(variance);
    }
    stats.SdevOfMean = stats.sdevS1S2 / sqrt((double) nn);
  } else {
    double variance =
      (sumSqSS - (sumSS * sumSS) / (double) nn) /
      ((double) nn - 1.0);
    stats.sdevSS = 0.000001;
    if (variance >= 0.0) {
      stats.sdevSS = sqrt(variance);
    }
    stats.SdevOfMean = stats.sdevSS / sqrt((double) nn);
  }

}

/////////////////////////////////////////////////
// compute ellipse power differences using solid angle
    
void SunCal::_computeEllipsePowerDiffsUsingSolidAngle()
  
{

  double maxAngle = _params.solid_angle_for_ellipse_power_diffs;
  double searchRadius = maxAngle / 2.0;

  // find points within the required solid angle
  // for computing the ellipse power diffs
  
  vector<MomentsSun> selectedEl;
  vector<MomentsSun> selectedAz;
    
  int elCentroidIndex = (int) ((_sunCentroidElOffset - _gridMinEl) / _gridDeltaEl + 0.5);
  int azCentroidIndex = (int) ((_sunCentroidAzOffset - _gridMinAz) / _gridDeltaAz + 0.5);

  double minEl = _sunCentroidElOffset - searchRadius;
  double maxEl = _sunCentroidElOffset + searchRadius;
  int minElIndex = (int) ((minEl - _gridMinEl) / _gridDeltaEl + 0.5);
  int maxElIndex = (int) ((maxEl - _gridMinEl) / _gridDeltaEl + 0.5);
  if (minElIndex < 0) {
    minElIndex = 0;
  }
  if (maxElIndex > _gridNEl - 1) {
    maxElIndex = _gridNEl - 1;
  }
  for (int iel = minElIndex; iel <= maxElIndex; iel++) {
    MomentsSun *moments = _interpMoments[azCentroidIndex][iel];
    selectedEl.push_back(*moments);
  } // iel
  
  double minAz = _sunCentroidAzOffset - searchRadius;
  double maxAz = _sunCentroidAzOffset + searchRadius;
  int minAzIndex = (int) ((minAz - _gridMinAz) / _gridDeltaAz + 0.5);
  int maxAzIndex = (int) ((maxAz - _gridMinAz) / _gridDeltaAz + 0.5);
  if (minAzIndex < 0) {
    minAzIndex = 0;
  }
  if (maxAzIndex > _gridNAz - 1) {
    maxAzIndex = _gridNAz - 1;
  }
  for (int iaz = minAzIndex; iaz <= maxAzIndex; iaz++) {
    MomentsSun *moments = _interpMoments[iaz][elCentroidIndex];
    selectedAz.push_back(*moments);
  } // iaz
  
  // compute ratios using the selected points
  
  Stats statsEl, statsAz;
  _computeSS(selectedEl, maxAngle, statsEl);
  _computeSS(selectedAz, maxAngle, statsAz);
  
  _zdrDiffElAz = -(statsEl.meanRatioDbmVcHc - statsAz.meanRatioDbmVcHc);

  if (_params.debug) {
    cerr << "==>> Computing ZDR diff along el and az, at sun axes" << endl;
    cerr << "  el.meanRatioDbmVcHc,  az.meanRatioDbmVcHc, zdrDiffElAz: "
         << statsEl.meanRatioDbmVcHc << ", "
         << statsAz.meanRatioDbmVcHc << ", "
         << _zdrDiffElAz << endl;
  }
  
}

/////////////////////////////////////////////////
// compute xpol power ratio
    
void SunCal::_computeXpolRatio(const vector<Xpol> &xpolMoments)
  
{

  _nXpolPoints = 0;
  _meanXpolRatioDb = MomentsSun::missing;

  if (!_alternating) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - SunCal::_computeXpolRatio" << endl;
      cerr << "  Cross polar method only applicable in alternating mode" << endl;
    }
    return;
  }

  double sumRatioDb = 0.0;
  
  double minValidRatioDb = _params.cross_polar_min_valid_ratio_db;
  double maxValidRatioDb = _params.cross_polar_max_valid_ratio_db;
  
  double noisePowerHx = pow(10.0, _noiseDbmHx / 10.0);
  double noisePowerVx = pow(10.0, _noiseDbmVx / 10.0);

  // loop through moments

  for (size_t ii = 0; ii < xpolMoments.size(); ii++) {
    
    const Xpol &moments = xpolMoments[ii];
    
    // check vx-hx correlation

    if (moments.rhoVxHx < _params.cross_polar_min_rho_vx_hx) {
      continue;
    }

    // noise subtracted power and SNR
    
    double powerHxNs = moments.powerHx - noisePowerHx;
    double powerVxNs = moments.powerVx - noisePowerVx;

    // check mean snr in cross-polar channels
    
    if (powerHxNs <= 0 || powerVxNs <= 0) {
      continue;
    }
    
    double snrhx = 10.0 * log10(powerHxNs / noisePowerHx);
    double snrvx = 10.0 * log10(powerVxNs / noisePowerVx);
    
    if (snrhx < _params.cross_polar_min_snr ||
        snrhx > _params.cross_polar_max_snr) {
      continue;
    }
    if (snrvx < _params.cross_polar_min_snr ||
        snrvx > _params.cross_polar_max_snr) {
      continue;
    }

    // compute ratio
    
    double ratio = powerHxNs / powerVxNs;
    double ratioDb = 10.0 * log10(ratio);

    // check ratio for sanity
    
    if (ratioDb < minValidRatioDb || ratioDb > maxValidRatioDb) {
      continue;
    }
    sumRatioDb += ratioDb;
    _nXpolPoints++;

  }

  _meanXpolRatioDb = sumRatioDb / _nXpolPoints;
  if (!_params.use_xpol_ratio_from_spdb || _zdrCorr < -99) {
    _zdrCorr = _statsForZdrBias.meanS1S2 + _meanXpolRatioDb;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "======= cross polar ration from scan =======" << endl;
    cerr << "  nXpolPoints: " << _nXpolPoints << endl;
    cerr << "  _meanXpolRatioDb: " << _meanXpolRatioDb << endl;
    cerr << "  _meanS1S2: " << _statsForZdrBias.meanS1S2 << endl;
    cerr << "  _zdrCorr: " << _zdrCorr << endl;
    cerr << "============================================" << endl;
  }

}

/////////////////////////////////////////////////
// compute test pulse powers and ratios
    
void SunCal::_computeTestPulse()
  
{

  _testPulseDbmHc = MomentsSun::missing;
  _testPulseDbmVc = MomentsSun::missing;
  _testPulseDbmHx = MomentsSun::missing;
  _testPulseDbmVx = MomentsSun::missing;
  
  double sumPowerHc = 0.0;
  double sumPowerVc = 0.0;
  double sumPowerHx = 0.0;
  double sumPowerVx = 0.0;

  double nHc = 0.0;
  double nVc = 0.0;
  double nHx = 0.0;
  double nVx = 0.0;
  
  double noisePowerHc = pow(10.0, _noiseDbmHc / 10.0);
  double noisePowerVc = pow(10.0, _noiseDbmVc / 10.0);
  double noisePowerHx = pow(10.0, _noiseDbmHx / 10.0);
  double noisePowerVx = pow(10.0, _noiseDbmVx / 10.0);

  // loop through moments

  for (size_t ii = 0; ii < _testPulseMoments.size(); ii++) {
    
    const TestPulse &moments = _testPulseMoments[ii];
    
    // noise subtracted power
    
    if (moments.powerHc != MomentsSun::missing) {
      double powerHcNs = moments.powerHc - noisePowerHc;
      sumPowerHc += powerHcNs;
      nHc++;
    }

    if (moments.powerVc != MomentsSun::missing) {
      double powerVcNs = moments.powerVc - noisePowerVc;
      sumPowerVc += powerVcNs;
      nVc++;
    }

    if (moments.powerHx != MomentsSun::missing) {
      double powerHxNs = moments.powerHx - noisePowerHx;
      sumPowerHx += powerHxNs;
      nHx++;
    }

    if (moments.powerVx != MomentsSun::missing) {
      double powerVxNs = moments.powerVx - noisePowerVx;
      sumPowerVx += powerVxNs;
      nVx++;
    }

  }

  if (nHc > 0) {
    _testPulseDbmHc = 10.0 * log10(sumPowerHc / nHc);
  }

  if (nVc > 0) {
    _testPulseDbmVc = 10.0 * log10(sumPowerVc / nHc);
  }

  if (nHx > 0) {
    _testPulseDbmHx = 10.0 * log10(sumPowerHx / nHx);
  }

  if (nVx > 0) {
    _testPulseDbmVx = 10.0 * log10(sumPowerVx / nVx);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=============== test pulse powers ================" << endl;
    cerr << "  nHc, _testPulseDbmHc: "
         << nHc << ", " << _testPulseDbmHc << endl;
    cerr << "  nvc, _testPulseDbmVc: "
         << nVc << ", " << _testPulseDbmVc << endl;
    cerr << "  nHx, _testPulseDbmHx: "
         << nHx << ", " << _testPulseDbmHx << endl;
    cerr << "  nVx, _testPulseDbmVx: "
         << nVx << ", " << _testPulseDbmVx << endl;
    cerr << "==================================================" << endl;
  }

}

/////////////////////////////////////////////////
// print the PointsMoments data
    
void SunCal::_printRawMomentsArray(ostream &out)
  
{
  
  out << "================= POINT MOMENTS ARRAY ===================" << endl;

  for (int iaz = 0; iaz < (int) _rawMoments.size(); iaz++) {
    double az = _gridMinAz + iaz * _gridDeltaAz;
    vector<MomentsSun *> &moments = _rawMoments[iaz];
    cerr << "--------- Az " << az << " deg ---------------" << endl;
    cerr << "nmoments: " << moments.size() << endl;
    for (int iel = 0; iel < (int) moments.size(); iel++) {
      moments[iel]->print(out);
    } // iel
  } // iaz

  out << "=======================================================" << endl;

}

////////////////////////////////////////
// compare moments for sorting puroses

// Sort on elevation angles

bool SunCal::_compareMomentsEl(MomentsSun *lhs, MomentsSun *rhs)

{
  if (lhs->offsetEl < rhs->offsetEl) {
    return true;
  } else {
    return false;
  }
}

// Sort on azimuth angles

bool SunCal::_compareMomentsAz(MomentsSun *lhs, MomentsSun *rhs)

{
  if (lhs->offsetAz < rhs->offsetAz) {
    return true;
  } else {
    return false;
  }
}

/////////////////////////////////
// print the info about this run

void SunCal::_printRunDetails(ostream &out)

{

  // get time

  DateTime now(time(NULL));
  
  out << "# RUN INFO: "
      << "year,month,day,hour,min,sec,nSamples,startGate,nGates"
      << endl;
  
  out << setfill('0');
  out << setw(4) << now.getYear() << ","
      << setw(2) << now.getMonth() << ","
      << setw(2) << now.getDay() << ","
      << setw(2) << now.getHour() << ","
      << setw(2) << now.getMin() << ","
      << setw(2) << now.getSec() << ","
      << _params.n_samples << ","
      << _params.start_gate << ","
      << _params.n_gates << endl;
  out << setfill(' ');
  out << flush;
  
}

////////////////////////
// print the ops info

void SunCal::_printOpsInfo(ostream &out)

{

  const IwrfTsInfo &info = _tsReader->getOpsInfo();
  info.print(stdout);
  out << flush;

}

/////////////////////////////////
// print moments labels

void SunCal::_printMomentsLabels(ostream &out)
  
{

  out << "#   Start gate: " << _params.start_gate << endl;
  out << "#   N gates: " << _params.n_gates << endl;
  out << "#   N samples: " << _params.n_samples << endl;

  out << "#                  "
      << "                                            "
      << "   ----------- HV-flag 1 -----------"
      << "   ----------- HV-flag 0 -----------"
      << "  --All--  --All--"
      << endl;

  out << "#                  time     prf"
      << "      el      az"
      << "  del_el  del_az"
      << "    Copol     XPol     Corr      Arg"
      << "    Copol     Cpol     Corr      Arg"
      << "     IFD0     IFD1"
      << endl;

  out << flush;

}

/////////////////////////////////////////////////////////////////
// quadFit : fit a quadratic to a data series
//
// Mike Dixon  RAP, NCAR, Boulder, Colorado
//
// October 1990
//
//  n: number of points in (x, y) data set
//  x: array of x data
//  y: array of y data
//  a? - quadratic coefficients (cc - bias, bb - linear, aa - squared)
//  std_error - standard error of estimate
//  r_squared - correlation coefficient squared
//
// Returns 0 on success, -1 on error.
//
/////////////////////////////////////////////////////////////////

int SunCal::_quadFit(int n,
                     const vector<double> &x,
                     const vector<double> &y,
                     double &cc,
                     double &bb,
                     double &aa,
                     double &std_error_est,
                     double &r_squared)
  
{
  
  long i;

  double sumx = 0.0, sumx2 = 0.0, sumx3 = 0.0, sumx4 = 0.0;
  double sumy = 0.0, sumxy = 0.0, sumx2y = 0.0;
  double dn;
  double term1, term2, term3, term4, term5;
  double diff;
  double ymean, sum_dy_squared = 0.0;
  double sum_of_residuals = 0.0;
  double xval, yval;

  if (n < 4)
    return (-1);

  dn = (double) n;
  
  // sum the various terms

  for (i = 0; i < n; i++) {

    xval = x[i];
    yval = y[i];

    sumx = sumx + xval;
    sumx2 += xval * xval;
    sumx3 += xval * xval * xval;
    sumx4 += xval * xval * xval * xval;
    sumy += yval;
    sumxy += xval  * yval;
    sumx2y += xval * xval * yval;

  }

  ymean = sumy / dn;

  // compute the coefficients

  term1 = sumx2 * sumy / dn - sumx2y;
  term2 = sumx * sumx / dn - sumx2;
  term3 = sumx2 * sumx / dn - sumx3;
  term4 = sumx * sumy / dn - sumxy;
  term5 = sumx2 * sumx2 / dn - sumx4;

  aa = (term1 * term2 / term3 - term4) / (term5 * term2 / term3 - term3);
  bb = (term4 - term3 * aa) / term2;
  cc = (sumy - sumx * bb  - sumx2 * aa) / dn;

  // compute the sum of the residuals

  for (i = 0; i < n; i++) {
    xval = x[i];
    yval = y[i];
    diff = (yval - cc - bb * xval - aa * xval * xval);
    sum_of_residuals += diff * diff;
    sum_dy_squared += (yval - ymean) * (yval - ymean);
  }

  // compute standard error of estimate and r-squared
  
  std_error_est = sqrt(sum_of_residuals / (dn - 3.0));
  r_squared = ((sum_dy_squared - sum_of_residuals) /
	       sum_dy_squared);
  
  return 0;

}


///////////////////////////////
// check for end of vol

void SunCal::_checkEndOfVol(double el)

{

  if (_nBeamsThisVol == 0) {
    _volMinEl = el;
    _volMaxEl = el;
  } else {
    if (el < _volMinEl) {
      _volMinEl = el;
    }
    if (el > _volMaxEl) {
      _volMaxEl = el;
    }
  }

  _endOfVol = false;
  
  if (_volNum != _prevVolNum) {
    if (_prevVolNum >= 0) {
      if (_params.debug) {
        cerr << "-->> END OF VOL, change in vol num" << "<<--" << endl;
        cerr << "     prev vol num: " << _prevVolNum << endl;
        cerr << "     this vol num: " << _volNum << endl;
      }
      _endOfVol = true;
    }
    _prevVolNum = _volNum;
  }

  if (!_endOfVol && !_isRhi &&
      _params.check_for_elevation_change &&
      _volInProgress) {
    if (_nBeamsThisVol >= _params.min_beams_per_volume) {
      if (_params.volume_starts_at_bottom) {
        double deltaEl = _volMaxEl - el;
        if (deltaEl > 1.0) {
        }
        if (deltaEl > _params.elev_change_for_end_of_volume) {
          _endOfVol = true;
          _volInProgress = false;
          if (_params.debug) {
            cerr << "-->> END OF VOL, antenna coming down" << "<<--" << endl;
            cerr << "     el, deltaEl: " << el << ", " << deltaEl << endl;
          }
        }
      } else {
        double deltaEl = el - _volMinEl;
        if (deltaEl > _params.elev_change_for_end_of_volume) {
          _endOfVol = true;
          _volInProgress = false;
          if (_params.debug) {
            cerr << "-->> END OF VOL, antenna going up" << "<<--" << endl;
            cerr << "     el, deltaEl: " << el << ", " << deltaEl << endl;
          }
        }
      } // if (_params.volume_starts_at_bottom) 
    } // if (_nBeamsThisVol >= _params.min_beams_per_volume)
  }
  
  if (_nBeamsThisVol >= _params.max_beams_per_volume) {
    if (_params.debug) {
      cerr << "-->> END OF VOL, max beams exceeded: "
           << _nBeamsThisVol << endl;
    }
    _endOfVol = true;
  }

  // look for reversal after end of volume

  if (_params.check_for_elevation_change) {
    if (_params.volume_starts_at_bottom) {
      if (el > _prevEl) {
        _volInProgress = true;
      }
    } else {
      if (el < _prevEl) {
        _volInProgress = true;
      }
    }
  }
    
  if (_endOfVol) {
    _nBeamsThisVol = 0;
    _volMinEl = el;
    _volMaxEl = el;
  } else {
    if (el < _volMinEl) {
      _volMinEl = el;
    }
    if (el > _volMaxEl) {
      _volMaxEl = el;
    }
  }

  _prevEl = el;

}

/////////////////////////////////////////////////////////////////
// Allocate or re-allocate gate data

void SunCal::_allocGateData()

{

  // set n samples
  // ensure we have an even number of samples

  _nSamples = (int) _pulseQueue.size();
  _nSamplesHalf = _nSamples / 2;
  _nSamples = _nSamplesHalf * 2;

  // set n gates to minimum number for any pulse in queue

  _nGates = _pulseQueue[0]->getNGates();
  for (size_t ii = 1; ii < _pulseQueue.size(); ii++) {
    if (_nGates > _pulseQueue[ii]->getNGates()) {
      _nGates = _pulseQueue[ii]->getNGates();
    }
  }

  _startGateSun = _params.start_gate;
  _endGateSun = _startGateSun + _params.n_gates - 1;
  if (_endGateSun > _nGates - 1) {
    _endGateSun = _nGates - 1;
  }

  // allocate

  int nNeeded = _nGates - (int) _gateData.size();
  if (nNeeded > 0) {
    for (int ii = 0; ii < nNeeded; ii++) {
      GateData *gate = new GateData();
      _gateData.push_back(gate);
    }
  }
  for (size_t ii = 0; ii < _gateData.size(); ii++) {
    _gateData[ii]->allocArrays(_nSamples, false, false, false);
  }

  _startRangeKm = _pulseQueue[0]->get_start_range_km();
  _gateSpacingKm = _pulseQueue[0]->get_gate_spacing_km();

}

/////////////////////////////////////////////////////////////////
// Free gate data

void SunCal::_freeGateData()

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

int SunCal::_loadGateData()
  
{

  // alloc gate data
  
  _allocGateData();

  if (_isDualPol()) {
    _dualPol = true;
  } else {
    _dualPol = false;
  }

  if (_isAlternating()) {
    _alternating = true;
  } else {
    _alternating = false;
  }

  if (_dualPol) {
    if (_alternating) {
      return _loadGateDataDualPolAlt();
    } else {
      return _loadGateDataDualPolSim();
    }
  } else {
    return _loadGateDataSinglePol();
  }

}

/////////////////////////////////////////////////////////////////
// Load gate IQ data for alternating mode
// returns 0 on success, -1 on failure

int SunCal::_loadGateDataDualPolAlt()
  
{

  // set up data pointer arrays

  TaArray<const fl32 *> iqChan0_, iqChan1_;
  const fl32* *iqChan0 = iqChan0_.alloc(_nSamples);
  const fl32* *iqChan1 = iqChan1_.alloc(_nSamples);
  for (int isamp = 0; isamp < _nSamples; isamp++) {
    if (_pulseQueue[isamp]->getIq0() == NULL) {
      cerr << "ERROR - SunCal::_loadGateDataDualPolAlt()" << endl;
      cerr << "  Bad channel 0 IQ data, found NULL" << endl;
      return -1;
    }
    if (_pulseQueue[isamp]->getIq1() == NULL) {
      cerr << "ERROR - SunCal::_loadGateDataDualPolAlt()" << endl;
      cerr << "  Bad channel 1 IQ data, found NULL" << endl;
      return -1;
    }
    iqChan0[isamp] = _pulseQueue[isamp]->getIq0();
    iqChan1[isamp] = _pulseQueue[isamp]->getIq1();
  }
  
  // load up IQ arrays, gate by gate

  for (int igate = 0, ipos = 0;  igate < _nGates; igate++, ipos += 2) {
    
    GateData *gate = _gateData[igate];
    RadarComplex_t *iqhc = gate->iqhc;
    RadarComplex_t *iqvc = gate->iqvc;
    RadarComplex_t *iqhx = gate->iqhx;
    RadarComplex_t *iqvx = gate->iqvx;

    // samples start on horiz pulse
    
    for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
         isamp++, iqhc++, iqvc++, iqhx++, iqvx++) {
      
      if (_switching) {
        
        // H co-polar, V cross-polar
        
        iqhc->re = iqChan0[jsamp][ipos];
        iqhc->im = iqChan0[jsamp][ipos + 1];
        iqvx->re = iqChan1[jsamp][ipos];
        iqvx->im = iqChan1[jsamp][ipos + 1];
        jsamp++;
        
        // V co-polar, H cross-polar
        
        iqvc->re = iqChan0[jsamp][ipos];
        iqvc->im = iqChan0[jsamp][ipos + 1];
        iqhx->re = iqChan1[jsamp][ipos];
        iqhx->im = iqChan1[jsamp][ipos + 1];
        jsamp++;
        
      } else {
        
        // H from chan 0, V from chan 1
        
        iqhc->re = iqChan0[jsamp][ipos];
        iqhc->im = iqChan0[jsamp][ipos + 1];
        iqvx->re = iqChan1[jsamp][ipos];
        iqvx->im = iqChan1[jsamp][ipos + 1];
        jsamp++;
        
        iqhx->re = iqChan0[jsamp][ipos];
        iqhx->im = iqChan0[jsamp][ipos + 1];
        iqvc->re = iqChan1[jsamp][ipos];
        iqvc->im = iqChan1[jsamp][ipos + 1];
        jsamp++;

      }
      
    } // isamp

  } // igate

  return 0;

}

/////////////////////////////////////////////////////////////////
// Load gate IQ data for simultaneous mode

int SunCal::_loadGateDataDualPolSim()
  
{

  // set up data pointer arrays

  TaArray<const fl32 *> iqChan0_, iqChan1_;
  const fl32* *iqChan0 = iqChan0_.alloc(_nSamples);
  const fl32* *iqChan1 = iqChan1_.alloc(_nSamples);
  for (int isamp = 0; isamp < _nSamples; isamp++) {
    if (_pulseQueue[isamp]->getIq0() == NULL) {
      cerr << "ERROR - SunCal::_loadGateDataDualPolSim()" << endl;
      cerr << "  Bad channel 0 IQ data, found NULL" << endl;
      return -1;
    }
    if (_pulseQueue[isamp]->getIq1() == NULL) {
      cerr << "ERROR - SunCal::_loadGateDataDualPolSim()" << endl;
      cerr << "  Bad channel 1 IQ data, found NULL" << endl;
      return -1;
    }
    iqChan0[isamp] = _pulseQueue[isamp]->getIq0();
    iqChan1[isamp] = _pulseQueue[isamp]->getIq1();
  }
  
  // load up IQ arrays, gate by gate

  for (int igate = 0, ipos = 0;  igate < _nGates; igate++, ipos += 2) {
    
    GateData *gate = _gateData[igate];
    RadarComplex_t *iqhc = gate->iqhc;
    RadarComplex_t *iqvc = gate->iqvc;
    
    // samples start on horiz pulse
    
    for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++, iqvc++) {
      
      // H from chan 0, V from chan 1
        
      iqhc->re = iqChan0[isamp][ipos];
      iqhc->im = iqChan0[isamp][ipos + 1];
      iqvc->re = iqChan1[isamp][ipos];
      iqvc->im = iqChan1[isamp][ipos + 1];

    } // isamp

  } // igate
  
  return 0;

}

/////////////////////////////////////////////////////////////////
// Load gate IQ data for single pol

int SunCal::_loadGateDataSinglePol()
  
{

  // set up data pointer arrays
  
  TaArray<const fl32 *> iqChan0_;
  const fl32* *iqChan0 = iqChan0_.alloc(_nSamples);
  for (int isamp = 0; isamp < _nSamples; isamp++) {
    if (_pulseQueue[isamp]->getIq0() == NULL) {
      cerr << "ERROR - SunCal::_loadGateDataSinglePol()" << endl;
      cerr << "  Bad channel 0 IQ data, found NULL" << endl;
      return -1;
    }
    iqChan0[isamp] = _pulseQueue[isamp]->getIq0();
  }
  
  // load up IQ arrays, gate by gate

  for (int igate = 0, ipos = 0;  igate < _nGates; igate++, ipos += 2) {
    
    GateData *gate = _gateData[igate];
    RadarComplex_t *iqhc = gate->iqhc;
    
    // samples start on horiz pulse
    
    for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++) {
      iqhc->re = iqChan0[isamp][ipos];
      iqhc->im = iqChan0[isamp][ipos + 1];
    }

  }

  return 0;

}

//////////////////////////////////////////////////////////
// Make sure pulse queue starts on H for alternating mode
// Returns 0 on success, -1 on failure

int SunCal::_checkAlternatingStartsOnH()
{

  if (!_alternating) {
    // does not apply
    return 0;
  }

  if (_startsOnH()) {
    // already starts on H
    return 0;
  }

  // does not start on H, need to shift the queue by 1 pulse

  // first delete oldest pulse
  
  const IwrfTsPulse *oldest = _pulseQueue.front();
  if (oldest->removeClient() == 0) {
    delete oldest;
  }
  _pulseQueue.pop_front();

  // read another pulse
  
  const IwrfTsPulse *pulse = _tsReader->getNextPulse(true);
  if (pulse == NULL) {
    return -1;
  }

  // push pulse onto queue
  
  pulse->addClient();
  _pulseQueue.push_back(pulse);
  
  return 0;

}

///////////////////////////////////////////      
// check if we have alternating h/v pulses

bool SunCal::_isAlternating()
  
{
  
  bool prevHoriz = _pulseQueue[0]->isHoriz();
  for (size_t ii = 1; ii < _pulseQueue.size(); ii++) {
    bool thisHoriz = _pulseQueue[ii]->isHoriz();
    if (thisHoriz == prevHoriz) {
      return false;
    }
    prevHoriz = thisHoriz;
  }

  return true;
  
}

///////////////////////////////////////////      
// check if pulse queue starts on h
// only applies to alternating mode

bool SunCal::_startsOnH()
  
{
  
  // we want to start on H
  // the starting pulse is at the end of the queue

  if (_pulseQueue[0]->isHoriz()) {
    return true;
  } else {
    return false;
  }

}

///////////////////////////////////////////      
// check if data is dual pol

bool SunCal::_isDualPol()
  
{

  // check if we have both channels

  if (_pulseQueue[0]->getIq0() != NULL &&
      _pulseQueue[0]->getIq1() != NULL) {
    return true;
  }

  // no, so single pol

  return false;
  
}

//////////////////////////////////////////////////
// retrieve xpol ratio from SPDB for scan time

int SunCal::_retrieveXpolRatioFromSpdb(time_t scanTime,
                                       double &xpolRatio,
                                       time_t &timeForXpolRatio)
  
{

  xpolRatio = -9999.0;
  timeForXpolRatio = time(NULL);
  
  if (!_params.read_xpol_ratio_from_spdb) {
    return 0;
  }
  
  // get temp data from SPDB
  
  DsSpdb spdb;
  si32 dataType = 0;
  if (strlen(_params.xpol_ratio_radar_name) > 0) {
    dataType =  Spdb::hash4CharsToInt32(_params.xpol_ratio_radar_name);
  }
  if (spdb.getClosest(_params.xpol_ratio_spdb_url,
                      scanTime,
                      _params.xpol_ratio_search_margin_secs,
                      dataType)) {
    cerr << "WARNING - SunCal::_retrieveXpolRatioFromSpdb" << endl;
    cerr << "  Cannot get xpol ratio from URL: " 
         << _params.xpol_ratio_spdb_url << endl;
    cerr << "  Search time: " << RadxTime::strm(scanTime) << endl;
    cerr << "  Search margin (secs): "
         << _params.xpol_ratio_search_margin_secs << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  // got chunks
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  if (chunks.size() < 1) {
    cerr << "WARNING - SunCal::_retrieveXpolRatioFromSpdb" << endl;
    cerr << "  No suitable xpol ratio data from URL: "
         << _params.xpol_ratio_spdb_url << endl;
    cerr << "  Search time: " << RadxTime::strm(scanTime) << endl;
    cerr << "  Search margin (secs): "
         << _params.xpol_ratio_search_margin_secs << endl;
    return -1;
  }

  const Spdb::chunk_t &chunk = chunks[0];
  string xml((const char *) chunk.data);
  double ratio = 0.0;
  if (TaXml::readDouble(xml, "ratioHxVxDbClutter", ratio)) {
    return -1;
  }

  xpolRatio = ratio;
  timeForXpolRatio = chunk.valid_time;
  
  return 0;
  
}

//////////////////////////////////////////////////
// retrieve site temp from SPDB for scan time

int SunCal::_retrieveSiteTempFromSpdb(time_t scanTime,
                                      double &tempC,
                                      time_t &timeForTemp)
  
{

  tempC = -9999.0;
  timeForTemp = time(NULL);
  
  if (!_params.read_site_temp_from_spdb) {
    return 0;
  }
  
  // get temp data from SPDB
  
  DsSpdb spdb;
  si32 dataType = _params.site_temp_data_type;
  if (dataType != -1 && strlen(_params.site_temp_station_name) > 0) {
    dataType =  Spdb::hash4CharsToInt32(_params.site_temp_station_name);
  }

  if (spdb.getClosest(_params.site_temp_spdb_url,
                      scanTime,
                      _params.site_temp_search_margin_secs,
                      dataType)) {
    cerr << "WARNING - SunCal::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  Cannot get temperature from URL: "
         << _params.site_temp_spdb_url << endl;
    cerr << "  Station name: " << _params.site_temp_station_name << endl;
    cerr << "  Search time: " << RadxTime::strm(scanTime) << endl;
    cerr << "  Search margin (secs): "
         << _params.site_temp_search_margin_secs << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  // got chunks
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  if (chunks.size() < 1) {
    cerr << "WARNING - SunCal::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  No suitable temp data from URL: " 
         << _params.site_temp_spdb_url << endl;
    cerr << "  Search time: " << RadxTime::strm(scanTime) << endl;
    cerr << "  Search margin (secs): "
         << _params.site_temp_search_margin_secs << endl;
    return -1;
  }

  const Spdb::chunk_t &chunk = chunks[0];
  WxObs obs;
  if (obs.disassemble(chunk.data, chunk.len)) {
    cerr << "WARNING - SunCal::_retrieveSiteTempFromSpdb" << endl;
    cerr << "  SPDB data is of incorrect type, prodLabel: "
         << spdb.getProdLabel() << endl;
    cerr << "  Should be station data type" << endl;
    cerr << "  URL: " << _params.site_temp_spdb_url << endl;
    return -1;
  }
  
  tempC = obs.getTempC();
  timeForTemp = obs.getObservationTime();

  if (_params.debug) {
    cerr << "Got temp data from URL: " << _params.site_temp_spdb_url << endl;
    cerr << "  Search time: " << RadxTime::strm(scanTime) << endl;
    cerr << "  Search margin (secs): "
         << _params.site_temp_search_margin_secs << endl;
    cerr << "  Temp time, tempC: "
         << DateTime::strm(timeForTemp) << ", " << tempC << endl;
  }
  
  return 0;
  
}

/////////////////////////////////////////////////////////////////
// get temperature from status xml in the time series,
// given the tag list
// returns 0 on success, -1 on failure

int SunCal::_retrieveSiteTempFromXml(time_t scanTime,
                                     double &tempC,
                                     time_t &timeForTemp)
  
{

  // init to error condition
  
  tempC = -9999.0;
  
  // get the XML string

  const IwrfTsInfo &info = _tsReader->getOpsInfo();
  if (!info.isStatusXmlActive()) {
    return -1;
  }
  timeForTemp = (time_t) info.getStatusXmlTime();
  string xmlStr = info.getStatusXmlStr();

  double tdiff = fabs((double) timeForTemp - (double) scanTime);
  if (tdiff > 3600) {
    cerr << "WARNING - SunCal::_retrieveSiteTempFromXml()" << endl;
    cerr << "  No recent temp data" << endl;
    cerr << "  Latest temp time: " << DateTime::strm(timeForTemp) << endl;
    return -1;
  }

  // get tags in list
    
  string tagList = _params.temp_tag_list_in_status_xml;
  vector<string> tags;
  TaStr::tokenize(tagList, "<>", tags);
  if (tags.size() == 0) {
    // no tags
    cerr << "WARNING - SunCal::_retrieveSiteTempFromXml()" << endl;
    cerr << "  Temp tags not found: " << tagList << endl;
    return -1;
  }
  
  // read through the outer tags in status XML
  
  string buf(xmlStr);
  for (size_t jj = 0; jj < tags.size(); jj++) {
    string val;
    if (TaXml::readString(buf, tags[jj], val)) {
      cerr << "WARNING - SunCal::_retrieveSiteTempFromXml()" << endl;
      cerr << "  Bad tags found in status xml, expecting: "
           << tagList << endl;
      return -1;
    }
    buf = val;
  }

  // read temperature
  
  double tempVal = -9999.0;
  if (TaXml::readDouble(buf, tempVal)) {
    cerr << "WARNING - SunCal::_retrieveSiteTempFromXml()" << endl;
    cerr << "  Bad temp found in status xml, buf: " << buf << endl;
    return -1;
  }

  // success

  tempC = tempVal;
  return 0;

}
  
//////////////////////////
// write out results data

int SunCal::_writeSummaryText()
  
{
  
  // create the directory for the output file

  if (ta_makedir_recurse(_params.text_output_dir)) {
    int errNum = errno;
    cerr << "ERROR - SunCal::_writeSummaryText";
    cerr << "  Cannot create output dir: " << _params.text_output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute file path
  
  time_t calTime = (time_t) _calTime;
  DateTime ctime(calTime);
  char path[1024];
  sprintf(path, "%s/SunCal.%s.%.4d%.2d%.2d_%.2d%.2d%.2d.txt",
          _params.text_output_dir,
          _params.radar_name,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());

  if (_params.debug) {
    cerr << "writing output file: " << path << endl;
  }

  // open file

  FILE *out;
  if ((out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SunCal::_writeSummaryTextFile";
    cerr << "  Cannot create file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  _writeSummaryText(out);

  fclose(out);

  return 0;

}

//////////////////////////
// write out results data

void SunCal::_writeSummaryText(FILE *out)
  
{
  
  fprintf(out, "SunCal output\n");
  fprintf(out, "=============\n");
  fprintf(out, "\n");
  
  fprintf(out, "Data time: %s\n",
          DateTime::strm((time_t) _calTime).c_str());
  fprintf(out, "\n");
  
  fprintf(out, "Radar name: %s\n", _params.radar_name);
  fprintf(out, "Site name: %s\n", _params.radar_site);
  fprintf(out, "Radar lat (deg): %g\n", _radarLat); 
  fprintf(out, "Radar lon (deg): %g\n", _radarLon); 
  fprintf(out, "Radar alt (km): %g\n", _radarAltKm); 
  fprintf(out, "Mean sun el (deg): %g\n", _meanSunEl); 
  fprintf(out, "Mean sun az (deg): %g\n", _meanSunAz); 
  fprintf(out, "\n");

  fprintf(out, "N samples per beam: %d\n", _params.n_samples); 
  fprintf(out, "N sun scans used: %d\n", _volCount); 
  fprintf(out, "\n");

  fprintf(out, "========================\n");
  fprintf(out, "Noise powers\n");
  if (_params.noise_method == Params::GET_NOISE_FROM_CAL_FILE) {
    fprintf(out, "  Reading noise from cal file: %s\n",
            _params.cal_xml_file_path);
  } else if (_params.noise_method == Params::GET_NOISE_FROM_TIME_SERIES) {
    fprintf(out, "  Getting noise from time series\n");
  } else if (_params.noise_method == Params::COMPUTE_MIN_NOISE) {
    fprintf(out, "  Using min value from grid for noise\n");
  } else {
    fprintf(out, "  Using mean noise value from grid for noise\n");
    fprintf(out, "  Noise computed for off-sun angles > %g deg\n",
            _params.min_angle_offset_for_noise_power); 
    fprintf(out, "  N beams noise: %g\n", _nBeamsNoise);
  }
  fprintf(out, "\n");
  fprintf(out, "  noiseDbmHc (dBm): %10.4f\n", _noiseDbmHc);
  fprintf(out, "  noiseDbmVc (dBm): %10.4f\n", _noiseDbmVc);
  if (_alternating) {
    fprintf(out, "  noiseDbmHx (dBm): %10.4f\n", _noiseDbmHx);
    fprintf(out, "  noiseDbmVx (dBm): %10.4f\n", _noiseDbmVx);
  }
  fprintf(out, "\n");

  fprintf(out, "===================================\n");
  fprintf(out, "Mean correlations over the sun disk\n");
  fprintf(out, "  Solid angle used: %g\n",
          _params.max_solid_angle_for_mean_correlation);
  fprintf(out, "\n");
  if (_alternating) {
    fprintf(out, "  meanCorr00H  :   %10.4f\n", _meanCorr00H);
    fprintf(out, "  meanCorr00V  :   %10.4f\n", _meanCorr00V);
  } else {
    fprintf(out, "  meanCorr00  :  %10.4f\n", _meanCorr00);
  }
  fprintf(out, "\n");

  fprintf(out, "=========================\n");
  fprintf(out, "Sun offsets and max power\n");
  fprintf(out, "\n");
  fprintf(out, "  Hc channel\n");
  fprintf(out, "  Hc Sun max power          (dBm): %10.4f\n", _maxPowerDbmHc); 
  fprintf(out, "  Hc Sun quadratic power    (dBm): %10.4f\n", _quadPowerDbmHc); 
  fprintf(out, "  Hc Sun centroid offset az (deg): %10.4f\n", _sunCentroidAzOffsetHc); 
  fprintf(out, "  Hc Sun centroid offset el (deg): %10.4f\n", _sunCentroidElOffsetHc); 
  fprintf(out, "\n");
  fprintf(out, "  Vc channel\n");
  fprintf(out, "  Vc Sun max power          (dBm): %10.4f\n", _maxPowerDbmVc); 
  fprintf(out, "  Vc Sun quadratic power (   dBm): %10.4f\n", _quadPowerDbmVc); 
  fprintf(out, "  Vc Sun centroid offset az (deg): %10.4f\n", _sunCentroidAzOffsetVc); 
  fprintf(out, "  Vc Sun centroid offset el (deg): %10.4f\n", _sunCentroidElOffsetVc); 
  fprintf(out, "\n");
  fprintf(out, "  Mean of Hc & Vc\n");
  fprintf(out, "  Mc Sun max power          (dBm): %10.4f\n", _maxPowerDbm); 
  fprintf(out, "  Mc Sun quadratic power    (dBm): %10.4f\n", _quadPowerDbm); 
  fprintf(out, "  Mc Sun centroid offset az (deg): %10.4f\n", _sunCentroidAzOffset); 
  fprintf(out, "  Mc Sun centroid offset el (deg): %10.4f\n", _sunCentroidElOffset); 
  fprintf(out, "\n");

  fprintf(out, "  widthRatioElAzHc               : %10.4f\n", _widthRatioElAzHc); 
  fprintf(out, "  widthRatioElAzVc               : %10.4f\n", _widthRatioElAzVc); 
  fprintf(out, "  widthRatioElAzDiffHV           : %10.4f\n", _widthRatioElAzDiffHV); 
  fprintf(out, "  zdrDiffElAz                (dB): %10.4f\n", _zdrDiffElAz); 

  if (_params.compute_cross_polar_power_ratio) {
    fprintf(out, "========== cross polar ratio ==========\n");
    fprintf(out, "  nXpolPoints: %d\n", _nXpolPoints);
    fprintf(out, "  _meanXpolRatioDb: %g\n", _meanXpolRatioDb);
    fprintf(out, "  _meanS1S2: %g\n", _statsForZdrBias.meanS1S2);
    fprintf(out, "  _zdrCorr: %g\n", _zdrCorr);
    fprintf(out, "=======================================\n\n");
  }

  if (_params.read_xpol_ratio_from_spdb) {
    fprintf(out, "========== cross polar ratio from spdb ==========\n");
    fprintf(out, "  _xpolRatioDbFromSpdb: %g\n", _xpolRatioDbFromSpdb);
    fprintf(out, "  _meanS1S2: %g\n", _statsForZdrBias.meanS1S2);
    fprintf(out, "  _zdrCorr: %g\n", _zdrCorr);
    fprintf(out, "  _timeForXpolRatio: %s\n", DateTime::strm(_timeForXpolRatio).c_str());
    fprintf(out, "=================================================\n\n");
  }

  if (_params.read_site_temp_from_spdb) {
    fprintf(out, "========== site temp from spdb ==========\n");
    fprintf(out, "  _siteTempC: %g\n", _siteTempC);
    fprintf(out, "  _timeForSiteTemp: %s\n", DateTime::strm(_timeForSiteTemp).c_str());
    fprintf(out, "=================================================\n\n");
  }

  if (_params.read_site_temp_from_time_series_xml) {
    fprintf(out, "========== site temp from status xml ==========\n");
    fprintf(out, "  _siteTempC: %g\n", _siteTempC);
    fprintf(out, "  _timeForSiteTemp: %s\n", DateTime::strm(_timeForSiteTemp).c_str());
    fprintf(out, "=================================================\n\n");
  }

  if (_params.compute_test_pulse_powers) {
    fprintf(out, "=============== test pulse powers ================\n");
    fprintf(out, "  _testPulseDbmHc: %g\n", _testPulseDbmHc);
    fprintf(out, "  _testPulseDbmVc: %g\n", _testPulseDbmVc);
    fprintf(out, "  _testPulseDbmHx: %g\n", _testPulseDbmHx);
    fprintf(out, "  _testPulseDbmVx: %g\n", _testPulseDbmVx);
    fprintf(out, "==================================================\n\n");
  }

  if (_params.compute_mean_transmit_powers) {
    fprintf(out, "==================== xmit powers =================\n");
    fprintf(out, "  _meanXmitPowerHDbm: %g\n", _meanXmitPowerHDbm);
    fprintf(out, "  _meanXmitPowerVDbm: %g\n", _meanXmitPowerVDbm);
    if (_meanXmitPowerHDbm > 0 && _meanXmitPowerVDbm > 0) {
      fprintf(out, "  _xmitPowerHVDiff: %g\n", _meanXmitPowerHDbm - _meanXmitPowerVDbm);
    }
    fprintf(out, "==================================================\n\n");
  }

  for (int ii = 0; ii < (int) _stats.size(); ii++) {
    Stats &stats = _stats[ii];
    fprintf(out, "========================\n");
    fprintf(out, "Cross-polar power stats\n");
    if (stats.solidAngle >= 0) {
      fprintf(out, "Solid angle (deg): %g\n", stats.solidAngle);
    } else {
      fprintf(out, "Computed from beams ranked by power\n");
    }
    fprintf(out, "  n beamns used        : %10d\n", stats.nBeamsUsed);
    fprintf(out, "  mean power      (dBm): %10.4f\n", stats.meanDbm);
    fprintf(out, "  min power       (dBm): %10.4f\n", stats.minDbm);
    fprintf(out, "  quad peak power (dBm): %10.4f\n", _quadPowerDbm);
    fprintf(out, "  min below peak  (dBm): %10.4f\n",
            stats.minDbm - _quadPowerDbm);
    fprintf(out, "  mean Vc/Hc      (dBm): %10.4f\n", stats.meanRatioDbmVcHc);
    if (_alternating) {
      fprintf(out, "  mean Vx/Hx      (dBm): %10.4f\n", stats.meanRatioDbmVxHx);
      fprintf(out, "  mean Vc/Hx      (dBm): %10.4f\n", stats.meanRatioDbmVcHx);
      fprintf(out, "  mean Vx/Hc      (dBm): %10.4f\n", stats.meanRatioDbmVxHc);
      fprintf(out, "  meanCorr00Hc         : %10.4f\n", stats.meanCorr00Hc);
      fprintf(out, "  meanCorr00Vc         : %10.4f\n", stats.meanCorr00Vc);
    } else {
      fprintf(out, "  meanCorr00           : %10.4f\n", stats.meanCorr00);
    }
    fprintf(out, "  sdev of mean    (dBm): %10.4f\n", stats.SdevOfMean);
    fprintf(out, "  meanVc/meanHc   (dBm): %10.4f\n", stats.ratioMeanVcHcDb);
    if (_alternating) {
      fprintf(out, "  meanVx/meanHx   (dBm): %10.4f\n", stats.ratioMeanVxHxDb);
      // fprintf(out, "  S1S2 from ratios(dBm): %10.4f\n", stats.meanS1S2);
      fprintf(out, "  sdev S1S2       (dBm): %10.4f\n", stats.sdevS1S2);
      fprintf(out, "  mean S1S2       (dBm): %10.4f\n", stats.ratioMeanS1S2Db);
    } else {
      // fprintf(out, "  SS from ratios(dBm): %10.4f\n", stats.meanSS);
      fprintf(out, "  sdev SS       (dBm): %10.4f\n", stats.sdevSS);
      fprintf(out, "  mean SS       (dBm): %10.4f\n", stats.ratioMeanSSDb);
    }
    fprintf(out, "\n");

  }
  
  fprintf(out, "\n");

}

///////////////////////////////////
// write out global results to file

int SunCal::_appendToGlobalResults()

{

  // create the directory for the output file

  if (ta_makedir_recurse(_params.text_output_dir)) {
    int errNum = errno;
    cerr << "ERROR - SunCal::_appendToGlobalResults";
    cerr << "  Cannot create output dir: " << _params.text_output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute path

  char path[1024];
  sprintf(path, "%s/%s",
          _params.text_output_dir, _params.global_file_name);
  
  if (_params.debug) {
    cerr << "Appending to global file: " << path << endl;
  }

  // open file
  
  FILE *out;
  if ((out = fopen(path, "a")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SunCal::_appendToGlobalResults";
    cerr << "  Cannot open file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if ((_globalPrintCount % 20) == 0) {
    _writeGlobalHeader(out);
  }
  _globalPrintCount++;

  // print date/time

  DateTime ctime((time_t) _calTime);
  
  fprintf(out, "%.4d %.2d %.2d %.2d %.2d %.2d",
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());

  long utime = (long) ctime.utime();
  double udays = (double) utime / 86400.0;
  fprintf(out, " %12.6f", udays);

  // sun location

  _appendFloatToFile(out, _meanSunEl);
  _appendFloatToFile(out, _meanSunAz);

  // noise

  _appendFloatToFile(out, _noiseDbmHc);
  _appendFloatToFile(out, _noiseDbmVc);
  if (_alternating) {
    _appendFloatToFile(out, _noiseDbmHx);
    _appendFloatToFile(out, _noiseDbmVx);
  }

  // mean correlation

  if (_alternating) {
    _appendFloatToFile(out, _meanCorr00H);
    _appendFloatToFile(out, _meanCorr00V);
  } else {
    _appendFloatToFile(out, _meanCorr00);
  }

  // sun power and centroid offset

  _appendFloatToFile(out, _maxPowerDbm); 
  _appendFloatToFile(out, _quadPowerDbm); 
  _appendFloatToFile(out, _sunCentroidAzOffset); 
  _appendFloatToFile(out, _sunCentroidElOffset); 

  // channel power ratios

  _appendFloatToFile(out, _statsForZdrBias.meanRatioDbmVcHc);
  if (_alternating) {
    _appendFloatToFile(out, _statsForZdrBias.meanRatioDbmVxHx);
    _appendFloatToFile(out, _statsForZdrBias.meanRatioDbmVcHx);
    _appendFloatToFile(out, _statsForZdrBias.meanRatioDbmVxHc);
  }
  
  // cross-polar power ratio

  if (_params.compute_cross_polar_power_ratio) {
    _appendFloatToFile(out, _meanXpolRatioDb);
    _appendFloatToFile(out, _statsForZdrBias.meanS1S2);
    _appendFloatToFile(out, _zdrCorr);
  }

  if (_params.read_xpol_ratio_from_spdb) {
    _appendFloatToFile(out, _xpolRatioDbFromSpdb);
  }

  if (_params.read_site_temp_from_spdb ||
      _params.read_site_temp_from_time_series_xml) {
    _appendFloatToFile(out, _siteTempC);
  }

  if (_params.compute_test_pulse_powers) {
    _appendFloatToFile(out, _testPulseDbmHc);
    _appendFloatToFile(out, _testPulseDbmVc);
    _appendFloatToFile(out, _testPulseDbmHx);
    _appendFloatToFile(out, _testPulseDbmVx);
    _appendFloatToFile(out,
                       _testPulseDbmVc - _testPulseDbmHc);
    _appendFloatToFile(out,
                       _testPulseDbmVx - _testPulseDbmHx);
    _appendFloatToFile(out,
                       _testPulseDbmVc - _testPulseDbmHx);
    _appendFloatToFile(out,
                       _testPulseDbmVx - _testPulseDbmHc);
  }

  if (_params.compute_mean_transmit_powers) {
    _appendFloatToFile(out, _meanXmitPowerHDbm);
    _appendFloatToFile(out, _meanXmitPowerVDbm);
    if (_meanXmitPowerHDbm > 0 && _meanXmitPowerVDbm > 0) {
      _appendFloatToFile(out, _meanXmitPowerHDbm - _meanXmitPowerVDbm);
    } else {
      _appendFloatToFile(out, -9999.0);
    }
  }

  // ellipse ratios

  if (_params.compute_ellipse_hv_power_diffs) {
    _appendFloatToFile(out, _widthRatioElAzHc, 16); 
    _appendFloatToFile(out, _widthRatioElAzVc, 16); 
    _appendFloatToFile(out, _widthRatioElAzDiffHV, 20); 
    _appendFloatToFile(out, _zdrDiffElAz, 12); 
  }

  fprintf(out, "\n");
  fclose(out);

  return 0;

}
  

///////////////////////////////////
// write out floating point value

void SunCal::_appendFloatToFile(FILE *out, double val, int width /* = 10 */)

{
  char format[32];
  sprintf(format, " %%%d.4f", width);
  if (isfinite(val)) {
    fprintf(out, format, val);
  } else {
    fprintf(out, format, -9999.0);
  }
}

///////////////////////////////////
// write out global header to file

void SunCal::_writeGlobalHeader(FILE *out)

{
  
  // print date/time

  fprintf(out, "yyyy mm dd hh mm ss");
  fprintf(out, " %12s", "days");

  // sun location

  fprintf(out, " %10s", "meanSunEl"); 
  fprintf(out, " %10s", "meanSunAz"); 

  // noise
  
  fprintf(out, " %10s", "NoiseHc");
  fprintf(out, " %10s", "NoiseVc");
  if (_alternating) {
    fprintf(out, " %10s", "NoiseHx");
    fprintf(out, " %10s", "NoiseVx");
  }

  // mean correlation

  if (_alternating) {
    fprintf(out, " %10s", "Corr00H");
    fprintf(out, " %10s", "Corr00V");
  } else {
    fprintf(out, " %10s", "Corr00");
  }

  // sun power and centroid offset

  fprintf(out, " %10s", "maxPower"); 
  fprintf(out, " %10s", "quadPower"); 
  fprintf(out, " %10s", "AzOffset"); 
  fprintf(out, " %10s", "ElOffset"); 

  // channel power ratios

  fprintf(out, " %10s", "RatioVcHc");
  if (_alternating) {
    fprintf(out, " %10s", "RatioVxHx");
    fprintf(out, " %10s", "RatioVcHx");
    fprintf(out, " %10s", "RatioVxHc");
  }
  
  // cross-polar power ratio

  if (_params.compute_cross_polar_power_ratio) {
    fprintf(out, " %10s", "XpolRatio");
    fprintf(out, " %10s", "meanS1S2");
    fprintf(out, " %10s", "zdrCorr");
  }

  if (_params.read_xpol_ratio_from_spdb) {
    fprintf(out, " %10s", "xRatioDb");
  }

  if (_params.read_site_temp_from_spdb ||
      _params.read_site_temp_from_time_series_xml) {
    fprintf(out, " %10s", "siteTemp");
  }

  if (_params.compute_test_pulse_powers) {
    fprintf(out, " %10s", "TpowerHc");
    fprintf(out, " %10s", "TpowerVc");
    fprintf(out, " %10s", "TpowerHx");
    fprintf(out, " %10s", "TpowerVx");
    fprintf(out, " %10s", "TratioVcHc");
    fprintf(out, " %10s", "TratioVxHx");
    fprintf(out, " %10s", "TratioVcHx");
    fprintf(out, " %10s", "TratioVxHc");
  }

  if (_params.compute_mean_transmit_powers) {
    fprintf(out, " %10s", "XmitPowerH");
    fprintf(out, " %10s", "XmitPowerV");
    fprintf(out, " %10s", "XmitHVDiff");
  }

  // ellipse ratios

  if (_params.compute_ellipse_hv_power_diffs) {
    fprintf(out, " %16s", "widthRatioElAzHc"); 
    fprintf(out, " %16s", "widthRatioElAzVc"); 
    fprintf(out, " %20s", "widthRatioElAzDiffHV"); 
    fprintf(out, " %12s", "zdrDiffElAz"); 
  }

  fprintf(out, "\n");

}
  

/////////////////////////////////////////
// write gridded results to text files

int SunCal::_writeGriddedTextFiles()

{
  
  // create the directory for the output files

  time_t calTime = (time_t) _calTime;
  DateTime ctime(calTime);
  char dirPath[1024];
  sprintf(dirPath, "%s/%.4d%.2d%.2d_%.2d%.2d%.2d",
          _params.text_output_dir,
          ctime.getYear(),
          ctime.getMonth(),
          ctime.getDay(),
          ctime.getHour(),
          ctime.getMin(),
          ctime.getSec());

  if (ta_makedir_recurse(dirPath)) {
    int errNum = errno;
    cerr << "ERROR - SunCal::_writeGriddedText";
    cerr << "  Cannot create output dir: " << dirPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // write out various data sets

  MomentsSun moments;
  int offset = 0;

  offset = (char *) &moments.dbm - (char *) &moments.time;
  _writeGriddedField(dirPath, "dbm", offset);
  
  offset = (char *) &moments.dbBelowPeak - (char *) &moments.time;
  _writeGriddedField(dirPath, "dbBelowPeak", offset);
  
  offset = (char *) &moments.dbmHc - (char *) &moments.time;
  _writeGriddedField(dirPath, "dbmHc", offset);
  
  offset = (char *) &moments.dbmVc - (char *) &moments.time;
  _writeGriddedField(dirPath, "dbmVc", offset);
  
  if (_alternating) {
    
    offset = (char *) &moments.dbmHx - (char *) &moments.time;
    _writeGriddedField(dirPath, "dbmHx", offset);
  
    offset = (char *) &moments.dbmVx - (char *) &moments.time;
    _writeGriddedField(dirPath, "dbmVx", offset);

    offset = (char *) &moments.corr00Hc - (char *) &moments.time;
    _writeGriddedField(dirPath, "corr00Hc", offset);
    
    offset = (char *) &moments.corr00Vc - (char *) &moments.time;
    _writeGriddedField(dirPath, "corr00Vc", offset);
    
    offset = (char *) &moments.arg00Hc - (char *) &moments.time;
    _writeGriddedField(dirPath, "arg00Hc", offset);
    
    offset = (char *) &moments.arg00Vc - (char *) &moments.time;
    _writeGriddedField(dirPath, "arg00Vc", offset);
  
  } else {

    offset = (char *) &moments.corr00 - (char *) &moments.time;
    _writeGriddedField(dirPath, "corr00", offset);
    
    offset = (char *) &moments.arg00 - (char *) &moments.time;
    _writeGriddedField(dirPath, "arg00", offset);
    
  }
  
  offset = (char *) &moments.ratioDbmVcHc - (char *) &moments.time;
  _writeGriddedField(dirPath, "ratioDbmVcHc", offset);
  
  if (_alternating) {
    
    offset = (char *) &moments.ratioDbmHcHx - (char *) &moments.time;
    _writeGriddedField(dirPath, "ratioDbmHcHx", offset);
    
    offset = (char *) &moments.ratioDbmVcVx - (char *) &moments.time;
    _writeGriddedField(dirPath, "ratioDbmVcVx", offset);
    
    offset = (char *) &moments.ratioDbmVxHc - (char *) &moments.time;
    _writeGriddedField(dirPath, "ratioDbmVxHc", offset);
    
    offset = (char *) &moments.ratioDbmVcHx - (char *) &moments.time;
    _writeGriddedField(dirPath, "ratioDbmVcHx", offset);
    
    offset = (char *) &moments.ratioDbmVxHx - (char *) &moments.time;
    _writeGriddedField(dirPath, "ratioDbmVxHx", offset);

    offset = (char *) &moments.S1S2 - (char *) &moments.time;
    _writeGriddedField(dirPath, "S1S2", offset);
  
  } else {

    offset = (char *) &moments.SS - (char *) &moments.time;
    _writeGriddedField(dirPath, "SS", offset);
  
  }
  
  return 0;

}

// write out file for data at given offset from 'time'

int SunCal::_writeGriddedField(const string &dirPath,
                               const string &field,
                               int offset)
  
{
  
  // compute file path

  char path[1024];
  sprintf(path, "%s/%s.txt", dirPath.c_str(), field.c_str());
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "writing moments file: " << path << endl;
  }

  // open file
  
  FILE *out;
  if ((out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SunCal::_writeGriddedField";
    cerr << "  Cannot create file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute the min value, set missing to this

  double minVal = 1.0e99;

  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++) {
      const MomentsSun *moments = _interpMoments[iaz][iel];
      char *ptr = (char *) &moments->time + offset;
      double val = *((double *) ptr);
      if (val != moments->missing) {
        if (val < minVal) {
          minVal = val;
        }
      }
    }
  }

  // write out grid details

  fprintf(out, "%g %g %d\n", _gridMinAz, _gridDeltaAz, _gridNAz);
  fprintf(out, "%g %g %d\n", _gridMinEl, _gridDeltaEl, _gridNEl);

  // write out grid data

  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++) {
      const MomentsSun *moments = _interpMoments[iaz][iel];
      char *ptr = (char *) &moments->time + offset;
      double val = *((double *) ptr);
      if (val == moments->missing) {
        fprintf(out, " %10.3f", minVal);
      } else {
        fprintf(out, " %10.3f", val);
      }
    } // iaz
    fprintf(out, "\n");
  } // iel

  // write out sun centroid

  fprintf(out, "%g %g\n", _sunCentroidAzOffset, _sunCentroidElOffset);

  fclose(out);

  if (_params.debug) {
    _writeGriddedFieldDebug(dirPath, field, offset);
  }

  return 0;

}

// write out file for data at given offset from 'time'

int SunCal::_writeGriddedFieldDebug(const string &dirPath,
                                    const string &field,
                                    int offset)
  
{
  
  // compute file path

  char path[1024];
  sprintf(path, "%s/%s.debug.txt", dirPath.c_str(), field.c_str());
  
  // open file
  
  FILE *out;
  if ((out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SunCal::_writeGriddedField";
    cerr << "  Cannot create file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute the min value, set missing to this
  
  double minVal = 1.0e99;
  
  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++) {
      const MomentsSun *moments = _interpMoments[iaz][iel];
      char *ptr = (char *) &moments->time + offset;
      double val = *((double *) ptr);
      if (val != moments->missing) {
        if (val < minVal) {
          minVal = val;
        }
      }
    }
  }

  // write out grid details
  
  fprintf(out, "# minEl, deltaEl, nEl: %g %g %d\n",
          _gridMinEl, _gridDeltaEl, _gridNEl);
  fprintf(out, "# minAz, deltaAz, nAz: %g %g %d\n",
          _gridMinAz, _gridDeltaAz, _gridNAz);
  fprintf(out, "# sun offset az, el (deg): %g %g\n",
          _sunCentroidAzOffset, _sunCentroidElOffset);


  // write out grid data

  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++) {
      double el = _gridMinEl + iel * _gridDeltaEl;
      double az = _gridMinAz + iaz * _gridDeltaAz;
      const MomentsSun *moments = _interpMoments[iaz][iel];
      char *ptr = (char *) &moments->time + offset;
      double val = *((double *) ptr);
      if (val == moments->missing) {
        fprintf(out, " el, az, val: %10.5f %10.5f %10.5f\n", el, az, minVal);
      } else {
        fprintf(out, " el, az, val: %10.5f %10.5f %10.5f\n", el, az, val);
      }
    } // iaz
  } // iel

  // write out sun centroid

  fclose(out);
  return 0;

}

/////////////////////////////////////////////////////////////
// write results to MDV file

int SunCal::_writeToMdv()

{

  // create output file object, initialize master header

  DsMdvx mdvx;
  _initMdvMasterHeader(mdvx, (time_t) _calTime);

  // add the fields

  MomentsSun moments;
  int offset = 0;

  offset = (char *) &moments.offsetAz - (char *) &moments.time;
  _addMdvField(mdvx, "az", "deg", "", offset);
  
  offset = (char *) &moments.offsetEl - (char *) &moments.time;
  _addMdvField(mdvx, "el", "deg", "", offset);
  
  offset = (char *) &moments.dbm - (char *) &moments.time;
  _addMdvField(mdvx, "DBM", "dBm", "dB", offset);
  
  offset = (char *) &moments.dbBelowPeak - (char *) &moments.time;
  _addMdvField(mdvx, "DB_BELOW_PEAK", "dB", "dB", offset);
  
  offset = (char *) &moments.dbmHc - (char *) &moments.time;
  _addMdvField(mdvx, "DBM_HC", "dBm", "dB", offset);
  
  offset = (char *) &moments.dbmVc - (char *) &moments.time;
  _addMdvField(mdvx, "DBM_VC", "dBm", "dB", offset);
  
  offset = (char *) &moments.zdr - (char *) &moments.time;
  _addMdvField(mdvx, "ZDR", "dB", "dB", offset);
  
  offset = (char *) &moments.phidp - (char *) &moments.time;
  _addMdvField(mdvx, "PHIDP", "deg", "", offset);
  
  offset = (char *) &moments.rhohv - (char *) &moments.time;
  _addMdvField(mdvx, "RHOHV", "", "", offset);
  
  offset = (char *) &moments.ncp - (char *) &moments.time;
  _addMdvField(mdvx, "NCP", "", "", offset);
  
  if (_alternating) {
    
    offset = (char *) &moments.dbmHx - (char *) &moments.time;
    _addMdvField(mdvx, "DBM_HX", "dBm", "dB", offset);
    
    offset = (char *) &moments.dbmVx - (char *) &moments.time;
    _addMdvField(mdvx, "DBM_VX", "dBm", "dB", offset);

    offset = (char *) &moments.corr00Hc - (char *) &moments.time;
    _addMdvField(mdvx, "CORR_00_H", "", "", offset);
    
    offset = (char *) &moments.corr00Vc - (char *) &moments.time;
    _addMdvField(mdvx, "CORR_00_V", "", "", offset);
    
    offset = (char *) &moments.arg00Hc - (char *) &moments.time;
    _addMdvField(mdvx, "ARG_00_H", "deg", "", offset);
    
    offset = (char *) &moments.arg00Vc - (char *) &moments.time;
    _addMdvField(mdvx, "ARG_00_V", "deg", "", offset);
  
  } else {

    offset = (char *) &moments.corr00 - (char *) &moments.time;
    _addMdvField(mdvx, "CORR_00", "", "", offset);
    
    offset = (char *) &moments.arg00 - (char *) &moments.time;
    _addMdvField(mdvx, "ARG_00", "", "", offset);
    
  }
  
  offset = (char *) &moments.ratioDbmVcHc - (char *) &moments.time;
  _addMdvField(mdvx, "RATIO_VC_HC", "dB", "dB", offset);
  
  if (_alternating) {
    
    offset = (char *) &moments.ratioDbmHcHx - (char *) &moments.time;
    _addMdvField(mdvx, "RATIO_HC_HX", "dB", "dB", offset);
    
    offset = (char *) &moments.ratioDbmVcVx - (char *) &moments.time;
    _addMdvField(mdvx, "RATIO_VC_VX", "dB", "dB", offset);
    
    offset = (char *) &moments.ratioDbmVxHc - (char *) &moments.time;
    _addMdvField(mdvx, "RATIO_VX_HC", "dB", "dB", offset);
    
    offset = (char *) &moments.ratioDbmVcHx - (char *) &moments.time;
    _addMdvField(mdvx, "RATIO_VC_HX", "dB", "dB", offset);
    
    offset = (char *) &moments.ratioDbmVxHx - (char *) &moments.time;
    _addMdvField(mdvx, "RATIO_VX_HX", "dB", "dB", offset);

    offset = (char *) &moments.S1S2 - (char *) &moments.time;
    _addMdvField(mdvx, "S1S2", "", "", offset);

  } else {
  
    offset = (char *) &moments.SS - (char *) &moments.time;
    _addMdvField(mdvx, "SS", "", "", offset);

  }
  
  // write the file

  if (mdvx.writeToDir(_params.mdv_output_url)) {
    cerr << "ERROR - SunCal::writeToMdv" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
  }

  return 0;

}

////////////////////////////////////////////////////
// init MDV master header

void SunCal::_initMdvMasterHeader(DsMdvx &mdvx, time_t dataTime)

{

  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  mhdr.time_begin = dataTime;
  mhdr.time_end = dataTime;
  mhdr.time_centroid = dataTime;
    
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 2;
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.max_nz = 1;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.max_nx = _gridNAz;
  mhdr.max_ny = _gridNEl;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = _radarLat;
  mhdr.sensor_lat = _radarLon;
  mhdr.sensor_alt = _radarAltKm;
  mdvx.setDataSetInfo("Sun scan results");
  mdvx.setDataSetName("Sun calibration");
  mdvx.setDataSetSource("SunCal application");

  mdvx.setMasterHeader(mhdr);
  
}

/////////////////////////////////////////////////////
// add field to MDV object

void SunCal::_addMdvField(DsMdvx &mdvx,
                          const string &fieldName,
                          const string &units,
                          const string &transform,
                          int memOffset)
  
{

  // initialize field header and vlevel header
  
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  
  // fill out field header
  
  fhdr.nx = _gridNAz;
  fhdr.ny = _gridNEl;
  fhdr.nz = 1;

  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = 1;
  fhdr.data_dimension = 2;

  fhdr.proj_origin_lat = 0.0;
  fhdr.proj_origin_lon = 0.0;

  fhdr.grid_dx = _gridDeltaAz;
  fhdr.grid_dy = _gridDeltaEl;
  fhdr.grid_dz = 1.0;
  
  fhdr.grid_minx = _gridMinAz;
  fhdr.grid_miny = _gridMinEl;
  fhdr.grid_minz = 0.0;

  fhdr.bad_data_value = -9999.0;
  fhdr.missing_data_value = -9999.0;

  // fill out vlevel header
  
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0.0;

  // load up data array

  fl32 *data = new fl32[fhdr.nx * fhdr.ny * fhdr.nz];
  int index = 0;
  for (int iel = 0; iel < _gridNEl; iel++) {
    for (int iaz = 0; iaz < _gridNAz; iaz++) {
      const MomentsSun *moments = _interpMoments[iaz][iel];
      char *ptr = (char *) &moments->time + memOffset;
      double val = *((double *) ptr);
      if (val == moments->missing) {
        data[index] = -9999.0;
      } else {
        data[index] = val;
      }
      index++;
    } // iaz
  } // iel

  // create field
  
  MdvxField *field = new MdvxField(fhdr, vhdr, data);

  // set the names

  field->setFieldName(fieldName.c_str());
  field->setFieldNameLong(fieldName.c_str());
  field->setUnits(units.c_str());
  field->setTransform(transform.c_str());

  // free up data array

  delete[] data;

  // add field to mdvx object

  mdvx.addField(field);

}

/////////////////////////////////////////////////////////////
// write summary results to SPDB in XML

int SunCal::_writeSummaryToSpdb()

{

  string xml;

  xml += TaXml::writeStartTag("RadarSunCal", 0);

  xml += TaXml::writeString("radarName", 1, _params.radar_name);
  xml += TaXml::writeString("radarSite", 1, _params.radar_site);
  xml += TaXml::writeDouble("radarLatitude", 1, _radarLat);
  xml += TaXml::writeDouble("radarLongitude", 1, _radarLon);
  xml += TaXml::writeDouble("radarAltitudeKm", 1, _radarAltKm);

  xml += TaXml::writeTime("CalTime", 1, (time_t) _calTime);
  xml += TaXml::writeInt("VolumeNumber", 1, _volNum);

  xml += TaXml::writeBoolean("validCentroid", 1, _validCentroid);
  xml += TaXml::writeDouble("meanSunEl", 1, _meanSunEl);
  xml += TaXml::writeDouble("meanSunAz", 1, _meanSunAz);

  xml += TaXml::writeDouble("nBeamsNoise", 1, _nBeamsNoise);
  xml += TaXml::writeDouble("noiseDbmHc", 1, _noiseDbmHc);
  xml += TaXml::writeDouble("noiseDbmHx", 1, _noiseDbmHx);
  xml += TaXml::writeDouble("noiseDbmVc", 1, _noiseDbmVc);
  xml += TaXml::writeDouble("noiseDbmVx", 1, _noiseDbmVx);
  xml += TaXml::writeDouble("maxPowerDbm", 1, _maxPowerDbm);
  xml += TaXml::writeDouble("quadPowerDbm", 1, _quadPowerDbm);
  xml += TaXml::writeDouble("maxPowerDbmHc", 1, _maxPowerDbmHc);
  xml += TaXml::writeDouble("quadPowerDbmHc", 1, _quadPowerDbmHc);
  xml += TaXml::writeDouble("maxPowerDbmVc", 1, _maxPowerDbmVc);
  xml += TaXml::writeDouble("quadPowerDbmVc", 1, _quadPowerDbmVc);
  xml += TaXml::writeDouble("centroidAzOffset", 1, _sunCentroidAzOffset);
  xml += TaXml::writeDouble("centroidElOffset", 1, _sunCentroidElOffset);
  xml += TaXml::writeDouble("centroidAzOffsetHc", 1, _sunCentroidAzOffsetHc);
  xml += TaXml::writeDouble("centroidElOffsetHc", 1, _sunCentroidElOffsetHc);
  xml += TaXml::writeDouble("centroidAzOffsetVc", 1, _sunCentroidAzOffsetVc);
  xml += TaXml::writeDouble("centroidElOffsetVc", 1, _sunCentroidElOffsetVc);
  xml += TaXml::writeInt("nXpolPoints", 1, _nXpolPoints);
  xml += TaXml::writeDouble("meanXpolRatioDb", 1, _meanXpolRatioDb);

  xml += TaXml::writeDouble("ratioDbmVcHc", 1, _statsForZdrBias.meanRatioDbmVcHc);
  xml += TaXml::writeDouble("ratioDbmVxHx", 1, _statsForZdrBias.meanRatioDbmVxHx);
  xml += TaXml::writeDouble("ratioDbmVcHx", 1, _statsForZdrBias.meanRatioDbmVcHx);
  xml += TaXml::writeDouble("ratioDbmVxHc", 1, _statsForZdrBias.meanRatioDbmVxHc);
  xml += TaXml::writeDouble("S1S2", 1, _statsForZdrBias.meanS1S2);
  xml += TaXml::writeDouble("sdevS1S2", 1, _statsForZdrBias.sdevS1S2);
  xml += TaXml::writeDouble("SS", 1, _statsForZdrBias.meanSS);
  xml += TaXml::writeDouble("sdevSS", 1, _statsForZdrBias.sdevSS);
  xml += TaXml::writeDouble("corr00Hc", 1, _statsForZdrBias.meanCorr00Hc);
  xml += TaXml::writeDouble("corr00Vc", 1, _statsForZdrBias.meanCorr00Vc);
  xml += TaXml::writeDouble("corr00", 1, _statsForZdrBias.meanCorr00);

  xml += TaXml::writeDouble("zdrCorr", 1, _zdrCorr);
  xml += TaXml::writeDouble("meanXmitPowerHDbm", 1, _meanXmitPowerHDbm);
  xml += TaXml::writeDouble("meanXmitPowerVDbm", 1, _meanXmitPowerVDbm);

  xml += TaXml::writeDouble("widthRatioElAzHc", 1, _widthRatioElAzHc);
  xml += TaXml::writeDouble("widthRatioElAzVc", 1, _widthRatioElAzVc);
  xml += TaXml::writeDouble("widthRatioElAzDiffHV", 1, _widthRatioElAzDiffHV);
  xml += TaXml::writeDouble("zdrDiffElAz", 1, _zdrDiffElAz);

  xml += TaXml::writeDouble("testPulseDbmHc", 1, _testPulseDbmHc);
  xml += TaXml::writeDouble("testPulseDbmHx", 1, _testPulseDbmHx);
  xml += TaXml::writeDouble("testPulseDbmVc", 1, _testPulseDbmVc);
  xml += TaXml::writeDouble("testPulseDbmVx", 1, _testPulseDbmVx);

  if (_params.read_xpol_ratio_from_spdb) {
    xml += TaXml::writeDouble("xpolRatioDbFromSpdb", 1, _xpolRatioDbFromSpdb);
  }
  if (_params.read_site_temp_from_spdb ||
      _params.read_site_temp_from_time_series_xml) {
    xml += TaXml::writeDouble("siteTempC", 1, _siteTempC);
    xml += TaXml::writeTime("timeForSiteTemp", 1, _timeForSiteTemp);
  }

  xml += TaXml::writeEndTag("RadarSunCal", 0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing XML results to SPDB:" << endl;
    cerr << xml << endl;
  }

  DsSpdb spdb;
  time_t calTime = (time_t) _calTime;
  spdb.addPutChunk(0, calTime, calTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - SunCal::_writeSummaryToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote to spdb, url: " << _params.spdb_output_url << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////////////
// write NEXRAD results to MDV file

int SunCal::_writeNexradToMdv()

{
  
  // create output file object, initialize master header
  
  DsMdvx mdvx;
  _initNexradMdvMasterHeader(mdvx,
                             (time_t) nexradSolarGetMeanSunTime());

  // add the fields

  solar_beam_t moments;
  int offset = 0;

  offset = (char *) &moments.azOffset - (char *) &moments.time;
  _addNexradMdvField(mdvx, "az", "deg", "", offset);
  
  offset = (char *) &moments.elOffset - (char *) &moments.time;
  _addNexradMdvField(mdvx, "el", "deg", "", offset);
  
  offset = (char *) &moments.dbm - (char *) &moments.time;
  _addNexradMdvField(mdvx, "DBM", "dBm", "dB", offset);
  
  offset = (char *) &moments.dbBelowPeak - (char *) &moments.time;
  _addNexradMdvField(mdvx, "DB_BELOW_PEAK", "dB", "dB", offset);
  
  offset = (char *) &moments.dbmH - (char *) &moments.time;
  _addNexradMdvField(mdvx, "DBM_H", "dBm", "dB", offset);
  
  offset = (char *) &moments.dbmV - (char *) &moments.time;
  _addNexradMdvField(mdvx, "DBM_V", "dBm", "dB", offset);
  
  offset = (char *) &moments.zdr - (char *) &moments.time;
  _addNexradMdvField(mdvx, "ZDR", "dB", "dB", offset);
  
  offset = (char *) &moments.corrHV - (char *) &moments.time;
  _addNexradMdvField(mdvx, "CORR_HV", "", "", offset);
  
  offset = (char *) &moments.phaseHV - (char *) &moments.time;
  _addNexradMdvField(mdvx, "PHASE_HV", "", "", offset);
    
  offset = (char *) &moments.ratioDbmVH - (char *) &moments.time;
  _addNexradMdvField(mdvx, "RATIO_V_H", "dB", "dB", offset);
  
  offset = (char *) &moments.SS - (char *) &moments.time;
  _addNexradMdvField(mdvx, "SS", "", "", offset);

  // write the file

  if (mdvx.writeToDir(_params.nexrad_mdv_output_url)) {
    cerr << "ERROR - SunCal::writeNexradToMdv" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
  }

  return 0;

}

////////////////////////////////////////////////////
// init NEXRAD MDV master header

void SunCal::_initNexradMdvMasterHeader(DsMdvx &mdvx, time_t dataTime)

{
  
  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  mhdr.time_begin = dataTime;
  mhdr.time_end = dataTime;
  mhdr.time_centroid = dataTime;

  mhdr.num_data_times = 1;
  mhdr.data_dimension = 2;
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.max_nz = 1;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.max_nx = nexradSolarGetGridNAz();
  mhdr.max_ny = nexradSolarGetGridNEl();
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = _radarLat;
  mhdr.sensor_lat = _radarLon;
  mhdr.sensor_alt = _radarAltKm;
  mdvx.setDataSetInfo("Sun scan results");
  mdvx.setDataSetName("Sun calibration");
  mdvx.setDataSetSource("SunCal application");

  mdvx.setMasterHeader(mhdr);
  
}

/////////////////////////////////////////////////////
// add NEXRAD field to MDV object

void SunCal::_addNexradMdvField(DsMdvx &mdvx,
                                const string &fieldName,
                                const string &units,
                                const string &transform,
                                int memOffset)
  
{

  // initialize field header and vlevel header
  
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  
  // fill out field header
  
  fhdr.nx = nexradSolarGetGridNAz();
  fhdr.ny = nexradSolarGetGridNEl();
  fhdr.nz = 1;

  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = 1;
  fhdr.data_dimension = 2;

  fhdr.proj_origin_lat = 0.0;
  fhdr.proj_origin_lon = 0.0;

  fhdr.grid_dx = nexradSolarGetGridDeltaAz();
  fhdr.grid_dy = nexradSolarGetGridDeltaEl();
  fhdr.grid_dz = 1.0;
  
  fhdr.grid_minx = nexradSolarGetGridStartAz();
  fhdr.grid_miny = nexradSolarGetGridStartEl();
  fhdr.grid_minz = 0.0;

  fhdr.bad_data_value = -9999.0;
  fhdr.missing_data_value = -9999.0;

  // fill out vlevel header
  
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0.0;

  // load up data array

  fl32 *data = new fl32[fhdr.nx * fhdr.ny * fhdr.nz];
  double miss = nexradSolarGetMissingVal();
  int index = 0;
  for (int iel = 0; iel < nexradSolarGetGridNEl(); iel++) {
    for (int iaz = 0; iaz < nexradSolarGetGridNAz(); iaz++) {
      const solar_beam_t moments =
        nexradSolarGetInterpBeamArray()[iaz][iel];
      char *ptr = (char *) &moments.time + memOffset;
      double val = *((double *) ptr);
      if (val == miss) {
        data[index] = -9999.0;
      } else {
        data[index] = val;
      }
      index++;
    } // iaz
  } // iel

  // create field
  
  MdvxField *field = new MdvxField(fhdr, vhdr, data);

  // set the names

  field->setFieldName(fieldName.c_str());
  field->setFieldNameLong(fieldName.c_str());
  field->setUnits(units.c_str());
  field->setTransform(transform.c_str());

  // free up data array

  delete[] data;

  // add field to mdvx object

  mdvx.addField(field);

}

/////////////////////////////////////////////////////////////
// write NEXRAD summary results to SPDB in XML

int SunCal::_writeNexradSummaryToSpdb()
  
{

  string xml;

  xml += TaXml::writeStartTag("RadarSunCal", 0);

  xml += TaXml::writeString("radarName", 1, _params.radar_name);
  xml += TaXml::writeString("radarSite", 1, _params.radar_site);
  xml += TaXml::writeDouble("radarLatitude", 1, _radarLat);
  xml += TaXml::writeDouble("radarLongitude", 1, _radarLon);
  xml += TaXml::writeDouble("radarAltitudeKm", 1, _radarAltKm);

  xml += TaXml::writeTime("CalTime", 1, (time_t) nexradSolarGetMeanSunTime());
  xml += TaXml::writeInt("VolumeNumber", 1, 0);
  
  xml += TaXml::writeBoolean("validCentroid", 1, true);
  xml += TaXml::writeDouble("meanSunEl", 1, nexradSolarGetMeanSunEl());
  xml += TaXml::writeDouble("meanSunAz", 1, nexradSolarGetMeanSunAz());

  xml += TaXml::writeDouble("maxPowerDbm", 1, nexradSolarGetMaxPowerDbm());
  xml += TaXml::writeDouble("quadPowerDbm", 1, nexradSolarGetQuadPowerDbm());
  xml += TaXml::writeDouble("centroidAzOffset", 1, nexradSolarGetQuadFitCentroidAzError());
  xml += TaXml::writeDouble("centroidElOffset", 1, nexradSolarGetQuadFitCentroidElError());
  xml += TaXml::writeDouble("SS", 1, nexradSolarGetMeanSS());

  xml += TaXml::writeDouble("widthRatioElAz", 1, nexradSolarGetElAzWidthRatio());

  xml += TaXml::writeEndTag("RadarSunCal", 0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing XML results to SPDB:" << endl;
    cerr << xml << endl;
  }

  DsSpdb spdb;
  time_t calTime = (time_t) _calTime;
  spdb.addPutChunk(0, calTime, calTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.nexrad_spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - SunCal::_writeSummaryToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote to spdb, url: " << _params.nexrad_spdb_output_url << endl;
  }

  return 0;

}

