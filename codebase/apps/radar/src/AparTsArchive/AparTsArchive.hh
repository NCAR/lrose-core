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
// AparTsArchive.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2020
//
///////////////////////////////////////////////////////////////
//
// AparTsArchive reads data from TsTcpServer in TS TimeSeries format
// and writes it to files in TsArchive format
//
////////////////////////////////////////////////////////////////

#ifndef AparTsArchive_HH
#define AparTsArchive_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>
#include <radar/AparTsInfo.hh>
#include <radar/AparTsPulse.hh>
#include <radar/AparTsReader.hh>
#include <Fmq/Fmq.hh>

using namespace std;

////////////////////////
// This class

class AparTsArchive {
  
public:

  // constructor

  AparTsArchive(int argc, char **argv);

  // destructor
  
  ~AparTsArchive();

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
  AparTsReader *_pulseReader;
  time_t _startTime;
  time_t _exitTime;

  // current scan mode

  apar_ts_scan_mode_t _currentScanMode;
  // apar_ts_scan_segment_t _scanPrev;
  
  // sector information to determine if we need a new file
  
  bool _needNewFile;
  int _nPulsesFile; // number of pulses in current file

  // output file

  FILE *_out;
  time_t _outputTime;
  string _outputDir;
  string _outputName;
  string _relPath;

  // debug print loop count
  
  si64 _nPulses;
  si64 _prevPulseSeqNum;
  si32 _prevPulseSweepNum;

  // functions
  
  int _checkNeedNewFile(const AparTsPulse &pulse);
  int _handlePulse(AparTsPulse &pulse);
  int _openNewFile(const AparTsPulse &pulse);
  int _closeFile();
  
};

#endif
