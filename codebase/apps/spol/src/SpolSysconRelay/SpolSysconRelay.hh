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
// SpolSysconRelay.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////
//
// SpolSysconRelay reads status packets from the Syscon server,
// and relays relevant information to SPOL processes.
//
////////////////////////////////////////////////////////////////

#ifndef SpolSysconRelay_HH
#define SpolSysconRelay_HH

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/Socket.hh>
#include <radar/iwrf_data.h>
#include <radar/iwrf_user_interface.h>
#include <radar/iwrf_functions.hh>
#include <radar/rsm_functions.hh>
#include <radar/syscon_to_spol.h>
#include <Fmq/DsFmq.hh>

class OpsInfo;
class RsmInfo;

using namespace std;

////////////////////////
// This class

class SpolSysconRelay {
  
public:

  // constructor

  SpolSysconRelay(int argc, char **argv);

  // destructor
  
  ~SpolSysconRelay();

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
  OpsInfo *_opsInfo;
  RsmInfo *_rsmInfo;

  // output FMQ
  
  DsFmq _outputFmq;

  // monitor FMQ
  
  DsFmq _monitorFmq;

  // heartbeat function for reading

  Socket::heartbeat_t _heartBeatFunc;

  // functions

  int _run();
  int _runMonitor();
  int _openOutputFmq();
  int _openMonitorFmq();

};

#endif
