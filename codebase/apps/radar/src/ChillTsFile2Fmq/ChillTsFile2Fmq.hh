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
// ChillTsFile2Fmq.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2009
//
///////////////////////////////////////////////////////////////
//
// ChillTsFile2Fmq reads legacy CHILL time-series data from a file
// and reformats it into IWRF time series forma.
// It saves the time series data out to a file message queue (FMQ),
// which can be read by multiple clients. Its purpose is mainly
// for simulation and debugging of time series operations.
//
////////////////////////////////////////////////////////////////

#ifndef ChillTsFile2Fmq_H
#define ChillTsFile2Fmq_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <Fmq/DsFmq.hh>
#include <didss/DsMessage.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/chill_types.h>
#include <didss/DsInputPath.hh>

using namespace std;

////////////////////////
// This class

class ChillTsFile2Fmq {
  
public:

  // constructor

  ChillTsFile2Fmq(int argc, char **argv);

  // destructor
  
  ~ChillTsFile2Fmq();

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
  DsInputPath *_reader;
  string _filePath;
  
  // sequence numbers

  si64 _packetSeqNum;
  si64 _pulseSeqNum;

  // Pulse count

  int _nPulses;

  // chill housekeeping types

  radar_info_t _radarInfo;
  scan_seg_t _scanSeg;
  processor_info_t _procInfo;
  power_update_t _powerUpdate;
  event_notice_t _eventNotice;
  cal_terms_t _calTerms;
  xmit_info_t _xmitInfo;
  antenna_correction_t _antCorr;
  xmit_sample_t _xmitSample;
  phasecode_t _phaseCode;

  bool _radarInfoAvail;
  bool _scanSegAvail;
  bool _procInfoAvail;
  bool _powerUpdateAvail;
  bool _eventNoticeAvail;
  bool _calTermsAvail;
  bool _xmitInfoAvail;
  bool _antCorrAvail;
  bool _xmitSampleAvail;
  bool _phaseCodeAvail;

  // iwrf types

  iwrf_radar_info_t _iwrfRadarInfo;
  iwrf_scan_segment_t _iwrfScanSeg;
  iwrf_antenna_correction_t _iwrfAntCorr;
  iwrf_ts_processing_t _iwrfTsProc;
  iwrf_xmit_power_t _iwrfXmitPower;
  iwrf_xmit_sample_t _iwrfXmitSample;
  iwrf_xmit_info_t _iwrfXmitInfo;
  iwrf_calibration_t _iwrfCalib;
  iwrf_event_notice_t _iwrfEventNotice;
  iwrf_phasecode_t _iwrfPhasecode;

  int _nGatesRemove;

  // IWRF info and pulse objects

  IwrfTsInfo _iwrfInfo;
  IwrfTsPulse _iwrfPulse;

  // sleep in sim mode

  double _usecsSleepSim;

  // output message and FMQ
  // we conbine a number of packets into a message before
  // writing to the FMQ

  DsFmq _fmq;
  DsMessage _msg;

  // functions

  int _processFile();

  int _seekAhead(FILE *in, int offset);

  int _readRadarInfo(FILE *in);
  int _readScanSeg(FILE *in);
  int _readProcInfo(FILE *in);
  int _readPowerUpdate(FILE *in);
  int _readEventNotice(FILE *in);
  int _readCalTerms(FILE *in);
  int _readXmitInfo(FILE *in);
  int _readAntCorr(FILE *in);
  int _readXmitSample(FILE *in);
  int _readPhaseCode(FILE *in);
  
  int _readPulse(FILE *in,
                 const generic_packet_header &genHdr);

  int _writeToFmq();

};

#endif
