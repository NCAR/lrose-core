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
// SpolTs2Fmq.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////
//
// SpolTs2Fmq reads time series data from a server in TCP,
// and writes it out to an FMQ. It also optionally merges in
// syscon scan information, and angles from the S2D system.
//
////////////////////////////////////////////////////////////////

#ifndef SpolTs2Fmq_HH
#define SpolTs2Fmq_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/Socket.hh>
#include <toolsa/ushmem.h>
#include <toolsa/MemBuf.hh>
#include <didss/DsMessage.hh>
#include <Fmq/DsFmq.hh>
#include <radar/syscon_to_spol.h>
#include <radar/spol_angles.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfCalib.hh>
#include <radar/RadarComplex.hh>

using namespace std;

////////////////////////
// This class

class SpolTs2Fmq {
  
public:

  // constructor

  SpolTs2Fmq(int argc, char **argv);

  // destructor
  
  ~SpolTs2Fmq();

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

  // FMQ input
  
  Fmq _inputFmq;
  DsMessage _msg;
  int _msgNParts;
  int _msgPos;
  
  // tcp input socket

  Socket _sock;
  bool _sockTimedOut;
  int _timedOutCount;
  bool _unknownMsgId;
  int _unknownCount;

  // input message

  MemBuf _msgBuf;
  si32 _packetId;
  si32 _packetLen;
  iwrf_packet_info_t _latestPacketInfo;

  // heartbeat function for reading

  Socket::heartbeat_t _heartBeatFunc;

  // output message and FMQ
  // we conbine a number of packets into a message before
  // writing to the FMQ

  DsFmq _outputFmq;
  DsMessage _outputMsg;
  
  // time series info

  IwrfTsInfo _info;

  // calibration info

  IwrfCalib _calib;

  // Pulse count

  int _pulseCount;

  // volume number

  int _prevVolNum;
  int _volNum;

  // sweep number
  
  int _sweepNum;
  int _volStartSweepNum;
  
  // delaying sweep number till change in dirn

  bool _sweepNumChangeInit;
  bool _sweepNumChangeInProgress;
  int _sweepNumOld;
  double _sweepNumAzOld;
  double _sweepNumTransDirn;

  // radar info etc from time series
  
  iwrf_xmit_rcv_mode _xmitRcvMode;
  iwrf_radar_info _tsRadarInfo;
  iwrf_scan_segment _tsScanSeg;
  iwrf_ts_processing _tsTsProc;
  iwrf_xmit_power _tsXmitPower;
  iwrf_xmit_info _tsXmitInfo;
  
  // FMQ for syscon information
  // This info is written by SpolSysconRelay

  DsFmq _sysconFmq;
  
  // syscon data

  iwrf_radar_info_t _sysconRadarInfo;
  iwrf_scan_segment_t _sysconScanSeg;
  iwrf_ts_processing_t _sysconTsProc;
  iwrf_xmit_power_t _sysconXmitPower;
  iwrf_event_notice_t _sysconStartOfSweep;
  iwrf_event_notice_t _sysconStartOfSweepPrev;
  iwrf_event_notice_t _sysconEndOfSweep;
  iwrf_event_notice_t _sysconEndOfSweepPrev;
  iwrf_event_notice_t _sysconStartOfVolume;
  iwrf_event_notice_t _sysconEndOfVolume;

  bool _sysconRadarInfoActive;
  bool _sysconScanSegActive;
  bool _sysconTsProcActive;
  bool _sysconXmitPowerActive;
  bool _sysconStartOfSweepActive;
  bool _sysconEndOfSweepActive;
  bool _sysconStartOfVolumeActive;
  bool _sysconEndOfVolumeActive;
  
  // read delay to impose on TCP pulse packets when
  // using scan info from shared memory. The wait is
  // needed to sync the pulse data with the shared memory events

  bool _sysconLateSet;
  double _sysconLateSecs;

  // angle input message and FMQ

  DsFmq _angleFmq;
  DsMessage _angleMsg;
  DsMsgPart *_anglePart;
  int _anglePos, _angleNParts;
  spol_short_angle_t _latestAngle;
  spol_short_angle_t _prevAngle;

  // test pulse

  int _nSamplesTestPulse;

  RadarComplex_t *_testIqHc;
  RadarComplex_t *_testIqHx;
  RadarComplex_t *_testIqVc;
  RadarComplex_t *_testIqVx;

  double _testPowerDbHc;
  double _testPowerDbHx;
  double _testPowerDbVc;
  double _testPowerDbVx;

  double _testVelHc;
  double _testVelHx;
  double _testVelVc;
  double _testVelVx;

  // angles for errors

  vector<double> _azErrors, _elErrors;
  double _maxAzError;
  double _maxElError;
  double _sumAzError;
  double _sumElError;
  double _sumsqAzError;
  double _sumsqElError;
  double _meanAzError;
  double _meanElError;
  double _sdevAzError;
  double _sdevElError;
  int _nErrorAz;
  int _nErrorEl;

  // secondary status Xml input message and FMQ

  DsFmq _secondaryStatusFmq;
  
  // status xml

  time_t _testPulseLatestTime;
  time_t _angleErrorsLatestTime;
  time_t _xmitPowerLatestTime;
  time_t _statusXmlLatestTime;
  time_t _secondaryStatusLatestTime;
  
  string _testPulseXml;
  string _angleErrorsXml;
  string _xmitPowerXml;
  string _statusXml;
  string _secondaryStatusXml;
  string _augmentedXml;
  
  // print warning on scaling

  bool _scaleWarningPrinted;

  // run mode

  int _runFmqMode();
  int _runTcpMode();

  // reading data from FMQ

  int _readFromFmq();
  const DsMsgPart *_getNextFromFmq();

  // reading data from TCP

  int _readFromTcpServer();
  int _readTcpMessage();
  int _readTcpPacket();

  // packet handling

  void _handlePacket();
  int _checkPacket();

  // handle the pulse
  
  void _handlePulse();
  
  // sweep number mods

  void _modifySweepNumber(iwrf_pulse_header_t &pHdr);
  void _delaySweepNumChange(iwrf_pulse_header_t &pHdr);
 
  // write to FMQ

  int _openOutputFmq();
  int _writeToOutputFmq(bool force = false);
  int _writeEndOfVol();
  
  // merging in the angle data

  int _openAngleFmq();
  int _readAngle();
  int _getNextAngle();
  void _mergeAntennaAngles(iwrf_pulse_header_t &pHdr);
  void _checkAntennaAngles(iwrf_pulse_header_t &pHdr);
  void _computeAngleStats();
  void _sumAngleStats(bool censorOutliers);
  double _getTimeAsDouble(si64 baseSecs, si64 secs, si32 nanoSecs);
  void _interpAngles(double prevEl, double latestEl,
                     double prevAz, double latestAz,
                     double pulseTime,
                     double prevTime, double latestTime,
                     double &interpEl, double &interpAz);
  void _correctAngles(iwrf_pulse_header_t &pHdr,
                      double errorEl,
                      double errorAz,
                      double s2dEl,
                      double s2dAz);


  // reading syscon info

  int _openSysconFmq();
  int _readSysconFromFmq();

  // synchronizing syscon with the time series data

  void _setReadDelay(iwrf_packet_info &shmemPacket);
  void _doReadDelay(iwrf_packet_info &pulsePacket);

  // modifying iwrf data based on syscon data

  void _writeRadarInfoToFmq();
  void _writeScanSegmentToFmq();
  void _writeTsProcessingToFmq();
  void _writeXmitPowerToFmq(const iwrf_xmit_power_t &power);
  void _writeXmitInfoToFmq(const iwrf_xmit_info_t &info);
  void _writeSysconEventToFmq(const iwrf_event_notice_t &event);
  void _modifyPulseHeaderFromSyscon(iwrf_pulse_header_t &pHdr);
  void _monitorTestPulse();

  void _loadTestPulseIq(IwrfTsPulse &pulse,
                        int channelNum,
                        int gateNum,
                        RadarComplex_t *iq);

  // status xml

  void _handleStatusXml();
  void _checkStatusXml();
  void _augmentStatusXml(const iwrf_packet_info_t &packet,
                         const string &xmlStr);

  // calibration

  void _handleCalibration();

  int _openSecondaryStatusFmq();
  int _readSecondaryStatusFromFmq();
  void _applyIQScale();

};

#endif
