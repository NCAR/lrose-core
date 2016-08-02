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
// TsTcp2File.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// TsTcp2File reads data from TsTcpServer in RVP8 TimeSeries format
// and writes it to files in TsArchive format
//
////////////////////////////////////////////////////////////////

#ifndef TsTcp2File_H
#define TsTcp2File_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include <toolsa/Socket.hh>
#include <rapformats/rvp8_ts_api.h>
using namespace std;

////////////////////////
// This class

class TsTcp2File {
  
public:

  // constructor

  TsTcp2File(int argc, char **argv);

  // destructor
  
  ~TsTcp2File();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:
  
  static const int INFO_ID = 77000;
  static const int PULSE_ID = 77001;

  string _progName;
  Args _args;

  // aquisition mode - got new info?

  si32 _iAqModePrev;

  // sector information to determine if we need a new file

  bool _needNewFile;
  int _nPulsesFile; // number of pulses in current file
  double _sectorWidth; // width of sector (file)
  int _currentSector; // which sector are we in?

  // output file

  FILE *_out;
  bool _fileWritten;

  // Pulse info and header

  rvp8_ops_info_t _pulseInfo;

  // debug print loop count
  
  int _nPulses;
  ui32 _prevSeqNum;
  double _prevAz;

  // functions

  int _readFromServer(Socket &sock);

  int _handleInfo(int len, const void *buf);
  int _handlePulse(int len, const void *buf);

  int _checkNewFile(const rvp8_pulse_hdr_t &pulseHeader);
  int _openNewFile(const rvp8_pulse_hdr_t &pulseHeader);
  int _closeFile();

  int _writePulseInfo();
  int _writePulseHeader(const rvp8_pulse_hdr_t &pulseHeader);
  int _writeIQ(void *iqPacked, int nBytesPacked);

  int _makedir(const char *path);
  int _makedir_recurse(const char *path);
  
};

#endif
