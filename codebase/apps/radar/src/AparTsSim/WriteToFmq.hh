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
// WriteToFmq.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2020
//
///////////////////////////////////////////////////////////////
//
// Resample IWRF time series data,
// convert to APAR time series format,
// and write out to FMQ
//
////////////////////////////////////////////////////////////////

#ifndef WriteToFmq_HH
#define WriteToFmq_HH

#include <string>
#include <vector>
#include <deque>
#include <cstdio>
#include <Radx/Radx.hh>
#include <Radx/RadxTime.hh>

#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include <radar/apar_ts_data.h>
#include <radar/AparTsInfo.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;

////////////////////////
// This class

class WriteToFmq {
  
public:

  // constructor
  
  WriteToFmq(const string &progName,
             const Params &params,
             vector<string> &inputFileList);

  // destructor
  
  ~WriteToFmq();

  // run 

  int Run();
  
protected:
  
private:
  
  string _progName;
  const Params &_params;
  vector<string> _inputFileList;

  // pulse details

  ui64 _dwellSeqNum;
  ui64 _pulseSeqNum;
  ui64 _sampleSeqNum; // for UDP only
  vector<IwrfTsPulse *> _dwellPulses;
  si64 _realtimeDeltaSecs;

  // data rate
  
  RadxTime _rateStartTime;
  double _nBytesForRate;

  // simulated scan strategy
  
  int _simVolNum;
  size_t _simBeamNum;
  vector<double> _simEl;
  vector<double> _simAz;
  vector<int> _simSweepNum;
  vector<Radx::SweepMode_t> _simSweepMode;

  // pulse IQ

  // vector<si16> _iqApar;

  // APAR-style metadata

  AparTsInfo *_aparTsInfo;
  AparTsDebug_t _aparTsDebug;
  apar_ts_radar_info_t _aparRadarInfo;
  apar_ts_scan_segment_t _aparScanSegment;
  apar_ts_processing_t _aparTsProcessing;
  apar_ts_calibration_t _aparCalibration;
  int _volNum;
  int _sweepNum;
  si64 _nPulsesOut;

  // output message and FMQ
  // we conbine a number of packets into a message before
  // writing to the FMQ

  DsFmq _outputFmq;
  DsMessage _outputMsg;
  
  // functions

  int _convertToFmq(const string &inputPath);
  int _processDwell(vector<IwrfTsPulse *> &dwellPulses);

  void _computeScanStrategy();
  void _sleepForDataRate();

  int _initMetaData(const IwrfTsInfo &tsInfo);
  void _convertMeta2Apar(const IwrfTsInfo &info);

  int _openOutputFmq();
  void _addMetaDataToMsg();
  int _writeToOutputFmq(bool force = false);
  int _writeEndOfVol();
  
};

#endif
