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
/////////////////////////////////////////////////////////////
// SpolAngles2Fmq.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////
//
// SpolAngles2Fmq reads angle data from the S2D processor via
// UDP, and writes them out to an FMQ.
//
////////////////////////////////////////////////////////////////

#ifndef SpolAngles2Fmq_HH
#define SpolAngles2Fmq_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <Fmq/DsFmq.hh>
#include <didss/DsMessage.hh>
#include <toolsa/Socket.hh>
#include <radar/spol_angles.hh>

using namespace std;

////////////////////////
// This class

class SpolAngles2Fmq {
  
public:

  // constructor

  SpolAngles2Fmq(int argc, char **argv);

  // destructor
  
  ~SpolAngles2Fmq();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  Args _args;
  char *_paramsPath;
  Params _params;

  // socket device

  Socket _sock;

  // output message and FMQ
  // we combine a number of angles into a message before
  // writing to the FMQ

  DsFmq _fmq;
  DsMessage _msg;
  int _msgCount;

  // status

  double _now;
  spol_short_angle_t _angles;
  
  // computing message rate
  
  double _prevTimeMsgRate;
  int _usecsSleepOnRead;

  // monitor mode

  double _prevTimeMonitor;
  DsFmq _monitorFmq;
  DsMessage _monitorMsg;
  DsMsgPart *_monitorPart;
  int _monitorPos, _monitorNParts;

  // checking for antenna motion

  double _prevTimeMotionCheck;
  double _stopTime;
  double _prevAzForMotion;
  double _prevElForMotion;

  // computing rate

  struct timeval _prevTimeForRate;
  double _prevAzForRate;
  double _prevElForRate;
  double _sumDeltaAz;
  double _sumDeltaEl;
  double _rateAz;
  double _rateEl;

  // methods

  int _runServerMode();
  int _openSocket();
  int _readFromServer();
  int _writeToFmq();

  void _computeAngularRates();
  void _computeMessageRate();
  void _checkMotionStatus();
  void _writeMotionStatusFile();

  int _runMonitorMode();
  int _openMonitorFmq();
  int _monitorGetNextAngle();
  void _writeMonitor();

};

#endif
