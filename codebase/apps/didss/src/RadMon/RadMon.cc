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
////////////////////////////////////////////////////////////////////////
// RadMon.cc
//
// RadMon object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2001
//
///////////////////////////////////////////////////////////////////////
//
// RadMon prints out info from a DsRadar FMQ
//
///////////////////////////////////////////////////////////////////////

#include <Fmq/DsRadarQueue.hh>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <rapmath/RapComplex.hh>

#include "RadMon.hh"
using namespace std;

// Constructor

RadMon::RadMon(int argc, char **argv)

{

  isOK = true;
  _lastPrintTime = 0;
  _printCount = 0;
  _numGetDsBeamFails = 0;
  _prevScanMode = -1;
  _prevVolNum = -1;
  _prevTiltNum = -1;
  _prevAz = 0.0;
  _prevEl = 0.0;

  // set programe name

  _progName = "RadMon";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

RadMon::~RadMon()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadMon::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    _run();
    sleep(10);
    cerr << "RadMon::Run:" << endl;
    cerr << "  Trying to contact input server at url: "
	 << _params.fmq_url << endl;
  }

  return (0);

}

//////////////////////////////////////////////////
// _run

int RadMon::_run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Instantiate and initialize the DsRadar queue and message
  
  DsRadarQueue radarQueue;
  DsRadarMsg radarMsg;
  DsFmq::openPosition open_pos;
  
  if (_params.seek_to_start_of_input) {
    open_pos = DsFmq::START;
  } else {
    open_pos = DsFmq::END;
  }

  int iret = radarQueue.init(_params.fmq_url, _progName.c_str(),
			     _params.verbose,
			     DsFmq::READ_ONLY, open_pos);

  if (iret) {
    cerr << "ERROR - RadMon::_run()" << endl;
    cerr << "  Could not initialize radar queue: "
	 << _params.fmq_url << endl;
    return -1;
  }

  int contents;
  while (true) { 

    PMU_auto_register("Reading radar queue");
    
    // get a message from the radar queue

    int radarQueueRetVal = radarQueue.getDsMsg( radarMsg, &contents );

    if (radarQueueRetVal) {
      
      cerr << "radarQueue:getDsBeam() failed, returned "
	   << radarQueueRetVal << endl;
      //
      // Keep count of consecuive failures.
      //
      _numGetDsBeamFails++;
      //
      // If we have maxed out, it is safe to assume that the program is
      // constantly failing. Exiting and restarting may solve this,
      // assuming the restarter is running.
      //
      if (_numGetDsBeamFails == _maxNumGetDsBeamFails){
	cerr << "The program is failing consistently, exiting ..." << endl;
	exit(-1);
      }
      sleep (1);

    } else { 

      //
      // getDsBeam succeded, reset the count of consecutive failures.
      //
      _numGetDsBeamFails = 0;

      // print beam or flag info

      if (contents & DsRadarMsg::RADAR_FLAGS) {
	_printFlags(radarMsg);
      } else {
	_printBeam(contents, radarMsg);
        if (_params.check_for_missing_beams) {
          _checkForMissingBeams(contents, radarMsg);
        }
      }

      
    } // if (radarQueue ...

  } // while

  return 0;

}

////////////////////////////////////////////
// print beam information

void RadMon::_printBeam(int contents,
			const DsRadarMsg &radarMsg)
  
{
  
  date_time_t data_time;

  const DsRadarParams &radarParams = radarMsg.getRadarParams();
  const DsRadarCalib &radarCalib = radarMsg.getRadarCalib();
  const string &statusXml = radarMsg.getStatusXml();
  const DsRadarBeam &radarBeam = radarMsg.getRadarBeam();
  const vector<DsFieldParams*> &fieldParams = radarMsg.getFieldParams();
  
  // If the update_interval is negative, print only when params are set
  // If update_interval is positive, print based on beam timestamps

  bool doPrint = false;
  if (_params.update_interval == 0) {
    doPrint = true;
  } else if (_params.update_interval < 0) {
    if (_printCount == _params.update_interval) {
      doPrint = true;
    }
  } else {
    if (radarBeam.dataTime - _lastPrintTime >= _params.update_interval) {
      doPrint = true;
    }
  }
  
  if (_params.print_type == Params::FULL ||
      _params.print_type == Params::DATA) {
    if (contents & DsRadarMsg::RADAR_PARAMS ||
	contents & DsRadarMsg::RADAR_CALIB ||
	contents & DsRadarMsg::STATUS_XML ||
	contents & DsRadarMsg::FIELD_PARAMS) {
      doPrint = true;
    }
  }
  
  if (doPrint) {
    
    _printCount = -1;
    switch (_params.print_type) {
      
      case Params::MONITOR: {
        
        data_time.unix_time = radarBeam.dataTime;
        uconvert_from_utime(&data_time);
        for (int i = 0; i < _params.n_monitor_line_feeds; i++) {
          fprintf(stdout, "\n");
        }
        if (_params.labels_in_monitor_mode) {
          if (_params.volume_num_in_monitor_mode) {
            fprintf(stdout, "\n%10s %8s %4s %5s %5s\n",
                    "Date", "Time", "Vnum", "az", "elev");
          } else {
            fprintf(stdout, "\n%10s %8s %5s %5s\n",
                    "Date", "Time", "az", "elev");
          }
        }
        if (_params.volume_num_in_monitor_mode) {
          fprintf(stdout, "\r%4d/%.2d/%.2d %.2d:%.2d:%.2d %4d %5.1f %5.1f",
                  data_time.year, data_time.month, data_time.day,
                  data_time.hour, data_time.min, data_time.sec,
                  radarBeam.volumeNum,
                  radarBeam.azimuth, radarBeam.elevation);
        } else {
          fprintf(stdout, "\r%4d/%.2d/%.2d %.2d:%.2d:%.2d %5.1f %5.1f",
                  data_time.year, data_time.month, data_time.day,
                  data_time.hour, data_time.min, data_time.sec, 
                  radarBeam.azimuth, radarBeam.elevation);
        }
        fflush(stdout);
        break;

      }
      case Params::DD_MONITOR: {
        
        data_time.unix_time = radarBeam.dataTime;
        uconvert_from_utime(&data_time);
        for (int i = 0; i < _params.n_monitor_line_feeds; i++) {
          fprintf(stdout, "\n");
        }
        if (_params.labels_in_monitor_mode) {
          if (_params.volume_num_in_monitor_mode) {
            fprintf(stdout, "\n%10s %8s %4s %5s %5s\n",
                    "Date", "Time", "Vnum", "az", "elev");
          } else {
            fprintf(stdout, "\n%10s %8s %5s\n",
                    "Date", "Time", "elev");
          }
        }
        if (_params.volume_num_in_monitor_mode) {
          fprintf(stdout, "\r%4d/%.2d/%.2d %.2d:%.2d:%.2d %4d %5.1f %5.1f",
                  _VolStartTime.year, _VolStartTime.month, _VolStartTime.day,
                  _VolStartTime.hour, _VolStartTime.min, _VolStartTime.sec,
                  radarBeam.volumeNum,
                  radarBeam.azimuth, radarBeam.elevation);
        } else {
          fprintf(stdout, "\r%4d/%.2d/%.2d %.2d:%.2d:%.2d %5.1f",
                   _VolStartTime.year, _VolStartTime.month, _VolStartTime.day,
                  _VolStartTime.hour, _VolStartTime.min, _VolStartTime.sec, 
                  radarBeam.elevation);
        }
        fflush(stdout);
        break;

      }
        
      case Params::SUMMARY: {
        if (contents & DsRadarMsg::RADAR_BEAM) {
          _printSummary(radarMsg);
        }
        break;
      }
        
      case Params::SINGLE_GATE: {
        if (contents & DsRadarMsg::RADAR_BEAM) {
          _printSummary(radarMsg);
          _printGate(radarMsg);
        }
        break;
      }
        
      case Params::POWER_AND_FREQ: {
        if (contents & DsRadarMsg::RADAR_BEAM) {
          _printPowerAndFreq(radarMsg);
        }
        break;
      }
        
      case Params::PLATFORM_GEOREF: {
        if ((contents & DsRadarMsg::RADAR_BEAM) &&
            (contents & DsRadarMsg::PLATFORM_GEOREF)) {
          _printPlatformGeoref(radarMsg);
          const DsRadarBeam &radarBeam = radarMsg.getRadarBeam();
          radarBeam.print(cout);
        }
        break;
      }
        
      case Params::FULL:
      case Params::DATA: {
        
        if (contents & DsRadarMsg::RADAR_PARAMS) {
          radarParams.print(cout);
        }
        
        if (contents & DsRadarMsg::RADAR_CALIB) {
          radarCalib.print(cout);
        }
        
        if (contents & DsRadarMsg::STATUS_XML) {
          cout << "STATUS XML" << endl;
          cout << "----------" << endl;
          cout << statusXml << endl;
          cout << "----------" << endl << endl;
        }
        
        if (contents & DsRadarMsg::FIELD_PARAMS) {
          for (size_t fieldNum = 0; fieldNum < fieldParams.size(); fieldNum++) {
            fprintf(stdout, "FIELD NUMBER %d\n", (int) fieldNum);
            fieldParams[fieldNum]->print(cout);
          }
        }
        
        if (contents & DsRadarMsg::RADAR_BEAM) {
          
          radarBeam.print(cout);
          
          if (_params.print_type == Params::DATA) {
            
            for (size_t fieldNum = 0;
                 fieldNum < fieldParams.size(); fieldNum++) {
              
              const char *fieldName = fieldParams[fieldNum]->name.c_str();
              
              radarBeam.printFloatData(stdout,
                                       fieldName,
                                       fieldNum,
                                       radarParams.getNumFields(),
                                       fieldParams[fieldNum]->scale,
                                       fieldParams[fieldNum]->bias,
                                       fieldParams[fieldNum]->byteWidth,
                                       fieldParams[fieldNum]->missingDataValue);
            } // fieldNum
            
          } // if (_params.print_type == Params::DATA)
          
        } // if (contents & DsRadarMsg::RADAR_BEAM)

      }
        
    } // switch
    
    _lastPrintTime = radarBeam.dataTime;
    
  } else {
    
    _printCount--;
    
  } // if (doPrint)
  
} 

///////////////////////////////////////////////////
// print summary of beam headers

void RadMon::_printSummary(const DsRadarMsg &radarMsg)

{

  const DsRadarParams &radarParams = radarMsg.getRadarParams();
  const DsRadarBeam &radarBeam = radarMsg.getRadarBeam();
  
  // Parse the time of the beam

  date_time_t  dataTime;
  dataTime.unix_time = radarBeam.dataTime;
  uconvert_from_utime( &dataTime );

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
    case DS_RADAR_SUNSCAN_RHI_MODE:
      sprintf(scanModeStr, " SUNR");
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
            dataTime.year,
            dataTime.month,
            dataTime.day,
            dataTime.hour,
            dataTime.min,
            dataTime.sec);

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
            dataTime.year,
            dataTime.month,
            dataTime.day,
            dataTime.hour,
            dataTime.min,
            dataTime.sec);

  }

  if (_params.subsecs_precision_in_summary_mode > 0) {
    double divisor =
      1.0e9 / pow(10.0, _params.subsecs_precision_in_summary_mode);
    int subsecs = (int) (radarBeam.nanoSecs / divisor + 0.5);
    char format[32];
    sprintf(format, ".%%.%dd", _params.subsecs_precision_in_summary_mode);
    fprintf(stdout, format, subsecs);
  }

  if (radarBeam.antennaTransition) {
    fprintf(stdout, " *");
  }
  fprintf(stdout, "\n");

  fflush(stdout);

}

/////////////////////////////////////////
// prints flags information

void RadMon::_printFlags(const DsRadarMsg &radarMsg)

{

  const DsRadarFlags& flags = radarMsg.getRadarFlags();
  
  if (flags.startOfVolume) {
    _VolStartTime.unix_time = flags.time;
    uconvert_from_utime(&_VolStartTime);
  }

  switch (_params.print_type) {
    
    case Params::MONITOR:
      if (_params.flags_in_monitor_mode) {
        if (flags.endOfTilt) {
          fprintf(stdout, " EOT");
        }
        if (flags.endOfVolume) {
          fprintf(stdout, " EOV");
        }
        if (flags.startOfVolume) {
          fprintf(stdout, " SOV");
        }
        if (flags.startOfTilt) {
          fprintf(stdout, " SOT");
        }
        if (flags.newScanType) {
          fprintf(stdout, " NST");
        }
      }
      break;

    case Params::DD_MONITOR:
      fprintf(stdout, " %d ", flags.scanType);

      if (_params.flags_in_monitor_mode) {
        if (flags.endOfTilt) {
          fprintf(stdout, " EOT");
        }
        if (flags.endOfVolume) {
          fprintf(stdout, " EOV");
        }
        if (flags.startOfVolume) {
          fprintf(stdout, " SOV");
        }
        if (flags.startOfTilt) {
          fprintf(stdout, " SOT");
        }
        if (flags.newScanType) {
          fprintf(stdout, " NST");
        }
      }
      break;
      
    case Params::SUMMARY:
      if (_params.tilt_flags_in_summary_mode) {
        if (flags.endOfTilt) {
          fprintf(stdout, "------> End-of-tilt\n");
        }
      }
      if (flags.endOfVolume) {
        fprintf(stdout, "---> End-of-volume\n");
      }
      if (flags.startOfVolume) {
        fprintf(stdout, "---> Start-of-volume\n");
      }
      if (_params.tilt_flags_in_summary_mode) {
        if (flags.startOfTilt) {
          fprintf(stdout, "------> Start-of-tilt\n");
        }
      }
      if (flags.newScanType) {
        fprintf(stdout, "===> New-scan-type\n");
      }
      break;
      
    case Params::FULL:
    case Params::DATA:
    case Params::SINGLE_GATE:
      if (flags.endOfTilt) {
        fprintf(stdout, "-----------> End-of-tilt <----------------\n\n");
      }
      if (flags.endOfVolume) {
        fprintf(stdout, "-----------> End-of-volume <--------------\n\n");
      }
      if (flags.startOfVolume) {
        fprintf(stdout, "-----------> Start-of-volume <------------\n\n");
      }
      if (flags.startOfTilt) {
        fprintf(stdout, "-----------> Start-of-tilt <--------------\n\n");
      }
      if (flags.newScanType) {
        fprintf(stdout, "-----------> New-scan-type <--------------\n\n");
      }
      break;
      
    case Params::POWER_AND_FREQ:
    default: {}

  }
  
  fflush(stdout);

} 

/////////////////////////////////////////
// prints gate data

void RadMon::_printGate(const DsRadarMsg &radarMsg)

{

  const DsRadarParams &radarParams = radarMsg.getRadarParams();
  const DsRadarBeam &radarBeam = radarMsg.getRadarBeam();
  const vector<DsFieldParams*> &fieldParams = radarMsg.getFieldParams();
  
  // compute gate number and exact range to that gate
  
  int gateNum =
    (int) ((_params.range_for_single_gate - radarParams.startRange) /
	   radarParams.gateSpacing + 0.5);
  
  if (gateNum > radarParams.numGates - 1) {
    gateNum = radarParams.numGates - 1;
  }
  
  double range = radarParams.startRange + gateNum * radarParams.gateSpacing;
  
  fprintf(stdout, "Data for gate %d at range %g:\n", gateNum, range);
  
  for (size_t fieldNum = 0; fieldNum < fieldParams.size(); fieldNum++) {
    
    const char *fieldName = fieldParams[fieldNum]->name.c_str();
    int index = gateNum * fieldParams.size() + fieldNum;
    
    double val = 0;
    
    switch (fieldParams[fieldNum]->byteWidth) {
    case 4:
      {
	fl32 *data = (fl32 *) radarBeam.getData();
	if (data[index] == (fl32) fieldParams[fieldNum]->missingDataValue) {
	  val = -9999.0;
	} else {
	  val = data[index];
	}
      }
      break;
    case 2:
      {
	ui16 *data = (ui16 *) radarBeam.getData();
	if (data[index] == fieldParams[fieldNum]->missingDataValue) {
	  val = -9999.0;
	} else {
	  val = data[index] * fieldParams[fieldNum]->scale +
	    fieldParams[fieldNum]->bias;
	}
      }
      break;
    case 1:
      {
	ui08 *data = (ui08 *) radarBeam.getData();
	if (data[index] == fieldParams[fieldNum]->missingDataValue) {
	  val = -9999.0;
	} else {
	  val = data[index] * fieldParams[fieldNum]->scale +
	    fieldParams[fieldNum]->bias;
	}
      }
      break;
    }
    
    fprintf(stdout, "  Field, val: %s, %g\n", fieldName, val);
    
  } // fieldNum
  
}


///////////////////////////////////////////////////
// print power and frequency

void RadMon::_printPowerAndFreq(const DsRadarMsg &radarMsg)

{

  const DsRadarParams &radarParams = radarMsg.getRadarParams();
  const DsRadarBeam &radarBeam = radarMsg.getRadarBeam();
  
  // Parse the time of the beam
  
  date_time_t  dataTime;
  dataTime.unix_time = radarBeam.dataTime;
  uconvert_from_utime( &dataTime );

  double wavelengthCm = radarParams.wavelength;
  double wavelengthM = wavelengthCm / 100.0;
  double freqHz = 3.0e8 / wavelengthM;
  double freqGHz = freqHz / 1.0e9;
  double powerH = radarBeam.measXmitPowerDbmH;
  double powerV = radarBeam.measXmitPowerDbmV;

  fprintf(stdout,
          "Time "
          "wavelength(cm) freq(GHz) powerH(dBm) powerV(dBm): "
          "%.4d/%.2d/%.2d %.2d:%.2d:%.2d %.5f %.7f %.5f %.5f\n",
          dataTime.year,
          dataTime.month,
          dataTime.day,
          dataTime.hour,
          dataTime.min,
          dataTime.sec,
          wavelengthCm,
          freqGHz,
          powerH,
          powerV);

  fflush(stdout);

}

///////////////////////////////////////////////////
// print platform georeference info

void RadMon::_printPlatformGeoref(const DsRadarMsg &radarMsg)

{

  const DsPlatformGeoref &platformGeoref = radarMsg.getPlatformGeoref();
  fprintf(stdout, "=================================================\n");
  platformGeoref.print(stdout);
  fflush(stdout);

}

////////////////////////////////////////////
// check for missing beams

void RadMon::_checkForMissingBeams(int contents,
                                   const DsRadarMsg &radarMsg)
  
{

  if (!(contents & DsRadarMsg::RADAR_BEAM)) {
    return;
  }

  const DsRadarParams &radarParams = radarMsg.getRadarParams();
  const DsRadarBeam &radarBeam = radarMsg.getRadarBeam();
  date_time_t  dataTime;
  dataTime.unix_time = radarBeam.dataTime;
  uconvert_from_utime(&dataTime);

  int scanMode = radarParams.scanMode;
  int volNum = radarBeam.volumeNum;
  int tiltNum = radarBeam.tiltNum;
  double az = radarBeam.azimuth;
  double el = radarBeam.elevation;

  if (scanMode < 0 || tiltNum < 0) {
    return;
  }

  if (scanMode != _prevScanMode ||
      volNum != _prevVolNum ||
      tiltNum != _prevTiltNum) {
    _prevScanMode = scanMode;
    _prevVolNum = volNum;
    _prevTiltNum = tiltNum;
    _prevAz = az;
    _prevEl = el;
    return;
  }

  if (scanMode == DS_RADAR_RHI_MODE) {
    
    double elDiff = RapComplex::computeDiffDeg(el, _prevEl);
    if (fabs(elDiff) > _params.max_delta_angle) {
      fprintf(stdout, 
              "==>> WARNING - RHI missing beam(s), el change: %.3f\n", elDiff);
      fprintf(stdout, "==>> El changed from %.2f to %.2f deg\n", _prevEl, el);
    }
    
  } else {

    double azDiff = RapComplex::computeDiffDeg(az, _prevAz);
    if (fabs(azDiff) > _params.max_delta_angle) {
      fprintf(stdout, 
              "==>> WARNING - missing beam(s), az change: %.3f\n", azDiff);
      fprintf(stdout, "==>> Az changed from %.2f to %.2f deg\n", _prevAz, az);
    }
    
  }
 
  _prevAz = az;
  _prevEl = el;

} 

