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
// CP2Udp2Fmq.hh
//
// Adapted from work by
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
// Phil Purdam, BMRC, Australian Bureau of Meteorology
// 
//
///////////////////////////////////////////////////////////////
//
// CP2Udp2Fmq reads data from CP2 Timeseries server in CP2 TimeSeries format
// and writes it to files in TsArchive format
//
////////////////////////////////////////////////////////////////

#ifndef CP2Udp2Fmq_H
#define CP2Udp2Fmq_H

#include <pthread.h>

#include <string>
#include <vector>
#include <deque>
#include <cstdio>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "Args.hh"
#include "Params.hh"
#include "CP2Net.hh"

#include <Fmq/DsFmq.hh>
#include <didss/DsMessage.hh>
#include <toolsa/MemBuf.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <rapformats/DsRadarCalib.hh>

using namespace std;
using namespace CP2Net;

////////////////////////
// This class

class CP2Udp2Fmq {
  
public:

  // constructor

  CP2Udp2Fmq(int argc, char **argv);

  // destructor
  
  ~CP2Udp2Fmq();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:
  
  static const int INFO_ID = 77000;
  static const int PULSE_ID = 77001;

  string _progName;
  string _defaultParamsPath;
  char *_paramsPath;
  Args _args;
  Params _params;

  // input Udp handle & errno

  int _udpFd;
  int _errno;

  CP2Packet _pulsePacket;
  
  // output FMQ

  DsFmq _fmq_s;
  DsFmq _fmq_x;

  // output messages

  DsMessage _msg_s;
  DsMessage _msg_x;

  // calibrations

  DsRadarCalib _calib_s;
  DsRadarCalib _calib_x;

  // incoming pulse info and header

  struct CP2PulseHeader _pulseHeader;
  CP2PulseHeader _prevPulseHeader_s;
  CP2PulseHeader _prevPulseHeader_x;

  IwrfTsInfo _tsInfo_s;
  IwrfTsInfo _tsInfo_x;
  IwrfTsPulse _tsPulse_s;
  IwrfTsPulse _tsPulse_x;
  bool _tsInfoValid_s;
  bool _tsInfoValid_x;

  // debug print loop count
  
  int _nPulses_s;
  int _nPulses_x;
  double _prevAz;

  int _thisVolNum;    // to trigger new vol event
  int _baseSweepNum;   // keep base tilt number on new vol
  int _thisSweepNum;

  bool _antTransFlag;

  MemBuf _convertBuf_s;
  MemBuf _convertBuf_x;// combined xh & xv buffer

  CP2PulseCollator x_pulseCollator;

  // pulse time error

  double _pulseTimeErrorSum;
  double _pulseTimeErrorCount;
  double _pulseTimeError;
  double _timeOfPrevErrorPrint;

  // checking for missing pulses

  static const int NCHANNELS = 3;
  si64 _prevSeqChan[NCHANNELS];

  // outgoing packet sequence number

  si64 _outPktSeqNum_s;
  si64 _outPktSeqNum_x;

  // read thread

  pthread_mutex_t _readMutex;
  deque<MemBuf *> _readQueue;
  static const int maxQueueSize = 10000;

  ///	prior cumulative pulse counts, used for throughput calcs
  int	_prevPulseCount[3];	
  ///	cumulative pulse counts 
  int	_pulseCount[3];	
  ///	cumulative error counts
  int	_errorCount[3];		
  /// A flag that is set true as soon as one EOF is detected on a channel.
  bool _eof[3];		
  /// The pulse number of the preceeding pules, used to detect dropped pulses.
  si64 _lastPulseNum[3];
  float _lastPulseAz[3];
  static const double PIRAQ3D_SCALE;

  // functions

  inline bool _debug() {
    return _params.debug >= Params::DEBUG_NORM;
  }
  
  inline bool _verbose() {
    return _params.debug >= Params::DEBUG_VERBOSE;
  }

  int  _openUdp();
  void _closeUdp();
  
  void _readFromQueue();

  int _handlePacket(MemBuf *mbuf);

  void _pulseBookKeeping(CP2Pulse* pPulse);

  int _reformatPulse(CP2Pulse* pPulse);
  int _reformatPulse_s(CP2Pulse* pPulse);
  int _reformatPulse_x(CP2Pulse* pPulse);
  
  int _reformatInfo(const CP2PulseHeader &cp2Header,
		    const Params::ts_pulse_info_params &pulseinfo,
		    const DsRadarCalib &calibinfo,
		    const Params::ts_ops_info_params &opsinfo,
		    IwrfTsInfo &tsinfo);
  
  bool _opsInfoChanged(const CP2PulseHeader &cp2Header,
		       const CP2PulseHeader &prevCp2Header);
  
  void _checkForMissing(const CP2PulseHeader &cp2Header);

  void _setPulseHeader(const CP2PulseHeader &cp2Header,
		       const Params::ts_pulse_info_params &pulseinfo,
		       const Params::ts_ops_info_params &opsinfo,
		       IwrfTsPulse &pulse);

  // functions which run in read thread
  
  static void* _readUdpInThread(void *thread_data);
  static int  readSelect(int sd, long wait_msecs);
  static int _checkPacket(MemBuf *mbuf, si64 *prevSeqChan,
			  deque<MemBuf *> &readQueue);

  // adding data to the message

  void _addInfo2Msg(IwrfTsInfo &info, DsMessage &msg,
		    si64 &outPktSeqNum);

  void _addPulse2Msg(IwrfTsPulse &pulse, DsMessage &msg,
		     si64 &outPktSeqNum);
  
};

#endif
