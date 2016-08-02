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
// TsSmartSave.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// TsSmartSave reads data from TsTcpServer in TS TimeSeries format
// and writes it to files in TsArchive format
//
////////////////////////////////////////////////////////////////

#ifndef TsSmartSave_H
#define TsSmartSave_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>
#include <euclid/SunPosn.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include <Fmq/Fmq.hh>

using namespace std;

////////////////////////
// This class

class TsSmartSave {
  
public:

  // constructor

  TsSmartSave(int argc, char **argv);

  // destructor
  
  ~TsSmartSave();

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
  IwrfTsReader *_pulseReader;
  time_t _startTime;
  time_t _exitTime;

  // current scan mode

  iwrf_scan_mode_t _currentScanMode;

  // previous values - to check for changes

  iwrf_scan_segment_t _scanPrev;
  iwrf_ts_processing_t _procPrev;
  bool _transPrev;

  // sector information to determine if we need a new file
  
  bool _needNewFile;
  int _nPulsesFile; // number of pulses in current file
  double _sectorWidth; // width of sector (file)
  int _currentSector; // which sector are we in?

  // pointing mode

  typedef enum {
    MODE_NORMAL,
    MODE_VERT_POINT,
    MODE_SUN_SCAN,
    MODE_FLAG_FILE,
    MODE_STATIONARY
  } pointing_mode_t;
  
  pointing_mode_t _currentPointingMode;
  bool _isStationary;
  
  double _timeEnterVertPoint;
  double _timeEnterSunScan;

  SunPosn _sunPosn;
  time_t _timePrintSunPosn;

  // movement check
  
  bool _isMoving;
  double _moveCheckAz;
  double _moveCheckEl;
  time_t _moveCheckTime;
  
  // output file

  FILE *_out;
  time_t _outputTime;
  string _outputDir;
  string _outputName;
  string _relPath;

  // debug print loop count
  
  si64 _nPulses;
  si64 _prevSeqNum;
  double _prevAz;

  // functions
  
  int _checkNeedNewFile(const IwrfTsPulse &pulse);
  int _handlePulse(IwrfTsPulse &pulse);
  pointing_mode_t _getPointingMode(time_t ptime, double el, double az);
  int _openNewFile(const IwrfTsPulse &pulse);
  int _closeFile();
  bool _moving(time_t ptime, double el, double az);
  
};

#endif
