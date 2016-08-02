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
// ChillTsTcp2Fmq.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2009
//
///////////////////////////////////////////////////////////////
//
// ChillTsTcp2Fmq reads data from a legacy CHILL server in TCP,
// and writes it out to an FMQ
//
////////////////////////////////////////////////////////////////

#ifndef ChillTsTcp2Fmq_HH
#define ChillTsTcp2Fmq_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/Socket.hh>
#include <Fmq/DsFmq.hh>
#include <toolsa/MemBuf.hh>
#include <didss/DsMessage.hh>
#include <didss/LdataInfo.hh>
#include <radar/IwrfTsReader.hh>
#include <radar/chill_types.h>
#include <rapformats/DsRadarSweep.hh>
#include <rapformats/DsRadarPower.hh>

using namespace std;

////////////////////////
// This class

class ChillTsTcp2Fmq {
  
public:

  // constructor

  ChillTsTcp2Fmq(int argc, char **argv);

  // destructor
  
  ~ChillTsTcp2Fmq();

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

  // reading cal files

  LdataInfo _calibLdata;

  // output message and FMQ
  // we conbine a number of packets into a message before
  // writing to the FMQ

  DsFmq _fmq;
  DsMessage _msg;
  time_t _prevMetaTime;
  
  // Pulse count

  int _nPulses;

  // sequence numbers
  
  si64 _packetSeqNum;
  si64 _pulseSeqNum;

  // vol and sweep num

  int _prevVolNum;
  int _prevSweepNum;
  int _sweepNum;
  int _volStartSweepNum;

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
  bool _calibAvail;
  bool _iwrfCalibAvail;

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

  // IWRF info and pulse objects

  IwrfTsInfo _iwrfInfo;
  IwrfTsPulse _iwrfPulse;

  // number of gates to be removed if we choose to remove
  // gates with negative range

  int _nGatesRemove;

  // functions

  int _sendCommands(Socket &sock);
  int _readFromServer(Socket &sock);
  int _reSync(Socket &sock);
  int _peekAtBuffer(Socket &sock, void *buf, int nbytes);
  int _seekAhead(Socket &sock, int nBytes);
  void _checkSweepNum(iwrf_pulse_header_t &pHdr);

  int _readRadarInfo(Socket &sock);
  int _readScanSeg(Socket &sock);
  int _readProcInfo(Socket &sock);
  int _readPowerUpdate(Socket &sock);
  int _readIwrfXmitPower(Socket &sock);
  int _readEventNotice(Socket &sock);
  int _readIwrfEventNotice(Socket &sock);
  int _readCalTerms(Socket &sock);
  int _readCalFromFile();
  int _readXmitInfo(Socket &sock);
  int _readAntCorr(Socket &sock);
  int _readXmitSample(Socket &sock);
  int _readPhaseCode(Socket &sock);
  
  int _readPulse(Socket &sock,
                 const generic_packet_header &genHdr);

  int _writeToFmq();
  
};

#endif
