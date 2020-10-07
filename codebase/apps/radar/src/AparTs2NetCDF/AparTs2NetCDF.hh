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
// AparTs2NetCDF.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2011
//
///////////////////////////////////////////////////////////////
//
// AparTs2NetCDF reads time-series data and saves it out as netCDF
//
////////////////////////////////////////////////////////////////

#ifndef AparTs2NetCDF_H
#define AparTs2NetCDF_H

#include <string>
#include <vector>
#include <deque>
#include <cstdio>
#include <Ncxx/Ncxx.hh>
#include <Ncxx/NcxxFile.hh>

#include "Args.hh"
#include "Params.hh"
#include <radar/AparTsInfo.hh>
#include <radar/AparTsPulse.hh>
#include <radar/AparTsReader.hh>
#include <rapformats/DsRadarCalib.hh>
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>

using namespace std;

////////////////////////
// This class

class AparTs2NetCDF {
  
public:

  // constructor

  AparTs2NetCDF (int argc, char **argv);

  // destructor
  
  ~AparTs2NetCDF();

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

  // read in pulses
  
  AparTsReader *_pulseReader;
  deque<AparTsPulse *> _pulses;
  
  // calib for override

  DsRadarCalib _calOverride;

  // debug print loop count
  
  si64 _nPulsesRead;

  // previous values - to check for changes

  int _prevPulseSweepNum;

  // geometry
  
  int _nGatesMax;
  double _startRangeM;
  double _gateSpacingM;

  size_t _nTimes;
  time_t _startTime;

  // bool _alternatingMode;

  double _startAz;
  double _startEl;
  double _startFixedAngle;
  apar_ts_scan_mode_t _startScanMode;

  time_t _pulseTimeSecs;
  double _pulseTime;

  double _el;
  double _az;
  double _prt;

  // meta data arrays

  vector<int> _nGatesRay;

  vector<double> _timeRay, _dtimeRay;
  vector<float> _elRay, _azRay, _fixedAngleRay;
  vector<float> _prtRay;
  vector<float> _pulseWidthRay;

  vector<int64_t> _pulseSeqNumRay;
  vector<int64_t> _dwellSeqNumRay;

  vector<int> _beamNumInDwellRay;
  vector<int> _visitNumInBeamRay;
  vector<int> _scanModeRay;
  vector<int> _sweepNumRay;
  vector<int> _volNumRay;
  
  vector<int> _hvFlagRay;
  vector<int> _chanIsCopolRay;

  // IQ data

  MemBuf _II, _QQ;
  MemBuf _pulseII, _pulseQQ;

  // output file

  string _outputName;
  string _outputPath;
  string _tmpPath;

  // functions
  
  bool _checkReadyToWrite(const AparTsPulse *pulse, bool &endOfFile);
  void _prepareToWrite();
  void _computeNGatesMax();
  void _checkForMissingPulses();
  void _clearPulseQueue();

  int _writeFile();
  int _writeFileTmp();
  int _computeOutputFilePaths();
  
  void _addGlobAtt(NcxxFile &out);
  
  int _writeBaseTimeVars(NcxxFile &file);
  int _writeTimeDimVars(NcxxFile &file, NcxxDim &timeDim);
  int _writeRangeVar(NcxxFile &file, NcxxDim &rangeDim);
    
  int _addAttr(NcxxVar &var,
               const string &name,
               const string &val);
  
  int _addAttr(NcxxVar &var,
               const string &name,
               double val);

  int _addAttr(NcxxVar &var,
               const string &name,
               int val);

  int _addVar(NcxxFile &file,
              NcxxVar &var,
              NcxxType ncType,
              const string &name,
              const string &standardName,
              const string &units = "");

  int _addVar(NcxxFile &file,
              NcxxVar &var,
              NcxxType ncType,
              NcxxDim &dim, 
              const string &name,
              const string &standardName,
              const string &units = "");

  int _addVar(NcxxFile &file,
              NcxxVar &var,
              NcxxType ncType,
              NcxxDim &dim0, 
              NcxxDim &dim1, 
              const string &name,
              const string &standardName,
              const string &units = "");

  int _writeVar(NcxxFile &file,
                NcxxDim &timeDim,
                const string &name,
                const string &standardName,
                const string &units,
                const vector<float> vals);
  
  int _writeVar(NcxxFile &file,
                NcxxDim &timeDim,
                const string &name,
                const string &standardName,
                const string &units,
                const vector<int> vals);
  
  int _writeVar(NcxxFile &file,
                NcxxDim &timeDim,
                const string &name,
                const string &standardName,
                const string &units,
                const vector<int64_t> vals);
  
  int _writeIqVars(NcxxFile &file,
                   NcxxDim &timeDim,
                   NcxxDim &rangeDim,
                   NcxxDim &rowsElementsDim,
                   const string &iName,
                   const string &qName,
                   const float *ivals,
                   const float *qvals);
  
  string _ncTypeToStr(NcxxType nctype);
  
};

#endif
