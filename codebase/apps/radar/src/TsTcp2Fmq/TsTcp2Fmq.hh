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
// TsTcp2Fmq.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// TsTcp2Fmq reads data from a server in TCP, and writes
// it out to an FMQ
//
////////////////////////////////////////////////////////////////

#ifndef TsTcp2Fmq_HH
#define TsTcp2Fmq_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/Socket.hh>
#include <Fmq/DsFmq.hh>
#include <toolsa/MemBuf.hh>
#include <toolsa/ushmem.h>
#include <didss/DsMessage.hh>
#include <radar/syscon_to_spol.h>
#include <radar/IwrfCalib.hh>

using namespace std;

////////////////////////
// This class

class TsTcp2Fmq {
  
public:

  // constructor

  TsTcp2Fmq(int argc, char **argv);

  // destructor
  
  ~TsTcp2Fmq();

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

  // heartbeat function for reading

  Socket::heartbeat_t _heartBeatFunc;

  // tcp input socket

  Socket _sock;
  bool _sockTimedOut;
  int _timedOutCount;
  bool _unknownMsgId;
  int _unknownCount;

  // input message

  MemBuf _inputBuf;
  si32 _packetId;
  si32 _packetLen;

  // output message and FMQ
  // we conbine a number of packets into a message before
  // writing to the FMQ

  DsFmq _outputFmq;
  DsMessage _outputMsg;
  
  // calibration info

  IwrfCalib _calib;

  // Pulse count

  int _pulseCount;

  // time
  
  double _prevPulseTime;

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

  // position update

  double _radarLatitude;
  double _radarLongitude;
  double _radarAltitude;
  double _vehicleHeading;

  time_t _posnLastChecked;
  time_t _posnLastModified;

  // print warning on scaling

  bool _scaleWarningPrinted;

  // reading data

  int _readFromServer();
  int _readMessage();
  int _readTcpPacket();

  // modifying meta data

  void _modifySweepNumber(iwrf_pulse_header_t &pHdr);
  void _modifyRadarLocation(iwrf_radar_info_t &ri);
  void _modifyAzFromHeading(iwrf_pulse_header_t &pHdr);
  void _setAzFromTime(iwrf_pulse_header_t &pHdr);
  bool _checkTimeGoesForward(iwrf_pulse_header_t &pHdr);
  void _handleCalibration();

  void _delaySweepNumChange(iwrf_pulse_header_t &pHdr);
  int _loadPosition();
 
  // write to FMQ

  int _openOutputFmq();
  int _writeToOutputFmq(bool force = false);
  int _writeEndOfVol();
  void _applyIQScale();

};

#endif
