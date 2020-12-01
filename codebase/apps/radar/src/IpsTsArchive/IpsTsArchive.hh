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
// IpsTsArchive.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2020
//
///////////////////////////////////////////////////////////////
//
// Support for Independent Pulse Sampling.
//
// IpsTsArchive reads data from TsTcpServer in TS TimeSeries format
// and writes it to files in TsArchive format
//
////////////////////////////////////////////////////////////////

#ifndef IpsTsArchive_HH
#define IpsTsArchive_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>
#include <radar/IpsTsInfo.hh>
#include <radar/IpsTsPulse.hh>
#include <radar/IpsTsReader.hh>
#include <Fmq/Fmq.hh>

using namespace std;

////////////////////////
// This class

class IpsTsArchive {
  
public:

  // constructor

  IpsTsArchive(int argc, char **argv);

  // destructor
  
  ~IpsTsArchive();

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
  IpsTsReader *_pulseReader;
  time_t _startTime;
  time_t _exitTime;


  // output file

  FILE *_out;
  time_t _outputTime;
  string _outputDir;
  string _outputName;
  string _relPath;
  int _nPulsesFile; // number of pulses in current file

  // debug print loop count
  
  si64 _nPulsesProcessed;
  si64 _prevPulseSeqNum;
  si32 _prevSweepNum;

  // functions
  
  int _checkNeedNewFile(const IpsTsPulse &pulse);
  int _handlePulse(IpsTsPulse &pulse);
  int _openNewFile(const IpsTsPulse &pulse);
  int _closeFile();
  
};

#endif
