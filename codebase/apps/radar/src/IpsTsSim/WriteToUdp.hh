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
// WriteToUdp.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2019
//
///////////////////////////////////////////////////////////////
//
// Resample IWRF time series data,
// convert to IPS UDP format,
// and write out to UDP stream
//
////////////////////////////////////////////////////////////////

#ifndef WriteToUdp_HH
#define WriteToUdp_HH

#include <string>
#include <vector>
#include <deque>
#include <cstdio>
#include <Radx/Radx.hh>
#include <Radx/RadxTime.hh>

#include "Args.hh"
#include "Params.hh"

class IwrfTsPulse;
class SimScanStrategy;
using namespace std;

////////////////////////
// This class

class WriteToUdp {
  
public:

  // constructor
  
  WriteToUdp(const string &progName,
             const Params &params,
             vector<string> &inputFileList);

  // destructor
  
  ~WriteToUdp();

  // run 

  int Run();
  
protected:
  
private:
  
  string _progName;
  const Params &_params;
  vector<string> _inputFileList;

  // output UDP

  int _udpFd;
  int _errCount;

  // pulse details

  ui64 _dwellSeqNum;
  ui64 _pulseSeqNum;
  ui64 _sampleSeqNum; // for UDP only
  vector<IwrfTsPulse *> _dwellPulses;
  si64 _realtimeDeltaSecs;

  // packet details

  size_t _nBytesHeader;
  size_t _nBytesPacket;
  size_t _nGatesPacket;
  size_t _nBytesData;
  MemBuf _sampleBuf;

  // between-pulse data

  size_t _nGatesRemaining;
  deque<si16> _iqQueue;

  // data rate
  
  RadxTime _rateStartTime;
  double _nBytesForRate;

  // simulated scan strategy

  SimScanStrategy *_strategy;
  
  // functions

  int _convertToUdp(const string &inputPath);
  int _processDwell(vector<IwrfTsPulse *> &dwellPulses);

  void _fillIqData(IwrfTsPulse *iwrfPulse,
                   int channelNum,
                   vector<si16> &iqData);

  int _sendPulse(si64 secondsTime,
                 ui32 nanoSecs,
                 ui64 dwellNum,
                 ui32 beamNumInDwell,
                 ui32 visitNumInBeam,
                 double uu,
                 double vv,
                 bool isXmitH,
                 bool isCoPolRx,
                 int nGates,
                 vector<si16> &iqIps);

  void _addIpsHeader(si64 secondsTime,
                      ui32 nanoSecs,
                      ui32 pulseStartIndex,
                      ui64 dwellNum,
                      ui32 beamNumInDwell,
                      ui32 visitNumInBeam,
                      double uu,
                      double vv,
                      bool isFirstPktInPulse,
                      bool isXmitH,
                      bool isCoPolRx,
                      int nSamples,
                      MemBuf &buf);

  int _openOutputUdp();
  int _writeBufToUdp(const MemBuf &buf);
  void _sleepForDataRate();

};

#endif
