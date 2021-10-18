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
// FmqTest.hh
//
// FmqTest object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2021
//
///////////////////////////////////////////////////////////////
//
// FmqTest reads an input FMQ and copies the contents unchanged to an 
// output FMQ. It is useful for reading data from a remote queue and 
// copying it to a local queue. The clients can then read the local 
// queue rather than all access the remote queue.
//
///////////////////////////////////////////////////////////////////////

#ifndef FmqTest_H
#define FmqTest_H

#include <string>
#include <dataport/port_types.h>
#include <toolsa/TaArray.hh>
#include <Fmq/DsFmq.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

class Socket;
class MsgLog;

////////////////////////
// This class

class FmqTest {
  
public:

  // constructor

  FmqTest (int argc, char **argv);

  // destructor
  
  ~FmqTest();

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
  MsgLog *_msgLog;

  DsFmq _inputFmq;
  TaArray<DsFmq> _outputFmqs_;
  DsFmq *_outputFmqs;
  time_t _prevTimeForOpen;

  int _run();
  void _openOutputFmqs();

};

#endif

