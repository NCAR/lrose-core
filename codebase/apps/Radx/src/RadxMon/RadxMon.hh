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
// RadxMon object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2012
//
///////////////////////////////////////////////////////////////
//
// RadxMon prints out radar moments in a variety of ways
//
///////////////////////////////////////////////////////////////////////

#ifndef RadxMon_HH
#define RadxMon_HH

#include <string>
#include <Radx/Radx.hh>
#include <toolsa/ServerSocket.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

class IwrfMomReader;
class RadxRay;

////////////////////////
// This class

class RadxMon {
  
public:

  // constructor

  RadxMon (int argc, char **argv);

  // destructor
  
  ~RadxMon();

  // run 

  int Run();
  
  // data members
  
  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  IwrfMomReader *_reader;

  int _printCount;
  int _summaryCount;
  time_t _lastPrintTime;

  // checking for missing beams
  
  int _numGetDsBeamFails;
  const static int _maxNumGetDsBeamFails = 120;
  Radx::SweepMode_t _prevSweepMode;
  int _prevVolNum;
  int _prevSweepNum;
  double _prevAz;
  double _prevEl;
  
  // server mode

  ServerSocket _server;
  Socket *_sock;

  // antenna rate

  bool _azRateInitialized;
  double _prevTimeForAzRate;
  double _prevAzForRate;
  double _azRate;
  
  bool _elRateInitialized;
  double _prevTimeForElRate;
  double _prevElForRate;
  double _elRate;

  double _nSecsForRate;

  // methods

  int _runPrint();
  int _runServer();
  
  void _printEvents();
  void _printRay(const RadxRay *ray);

  void _printSummary(const RadxRay *ray);
  void _printGate(const RadxRay *ray);
  void _printPowerAndFreq(const RadxRay *ray);
  void _checkForMissingBeams(const RadxRay *ray);

  int _serverInit();
  int _serviceRequest(const RadxRay *ray);
  int _readMessage(string &message);
  int _handleMessage(const string &message,
                     string &reply);
  int _createStatusReply(const RadxRay *ray, string &reply);
  void _createErrorReply(string &reply, const string &cause);
  int _sendReply(const string &reply);
  
  double _computeAzRate(const RadxRay *ray);
  double _computeElRate(const RadxRay *ray);

};

#endif

