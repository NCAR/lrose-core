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
///////////////////////////////////////////////////////////////
//
// ScanInfo2TsFmq.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2026
//
///////////////////////////////////////////////////////////////
//
// ScanInfo2Fmq reads scan information via UDP.
// It then writes the appropriate packets to a time series FMQ.
//
////////////////////////////////////////////////////////////////

#ifndef ScanInfo2TsFmq_HH
#define ScanInfo2TsFmq_HH

#include <string>
#include <sys/types.h>

#include "Args.hh"
#include "Params.hh"
#include <Fmq/DsFmq.hh>
#include <didss/DsMessage.hh>
#include <radar/iwrf_data.h>
#include <toolsa/DateTime.hh>

using namespace std;

////////////////////////
// This class

class ScanInfo2TsFmq {

public:

  // constructor

  ScanInfo2TsFmq(int argc, char **argv);

  // destructor

  ~ScanInfo2TsFmq();

  // run

  int Run();

  // data members

  bool isOK;

protected:

private:

  // decoded UDP telemetry

  struct ScanTelemetry {
    DateTime time;
    double azimuth;
    double elevation;
    int scanType;
    int volumeNum;
    int sweepNum;
    double fixedAngle;
    bool inTransition;
    string scanName;
  };

  string _progName;
  Args _args;
  char *_paramsPath;
  Params _params;

  // UDP input socket. This object owns the descriptor from bind through
  // shutdown, so it is closed only by the destructor.

  int _udpFd;

  // output message and FMQ

  DsFmq _outputFmq;
  DsMessage _outputMsg;

  // packet sequence numbers and input time

  si64 _packetSeqNum;
  si64 _pulseSeqNum;
  DateTime _prevTelemetryTime;
  DateTime _lastMetadataTime;
  time_t _lastReceiveTime;
  bool _staleEventWritten;

  // current scan state

  bool _haveScanState;
  ScanTelemetry _prevScanState;
  int _volumeStartSweepNum;
  int _outputVolumeNum;
  int _outputSweepNum;

  // reading and decoding UDP data

  int _openUdpSocket();
  int _readDatagram(string &datagram, bool &timedOut);
  bool _decodeTelemetry(const string &datagram, ScanTelemetry &telemetry,
                        string &error);
  bool _validateTelemetry(const ScanTelemetry &telemetry,
                          string &error) const;
  int _processTelemetry(ScanTelemetry telemetry);

  // parsing helpers

  string _trim(const string &input) const;
  bool _parseTime(const string &text, DateTime &value,
                  string &error) const;
  bool _parseDouble(const string &text, double &value) const;
  bool _parseInt(const string &text, int &value) const;
  void _copyString(char *dest, size_t destLen,
                   const string &source) const;

  // managing scan state

  int _scanMode(const ScanTelemetry &telemetry) const;
  bool _scanStateChanged(const ScanTelemetry &telemetry) const;

  // assembling IWRF packets

  void _addReaderMetadata(const ScanTelemetry &telemetry);
  void _addScanSegment(const ScanTelemetry &telemetry);
  void _addPulse(const ScanTelemetry &telemetry);
  void _setPacketTimeAndSequence(iwrf_packet_info_t &packet,
                                 const DateTime &time);

  // writing to the FMQ

  int _openOutputFmq();
  int _writeToOutputFmq(bool force = false);
  int _writeEndOfVol();
};

#endif
