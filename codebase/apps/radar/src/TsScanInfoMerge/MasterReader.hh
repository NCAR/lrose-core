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
// MasterReader
//
// Read the master FMQ
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2010
//
///////////////////////////////////////////////////////////////

#ifndef MasterReader_hh
#define MasterReader_hh

#include <string>
#include <toolsa/pmu.h>
#include <Fmq/DsFmq.hh>
#include <didss/DsMessage.hh>
#include <didss/DsMsgPart.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include "Params.hh"
using namespace std;

////////////////////////
// Base class

class MasterReader {
  
public:

  // constructor
  
  MasterReader(const Params &params);
  
  // destructor
  
  ~MasterReader();

  // get next pulse
  // returns NULL at end of data, or error
  
  IwrfTsPulse *getNextPulse();

  // get latest structs
  
  inline const IwrfTsInfo &getOpsInfo() const { 
    return _opsInfo; 
  }
  
  inline const iwrf_radar_info_t *getLatestRadarInfo() const { 
    return &_latestRadarInfo; 
  }

  inline const iwrf_scan_segment_t *getLatestScanSeg() const {
    return &_latestScanSeg;
  }

  inline const iwrf_ts_processing_t *getLatestTsProc() const {
    return &_latestTsProc;
  }

  inline const iwrf_pulse_header_t *getLatestPulseHdr() const {
    return &_latestPulseHdr;
  }

  inline const iwrf_pulse_header_t *getPrevPulseHdr() const {
    return &_prevPulseHdr;
  }

  // get sizes of FMQ
  
  inline int getFmqNumSlots() const {
    return _fmq.getNumSlots();
  }
  
  inline int getFmqBufSize() const {
    return _fmq.getBufSize();
  }

  // get time from first pulse in queue to the one provided
  
  double getTimeSinceStart(const iwrf_pulse_header_t &phdr);

protected:
private:
  
  const Params &_params;

  // input queue

  DsFmq _fmq;
  bool _fmqIsOpen;

  // input message

  DsMessage _msg;
  DsMsgPart *_part;
  int _nParts;
  int _pos;
  IwrfTsInfo _opsInfo;

  // save latest packets

  iwrf_radar_info_t _latestRadarInfo;
  iwrf_scan_segment_t _latestScanSeg;
  iwrf_ts_processing_t _latestTsProc;
  iwrf_pulse_header_t _firstPulseHdr;
  iwrf_pulse_header_t _latestPulseHdr;
  iwrf_pulse_header_t _prevPulseHdr;
  long long _pulseCount;

  int _getNextPart();

};

#endif

