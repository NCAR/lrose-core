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
///////////////////////////////////////////////////////////////////////////
// KbandBlanking.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2011
//
///////////////////////////////////////////////////////////////////////////
//
// KbandBlanking reads angles from the SPOL angle FMQ, and determines
// whether the azimith falls into a blanking region or not. It then
// issues blanking/not blanking commands to the kadrx. kadrx will then
// disable the transmit triggers while blanked.
//
///////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cmath>
#include <sys/stat.h>
#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <didss/DsMsgPart.hh>
#include "KbandBlanking.hh"

using namespace std;

// Constructor

KbandBlanking::KbandBlanking(int argc, char **argv)
  
{

  isOK = true;
  
  _angleNParts = 0;
  _anglePos = 0;
  spol_init(_latestAngle);
  spol_init(_prevAngle);
  _blanked = false;
  _blankedLut = NULL;
  _lowerElLimit = NULL;
  _upperElLimit = NULL;
  _prevRpcTime = 0;
  _rpcClient = NULL;
  
  // set programe name
 
  _progName = "KbandBlanking";
  
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

  // compute the blanking lookup table

  _computeBlankingLut();

  // instantiate RPC client

  _rpcClient = new XmlRpc::XmlRpcClient(_params.rpc_server_host, _params.rpc_server_port);
  
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  return;
  
}

// destructor

KbandBlanking::~KbandBlanking()

{

  if (_blankedLut) {
    delete[] _blankedLut;
  }

  if (_lowerElLimit) {
    delete[] _lowerElLimit;
  }

  if (_upperElLimit) {
    delete[] _upperElLimit;
  }

  if (_rpcClient) {
    delete _rpcClient;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int KbandBlanking::Run ()
{

  if (_params.simulate_mode) {
    return _runSim();
  } else {
    return _runOps();
  }

}
  
//////////////////////////////////////////////////
// Run in operational mode

int KbandBlanking::_runOps()
{

  PMU_auto_register("_runOps");
  int iret = 0;
  
  while (true) {
    
    PMU_auto_register("Getting angle");

    // get next angle
    
    if (_getNextAngle()) {
      iret = -1;
      umsleep(100);
      continue;
    }

    double az = _latestAngle.azimuth;
    int index = (int) (az / _azRes + 0.5);
    if (index < 0) {
      index = 0;
    } else if (index >= _nLut) {
      index = _nLut - 1;
    }

    double el = _latestAngle.elevation;
    bool blanked = (_blankedLut[index] != 0);
    if (el < _lowerElLimit[index] || el > _upperElLimit[index]) {
      blanked = false;
    }

    double now = _getCurrentTime();
    double secsSincePrevRpc = now - _prevRpcTime;
    
    if (secsSincePrevRpc >= _params.rpc_max_interval) {
      
      if (blanked) {
        _setBlankingOn(now, el, az, false);
      } else {
        _setBlankingOff(now, el, az, false);
      }
      _blanked = blanked;
      _prevRpcTime = now;
      
    } else {
      
      if (blanked != _blanked) {
        
        if (blanked) {
          _setBlankingOn(now, el, az, true);
        } else {
          _setBlankingOff(now, el, az, true);
        }
        _blanked = blanked;
        _prevRpcTime = now;

      } // if (blanked != _blanked)

    } // if (secsSincePrevRpc >= _params.rpc_max_interval) 

  } // while(true)

  return iret;
  
}

//////////////////////////////////////////////////
// Run in simulate mode

int KbandBlanking::_runSim()
{

  PMU_auto_register("_runSim");
  
  // start with blanking off

  double now = _getCurrentTime();
  double expire = now + _params.sim_mode_blanking_off_time_secs;
  bool blanked = false;
  _setBlankingOff(now, -9999, -9999, true);
  PMU_auto_register("Setting blanking off");
  
  // loop, alternating states with delays between

  while (true) {
    
    now = _getCurrentTime();
    if (now < expire) {
      umsleep(100);
      PMU_auto_register("Zzzz ....");
      continue;
    }
    
    if (blanked) {
      
      blanked = false;
      _setBlankingOff(now, -9999, -9999, true);
      PMU_auto_register("Setting blanking off");
      expire = now + _params.sim_mode_blanking_off_time_secs;

    } else {

      blanked = true;
      _setBlankingOn(now, -9999, -9999, true);
      PMU_auto_register("Setting blanking on");
      expire = now + _params.sim_mode_blanking_on_time_secs;

    }

  } // while

  return 0;

}

///////////////////////////////////////
// open the angle FMQ
// returns 0 on success, -1 on failure

int KbandBlanking::_openAngleFmq()

{
  
  Fmq::openPosition initPos = Fmq::END;
  
  if (_angleFmq.initReadBlocking
      (_params.angle_fmq_path,
       _progName.c_str(),
       _params.debug >= Params::DEBUG_EXTRA, // set debug?
       initPos)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot initialize FMQ: " << _params.angle_fmq_path << endl;
    cerr << _angleFmq.getErrStr() << endl;
    return -1;
  }

  _angleMsg.clearAll();
  _angleMsg.setType(0);

  return 0;

}

/////////////////////////////////////////////////
// get next antenna angle message
//
// Returns 0 on success, -1 on failure

int KbandBlanking::_getNextAngle()
  
{

  PMU_auto_register("Get next angle part");
  
  // check we have an open FMQ
  
  if (!_angleFmq.isOpen()) {
    if (_openAngleFmq()) {
      return -1;
    }
  }
  
  if (_anglePos >= _angleNParts) {
    
    // we need a new message
    
    if (_angleFmq.readMsgBlocking()) {
      cerr << "ERROR - KbandBlanking::_getNextAngle" << endl;
      cerr << "  Cannot read message from FMQ" << endl;
      cerr << "  Fmq: " << _params.angle_fmq_path << endl;
      cerr << _angleFmq.getErrStr() << endl;
      _angleFmq.closeMsgQueue();
      return -1;
    }

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "--->> Got angle FMQ message" << endl;
    }
    
    // disassemble the message
    
    const void *msg = _angleFmq.getMsg();
    int len = _angleFmq.getMsgLen();
    if (_angleMsg.disassemble(msg, len) == 0) {
      _anglePos = 0;
      _angleNParts = _angleMsg.getNParts();
    }
    
  } // if (_anglePos >= _angleNParts)

  _anglePart = _angleMsg.getPart(_anglePos);
  _anglePos++;

  if (_anglePart->getType() != SPOL_SHORT_ANGLE_ID ||
      _anglePart->getLength() != sizeof(spol_short_angle_t)) {
    cerr << "ERROR - KbandBlanking::_getNextAngle" << endl;
    cerr << "  Bad angle message part" << endl;
    fprintf(stderr, "  id: 0x%x\n", _anglePart->getType());
    cerr << "  length: " << _anglePart->getLength() << endl;
    return -1;
  }

  memcpy(&_prevAngle, &_latestAngle, sizeof(spol_short_angle_t));
  memcpy(&_latestAngle, _anglePart->getBuf(), sizeof(spol_short_angle_t));

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "========== latest antenna angle ==============" << endl;
    spol_print(stderr, _latestAngle);
    cerr << "==============================================" << endl;
  }

  return 0;

}

/////////////////////////////////////////////////
// Compute the azimuth lookup table for blanking

void KbandBlanking::_computeBlankingLut()
  
{

  _nLut = (int) (360.0 / _params.azimuth_lut_resolution + 0.5);
  _azRes = 360.0 / _nLut;

  _blankedLut = new int[_nLut];
  _lowerElLimit = new double[_nLut];
  _upperElLimit = new double[_nLut];

  memset(_blankedLut, 0, _nLut * sizeof(int));
  memset(_lowerElLimit, 0, _nLut * sizeof(double));
  memset(_upperElLimit, 0, _nLut * sizeof(double));


  for (int isec = 0; isec < _params.blanking_sectors_n; isec++) {

    string name = _params._blanking_sectors[isec].name;
    double startAz = _params._blanking_sectors[isec].counter_clockwise_azimuth_limit_deg;
    double endAz = _params._blanking_sectors[isec].clockwise_azimuth_limit_deg;
    double lowerEl = _params._blanking_sectors[isec].lower_elevation_limit_deg;
    double upperEl = _params._blanking_sectors[isec].upper_elevation_limit_deg;
    
    if (startAz > endAz) {
      startAz -= 360.0;
    }
    
    for (double az = startAz; az <= endAz; az += _azRes) {

      int index = (int) floor(az / _azRes + 0.5);
      if (index < 0) {
        index += _nLut;
      }
      if (index >= _nLut) {
        index = _nLut - 1;
      }

      _blankedLut[index] = isec + 1;
      _lowerElLimit[index] = lowerEl;
      _upperElLimit[index] = upperEl;
      
      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "Blanking name, az, elLower, elUpper, index: "
             << name << ", "
             << az << ", "
             << lowerEl << ", "
             << upperEl << ", "
             << index << endl;
      }
      
    }

  } // isec

}

/////////////////////////////////////////////////
// Get current time as a double

double KbandBlanking::_getCurrentTime()
  
{

  struct timeval tv;
  gettimeofday(&tv, NULL);
  double dtime = (double) tv.tv_sec + (double) tv.tv_usec / 1.0e6;
  return dtime;

}

//////////////////////////////////////////////////////////////////////////
// communicate blanking state to the server

int KbandBlanking::_setBlankingOn(double time,
                                  double el,
                                  double az,
                                  bool changingState)

{

  if (_params.debug) {
    if (changingState) {
      cerr << "==>> Changing blanking to ON, el, az: " << el << ", " << az << endl;
    } else {
      cerr << "==>> Confirming blanking ON, el, az: " << el << ", " << az << endl;
    }
  }

  // set calling parameters: time, el, az

  XmlRpc::XmlRpcValue params;
  params[0] = time;
  params[1] = el;
  params[2] = az;

  // make RPC call

  XmlRpc::XmlRpcValue result;
  if (!_rpcClient->execute("setBlankingOn", params, result) ||
      _rpcClient->isFault()) {
    cerr << "ERROR - KbandBlanking::_setBlankingOn" << endl;
    cerr << "  Executing RPC for setBlankingOn()" << endl;
    cerr << "  host, port: "
         << _params.rpc_server_host << ", "
         << _params.rpc_server_port << endl;
    return -1;
  }

  int iret = result;
  if (iret != 0) {
    cerr << "ERROR - KbandBlanking::_setBlankingOn" << endl;
    cerr << "  Error return from kadrx: " << iret << endl;
    cerr << "  Time: " << DateTime::strm((time_t) time) << endl;
    cerr << "  El: " << el << endl;
    cerr << "  Az: " << az << endl;
  }

  if (iret) {
    return -1;
  } else {
    return 0;
  }

}

int KbandBlanking::_setBlankingOff(double time,
                                   double el,
                                   double az,
                                   bool changingState)

{

  if (_params.debug) {
    if (changingState) {
      cerr << "==>> Changing blanking to OFF, el, az: " << el << ", " << az << endl;
    } else {
      cerr << "==>> Confirming blanking OFF, el, az: " << el << ", " << az << endl;
    }
  }

  // set calling parameters: time, el, az

  XmlRpc::XmlRpcValue params;
  params[0] = time;
  params[1] = el;
  params[2] = az;

  // make RPC call

  XmlRpc::XmlRpcValue result;
  if (!_rpcClient->execute("setBlankingOff", params, result) ||
      _rpcClient->isFault()) {
    cerr << "ERROR - KbandBlanking::_setBlankingOff" << endl;
    cerr << "  Executing RPC for setBlankingOff()" << endl;
    cerr << "  host, port: "
         << _params.rpc_server_host << ", "
         << _params.rpc_server_port << endl;
    return -1;
  }

  int iret = result;
  if (iret != 0) {
    cerr << "ERROR - KbandBlanking::_setBlankingOff" << endl;
    cerr << "  Error return from kadrx: " << iret << endl;
    cerr << "  Time: " << DateTime::strm((time_t) time) << endl;
    cerr << "  El: " << el << endl;
    cerr << "  Az: " << az << endl;
  }

  if (iret) {
    return -1;
  } else {
    return 0;
  }

  return 0;

}

