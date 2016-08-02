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
// OpsInfo.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2010
//
///////////////////////////////////////////////////////////////
//
// OpsInfo reads data from a SysCon server in legacy CHILL format,
// and converts to IWRF format
//
////////////////////////////////////////////////////////////////

#ifndef OpsInfo_HH
#define OpsInfo_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Params.hh"
#include <toolsa/Socket.hh>
#include <toolsa/MemBuf.hh>
#include <radar/iwrf_data.h>
#include <radar/chill_types.h>
#include <radar/syscon_to_spol.h>
#include <Fmq/DsFmq.hh>

using namespace std;

////////////////////////
// This class

class OpsInfo {
  
public:

  // constructor

  OpsInfo(const string &progName,
          const Params &params,
          DsFmq &outputFmq);

  // destructor
  
  ~OpsInfo();

  // check for data
  // waiting specified time
  
  int checkForData(int waitMsecs);

  // read data from server
  // returns 0 on success, -1 on failure
  // also sets id of iwrf data read
  
  int read(int &iwrfId);

  // get iwrf types
  
  const iwrf_radar_info_t &getIwrfRadarInfo() const {
    return _iwrfRadarInfo;
  }
  const iwrf_scan_segment_t &getIwrfScanSeg() const {
    return _iwrfScanSeg;
  }
  const iwrf_antenna_correction_t &getIwrfAntCorr() const {
    return _iwrfAntCorr;
  }
  const iwrf_ts_processing_t &getIwrfTsProc() const {
    return _iwrfTsProc;
  }
  const iwrf_xmit_power_t &getIwrfXmitPower() const {
    return _iwrfXmitPower;
  }
  const iwrf_xmit_sample_t &getIwrfXmitSample() const {
    return _iwrfXmitSample;
  }
  const iwrf_xmit_info_t &getIwrfXmitInfo() const {
    return _iwrfXmitInfo;
  }
  const iwrf_calibration_t &getIwrfCalib() const {
    return _iwrfCalib;
  }
  const iwrf_event_notice_t &getIwrfEventNotice() const {
    return _iwrfEventNotice;
  }
  const iwrf_phasecode_t &getIwrfPhasecode() const {
    return _iwrfPhaseCode;
  }

protected:
  
private:

  string _progName;
  const Params &_params;

  // socket

  Socket _sock;
  MemBuf _readBuf;
  
  // output FMQ

  DsFmq &_outputFmq;
  
  // chill housekeeping types
  
  radar_info_t _chillRadarInfo;
  scan_seg_t _chillScanSeg;
  processor_info_t _chillProcInfo;
  power_update_t _chillPowerUpdate;
  event_notice_t _chillEventNotice;
  cal_terms_t _chillCalTerms;
  xmit_info_t _chillXmitInfo;
  antenna_correction_t _chillAntCorr;
  xmit_sample_t _chillXmitSample;
  phasecode_t _chillPhaseCode;

  bool _chillRadarInfoAvail;
  bool _chillScanSegAvail;
  bool _chillProcInfoAvail;
  bool _chillPowerUpdateAvail;
  bool _chillEventNoticeAvail;
  bool _chillCalTermsAvail;
  bool _chillXmitInfoAvail;
  bool _chillAntCorrAvail;
  bool _chillXmitSampleAvail;
  bool _chillPhaseCodeAvail;

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
  iwrf_phasecode_t _iwrfPhaseCode;

  bool _iwrfRadarInfoAvail;
  bool _iwrfScanSegAvail;
  bool _iwrfAntCorrAvail;
  bool _iwrfTsProcAvail;
  bool _iwrfXmitPowerAvail;
  bool _iwrfXmitSampleAvail;
  bool _iwrfXmitInfoAvail;
  bool _iwrfCalibAvail;
  bool _iwrfEventNoticeAvail;
  bool _iwrfPhaseCodeAvail;

  // outgoing message sequence num

  si64 _iwrfSeqNum;

  // reporting with data mapper

  time_t _dmapPrevRegTime;

  // functions

  int _openSocket(const char *host, int port);
  void _closeSocket();

  int _processChillRadarInfo();
  int _processChillScanSeg();
  int _processChillProcInfo();
  int _processChillPowerUpdate();
  int _processChillEventNotice();
  int _processChillCalTerms();
  int _processChillXmitInfo();
  int _processChillAntCorr();
  int _processChillXmitSample();
  int _processChillPhaseCode();

  int _processIwrfRadarInfo();
  int _processIwrfScanSeg();
  int _processIwrfProcInfo();
  int _processIwrfXmitPower();
  int _processIwrfEventNotice();
  int _processIwrfCalib();
  int _processIwrfXmitInfo();
  int _processIwrfAntCorr();
  int _processIwrfXmitSample();
  int _processIwrfPhaseCode();

};

#endif

