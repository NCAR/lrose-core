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
// ReadFromUdp.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2019
//
///////////////////////////////////////////////////////////////
//
// Read UDP stream that is created by the WRITE_TO_UDP mode
// of this application.
// Creates IPS time series format stream,
// and write out to an FMQ
//
////////////////////////////////////////////////////////////////

#ifndef ReadFromUdp_HH
#define ReadFromUdp_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include <radar/ips_ts_data.h>
#include <radar/IpsTsInfo.hh>
#include <Radx/RadxTime.hh>

////////////////////////
// This class

class ReadFromUdp {
  
public:

  // constructor

  ReadFromUdp(const string &progName,
              const Params &params,
              vector<string> &inputFileList);
  
  // destructor
  
  ~ReadFromUdp();

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

  // pulse metadata

  ui64 _dwellSeqNum;
  ui64 _pulseSeqNum;
  vector<IwrfTsPulse *> _dwellPulses;
  
  ui16 _messageType;
  ui16 _aesaId;
  ui16 _chanNum;
  ui32 _flags;
  bool _isXmitH;
  bool _isRxH;
  bool _isCoPolRx;
  bool _isFirstPktInPulse;
  ui32 _beamIndex;
  ui64 _sampleNum;
  ui64 _pulseNum;
  ui64 _secs;
  ui32 _nsecs;
  RadxTime _rtime;
  ui32 _startIndex;
  fl32 _uu;
  fl32 _vv;
  double _el;
  double _az;

  ui64 _dwellNum;
  ui32 _beamNumInDwell;
  ui32 _visitNumInBeam;
  ui32 _nSamples;

  // pulse IQ

  vector<si16> _iqIps;

  // IPS-style metadata

  IpsTsInfo *_ipsTsInfo;
  IpsTsDebug_t _ipsTsDebug;
  ips_ts_radar_info_t _ipsRadarInfo;
  ips_ts_scan_segment_t _ipsScanSegment;
  ips_ts_processing_t _ipsTsProcessing;
  ips_ts_calibration_t _ipsCalibration;
  int _volNum;
  int _sweepNum;
  si64 _nPulsesOut;

  // output message and FMQ
  // we conbine a number of packets into a message before
  // writing to the FMQ

  DsFmq _outputFmq;
  DsMessage _outputMsg;
  
  // functions

  int _openUdpForReading();
  int _initMetaData(const string &inputPath);
  void _convertMeta2Ips(const IwrfTsInfo &info);
  int _handlePacket(ui08 *pktBuf, int pktLen);

  // write to FMQ

  int _writePulseToFmq();
  int _openOutputFmq();
  void _addMetaDataToMsg();
  int _writeToOutputFmq(bool force = false);
  int _writeEndOfVol();
  
};

#endif
