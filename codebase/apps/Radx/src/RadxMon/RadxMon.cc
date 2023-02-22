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
//////////////////////////////////////////////////////////////////////
// RadxMon.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2012
//
//////////////////////////////////////////////////////////////////////
//
// RadxMon prints out radar moments in a variety of ways
//
//////////////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#include <Fmq/DsRadarQueue.hh>
#include <rapmath/RapComplex.hh>
#include <radar/IwrfMomReader.hh>
#include "RadxMon.hh"
using namespace std;

// Constructor

RadxMon::RadxMon(int argc, char **argv)

{

  isOK = true;
  _lastPrintTime = 0;
  _summaryCount = 0;
  _printCount = 0;
  _numGetDsBeamFails = 0;
  _prevSweepMode = Radx::SWEEP_MODE_NOT_SET;
  _prevVolNum = -1;
  _prevSweepNum = -1;
  _prevAz = 0.0;
  _prevEl = 0.0;
  _reader = NULL;
  _sock = NULL;

  _azRateInitialized = false;
  _prevTimeForAzRate = 0;
  _prevAzForRate = -999;
  _azRate = -999;

  _elRateInitialized = false;
  _prevTimeForElRate = 0;
  _prevElForRate = -999;
  _elRate = -999;

  _nSecsForRate = _params.nsecs_for_antenna_rate;

  // set programe name

  _progName = "RadxMon";
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

  // create reader

  if (_params.input_mode == Params::FMQ_INPUT) {
    
    _reader = new IwrfMomReaderFmq(_params.fmq_url);
    if (_params.seek_to_start_of_fmq) {
      _reader->seekToStart();
    }

  } else if (_params.input_mode == Params::TCP_INPUT) {

    _reader = new IwrfMomReaderTcp(_params.input_tcp_host,
                                   _params.input_tcp_port);
    
  } else if (_params.input_mode == Params::FILE_LIST) {

    _reader = new IwrfMomReaderFile(_args.inputFileList);
    
  } else if (_params.input_mode == Params::FILE_REALTIME) {

    _reader = new IwrfMomReaderFile(_params.files_input_dir);
    
  } else {

    cerr << "ERROR: " << _progName << endl;
    cerr << "  Unknown input mode: " << _params.input_mode << endl;
    isOK = FALSE;
    return;

  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

RadxMon::~RadxMon()

{

  if (_reader) {
    delete _reader;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxMon::Run()
{

  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    if (_params.ops_mode == Params::OPS_MODE_PRINT) {
      _runPrint();
    } else {
      _runServer();
    }
    sleep(1);
  }

  return (0);

}

//////////////////////////////////////////////////
// _run in print mode

int RadxMon::_runPrint()
{

  // register with procmap
  
  PMU_auto_register("_runPrint");

  while (true) { 
    
    PMU_auto_register("Reading moments data");
    
    // get the next ray
    
    RadxRay *ray = _reader->readNextRay();
    if (ray == NULL) {
      // read error
      continue;
    }
    ray->convertToFl32();

    // print events if they apply
    
    if (_reader->getEvents().size() > 0) {
      _printEvents();
    }

    // print ray

    _printRay(ray);

    if (_params.check_for_missing_beams) {
      _checkForMissingBeams(ray);
    }

    // clean up

    delete ray;
    
  } // while

  return 0;

}

//////////////////////////////////////////////////
// _run in server mode

int RadxMon::_runServer()
{

  // open the server to listen for gui connections
  
  if (_params.debug) {
    cerr << "Listening on port: " << _params.output_tcp_port << endl;
  }
  if (_serverInit()) {
    return -1;
  }

  while (true) { 

    char text[1024];
    sprintf(text, "Listening on port: %d", _params.output_tcp_port);
    PMU_auto_register(text);
    
    // get the next ray
    
    RadxRay *ray = _reader->readNextRay();
    ray->convertToFl32();
    _computeAzRate(ray);
    _computeElRate(ray);

    // check if we have a request pending
    
    _sock = _server.getClient(10);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Checking for client ....." << endl;
    }
    if (_sock != NULL) {
      // client found
      if (_params.debug) {
        cerr << "  Got client ....." << endl;
      }
      _serviceRequest(ray);
      _sock->close();
      delete _sock;
      _sock = NULL;
    }
    
    // clean up
    
    delete ray;

  } // while

  return 0;

}

/////////////////////////////////////////
// print events

void RadxMon::_printEvents()

{
  
  bool startOfSweep = false;
  bool endOfSweep = false;
  bool startOfVolume = false;
  bool endOfVolume = false;

  for (size_t ii = 0; ii < _reader->getEvents().size(); ii++) {
    const RadxEvent &event = _reader->getEvents()[ii];
    if (event.getStartOfSweep()) {
      startOfSweep = true;
    }
    if (event.getEndOfSweep()) {
      endOfSweep = true;
    }
    if (event.getStartOfVolume()) {
      startOfVolume = true;
    }
    if (event.getEndOfVolume()) {
      endOfVolume = true;
    }
  } // ii

  switch (_params.print_mode) {
    
    case Params::PRINT_MONITOR:
      if (_params.flags_in_monitor_mode) {
        if (endOfSweep) {
          fprintf(stdout, " EOT");
        }
        if (endOfVolume) {
          fprintf(stdout, " EOV");
        }
        if (startOfVolume) {
          fprintf(stdout, " SOV");
        }
        if (startOfSweep) {
          fprintf(stdout, " SOT");
        }
      }
      break;
      
    case Params::PRINT_SUMMARY:
      if (_params.flags_in_summary_mode) {
        if (endOfSweep) {
          fprintf(stdout, "--> End-of-tilt\n");
        }
      }
      if (endOfVolume) {
        fprintf(stdout, "----> End-of-volume\n");
      }
      if (startOfVolume) {
        fprintf(stdout, "----> Start-of-volume\n");
      }
      if (_params.flags_in_summary_mode) {
        if (startOfSweep) {
          fprintf(stdout, "--> Start-of-tilt\n");
        }
      }
      break;
      
    case Params::PRINT_FULL:
    case Params::PRINT_DATA:
    case Params::PRINT_SINGLE_GATE:
      if (endOfSweep) {
        fprintf(stdout, "-----------> End-of-tilt <----------------\n\n");
      }
      if (endOfVolume) {
        fprintf(stdout, "-----------> End-of-volume <--------------\n\n");
      }
      if (startOfVolume) {
        fprintf(stdout, "-----------> Start-of-volume <------------\n\n");
      }
      if (startOfSweep) {
        fprintf(stdout, "-----------> Start-of-tilt <--------------\n\n");
      }
      break;
      
    case Params::PRINT_POWER_AND_FREQ:
    default: {}

  }
  
  fflush(stdout);

} 

////////////////////////////////////////////
// print ray information

void RadxMon::_printRay(const RadxRay *ray)
  
{

  DateTime rtime(ray->getTimeSecs());
  
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
    if (rtime.utime() - _lastPrintTime >= _params.update_interval) {
      doPrint = true;
    }
  }
  
  if (_params.print_mode == Params::PRINT_FULL ||
      _params.print_mode == Params::PRINT_DATA) {
    if (_reader->getPlatformUpdated() ||
        _reader->getRcalibUpdated() ||
        _reader->getStatusXmlUpdated()) {
      doPrint = true;
    }
  }

  if (doPrint) {
    
    _printCount = -1;
    switch (_params.print_mode) {
      
      case Params::PRINT_MONITOR: {
        
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
                  rtime.getYear(), rtime.getMonth(), rtime.getDay(),
                  rtime.getHour(), rtime.getMin(), rtime.getSec(),
                  ray->getVolumeNumber(),
                  ray->getAzimuthDeg(), ray->getElevationDeg());
        } else {
          fprintf(stdout, "\r%4d/%.2d/%.2d %.2d:%.2d:%.2d %5.1f %5.1f",
                  rtime.getYear(), rtime.getMonth(), rtime.getDay(),
                  rtime.getHour(), rtime.getMin(), rtime.getSec(),
                  ray->getAzimuthDeg(), ray->getElevationDeg());
        }
        fflush(stdout);
        break;
        
      }
        
      case Params::PRINT_SUMMARY: {
        _printSummary(ray);
        break;
      }
        
      case Params::PRINT_SINGLE_GATE: {
        _printSummary(ray);
        _printGate(ray);
        break;
      }
        
      case Params::PRINT_POWER_AND_FREQ: {
        _printPowerAndFreq(ray);
        break;
      }
        
      case Params::PRINT_PLATFORM_GEOREF: {
        ray->print(cout);
        break;
      }
        
      case Params::PRINT_FULL: {
        if (_reader->getPlatformUpdated()) {
          _reader->getPlatform().print(cerr);
        }
        if (_reader->getStatusXmlUpdated()) {
          cout << "STATUS XML" << endl;
          cout << "----------" << endl;
          cout << _reader->getStatusXml() << endl;
          cout << "----------" << endl << endl;
        }
        ray->printWithFieldMeta(cout);
        break;
      }
        
      case Params::PRINT_DATA: {
        
        if (_reader->getPlatformUpdated()) {
          _reader->getPlatform().print(cerr);
        }
        
        if (_reader->getStatusXmlUpdated()) {
          cout << "STATUS XML" << endl;
          cout << "----------" << endl;
          cout << _reader->getStatusXml() << endl;
          cout << "----------" << endl << endl;
        }
        
        ray->printWithFieldData(cout);
        
      }
        
    } // switch
    
    _lastPrintTime = rtime.utime();
    
  } else {
    
    _printCount--;
    
  } // if (doPrint)
  
} 

///////////////////////////////////////////////////
// print summary of ray headers

void RadxMon::_printSummary(const RadxRay *ray)

{

  // Parse the time of the beam
  
  DateTime rtime(ray->getTimeSecs());

  char scanModeStr[32];
  bool isPpi = true;
  switch(ray->getSweepMode()) {
    case Radx::SWEEP_MODE_SECTOR:
    case Radx::SWEEP_MODE_MANUAL_PPI:
      sprintf(scanModeStr, "SECT");
      break;
    case Radx::SWEEP_MODE_COPLANE:
      sprintf(scanModeStr, "COPL");
      break;
    case Radx::SWEEP_MODE_RHI:
    case Radx::SWEEP_MODE_MANUAL_RHI:
      sprintf(scanModeStr, " RHI");
      isPpi = false;
      break;
    case Radx::SWEEP_MODE_VERTICAL_POINTING:
      sprintf(scanModeStr, "VERT");
      break;
    case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
      sprintf(scanModeStr, " SUR");
      break;
    case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE:
      sprintf(scanModeStr, " ELSUR");
      break;
    case Radx::SWEEP_MODE_SUNSCAN:
      sprintf(scanModeStr, " SUN");
      break;
    case Radx::SWEEP_MODE_POINTING:
      sprintf(scanModeStr, " PNT");
      break;
    case Radx::SWEEP_MODE_CALIBRATION:
      sprintf(scanModeStr, " CAL");
      break;
    case Radx::SWEEP_MODE_IDLE:
      sprintf(scanModeStr, "IDLE");
      break;
    default:
      sprintf(scanModeStr, "%4d", ray->getSweepMode());
  }

  double fixedAngle = ray->getFixedAngleDeg();
  double sweepAngle, rayAngle;
  if (isPpi) {
    if (_summaryCount % _params.header_interval_in_summary_mode == 0) {
      fprintf(stdout,
              "Mode   Vol Swp  El_tgt  El_act      Az"
              " Ngat NSamp Gspac  PRF     "
              "  Date     Time\n");
    }
    sweepAngle = ray->getElevationDeg();
    rayAngle = ray->getAzimuthDeg();
  } else {
    if (_summaryCount % _params.header_interval_in_summary_mode == 0) {
      fprintf(stdout,
              "Mode   Vol Swp  Az_tgt  Az_act      El"
              " Ngat NSamp Gspac  PRF     "
              "  Date     Time\n");
    }
    sweepAngle = ray->getAzimuthDeg();
    rayAngle = ray->getElevationDeg();
  }
  
  
  fprintf(stdout,
          "%4s %5d %3d %7.2f %7.2f %7.2f"
          " %4d %5d %5.0f %4.0f "
          "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
          scanModeStr,
          ray->getVolumeNumber(),
          ray->getSweepNumber(),
          fixedAngle,
          sweepAngle,
          rayAngle,
          (int) ray->getNGates(),
          (int) ray->getNSamples(),
          (ray->getGateSpacingKm() * 1000),
          1.0 / ray->getPrtSec(),
          rtime.getYear(),
          rtime.getMonth(),
          rtime.getDay(),
          rtime.getHour(),
          rtime.getMin(),
          rtime.getSec());

  if (_params.subsecs_precision_in_summary_mode > 0) {
    double divisor =
      1.0e9 / pow(10.0, _params.subsecs_precision_in_summary_mode);
    int subsecs = (int) (ray->getNanoSecs() / divisor + 0.5);
    char format[32];
    sprintf(format, ".%%.%dd", _params.subsecs_precision_in_summary_mode);
    fprintf(stdout, format, subsecs);
  }

  if (_params.scan_name_in_summary_mode && ray->getScanName().size() > 0) {
    fprintf(stdout, " %s", ray->getScanName().c_str());
  }
  
  if (ray->getAntennaTransition()) {
    fprintf(stdout, " *");
  }
  fprintf(stdout, "\n");

  fflush(stdout);

  _summaryCount++;

}

/////////////////////////////////////////
// print gate data

void RadxMon::_printGate(const RadxRay *ray)

{

  // compute gate number and exact range to that gate

  double startRange = ray->getStartRangeKm();
  double gateSpacing = ray->getGateSpacingKm();
  
  int gateNum =
    (int) ((_params.range_for_single_gate - startRange) / gateSpacing + 0.5);
  
  if (gateNum > (int) ray->getNGates() - 1) {
    gateNum = ray->getNGates() - 1;
  }
  
  double range = startRange + gateNum * gateSpacing;
  fprintf(stdout, "Data for gate %d at range %g:\n", gateNum, range);
  
  const vector<RadxField *> &fields = ray->getFields();

  for (size_t fieldNum = 0; fieldNum < fields.size(); fieldNum++) {

    const RadxField &field = *fields[fieldNum];
    
    const char *fieldName = field.getName().c_str();
    double val = field.getDoubleValue(gateNum);
    fprintf(stdout, "  Field, val: %s, %g\n", fieldName, val);
    
  } // fieldNum
  
}


///////////////////////////////////////////////////
// print power and frequency

void RadxMon::_printPowerAndFreq(const RadxRay *ray)

{

  DateTime rtime(ray->getTimeSecs());

  double wavelengthCm = _reader->getPlatform().getWavelengthCm();
  double wavelengthM = _reader->getPlatform().getWavelengthM();
  double freqHz = 3.0e8 / wavelengthM;
  double freqGHz = freqHz / 1.0e9;
  double powerH = ray->getMeasXmitPowerDbmH();
  double powerV = ray->getMeasXmitPowerDbmV();

  fprintf(stdout,
          "Time "
          "wavelength(cm) freq(GHz) powerH(dBm) powerV(dBm): "
          "%.4d/%.2d/%.2d %.2d:%.2d:%.2d %.5f %.7f %.5f %.5f\n",
          rtime.getYear(),
          rtime.getMonth(),
          rtime.getDay(),
          rtime.getHour(),
          rtime.getMin(),
          rtime.getSec(),
          wavelengthCm,
          freqGHz,
          powerH,
          powerV);

  fflush(stdout);

}

////////////////////////////////////////////
// check for missing beams

void RadxMon::_checkForMissingBeams(const RadxRay *ray)
  
{

  DateTime rtime(ray->getTimeSecs());

  Radx::SweepMode_t sweepMode = ray->getSweepMode();
  int volNum = ray->getVolumeNumber();
  int sweepNum = ray->getSweepNumber();
  double az = ray->getAzimuthDeg();
  double el = ray->getElevationDeg();

  if (sweepMode == Radx::SWEEP_MODE_NOT_SET || sweepNum < 0) {
    return;
  }

  if (sweepMode != _prevSweepMode ||
      volNum != _prevVolNum ||
      sweepNum != _prevSweepNum) {
    _prevSweepMode = sweepMode;
    _prevVolNum = volNum;
    _prevSweepNum = sweepNum;
    _prevAz = az;
    _prevEl = el;
    return;
  }

  if (sweepMode == Radx::SWEEP_MODE_RHI) {
    
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

////////////////////////////////////////////
// initialize server
// returns 0 on success, -1 on failure

int RadxMon::_serverInit()

{
  
  if (_server.openServer(_params.output_tcp_port)) {
    cerr << "ERROR - RadxMon::_serverInit" << endl;
    cerr << "  Cannot open server, port: " <<
      _params.output_tcp_port << endl;
    cerr << "  " << _server.getErrStr() << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////
// service a request from client
// returns 0 on success, -1 on failure

int RadxMon::_serviceRequest(const RadxRay *ray)

{

  // read incoming message
  
  string message;
  if (_readMessage(message)) {
    // bad message
    return -1;
  }
  
  // check the message
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Got GUI message:  ===========>>" << endl;
    cerr << message << endl;
    cerr << "<<=============================" << endl;
  }

  string reply;
  string action;
  int iret = 0;

  if (TaXml::readString(message, "action", action)) {

    cerr << "Bad message from GUI";
    cerr << message;
    _createErrorReply(reply, "No 'action' tag in message");
    iret = -1;
    
  } else {

    if (action == "SendStatus") {
      _createStatusReply(ray, reply);
    } else {
      _createErrorReply(reply, "<action> not recognized: " + action);
      iret = -1;
    }

  }
  
  // send the reply
  
  _sendReply(reply);
  
  return iret;

}

//////////////////////////////////////////////////
// Read incoming message
//
// Returns 0 on success, -1 on failure

int RadxMon::_readMessage(string &message)
  
{
  
  message.clear();
  char c1 = '\0';
  char c2 = '\0';
  char c3 = '\0';
  char c4 = '\0';

  int badCount = 0;
  while (true) {

    // check for available data
    // time out after 0.1 sec if no end sequence
    
    int iret = _sock->readSelect(100);
    if (iret != 0) {
      if (_sock->getErrNum() == SockUtil::TIMED_OUT) {
        cerr << "ERROR - RadxMon::_readIncoming()";
        cerr << "  No /r/n/r/n sequence found";
      } else {
        cerr << "ERROR - RadxMon::_readIncoming()";
        cerr << "  readSelect() failed";
        cerr << "  " << _sock->getErrStr();
      }
      return -1;
    }

    // read next char, check for end sequence of "\r\n\r\n"
    
    char cc;
    if (_sock->readBuffer(&cc, 1, 100) == 0) {
      message += cc;
      c4 = c3;
      c3 = c2;
      c2 = c1;
      c1 = cc;
      if (c1 == '\n' && c2 == '\r' &&
          c3 == '\n' && c4 == '\r') {
        // end of message found
        return 0;
      }
      badCount = 0;
    } else {
      badCount++;
    }

    if (badCount > 1000) {
      return -1;
    }

  } // while

  return -1;

}

//////////////////////////////////////////////////
// Handle status request
// Create status reply message

int RadxMon::_createStatusReply(const RadxRay *ray, string &reply)
  
{

  reply.clear();

  reply += TaXml::writeStartTag("RadxMonReply", 0);
  reply += TaXml::writeStartTag("Status", 1);

  reply += TaXml::writeInt("volume_number", 2,
                           ray->getVolumeNumber());
  reply += TaXml::writeInt("sweep_number", 2,
                           ray->getSweepNumber());
  reply += TaXml::writeString("sweep_mode", 2,
                              Radx::sweepModeToStr(ray->getSweepMode()));
  reply += TaXml::writeTime("ray_time", 2,
                            ray->getTimeSecs());
  reply += TaXml::writeInt("nano_secs", 2,
                            ray->getNanoSecs());
  reply += TaXml::writeDouble("azimuth_deg", 2,
                              ray->getAzimuthDeg());
  reply += TaXml::writeDouble("elevation_deg", 2,
                              ray->getElevationDeg());
  reply += TaXml::writeDouble("fixed_angle_deg", 2,
                              ray->getFixedAngleDeg());
  if (ray->getSweepMode() == Radx::SWEEP_MODE_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
    reply += TaXml::writeDouble("scan_rate_deg_per_sec", 2, _elRate);
  } else {
    reply += TaXml::writeDouble("scan_rate_deg_per_sec", 2, _azRate);
  }

  reply += TaXml::writeBoolean("is_indexed", 2, 
                               ray->getIsIndexed());
  reply += TaXml::writeDouble("angle_res_deg", 2, 
                              ray->getAngleResDeg());
  reply += TaXml::writeBoolean("antenna_transition", 2, 
                               ray->getAntennaTransition());
  reply += TaXml::writeInt("n_samples", 2,
                           ray->getNSamples());
  reply += TaXml::writeString("prt_mode", 2,
                              Radx::prtModeToStr(ray->getPrtMode()));
  reply += TaXml::writeDouble("pulse_width_usec", 2, 
                              ray->getPulseWidthUsec());
  reply += TaXml::writeDouble("pulse_prt_sec", 2, 
                              ray->getPrtSec());
  reply += TaXml::writeDouble("prt_ratio", 2, 
                              ray->getPrtRatio());
  reply += TaXml::writeDouble("nyquist_mps", 2, 
                              ray->getNyquistMps());
  reply += TaXml::writeDouble("unambig_range_km", 2, 
                              ray->getUnambigRangeKm());
  
  reply += TaXml::writeEndTag("Status", 1);
  reply += TaXml::writeEndTag("RadxMonReply", 0);
  reply += "\r\n\r\n";

  return 0;

}

//////////////////////////////////////////////////
// Create an error reply

void RadxMon::_createErrorReply(string &reply,
                                const string &cause)
  
{
  
  reply.clear();
  
  reply += TaXml::writeStartTag("RadxMonReply", 0);
  reply += TaXml::writeStartTag("Error", 1);
  reply += TaXml::writeString("cause", 2, cause);
  reply += TaXml::writeEndTag("Error", 1);
  reply += TaXml::writeEndTag("RadxMonReply", 0);
  reply += "\r\n\r\n";

}

  
//////////////////////////////////////////////////
// Send the reply 
// Returns 0 on success, -1 on failure

int RadxMon::_sendReply(const string &reply)
  
{
  
  PMU_auto_register("Sending reply");
  if (_params.debug) {
    cerr << "Sending reply" << endl;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "========= sending reply ==========" << endl;
    cerr << reply << endl;
    cerr << "==================================" << endl;
  }

  if (_sock->writeBuffer((char *) reply.c_str(), reply.size() + 1)) {
    cerr << "ERROR - _sendReply" << endl;
    cerr << _sock->getErrStr() << endl;
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////////
// compute azimuth rate

double RadxMon::_computeAzRate(const RadxRay *ray)

{
  
  double az = ray->getAzimuthDeg();
  double rayTime = ray->getTimeDouble();

  // check we have initialized this routine
  
  if (!_azRateInitialized) {
    _azRate = 0;
    _prevTimeForAzRate = rayTime;
    _prevAzForRate = az;
    _azRateInitialized = true;
    return _azRate;
  }

  // is antenna in transition?
  // Only start comps when transition is completed
  
  if (ray->getAntennaTransition()) {
    _prevTimeForAzRate = rayTime;
    _prevAzForRate = az;
    return _azRate;
  }

  // check we have waited enough time
 
  double deltaTime = rayTime - _prevTimeForAzRate;
  if (deltaTime < _nSecsForRate) {
    return _azRate;
  }

  // compute rate based on time and az travel

  double deltaAz = az - _prevAzForRate;
  if (deltaAz > 180) {
    deltaAz -= 360;
  } else if (deltaAz < -180) {
    deltaAz += 360;
  }

  _azRate = fabs(deltaAz) / deltaTime;

  _prevTimeForAzRate = rayTime;
  _prevAzForRate = az;

  return _azRate;

}

////////////////////////////////////////////////////////////////
// compute elevation rate

double RadxMon::_computeElRate(const RadxRay *ray)
  
{
  
  double el = ray->getElevationDeg();
  double rayTime = ray->getTimeDouble();
  
  // check we have initialized this routine

  if (!_elRateInitialized) {
    _elRate = 0;
    _prevTimeForElRate = rayTime;
    _prevElForRate = el;
    _elRateInitialized = true;
    return _elRate;
  }

  // is antenna in transition?
  // Only start comps when transition is completed

  if (ray->getAntennaTransition()) {
    _prevTimeForElRate = rayTime;
    _prevElForRate = el;
    return _elRate;
  }

  // check we have waited enough time

  double deltaTime = rayTime - _prevTimeForElRate;
  if (deltaTime < _nSecsForRate) {
    return _elRate;
  }
  
  // compute rate based on time and az travel

  double deltaEl = el - _prevElForRate;
  if (deltaEl > 180) {
    deltaEl -= 360;
  } else if (deltaEl < -180) {
    deltaEl += 360;
  }
  
  _elRate = fabs(deltaEl) / deltaTime;
  
  _prevTimeForElRate = rayTime;
  _prevElForRate = el;

  return _elRate;

}

