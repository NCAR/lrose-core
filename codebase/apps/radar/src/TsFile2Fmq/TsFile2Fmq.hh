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
// TsFile2Fmq.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2009
//
///////////////////////////////////////////////////////////////
//
// TsFile2Fmq reads raw time-series data from a file.
// It saves the time series data out to a file message queue (FMQ),
// which can be read by multiple clients. Its purpose is mainly
// for simulation and debugging of time series operations.
//
////////////////////////////////////////////////////////////////

#ifndef TsFile2Fmq_H
#define TsFile2Fmq_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <Fmq/DsFmq.hh>
#include <toolsa/MemBuf.hh>
#include <didss/DsMessage.hh>
#include <radar/IwrfTsReader.hh>

using namespace std;

////////////////////////
// This class

class TsFile2Fmq {
  
public:

  // constructor

  TsFile2Fmq(int argc, char **argv);

  // destructor
  
  ~TsFile2Fmq();

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
  IwrfTsReader *_reader;
  
  // output message and FMQ
  // we conbine a number of packets into a message before
  // writing to the FMQ

  DsFmq _fmq;
  DsMessage _msg;
  
  // Pulse count

  int _inputPulseCount;
  int _outputPulseCount;

  // time offset if applicable

  int _timeOffset;

  // functions

  int _runSim();
  int _processPulse(IwrfTsPulse *pulse);
  int _processPulseNormal(IwrfTsPulse *pulse);
  int _processPulseSimStaggeredPrt(IwrfTsPulse *pulse);
  int _processPulseSimSelectInterval(IwrfTsPulse *pulse);
  int _addSecondChannel(IwrfTsPulse *pulse);
  void _setPulseTime(IwrfTsPulse *pulse);
  void _convertIqEncoding(IwrfTsPulse *pulse);
  int _writeToFmq();

};

#endif
