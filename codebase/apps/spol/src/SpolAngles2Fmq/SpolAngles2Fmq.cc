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
// SpolAngles2Fmq.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////
//
// SpolAngles2Fmq reads angle data from the S2D processor via
// UDP, and writes them out to an FMQ.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cmath>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif

#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>
#include <didss/DsMsgPart.hh>
#include "SpolAngles2Fmq.hh"

using namespace std;

// Constructor

SpolAngles2Fmq::SpolAngles2Fmq(int argc, char **argv)
  
{

  isOK = true;
  _msgCount = 0;
  _stopTime = 0;
  _prevTimeMonitor = 0;
  _prevTimeMotionCheck = 0;
  _prevAzForMotion = 0;
  _prevElForMotion = 0;

  gettimeofday(&_prevTimeForRate, NULL);
  _rateAz = 0;
  _rateEl = 0;
  _sumDeltaAz = 0.0;
  _sumDeltaEl = 0.0;
  _prevAzForRate = -9999.0;
  _prevElForRate = -9999.0;

  // set programe name
 
  _progName = "SpolAngles2Fmq";

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

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  return;
  
}

// destructor

SpolAngles2Fmq::~SpolAngles2Fmq()

{

  _sock.close();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SpolAngles2Fmq::Run ()
{

  PMU_auto_register("Run");

  if (_params.ops_mode == Params::READ_SERVER_MODE) {
    return _runServerMode();
  } else if (_params.ops_mode == Params::FMQ_MONITOR_MODE) {
    return _runMonitorMode();
  } else {
    cerr << "ERROR - SpolAngle2Fmq::Run()" << endl;
    cerr << "  Bad ops mode: " << _params.ops_mode << endl;
    return -1;
  }

}

//////////////////////////////////////////////////
// Run in mode that reads from the server

int SpolAngles2Fmq::_runServerMode()
{

  PMU_auto_register("runServerMode");

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Running SpolAngles2Fmq - verbose debug mode" << endl;
  } else if (_params.debug) {
    cerr << "Running SpolAngles2Fmq - debug mode" << endl;
  }
  if (_params.debug) {
    cerr << "  FMQ: " << _params.output_fmq_path << endl;
    cerr << "  nSlots: " << _params.output_fmq_nslots << endl;
    cerr << "  nBytes: " << _params.output_fmq_size << endl;
  }
  
  // initialize the output FMQ

  if (_fmq.initReadWrite(_params.output_fmq_path,
                         _progName.c_str(),
                         _params.debug >= Params::DEBUG_EXTRA, // set debug?
                         Fmq::END, // start position
                         false,    // compression
                         _params.output_fmq_nslots,
                         _params.output_fmq_size)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize FMQ: " << _params.output_fmq_path << endl;
    cerr << "  nSlots: " << _params.output_fmq_nslots << endl;
    cerr << "  nBytes: " << _params.output_fmq_size << endl;
    cerr << _fmq.getErrStr() << endl;
    return -1;
  }
  
  _fmq.setSingleWriter();
  if (_params.data_mapper_report_interval > 0) {
    _fmq.setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }
  
  // initialize message
  
  _msg.clearAll();
  _msg.setType(0);
  
  int iret = 0;
  
  while (true) {
    
    PMU_auto_register("Main loop");
    
    // open socket to UDP server
    
    if (_openSocket()) {
      // failure - wait a while
      umsleep(1000);
      continue;
    }

    // read from the server
    
    if (_readFromServer()) {
      iret = -1;
    }

    _sock.close();
    
  } // while(true)

  return iret;
  
}

///////////////////////////////////////////////////////
// open the socket to the server
//
// returns 0 on success, -1 on failure

int SpolAngles2Fmq::_openSocket()

{

  PMU_auto_register("Opening socket");
    
  if (_params.debug) {
    cerr << "Opening connection to angle server, host, port: "
         << _params.angle_server_host << ","
         << _params.angle_server_port << endl;
  }
  
  // open socket - wait 1 second

  _sock.close();
  if (_sock.open(_params.angle_server_host,
                 _params.angle_server_port,
                 1000)) {
    if (_sock.getErrNum() != SockUtil::TIMED_OUT) {
      cerr << "ERROR - SpolAngles2Fmq::_openSocket()" << endl;
      cerr << _sock.getErrStr() << endl;
    }
    return -1;
  }
  
  return 0;

}

/////////////////////////////
// read data from the server

int SpolAngles2Fmq::_readFromServer()

{

  int iret = 0;
  
  struct timeval tv;
  gettimeofday(&tv, NULL);
  _prevTimeMsgRate = (double) tv.tv_sec + tv.tv_usec / 1.0e6;
  _prevTimeMotionCheck = _prevTimeMsgRate;
  _usecsSleepOnRead = (int) floor((1.0 / _params.angle_read_frequency_hz) * 1000000.0 + 0.5);

  // read data
  
  while (true) {
    
    PMU_auto_register("Reading data");

    // write command to angle server

    si32 command = 0x66660001;
    if (_sock.writeBuffer(&command, sizeof(command))) {
      cerr << "ERROR - SpolAngles2Fmq::_readFromServer()" << endl;
      cerr << "  Writing command to server" << endl;
      cerr << _sock.getErrStr();
      return -1;
    }
    
    // get time
    
    gettimeofday(&tv, NULL);
    _now = (double) tv.tv_sec + tv.tv_usec / 1.0e6;

    // read angle struct from socket
    // this will block
    
    if (_sock.readBuffer(&_angles, sizeof(_angles))) {
      cerr << "ERROR - SpolAngles2Fmq::_readFromServer()" << endl;
      cerr << "  Reading angles from server" << endl;
      cerr << _sock.getErrStr();
      return -1;
    }
    _msgCount++;

    // swap angles as needed

    spol_swap(_angles);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "---------------------------------------\n");
      fprintf(stderr, "Read angles from server\n");
      spol_print(stderr, _angles);
      fprintf(stderr, "---------------------------------------\n");
    }

    // compute rates

    _computeAngularRates();

    // compute message rate

    _computeMessageRate();

    // add to the outgoing FMQ message
    
    _msg.addPart(SPOL_SHORT_ANGLE_ID, sizeof(_angles), &_angles);
    
    // write to fmq if sufficient number of angles in message
    
    if (_writeToFmq()) {
      iret = -1;
    }
      
    if (_params.check_antenna_motion_status) {
      _checkMotionStatus();
    }

    // sleep a while

    uusleep(_usecsSleepOnRead);
    
  } // while

  return iret;

}

///////////////////////////////////////
// write to output FMQ if ready
// returns 0 on success, -1 on failure

int SpolAngles2Fmq::_writeToFmq()

{

  // if the message is large enough, write to the FMQ
  
  int nParts = _msg.getNParts();
  if (nParts < _params.n_angles_per_message) {
    return 0;
  }
  
  PMU_auto_register("writeToFmq");

  void *buf = _msg.assemble();
  int len = _msg.lengthAssembled();
  if (_fmq.writeMsg(0, 0, buf, len)) {
    cerr << "ERROR - SpolAngles2Fmq" << endl;
    cerr << "  Cannot write FMQ: " << _params.output_fmq_path << endl;
    _msg.clearParts();
    return -1;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "  Wrote msg, nparts, len: "
         << nParts << ", " << len << endl;
  }
  _msg.clearParts();

  return 0;

}

///////////////////////////
// compute angular rates

void SpolAngles2Fmq::_computeAngularRates()
  
{

  double el = _angles.elevation;
  double az = _angles.azimuth;

  // initialize

  if (_prevAzForRate < -9990 || _prevElForRate < -9990) {
    gettimeofday(&_prevTimeForRate, NULL);
    _prevElForRate = el;
    _prevAzForRate = az;
    _sumDeltaAz = 0.0;
    _sumDeltaEl = 0.0;
    _rateAz = 0.0;
    _rateEl = 0.0;
    return;
  }
  
  double deltaAz = az - _prevAzForRate;
  if (deltaAz > 180.0) {
    deltaAz -= 360.0;
  } else if (deltaAz < -180.0) {
    deltaAz += 360.0;
  }
  _sumDeltaAz += deltaAz;

  double deltaEl = el - _prevElForRate;
  if (deltaEl > 180.0) {
    deltaEl -= 360.0;
  } else if (deltaEl < -180.0) {
    deltaEl += 360.0;
  }
  _sumDeltaEl += deltaEl;

  struct timeval now;
  gettimeofday(&now, NULL);
  
  double deltaTime =
    (double) (now.tv_sec - _prevTimeForRate.tv_sec) +
    (double) (now.tv_usec - _prevTimeForRate.tv_usec) / 1.0e6;
  
  if (deltaTime > _params.antenna_rate_check_interval) {
    _rateAz = _sumDeltaAz / deltaTime;
    _rateEl = _sumDeltaEl / deltaTime;
    _prevTimeForRate = now;
    _sumDeltaAz = 0.0;
    _sumDeltaEl = 0.0;
  }

  _prevElForRate = el;
  _prevAzForRate = az;

}

///////////////////////////
// compute message rate

void SpolAngles2Fmq::_computeMessageRate()
  
{
  
  // compute latency in secs
  
  double angleTime = (double) _angles.time_secs_utc + _angles.time_nano_secs / 1.0e9;
  double latencySecs = _now - angleTime;
  
  // print message rate in debug mode
  
  if (_now - _prevTimeMsgRate > 1) {
    double dt = _now - _prevTimeMsgRate;
    double msgRate = 1000.0 / dt;
    _prevTimeMsgRate = _now;
    double rateError = msgRate / _params.angle_read_frequency_hz;
    _usecsSleepOnRead = (int) floor(_usecsSleepOnRead * rateError + 0.5);
    if (_params.debug) {
      fprintf(stderr, "el, az, elRate, azRate, msg/sec, error, usecsSleep, latencySecs: "
              "%g, %g, %g, %g, %g, %g, %d, %g\n",
              _angles.elevation, _angles.azimuth,
              _rateEl, _rateAz,
              msgRate, rateError, _usecsSleepOnRead, latencySecs);
    }
  }

}
  
///////////////////////////////////////
// check antenna motion status

void SpolAngles2Fmq::_checkMotionStatus()

{
  
  double timeSinceCheck = _now - _prevTimeMotionCheck;

  if (timeSinceCheck >= _params.antenna_motion_check_interval) {
    
    if ((fabs(_angles.elevation - _prevElForMotion) > 1) ||
        (fabs(_angles.azimuth - _prevAzForMotion) > 1)) {

      // antenna is in motion

      _prevElForMotion = _angles.elevation;
      _prevAzForMotion = _angles.azimuth;
      _stopTime = _now;
      
    }
    
    _writeMotionStatusFile();

    _prevTimeMotionCheck = _now;
    
  }

}

///////////////////////////////////////
// write the motion status file

void SpolAngles2Fmq::_writeMotionStatusFile()

{

  // set message

  double timeStopped = _now - _stopTime;
  char message[1024];
  
  if (timeStopped < _params.antenna_motion_warning_secs) {

    sprintf(message, "0 AntennaStatus - OK");

  } else if (timeStopped < _params.antenna_motion_critical_secs) {

    sprintf(message,
            "1 AntennaStatus - WARN Antenna stopped for %d seconds",
            (int) timeStopped);

  } else {

    sprintf(message,
            "2 AntennaStatus - CRITICAL Antenna stopped for %d seconds",
            (int) timeStopped);
    
  }

  if (_params.debug) {
    cerr << "Antenna motion status message:" << endl;
    cerr << "  " << message << endl;
  }
  
  // open motion status file

  FILE *out;
  if ((out = fopen(_params.antenna_status_file_path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SpolAngles2Fmq::_writeMotionStatusFile()" << endl;
    cerr << "  Cannot open antenna status file: "
         << _params.antenna_status_file_path << endl;
    cerr << strerror(errNum);
    return;
  }

  // write message

  fprintf(out, "%s\n", message);

  // close file

  fclose(out);
  
}

//////////////////////////////////////////////////
// Run in monitor mode

int SpolAngles2Fmq::_runMonitorMode()
{

  PMU_auto_register("_runMonitorMode");
  
  while (true) {

    // get time

    struct timeval tv;
    gettimeofday(&tv, NULL);
    _now = (double) tv.tv_sec + tv.tv_usec / 1.0e6;

    if (_monitorGetNextAngle()) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Cannot angle read FMQ: " << _params.output_fmq_path << endl;
      cerr << _monitorFmq.getErrStr() << endl;
      return -1;
    }
  
    _writeMonitor();

  } // while

  return 0;
  
}

///////////////////////////////////////
// open the FMQ for monitoring
// returns 0 on success, -1 on failure

int SpolAngles2Fmq::_openMonitorFmq()

{
  
  if (_monitorFmq.initReadBlocking
      (_params.output_fmq_path,
       _progName.c_str(),
       _params.debug >= Params::DEBUG_EXTRA, // set debug?
       Fmq::END)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize FMQ: " << _params.output_fmq_path << endl;
    cerr << _monitorFmq.getErrStr() << endl;
    return -1;
  }
  
  _monitorMsg.clearAll();
  _monitorMsg.setType(0);

  return 0;

}

/////////////////////////////////////////////////
// get next antenna angle message for monitoring
//
// Returns 0 on success, -1 on failure

int SpolAngles2Fmq::_monitorGetNextAngle()
  
{
  
  PMU_auto_register("Get next angle part");
  
  // check we have an open FMQ
  
  if (!_monitorFmq.isOpen()) {
    if (_openMonitorFmq()) {
      return -1;
    }
  }
  
  if (_monitorPos >= _monitorNParts) {
    
    // we need a new message
    
    if (_monitorFmq.readMsgBlocking()) {
      cerr << "ERROR - SpolAngles2Fmq::_monitorNextAngle" << endl;
      cerr << "  Cannot read message from FMQ" << endl;
      cerr << "  Fmq: " << _params.output_fmq_path << endl;
      cerr << _monitorFmq.getErrStr() << endl;
      _monitorFmq.closeMsgQueue();
      return -1;
    }

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "--->> Got angle FMQ message" << endl;
    }
    
    // disassemble the message
    
    const void *msg = _monitorFmq.getMsg();
    int len = _monitorFmq.getMsgLen();
    if (_monitorMsg.disassemble(msg, len) == 0) {
      _monitorPos = 0;
      _monitorNParts = _monitorMsg.getNParts();
    }
    
  } // while
  
  _monitorPart = _monitorMsg.getPart(_monitorPos);
  _monitorPos++;
  
  if (_monitorPart->getType() != SPOL_SHORT_ANGLE_ID ||
      _monitorPart->getLength() != sizeof(spol_short_angle_t)) {
    cerr << "ERROR - SpolAngles2Fmq::_getNextAngle" << endl;
    cerr << "  Bad angle message part" << endl;
    fprintf(stderr, "  id: 0x%x\n", _monitorPart->getType());
    cerr << "  length: " << _monitorPart->getLength() << endl;
    return -1;
  }

  // copy to struct

  memcpy(&_angles, _monitorPart->getBuf(), sizeof(spol_short_angle_t));
  spol_swap(_angles);

  // compute rates
  
  _computeAngularRates();
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "========== latest antenna angle ==============" << endl;
    spol_print(stderr, _angles);
    cerr << "==============================================" << endl;
  }

  return 0;

}

///////////////////////////////////////
// write monitoring info to stdout

void SpolAngles2Fmq::_writeMonitor()

{
  
  double timeSinceMonitor = _now - _prevTimeMonitor;
  if (timeSinceMonitor >= _params.monitor_update_interval) {
    
    DateTime atime(_angles.time_secs_utc);
    int msecs = _angles.time_nano_secs / 1000000;
    
    fprintf(stdout,
            "%16s %10s %10s %10s %10s"
            "%12s %14s\n",
            "SPOL S2D angles:", "Elevation", "Azimuth", "ElRate", "AzRate", "Date", "Time");
    
    if (_params.monitor_update_interval < 0.99) {
      fprintf(stdout,
              "%16s %10.2f %10.2f %10.2f %10.2f "
              "  %.4d/%.2d/%.2d   %.2d:%.2d:%.2d.%.3d\n",
              "", _angles.elevation, _angles.azimuth,
              _rateEl, _rateAz,
              atime.getYear(), atime.getMonth(), atime.getDay(),
              atime.getHour(), atime.getMin(), atime.getSec(),
              msecs);
    } else {
      fprintf(stdout,
              "%16s %10.2f %10.2f %10.2f %10.2f "
              "  %.4d/%.2d/%.2d       %.2d:%.2d:%.2d\n",
              "", _angles.elevation, _angles.azimuth,
              _rateEl, _rateAz,
              atime.getYear(), atime.getMonth(), atime.getDay(),
              atime.getHour(), atime.getMin(), atime.getSec());
    }

    fflush(stdout);

    _prevTimeMonitor = _now;
    
  }

}

