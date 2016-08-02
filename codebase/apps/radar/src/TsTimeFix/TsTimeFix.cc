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
/////////////////////////////////////////////////////////////////////
// TsTimeFix.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2014
//
/////////////////////////////////////////////////////////////////////
//
// TsTimeFix reads raw time-series data, adjusts the time of selected
// components, and writes the results out to a specified directory.
//
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <cmath>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
#include <dsserver/DsLdataInfo.hh>
#include "TsTimeFix.hh"

using namespace std;

// Constructor

TsTimeFix::TsTimeFix(int argc, char **argv)
  
{

  isOK = true;
  _inputPulseCount = 0;
  _outputPulseCount = 0;
  _masterReader = NULL;
  _georefReader = NULL;
  _georefsFound = false;
  _prevPulseSeqNum = -1;
  _prevGeorefSeqNum = -1;

  // set programe name
 
  _progName = "TsTimeFix";

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
  
  // initialize the time series data reader objects
  
  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_params.debug >= Params::DEBUG_EXTRA) {
    iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_NORM) {
    iwrfDebug = IWRF_DEBUG_NORM;
  }
  
  if (_params.input_mode == Params::INPUT_MODE_FILES) {
    if (_args.inputFileList.size() < 1) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  No files specified on command line" << endl;
      cerr << "  You must use the -f option to specify the file list." << endl;
      isOK = false;
      return;
    }
    _masterReader = new IwrfTsReaderFile(_args.inputFileList, iwrfDebug);
    _georefReader = new IwrfTsReaderFile(_args.inputFileList, iwrfDebug);
  } else if (_params.input_mode == Params::INPUT_MODE_FMQ) {
    _masterReader = new IwrfTsReaderFmq(_params.input_fmq_path, iwrfDebug,
                                        _params.position_fmq_at_start);
    _georefReader = new IwrfTsReaderFmq(_params.input_fmq_path, iwrfDebug,
                                        _params.position_fmq_at_start);
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

TsTimeFix::~TsTimeFix()

{

  if (_masterReader) {
    delete _masterReader;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TsTimeFix::Run()
{
  
  int iret = 0;
  PMU_auto_register("Run");
  
  // get pulses from master queue
    
  IwrfTsPulse *pulse;
  while ((pulse = _masterReader->getNextPulse(false)) != NULL) {
    
    PMU_auto_register("Procssing pulse");
    if (_processPulse(pulse)) {
      iret = -1;
    }
    delete pulse;
  } // while

  // close current output file

  _closeFile();

  return iret;

}

//////////////////////////////////////////////////
// process pulse

int TsTimeFix::_processPulse(IwrfTsPulse *pulse)
  
{

  int iret = 0;
  _inputPulseCount++;
  IwrfTsInfo &info = pulse->getTsInfo();

  if (!info.isPlatformGeorefActive()) {
    return 0;
  }

  // save the georef struct, because the info.georef gets modified below

  if (info.getPlatformGeorefPktSeqNum() > _prevGeorefSeqNum &&
      info.getPlatformGeorefPktSeqNum() < pulse->getHdr().packet.seq_num) {
    _pulseGeoref = info.getPlatformGeoref();
    _prevGeorefSeqNum = info.getPlatformGeorefPktSeqNum();
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    info.setDebug(IWRF_DEBUG_EXTRA);
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    info.setDebug(IWRF_DEBUG_VERBOSE);
  } else if (_params.debug) {
    info.setDebug(IWRF_DEBUG_NORM);
  }
  // info.setDebug(IWRF_DEBUG_EXTRA);
  RadxTime origTime(pulse->getTime(), pulse->getNanoSecs() / 1.0e9);

  // debug print
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=========================================================" << endl;
    cerr << "Processing pulse, orig time: " << origTime.asString(3) << endl;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    pulse->printHeader(stderr);
  }

  // adjust times on pulse-based data

  bool georefFound = false;
  if (_params.time_offset_secs != 0) {
    _adjustTime(pulse, georefFound);
  }
  
  RadxTime pulseTime2(pulse->getTime(),
                      pulse->getNanoSecs() / 1.0e9);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    RadxTime updatedTime(pulse->getTime(), pulse->getNanoSecs() / 1.0e9);
    cerr << "               updated time: " 
         << updatedTime.asString(3) << endl;
  }

  // adjust georef time

  if (_params.preserve_georef_time) {
    // find the relevant georef object
    if (_getMatchingGeoref(pulse) == 0) {
      iwrf_platform_georef_t modGeoref = _prevGeoref;
      RadxTime modGeorefTime(modGeoref.packet.time_secs_utc,
                             modGeoref.packet.time_nano_secs / 1.0e9);
      
      // copy over the drive angle and antenna angles

      modGeoref.drive_angle_1_deg = _pulseGeoref.drive_angle_1_deg;
      modGeoref.drive_angle_2_deg = _pulseGeoref.drive_angle_2_deg;
      modGeoref.rotation_angle_deg = _pulseGeoref.rotation_angle_deg;
      modGeoref.tilt_deg = _pulseGeoref.tilt_deg;
      
      pulse->setPlatformGeoref(modGeoref);
      info.setPlatformGeoref(modGeoref, false);
      if (_prevGeorefTime != _usedGeorefTime) {
        info.setPlatformGeoref(modGeoref, true);
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "                georef time: "
               << _prevGeorefTime.asString(3) << endl;
        }
        _usedGeorefTime = _prevGeorefTime;
      }
    } else {
      return 0;
    }
  }

  // apply georef?

  if (_params.apply_georef_to_compute_angles) {
    pulse->computeElAzFromGeoref();
  }

  // open file as needed

  if (_out == NULL || _masterReader->endOfFile()) {
    // open new file
    if (_openNewFile(pulse)) {
      return -1;
    }
    // write metadata at start of file
    info.writeMetaToFile(_out, 0);
  }
  
  // write ops info to file, if info has changed since last write
  
  info.writeMetaQueueToFile(_out, true);

  // write burst if applicable

  if (_masterReader->isBurstForLatestPulse()) {
    _masterReader->getBurst().writeToFile(_out);
  }

  // write pulse to file
  
  pulse->writeToFile(_out);
  _outputPulseCount++;

  _prevPulseSeqNum = pulse->get_pulse_seq_num();
  return iret;

}

//////////////////////////////////////////////////
// adjust time on pulse data

void TsTimeFix::_adjustTime(IwrfTsPulse *pulse, bool &georefFound)
  
{

  // set pulse time

  RadxTime pulseTime(pulse->getTime(), pulse->getNanoSecs() / 1.0e9);
  pulseTime += _params.time_offset_secs;
  pulse->setTime(pulseTime.utime(), (int) (pulseTime.getSubSec() * 1.0e9 + 0.5));

  if (!_masterReader->isOpsInfoNew()) {
    return;
  }

  IwrfTsInfo &info = _masterReader->getOpsInfo();

  if (info.isRadarInfoActive() &&
      info.getRadarInfoPktSeqNum() > _prevPulseSeqNum) {
    RadxTime rTime(info.getRadarInfo().packet.time_secs_utc,
                   info.getRadarInfo().packet.time_nano_secs / 1.0e9);
    rTime += _params.time_offset_secs;
    info.setRadarInfoTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }
    
  if (info.isScanSegmentActive() &&
      info.getScanSegmentPktSeqNum() > _prevPulseSeqNum) {
    RadxTime rTime(info.getScanSegment().packet.time_secs_utc,
                   info.getScanSegment().packet.time_nano_secs / 1.0e9);
    rTime += _params.time_offset_secs;
    info.setScanSegmentTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }
    
  if (info.isAntennaCorrectionActive() &&
      info.getAntennaCorrectionPktSeqNum() > _prevPulseSeqNum) {
    RadxTime rTime(info.getAntennaCorrection().packet.time_secs_utc,
                   info.getAntennaCorrection().packet.time_nano_secs / 1.0e9);
    rTime += _params.time_offset_secs;
    info.setAntennaCorrectionTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }
    
  if (info.isTsProcessingActive() &&
      info.getTsProcessingPktSeqNum() > _prevPulseSeqNum) {
    RadxTime rTime(info.getTsProcessing().packet.time_secs_utc,
                   info.getTsProcessing().packet.time_nano_secs / 1.0e9);
    rTime += _params.time_offset_secs;
    info.setTsProcessingTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }
    
  if (info.isXmitPowerActive() &&
      info.getXmitPowerPktSeqNum() > _prevPulseSeqNum) {
    RadxTime rTime(info.getXmitPower().packet.time_secs_utc,
                   info.getXmitPower().packet.time_nano_secs / 1.0e9);
    rTime += _params.time_offset_secs;
    info.setXmitPowerTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }
    
  if (info.isXmitSampleActive() &&
      info.getXmitSamplePktSeqNum() > _prevPulseSeqNum) {
    RadxTime rTime(info.getXmitSample().packet.time_secs_utc,
                   info.getXmitSample().packet.time_nano_secs / 1.0e9);
    rTime += _params.time_offset_secs;
    info.setXmitSampleTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }
    
  if (info.isStatusXmlActive() &&
      info.getStatusXmlPktSeqNum() > _prevPulseSeqNum) {
    RadxTime rTime(info.getStatusXmlHdr().packet.time_secs_utc,
                   info.getStatusXmlHdr().packet.time_nano_secs / 1.0e9);
    rTime += _params.time_offset_secs;
    info.setStatusXmlTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }
    
  if (info.isCalibrationActive() &&
      info.getCalibrationPktSeqNum() > _prevPulseSeqNum) {
    RadxTime rTime(info.getCalibration().packet.time_secs_utc,
                   info.getCalibration().packet.time_nano_secs / 1.0e9);
    rTime += _params.time_offset_secs;
    info.setCalibrationTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }
    
  if (info.isEventNoticeActive() &&
      info.getEventNoticePktSeqNum() > _prevPulseSeqNum) {
    RadxTime rTime(info.getEventNotice().packet.time_secs_utc,
                   info.getEventNotice().packet.time_nano_secs / 1.0e9);
    rTime += _params.time_offset_secs;
    info.setEventNoticeTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }
    
  if (info.isPhasecodeActive() &&
      info.getPhasecodePktSeqNum() > _prevPulseSeqNum) {
    RadxTime rTime(info.getPhasecode().packet.time_secs_utc,
                   info.getPhasecode().packet.time_nano_secs / 1.0e9);
    rTime += _params.time_offset_secs;
    info.setPhasecodeTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }
    
  if (info.isXmitInfoActive() &&
      info.getXmitInfoPktSeqNum() > _prevPulseSeqNum) {
    RadxTime rTime(info.getXmitInfo().packet.time_secs_utc,
                   info.getXmitInfo().packet.time_nano_secs / 1.0e9);
    rTime += _params.time_offset_secs;
    info.setXmitInfoTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }
    
  if (info.isRvp8InfoActive() &&
      info.getRvp8InfoPktSeqNum() > _prevPulseSeqNum) {
    RadxTime rTime(info.getRvp8Info().packet.time_secs_utc,
                   info.getRvp8Info().packet.time_nano_secs / 1.0e9);
    rTime += _params.time_offset_secs;
    info.setRvp8InfoTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }

  if (_masterReader->isBurstForLatestPulse()) {
    IwrfTsBurst &burst = _masterReader->getBurst();
    RadxTime rTime(burst.getTime(), burst.getNanoSecs() / 1.0e9);
    rTime += _params.time_offset_secs;
    burst.setTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
  }

  georefFound = false;
  if (info.isPlatformGeorefActive() &&
      info.getPlatformGeorefPktSeqNum() > _prevPulseSeqNum) {
    georefFound = true;
    if (!_params.preserve_georef_time) {
      RadxTime rTime(info.getPlatformGeoref().packet.time_secs_utc,
                     info.getPlatformGeoref().packet.time_nano_secs / 1.0e9);
      rTime += _params.time_offset_secs;
      info.setPlatformGeorefTime(rTime.utime(), (int) (rTime.getSubSec() * 1.0e9 + 0.5));
    }
  }
    
}

//////////////////////////////////////////////////
// get georef that matches in time

int TsTimeFix::_getMatchingGeoref(const IwrfTsPulse *pulse)
  
{
  
  // get pulse time

  RadxTime pTime(pulse->getTime(), pulse->getNanoSecs() / 1.0e9);

  // do we already have the georefs that straddle the pulse time?

  if (_georefsFound) {
    if (pTime >= _prevGeorefTime && pTime <= _latestGeorefTime) {
      if (fabs(pTime - _prevGeorefTime) < _params.georef_time_margin_secs) {
        return 0;
      }
    }
    // do we need pulses to catch up to georefs?
    if (pTime < _prevGeorefTime && pTime < _latestGeorefTime) {
      return -1;
    }
  }

  // get pulses from georef queue

  IwrfTsPulse *georefPulse = NULL;
  while ((georefPulse = _georefReader->getNextPulse(false)) != NULL) {
    
    IwrfTsInfo &info = _georefReader->getOpsInfo();
    if (info.isPlatformGeorefActive()) {
      
      // copy latest to previous
      
      _prevGeoref = _latestGeoref;
      _prevGeorefTime = _latestGeorefTime;

      // get latest
      
      _latestGeoref = info.getPlatformGeoref();
      _latestGeorefTime.set(info.getPlatformGeoref().packet.time_secs_utc,
                            info.getPlatformGeoref().packet.time_nano_secs / 1.0e9);
      // _latestGeoref.packet.seq_num = pulse->getPktSeqNum();
      
      if (_georefsFound) {

        // is the match within bounds

        if (pTime >= _prevGeorefTime && pTime <= _latestGeorefTime) {
          if (fabs(pTime - _prevGeorefTime) < _params.georef_time_margin_secs) {
            delete georefPulse;
            return 0;
          }
        }

        // have the georefs gone past the pulse time?

        if (pTime < _prevGeorefTime && pTime < _latestGeorefTime) {
          delete georefPulse;
          return -1;
        }

      } // if (_georefsFound) 

      _georefsFound = true;

    }

    // free pulse

    delete georefPulse;

  } // while

  if (georefPulse) {
    delete georefPulse;
  }

  return -1;
  

}

/////////////////////////////////
// open a new file

int TsTimeFix::_openNewFile(const IwrfTsPulse *pulse)

{

  // close out old file
  
  _closeFile();

  // get time

  time_t ptime = pulse->getTime();
  int nanoSecs = pulse->getNanoSecs();
  int milliSecs = nanoSecs / 1000000;
  if (milliSecs > 999) {
    milliSecs = 999;
  }

  date_time_t ttime;
  ttime.unix_time = ptime;
  _outputTime = ptime;
  uconvert_from_utime(&ttime);

  // compute antenna pos strings
  
  iwrf_scan_mode_t scanMode = pulse->getScanMode();

  char fixedAngleStr[64];
  char movingAngleStr[64];
  
  if (scanMode == IWRF_SCAN_MODE_RHI ||
      scanMode == IWRF_SCAN_MODE_MANRHI) {
    double el = pulse->getEl();
    if (el < 0) {
      el += 360;
    }
    double az = pulse->getAz();
    if (_params.use_fixed_angle_for_file_name) {
      az = pulse->getFixedAz();
      if (az < -9990) {
        az = 999; // missing
      }
    }
    sprintf(fixedAngleStr, "_%.3d", (int) (az * 10.0 + 0.5));
    sprintf(movingAngleStr, "_%.3d", (int) (el + 0.5));
  } else {
    double az = pulse->getAz();
    if (az < 0) {
      az += 360;
    }
    double el = pulse->getEl();
    if (_params.use_fixed_angle_for_file_name) {
      el = pulse->getFixedEl();
      if (el < -9990) {
        el = 99.9; // missing
      }
    }
    sprintf(fixedAngleStr, "_%.3d", (int) (el * 10.0 + 0.5));
    sprintf(movingAngleStr, "_%.3d", (int) (az + 0.5));
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

  if (pulse->antennaTransition()) {
    scanModeStr += "_trans";
  }
    
  // make the output dir

  char subdir[1024];
  _outputDir = _params.output_dir;
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

  string format = ".iwrf_ts";

  iwrf_iq_encoding_t encoding = pulse->getPackedEncoding();

  string packing;
  if (encoding == IWRF_IQ_ENCODING_FL32) {
    packing = ".fl32";
  } else if (encoding == IWRF_IQ_ENCODING_SCALED_SI16) {
    packing = ".scaled";
  } else if (encoding == IWRF_IQ_ENCODING_SCALED_SI32) {
    packing = ".scaled32";
  } else if (encoding == IWRF_IQ_ENCODING_DBM_PHASE_SI16) {
    packing = ".dbm_phase";
  } else if (encoding == IWRF_IQ_ENCODING_SIGMET_FL16) {
    packing = ".sigmet";
  }

  char name[1024];
  sprintf(name, "%.4d%.2d%.2d_%.2d%.2d%.2d.%.3d%s%s%s%s%s",
          ttime.year, ttime.month, ttime.day,
          ttime.hour, ttime.min, ttime.sec, milliSecs,
	  fixedAngleStr, movingAngleStr, scanModeStr.c_str(),
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
    cerr << "====>>> opened new output file: " << path << endl;
  }

  return 0;

}

///////////////////////////////////////////
// close file
//
// Returns 0 if file already open, -1 if not

int TsTimeFix::_closeFile()

{

  // close out old file
  
  if (_out != NULL) {

    fclose(_out);
    _out = NULL;

    if (_params.debug) {
      cerr << "====>>> closed output file: " << _relPath << endl;
      cerr << "  inputPulseCount: " << _inputPulseCount << endl;
      cerr << "  outputPulseCount: " << _outputPulseCount << endl;
    }
    _inputPulseCount = 0;
    _outputPulseCount = 0;

    // write latest data info file
    
    DsLdataInfo ldata(_outputDir,
		      _params.debug >= Params::DEBUG_VERBOSE);
    ldata.setLatestTime(_outputTime);
    ldata.setRelDataPath(_relPath);
    ldata.setWriter("TsTimeFix");
    ldata.setDataType("iwrf_ts");
    if (ldata.write(_outputTime)) {
      cerr << "ERROR - cannot write LdataInfo" << endl;
      cerr << " outputDir: " << _outputDir << endl;
    }
   
    return 0;

  }

  return -1;

}
  
