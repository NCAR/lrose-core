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
// Ts2NetCDF.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2011
//
///////////////////////////////////////////////////////////////
//
// Ts2NetCDF reads time-series data and saves it out as netCDF
//
////////////////////////////////////////////////////////////////

#ifndef Ts2NetCDF_H
#define Ts2NetCDF_H

#include <string>
#include <vector>
#include <cstdio>
#include <Ncxx/Ncxx.hh>
#include <Ncxx/NcxxFile.hh>

#include "Args.hh"
#include "Params.hh"
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include <rapformats/DsRadarCalib.hh>
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>

using namespace std;

////////////////////////
// This class

class Ts2NetCDF {
  
public:

  // constructor

  Ts2NetCDF (int argc, char **argv);

  // destructor
  
  ~Ts2NetCDF();

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
  
  IwrfTsReader *_pulseReader;
  
  // calib for override

  DsRadarCalib _calOverride;

  // debug print loop count
  
  si64 _nPulsesRead;
  si64 _prevSeqNum;
  double _prevAz;

  // previous values - to check for changes

  int _nGatesPrev, _nGatesPrev2;
  int _nChannelsPrev;
  iwrf_radar_info_t _radarPrev;
  iwrf_scan_segment_t _scanPrev;
  iwrf_ts_processing_t _procPrev;
  iwrf_calibration_t _calibPrev;

  // sector information to determine if we need a new file
  
  bool _needNewFile;
  int _nPulsesFile; // number of pulses in current file
  double _sectorWidth; // width of sector (file)
  int _currentSector; // which sector are we in?

  // ray metadata

  size_t _nTimes;
  time_t _startTime;

  int _nGates;
  int _nGatesSave;
  int _nGatesMax;

  bool _alternatingMode;

  double _startAz;
  double _startEl;
  iwrf_scan_mode_t _scanMode;
  iwrf_xmit_rcv_mode _xmitRcvMode;

  time_t _pulseTimeSecs;
  double _pulseTime;

  double _el;
  double _az;
  double _prt;
  double _phaseDiff;
  double _txPhaseDeg;
  
  // meta data arrays

  vector<int> _nGatesRay;

  vector<double> _timeArrayHc, _dtimeArrayHc;
  vector<float> _elArrayHc, _azArrayHc, _fixedAngleArrayHc;
  vector<int> _transitionFlagArrayHc;
  vector<float> _prtArrayHc;
  vector<float> _pulseWidthArrayHc;
  vector<float> _modCodeArrayHc;
  vector<float> _txPhaseArrayHc;
  vector<float> _burstMagArrayHc;
  vector<float> _burstArgArrayHc;

  vector<double> _timeArrayVc, _dtimeArrayVc;
  vector<float> _elArrayVc, _azArrayVc, _fixedAngleArrayVc;
  vector<int> _transitionFlagArrayVc;
  vector<float> _prtArrayVc;
  vector<float> _pulseWidthArrayVc;
  vector<float> _modCodeArrayVc;
  vector<float> _txPhaseArrayVc;
  vector<float> _burstMagArrayVc;
  vector<float> _burstArgArrayVc;

  vector<double> _timeArrayHx, _dtimeArrayHx;
  vector<float> _elArrayHx, _azArrayHx, _fixedAngleArrayHx;
  vector<int> _transitionFlagArrayHx;
  vector<float> _prtArrayHx;
  vector<float> _pulseWidthArrayHx;
  vector<float> _modCodeArrayHx;
  vector<float> _txPhaseArrayHx;
  vector<float> _burstMagArrayHx;
  vector<float> _burstArgArrayHx;

  vector<double> _timeArrayVx, _dtimeArrayVx;
  vector<float> _elArrayVx, _azArrayVx, _fixedAngleArrayVx;
  vector<int> _transitionFlagArrayVx;
  vector<float> _prtArrayVx;
  vector<float> _pulseWidthArrayVx;
  vector<float> _modCodeArrayVx;
  vector<float> _txPhaseArrayVx;
  vector<float> _burstMagArrayVx;
  vector<float> _burstArgArrayVx;

  // IQ data

  int _nPulsesHc;
  int _nPulsesVc;
  int _nPulsesHx;
  int _nPulsesVx;
  
  MemBuf _iBuf0, _qBuf0;
  MemBuf _iBuf1, _qBuf1;
  MemBuf _iBufHc, _qBufHc;
  MemBuf _iBufVc, _qBufVc;
  MemBuf _iBufHx, _qBufHx;
  MemBuf _iBufVx, _qBufVx;

  // output file

  string _outputName;
  string _outputPath;
  string _tmpPath;

  // functions
  
  void _setNGatesMax();
  void _checkAltMode();

  bool _checkInfoChanged(const IwrfTsPulse &pulse);
  bool _checkReadyToWrite(const IwrfTsPulse &pulse);
  int _handlePulse(IwrfTsPulse &pulse);
  int _savePulseDataSimHV(IwrfTsPulse &pulse);
  int _savePulseDataHXmit(IwrfTsPulse &pulse);
  int _savePulseDataVXmit(IwrfTsPulse &pulse);
  int _savePulseDataAltH(IwrfTsPulse &pulse);
  int _savePulseDataAltV(IwrfTsPulse &pulse);
  int _savePulseDataSinglePol(IwrfTsPulse &pulse);
  int _savePulseDataSinglePolV(IwrfTsPulse &pulse);
  int _savePulseData(IwrfTsPulse &pulse);
  void _reset();

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
  
  int _writeIqVars(NcxxFile &file,
                   NcxxDim &timeDim,
                   NcxxDim &rangeDim,
                   const string &iName,
                   const string &qName,
                   const float *ivals,
                   const float *qvals);
  
  string _ncTypeToStr(NcxxType nctype);
  
};

#endif
